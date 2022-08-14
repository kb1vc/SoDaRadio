#pragma once
/*
  Copyright (c) 2022 Matthew H. Reilly (kb1vc)
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


///
///  @file Filter.hxx
///  @brief This class creates an impulse response and a frequency image for
///  a filter given the specification, number of taps, resulting image size,
///  and sample rate.
///  It can be directly applied to an input buffer. 
///
///  This scheme replaces the awful filter creation scheme in SoDaRadio versions
///  8.x and before. 
///
///  @author M. H. Reilly (kb1vc)
///  @date   July 2022
///

#include <memory>
#include <iostream>
#include <complex>
#include <vector>
#include <fftw3.h>
#include "BlockOp.hxx"
#include "Radio.hxx"
#include "FilterSpec.hxx"
#include "FFT.hxx"

namespace SoDa {

  
  class Filter : public BlockOp {
  public:
    class BadBufferSize : public Radio::Exception {
    public:
      BadBufferSize(const std::string & st, unsigned int in, unsigned int out, unsigned int req) :
	Radio::Exception(SoDa::Format("Filter::%3 input and output buffer sizes (%0 and %1) must be equal to %2\n")
			 .addI(in).addI(out).addI(req).addS(st).str()) { }
    };


    
    /// constructor
    /// Build the filter from a filter spec for a bandpass filter
    /// 
    /// @param filter_spec object of class FilterSpec identifying corner frequencies and amplitudes
    /// @param image_size the impulse response and frequency image will be this long

    Filter(FilterSpec & filter_spec, 
	   unsigned int image_size); 

    /// Alternate constructor, for very simple filters
    Filter(float low_cutoff, float high_cutoff, float skirt, float sample_rate);
    
    /// run the filter on a complex input stream
    /// @param in_buf the input buffer I/Q samples (complex)
    /// @param out_buf the output buffer I/Q samples (complex)
    /// @param in_out_mode input or output can be time samples or frequency (FFT format) samples
    /// @return the length of the input buffer
    unsigned int apply(std::vector<std::complex<float>> & in_buf, 
		       std::vector<std::complex<float>> & out_buf, 
		       InOutMode in_out_mode = InOutMode(true,true));

    /// run the filter on a real input stream
    /// @param in_buf the input buffer samples
    /// @param out_buf the output buffer samples (this can overlap the in_buf vector)
    /// @param in_out_mode ignored -- input and output must be time-domain samples.
    /// @return the length of the input buffer
    ///
    /// Throws Filter::BadRealFilter if the original filter spec was not "real"
    unsigned int apply(std::vector<float> & in_buf, 
		       std::vector<float> & out_buf, 
		       InOutMode in_out_mode = InOutMode(true,true));

    /// dump the filter FFT to the output stream
    /// @param os an output stream. 
    void dump(std::ostream & os);

    /**
     * @brief Return the lowest and highest corner frequency for this filter. 
     */
    std::pair<float, float> getFilterEdges();

    /**
     * @brief how long must an output buffer be?
     * 
     * @param in_size the size of an input buffer to this filter
     * @return the same as in_size
     */
    unsigned int outLenRequired(unsigned int in_size) { return in_size; }

  protected:
    /// parameters that we keep to support display masks on the spectrogram
    double low_edge, high_edge; 

    void hammingWindow(std::vector<float> & w);
    
    // this is the FFT image of the filter
    std::vector<std::complex<float>> H;  ///< FFT image of the filter

    std::vector<std::complex<float>> h; ///< impulse response of the filter

    // We need an FFT widget for the input/output transforms
    std::unique_ptr<FFT> fft; 
    
    // and we need a temporary vector for the frequency domain product
    std::vector<std::complex<float>> temp_buf;

    // and a two more vectors if we're doing a real valued filter
    std::vector<std::complex<float>> temp_in_buf;
    std::vector<std::complex<float>> temp_out_buf;        
    
    unsigned int image_size; 
  };
}

