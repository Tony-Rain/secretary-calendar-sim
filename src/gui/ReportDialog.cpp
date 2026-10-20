#include "ReportDialog.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFileDialog>

using namespace sec;

ReportDialog::ReportDialog(Simulation* s, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Отчет");
    resize(500, 400);
    QVBoxLayout* layout = new QVBoxLayout(this);

    QTextEdit* text = new QTextEdit();
    text->setReadOnly(true);

    QString report = "Всего проведено: " + QString::number(s->stats().held) + "\n";
    report += "Конфликтов: " + QString::number(s->stats().conflicts) + "\n";
    report += "Отклонено: " + QString::number(s->stats().rejected) + "\n";

    text->setText(report);
    layout->addWidget(text);

    QPushButton* btnSave = new QPushButton("Сохранить");
    layout->addWidget(btnSave);

    connect(btnSave, &QPushButton::clicked, this, [report]() {
        QFileDialog::saveFileContent(report.toUtf8(), "report.txt");
    });
}
