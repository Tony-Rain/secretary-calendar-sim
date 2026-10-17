#pragma once
#include <QDialog>
#include "Simulation.h"

class QSpinBox;
class QComboBox;

class ParamsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ParamsDialog(QWidget* parent = nullptr);
    sec::SimulationParams getParams() const;

private:
    QSpinBox* spinDeps;
    QSpinBox* spinDays;
    QComboBox* comboStep;
    QComboBox* comboRole;
};
