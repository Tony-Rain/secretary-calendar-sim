#pragma once
#include <QString>
#include <QColor>
#include <QDate>
#include <string>
#include <chrono>

inline QString qs(const std::string& s) {
    return QString::fromStdString(s);
}

inline QDate toQDate(const std::chrono::year_month_day& d) {
    return QDate(int(d.year()), unsigned(d.month()), unsigned(d.day()));
}

inline QColor departmentColor(int id) {
    static const QColor palette[9] = {
        QColor(66,133,244), QColor(219,68,55), QColor(244,180,0),
        QColor(15,157,88), QColor(171,71,188), QColor(255,112,67),
        QColor(0,172,193), QColor(124,179,66), QColor(109,76,65)
    };
    if (id < 0 || id > 8) return QColor(150,150,150);
    return palette[id];
}

inline QColor importanceColor(int imp) {
    if (imp == 0) return QColor(200,235,200);
    if (imp == 1) return QColor(250,230,150);
    return QColor(250,170,170);
}
