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

#ifndef SODAWFALL_H
#define SODAWFALL_H

#include <QWidget>
#include <qwt/qwt_interval.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_spectrogram.h>
#include <qwt/qwt_raster_data.h>
#include <qwt/qwt_plot_picker.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_plot_panner.h>
#include <qwt/qwt_scale_draw.h>
#include <qwt/qwt_plot_marker.h>
#include <qwt/qwt_plot_shapeitem.h>


#include <QMouseEvent>
#include <QPointF>

#include <iostream>

#include "soda_wfall_picker.hpp"
#include "soda_freq_scale_draw.hpp"
#include "soda_wfall_data.hpp"

#include <cmath>

namespace GUISoDa {
  class PlotSpectrogram ;
  class WFColorMap; 

  class WFall : public QwtPlot, WFallPickerClient
  {
    Q_OBJECT
  
  public:
    explicit WFall(QWidget *parent = 0);
    ~WFall();

    double freqCenter() { return center_freq; }

    void handleMouseEvent(const QMouseEvent * ev) {
      // if we get here, the user clicked MB3 in the window -- 
      // center the display on his clicked frequency.
      center_on_next_setting = true; 
    }

  public slots:
    void updateData(double cf, float * y);
    void pickPoint(const QPointF & pos);

    void setFreqMarker(double freq); 

    void setDynamicRange(double drange);
    void setRefLevel(int rlvl);
    void setFreqCenter(double cf, bool check_boundary = false);
    void setFreqSpan(double fs, bool check_boundary = false);
    void setFreqSpanKHz(double fs) { setFreqSpan(1e3 * fs, true); }     
    void scrollRight(bool v) { 
      (void) v;
      setFreqCenter(center_freq + freq_span * 0.25, true); 
    }
    void scrollLeft(bool v) { 
      (void) v;
      setFreqCenter(center_freq - freq_span * 0.25, true); }
    void configureSpectrum(double cfreq, double span, long buckets) {
      setFreqCenter(cfreq); 
      // setFreqSpan(span);
      spectrum_input_span = span; 
      wfall_data->setSpectrumDimensions(cfreq, span, buckets); 
    }
    void setMarkerOffset(double lo, double hi);

  private:
    void setZAxis();
    void setMarkers(double lo, double hi) { 
      wfall_data->setMarkers(lo, hi); 
    }
    double marker_lo_offset; 
    double marker_hi_offset; 
    
  public:
    
  signals:
    void xClick(double x);
    void setSpectrumCenter(double x); // send when we should re-center the spectrum display
    
  protected:

    double correctCenterFreq(double cfreq);
    
    double center_freq; 
    double freq_span;
    double spectrum_input_span;     
    double last_input_cfreq;
    double marker_freq; 


    WFallData * wfall_data ;
    PlotSpectrogram * sgram; 

    WFallPicker * picker_p;     

    FreqScaleDraw * freq_scale_p; 

    QwtScaleWidget * right_axis;

    // if true, reset the plot center the next time the user
    // picks a frequency. 
    bool center_on_next_setting;
  private:
    void initPlot();

  };
}
#endif
