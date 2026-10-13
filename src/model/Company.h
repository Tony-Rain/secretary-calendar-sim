#pragma once

#include <memory>
#include <random>
#include <string>
#include <vector>

namespace sec {

class Department;

class Person {
public:
    Person(int id, const std::string& name, const std::string& email);
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

class Employee : public Person {
public:
    Employee(int id, const std::string& name, const std::string& email, Department* department);

    Department* department() const { return department_; }
    virtual bool isHead() const { return false; }
    std::string position() const override { return "Сотрудник"; }

private:
    Department* department_;
};

class DepartmentHead : public Employee {
public:
    DepartmentHead(int id, const std::string& name, const std::string& email, Department* department)
        : Employee(id, name, email, department) {}
    bool isHead() const override { return true; }
    std::string position() const override { return "Руководитель отдела"; }
};

class Room {
public:
    Room(int id, const std::string& name, int capacity);
    virtual ~Room() = default;

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    int capacity() const { return capacity_; }
    virtual bool isShared() const = 0;

private:
    int id_;
    std::string name_;
    int capacity_;
};

class DepartmentRoom : public Room {
public:
    DepartmentRoom(int id, const std::string& name, int capacity) : Room(id, name, capacity) {}
    bool isShared() const override { return false; }
};

class ConferenceHall : public Room {
public:
    ConferenceHall(int id, const std::string& name, int capacity) : Room(id, name, capacity) {}
    bool isShared() const override { return true; }
};

class Department {
public:
    Department(int id, const std::string& name);

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    DepartmentRoom* room() const { return room_; }
    DepartmentHead* head() const { return head_; }
    std::vector<Employee*> staff() const;

    void setRoom(DepartmentRoom* room) { room_ = room; }
    void setHead(DepartmentHead* head) { head_ = head; }
    void addEmployee(std::unique_ptr<Employee> e) { staff_.push_back(std::move(e)); }

private:
    int id_;
    std::string name_;
    DepartmentRoom* room_ = nullptr;
    DepartmentHead* head_ = nullptr;
    std::vector<std::unique_ptr<Employee>> staff_;
};

class Company {
public:
    static std::unique_ptr<Company> generate(int departments, int staffPerDepartment,
                                             std::mt19937& rng);

    const std::vector<std::unique_ptr<Department>>& departments() const { return departments_; }
    const std::vector<std::unique_ptr<Room>>& rooms() const { return rooms_; }
    ConferenceHall* conferenceHall() const { return hall_; }

    std::vector<Employee*> allEmployees() const;
    std::vector<Employee*> heads() const;
    Employee* findEmployee(int id) const;

private:
    std::vector<std::unique_ptr<Department>> departments_;
    std::vector<std::unique_ptr<Room>> rooms_;
    ConferenceHall* hall_ = nullptr;
};

}
