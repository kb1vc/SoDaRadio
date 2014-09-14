/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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
#ifndef OS_FILTER_HDR
#define OS_FILTER_HDR


 ///
 ///  @file OSFilter.hxx
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
  class OSFilter {
  public:
    

    /// constructor
    /// Build the filter from the time domain FIR filter sequence CASCADED with the filter
    /// specified by the [cascade] parameter
    /// @param filter_impulse_response time domain FIR filter coefficient array -- real
    /// @param filter_length number of filter taps
    /// @param filter_gain desired gain in passband
    /// @param inout_buffer_length used to set aside storage for overlap and save buffer
    /// @param cascade use this filter as a "prefilter" if the inout_buffer_lengths are equal
    /// @param suggested_transform_length a hint for optimizing FFT operations
    OSFilter(float * filter_impulse_response,
	     unsigned int filter_length,
	     float filter_gain, 
	     unsigned int inout_buffer_length,
	     OSFilter * cascade = NULL,
	     unsigned int suggested_transform_length = 0);

    /// constructor
    /// Build the filter from a filter spec for a bandpass filter
    /// 
    /// @param low_cutoff frequency below which the response is (ideally) zero
    /// @param low_pass_edge low end of the bandpass range
    /// @param high_pass_edge high end of the bandpass range
    /// @param high_cutoff frequency above which the response is (ideally) zero
    /// @param filter_length minimum number of effective filter taps
    /// @param filter_gain desired gain in passband
    /// @param sample_rate in samples/sec to allow normalization of the frequency specs
    /// @param inout_buffer_length used to set aside storage for overlap and save buffer
    /// @param suggested_transform_length a hint for optimizing FFT operations
    OSFilter(float low_cutoff,
	     float low_pass_edge,
	     float high_pass_edge,
	     float high_cutoff,

	     unsigned int filter_length,
	     float filter_gain, 
	     float sample_rate, 

	     unsigned int inout_buffer_length,
	     unsigned int suggested_transform_length = 0);
    
    /// run the filter on a complex input stream
    /// @param inbuf the input buffer I/Q samples (complex)
    /// @param outbuf the output buffer I/Q samples (complex)
    /// @param outgain normalized output gain
    /// @return the length of the input buffer
    unsigned int apply(std::complex<float> * inbuf, std::complex<float> * outbuf, float outgain = 1.0);

    /// run the filter on a real input stream
    /// @param inbuf the input buffer samples
    /// @param outbuf the output buffer samples
    /// @param outgain normalized output gain
    /// @param instride index increment for the input buffer
    /// @param outstride index increment for the output buffer
    /// @return the length of the input buffer
    unsigned int apply(float * inbuf, float * outbuf, float outgain = 1.0,
		       int instride = 1, int outstride = 1);

    /// dump the filter FFT to the output stream
    /// @param os an output stream. 
    void dump(std::ostream & os);
  private:
    /// pick a likly N - FFT length.
    int guessN();
    void setupFFT();

    
    
    // these are the salient dimensions for this Overlap/Save
    // widget (for terminology, see Lyons pages 719ff
    unsigned int M; ///< the input buffer length;
    unsigned int Q; ///< the filter length
    unsigned int N; ///< the total length of the transform N > (M + Q-1)

    // some helpful stuff.
    unsigned int tail_index; ///< the beginning of the end. 
    
    // these are the intermediate buffers
    std::complex<float> * fft_input;  ///< a copy of the input stream.
    std::complex<float> * fft_output; ///< the transformed input stream + overlap
    std::complex<float> * ifft_output; ///< the output stream + overlap discard

    // this is the FFT image of the filter
    std::complex<float> * filter_fft;  ///< FFT image of the input filter
    
    // each filter needs two plans, a forward and backward
    // plan for the FFT and IFFT
    fftwf_plan forward_plan, backward_plan; ///< plans for fftw transform ops
  };
}

#endif
