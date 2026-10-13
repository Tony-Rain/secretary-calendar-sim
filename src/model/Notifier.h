#pragma once

#include <string>
#include <vector>

#include "Calendar.h"

namespace sec {

struct ReminderRule {
    int daysBefore;  // за сколько дней (0 - не напоминать)
    int everyHours;  // раз в сколько часов (0 - один раз)
};

struct Message {
    Minutes time;
    const Employee* to;
    std::string eventTitle;
    Minutes eventStart;
};

class Notifier {
public:
    explicit Notifier(const ReminderRule rules[3]);

    std::vector<Message> process(Calendar& calendar, Minutes now);

private:
    bool isTimeToRemind(const Event& e, Minutes now) const;

    ReminderRule rules_[3];
};

}
