#ifndef SODA_WFALL_DATA_H
#define SODA_WFALL_DATA_H

#include <QWidget>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_raster_data.h>

class SoDaWFallData : public QwtRasterData
{
 public:
  SoDaWFallData();

  ~SoDaWFallData();
  
  virtual double value(double f, double t) const; 

  void setZRange();

  void setDynamicRange(double drange);

  void setRefLevel(double reflvl);
  
  virtual void updateData(double cfreq, double * spect);

  void setSpectrumDimensions(double cfreq, double span, long buckets);

  void setMarkers(double lo, double hi) { 
    f_lo_marker =  lo; 
    f_hi_marker = hi; 
  }
  
 private:
  void setReady() { is_ready = true; }
  void clearReady() { is_ready = false; }
  bool is_ready;

  double r_fbucket_size;  
  double span_in_freq; // the frequency limits/span for the input stream

  double f_lo_marker, f_hi_marker;
  mutable double last_freq;  // last frequency seen by the value function

  double min_time; ///< earliest time relative to "now" 

  // The heatmap contains spectral power buckets for each "scan" at a frequency "f"
  // such that
  // heatmap[i * num_buckets] is the start of the scan for a sample between
  // start_freq[i] and start_freq[i] + span_in_freq
  double *heatmap;
  long heatmap_size; 
  double *start_freq;  ///< for each row in the heat map, this is the centerfreq. 
  long num_buckets, num_rows; 
  long cur_row_idx; 
  long cur_freq_idx; 

  mutable long last_cidx; 

  double ref_level; 
  double dynamic_range; 
};


#endif

