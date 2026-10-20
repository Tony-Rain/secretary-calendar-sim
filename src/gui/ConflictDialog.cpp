#include "ConflictDialog.h"
#include "EventDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QMessageBox>

using namespace sec;

ConflictDialog::ConflictDialog(Simulation* s, QWidget* parent) : QDialog(parent), sim(s) {
    setWindowTitle("Конфликт!");
    QVBoxLayout* layout = new QVBoxLayout(this);

    const Event& ev = *sim->pending().event;
    layout->addWidget(new QLabel("<b>" + QString::fromStdString(ev.title()) + "</b>, " +
                                 QString::fromStdString(sim->schedule().clock.timeString(ev.start()))));

    for (const auto& c : sim->pending().conflicts) {
        layout->addWidget(new QLabel(QString::fromStdString("! " + c.text)));
    }

    layout->addWidget(new QLabel("Варианты решения:"));
    QList<QRadioButton*> radios;
    for (const auto& opt : sim->pending().options) {
        QString text = QString::fromStdString(opt.text);
        QRadioButton* rb = new QRadioButton(text);
        if (opt.adminOnly && !sim->isAdmin()) {
            rb->setText(text + "  (только админ)");
            rb->setEnabled(false);
        }
        layout->addWidget(rb);
        radios.append(rb);
    }
    if (radios.isEmpty()) layout->addWidget(new QLabel("Система не нашла вариантов"));

    QPushButton* btnApply = new QPushButton("Применить");
    QPushButton* btnEdit = new QPushButton("Изменить вручную...");
    QPushButton* btnReject = new QPushButton("Отклонить");
    layout->addWidget(btnApply);
    layout->addWidget(btnEdit);
    layout->addWidget(btnReject);

    connect(btnReject, &QPushButton::clicked, this, [this]() {
        sim->rejectPending();
        accept();
    });

    connect(btnApply, &QPushButton::clicked, this, [this, radios]() {
        for (int i = 0; i < radios.size(); ++i) {
            if (radios[i]->isChecked()) {
                sim->choosePending(i);
                accept();
                return;
            }
        }
        QMessageBox::information(this, "Конфликт", "Выберите вариант решения");
    });

    connect(btnEdit, &QPushButton::clicked, this, [this]() {
        EventDialog dlg(sim, this);
        dlg.loadEvent(sim->pending().event->clone());
        if (dlg.exec() == QDialog::Accepted) {
            sim->replacePending(dlg.getResult());
            accept();
        }
    });
}
