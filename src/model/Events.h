#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Company.h"
#include "Time.h"

namespace sec {

enum class Importance { Low = 0, Medium = 1, High = 2 };
const char* importanceName(Importance imp);

enum class EventKind { DailyMeeting, ManagersMeeting, Seminar, WorkMeeting, BusinessTrip };
const char* eventKindName(EventKind kind);

class Event {
public:
    Event(const std::string& title, Minutes start, Minutes duration, Room* room,
          Importance importance);
    virtual ~Event() = default;

    virtual EventKind kind() const = 0;
    virtual std::unique_ptr<Event> clone() const = 0;
    // планерку можно пропустить, это не конфликт
    virtual bool allowsAbsence() const { return false; }
    virtual const Department* ownerDepartment() const;

    int id() const { return id_; }
    const std::string& title() const { return title_; }
    Minutes start() const { return start_; }
    Minutes duration() const { return duration_; }
    Minutes end() const { return start_ + duration_; }
    Room* room() const { return room_; }
    Importance importance() const { return importance_; }
    const std::vector<Employee*>& participants() const { return participants_; }

    void setId(int id) { id_ = id; }
    void setTitle(const std::string& title) { title_ = title; }
    void setStart(Minutes start) { start_ = start; }
    void setDuration(Minutes duration) { duration_ = duration; }
    void setRoom(Room* room) { room_ = room; }
    void setImportance(Importance imp) { importance_ = imp; }
    void setParticipants(const std::vector<Employee*>& people) { participants_ = people; }
    void removeParticipant(const Employee* e);

    bool overlaps(const Event& other) const { return start_ < other.end() && other.start() < end(); }
    bool hasParticipant(const Employee* e) const;
    bool involves(const Department* d) const;

    int remindersSent() const { return remindersSent_; }
    Minutes lastReminder() const { return lastReminder_; }
    void markReminded(Minutes t) { ++remindersSent_; lastReminder_ = t; }

private:
    int id_ = 0;
    std::string title_;
    Minutes start_;
    Minutes duration_;
    Room* room_;  // nullptr у командировки
    Importance importance_;
    std::vector<Employee*> participants_;
    int remindersSent_ = 0;
    Minutes lastReminder_ = -1;
};

class DailyMeeting : public Event {
public:
    DailyMeeting(const std::string& title, Minutes start, Minutes duration, Room* room,
                 Importance importance, Department* department);
    EventKind kind() const override { return EventKind::DailyMeeting; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<DailyMeeting>(*this); }
    bool allowsAbsence() const override { return true; }
    const Department* ownerDepartment() const override { return department_; }

private:
    Department* department_;
};

class ManagersMeeting : public Event {
public:
    ManagersMeeting(const std::string& title, Minutes start, Minutes duration, Room* room,
                    Importance importance)
        : Event(title, start, duration, room, importance) {}
    EventKind kind() const override { return EventKind::ManagersMeeting; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<ManagersMeeting>(*this); }
    const Department* ownerDepartment() const override { return nullptr; }
};

class Seminar : public Event {
public:
    Seminar(const std::string& title, Minutes start, Minutes duration, Room* room,
            Importance importance)
        : Event(title, start, duration, room, importance) {}
    EventKind kind() const override { return EventKind::Seminar; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<Seminar>(*this); }
};

class WorkMeeting : public Event {
public:
    WorkMeeting(const std::string& title, Minutes start, Minutes duration, Room* room,
                Importance importance)
        : Event(title, start, duration, room, importance) {}
    EventKind kind() const override { return EventKind::WorkMeeting; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<WorkMeeting>(*this); }
};

class BusinessTrip : public Event {
public:
    BusinessTrip(const std::string& title, Minutes start, Minutes duration, Importance importance)
        : Event(title, start, duration, nullptr, importance) {}
    EventKind kind() const override { return EventKind::BusinessTrip; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<BusinessTrip>(*this); }
};

}
