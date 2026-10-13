#pragma once

#include <memory>
#include <vector>

#include "Events.h"

namespace sec {

// Критерий просмотра: вся фирма, отдел или конкретный сотрудник.
struct EventFilter {
    const Department* department = nullptr;
    const Employee* employee = nullptr;

    bool matches(const Event& e) const;
};

// Общий календарь фирмы — единственный владелец запланированных событий.
class Calendar {
public:
    // Добавляет событие; если у него нет id, присваивает новый. Возвращает id.
    int add(std::unique_ptr<Event> event);
    std::unique_ptr<Event> remove(int id);
    Event* find(int id) const;

    // Изымает из календаря завершившиеся к моменту now события.
    std::vector<std::unique_ptr<Event>> extractFinished(Minutes now);

    // События, пересекающие интервал [from, to), отсортированные по началу.
    std::vector<Event*> query(Minutes from, Minutes to, const EventFilter& filter = {}) const;

    const std::vector<std::unique_ptr<Event>>& events() const { return events_; }
    size_t size() const { return events_.size(); }

private:
    // Участник события, допускающего отсутствие, исключается из него,
    // если в это время у него другое (обязательное) событие.
    void markAbsences(Event& added);

    std::vector<std::unique_ptr<Event>> events_;
    int nextId_ = 1;
};

}  // namespace sec
