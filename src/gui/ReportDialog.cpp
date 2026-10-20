#include "ReportDialog.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <algorithm>

using namespace sec;

static bool byStart(Event* a, Event* b) { return a->start() < b->start(); }

ReportDialog::ReportDialog(Simulation* s, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Итоговый отчет");
    resize(650, 550);
    QVBoxLayout* layout = new QVBoxLayout(this);

    const Statistics& st = s->stats();
    const Clock& clock = s->schedule().clock;

    int workDays = 0;
    for (int d = 0; d < s->params().days; ++d)
        if (s->schedule().isWorkingDay(d)) workDays++;
    int hallTotal = workDays * s->schedule().workdayLength();
    int hallPercent = st.hallBusy * 100 / hallTotal;

    QString report = "ИТОГИ МОДЕЛИРОВАНИЯ\n";
    report += "Период: " + QString::number(s->params().days) + " дн., отделов: " +
              QString::number(s->params().departments) + "\n\n";
    report += "Состоялось событий: " + QString::number(st.held) + "\n";
    report += "Поступило заявок: " + QString::number(st.requests) + "\n";
    report += "Заявок с конфликтами: " + QString::number(st.conflicts) + "\n";
    report += "Отклонено заявок: " + QString::number(st.rejected) + "\n";
    report += "Отменено событий: " + QString::number(st.cancelled) + "\n";
    report += "Отправлено напоминаний: " + QString::number(st.reminders) + "\n";
    report += "Загрузка конференц-зала: " + QString::number(hallPercent) + "%\n";

    report += "\nСОСТОЯВШИЕСЯ СОБЫТИЯ\n";
    std::vector<Event*> held;
    for (const auto& e : s->history()) held.push_back(e.get());
    std::sort(held.begin(), held.end(), byStart);

    int lastDay = -1;
    for (Event* e : held) {
        int day = dayOf(e->start());
        if (day != lastDay) {
            report += "\n" + QString::fromStdString(clock.dayString(day)) + "\n";
            lastDay = day;
        }
        report += "  " + QString::fromStdString(Clock::hhmm(e->start())) + "  " +
                  QString::fromStdString(e->title()) + " (участников: " +
                  QString::number(e->participants().size()) + ")\n";
    }

    QTextEdit* text = new QTextEdit();
    text->setReadOnly(true);
    text->setPlainText(report);
    layout->addWidget(text);

    QPushButton* btnSave = new QPushButton("Сохранить");
    QPushButton* btnClose = new QPushButton("Закрыть");
    layout->addWidget(btnSave);
    layout->addWidget(btnClose);

    connect(btnSave, &QPushButton::clicked, this, [report]() {
        QFileDialog::saveFileContent(report.toUtf8(), "report.txt");
    });
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
}
