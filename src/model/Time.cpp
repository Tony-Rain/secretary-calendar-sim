#include "Time.h"

#include <cstdio>

namespace sec {

Clock::Clock(std::chrono::year_month_day start) : start_(std::chrono::sys_days{start}) {}

std::chrono::year_month_day Clock::dateOf(int day) const {
    return std::chrono::year_month_day{start_ + std::chrono::days{day}};
}

int Clock::weekdayOf(int day) const {
    std::chrono::weekday wd{start_ + std::chrono::days{day}};
    return static_cast<int>(wd.iso_encoding()) - 1;
}

const char* Clock::weekdayName(int weekday) {
    static const char* names[] = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
    return names[weekday % 7];
}

std::string Clock::dayString(int day) const {
    auto d = dateOf(day);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%s %02u.%02u", weekdayName(weekdayOf(day)),
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
