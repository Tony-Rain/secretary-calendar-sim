#pragma once
#include <QDialog>
#include "Simulation.h"

class QSpinBox;
class QDoubleSpinBox;
class QComboBox;

class ParamsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ParamsDialog(QWidget* parent = nullptr);
    sec::SimulationParams getParams() const;

private:
    QSpinBox* spinDeps;
    QSpinBox* spinStaff;
    QSpinBox* spinDays;
    QComboBox* comboStep;
    QDoubleSpinBox* spinRequests;
    QComboBox* comboMode;
    QComboBox* comboRole;
};
