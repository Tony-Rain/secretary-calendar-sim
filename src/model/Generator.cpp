#include "Generator.h"

#include <algorithm>

namespace sec {

static const std::vector<std::string> kTopics = {
    "Отчётность за квартал", "Новые клиенты",  "Бюджет",         "Обучение персонала",
    "Охрана труда",          "Внедрение CRM",  "Планирование",   "Безопасность данных",
    "Корпоративная этика",   "Логистика поставок",
};

static const std::vector<std::string> kCities = {
    "Санкт-Петербург", "Казань", "Новосибирск", "Екатеринбург", "Нижний Новгород", "Самара",
};

// Случайное целое от from до to включительно (равномерное распределение).
int EventGenerator::randomInt(int from, int to) {
    std::uniform_int_distribution<int> dist(from, to);
    return dist(rng_);
}

// Случайный номер элемента в списке из size элементов.
int EventGenerator::randomIndex(int size) {
    return randomInt(0, size - 1);
}

// Низкая — 30%, средняя — 50%, высокая — 20%.
Importance EventGenerator::randomImportance() {
    int roll = randomInt(1, 100);
    if (roll <= 30) return Importance::Low;
    if (roll <= 80) return Importance::Medium;
    return Importance::High;
}

// count случайных людей из списка (без повторов).
std::vector<Employee*> EventGenerator::randomPeople(std::vector<Employee*> from, int count) {
    std::shuffle(from.begin(), from.end(), rng_);  // перемешиваем список
    int size = from.size();
    if (count < size) from.resize(count);          // и берём первых count человек
    return from;
}

// Случайное начало в рабочие часы дня day, с шагом в полчаса.
Minutes EventGenerator::randomStart(int day, Minutes duration) {
    int slots = (schedule_.workdayLength() - duration) / 30;
    return dayStart(day) + schedule_.workStart + randomInt(0, slots) * 30;
}

std::unique_ptr<Event> EventGenerator::makeTrip(int day) {
    int daysLeft = schedule_.days - day;
    int n = randomInt(1, std::min(3, daysLeft));
    std::vector<Employee*> all = company_.allEmployees();
    Employee* who = all[randomIndex(all.size())];
    std::string city = kCities[randomIndex(kCities.size())];

    auto trip = std::make_unique<BusinessTrip>("Командировка: " + who->name() + " → " + city,
                                               dayStart(day) + schedule_.workStart,
                                               schedule_.tripDuration(n), Importance::Medium);
    trip->setParticipants({who});
    return trip;
}

void EventGenerator::fillInitial(Calendar& calendar, const ConflictChecker& checker) {
    Room* hall = company_.conferenceHall();
    for (int day = 0; day < schedule_.days; ++day) {
        if (!schedule_.isWorkingDay(day)) continue;
        Minutes morning = dayStart(day) + schedule_.workStart;

        // Планёрка каждого отдела в начале дня.
        for (const std::unique_ptr<Department>& dept : company_.departments()) {
            auto meeting = std::make_unique<DailyMeeting>("Планёрка: " + dept->name(), morning, 30,
                                                          dept->room(), Importance::Low, dept.get());
            meeting->setParticipants(dept->staff());
            calendar.add(std::move(meeting));
        }

        int weekday = schedule_.clock.weekdayOf(day);
        // По понедельникам — совещание руководителей.
        if (weekday == 0) {
            auto meeting = std::make_unique<ManagersMeeting>("Совещание руководителей",
                                                             morning + 60, 60, hall, Importance::High);
            meeting->setParticipants(company_.heads());
            calendar.add(std::move(meeting));
        }
        // По средам — семинар для сотрудников разных отделов.
        if (weekday == 2) {
            std::string topic = kTopics[randomIndex(kTopics.size())];
            auto seminar = std::make_unique<Seminar>("Семинар «" + topic + "»", morning + 5 * 60,
                                                     120, hall, Importance::Medium);
            seminar->setParticipants(randomPeople(company_.allEmployees(), randomInt(6, 12)));
            calendar.add(std::move(seminar));
        }
    }

    // Две командировки в первую неделю (без конфликтов).
    int added = 0;
    for (int attempt = 0; attempt < 20 && added < 2; ++attempt) {
        int day = randomInt(0, std::min(6, schedule_.days - 1));
        if (!schedule_.isWorkingDay(day)) continue;
        std::unique_ptr<Event> trip = makeTrip(day);
        if (checker.check(*trip).empty()) {
            calendar.add(std::move(trip));
            ++added;
        }
    }
}

// Вероятность заявки на шаге: p = (заявок в день) * (шаг) / (длина рабочего дня).
// Например, 3 заявки в день, шаг 30 мин, день 540 мин: p = 3 * 30 / 540 = 1/6.
bool EventGenerator::requestArrives(double perDay, Minutes step) {
    double probability = perDay * step / schedule_.workdayLength();
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng_) < probability;
}

std::unique_ptr<Event> EventGenerator::randomRequest(Minutes now) {
    // День: от завтра до +5 дней, ближайший рабочий.
    int day = dayOf(now) + randomInt(1, 5);
    while (day < schedule_.days && !schedule_.isWorkingDay(day)) ++day;
    if (day >= schedule_.days) return nullptr;

    Room* hall = company_.conferenceHall();
    std::string topic = kTopics[randomIndex(kTopics.size())];
    int roll = randomInt(1, 100);

    if (roll <= 45) {
        // Рабочее совещание: несколько человек одного отдела, иногда гость из другого.
        const std::vector<std::unique_ptr<Department>>& depts = company_.departments();
        Department* dept = depts[randomIndex(depts.size())].get();
        std::vector<Employee*> people = randomPeople(dept->staff(), randomInt(2, 5));
        Room* room = dept->room();
        if (randomInt(1, 100) <= 30) {
            Department* guestDept = depts[randomIndex(depts.size())].get();
            Employee* guest = randomPeople(guestDept->staff(), 1)[0];
            if (guestDept != dept) {
                people.push_back(guest);
                room = hall;  // с гостями — в конференц-зале
            }
        }
        Minutes duration = randomInt(1, 3) * 30;
        auto meeting = std::make_unique<WorkMeeting>("Совещание: " + topic,
                                                     randomStart(day, duration), duration, room,
                                                     randomImportance());
        meeting->setParticipants(people);
        return meeting;
    }
    if (roll <= 70) {
        // Семинар для сотрудников разных отделов.
        Minutes duration = randomInt(1, 2) * 60;
        auto seminar = std::make_unique<Seminar>("Семинар «" + topic + "»",
                                                 randomStart(day, duration), duration, hall,
                                                 randomImportance());
        seminar->setParticipants(randomPeople(company_.allEmployees(), randomInt(6, 15)));
        return seminar;
    }
    if (roll <= 85) {
        return makeTrip(day);
    }
    // Внеплановое совещание руководителей.
    auto meeting = std::make_unique<ManagersMeeting>("Внеплановое совещание руководителей",
                                                     randomStart(day, 60), 60, hall,
                                                     Importance::High);
    meeting->setParticipants(company_.heads());
    return meeting;
}

}  // namespace sec
