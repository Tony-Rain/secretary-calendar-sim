#include "Company.h"

#include <set>

namespace sec {

Person::Person(int id, const std::string& name, const std::string& email)
    : id_(id), name_(name), email_(email) {}

Employee::Employee(int id, const std::string& name, const std::string& email,
                   Department* department)
    : Person(id, name, email), department_(department) {}

Room::Room(int id, const std::string& name, int capacity)
    : id_(id), name_(name), capacity_(capacity) {}

Department::Department(int id, const std::string& name) : id_(id), name_(name) {}

std::vector<Employee*> Department::staff() const {
    std::vector<Employee*> result;
    for (const std::unique_ptr<Employee>& e : staff_) result.push_back(e.get());
    return result;
}

static const std::vector<std::string> kDepartmentNames = {
    "Бухгалтерия", "Отдел кадров", "Отдел продаж",      "Маркетинг",   "IT-отдел",
    "Юридический отдел", "Логистика", "Отдел закупок", "Аналитический отдел",
};

static const std::vector<std::string> kSurnames = {
    "Иванов",   "Петров",   "Сидоров",  "Смирнов",   "Кузнецов", "Попов",    "Васильев",
    "Соколов",  "Михайлов", "Новиков",  "Фёдоров",   "Морозов",  "Волков",   "Алексеев",
    "Лебедев",  "Семёнов",  "Егоров",   "Павлов",    "Козлов",   "Степанов", "Николаев",
    "Орлов",    "Андреев",  "Макаров",  "Никитин",   "Захаров",  "Зайцев",   "Соловьёв",
    "Борисов",  "Яковлев",  "Григорьев", "Романов",  "Воробьёв", "Сергеев",  "Кузьмин",
    "Фролов",   "Александров", "Дмитриев", "Королёв", "Гусев",   "Киселёв",  "Ильин",
    "Максимов", "Поляков",  "Сорокин",  "Виноградов", "Ковалёв", "Белов",    "Медведев",
};

static const std::vector<std::string> kInitials = {"А", "Б", "В", "Г", "Д", "Е", "И", "К", "Л", "М",
                           "Н", "О", "П", "Р", "С", "Т", "Ф", "Ю", "Я"};

// Случайный элемент списка.
static std::string pick(const std::vector<std::string>& items, std::mt19937& rng) {
    int last = items.size() - 1;
    std::uniform_int_distribution<int> dist(0, last);  // равномерно от 0 до last
    return items[dist(rng)];
}

// Случайное ФИО вида "Петров А.С.", не совпадающее с уже выданными.
static std::string uniqueName(std::set<std::string>& used, std::mt19937& rng) {
    while (true) {
        std::string name = pick(kSurnames, rng) + " " + pick(kInitials, rng) + "." +
                           pick(kInitials, rng) + ".";
        if (used.count(name) == 0) {
            used.insert(name);
            return name;
        }
    }
}

std::unique_ptr<Company> Company::generate(int departments, int staffPerDepartment,
                                           std::mt19937& rng) {
    auto company = std::make_unique<Company>();
    std::set<std::string> usedNames;
    int nextPersonId = 1;
    int nextRoomId = 1;

    auto hall = std::make_unique<ConferenceHall>(nextRoomId++, "Конференц-зал", 40);
    company->hall_ = hall.get();
    company->rooms_.push_back(std::move(hall));

    for (int d = 0; d < departments; ++d) {
        auto dept = std::make_unique<Department>(d, kDepartmentNames[d % kDepartmentNames.size()]);
        auto room = std::make_unique<DepartmentRoom>(
            nextRoomId++, "Каб. " + std::to_string(101 + d * 100), staffPerDepartment + 3);
        dept->setRoom(room.get());
        company->rooms_.push_back(std::move(room));

        // Сначала руководитель, потом сотрудники.
        for (int i = 0; i <= staffPerDepartment; ++i) {
            int id = nextPersonId++;
            std::string name = uniqueName(usedNames, rng);
            std::string email = "emp" + std::to_string(id) + "@firm.local";
            if (i == 0) {
                auto head = std::make_unique<DepartmentHead>(id, name, email, dept.get());
                dept->setHead(head.get());
                dept->addEmployee(std::move(head));
            } else {
                dept->addEmployee(std::make_unique<Employee>(id, name, email, dept.get()));
            }
        }
        company->departments_.push_back(std::move(dept));
    }
    return company;
}

std::vector<Employee*> Company::allEmployees() const {
    std::vector<Employee*> result;
    for (const std::unique_ptr<Department>& d : departments_)
        for (Employee* e : d->staff()) result.push_back(e);
    return result;
}

std::vector<Employee*> Company::heads() const {
    std::vector<Employee*> result;
    for (const std::unique_ptr<Department>& d : departments_) result.push_back(d->head());
    return result;
}

Employee* Company::findEmployee(int id) const {
    for (const std::unique_ptr<Department>& d : departments_)
        for (Employee* e : d->staff())
            if (e->id() == id) return e;
    return nullptr;
}

}  // namespace sec
