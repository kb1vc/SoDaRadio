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
