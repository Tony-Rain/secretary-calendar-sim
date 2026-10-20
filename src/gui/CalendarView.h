#pragma once
#include <QWidget>
#include <vector>
#include <utility>
#include "Simulation.h"

class CalendarView : public QWidget {
    Q_OBJECT
public:
    explicit CalendarView(QWidget* parent = nullptr);
    void setSimulation(sec::Simulation* s);
    void setWeekStart(int day);
    int weekStart() const { return weekStartDay; }
    void setFilter(const sec::EventFilter& f);
    void setColorMode(int mode);
    void setSelectedId(int id);
    int selectedId() const { return selected; }
signals:
    void eventSelected(int id);
    void editRequested(int id);
protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
private:
    sec::Simulation* sim = nullptr;
    int weekStartDay = 0;
    sec::EventFilter filter{nullptr, nullptr};
    int colorMode = 0;
    int selected = 0;
    std::vector<std::pair<int, QRect>> hitRects;
    QRect rectOf(int id) const;
};
