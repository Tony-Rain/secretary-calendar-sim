#pragma once

#include <string>
#include <vector>

#include "Calendar.h"

namespace sec {

// Виды конфликтов.
enum class ConflictKind {
    PersonBusy,    // сотрудник в это время уже участвует в другом событии
    RoomBusy,      // помещение в это время занято
    RoomTooSmall,  // участников больше, чем мест в помещении
};

struct Conflict {
    ConflictKind kind = ConflictKind::PersonBusy;
    const Event* other = nullptr;      // событие, с которым конфликт
    Employee* person = nullptr;        // занятый сотрудник (для PersonBusy)
    std::string text;                  // описание для пользователя
};

// Проверка события на конфликты с календарём.
class ConflictChecker {
public:
    ConflictChecker(const Calendar& calendar, const WorkSchedule& schedule)
        : calendar_(calendar), schedule_(schedule) {}

    // skipId — id события, которое не учитывать (когда изменяем уже существующее).
    std::vector<Conflict> check(const Event& event, int skipId = 0) const;

private:
    const Calendar& calendar_;
    const WorkSchedule& schedule_;
};

// Способы разрешения конфликта.
enum class FixType {
    MoveNew,       // перенести новое событие на свободное время
    ChangeRoom,    // провести новое событие в другом помещении
    RemovePeople,  // не приглашать занятых сотрудников
    MoveOld,       // перенести уже назначенное событие (только админ)
};

struct FixOption {
    FixType type = FixType::MoveNew;
    std::string text;        // описание для пользователя
    bool adminOnly = false;  // меняет уже назначенное событие
    Minutes newStart = 0;    // для MoveNew и MoveOld
    Room* newRoom = nullptr; // для ChangeRoom
    std::vector<Employee*> people;  // для RemovePeople
    int oldEventId = 0;      // для MoveOld
};

// Подбор и применение способов разрешения конфликтов.
class ConflictResolver {
public:
    ConflictResolver(Calendar& calendar, const Company& company, const WorkSchedule& schedule)
        : calendar_(calendar), company_(company), schedule_(schedule),
          checker_(calendar, schedule) {}

    std::vector<FixOption> suggest(const Event& newEvent, const std::vector<Conflict>& conflicts,
                                   Minutes now, int skipId = 0) const;
    void apply(const FixOption& option, Event& newEvent);

    // Для автоматического режима: номер первого разрешённого варианта или -1.
    int chooseAuto(const std::vector<FixOption>& options, bool isAdmin) const;

    // Ближайшее время не раньше from, когда событие проходит без конфликтов
    // (в рабочие часы и в пределах периода). avoid — событие, с которым нельзя
    // пересекаться по времени. Возвращает -1, если такого времени нет.
    Minutes findFreeTime(const Event& event, Minutes from, int skipId,
                         const Event* avoid = nullptr) const;

private:
    bool fitsWorkTime(const Event& event, Minutes start) const;

    Calendar& calendar_;
    const Company& company_;
    const WorkSchedule& schedule_;
    ConflictChecker checker_;
};

}  // namespace sec
