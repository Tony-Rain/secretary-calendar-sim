#pragma once

#include <chrono>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "Conflicts.h"
#include "Generator.h"
#include "Notifier.h"

namespace sec {

enum class ConflictMode { Manual, Auto };  // Manual — решает пользователь, Auto — система
enum class UserRole { Admin, User };

// Параметры моделирования (задаются в окне параметров).
struct SimulationParams {
    int departments = 6;          // K, 5..9
    int staffPerDepartment = 6;   // сотрудников в отделе (без руководителя)
    int days = 14;                // N, 7..30
    Minutes step = 30;            // 30 или 60
    std::chrono::year_month_day startDate{std::chrono::year{2026}, std::chrono::month{10},
                                          std::chrono::day{5}};
    Minutes workStart = 9 * 60, workEnd = 18 * 60;
    double requestsPerDay = 3.0;  // среднее число случайных заявок в день
    unsigned seed = 1;
    ReminderRule reminders[3] = {{1, 0}, {2, 24}, {4, 24}};  // индекс = (int)Importance
    ConflictMode conflictMode = ConflictMode::Manual;
    UserRole role = UserRole::Admin;
};

// Запись журнала.
struct LogEntry {
    Minutes time;
    std::string text;
};

// Итоговая статистика.
struct Statistics {
    int held = 0;          // состоялось событий
    int cancelled = 0;     // отменено админом
    int requests = 0;      // поступило заявок
    int conflicts = 0;     // заявок с конфликтами
    int rejected = 0;      // отклонено заявок
    int reminders = 0;     // отправлено напоминаний
    Minutes hallBusy = 0;  // сколько минут был занят конференц-зал
};

// Заявка с конфликтом, ждущая решения пользователя.
struct PendingRequest {
    std::unique_ptr<Event> event;  // новое (или изменённое) событие
    int replacesId = 0;            // id изменяемого события, 0 — новое
    std::vector<Conflict> conflicts;
    std::vector<FixOption> options;
};

// Моделирование работы календаря фирмы во времени.
class Simulation {
public:
    explicit Simulation(const SimulationParams& params);

    // Время
    void step();  // один шаг; ничего не делает, если hasPending() или finished()
    bool finished() const { return now_ >= schedule_.periodEnd(); }
    Minutes now() const { return now_; }
    int stepNumber() const { return stepNumber_; }
    int totalSteps() const { return totalSteps_; }

    // Добавление и изменение событий пользователем
    void submit(std::unique_ptr<Event> event, int replacesId = 0);
    bool hasPending() const { return pending_ != nullptr; }
    const PendingRequest& pending() const { return *pending_; }
    void choosePending(int optionIndex);
    void replacePending(std::unique_ptr<Event> edited);
    void rejectPending();
    bool cancelEvent(int id);  // только админ

    UserRole role() const { return role_; }
    void setRole(UserRole role) { role_ = role; }
    bool isAdmin() const { return role_ == UserRole::Admin; }

    const SimulationParams& params() const { return params_; }
    const WorkSchedule& schedule() const { return schedule_; }
    const Company& company() const { return *company_; }
    const Calendar& calendar() const { return calendar_; }
    const std::vector<LogEntry>& log() const { return log_; }
    const std::vector<Message>& messages() const { return messages_; }
    const std::vector<std::unique_ptr<Event>>& history() const { return history_; }
    const Statistics& stats() const { return stats_; }

private:
    Minutes nextWorkingTime(Minutes t) const;
    void removeFinished();
    void sendReminders();
    // Обработка заявки: без конфликтов — в календарь, иначе авто-решение или ожидание.
    void handleRequest(std::unique_ptr<Event> event, int replacesId, bool autoResolve);
    void resolveAutomatically(std::unique_ptr<Event> event, int replacesId,
                              std::vector<Conflict> conflicts);
    void recheckPending();
    void commit(std::unique_ptr<Event> event, int replacesId);
    void addLog(const std::string& text);

    // Порядок полей важен: они создаются в этом порядке.
    SimulationParams params_;
    std::mt19937 rng_;
    WorkSchedule schedule_;
    std::unique_ptr<Company> company_;
    Calendar calendar_;
    ConflictChecker checker_;
    ConflictResolver resolver_;
    Notifier notifier_;
    EventGenerator generator_;

    UserRole role_;
    Minutes now_ = 0;
    int stepNumber_ = 0;
    int totalSteps_ = 0;
    std::unique_ptr<PendingRequest> pending_;

    std::vector<LogEntry> log_;
    std::vector<Message> messages_;
    std::vector<std::unique_ptr<Event>> history_;
    Statistics stats_;
};

}  // namespace sec
