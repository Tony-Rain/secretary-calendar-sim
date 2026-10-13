#include "Simulation.h"

namespace sec {

static WorkSchedule makeSchedule(const SimulationParams& p) {
    WorkSchedule s;
    s.clock = Clock(p.startDate);
    s.workStart = p.workStart;
    s.workEnd = p.workEnd;
    s.days = p.days;
    return s;
}

Simulation::Simulation(const SimulationParams& params)
    : params_(params),
      rng_(params.seed),
      schedule_(makeSchedule(params)),
      company_(Company::generate(params.departments, params.staffPerDepartment, rng_)),
      checker_(calendar_, schedule_),
      resolver_(calendar_, *company_, schedule_),
      notifier_(params.reminders),
      generator_(*company_, schedule_, rng_),
      role_(params.role) {
    generator_.fillInitial(calendar_, checker_);
    now_ = nextWorkingTime(0);
    for (int day = 0; day < schedule_.days; ++day)
        if (schedule_.isWorkingDay(day)) totalSteps_ += schedule_.workdayLength() / params_.step;
    addLog("Начало моделирования. Событий в календаре: " +
           std::to_string(calendar_.events().size()));
}

// Ближайший рабочий момент не раньше t (ночи и выходные пропускаются).
Minutes Simulation::nextWorkingTime(Minutes t) const {
    while (t < schedule_.periodEnd()) {
        int day = dayOf(t);
        Minutes m = minuteOfDay(t);
        if (!schedule_.isWorkingDay(day) || m >= schedule_.workEnd)
            t = dayStart(day + 1) + schedule_.workStart;
        else if (m < schedule_.workStart)
            t = dayStart(day) + schedule_.workStart;
        else
            return t;
    }
    return schedule_.periodEnd();
}

void Simulation::step() {
    if (finished() || hasPending()) return;

    now_ = nextWorkingTime(now_ + params_.step);
    ++stepNumber_;

    // 1. Убираем прошедшие события.
    removeFinished();
    if (finished()) {
        addLog("Моделирование завершено");
        return;
    }

    // 2. Рассылаем напоминания.
    sendReminders();

    // 3. Возможно, поступила новая заявка.
    if (generator_.requestArrives(params_.requestsPerDay, params_.step)) {
        std::unique_ptr<Event> event = generator_.randomRequest(now_);
        if (event) {
            ++stats_.requests;
            addLog("Новая заявка: «" + event->title() + "» на " +
                   schedule_.clock.timeString(event->start()));
            handleRequest(std::move(event), 0, params_.conflictMode == ConflictMode::Auto);
        }
    }
}

void Simulation::removeFinished() {
    for (auto& e : calendar_.removeFinished(now_)) {
        ++stats_.held;
        if (e->room() == company_->conferenceHall()) stats_.hallBusy += e->duration();
        addLog("Состоялось: «" + e->title() + "»");
        history_.push_back(std::move(e));
    }
}

void Simulation::sendReminders() {
    std::vector<Message> sent = notifier_.process(calendar_, now_);
    stats_.reminders += static_cast<int>(sent.size());

    // Письма об одном событии идут подряд — пишем в журнал одну строку на событие.
    size_t i = 0;
    while (i < sent.size()) {
        size_t j = i;
        while (j < sent.size() && sent[j].eventTitle == sent[i].eventTitle &&
               sent[j].eventStart == sent[i].eventStart)
            ++j;
        addLog("Напоминание о «" + sent[i].eventTitle + "» (" +
               schedule_.clock.timeString(sent[i].eventStart) + ") отправлено " +
               std::to_string(j - i) + " участникам");
        i = j;
    }
    for (const Message& m : sent) messages_.push_back(m);
}

void Simulation::submit(std::unique_ptr<Event> event, int replacesId) {
    ++stats_.requests;
    handleRequest(std::move(event), replacesId, false);  // пользователь всегда решает сам
}

void Simulation::handleRequest(std::unique_ptr<Event> event, int replacesId, bool autoResolve) {
    std::vector<Conflict> conflicts = checker_.check(*event, replacesId);
    if (conflicts.empty()) {
        commit(std::move(event), replacesId);
        return;
    }

    ++stats_.conflicts;
    addLog("Конфликт у «" + event->title() + "»: " + conflicts[0].text);

    if (autoResolve) {
        resolveAutomatically(std::move(event), replacesId);
        return;
    }
    pending_ = std::make_unique<PendingRequest>();
    pending_->event = std::move(event);
    pending_->replacesId = replacesId;
    recheckPending();
}

void Simulation::resolveAutomatically(std::unique_ptr<Event> event, int replacesId) {
    std::vector<Conflict> conflicts = checker_.check(*event, replacesId);
    // Несколько попыток: после одного исправления может остаться другой конфликт.
    for (int attempt = 0; attempt < 5 && !conflicts.empty(); ++attempt) {
        std::vector<FixOption> options = resolver_.suggest(*event, conflicts, now_, replacesId);
        int i = resolver_.chooseAuto(options, isAdmin());
        if (i < 0) break;
        resolver_.apply(options[i], *event);
        addLog("Решено автоматически: " + options[i].text);
        conflicts = checker_.check(*event, replacesId);
    }

    if (conflicts.empty()) {
        commit(std::move(event), replacesId);
    } else {
        ++stats_.rejected;
        addLog("Заявка «" + event->title() + "» отклонена: конфликт не удалось разрешить");
    }
}

// Пересчитать конфликты ожидающей заявки; если их нет — внести её в календарь.
void Simulation::recheckPending() {
    pending_->conflicts = checker_.check(*pending_->event, pending_->replacesId);
    if (pending_->conflicts.empty()) {
        commit(std::move(pending_->event), pending_->replacesId);
        pending_.reset();
        return;
    }
    pending_->options =
        resolver_.suggest(*pending_->event, pending_->conflicts, now_, pending_->replacesId);
}

void Simulation::choosePending(int optionIndex) {
    if (!pending_ || optionIndex < 0 || optionIndex >= static_cast<int>(pending_->options.size()))
        return;
    const FixOption& option = pending_->options[optionIndex];
    if (option.adminOnly && !isAdmin()) return;

    resolver_.apply(option, *pending_->event);
    addLog("Выбрано: " + option.text);
    recheckPending();
}

void Simulation::replacePending(std::unique_ptr<Event> edited) {
    if (!pending_) return;
    pending_->event = std::move(edited);
    addLog("Заявка «" + pending_->event->title() + "» изменена вручную");
    recheckPending();
}

void Simulation::rejectPending() {
    if (!pending_) return;
    ++stats_.rejected;
    addLog("Заявка «" + pending_->event->title() + "» отклонена");
    pending_.reset();
}

bool Simulation::cancelEvent(int id) {
    if (!isAdmin()) return false;
    std::unique_ptr<Event> e = calendar_.remove(id);
    if (!e) return false;
    ++stats_.cancelled;
    addLog("Отменено: «" + e->title() + "»");
    return true;
}

void Simulation::commit(std::unique_ptr<Event> event, int replacesId) {
    std::string text = "«" + event->title() + "» на " + schedule_.clock.timeString(event->start());
    if (replacesId != 0) {
        calendar_.remove(replacesId);
        event->setId(replacesId);
        addLog("Изменено: " + text);
    } else {
        addLog("Добавлено в календарь: " + text);
    }
    calendar_.add(std::move(event));
}

void Simulation::addLog(const std::string& text) {
    log_.push_back(LogEntry{now_, text});
}

}  // namespace sec
