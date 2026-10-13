#include "Notifier.h"

namespace sec {

Notifier::Notifier(const ReminderRule rules[3]) {
    for (int i = 0; i < 3; ++i) rules_[i] = rules[i];
}

bool Notifier::isTimeToRemind(const Event& e, Minutes now) const {
    const ReminderRule& rule = rules_[static_cast<int>(e.importance())];
    Minutes left = e.start() - now;
    if (left <= 0 || left > rule.daysBefore * kMinutesPerDay) return false;  // ещё рано или уже поздно

    if (e.remindersSent() == 0) return true;  // первое напоминание
    if (rule.everyHours == 0) return false;   // напоминать только один раз
    return now - e.lastReminder() >= rule.everyHours * kMinutesPerHour;
}

std::vector<Message> Notifier::process(Calendar& calendar, Minutes now) {
    std::vector<Message> sent;
    for (const auto& e : calendar.events()) {
        if (!isTimeToRemind(*e, now)) continue;
        for (const Employee* p : e->participants())
            sent.push_back(Message{now, p, e->title(), e->start()});
        e->markReminded(now);
    }
    return sent;
}

}  // namespace sec
