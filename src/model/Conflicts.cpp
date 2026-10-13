#include "Conflicts.h"

#include <algorithm>

namespace sec {

const Minutes kSlot = 30;  // время подбирается с шагом в полчаса

// "«Семинар» (Ср 07.10 14:00)"
static std::string eventLabel(const Event& e, const Clock& clock) {
    return "«" + e.title() + "» (" + clock.timeString(e.start()) + ")";
}

// ---------- ConflictChecker ----------

std::vector<Conflict> ConflictChecker::check(const Event& event, int skipId) const {
    std::vector<Conflict> result;
    const Clock& clock = schedule_.clock;

    // 1. Хватает ли мест в помещении.
    Room* room = event.room();
    int count = static_cast<int>(event.participants().size());
    if (room && count > room->capacity()) {
        Conflict c;
        c.kind = ConflictKind::RoomTooSmall;
        c.text = "В помещении «" + room->name() + "» " + std::to_string(room->capacity()) +
                 " мест, а участников " + std::to_string(count);
        result.push_back(c);
    }

    // 2. Пересечения с другими событиями календаря.
    for (const auto& other : calendar_.events()) {
        if (other->id() == skipId || !event.overlaps(*other)) continue;

        if (room && other->room() == room) {
            Conflict c;
            c.kind = ConflictKind::RoomBusy;
            c.other = other.get();
            c.text = "Помещение «" + room->name() + "» занято: " + eventLabel(*other, clock);
            result.push_back(c);
        }

        // Планёрку можно пропустить — это не конфликт.
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

// ---------- ConflictResolver ----------

std::vector<FixOption> ConflictResolver::suggest(const Event& newEvent,
                                                 const std::vector<Conflict>& conflicts,
                                                 Minutes now, int skipId) const {
    std::vector<FixOption> options;
    const Clock& clock = schedule_.clock;

    // Какие проблемы есть.
    bool roomProblem = false;
    std::vector<Employee*> busyPeople;
    std::vector<const Event*> otherEvents;
    for (const Conflict& c : conflicts) {
        if (c.kind == ConflictKind::RoomBusy || c.kind == ConflictKind::RoomTooSmall)
            roomProblem = true;
        if (c.person && std::find(busyPeople.begin(), busyPeople.end(), c.person) == busyPeople.end())
            busyPeople.push_back(c.person);
        if (c.other && std::find(otherEvents.begin(), otherEvents.end(), c.other) == otherEvents.end())
            otherEvents.push_back(c.other);
    }

    // 1. Перенести новое событие: сначала ищем позже, потом с текущего момента.
    Minutes t = findFreeTime(newEvent, std::max(newEvent.start(), now), skipId);
    if (t < 0) t = findFreeTime(newEvent, now, skipId);
    if (t >= 0) {
        FixOption o;
        o.type = FixType::MoveNew;
        o.text = "Перенести новое событие на " + clock.timeString(t);
        o.newStart = t;
        options.push_back(o);
    }

    // 2. Другое помещение (не больше двух вариантов).
    if (roomProblem && newEvent.room()) {
        int found = 0;
        for (const auto& room : company_.rooms()) {
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

    // 3. Не приглашать занятых (если кто-то останется).
    if (!busyPeople.empty() && busyPeople.size() < newEvent.participants().size()) {
        FixOption o;
        o.type = FixType::RemovePeople;
        o.text = "Не приглашать:";
        for (Employee* p : busyPeople) o.text += " " + p->name();
        o.people = busyPeople;
        options.push_back(o);
    }

    // 4. Перенести уже назначенное событие (только админ, и только если оно ещё не началось).
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
    for (size_t i = 0; i < options.size(); ++i)
        if (!options[i].adminOnly || isAdmin) return static_cast<int>(i);
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
    Minutes t = (from + kSlot - 1) / kSlot * kSlot;  // округляем вверх до получаса
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

}  // namespace sec
