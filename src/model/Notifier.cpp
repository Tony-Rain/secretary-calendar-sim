#include "Notifier.h"

namespace sec {

Notifier::Notifier(const ReminderRule rules[3]) {
    for (int i = 0; i < 3; ++i) rules_[i] = rules[i];
}

bool Notifier::isTimeToRemind(const Event& e, Minutes now) const {
    int index = static_cast<int>(e.importance());
    const ReminderRule& rule = rules_[index];
    Minutes left = e.start() - now;
    if (left <= 0 || left > rule.daysBefore * kMinutesPerDay) return false;

    if (e.remindersSent() == 0) return true;
    if (rule.everyHours == 0) return false;
    return now - e.lastReminder() >= rule.everyHours * kMinutesPerHour;
}

std::vector<Message> Notifier::process(Calendar& calendar, Minutes now) {
    std::vector<Message> sent;
    for (const std::unique_ptr<Event>& e : calendar.events()) {
        if (!isTimeToRemind(*e, now)) continue;
        for (const Employee* p : e->participants())
            sent.push_back(Message{now, p, e->title(), e->start()});
        e->markReminded(now);
    }
    return sent;
}

}
