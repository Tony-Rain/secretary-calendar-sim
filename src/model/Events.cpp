#include "Events.h"

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

Event::Event(const std::string& title, Minutes start, Minutes duration, Room* room,
             Importance importance)
    : title_(title), start_(start), duration_(duration), room_(room),
      importance_(importance) {}

// Если все участники из одного отдела — событие относится к этому отделу.
const Department* Event::ownerDepartment() const {
    if (participants_.empty()) return nullptr;
    const Department* d = participants_[0]->department();
    for (const Employee* e : participants_)
        if (e->department() != d) return nullptr;
    return d;
}

void Event::removeParticipant(const Employee* e) {
    for (size_t i = 0; i < participants_.size(); ++i) {
        if (participants_[i] == e) {
            participants_.erase(participants_.begin() + i);
            return;
        }
    }
}

bool Event::hasParticipant(const Employee* e) const {
    for (const Employee* p : participants_)
        if (p == e) return true;
    return false;
}

bool Event::involves(const Department* d) const {
    if (ownerDepartment() == d) return true;
    for (const Employee* p : participants_)
        if (p->department() == d) return true;
    return false;
}

DailyMeeting::DailyMeeting(const std::string& title, Minutes start, Minutes duration, Room* room,
                           Importance importance, Department* department)
    : Event(title, start, duration, room, importance), department_(department) {}

}  // namespace sec
