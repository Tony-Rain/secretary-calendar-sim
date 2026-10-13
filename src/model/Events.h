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

// Календарное событие фирмы (абстрактный базовый класс).
class Event {
public:
    Event(std::string title, Minutes start, Minutes duration, Room* room, Importance importance);
    virtual ~Event() = default;

    virtual EventKind kind() const = 0;
    // Копия события (нужна, чтобы примерить изменения, не трогая оригинал).
    virtual std::unique_ptr<Event> clone() const = 0;
    // Можно ли пропустить событие ради другого без конфликта (планёрка).
    virtual bool allowsAbsence() const { return false; }
    // Отдел, к которому относится событие (для фильтра и цвета); nullptr — общее.
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
    void setTitle(std::string title) { title_ = std::move(title); }
    void setStart(Minutes start) { start_ = start; }
    void setDuration(Minutes duration) { duration_ = duration; }
    void setRoom(Room* room) { room_ = room; }
    void setImportance(Importance imp) { importance_ = imp; }
    void setParticipants(std::vector<Employee*> people) { participants_ = std::move(people); }
    void removeParticipant(const Employee* e);

    // Пересекаются ли события по времени.
    bool overlaps(const Event& other) const { return start_ < other.end() && other.start() < end(); }
    bool hasParticipant(const Employee* e) const;
    bool involves(const Department* d) const;

    // Учёт отправленных напоминаний.
    int remindersSent() const { return remindersSent_; }
    Minutes lastReminder() const { return lastReminder_; }
    void markReminded(Minutes t) { ++remindersSent_; lastReminder_ = t; }

private:
    int id_ = 0;
    std::string title_;
    Minutes start_;
    Minutes duration_;
    Room* room_;  // nullptr — событие вне фирмы (командировка)
    Importance importance_;
    std::vector<Employee*> participants_;
    int remindersSent_ = 0;
    Minutes lastReminder_ = -1;
};

// Ежедневная планёрка отдела.
class DailyMeeting : public Event {
public:
    DailyMeeting(std::string title, Minutes start, Minutes duration, Room* room,
                 Importance importance, Department* department);
    EventKind kind() const override { return EventKind::DailyMeeting; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<DailyMeeting>(*this); }
    bool allowsAbsence() const override { return true; }
    const Department* ownerDepartment() const override { return department_; }

private:
    Department* department_;
};

// Совещание руководителей отделов.
class ManagersMeeting : public Event {
public:
    using Event::Event;
    EventKind kind() const override { return EventKind::ManagersMeeting; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<ManagersMeeting>(*this); }
    const Department* ownerDepartment() const override { return nullptr; }
};

// Тематический семинар (участники из разных отделов).
class Seminar : public Event {
public:
    using Event::Event;
    EventKind kind() const override { return EventKind::Seminar; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<Seminar>(*this); }
};

// Рабочее совещание группы сотрудников.
class WorkMeeting : public Event {
public:
    using Event::Event;
    EventKind kind() const override { return EventKind::WorkMeeting; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<WorkMeeting>(*this); }
};

// Командировка: проходит вне фирмы (без помещения), может длиться несколько дней.
class BusinessTrip : public Event {
public:
    BusinessTrip(std::string title, Minutes start, Minutes duration, Importance importance)
        : Event(std::move(title), start, duration, nullptr, importance) {}
    EventKind kind() const override { return EventKind::BusinessTrip; }
    std::unique_ptr<Event> clone() const override { return std::make_unique<BusinessTrip>(*this); }
};

}  // namespace sec
