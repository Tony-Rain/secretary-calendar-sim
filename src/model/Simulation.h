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

enum class ConflictMode { Manual, Auto };
enum class UserRole { Admin, User };

struct SimulationParams {
    int departments = 6;  // K
    int staffPerDepartment = 6;
    int days = 14;  // N
    Minutes step = 30;
    std::chrono::year_month_day startDate{std::chrono::year{2026}, std::chrono::month{10},
                                          std::chrono::day{5}};
    Minutes workStart = 9 * 60, workEnd = 18 * 60;
    double requestsPerDay = 3.0;
    unsigned seed = 1;
    ReminderRule reminders[3] = {{1, 0}, {2, 24}, {4, 24}};  // low, medium, high
    ConflictMode conflictMode = ConflictMode::Manual;
    UserRole role = UserRole::Admin;
};

struct LogEntry {
    Minutes time;
    std::string text;
};

struct Statistics {
    int held = 0;
    int cancelled = 0;
    int requests = 0;
    int conflicts = 0;
    int rejected = 0;
    int reminders = 0;
    Minutes hallBusy = 0;
};

struct PendingRequest {
    std::unique_ptr<Event> event;
    int replacesId = 0;  // 0 - новое событие
    std::vector<Conflict> conflicts;
    std::vector<FixOption> options;
};

class Simulation {
public:
    explicit Simulation(const SimulationParams& params);

    void step();
    bool finished() const { return now_ >= schedule_.periodEnd(); }
    Minutes now() const { return now_; }
    int stepNumber() const { return stepNumber_; }
    int totalSteps() const { return totalSteps_; }

    void submit(std::unique_ptr<Event> event, int replacesId = 0);
    bool hasPending() const { return pending_ != nullptr; }
    const PendingRequest& pending() const { return *pending_; }
    void choosePending(int optionIndex);
    void replacePending(std::unique_ptr<Event> edited);
    void rejectPending();
    bool cancelEvent(int id);

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
    void handleRequest(std::unique_ptr<Event> event, int replacesId, bool autoResolve);
    void resolveAutomatically(std::unique_ptr<Event> event, int replacesId,
                              std::vector<Conflict> conflicts);
    void recheckPending();
    void commit(std::unique_ptr<Event> event, int replacesId);
    void addLog(const std::string& text);

    // порядок полей важен (инициализация)
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

}
