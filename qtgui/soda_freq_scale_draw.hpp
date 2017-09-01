/*
Copyright (c) 2017 Matthew H. Reilly (kb1vc)
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

#ifndef SODA_FREQ_SCALE_DRAW_HDR
#define SODA_FREQ_SCALE_DRAW_HDR
#include <qwt/qwt_scale_draw.h>
#include <boost/format.hpp>
#include <cmath>

class SoDaFreqScaleDraw : public QwtScaleDraw
{
 public:
  explicit SoDaFreqScaleDraw() {
    center_freq = 0.0;
  }

  void setFreqStep(double cf, double st) { center_freq = cf; freq_step = st; }

  virtual QwtText label(double f) const override {
    if(1 || (fabs(f - center_freq) < 0.5 * freq_step)) {
      return QwtText(QString::number(f * 1e-6, 'f', 3));
    }
    else {
      double df = f * 1e-6;
      df = df - floor(df);

      return QwtText(QString::number(df * 1000.0, 'f', 0));
    }
  }
 protected:
  double center_freq;
  double freq_step;
};

#endif
