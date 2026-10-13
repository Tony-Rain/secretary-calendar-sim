#pragma once

#include <memory>
#include <random>
#include <string>
#include <vector>

namespace sec {

class Department;

// ---------- Люди ----------

class Person {
public:
    Person(int id, std::string name, std::string email);
    virtual ~Person() = default;

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& email() const { return email_; }
    virtual std::string position() const = 0;

private:
    int id_;
    std::string name_;
    std::string email_;
};

// Сотрудник фирмы, приписан к одному отделу.
class Employee : public Person {
public:
    Employee(int id, std::string name, std::string email, Department* department);

    Department* department() const { return department_; }
    virtual bool isHead() const { return false; }
    std::string position() const override { return "Сотрудник"; }

private:
    Department* department_;
};

// Руководитель отдела — участник совещаний руководителей.
class DepartmentHead : public Employee {
public:
    using Employee::Employee;
    bool isHead() const override { return true; }
    std::string position() const override { return "Руководитель отдела"; }
};

// ---------- Помещения ----------

class Room {
public:
    Room(int id, std::string name, int capacity);
    virtual ~Room() = default;

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    int capacity() const { return capacity_; }
    virtual bool isShared() const = 0;  // общее для всей фирмы?

private:
    int id_;
    std::string name_;
    int capacity_;
};

class DepartmentRoom : public Room {
public:
    DepartmentRoom(int id, std::string name, int capacity, Department* owner);
    Department* owner() const { return owner_; }
    bool isShared() const override { return false; }

private:
    Department* owner_;
};

class ConferenceHall : public Room {
public:
    using Room::Room;
    bool isShared() const override { return true; }
};

// ---------- Отдел и фирма ----------

class Department {
public:
    Department(int id, std::string name);

    int id() const { return id_; }  // 0..K-1, используется и для цвета
    const std::string& name() const { return name_; }
    DepartmentRoom* room() const { return room_; }
    DepartmentHead* head() const { return head_; }
    std::vector<Employee*> staff() const;  // руководитель + сотрудники

    void setRoom(DepartmentRoom* room) { room_ = room; }
    Employee* addEmployee(std::unique_ptr<Employee> e);

private:
    int id_;
    std::string name_;
    DepartmentRoom* room_ = nullptr;
    DepartmentHead* head_ = nullptr;
    std::vector<std::unique_ptr<Employee>> staff_;
};

class Company {
public:
    // Генерация фирмы: K отделов, в каждом руководитель и staffPerDepartment сотрудников.
    static std::unique_ptr<Company> generate(int departments, int staffPerDepartment,
                                             std::mt19937& rng);

    const std::vector<std::unique_ptr<Department>>& departments() const { return departments_; }
    const std::vector<std::unique_ptr<Room>>& rooms() const { return rooms_; }
    ConferenceHall* conferenceHall() const { return hall_; }

    std::vector<Employee*> allEmployees() const;
    std::vector<Employee*> heads() const;
    Employee* findEmployee(int id) const;
    Room* findRoom(int id) const;

private:
    std::vector<std::unique_ptr<Department>> departments_;
    std::vector<std::unique_ptr<Room>> rooms_;
    ConferenceHall* hall_ = nullptr;
};

}  // namespace sec
