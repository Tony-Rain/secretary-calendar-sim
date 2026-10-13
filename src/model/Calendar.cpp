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
    nextId_ = std::max(nextId_, event->id() + 1);
    markAbsences(*event);
    int id = event->id();
    events_.push_back(std::move(event));
    return id;
}

std::unique_ptr<Event> Calendar::remove(int id) {
    auto it = std::find_if(events_.begin(), events_.end(),
                           [id](const auto& e) { return e->id() == id; });
    if (it == events_.end()) return nullptr;
    auto result = std::move(*it);
    events_.erase(it);
    return result;
}

Event* Calendar::find(int id) const {
    for (const auto& e : events_)
        if (e->id() == id) return e.get();
    return nullptr;
}

std::vector<std::unique_ptr<Event>> Calendar::extractFinished(Minutes now) {
    std::vector<std::unique_ptr<Event>> finished;
    auto split = std::stable_partition(events_.begin(), events_.end(),
                                       [now](const auto& e) { return e->end() > now; });
    for (auto it = split; it != events_.end(); ++it) finished.push_back(std::move(*it));
    events_.erase(split, events_.end());
    std::sort(finished.begin(), finished.end(),
              [](const auto& a, const auto& b) { return a->start() < b->start(); });
    return finished;
}

std::vector<Event*> Calendar::query(Minutes from, Minutes to, const EventFilter& filter) const {
    std::vector<Event*> result;
    for (const auto& e : events_)
        if (e->overlaps(from, to) && filter.matches(*e)) result.push_back(e.get());
    std::sort(result.begin(), result.end(), [](const Event* a, const Event* b) {
        return a->start() != b->start() ? a->start() < b->start() : a->id() < b->id();
    });
    return result;
}

void Calendar::markAbsences(Event& added) {
    for (const auto& other : events_) {
        if (!other->overlaps(added) || added.allowsAbsence() == other->allowsAbsence()) continue;
        Event& optional = added.allowsAbsence() ? added : *other;
        const Event& required = added.allowsAbsence() ? *other : added;
        for (const Employee* e : required.participants()) optional.removeParticipant(e);
    }
}

}  // namespace sec
