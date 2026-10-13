#pragma once

#include <memory>
#include <random>
#include <vector>

#include "Conflicts.h"

namespace sec {

class EventGenerator {
public:
    EventGenerator(const Company& company, const WorkSchedule& schedule, std::mt19937& rng)
        : company_(company), schedule_(schedule), rng_(rng) {}

    void fillInitial(Calendar& calendar, const ConflictChecker& checker);

    bool requestArrives(double perDay, Minutes step);

    std::unique_ptr<Event> randomRequest(Minutes now);

private:
    int randomInt(int from, int to);
    int randomIndex(int size);
    Importance randomImportance();
    std::vector<Employee*> randomPeople(std::vector<Employee*> from, int count);
    Minutes randomStart(int day, Minutes duration);
    std::unique_ptr<Event> makeTrip(int day);

    const Company& company_;
    const WorkSchedule& schedule_;
    std::mt19937& rng_;
};

}
