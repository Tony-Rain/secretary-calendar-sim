#include "Company.h"

#include <set>

namespace sec {

Person::Person(int id, std::string name, std::string email)
    : id_(id), name_(std::move(name)), email_(std::move(email)) {}

Employee::Employee(int id, std::string name, std::string email, Department* department)
    : Person(id, std::move(name), std::move(email)), department_(department) {}

Room::Room(int id, std::string name, int capacity)
    : id_(id), name_(std::move(name)), capacity_(capacity) {}

DepartmentRoom::DepartmentRoom(int id, std::string name, int capacity, Department* owner)
    : Room(id, std::move(name), capacity), owner_(owner) {}

Department::Department(int id, std::string name) : id_(id), name_(std::move(name)) {}

std::vector<Employee*> Department::staff() const {
    std::vector<Employee*> result;
    for (const auto& e : staff_) result.push_back(e.get());
    return result;
}

Employee* Department::addEmployee(std::unique_ptr<Employee> e) {
    Employee* raw = e.get();
    if (raw->isHead()) head_ = static_cast<DepartmentHead*>(raw);
    staff_.push_back(std::move(e));
    return raw;
}

namespace {

const char* kDepartmentNames[] = {
    "Бухгалтерия", "Отдел кадров", "Отдел продаж",      "Маркетинг",   "IT-отдел",
    "Юридический отдел", "Логистика", "Отдел закупок", "Аналитический отдел",
};

const char* kSurnames[] = {
    "Иванов",   "Петров",   "Сидоров",  "Смирнов",   "Кузнецов", "Попов",    "Васильев",
    "Соколов",  "Михайлов", "Новиков",  "Фёдоров",   "Морозов",  "Волков",   "Алексеев",
    "Лебедев",  "Семёнов",  "Егоров",   "Павлов",    "Козлов",   "Степанов", "Николаев",
    "Орлов",    "Андреев",  "Макаров",  "Никитин",   "Захаров",  "Зайцев",   "Соловьёв",
    "Борисов",  "Яковлев",  "Григорьев", "Романов",  "Воробьёв", "Сергеев",  "Кузьмин",
    "Фролов",   "Александров", "Дмитриев", "Королёв", "Гусев",   "Киселёв",  "Ильин",
    "Максимов", "Поляков",  "Сорокин",  "Виноградов", "Ковалёв", "Белов",    "Медведев",
};

const char* kInitials[] = {"А", "Б", "В", "Г", "Д", "Е", "И", "К", "Л", "М",
                           "Н", "О", "П", "Р", "С", "Т", "Ф", "Ю", "Я"};

template <typename T, size_t N>
const T& pick(const T (&arr)[N], std::mt19937& rng) {
    return arr[std::uniform_int_distribution<size_t>(0, N - 1)(rng)];
}

}  // namespace

std::unique_ptr<Company> Company::generate(int departments, int staffPerDepartment,
                                           std::mt19937& rng) {
    auto company = std::make_unique<Company>();
    std::set<std::string> usedNames;
    int nextPersonId = 1;
    int nextRoomId = 1;

    auto uniqueName = [&]() {
        std::string name;
        do {
            name = std::string(pick(kSurnames, rng)) + " " + pick(kInitials, rng) + "." +
                   pick(kInitials, rng) + ".";
        } while (!usedNames.insert(name).second);
        return name;
    };

    auto hall = std::make_unique<ConferenceHall>(nextRoomId++, "Конференц-зал", 40);
    company->hall_ = hall.get();
    company->rooms_.push_back(std::move(hall));

    for (int d = 0; d < departments; ++d) {
        auto dept = std::make_unique<Department>(d, kDepartmentNames[d % 9]);
        auto room = std::make_unique<DepartmentRoom>(
            nextRoomId++, "Каб. " + std::to_string(101 + d * 100), staffPerDepartment + 3,
            dept.get());
        dept->setRoom(room.get());
        company->rooms_.push_back(std::move(room));

        for (int i = 0; i <= staffPerDepartment; ++i) {
            int id = nextPersonId++;
            std::string email = "emp" + std::to_string(id) + "@firm.local";
            std::unique_ptr<Employee> e;
            if (i == 0)
                e = std::make_unique<DepartmentHead>(id, uniqueName(), email, dept.get());
            else
                e = std::make_unique<Employee>(id, uniqueName(), email, dept.get());
            dept->addEmployee(std::move(e));
        }
        company->departments_.push_back(std::move(dept));
    }
    return company;
}

std::vector<Employee*> Company::allEmployees() const {
    std::vector<Employee*> result;
    for (const auto& d : departments_)
        for (Employee* e : d->staff()) result.push_back(e);
    return result;
}

std::vector<Employee*> Company::heads() const {
    std::vector<Employee*> result;
    for (const auto& d : departments_) result.push_back(d->head());
    return result;
}

Employee* Company::findEmployee(int id) const {
    for (const auto& d : departments_)
        for (Employee* e : d->staff())
            if (e->id() == id) return e;
    return nullptr;
}

Room* Company::findRoom(int id) const {
    for (const auto& r : rooms_)
        if (r->id() == id) return r.get();
    return nullptr;
}

}  // namespace sec
