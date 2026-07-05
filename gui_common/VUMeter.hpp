#pragma once
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

#include <QWidget>
#include <QTimer>

namespace GUISoDa {

/**
 * @brief Floating analog-needle VU meter driven by the RX audio stream.
 * Toggle with Alt+V. Ballistic needle with peak hold tick.
 */
class VUMeter : public QWidget {
    Q_OBJECT

public:
    explicit VUMeter(QWidget* parent = nullptr);

public slots:
    /// Receive audio samples from AudioRXListener::pendAudioBuffer
    void updateLevel(float* buf, qint64 len);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onTimer();

private:
    float  target_db;       // raw RMS dB from latest audio buffer
    float  display_db;      // ballistic-smoothed value driving the needle
    float  peak_db;
    int    peak_hold_ticks;
    QTimer timer;

    // Scale: -60 dBFS (full left) to 0 dBFS (full right)
    static constexpr float MIN_DB     = -60.0f;
    static constexpr float MAX_DB     =   0.0f;
    // Ballistic time constants (fraction of gap closed per 100 ms tick)
    static constexpr float ATTACK     =   0.4f;
    static constexpr float DECAY_K    =   0.25f;
    // Peak hold
    static constexpr int   PEAK_HOLD  =  10;     // ticks (~1 s)
    static constexpr float PEAK_DECAY =   3.0f;  // dB per tick after hold

    double db_to_angle(float db) const;
};

} // namespace GUISoDa
