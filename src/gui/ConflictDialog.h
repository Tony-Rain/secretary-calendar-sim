#pragma once
#include <QDialog>
#include "Simulation.h"

class ConflictDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConflictDialog(sec::Simulation* sim, QWidget* parent = nullptr);
private:
    sec::Simulation* sim;
};
