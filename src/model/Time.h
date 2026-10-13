#pragma once

#include <chrono>
#include <string>

namespace sec {

// время в минутах от начала первого дня
using Minutes = int;

constexpr Minutes kMinutesPerHour = 60;
constexpr Minutes kMinutesPerDay = 24 * kMinutesPerHour;

inline int dayOf(Minutes t) { return t / kMinutesPerDay; }
inline Minutes minuteOfDay(Minutes t) { return t % kMinutesPerDay; }
inline Minutes dayStart(int day) { return day * kMinutesPerDay; }

class Clock {
public:
    explicit Clock(std::chrono::year_month_day start = std::chrono::year_month_day{
                       std::chrono::year{2026}, std::chrono::month{10}, std::chrono::day{5}});

    int weekdayOf(int day) const;  // 0 - пн, 6 - вс
    bool isWeekend(int day) const { return weekdayOf(day) >= 5; }

    std::string dayString(int day) const;
    std::string timeString(Minutes t) const;
    static std::string hhmm(Minutes t);

private:
    std::chrono::sys_days start_;
};

struct WorkSchedule {
    Clock clock;
    Minutes workStart = 9 * kMinutesPerHour;
    Minutes workEnd = 18 * kMinutesPerHour;
    int days = 14;

    Minutes workdayLength() const { return workEnd - workStart; }
    Minutes periodEnd() const { return dayStart(days - 1) + workEnd; }
    bool isWorkingDay(int day) const { return day >= 0 && day < days && !clock.isWeekend(day); }
    // командировка с утра первого дня до вечера последнего
    Minutes tripDuration(int n) const { return (n - 1) * kMinutesPerDay + workdayLength(); }
};

}
