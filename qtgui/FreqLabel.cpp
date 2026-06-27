/*
  Copyright (c) 2017, 2025 Matthew H. Reilly (kb1vc)
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

#include <cmath>
#include <QPainter>
#include <QPalette>
#include <QFontMetrics>

#include "FreqLabel.hpp"

namespace GUISoDa {

  // Slot 0 is the leftmost (10 GHz) digit; slot 10 is the rightmost (1 Hz).
  // Comma between slot 1 and slot 2; decimal point between slot 4 and slot 5;
  // space between slot 7 and slot 8.
  static const long long kSlotWeights[11] = {
    10000000000LL, // 10 GHz
     1000000000LL, //  1 GHz
      100000000LL, // 100 MHz
       10000000LL, //  10 MHz
        1000000LL, //   1 MHz
         100000LL, // 100 kHz
          10000LL, //  10 kHz
           1000LL, //   1 kHz
            100LL, // 100 Hz
             10LL, //  10 Hz
              1LL  //   1 Hz
  };

  FreqLabel::FreqLabel(QWidget * parent, Qt::WindowFlags f)
    : QLabel(parent), frequency(0.0), hover_index(-1)
  {
    (void) f;
    setMouseTracking(true);
    // We render the digits ourselves; the .ui-supplied text is ignored.
    QLabel::setText(QString());
    setFreq(144.295e6);
  }

  FreqLabel::~FreqLabel() {}

  void FreqLabel::clampFrequency()
  {
    if (frequency < 0.0) frequency = 0.0;
    // 99,999.999 999 MHz upper bound (matches original 5-digit integer field)
    const double max_hz = 99999999999.0;
    if (frequency > max_hz) frequency = max_hz;
  }

  void FreqLabel::setFreq(double hzfreq)
  {
    frequency = hzfreq;
    clampFrequency();
    rebuildLayout();
    update();
  }

  void FreqLabel::setFreqUpdate(double hzfreq)
  {
    setFreq(hzfreq);
    emit newFreq(frequency);
  }

  void FreqLabel::rebuildLayout()
  {
    digit_rects.clear();
    if (width() <= 0 || height() <= 0) return;

    QFontMetrics fm(font());
    int digit_w = fm.horizontalAdvance(QChar('0'));
    int comma_w = fm.horizontalAdvance(QChar(','));
    int dot_w   = fm.horizontalAdvance(QChar('.'));
    int space_w = fm.horizontalAdvance(QChar(' '));
    if (digit_w <= 0) digit_w = 1;

    int total_w = 11 * digit_w + comma_w + dot_w + space_w;
    int x = (width() - total_w) / 2;
    int y = 0;
    int h = height();

    long long ifq = (long long) llround(frequency);

    auto digit_at = [&](long long w) -> int {
      return (int)((ifq / w) % 10);
    };

    // Slots 0..1 (10 GHz, 1 GHz)
    for (int i = 0; i < 2; i++) {
      digit_rects.push_back({QRect(x, y, digit_w, h), kSlotWeights[i], digit_at(kSlotWeights[i])});
      x += digit_w;
    }
    comma_rect = QRect(x, y, comma_w, h);
    x += comma_w;
    // Slots 2..4 (100 MHz, 10 MHz, 1 MHz)
    for (int i = 2; i < 5; i++) {
      digit_rects.push_back({QRect(x, y, digit_w, h), kSlotWeights[i], digit_at(kSlotWeights[i])});
      x += digit_w;
    }
    dot_rect = QRect(x, y, dot_w, h);
    x += dot_w;
    // Slots 5..7 (100 kHz, 10 kHz, 1 kHz)
    for (int i = 5; i < 8; i++) {
      digit_rects.push_back({QRect(x, y, digit_w, h), kSlotWeights[i], digit_at(kSlotWeights[i])});
      x += digit_w;
    }
    space_rect = QRect(x, y, space_w, h);
    x += space_w;
    // Slots 8..10 (100 Hz, 10 Hz, 1 Hz)
    for (int i = 8; i < 11; i++) {
      digit_rects.push_back({QRect(x, y, digit_w, h), kSlotWeights[i], digit_at(kSlotWeights[i])});
      x += digit_w;
    }
  }

  int FreqLabel::hitDigit(const QPoint & p) const
  {
    for (size_t i = 0; i < digit_rects.size(); i++) {
      if (digit_rects[i].rect.contains(p)) return (int)i;
    }
    return -1;
  }

  void FreqLabel::applyDelta(long long delta_hz)
  {
    frequency += (double) delta_hz;
    clampFrequency();
    rebuildLayout();
    update();
    emit newFreq(frequency);
  }

  void FreqLabel::paintEvent(QPaintEvent * event)
  {
    (void) event;
    if (digit_rects.empty()) rebuildLayout();
    if (digit_rects.empty()) return;

    QPainter painter(this);
    painter.setFont(font());

    QColor base_color = palette().color(foregroundRole());
    QColor dim_color = base_color;
    dim_color.setAlpha(80);
    QColor highlight = palette().color(QPalette::Highlight);

    // Leading zeros in the integer field (slots 0..4) are dimmed up to but
    // not including the most-significant nonzero digit; if the whole integer
    // part is zero, only the 1-MHz digit is shown bright.
    int first_significant = 4;
    for (int i = 0; i < 5; i++) {
      if (digit_rects[i].digit_value != 0) { first_significant = i; break; }
    }

    // Draw separators
    painter.setPen((first_significant <= 1) ? base_color : dim_color);
    painter.drawText(comma_rect, Qt::AlignCenter, QStringLiteral(","));
    painter.setPen(base_color);
    painter.drawText(dot_rect, Qt::AlignCenter, QStringLiteral("."));
    // (no glyph drawn for the kHz/Hz space)

    // Draw digits
    for (int i = 0; i < (int)digit_rects.size(); i++) {
      QColor c = base_color;
      if (i < first_significant) c = dim_color;
      if (i == hover_index)      c = highlight;
      painter.setPen(c);
      painter.drawText(digit_rects[i].rect, Qt::AlignCenter,
                       QString::number(digit_rects[i].digit_value));
    }

    // Hover affordance: a thin colored bar above (inc) or below (dec)
    // the digit under the cursor, signalling what a click will do.
    if (hover_index >= 0 && hover_index < (int)digit_rects.size()) {
      const QRect & r = digit_rects[hover_index].rect;
      bool top = last_mouse_pos.y() < r.center().y();
      int bar_h = qMax(2, r.height() / 14);
      QRect bar = top
        ? QRect(r.left(), r.top(), r.width(), bar_h)
        : QRect(r.left(), r.bottom() - bar_h + 1, r.width(), bar_h);
      painter.fillRect(bar, highlight);
    }
  }

  void FreqLabel::mousePressEvent(QMouseEvent * event)
  {
    if (event->button() != Qt::LeftButton) {
      event->ignore();
      return;
    }
    int idx = hitDigit(event->pos());
    if (idx < 0) {
      event->ignore();
      return;
    }
    bool top = event->pos().y() < digit_rects[idx].rect.center().y();
    long long delta = digit_rects[idx].weight_hz * (top ? 1 : -1);
    applyDelta(delta);
  }

  void FreqLabel::mouseMoveEvent(QMouseEvent * event)
  {
    last_mouse_pos = event->pos();
    hover_index = hitDigit(last_mouse_pos);
    update();
  }

  void FreqLabel::wheelEvent(QWheelEvent * event)
  {
    QPoint pos = event->position().toPoint();
    int idx = hitDigit(pos);
    if (idx < 0) {
      event->ignore();
      return;
    }
    int notches = event->angleDelta().y() / 120;
    if (notches == 0) {
      event->ignore();
      return;
    }
    applyDelta(digit_rects[idx].weight_hz * notches);
    event->accept();
  }

  void FreqLabel::leaveEvent(QEvent * event)
  {
    (void) event;
    hover_index = -1;
    update();
  }

  void FreqLabel::resizeEvent(QResizeEvent * event)
  {
    QLabel::resizeEvent(event);
    rebuildLayout();
  }

  QSize FreqLabel::sizeHint() const
  {
    QFontMetrics fm(font());
    int digit_w = fm.horizontalAdvance(QChar('0'));
    int comma_w = fm.horizontalAdvance(QChar(','));
    int dot_w   = fm.horizontalAdvance(QChar('.'));
    int space_w = fm.horizontalAdvance(QChar(' '));
    int total_w = 11 * digit_w + comma_w + dot_w + space_w;
    int h = fm.height();
    return QSize(total_w + 8, h + 4);
  }

  QSize FreqLabel::minimumSizeHint() const
  {
    return sizeHint();
  }
}
