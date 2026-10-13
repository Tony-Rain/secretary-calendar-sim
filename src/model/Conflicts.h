#pragma once

#include <string>
#include <vector>

#include "Calendar.h"

namespace sec {

enum class ConflictKind {
    PersonBusy,
    RoomBusy,
    RoomTooSmall,
};

struct Conflict {
    ConflictKind kind = ConflictKind::PersonBusy;
    const Event* other = nullptr;
    Employee* person = nullptr;
    std::string text;
};

class ConflictChecker {
public:
    ConflictChecker(const Calendar& calendar, const WorkSchedule& schedule)
        : calendar_(calendar), schedule_(schedule) {}

    std::vector<Conflict> check(const Event& event, int skipId = 0) const;

private:
    const Calendar& calendar_;
    const WorkSchedule& schedule_;
};

enum class FixType {
    MoveNew,
    ChangeRoom,
    RemovePeople,
    MoveOld,
};

struct FixOption {
    FixType type = FixType::MoveNew;
    std::string text;
    bool adminOnly = false;
    Minutes newStart = 0;
    Room* newRoom = nullptr; // для ChangeRoom
    std::vector<Employee*> people;
    int oldEventId = 0;
};

class ConflictResolver {
public:
    ConflictResolver(Calendar& calendar, const Company& company, const WorkSchedule& schedule)
        : calendar_(calendar), company_(company), schedule_(schedule),
          checker_(calendar, schedule) {}

    std::vector<FixOption> suggest(const Event& newEvent, const std::vector<Conflict>& conflicts,
                                   Minutes now, int skipId = 0) const;
    void apply(const FixOption& option, Event& newEvent);

    int chooseAuto(const std::vector<FixOption>& options, bool isAdmin) const;

    // -1 если не нашли
    Minutes findFreeTime(const Event& event, Minutes from, int skipId,
                         const Event* avoid = nullptr) const;

private:
    bool fitsWorkTime(const Event& event, Minutes start) const;

    Calendar& calendar_;
    const Company& company_;
    const WorkSchedule& schedule_;
    ConflictChecker checker_;
};

}
