#pragma once
/*
  Copyright (c) 2017,2025 Matthew H. Reilly (kb1vc)
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

#include <QLabel>
#include <QWidget>
#include <Qt>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPoint>
#include <QRect>
#include <vector>

namespace GUISoDa {

  class FreqLabel : public QLabel {
    Q_OBJECT

  public:
    explicit FreqLabel(QWidget * parent = Q_NULLPTR,
                       Qt::WindowFlags f = Qt::WindowFlags());
    ~FreqLabel();

    double getFreq() const { return frequency; }

    void setFreqUpdate(double freq);

  signals:
    void newFreq(double freq);

  public slots:
    void setFreq(double freq);

  protected:
    void paintEvent(QPaintEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent * event) override;
    void leaveEvent(QEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  private:
    double frequency;        // in Hz

    struct DigitSlot {
      QRect rect;            // hit rect in widget coords
      long long weight_hz;   // power-of-10 weight in Hz (1, 10, 100, ..., 10^10)
      int digit_value;       // 0..9
    };
    std::vector<DigitSlot> digit_rects;
    QRect comma_rect;
    QRect dot_rect;
    QRect space_rect;

    int hover_index;         // -1 if none, else index into digit_rects
    QPoint last_mouse_pos;

    void rebuildLayout();
    int hitDigit(const QPoint & p) const;
    void applyDelta(long long delta_hz);
    void clampFrequency();
  };
}
