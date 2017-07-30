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

class SoDaSpect : public QwtPlot
{
    Q_OBJECT

public:
    explicit SoDaSpect(QWidget *parent = 0);
    ~SoDaSpect();

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
    std::cerr << "in SoDaSpect scrollRight\n";    
    setFreqCenter(center_freq_disp + freq_span_disp * 0.25, true); 
  }
  void scrollLeft(bool v) { 
    (void) v;
    std::cerr << "in SoDaSpect scrollLeft\n";        
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

  SoDaFreqScaleDraw * freq_draw_p;

  QwtPlotCurve * curve_p;

  SoDaPlotPicker * picker_p;

  QwtPlotGrid * grid_p;

  QwtPlotShapeItem freq_marker;


private:
  void initPlot();

};

#endif // XYPLOTWIDGET_H
