#include "Calendar.h"

#include <algorithm>

namespace sec {

bool EventFilter::matches(const Event& e) const {
    if (employee) return e.hasParticipant(employee);
    if (department) return e.involves(department);
    return true;
}

int Calendar::add(std::unique_ptr<Event> event) {
    if (event->id() == 0) event->setId(nextId_++);
    markAbsences(*event);
    int id = event->id();
    events_.push_back(std::move(event));
    return id;
}

std::unique_ptr<Event> Calendar::remove(int id) {
    for (size_t i = 0; i < events_.size(); ++i) {
        if (events_[i]->id() == id) {
            std::unique_ptr<Event> result = std::move(events_[i]);
            events_.erase(events_.begin() + i);
            return result;
        }
    }
    return nullptr;
}

Event* Calendar::find(int id) const {
    for (const std::unique_ptr<Event>& e : events_)
        if (e->id() == id) return e.get();
    return nullptr;
}

std::vector<std::unique_ptr<Event>> Calendar::removeFinished(Minutes now) {
    std::vector<std::unique_ptr<Event>> finished;
    std::vector<std::unique_ptr<Event>> rest;
    for (std::unique_ptr<Event>& e : events_) {
        if (e->end() <= now)
            finished.push_back(std::move(e));
        else
            rest.push_back(std::move(e));
    }
    events_ = std::move(rest);  // в календаре остаются только будущие события
    return finished;
}

static bool byStart(const Event* a, const Event* b) { return a->start() < b->start(); }

std::vector<Event*> Calendar::query(Minutes from, Minutes to, const EventFilter& filter) const {
    std::vector<Event*> result;
    for (const std::unique_ptr<Event>& e : events_) {
        bool inPeriod = e->start() < to && from < e->end();
        if (inPeriod && filter.matches(*e)) result.push_back(e.get());
    }
    std::sort(result.begin(), result.end(), byStart);
    return result;
}

void Calendar::markAbsences(Event& added) {
    for (std::unique_ptr<Event>& other : events_) {
        if (!other->overlaps(added)) continue;
        if (other->allowsAbsence() && !added.allowsAbsence()) {
            for (Employee* p : added.participants()) other->removeParticipant(p);
        } else if (added.allowsAbsence() && !other->allowsAbsence()) {
            for (Employee* p : other->participants()) added.removeParticipant(p);
        }
    }
}

}  // namespace sec
