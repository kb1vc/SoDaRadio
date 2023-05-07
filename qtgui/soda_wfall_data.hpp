#pragma once
/*
Copyright (c) 2017,2023 Matthew H. Reilly (kb1vc)
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
#include <qwt/qwt_interval.h>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_raster_data.h>

namespace GUISoDa {
  class WFallData : public QwtRasterData
  {
  public:
    WFallData();

    ~WFallData();
  
    virtual double value(double f, double t) const; 

    void setZRange();

    void setDynamicRange(double drange);

    void setRefLevel(double reflvl);
  
    virtual void updateData(double cfreq, float * spect);

    void setSpectrumDimensions(double cfreq, double span, long buckets);

    void setMarkers(double lo, double hi) { 
      f_lo_marker =  lo; 
      f_hi_marker = hi; 
    }
  
    QwtInterval interval(Qt::Axis ax) const;
    
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
}


