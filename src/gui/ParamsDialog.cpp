#include "ParamsDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

using namespace sec;

ParamsDialog::ParamsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Настройки симуляции");

    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();

    spinDeps = new QSpinBox();
    spinDeps->setRange(5, 9);
    spinDeps->setValue(6);
    form->addRow("Количество отделов:", spinDeps);

    spinDays = new QSpinBox();
    spinDays->setRange(7, 30);
    spinDays->setValue(14);
    form->addRow("Дней симуляции:", spinDays);

    comboStep = new QComboBox();
    comboStep->addItem("30 минут", 30);
    comboStep->addItem("1 час", 60);
    form->addRow("Шаг времени:", comboStep);

    comboRole = new QComboBox();
    comboRole->addItem("Администратор", 0);
    comboRole->addItem("Пользователь", 1);
    form->addRow("Роль:", comboRole);

    layout->addLayout(form);

    QPushButton* btnStart = new QPushButton("Начать");
    layout->addWidget(btnStart);

    connect(btnStart, &QPushButton::clicked, this, &QDialog::accept);
}

SimulationParams ParamsDialog::getParams() const {
    SimulationParams p;
    p.departments = spinDeps->value();
    p.days = spinDays->value();
    p.step = comboStep->currentData().toInt();

    auto now = std::chrono::system_clock::now();
    p.startDate = std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>(now) };

    p.role = (comboRole->currentIndex() == 0) ? UserRole::Admin : UserRole::User;
    return p;
}
