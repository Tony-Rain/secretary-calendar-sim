#include "ConflictDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>

using namespace sec;

ConflictDialog::ConflictDialog(Simulation* s, QWidget* parent) : QDialog(parent), sim(s) {
    setWindowTitle("Конфликт!");
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(QString::fromStdString(sim->pending().event->title())));

    for (const auto& c : sim->pending().conflicts) {
        layout->addWidget(new QLabel(QString::fromStdString("! " + c.text)));
    }

    QList<QRadioButton*> radios;
    for (const auto& opt : sim->pending().options) {
        QRadioButton* rb = new QRadioButton(QString::fromStdString(opt.text));
        if (opt.adminOnly && !sim->isAdmin()) rb->setEnabled(false);
        layout->addWidget(rb);
        radios.append(rb);
    }

    QPushButton* btnApply = new QPushButton("Применить");
    QPushButton* btnReject = new QPushButton("Отклонить");
    layout->addWidget(btnApply);
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
    });
}
