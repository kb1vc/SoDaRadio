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
///  @file FilterSpec.hxx
///  @brief This class describes a filter profile -- corner frequencies and
///  amplitudes. 
///
///  This scheme replaces the awful filter creation scheme in SoDaRadio versions
///  8.x and before. 
///
///  @author M. H. Reilly (kb1vc)
///  @date   July 2022
///

#include <vector>
#include <list>
#include "Radio.hxx"

namespace SoDa {

  class FilterSpec {
  public:
    enum FType { REAL, COMPLEX };
    
    struct Corner {
      Corner(float freq, float gain) : freq(freq), gain(gain) { }
      float freq;
      float gain;
    };

    class BadRealSpec : public Radio::Exception {
    public:
      BadRealSpec(const std::string & st, float freq) :
	Radio::Exception(SoDa::Format("FilterSpec::%1 specification for REAL valued filter contains a negative frequency %0. Not good.\n")
			 .addF(freq).addS(st).str()) { }
    };
    
    /**
     * @brief construct a filter specification
     *
     * @param sample_rate 
     * @param taps number of taps required
     * @param filter_type if REAL, then the filter shape (specified
     * by the added corners) must contain only positive
     * frequencies. The constructor will throw Filter::BadRealSpec
     * otherwise.
     */
    FilterSpec(float sample_rate, unsigned int taps,
	       FType filter_type = COMPLEX);


    /**
     * @brief Alternate constructor, for very simple band-pass filters
     * 
     * @param sample_rate in Hz -- as are all specified frequencies
     * @param low_cutoff lower 3dB point
     * @param high_cutoff upper 3dB point
     * @param skirt_width width of transition band
     * @param taps number of taps required
     * @param filter_type if REAL, then the filter shape (specified
     * by the added corners) must contain only positive
     * frequencies. The constructor will throw Filter::BadRealSpec
     * otherwise.
     */
    Filter(float sample_rate, float low_cutoff, float high_cutoff, float skirt_width, 
	   unsigned int taps, FType filter_type = COMPLEX);
    
    /**
     * Set the starting gain for this filter. (Defaults to -200dB)
     * @param gain the ideal amplitude at the lowest filter frequency (- sample_freq / 2)
     * @return reference to this FilterSpec
     */
    FilterSpec & start(float gain); 

    /**
     * Add a point in the transfer function. This specifies
     * a "corner" in the frequency response. 
     * 
     * @param freq the corner frequency
     * @param gain the ideal gain at the corner frequency (in dB)
     * @return reference to this FilterSpec
     */
    FilterSpec & add(float freq, float gain); 
      
    const std::list<Corner> getSpec();

    float getSampleRate() { return sample_rate; }

    void fillHproto(std::vector<std::complex<float>> & Hproto);
    
    unsigned int indexHproto(float freq); 
      
    /**
     * @brief How many taps do we need to provide the requested 
     * shortest transition edge? 
     * 
     * Estimate and set the taps as appropriate. But the 
     * number of taps must be in the range min...max
     * 
     * @param min imum number of taps provided
     * @param max imum number of taps provided
     * @return number of taps chosen
     */
    unsigned int estimateTaps(unsigned int min, unsigned int max); 

    unsigned int setTaps(unsigned int new_tapcount) { 
      taps = new_tapcount; 
      return taps;
    }
    
    unsigned int getTaps() {
      return taps; 
    }

    std::pair<float, float> getFilterEdges();
    
  protected:
    bool sorted; 
    void sortSpec();
      
    unsigned int taps; 
    float sample_rate;
    FType filter_type;
      
    std::list<Corner> spec; 
  };
}

