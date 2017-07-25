#include "soda_wfall_data.h"
#include <iostream>
#include <boost/format.hpp>

SoDaWFallData::SoDaWFallData() {
  clearReady(); 
  num_rows = 300;
  min_time = -1.0 * ((double) num_rows);
  
  setInterval( Qt::XAxis, QwtInterval(0.0, 100e9)); // I don't think we'll see too many 100GHz scans
  setInterval( Qt::YAxis, QwtInterval(-1.0 * ((double) num_rows), 0.0)); // store the last 300 rows

  ref_level = -80.0; 
  dynamic_range = 80.0; 
  setZRange();

  start_freq = new double[num_rows]; 
  cur_freq_idx = 0; 
  cur_row_idx = 0; 
  last_cidx = 0; 

  f_lo_marker = f_hi_marker = 0.0; 

  heatmap = NULL; 
  num_buckets = 0; 

}

SoDaWFallData::~SoDaWFallData() {
  delete[] heatmap;
}

void SoDaWFallData::setSpectrumDimensions(double cfreq, double span, long buckets)
{
  (void) cfreq; 
  clearReady();

  if((heatmap != NULL) && (buckets != num_buckets)) {
    delete[] heatmap; 
    heatmap = NULL; 
  }
  if(heatmap == NULL) {
    heatmap_size = buckets * num_rows;     
    heatmap = new double[heatmap_size];
  }
  
  num_buckets = buckets; 

  // these are the dimensions for the input data -- it may 
  span_in_freq = span;

  r_fbucket_size = ((double) num_buckets) / span; 
  setReady();
}

double SoDaWFallData::value(double f, double t) const
{
  // if we don't have the scales setup, then report a very negative number
  if(!is_ready) return -1e6;
  
  // first, see if this is a marker position.
  if(((last_freq < f_lo_marker) && (f >= f_lo_marker)) ||
     ((last_freq < f_hi_marker) && (f >= f_hi_marker))) {
    last_freq = f; 
    return 1e6; // bright line
  }
  last_freq = f; 

  // t goes back in time.  0 is now, min_time is "back there"
  // cur_row_idx in the heatmap is the latest entry.
  //
  long rowidx;       
  long freqidx; 
  if(t == 0.0) {
    rowidx = cur_row_idx; 
    freqidx = cur_freq_idx; 
  }
  else {
    long tidx = ((long) (t - min_time)); 
    rowidx = cur_row_idx - num_buckets * tidx; 
    freqidx = cur_freq_idx - tidx; 
    if(rowidx < 0) {
      rowidx += heatmap_size;
      freqidx += num_rows; 
    }
    if(rowidx > (heatmap_size - num_buckets)) {
      rowidx -= heatmap_size;
      freqidx -= num_rows;       
    }
  }
  // now find the index for the frequency.
  double min_spect_freq = start_freq[freqidx];
  // if the frequency is out of range, return nosignal
  if((f < min_spect_freq) || (f > (min_spect_freq + span_in_freq))) return -1e6; 

  

  long cidx = ((long) ((f - min_spect_freq) * r_fbucket_size)); 
  // since the data points requested are at intervals that don't match
  // our actual bucket size, we sample the range of values from the point
  // of the last sample to this one... (pixels will then show the max value
  // in the region to the pixel's left.)
  double ret;
  if(cidx > last_cidx) {
    // report out the max value;
    ret = heatmap[rowidx + cidx]; 
    for(int i = rowidx + last_cidx + 1; i <= rowidx + cidx; i++) {
      if(ret < heatmap[i]) ret = heatmap[i]; 
    }
  }
  else {
    ret = heatmap[rowidx + cidx];
  }
  last_cidx = cidx; 

  return ret; 
}

void SoDaWFallData::setZRange() {
  setInterval( Qt::ZAxis, QwtInterval(ref_level, ref_level + dynamic_range));
}

void SoDaWFallData::setDynamicRange(double drange) {
  dynamic_range = drange; 
  setZRange(); 
}

void SoDaWFallData::setRefLevel(double reflvl) {
  ref_level = reflvl;
  setZRange(); 	
}

void SoDaWFallData::updateData(double cfreq, float * spect) {
  if(!is_ready) return; 
  cur_row_idx += num_buckets;
  cur_freq_idx++; 
  if(cur_row_idx > (heatmap_size - num_buckets)) {
    cur_row_idx = 0;
    cur_freq_idx = 0; 
  }
  
  double * vec = heatmap + cur_row_idx; 
  for(int i = 0; i < num_buckets; i++) {
    vec[i] = spect[i]; 
  }
  start_freq[cur_freq_idx] = cfreq - 0.5 * span_in_freq; 
}
