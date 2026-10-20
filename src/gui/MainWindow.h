#pragma once
#include <QMainWindow>
#include "Simulation.h"

class QTimer;
class QListWidget;
class QTextEdit;
class QPlainTextEdit;
class QLabel;
class QComboBox;
class QDateEdit;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const sec::SimulationParams& params, QWidget* parent = nullptr);

private slots:
    void onTick();
    void onStart();
    void onStep();
    void onStop();
    void onAdd();
    void onEdit();
    void onDelete();
    void onFilterChanged();
    void onDayChanged();
    void onEventSelected();

private:
    void updateUi();
    void handleConflicts();

    sec::Simulation* sim;
    QTimer* timer;

    QLabel* lblTime;
    QListWidget* listEvents;
    QTextEdit* textDetails;
    QPlainTextEdit* textLog;

    QComboBox* comboDep;
    QComboBox* comboEmp;
    QDateEdit* dateEdit;
    QPushButton* btnEdit;
    QPushButton* btnDel;
    int lastLogSize;
};
