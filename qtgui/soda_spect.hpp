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

#ifndef SODASPECT_H
#define SODASPECT_H

#include <iostream>

#include <QWidget>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_picker.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_plot_marker.h>
#include <qwt/qwt_plot_shapeitem.h>

#include <QMouseEvent>
#include <QPointF>

#include "soda_plot_picker.hpp"
#include "soda_freq_scale_draw.hpp"

namespace GUISoDa {
class Spect : public QwtPlot
{
    Q_OBJECT

public:
    explicit Spect(QWidget *parent = 0);
    ~Spect();

  double freqCenter() { return center_freq_disp; }

public slots:
  void updateData(double cfreq, float * y); 
  void pickPoint(const QPointF & pos);
  void setDynamicRange(double drange);
  void setRefLevel(int rlvl);
  void setFreqCenter(double cf, bool check_boundary = false);
  void setFreqSpan(double fs, bool check_boundary = false);  
  void setFreqSpanKHz(double fs) { setFreqSpan(1e3 * fs, true); }     
  void setFreqMarker(double f); 

  void scrollRight(bool v) { 
    (void) v;
    setFreqCenter(center_freq_disp + freq_span_disp * 0.25, true); 
  }
  void scrollLeft(bool v) { 
    (void) v;
    setFreqCenter(center_freq_disp - freq_span_disp * 0.25, true); 
  }
  
  void configureSpectrum(double cfreq, double span, long buckets);
    
  void setMarkerOffset(double lo, double hi);

protected:

  void resetFreqAxis(double cfreq);
  
  void replotXAxis();
  void replotYAxis();

  double correctCenterFreq(double cfreq);

public:
signals:
  void xClick(double x);

protected:
  // storage for data to be plotted. 
  double * freqs; 
  double * vals; 
  int num_buckets; 

  // the input data covers a frequency range that 
  // is not necessarily related to the display range. 
  double center_freq_in;
  double freq_span_in; 

  // This is the display range (x and y dimensions)
  // x axis settings
  double center_freq_disp;
  double freq_span_disp;

  // y axis settings
  double val_ref; // value for maximum point on the scale
  double val_range;

  // marker offset   
  double marker_lo_offset; 
  double marker_hi_offset; 

  // marked frequency
  double marker_freq; 

  FreqScaleDraw * freq_draw_p;

  QwtPlotCurve * curve_p;

  PlotPicker * picker_p;

  QwtPlotGrid * grid_p;

  QwtPlotShapeItem freq_marker;


private:
  void initPlot();

};
}

#endif // XYPLOTWIDGET_H
