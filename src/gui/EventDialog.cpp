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
    comboType->addItems({"Рабочая встреча", "Семинар", "Совещание", "Командировка"});
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
    spinDur->setValue(60);
    form->addRow("Длительность (мин):", spinDur);

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
        auto item = new QListWidgetItem(QString::fromStdString(e->name()));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        listParts->addItem(item);
    }
    layout->addWidget(listParts);

    QPushButton* btnOk = new QPushButton("OK");
    layout->addWidget(btnOk);
    connect(btnOk, &QPushButton::clicked, this, &EventDialog::onOkClicked);
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

    accept();
}

void EventDialog::loadEvent(std::unique_ptr<Event> e) {
    original = std::move(e);
    editTitle->setText(QString::fromStdString(original->title()));
    comboType->setCurrentIndex((int)original->kind());

    QDate simStart(int(sim->params().startDate.year()), unsigned(sim->params().startDate.month()), unsigned(sim->params().startDate.day()));
    int day = dayOf(original->start());
    dateEdit->setDate(simStart.addDays(day));

    timeEdit->setTime(QTime(original->start() / 60 % 24, original->start() % 60));
    spinDur->setValue(original->duration());
    comboImp->setCurrentIndex((int)original->importance());
}

std::unique_ptr<Event> EventDialog::getResult() {
    std::string title = editTitle->text().toStdString();

    QDate simStart(int(sim->params().startDate.year()), unsigned(sim->params().startDate.month()), unsigned(sim->params().startDate.day()));
    int day = simStart.daysTo(dateEdit->date());
    if (day < 0) day = 0;

    Minutes start = dayStart(day) + timeEdit->time().hour() * 60 + timeEdit->time().minute();
    Minutes dur = spinDur->value();
    Importance imp = (Importance)comboImp->currentIndex();

    Room* room = nullptr;
    if (comboRoom->count() > 0 && comboRoom->currentIndex() >= 0) {
        room = sim->company().rooms()[comboRoom->currentIndex()].get();
    }

    std::vector<Employee*> parts;
    for (int i = 0; i < listParts->count(); ++i) {
        if (listParts->item(i)->checkState() == Qt::Checked) {
            parts.push_back(sim->company().allEmployees()[i]);
        }
    }

    std::unique_ptr<Event> res;
    int type = comboType->currentIndex();
    if (type == 0) res = std::make_unique<WorkMeeting>(title, start, dur, room, imp);
    else if (type == 1) res = std::make_unique<Seminar>(title, start, dur, room, imp);
    else if (type == 2) res = std::make_unique<ManagersMeeting>(title, start, dur, room, imp);
    else res = std::make_unique<BusinessTrip>(title, start, dur, imp);

    res->setParticipants(parts);
    return res;
}
