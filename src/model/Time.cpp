#include "Time.h"

namespace sec {

Clock::Clock(std::chrono::year_month_day start) : start_(std::chrono::sys_days{start}) {}

static std::string twoDigits(int n) {
    if (n < 10) return "0" + std::to_string(n);
    return std::to_string(n);
}

int Clock::weekdayOf(int day) const {
    std::chrono::weekday wd{start_ + std::chrono::days{day}};
    int iso = wd.iso_encoding();
    return iso - 1;
}

std::string Clock::dayString(int day) const {
    const char* names[] = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
    std::chrono::year_month_day date{start_ + std::chrono::days{day}};
    int dayOfMonth = static_cast<unsigned>(date.day());
    int month = static_cast<unsigned>(date.month());
    return std::string(names[weekdayOf(day)]) + " " + twoDigits(dayOfMonth) + "." + twoDigits(month);
}

std::string Clock::hhmm(Minutes t) {
    Minutes m = minuteOfDay(t);
    return twoDigits(m / 60) + ":" + twoDigits(m % 60);
}

std::string Clock::timeString(Minutes t) const {
    return dayString(dayOf(t)) + " " + hhmm(t);
}

}
