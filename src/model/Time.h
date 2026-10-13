#pragma once

#include <chrono>
#include <string>

namespace sec {

// Модельное время: минуты от 00:00 первого дня периода моделирования.
using Minutes = int;

constexpr Minutes kMinutesPerHour = 60;
constexpr Minutes kMinutesPerDay = 24 * kMinutesPerHour;

inline int dayOf(Minutes t) { return t / kMinutesPerDay; }
inline Minutes minuteOfDay(Minutes t) { return t % kMinutesPerDay; }
inline Minutes dayStart(int day) { return day * kMinutesPerDay; }

// Перевод модельного времени в календарные даты и строки для вывода.
class Clock {
public:
    explicit Clock(std::chrono::year_month_day start = std::chrono::year_month_day{
                       std::chrono::year{2026}, std::chrono::month{10}, std::chrono::day{5}});

    std::chrono::year_month_day dateOf(int day) const;
    int weekdayOf(int day) const;  // 0 = понедельник ... 6 = воскресенье
    bool isWeekend(int day) const { return weekdayOf(day) >= 5; }

    std::string dayString(int day) const;     // "Пн 05.10"
    std::string timeString(Minutes t) const;  // "Пн 05.10 14:30"
    static std::string hhmm(Minutes t);       // "14:30"
    static const char* weekdayName(int weekday);

private:
    std::chrono::sys_days start_;
};

}  // namespace sec
