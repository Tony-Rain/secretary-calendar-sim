#pragma once
#include <QDialog>
#include <memory>
#include "Simulation.h"

class QLineEdit;
class QComboBox;
class QDateEdit;
class QTimeEdit;
class QSpinBox;
class QListWidget;

class EventDialog : public QDialog {
    Q_OBJECT
public:
    explicit EventDialog(sec::Simulation* sim, QWidget* parent = nullptr);
    void loadEvent(std::unique_ptr<sec::Event> e);
    std::unique_ptr<sec::Event> getResult();

private slots:
    void onOkClicked();
    void onTypeChanged();

private:
    bool isTrip() const;
    sec::Minutes startTime() const;

    sec::Simulation* sim;
    std::unique_ptr<sec::Event> original;

    QComboBox* comboType;
    QLineEdit* editTitle;
    QDateEdit* dateEdit;
    QTimeEdit* timeEdit;
    QSpinBox* spinDur;
    QSpinBox* spinDays;
    QComboBox* comboRoom;
    QComboBox* comboImp;
    QListWidget* listParts;
};
