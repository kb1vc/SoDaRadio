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

#include <iostream>
#include <boost/format.hpp>
#include <cmath>

#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_scale_engine.h>
#include <qwt/qwt_scale_widget.h>
#include <qwt/qwt_color_map.h>

#include "soda_wfall.hpp"

using namespace GUISoDa;

#define FIVECOLOR


class GUISoDa::WFColorMap : public QwtLinearColorMap
{
public:
  enum MapSelector {C5, C7}; 
  WFColorMap(MapSelector sel = C5)
    :  QwtLinearColorMap(Qt::black, Qt::red, QwtColorMap::RGB)
  {
    if(sel == C5) {
      addColorStop(1.0, Qt::red);
      addColorStop(0.75, Qt::yellow);
      // addColorStop(0.50, Qt::green);
      addColorStop(0.25, Qt::cyan);
      addColorStop(0.0, Qt::blue); 
    }
    else {
      addColorStop(1.0, Qt::white);      
      addColorStop(0.84, Qt::red);
      addColorStop(0.67, Qt::yellow);
      addColorStop(0.5, Qt::green);
      addColorStop(0.33, Qt::cyan);
      addColorStop(0.16, Qt::blue);
      addColorStop(0.0, Qt::black); 
    }
  }
}; 


class GUISoDa::PlotSpectrogram : public QwtPlotSpectrogram
{
public:
  explicit PlotSpectrogram() : QwtPlotSpectrogram() {
    rescan_required = false; 
  }

  ~PlotSpectrogram() {
  }

  void setData(QwtRasterData * dp) { 
    data_p = dp; 
    QwtPlotSpectrogram::setData(dp); 
  }

  void rescan() {
    rescan_required = true; 
  }

protected:
  QImage renderImage(
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    const QRectF &area, const QSize &imageSize ) const
  {
    const QwtColorMap * cmap_p = colorMap();

   if ( imageSize.isEmpty() || data_p == NULL 
        || cmap_p == NULL )
    {
        return QImage();
    }

    const QwtInterval intensityRange = data_p->interval( Qt::ZAxis );
    if ( !intensityRange.isValid() )
        return QImage();

    QImage::Format format = ( cmap_p->format() == QwtColorMap::RGB )
        ? QImage::Format_ARGB32 : QImage::Format_Indexed8;

    data_p->initRaster(area, imageSize);
      
    QImage image( imageSize, format );    
    if(rescan_required || (lastSize != imageSize)) {
      rescan_required = false; 
      const QRect tile( 0, 0, image.width(), image.height() );
      renderTile( xMap, yMap, tile, &image );
      lastSize = imageSize;
    }
    else {
      // copy all the lines from the old image to the new image
      uchar * to_start = image.bits();
      uchar * from_start = saved_image.scanLine(1);
      memcpy(to_start, from_start, image.byteCount() - image.bytesPerLine());
      // scan a new line in.
      const QRect tile( 0, image.height()-1, image.width(), 1);
      renderTile( xMap, yMap, tile, &image ); 
    }
    saved_image = image; // .copy();
    return image;         
  }

  mutable QSize lastSize;
  mutable QImage saved_image;
  mutable bool rescan_required; 
  QwtRasterData * data_p;
};

GUISoDa::WFall::WFall(QWidget *parent) :
    QwtPlot(parent)
{
    initPlot();
}

GUISoDa::WFall::~WFall()
{
}


void GUISoDa::WFall::initPlot()
{
  center_on_next_setting = false;
  
  sgram = new GUISoDa::PlotSpectrogram();   
  sgram->setRenderThreadCount(1);

  sgram->setColorMap(new GUISoDa::WFColorMap()); 
  wfall_data = new WFallData();
  sgram->setData(wfall_data);
  sgram->attach(this);
  right_axis = axisWidget( QwtPlot::yRight);
  right_axis->setTitle("Signal Strength");
  right_axis->setColorBarEnabled(true);
  right_axis->setColorBarWidth(10);
  right_axis->setColorMap(wfall_data->interval(Qt::ZAxis), new GUISoDa::WFColorMap()); 

  setAxisScale(QwtPlot::yRight, wfall_data->interval(Qt::ZAxis).minValue(),
	       wfall_data->interval(Qt::ZAxis).maxValue());
  enableAxis(QwtPlot::yRight, true);
  enableAxis(QwtPlot::yLeft, false);

  freq_scale_p = new FreqScaleDraw();
  setAxisScaleDraw(QwtPlot::xBottom, freq_scale_p);  
  setFreqCenter(1.0e6);
  freq_span = 200e3;
  
  sgram->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
  sgram->setDefaultContourPen(QPen(Qt::black, 0));

  picker_p = new WFallPicker(QwtPlot::xBottom, QwtPlot::yLeft, canvas(), this);
  connect(picker_p, SIGNAL(selected(const QPointF&)), SLOT(pickPoint(const QPointF&)));

  last_input_cfreq = 0.0; 

  setMarkerOffset(-1.0, 1.0);

  replot();
  show();
}

double GUISoDa::WFall::correctCenterFreq(double cfreq)
{
  if((cfreq + 0.5 * freq_span) > (last_input_cfreq + 0.5 * spectrum_input_span)) {
    cfreq = (last_input_cfreq + 0.5 * (spectrum_input_span - freq_span));
  }
  if((cfreq - 0.5 * freq_span) < (last_input_cfreq - 0.5 * spectrum_input_span)) {
    cfreq = (last_input_cfreq - 0.5 * (spectrum_input_span - freq_span));
  }
  return cfreq; 
}

void GUISoDa::WFall::setFreqCenter(double cfreq, bool check_boundary)
{
  if(check_boundary) {
    cfreq = correctCenterFreq(cfreq); 
  }
  center_freq = cfreq; 
  freq_scale_p->setFreqStep(cfreq, freq_span);
  setAxisScale(QwtPlot::xBottom, cfreq - 0.5 * freq_span, cfreq + 0.5 * freq_span);
  sgram->rescan();    
  replot();
}

void GUISoDa::WFall::setFreqSpan(double span, bool check_boundary)
{
  freq_span = span;
  if(check_boundary && (center_freq != 0.0)) {
    center_freq = correctCenterFreq(center_freq); 
    // and make sure we don't scroll the marker away. 
    if((marker_freq < (center_freq - 0.5 * freq_span)) ||
       (marker_freq > (center_freq + 0.5 * freq_span))) {
      center_freq = correctCenterFreq(marker_freq);       
    }
  }
  
  freq_scale_p->setFreqStep(center_freq, freq_span);
  setAxisScale(QwtPlot::xBottom, center_freq - 0.5 * freq_span, center_freq + 0.5 * freq_span);
  sgram->rescan();  
  replot();  
}


void GUISoDa::WFall::updateData(double cfreq, float * spect)
{
  last_input_cfreq = cfreq; 
  wfall_data->updateData(cfreq, spect);
  replot();
}

void GUISoDa::WFall::setMarkerOffset(double lo, double hi) { 
  marker_lo_offset = lo;
  marker_hi_offset = hi;       
  setFreqMarker(marker_freq); 
}

void GUISoDa::WFall::setFreqMarker(double freq) 
{
  marker_freq = freq; 
  setMarkers(freq + marker_lo_offset, freq + marker_hi_offset);   
} 

void GUISoDa::WFall::pickPoint(const QPointF & pos)
{
  double freq = pos.x() - marker_lo_offset;   
  setFreqMarker(freq);
  if(center_on_next_setting) {
    center_on_next_setting = false;
    emit setSpectrumCenter(freq);
  }

  emit xClick(freq);
}

void GUISoDa::WFall::setDynamicRange(double drange)
{
  wfall_data->setDynamicRange(drange);
  setZAxis();
}

void GUISoDa::WFall::setRefLevel(int reflvl)
{
  wfall_data->setRefLevel((double) reflvl);
  setZAxis();  
}

void GUISoDa::WFall::setZAxis()
{
  const QwtInterval z_interval = wfall_data->interval(Qt::ZAxis); 
  right_axis->setColorMap(z_interval, new GUISoDa::WFColorMap()); 

  setAxisScale(QwtPlot::yRight, z_interval.minValue(),
	       z_interval.maxValue());

  sgram->rescan();
}

