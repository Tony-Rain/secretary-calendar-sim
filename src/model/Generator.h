#pragma once

#include <memory>
#include <random>
#include <vector>

#include "Conflicts.h"

namespace sec {

// Генерация событий: начальный календарь и случайные заявки во время моделирования.
class EventGenerator {
public:
    EventGenerator(const Company& company, const WorkSchedule& schedule, std::mt19937& rng)
        : company_(company), schedule_(schedule), rng_(rng) {}

    // Регулярные события на весь период: планёрки, совещания руководителей,
    // семинары, плюс пара командировок.
    void fillInitial(Calendar& calendar, const ConflictChecker& checker);

    // Пришла ли заявка на этом шаге (в среднем perDay заявок за рабочий день).
    bool requestArrives(double perDay, Minutes step);

    // Случайная заявка на одно из ближайших дней; nullptr, если период уже кончается.
    std::unique_ptr<Event> randomRequest(Minutes now);

private:
    int randomInt(int from, int to);  // включая обе границы
    Importance randomImportance();
    std::vector<Employee*> randomPeople(std::vector<Employee*> from, int count);
    Minutes randomStart(int day, Minutes duration);
    std::unique_ptr<Event> makeTrip(int day);

    const Company& company_;
    const WorkSchedule& schedule_;
    std::mt19937& rng_;
};

}  // namespace sec
