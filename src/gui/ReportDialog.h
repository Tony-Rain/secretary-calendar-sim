#pragma once
#include <QDialog>
#include "Simulation.h"

class ReportDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReportDialog(sec::Simulation* sim, QWidget* parent = nullptr);
};
