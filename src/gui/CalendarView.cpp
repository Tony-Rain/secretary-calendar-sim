#include "CalendarView.h"
#include "Common.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>

using namespace sec;

CalendarView::CalendarView(QWidget* parent) : QWidget(parent) {
    setMinimumSize(700, 500);
}

void CalendarView::setSimulation(Simulation* s) {
    sim = s;
    if (sim) {
        int slotCount = (sim->schedule().workEnd - sim->schedule().workStart) / 30;
        setMinimumHeight(30 + slotCount * 22 + 10);
    }
    update();
}

void CalendarView::setWeekStart(int day) { weekStartDay = day; update(); }
void CalendarView::setFilter(const EventFilter& f) { filter = f; update(); }
void CalendarView::setColorMode(int mode) { colorMode = mode; update(); }

void CalendarView::setSelectedId(int id) {
    selected = id;
    update();
}

QRect CalendarView::rectOf(int id) const {
    for (size_t i = 0; i < hitRects.size(); ++i)
        if (hitRects[i].first == id) return hitRects[i].second;
    return QRect();
}

void CalendarView::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), Qt::white);
    hitRects.clear();
    if (!sim) return;

    const WorkSchedule& sch = sim->schedule();
    int wStart = sch.workStart;
    int wEnd = sch.workEnd;
    int slotCount = (wEnd - wStart) / 30;
    int leftM = 60;
    int headH = 30;
    int rowH = 22;
    int colW = (width() - leftM) / 7;
    if (colW < 1) return;

    for (int d = 0; d < 7; ++d) {
        int day = weekStartDay + d;
        QString label = "";
        if (day >= 0 && day < sch.days) label = qs(sch.clock.dayString(day));
        QRect hr(leftM + d * colW, 0, colW, headH);
        p.fillRect(hr, QColor(230, 230, 230));
        p.drawRect(hr);
        p.drawText(hr, Qt::AlignCenter, label);
    }

    for (int s = 0; s < slotCount; ++s) {
        int y = headH + s * rowH;
        if (s % 2 == 0) {
            int mins = wStart + s * 30;
            p.drawText(QRect(0, y, leftM - 6, rowH * 2),
                       Qt::AlignRight | Qt::AlignTop,
                       qs(Clock::hhmm(mins)));
        }
    }

    for (int d = 0; d < 7; ++d) {
        int day = weekStartDay + d;
        bool shade = (day < 0 || day >= sch.days) || sch.clock.isWeekend(day);
        if (shade)
            p.fillRect(QRect(leftM + d * colW, headH, colW, slotCount * rowH),
                       QColor(220, 220, 220, 130));
    }

    p.setPen(QColor(180, 180, 180));
    for (int d = 0; d <= 7; ++d) {
        int x = leftM + d * colW;
        p.drawLine(x, headH, x, headH + slotCount * rowH);
    }
    for (int s = 0; s <= slotCount; ++s) {
        int y = headH + s * rowH;
        p.drawLine(leftM, y, width(), y);
    }
    p.setPen(Qt::black);

    Minutes from = dayStart(weekStartDay);
    Minutes to = dayStart(weekStartDay + 7);
    std::vector<Event*> events = sim->calendar().query(from, to, filter);

    for (int day = weekStartDay; day < weekStartDay + 7; ++day) {
        if (day < 0 || day >= sch.days) continue;
        struct DE { Event* ev; int r1; int r2; int lane; };
        std::vector<DE> des;
        for (size_t i = 0; i < events.size(); ++i) {
            Event* ev = events[i];
            int sd = dayOf(ev->start());
            int ed = dayOf(ev->end() - 1);
            if (day < sd || day > ed) continue;
            Minutes ds = dayStart(day);
            Minutes sT = std::max(ev->start(), ds + wStart);
            Minutes eT = std::min(ev->end(), ds + wEnd);
            if (sT >= eT) continue;
            DE de;
            de.ev = ev;
            de.r1 = (sT - ds - wStart) / 30;
            de.r2 = (eT - ds - wStart + 29) / 30;
            de.lane = 0;
            des.push_back(de);
        }
        int numLanes = 0;
        for (size_t i = 0; i < des.size(); ++i) {
            int lane = 0;
            while (true) {
                bool ok = true;
                for (size_t j = 0; j < i; ++j) {
                    if (des[j].lane == lane &&
                        des[i].r1 < des[j].r2 && des[i].r2 > des[j].r1) {
                        ok = false;
                        break;
                    }
                }
                if (ok) break;
                ++lane;
            }
            des[i].lane = lane;
            if (lane + 1 > numLanes) numLanes = lane + 1;
        }
        if (numLanes < 1) numLanes = 1;
        int col = day - weekStartDay;
        for (size_t i = 0; i < des.size(); ++i) {
            DE& de = des[i];
            int laneW = colW / numLanes;
            int x = leftM + col * colW + de.lane * laneW + 1;
            int y = headH + de.r1 * rowH + 1;
            int w = laneW - 2;
            int h = (de.r2 - de.r1) * rowH - 2;
            QRect r(x, y, w, h);
            QColor c;
            if (colorMode == 0) {
                const Department* dep = de.ev->ownerDepartment();
                c = dep ? departmentColor(dep->id()) : QColor(150, 150, 150);
            } else {
                c = importanceColor(int(de.ev->importance()));
            }
            p.fillRect(r, c);
            p.drawRect(r);
            if (de.ev->id() == selected) {
                QPen old = p.pen();
                p.setPen(QPen(Qt::black, 2));
                p.drawRect(r);
                p.setPen(old);
            }
            QString text = qs(de.ev->title());
            if (de.ev->kind() == EventKind::BusinessTrip)
                text = QString::fromUtf8("✈ ") + text;
            p.drawText(r.adjusted(3, 2, -3, -2), Qt::TextWordWrap, text);
            hitRects.push_back(std::make_pair(de.ev->id(), r));
        }
    }

    int curDay = dayOf(sim->now());
    if (curDay >= weekStartDay && curDay < weekStartDay + 7) {
        int c = curDay - weekStartDay;
        int mins = sim->now() - dayStart(curDay);
        if (mins >= wStart && mins <= wEnd) {
            int y = headH + ((mins - wStart) / 30) * rowH;
            p.setPen(QPen(Qt::red, 2));
            p.drawLine(leftM + c * colW, y, leftM + (c + 1) * colW, y);
            p.setPen(Qt::black);
        }
    }
}

void CalendarView::mousePressEvent(QMouseEvent* e) {
    for (size_t i = 0; i < hitRects.size(); ++i) {
        if (hitRects[i].second.contains(e->pos())) {
            selected = hitRects[i].first;
            emit eventSelected(selected);
            update();
            return;
        }
    }
    selected = 0;
    emit eventSelected(0);
    update();
}

void CalendarView::mouseDoubleClickEvent(QMouseEvent* e) {
    for (size_t i = 0; i < hitRects.size(); ++i) {
        if (hitRects[i].second.contains(e->pos())) {
            selected = hitRects[i].first;
            emit editRequested(selected);
            return;
        }
    }
}
