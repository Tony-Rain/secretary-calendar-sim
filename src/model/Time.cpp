#include "Time.h"

#include <cstdio>

namespace sec {

Clock::Clock(std::chrono::year_month_day start) : start_(std::chrono::sys_days{start}) {}

int Clock::weekdayOf(int day) const {
    std::chrono::weekday wd{start_ + std::chrono::days{day}};
    return static_cast<int>(wd.iso_encoding()) - 1;  // iso: 1 = Пн ... 7 = Вс
}

std::string Clock::dayString(int day) const {
    static const char* names[] = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
    std::chrono::year_month_day d{start_ + std::chrono::days{day}};
    char buf[32];
    std::snprintf(buf, sizeof buf, "%s %02u.%02u", names[weekdayOf(day)],
                  static_cast<unsigned>(d.day()), static_cast<unsigned>(d.month()));
    return buf;
}

std::string Clock::hhmm(Minutes t) {
    Minutes m = minuteOfDay(t);
    char buf[16];
    std::snprintf(buf, sizeof buf, "%02d:%02d", m / 60, m % 60);
    return buf;
}

std::string Clock::timeString(Minutes t) const {
    return dayString(dayOf(t)) + " " + hhmm(t);
}

}  // namespace sec
