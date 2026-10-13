#pragma once

#include <memory>
#include <vector>

#include "Events.h"

namespace sec {

struct EventFilter {
    const Department* department = nullptr;
    const Employee* employee = nullptr;

    bool matches(const Event& e) const;
};

class Calendar {
public:
    int add(std::unique_ptr<Event> event);
    std::unique_ptr<Event> remove(int id);
    Event* find(int id) const;

    std::vector<std::unique_ptr<Event>> removeFinished(Minutes now);

    std::vector<Event*> query(Minutes from, Minutes to, const EventFilter& filter = {}) const;

    const std::vector<std::unique_ptr<Event>>& events() const { return events_; }

private:
    // кто занят другим событием - убираем из планерки
    void markAbsences(Event& added);

    std::vector<std::unique_ptr<Event>> events_;
    int nextId_ = 1;
};

}
