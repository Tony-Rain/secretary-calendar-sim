#include "EventDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QListWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QDate>

using namespace sec;

EventDialog::EventDialog(Simulation* s, QWidget* parent) : QDialog(parent), sim(s) {
    setWindowTitle("Событие");
    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();

    comboType = new QComboBox();
    comboType->addItem("Рабочее совещание", (int)EventKind::WorkMeeting);
    comboType->addItem("Семинар", (int)EventKind::Seminar);
    comboType->addItem("Совещание руководителей", (int)EventKind::ManagersMeeting);
    comboType->addItem("Командировка", (int)EventKind::BusinessTrip);
    form->addRow("Тип:", comboType);

    editTitle = new QLineEdit();
    form->addRow("Название:", editTitle);

    QDate simStart(int(sim->params().startDate.year()), unsigned(sim->params().startDate.month()), unsigned(sim->params().startDate.day()));
    dateEdit = new QDateEdit(simStart);
    dateEdit->setMinimumDate(simStart);
    dateEdit->setMaximumDate(simStart.addDays(sim->params().days - 1));
    form->addRow("Дата:", dateEdit);

    timeEdit = new QTimeEdit(QTime(10, 0));
    form->addRow("Время:", timeEdit);

    spinDur = new QSpinBox();
    spinDur->setRange(30, 480);
    spinDur->setSingleStep(30);
    spinDur->setValue(60);
    form->addRow("Длительность (мин):", spinDur);

    spinDays = new QSpinBox();
    spinDays->setRange(1, 5);
    form->addRow("Дней (командировка):", spinDays);

    comboRoom = new QComboBox();
    for (auto& r : sim->company().rooms()) {
        comboRoom->addItem(QString::fromStdString(r->name()));
    }
    form->addRow("Помещение:", comboRoom);

    comboImp = new QComboBox();
    comboImp->addItems({"Низкая", "Средняя", "Высокая"});
    form->addRow("Важность:", comboImp);

    layout->addLayout(form);

    layout->addWidget(new QLabel("Участники:"));
    listParts = new QListWidget();
    for (auto& e : sim->company().allEmployees()) {
        QString text = QString::fromStdString(e->name() + " (" + e->department()->name() + ")");
        auto item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, e->id());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        listParts->addItem(item);
    }
    layout->addWidget(listParts);

    QPushButton* btnOk = new QPushButton("OK");
    layout->addWidget(btnOk);
    connect(btnOk, &QPushButton::clicked, this, &EventDialog::onOkClicked);
    connect(comboType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &EventDialog::onTypeChanged);
    onTypeChanged();
}

bool EventDialog::isTrip() const {
    return comboType->currentData().toInt() == (int)EventKind::BusinessTrip;
}

// у командировки нет помещения и длительность в днях
void EventDialog::onTypeChanged() {
    bool trip = isTrip();
    comboRoom->setEnabled(!trip);
    spinDur->setEnabled(!trip);
    spinDays->setEnabled(trip);
    timeEdit->setEnabled(!trip);
}

void EventDialog::onOkClicked() {
    if (editTitle->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название события");
        return;
    }

    bool hasParticipants = false;
    for (int i = 0; i < listParts->count(); ++i) {
        if (listParts->item(i)->checkState() == Qt::Checked) {
            hasParticipants = true;
            break;
        }
    }
    if (!hasParticipants) {
        QMessageBox::warning(this, "Ошибка", "Выберите участников (галочками)");
        return;
    }

    const WorkSchedule& ws = sim->schedule();
    Minutes start = startTime();
    if (!isTrip()) {
        Minutes m = minuteOfDay(start);
        if (m < ws.workStart || m + spinDur->value() > ws.workEnd) {
            QMessageBox::warning(this, "Ошибка", "Событие должно быть в рабочее время");
            return;
        }
    }
    if (start < sim->now()) {
        QMessageBox::warning(this, "Ошибка", "Нельзя назначить событие в прошлом");
        return;
    }

    accept();
}

Minutes EventDialog::startTime() const {
    QDate simStart(int(sim->params().startDate.year()), unsigned(sim->params().startDate.month()), unsigned(sim->params().startDate.day()));
    int day = simStart.daysTo(dateEdit->date());
    if (day < 0) day = 0;
    if (isTrip()) return dayStart(day) + sim->schedule().workStart;
    return dayStart(day) + timeEdit->time().hour() * 60 + timeEdit->time().minute();
}

void EventDialog::loadEvent(std::unique_ptr<Event> e) {
    original = std::move(e);
    editTitle->setText(QString::fromStdString(original->title()));

    // тип при изменении не меняется
    if (original->kind() == EventKind::DailyMeeting)
        comboType->addItem("Планёрка", (int)EventKind::DailyMeeting);
    comboType->setCurrentIndex(comboType->findData((int)original->kind()));
    comboType->setEnabled(false);

    QDate simStart(int(sim->params().startDate.year()), unsigned(sim->params().startDate.month()), unsigned(sim->params().startDate.day()));
    dateEdit->setDate(simStart.addDays(dayOf(original->start())));
    Minutes m = minuteOfDay(original->start());
    timeEdit->setTime(QTime(m / 60, m % 60));

    if (isTrip())
        spinDays->setValue(original->duration() / kMinutesPerDay + 1);
    else
        spinDur->setValue(original->duration());
    comboImp->setCurrentIndex((int)original->importance());

    const auto& rooms = sim->company().rooms();
    for (int i = 0; i < (int)rooms.size(); ++i)
        if (rooms[i].get() == original->room()) comboRoom->setCurrentIndex(i);

    for (int i = 0; i < listParts->count(); ++i) {
        int id = listParts->item(i)->data(Qt::UserRole).toInt();
        if (original->hasParticipant(sim->company().findEmployee(id)))
            listParts->item(i)->setCheckState(Qt::Checked);
    }
}

std::unique_ptr<Event> EventDialog::getResult() {
    std::string title = editTitle->text().toStdString();
    Minutes start = startTime();
    Minutes dur = isTrip() ? sim->schedule().tripDuration(spinDays->value()) : spinDur->value();
    Importance imp = (Importance)comboImp->currentIndex();

    Room* room = nullptr;
    if (!isTrip() && comboRoom->currentIndex() >= 0) {
        room = sim->company().rooms()[comboRoom->currentIndex()].get();
    }

    std::vector<Employee*> parts;
    for (int i = 0; i < listParts->count(); ++i) {
        if (listParts->item(i)->checkState() == Qt::Checked) {
            int id = listParts->item(i)->data(Qt::UserRole).toInt();
            parts.push_back(sim->company().findEmployee(id));
        }
    }

    std::unique_ptr<Event> res;
    if (original) {
        res = original->clone();  // сохраняем вид события и счетчик напоминаний
    } else {
        EventKind kind = (EventKind)comboType->currentData().toInt();
        if (kind == EventKind::WorkMeeting) res = std::make_unique<WorkMeeting>(title, start, dur, room, imp);
        else if (kind == EventKind::Seminar) res = std::make_unique<Seminar>(title, start, dur, room, imp);
        else if (kind == EventKind::ManagersMeeting) res = std::make_unique<ManagersMeeting>(title, start, dur, room, imp);
        else res = std::make_unique<BusinessTrip>(title, start, dur, imp);
    }

    res->setTitle(title);
    res->setStart(start);
    res->setDuration(dur);
    res->setRoom(room);
    res->setImportance(imp);
    res->setParticipants(parts);
    return res;
}
