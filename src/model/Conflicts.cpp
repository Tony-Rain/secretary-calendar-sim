#include "Conflicts.h"

#include <algorithm>

namespace sec {

static bool containsPerson(const std::vector<Employee*>& list, Employee* p) {
    for (Employee* item : list)
        if (item == p) return true;
    return false;
}

static bool containsEvent(const std::vector<const Event*>& list, const Event* e) {
    for (const Event* item : list)
        if (item == e) return true;
    return false;
}

const Minutes kSlot = 30;

static std::string eventLabel(const Event& e, const Clock& clock) {
    return "«" + e.title() + "» (" + clock.timeString(e.start()) + ")";
}

std::vector<Conflict> ConflictChecker::check(const Event& event, int skipId) const {
    std::vector<Conflict> result;
    const Clock& clock = schedule_.clock;

    Room* room = event.room();
    int count = event.participants().size();
    if (room && count > room->capacity()) {
        Conflict c;
        c.kind = ConflictKind::RoomTooSmall;
        c.text = "В помещении «" + room->name() + "» " + std::to_string(room->capacity()) +
                 " мест, а участников " + std::to_string(count);
        result.push_back(c);
    }

    for (const std::unique_ptr<Event>& other : calendar_.events()) {
        if (other->id() == skipId || !event.overlaps(*other)) continue;

        if (room && other->room() == room) {
            Conflict c;
            c.kind = ConflictKind::RoomBusy;
            c.other = other.get();
            c.text = "Помещение «" + room->name() + "» занято: " + eventLabel(*other, clock);
            result.push_back(c);
        }

        // планерки не считаем
        if (event.allowsAbsence() || other->allowsAbsence()) continue;

        for (Employee* p : event.participants()) {
            if (other->hasParticipant(p)) {
                Conflict c;
                c.kind = ConflictKind::PersonBusy;
                c.other = other.get();
                c.person = p;
                c.text = p->name() + " в это время участвует в " + eventLabel(*other, clock);
                result.push_back(c);
            }
        }
    }
    return result;
}

std::vector<FixOption> ConflictResolver::suggest(const Event& newEvent,
                                                 const std::vector<Conflict>& conflicts,
                                                 Minutes now, int skipId) const {
    std::vector<FixOption> options;
    const Clock& clock = schedule_.clock;

    bool roomProblem = false;
    std::vector<Employee*> busyPeople;
    std::vector<const Event*> otherEvents;
    for (const Conflict& c : conflicts) {
        if (c.kind == ConflictKind::RoomBusy || c.kind == ConflictKind::RoomTooSmall)
            roomProblem = true;
        if (c.person && !containsPerson(busyPeople, c.person)) busyPeople.push_back(c.person);
        if (c.other && !containsEvent(otherEvents, c.other)) otherEvents.push_back(c.other);
    }

    Minutes t = findFreeTime(newEvent, std::max(newEvent.start(), now), skipId);
    if (t < 0) t = findFreeTime(newEvent, now, skipId);
    if (t >= 0) {
        FixOption o;
        o.type = FixType::MoveNew;
        o.text = "Перенести новое событие на " + clock.timeString(t);
        o.newStart = t;
        options.push_back(o);
    }

    if (roomProblem && newEvent.room()) {
        int found = 0;
        for (const std::unique_ptr<Room>& room : company_.rooms()) {
            if (found == 2) break;
            if (room.get() == newEvent.room()) continue;
            std::unique_ptr<Event> copy = newEvent.clone();
            copy->setRoom(room.get());
            bool roomOk = true;
            for (const Conflict& c : checker_.check(*copy, skipId))
                if (c.kind != ConflictKind::PersonBusy) roomOk = false;
            if (roomOk) {
                FixOption o;
                o.type = FixType::ChangeRoom;
                o.text = "Провести в помещении «" + room->name() + "»";
                o.newRoom = room.get();
                options.push_back(o);
                ++found;
            }
        }
    }

    if (!busyPeople.empty() && busyPeople.size() < newEvent.participants().size()) {
        FixOption o;
        o.type = FixType::RemovePeople;
        o.text = "Не приглашать:";
        for (Employee* p : busyPeople) o.text += " " + p->name();
        o.people = busyPeople;
        options.push_back(o);
    }

    for (const Event* other : otherEvents) {
        if (other->start() <= now) continue;
        Minutes t2 = findFreeTime(*other, other->start(), other->id(), &newEvent);
        if (t2 >= 0) {
            FixOption o;
            o.type = FixType::MoveOld;
            o.text = "Перенести «" + other->title() + "» на " + clock.timeString(t2);
            o.adminOnly = true;
            o.newStart = t2;
            o.oldEventId = other->id();
            options.push_back(o);
        }
    }
    return options;
}

void ConflictResolver::apply(const FixOption& option, Event& newEvent) {
    switch (option.type) {
        case FixType::MoveNew:
            newEvent.setStart(option.newStart);
            break;
        case FixType::ChangeRoom:
            newEvent.setRoom(option.newRoom);
            break;
        case FixType::RemovePeople:
            for (Employee* p : option.people) newEvent.removeParticipant(p);
            break;
        case FixType::MoveOld:
            if (Event* old = calendar_.find(option.oldEventId)) old->setStart(option.newStart);
            break;
    }
}

int ConflictResolver::chooseAuto(const std::vector<FixOption>& options, bool isAdmin) const {
    int count = options.size();
    for (int i = 0; i < count; ++i)
        if (!options[i].adminOnly || isAdmin) return i;
    return -1;
}

bool ConflictResolver::fitsWorkTime(const Event& event, Minutes start) const {
    if (!schedule_.isWorkingDay(dayOf(start))) return false;
    Minutes m = minuteOfDay(start);
    if (event.kind() == EventKind::BusinessTrip) return m == schedule_.workStart;
    return m >= schedule_.workStart && m + event.duration() <= schedule_.workEnd;
}

Minutes ConflictResolver::findFreeTime(const Event& event, Minutes from, int skipId,
                                       const Event* avoid) const {
    std::unique_ptr<Event> copy = event.clone();
    Minutes t = from;
    while (t % kSlot != 0) ++t;  // округляем до получаса

    while (t + event.duration() <= schedule_.periodEnd()) {
        if (fitsWorkTime(event, t)) {
            copy->setStart(t);
            bool free = checker_.check(*copy, skipId).empty();
            if (avoid && copy->overlaps(*avoid)) free = false;
            if (free) return t;
        }
        t += kSlot;
    }
    return -1;
}

}
