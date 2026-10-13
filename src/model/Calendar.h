#pragma once

#include <memory>
#include <vector>

#include "Events.h"

namespace sec {

// Что показывать: вся фирма (оба поля пустые), отдел или конкретный сотрудник.
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
    // Убирает событие из календаря и отдаёт его вызывающему.
    std::unique_ptr<Event> remove(int id);
    Event* find(int id) const;

    // Убирает из календаря события, закончившиеся к моменту now, и возвращает их.
    std::vector<std::unique_ptr<Event>> removeFinished(Minutes now);

    // События, идущие в промежутке [from, to) и подходящие под фильтр, по времени начала.
    std::vector<Event*> query(Minutes from, Minutes to, const EventFilter& filter = {}) const;

    const std::vector<std::unique_ptr<Event>>& events() const { return events_; }

private:
    // Если новое событие пересекается с планёркой, его участники
    // просто пропускают планёрку (убираются из её участников).
    void markAbsences(Event& added);

    std::vector<std::unique_ptr<Event>> events_;
    int nextId_ = 1;
};

}  // namespace sec
