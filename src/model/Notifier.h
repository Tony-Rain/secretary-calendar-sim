#pragma once

#include <string>
#include <vector>

#include "Calendar.h"

namespace sec {

// Правило напоминаний для одной степени важности.
struct ReminderRule {
    int daysBefore;  // за сколько дней до события начинать (0 — не напоминать)
    int everyHours;  // как часто повторять (0 — один раз)
};

// Отправленное напоминание («письмо» сотруднику).
struct Message {
    Minutes time;
    const Employee* to;
    std::string eventTitle;
    Minutes eventStart;
};

// Рассылка напоминаний участникам предстоящих событий.
class Notifier {
public:
    // rules[0..2] — правила для важности Low, Medium, High.
    explicit Notifier(const ReminderRule rules[3]);

    // Проверяет все события и рассылает нужные напоминания. Возвращает отправленные письма.
    std::vector<Message> process(Calendar& calendar, Minutes now);

private:
    bool isTimeToRemind(const Event& e, Minutes now) const;

    ReminderRule rules_[3];
};

}  // namespace sec
