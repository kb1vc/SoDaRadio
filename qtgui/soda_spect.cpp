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

#include "soda_spect.hpp"
#include <iostream>
#include <boost/format.hpp>
#include <cmath>

#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_scale_engine.h>



SoDaSpect::SoDaSpect(QWidget *parent) :
    QwtPlot(parent)
{
    initPlot();
}

SoDaSpect::~SoDaSpect()
{
  if(freqs != NULL) delete[] freqs; 
  if(vals != NULL) delete[] vals; 
  num_buckets = 0; 
}


void SoDaSpect::initPlot()
{

  freq_span_disp = 10e3;
  val_ref = 10.0;

  num_buckets = 0; 
  freqs = NULL; 
  vals = NULL; 

  setCanvasBackground(Qt::black);
  //QwtLinearScaleEngine se;
  //setAxisScaleDiv(QwtPlot::xBottom, se.divideScale(144.1e6,144.2e6,10,5));
  enableAxis(QwtPlot::xBottom, true);
  enableAxis(QwtPlot::yLeft, true);
  enableAxis(QwtPlot::yRight, true);

  freq_draw_p = new SoDaFreqScaleDraw(); 
  setAxisScaleDraw(QwtPlot::xBottom, freq_draw_p);

  setFreqCenter(144.15e6);
  setRefLevel(20);

  curve_p = new QwtPlotCurve();
  curve_p->attach(this);

  grid_p = new QwtPlotGrid();
  grid_p->setPen(Qt::gray);
  grid_p->attach(this);

  curve_p->setPen(Qt::red);
  // curve_p->setSamples(xdat, ydat, 1000);

  // setup the plotpicker -- this creates click events for us.
  picker_p = new SoDaPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, canvas());

  connect(picker_p, SIGNAL(selected(const QPointF&)), SLOT(pickPoint(const QPointF&)));

  QColor fcol = Qt::green;
  fcol.setAlpha(96);
  freq_marker.setBrush(fcol);
  freq_marker.setPen(fcol);
  // make a dummy marker
  freq_marker.setRect(QRectF(1.0, -100.0, 2.0, 100.0));
  freq_marker.attach(this);
  replot();
  show();
}

void SoDaSpect::updateData(double cfreq, float * y)
{
  if(cfreq != center_freq_in) resetFreqAxis(cfreq); 
  for(int i = 0; i < num_buckets; i++) vals[i] = y[i]; 
  curve_p->setSamples(freqs, vals, num_buckets);
  replot();
}

void SoDaSpect::setRefLevel(int rlvl)
{
  val_ref = ((double) rlvl);
  replotYAxis();
}

void SoDaSpect::setDynamicRange(double drange)
{
  val_range = drange;
  replotYAxis();
}

void SoDaSpect::replotYAxis()
{
  double y_min = val_ref - val_range;
  double y_step = 5.0; 
  if(val_range < 25.1) y_step = 2.5; 
  if(val_range < 10.1) y_step = 1.0;
  setAxisScale(QwtPlot::yLeft, y_min, val_ref, y_step);
  setAxisScale(QwtPlot::yRight, y_min, val_ref, y_step);
  replot();
}


void SoDaSpect::replotXAxis()
{
  double min = center_freq_disp - freq_span_disp * 0.5;
  double max = center_freq_disp + freq_span_disp * 0.5;

  
  setAxisScale(QwtPlot::xBottom, min, max, freq_span_disp / 5.0); // step);

  QwtLinearScaleEngine se;
  setAxisScaleDiv(QwtPlot::xBottom, se.divideScale(min, max, 5, 5));
  freq_draw_p->setFreqStep(center_freq_disp, freq_span_disp / 5.0);
  replot();
}

void SoDaSpect::setFreqCenter(double cf, bool check_boundary) 
{
  (void) check_boundary;
  center_freq_disp = cf; 
  replotXAxis();
}


double SoDaSpect::correctCenterFreq(double cfreq)
{
  if((cfreq + 0.5 * freq_span_disp) > (center_freq_in + 0.5 * freq_span_in)) {
    cfreq = (center_freq_in + 0.5 * (freq_span_in - freq_span_disp));
  }
  if((cfreq - 0.5 * freq_span_disp) < (center_freq_in - 0.5 * freq_span_in)) {
    cfreq = (center_freq_in - 0.5 * (freq_span_in - freq_span_disp));
  }
  return cfreq; 
}

void SoDaSpect::setFreqSpan(double fs, bool check_boundary) {
  freq_span_disp = fs;   
  if(check_boundary) {
    center_freq_disp = correctCenterFreq(center_freq_disp);
    if((marker_freq < (center_freq_disp - 0.5 * freq_span_disp)) ||
       (marker_freq > (center_freq_disp + 0.5 * freq_span_disp))) {
      center_freq_disp = correctCenterFreq(marker_freq);
    }
  }
  replotXAxis();
}


void SoDaSpect::pickPoint(const QPointF & pos)
{
  double freq = pos.x() - marker_lo_offset; 
  setFreqMarker(freq);
  emit xClick(freq);
}

void SoDaSpect::setMarkerOffset(double lo, double hi) { 
  marker_lo_offset = lo;
  marker_hi_offset = hi;       
  setFreqMarker(marker_freq); 
}


void SoDaSpect::setFreqMarker(double freq)
{
  double f = freq + marker_lo_offset; 
  double width = marker_hi_offset - marker_lo_offset;
  freq_marker.setRect(QRectF(f, -200.0, width, 300.0));

  marker_freq = freq;   
  replot();
}

void SoDaSpect::resetFreqAxis(double cfreq) {
  // load up the X axis values. (frequency)
  double fincr = freq_span_in / ((double) (num_buckets-1));
  double fr = cfreq - 0.5 * freq_span_in; 
  for(int i = 0; i < num_buckets; i++) {
    freqs[i] = fr; 
    fr += fincr; 
  }
}

void SoDaSpect::configureSpectrum(double cfreq, double span, long buckets) {
  marker_freq = cfreq; 
  center_freq_in = cfreq; 
  freq_span_in = span; 
  if(num_buckets < buckets) {
    if(freqs != NULL) delete[] freqs; 
    if(vals != NULL) delete[] vals; 
    freqs = new double[buckets];
    vals = new double[buckets];
    num_buckets = buckets; 
  }

  resetFreqAxis(cfreq);  
  // load up the Y axis values. 
  for(int i = 0; i < num_buckets; i++) {
    vals[i] = -200.0; 
  }
    
  // set the center display frequency and span. 
  setFreqCenter(cfreq); 
  //   setFreqSpan(200.0e3); // default span. 
}
