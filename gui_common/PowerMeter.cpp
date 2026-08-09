/*
Copyright (c) 2026 Matthew H. Reilly (kb1vc)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PowerMeter.hpp"

#include <QPainter>
#include <QPen>
#include <QFont>
#include <QString>
#include <QPointF>
#include <QRectF>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GUISoDa {

PowerMeter::PowerMeter(QWidget* parent)
    : QWidget(parent, Qt::Tool)
    , target_db(MIN_DB - 2.0f)
    , display_db(MIN_DB - 2.0f)
    , peak_db(MIN_DB - 2.0f)
    , peak_hold_ticks(0)
{
    setWindowTitle("TX Power");
    setFixedSize(300, 180);

    connect(&timer, &QTimer::timeout, this, &PowerMeter::onTimer);
    timer.start(100); // 10 Hz
}

void PowerMeter::updatePower(double v) {
    if (!isVisible()) return;

    float db = (v > 1e-12) ? (float)(10.0 * std::log10(v)) : MIN_DB - 2.0f;
    target_db = db;
}

void PowerMeter::onTimer() {
    if (!isVisible()) return;

    float diff  = target_db - display_db;
    float alpha = (diff > 0.0f) ? ATTACK : DECAY_K;
    display_db += alpha * diff;

    if (display_db >= peak_db) {
        peak_db        = display_db;
        peak_hold_ticks = 0;
    } else if (peak_hold_ticks < PEAK_HOLD) {
        peak_hold_ticks++;
    } else {
        peak_db -= PEAK_DECAY;
    }

    update();
}

// MIN_DB maps to 150° (upper-left), MAX_DB maps to 30° (upper-right).
double PowerMeter::db_to_angle(float db) const {
    float f = (db - MIN_DB) / (MAX_DB - MIN_DB);
    f = std::clamp(f, -0.08f, 1.08f);
    return 150.0 - (double)f * 120.0;
}

void PowerMeter::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const double W = width();
    const double H = height();

    p.fillRect(rect(), QColor(22, 22, 22));

    const double pivot_x = W / 2.0;
    const double pivot_y = H - 28.0;
    const double R       = W * 0.38;

    auto pt = [&](double deg, double r) -> QPointF {
        double rad = deg * M_PI / 180.0;
        return QPointF(pivot_x + r * std::cos(rad),
                       pivot_y - r * std::sin(rad));
    };

    // ── Coloured arc (green up to -1 dB, red above) ──────────────────────
    QRect arc_rect((int)(pivot_x - R), (int)(pivot_y - R),
                   (int)(2.0 * R),     (int)(2.0 * R));
    const double angle_m1 = db_to_angle(-1.0f);

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(0, 160, 0), 5));
    p.drawArc(arc_rect, (int)(angle_m1 * 16),
              (int)((150.0 - angle_m1) * 16));
    p.setPen(QPen(QColor(200, 30, 0), 5));
    p.drawArc(arc_rect, (int)(30.0 * 16),
              (int)((angle_m1 - 30.0) * 16));

    struct Mark { float db; bool major; const char* lbl; };
    static const Mark marks[] = {
        {-10, true,  "-10"}, { -9, false, ""}, { -8, true,  "-8"},
        { -7, false,    ""}, { -6, true,  "-6"}, { -5, false, ""},
        { -4, true,   "-4"}, { -3, false,  ""}, { -2, true,  "-2"},
        { -1, true,   "-1"}, {  0, true,   "0"},
    };

    for (const auto& m : marks) {
        double a   = db_to_angle(m.db);
        double rin = m.major ? R - 11.0 : R - 6.0;

        p.setPen(QPen(QColor(190, 190, 190), m.major ? 2 : 1));
        p.drawLine(pt(a, rin), pt(a, R + 2.0));

        if (m.major) {
            QFont f = font();
            f.setPixelSize(13);
            p.setFont(f);
            p.setPen(QColor(215, 215, 215));
            QPointF lp = pt(a, R + 20.0);
            p.drawText(QRectF(lp.x() - 18, lp.y() - 9, 36, 18),
                       Qt::AlignCenter, m.lbl);
        }
    }

    if (peak_db > MIN_DB - 1.0f) {
        double pa = db_to_angle(peak_db);
        p.setPen(QPen(QColor(255, 210, 0), 3));
        p.drawLine(pt(pa, R - 14.0), pt(pa, R + 4.0));
    }

    double na   = db_to_angle(display_db);
    QPointF tip  = pt(na, R * 0.90);
    QPointF tail = pt(na + 180.0, R * 0.08);

    p.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(tail, tip);

    p.setBrush(QColor(180, 180, 180));
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(pivot_x, pivot_y), 5.0, 5.0);

    QFont rf = font();
    rf.setPixelSize(16);
    p.setFont(rf);
    p.setPen(Qt::white);
    QString txt = (display_db <= MIN_DB - 0.5f)
                  ? QString("< -10 dB")
                  : QString("%1 dB").arg((double)display_db, 6, 'f', 1);
    p.drawText(QRectF(0, H - 25.0, W, 22.0), Qt::AlignCenter, txt);
}

} // namespace GUISoDa
