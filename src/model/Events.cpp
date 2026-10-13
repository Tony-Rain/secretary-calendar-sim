#include "Events.h"

#include <algorithm>

namespace sec {

const char* importanceName(Importance imp) {
    switch (imp) {
        case Importance::Low: return "низкая";
        case Importance::Medium: return "средняя";
        case Importance::High: return "высокая";
    }
    return "";
}

const char* eventKindName(EventKind kind) {
    switch (kind) {
        case EventKind::DailyMeeting: return "Планёрка";
        case EventKind::ManagersMeeting: return "Совещание руководителей";
        case EventKind::Seminar: return "Семинар";
        case EventKind::WorkMeeting: return "Рабочее совещание";
        case EventKind::BusinessTrip: return "Командировка";
    }
    return "";
}

Event::Event(std::string title, Minutes start, Minutes duration, Room* room, Importance importance)
    : title_(std::move(title)), start_(start), duration_(duration), room_(room),
      importance_(importance) {}

const Department* Event::ownerDepartment() const {
    if (participants_.empty()) return nullptr;
    const Department* d = participants_.front()->department();
    for (const Employee* e : participants_)
        if (e->department() != d) return nullptr;
    return d;
}

void Event::removeParticipant(const Employee* e) {
    participants_.erase(std::remove(participants_.begin(), participants_.end(), e),
                        participants_.end());
}

bool Event::hasParticipant(const Employee* e) const {
    return std::find(participants_.begin(), participants_.end(), e) != participants_.end();
}

bool Event::involves(const Department* d) const {
    if (ownerDepartment() == d) return true;
    return std::any_of(participants_.begin(), participants_.end(),
                       [d](const Employee* e) { return e->department() == d; });
}

DailyMeeting::DailyMeeting(std::string title, Minutes start, Minutes duration, Room* room,
                           Importance importance, Department* department)
    : EventOf(std::move(title), start, duration, room, importance), department_(department) {}

std::unique_ptr<Event> makeEvent(EventKind kind, std::string title, Minutes start,
                                 Minutes duration, Room* room, Importance importance,
                                 Department* department) {
    switch (kind) {
        case EventKind::DailyMeeting:
            return std::make_unique<DailyMeeting>(std::move(title), start, duration, room,
                                                  importance, department);
        case EventKind::ManagersMeeting:
            return std::make_unique<ManagersMeeting>(std::move(title), start, duration, room,
                                                     importance);
        case EventKind::Seminar:
            return std::make_unique<Seminar>(std::move(title), start, duration, room, importance);
        case EventKind::WorkMeeting:
            return std::make_unique<WorkMeeting>(std::move(title), start, duration, room,
                                                 importance);
        case EventKind::BusinessTrip:
            return std::make_unique<BusinessTrip>(std::move(title), start, duration, nullptr,
                                                  importance);
    }
    return nullptr;
}

}  // namespace sec
