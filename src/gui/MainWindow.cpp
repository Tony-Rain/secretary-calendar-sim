#include "MainWindow.h"
#include "ParamsDialog.h"
#include "EventDialog.h"
#include "ConflictDialog.h"
#include "ReportDialog.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QMessageBox>
#include <QDate>
#include <QMenuBar>

using namespace sec;

MainWindow::MainWindow(const SimulationParams& params, QWidget* parent) : QMainWindow(parent) {
    sim = nullptr;
    lastLogSize = 0;
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onTick);

    QWidget* central = new QWidget();
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    QHBoxLayout* topBar = new QHBoxLayout();
    btnStart = new QPushButton("Старт");
    btnStep = new QPushButton("Шаг");
    btnStop = new QPushButton("Стоп");
    lblState = new QLabel();
    lblState->setMinimumWidth(130);
    lblTime = new QLabel("Время: ");

    topBar->addWidget(btnStart);
    topBar->addWidget(btnStep);
    topBar->addWidget(btnStop);
    topBar->addWidget(lblState);
    topBar->addWidget(lblTime);
    topBar->addStretch();
    mainLayout->addLayout(topBar);

    QHBoxLayout* filterBar = new QHBoxLayout();
    dateEdit = new QDateEdit(QDate::currentDate());
    comboDep = new QComboBox();
    comboEmp = new QComboBox();
    // список выпадает вниз, а не поверх поля (иначе закрывается сразу после клика)
    comboDep->setStyleSheet("combobox-popup: 0;");
    comboEmp->setStyleSheet("combobox-popup: 0;");
    comboEmp->setMaxVisibleItems(15);
    filterBar->addWidget(new QLabel("Дата:"));
    filterBar->addWidget(dateEdit);
    filterBar->addWidget(new QLabel("Отдел:"));
    filterBar->addWidget(comboDep);
    filterBar->addWidget(new QLabel("Сотрудник:"));
    filterBar->addWidget(comboEmp);
    mainLayout->addLayout(filterBar);

    QHBoxLayout* centerLayout = new QHBoxLayout();
    listEvents = new QListWidget();
    textDetails = new QTextEdit();
    textDetails->setReadOnly(true);
    centerLayout->addWidget(listEvents, 1);
    centerLayout->addWidget(textDetails, 1);
    mainLayout->addLayout(centerLayout);

    QHBoxLayout* actionBar = new QHBoxLayout();
    QPushButton* btnAdd = new QPushButton("Добавить");
    btnEdit = new QPushButton("Изменить");
    btnDel = new QPushButton("Удалить");
    actionBar->addWidget(btnAdd);
    actionBar->addWidget(btnEdit);
    actionBar->addWidget(btnDel);
    mainLayout->addLayout(actionBar);

    textLog = new QPlainTextEdit();
    textLog->setReadOnly(true);
    textLog->setMaximumHeight(120);
    mainLayout->addWidget(new QLabel("Журнал:"));
    mainLayout->addWidget(textLog);

    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(btnStep, &QPushButton::clicked, this, &MainWindow::onStep);
    connect(btnStop, &QPushButton::clicked, this, &MainWindow::onStop);
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEdit);
    connect(btnDel, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(listEvents, &QListWidget::itemClicked, this, &MainWindow::onEventSelected);

    connect(comboDep, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    connect(comboEmp, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterChanged);
    connect(dateEdit, static_cast<void(QDateEdit::*)(const QDate&)>(&QDateEdit::dateChanged), this, &MainWindow::onDayChanged);

    QMenu* menu = menuBar()->addMenu("Эксперимент");
    menu->addAction("Новый эксперимент...", this, &MainWindow::onNewExperiment);
    menu->addAction("Итоговый отчет", this, &MainWindow::onReport);
    menu->addSeparator();
    menu->addAction("Выход", this, &QWidget::close);

    startSimulation(params);
}

void MainWindow::startSimulation(const SimulationParams& params) {
    timer->stop();
    delete sim;
    sim = new Simulation(params);
    lastLogSize = 0;
    textLog->clear();
    textDetails->clear();

    // пока заполняем списки, фильтр не пересчитываем
    comboDep->blockSignals(true);
    comboEmp->blockSignals(true);
    dateEdit->blockSignals(true);

    comboDep->clear();
    comboDep->addItem("Все отделы", -1);
    for (int i = 0; i < sim->params().departments; ++i) {
        comboDep->addItem(QString::fromStdString(sim->company().departments()[i]->name()), i);
    }
    comboEmp->clear();
    comboEmp->addItem("Все сотрудники", -1);
    for (auto& emp : sim->company().allEmployees()) {
        comboEmp->addItem(QString::fromStdString(emp->name()), emp->id());
    }
    QDate simStart(int(sim->params().startDate.year()), unsigned(sim->params().startDate.month()), unsigned(sim->params().startDate.day()));
    dateEdit->setMinimumDate(simStart);
    dateEdit->setMaximumDate(simStart.addDays(sim->params().days - 1));
    dateEdit->setDate(simStart);

    comboDep->blockSignals(false);
    comboEmp->blockSignals(false);
    dateEdit->blockSignals(false);

    updateUi();
    setRunning(false);
}

void MainWindow::onNewExperiment() {
    setRunning(false);
    ParamsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) startSimulation(dlg.getParams());
}

void MainWindow::onReport() {
    setRunning(false);
    ReportDialog* dlg = new ReportDialog(sim, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open();
}

void MainWindow::setRunning(bool running) {
    if (running && !sim->finished())
        timer->start(500);
    else
        timer->stop();

    if (sim->finished()) {
        lblState->setText("■ Завершено");
        lblState->setStyleSheet("font-weight: bold; color: gray;");
    } else if (timer->isActive()) {
        lblState->setText("▶ Идёт");
        lblState->setStyleSheet("font-weight: bold; color: green;");
    } else {
        lblState->setText("⏸ Пауза");
        lblState->setStyleSheet("font-weight: bold; color: darkorange;");
    }
    btnStart->setEnabled(!timer->isActive() && !sim->finished());
    btnStep->setEnabled(!sim->finished());
    btnStop->setEnabled(timer->isActive());
}

void MainWindow::onStart() { setRunning(true); }
void MainWindow::onStop() { setRunning(false); }

void MainWindow::onTick() {
    sim->step();
    updateUi();
    if (sim->hasPending()) {
        setRunning(false);
        handleConflicts();
    }
    if (sim->finished()) onReport();
}

void MainWindow::onStep() {
    setRunning(false);
    sim->step();
    updateUi();
    setRunning(false);
    if (sim->hasPending()) handleConflicts();
}

void MainWindow::handleConflicts() {
    if (sim->hasPending()) {
        ConflictDialog* dlg = new ConflictDialog(sim, this);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        connect(dlg, &QDialog::finished, this, [this]() {
            updateUi();
            if (sim->hasPending()) handleConflicts();
        });
        dlg->open();
    } else {
        updateUi();
    }
}

void MainWindow::updateUi() {
    lblTime->setText("Время: " + QString::fromStdString(sim->schedule().clock.timeString(sim->now())));

    EventFilter filter;
    filter.department = nullptr;
    filter.employee = nullptr;

    int depIdx = comboDep->currentData().toInt();
    if (depIdx >= 0) filter.department = sim->company().departments()[depIdx].get();

    int empIdx = comboEmp->currentData().toInt();
    if (empIdx >= 0) {
        for (auto& e : sim->company().allEmployees()) {
            if (e->id() == empIdx) filter.employee = e;
        }
    }
    QDate simStart(int(sim->params().startDate.year()), unsigned(sim->params().startDate.month()), unsigned(sim->params().startDate.day()));
    int d = simStart.daysTo(dateEdit->date());
    if (d < 0) d = 0;

    Minutes from = dayStart(d);
    Minutes to = dayStart(d + 1);

    listEvents->clear();
    if (d < sim->params().days) {
        for (Event* e : sim->calendar().query(from, to, filter)) {
            QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(sim->schedule().clock.hhmm(e->start()) + " - " + e->title()));
            item->setData(Qt::UserRole, e->id());
            listEvents->addItem(item);
        }
    }

    for (int i = lastLogSize; i < (int)sim->log().size(); ++i) {
        textLog->appendPlainText(QString::fromStdString(sim->log()[i].text));
    }
    lastLogSize = (int)sim->log().size();

    bool isAdmin = sim->isAdmin();
    btnEdit->setEnabled(isAdmin && listEvents->currentItem() != nullptr);
    btnDel->setEnabled(isAdmin && listEvents->currentItem() != nullptr);
}

void MainWindow::onDayChanged() { updateUi(); }
void MainWindow::onFilterChanged() { updateUi(); }

void MainWindow::onEventSelected() {
    QListWidgetItem* item = listEvents->currentItem();
    bool isAdmin = sim->isAdmin();
    btnEdit->setEnabled(isAdmin && item != nullptr);
    btnDel->setEnabled(isAdmin && item != nullptr);
    if (!item) {
        textDetails->clear();
        return;
    }
    int id = item->data(Qt::UserRole).toInt();
    Event* e = sim->calendar().find(id);
    if (!e) return;

    const Clock& clock = sim->schedule().clock;
    QString text = "Название: " + QString::fromStdString(e->title()) + "\n";
    text += "Тип: " + QString(eventKindName(e->kind())) + "\n";
    text += "Начало: " + QString::fromStdString(clock.timeString(e->start())) + "\n";
    text += "Конец: " + QString::fromStdString(clock.timeString(e->end())) + "\n";
    text += "Место: " + (e->room() ? QString::fromStdString(e->room()->name()) : QString("вне фирмы")) + "\n";
    text += "Важность: " + QString(importanceName(e->importance())) + "\n";
    text += "Напоминаний отправлено: " + QString::number(e->remindersSent()) + "\n";
    text += "\nУчастники (" + QString::number(e->participants().size()) + "):\n";
    for (Employee* p : e->participants()) {
        text += "  " + QString::fromStdString(p->name() + ", " + p->department()->name()) + "\n";
    }
    textDetails->setText(text);
}

void MainWindow::onAdd() {
    setRunning(false);
    EventDialog* dlg = new EventDialog(sim, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &QDialog::accepted, this, [this, dlg]() {
        auto e = dlg->getResult();
        if (e) {
            sim->submit(std::move(e));
            updateUi();
            if (sim->hasPending()) handleConflicts();
        }
    });
    dlg->open();
}


void MainWindow::onEdit() {
    QListWidgetItem* item = listEvents->currentItem();
    if (!item) return;
    setRunning(false);
    int id = item->data(Qt::UserRole).toInt();
    Event* e = sim->calendar().find(id);

    EventDialog* dlg = new EventDialog(sim, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->loadEvent(e->clone());
    connect(dlg, &QDialog::accepted, this, [this, dlg, id]() {
        auto e = dlg->getResult();
        if (e) {
            sim->submit(std::move(e), id);
            updateUi();
            if (sim->hasPending()) handleConflicts();
        }
    });
    dlg->open();
}

void MainWindow::onDelete() {
    QListWidgetItem* item = listEvents->currentItem();
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    sim->cancelEvent(id);
    updateUi();
}
