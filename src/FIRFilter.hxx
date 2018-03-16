/*
  Copyright (c) 2017, Matthew H. Reilly (kb1vc)
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
#ifndef FIR_FILTER_HDR
#define FIR_FILTER_HDR


 ///
 ///  @file TDFIRFilter.hxx
 ///  @brief This is an overlap-and-save frequency domain implementation
 ///  of a general FIR filter widget.
 ///
 ///  @author M. H. Reilly (kb1vc)
 ///  @date   July 2013
 ///

#include <fstream>
#include <complex>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fftw3.h>
namespace SoDa {
  /// Overlap-and-save filter class.  
  class FIR_Filter {
  public:
    /// constructor
    /// Build the filter from a filter spec for a lowpass filter
    /// 
    /// @param high_pass_edge high end of the bandpass range
    /// @param high_cutoff frequency above which the response is (ideally) zero
    /// @param filter_length maximum number of effective filter taps
    /// @param filter_gain desired gain in passband
    /// @param sample_rate in samples/sec to allow normalization of the frequency specs
    /// @param inout_buffer_length used to set aside storage for overlap and save buffer
    /// @param suggested_transform_length a hint for optimizing FFT operations
    FIR_Filter(float high_pass_edge,
		float high_cutoff,
		
		float sample_rate, 		
		unsigned int filter_length = 16,
		float filter_gain = 1.0);
    
    /// run the filter on a complex input stream
    /// @param inbuf the input buffer I/Q samples (complex)
    /// @param outbuf the output buffer I/Q samples (complex)
    /// @param outgain normalized output gain
    /// @return the length of the input buffer
    unsigned int apply(std::complex<float> * inbuf, std::complex<float> * outbuf, float outgain = 1.0);

    /// run the filter on a real input stream
    /// @param inbuf the input buffer samples
    /// @param outbuf the output buffer samples (this can overlap the inbuf vector)
    /// @param outgain normalized output gain
    /// @param instride index increment for the input buffer
    /// @param outstride index increment for the output buffer
    /// @return the length of the input buffer
    unsigned int apply(float * inbuf, float * outbuf, float outgain = 1.0,
		       int instride = 1, int outstride = 1);

  protected:
    float * filter; // the filter coefficients
    float * invec; // the last N samples. 
  };
}

#endif
