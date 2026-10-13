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
    virtual std::unique_ptr<Event> clone() const = 0;
    // Участник может пропустить событие ради другого — это не конфликт (планёрки).
    virtual bool allowsAbsence() const { return false; }
    // Проходит ли событие в помещении фирмы.
    virtual bool needsRoom() const { return true; }
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

    bool overlaps(Minutes from, Minutes to) const { return start_ < to && from < end(); }
    bool overlaps(const Event& other) const { return overlaps(other.start(), other.end()); }
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
    Room* room_;
    Importance importance_;
    std::vector<Employee*> participants_;
    int remindersSent_ = 0;
    Minutes lastReminder_ = -1;
};

// Шаблон для однотипной реализации kind() и clone() в наследниках.
template <typename Derived, EventKind K>
class EventOf : public Event {
public:
    using Event::Event;
    EventKind kind() const override { return K; }
    std::unique_ptr<Event> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

// Ежедневная планёрка отдела.
class DailyMeeting : public EventOf<DailyMeeting, EventKind::DailyMeeting> {
public:
    DailyMeeting(std::string title, Minutes start, Minutes duration, Room* room,
                 Importance importance, Department* department);
    bool allowsAbsence() const override { return true; }
    const Department* ownerDepartment() const override { return department_; }

private:
    Department* department_;
};

// Совещание руководителей отделов.
class ManagersMeeting : public EventOf<ManagersMeeting, EventKind::ManagersMeeting> {
public:
    using EventOf::EventOf;
    const Department* ownerDepartment() const override { return nullptr; }
};

// Тематический семинар (участники из разных отделов).
class Seminar : public EventOf<Seminar, EventKind::Seminar> {
public:
    using EventOf::EventOf;
};

// Рабочее совещание группы сотрудников.
class WorkMeeting : public EventOf<WorkMeeting, EventKind::WorkMeeting> {
public:
    using EventOf::EventOf;
};

// Командировка: проходит вне фирмы, может длиться несколько дней.
class BusinessTrip : public EventOf<BusinessTrip, EventKind::BusinessTrip> {
public:
    using EventOf::EventOf;
    bool needsRoom() const override { return false; }
};

// Создание события нужного вида (для диалога добавления).
std::unique_ptr<Event> makeEvent(EventKind kind, std::string title, Minutes start,
                                 Minutes duration, Room* room, Importance importance,
                                 Department* department = nullptr);

}  // namespace sec
