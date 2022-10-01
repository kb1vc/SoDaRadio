/*
Copyright (c) 2013,2014 Matthew H. Reilly (kb1vc)
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

#ifndef PARAMS_HDR
#define PARAMS_HDR

#include <SoDa/Options.hxx>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>

namespace SoDa {
  /**
   * This class handles command line parameters and built-ins. 
   */
  class Params {
  public:
    /**
     * @brief Constructor
     *
     * @param argc count of command line arguments
     * @param argv pointer to list of parameter strings
     */
    Params(int argc, char * argv[]);

    /**
     * @brief return args that point to a particular USRP unit
     * @return string identifying which USRP we are selecting
     */
    std::string getRadioArgs() const { return radio_args; }

    

    /**
     * @brief where does the reference come from?
     * @return 1 for external, 0 for internal
     */
    int getClockSource() const { return clock_source_internal ? 0 : 1; }


    /**
     * @brief which port is the RX channel?
     * @return string RX2 or TX/RX
     */
    std::string getRXAnt() const { return rx_ant; }
    /**
     * @brief which port is the TX channel?
     * @return string TX
     */
    std::string getTXAnt() const { return tx_ant; }

    /**
     * Sample rates and all that other stuff are fixed.
     * we do this because downsampling the RX can take a lot
     * of time (in general) unless we focus on the simple
     * common case.  Here we're going to use a
     * 625000 S/s IF rate from the USRP and down sample
     * with an FFT based decimator to 48KS/s .   This
     * 625/48 ratio is easiest to achieve with an IF buffer
     * size of 30000 samples 30,000 * 48 / 625 = 2304 --
     * the size of the RX buffer
     */
    double getRXRate() const { 
      return rx_sample_rate; 
    }
    
    void setRXRate(double rate) {
      rx_sample_rate = rate; 
    }

    /**
     * @brief TX rate will always be 625K
     */
    double getTXRate() const {
      return tx_sample_rate;
    }
    
    void setTXRate(double rate) {
      tx_sample_rate = rate; 
    }

    double getAudioSampleRate() const { return 48000.0 ; }
    unsigned int getTXRFBufferSize() const { return tx_rf_buffer_size; }
    unsigned int getRXRFBufferSize() const { return rx_rf_buffer_size; }    
    unsigned int getTXAFBufferSize() const { return tx_af_buffer_size; }
    unsigned int getRXAFBufferSize() const { return rx_af_buffer_size; }    

    void setTXRFBufferSize(unsigned int s)  { tx_rf_buffer_size = s; }
    void setRXRFBufferSize(unsigned int s)  { rx_rf_buffer_size = s; }
    void setTXAFBufferSize(unsigned int s)  { tx_af_buffer_size = s; }
    void setRXAFBufferSize(unsigned int s)  { rx_af_buffer_size = s; }        

    float getSampleChunkDuration() { return 0.05; }

    std::string getServerSocketBasename() const { return server_sock_basename; }

    std::string getAudioPortName() const { return audio_portname; }
    
    bool forceFracN() { return force_frac_N_mode && !force_integer_N_mode; }
    bool forceIntN() { return !force_frac_N_mode && force_integer_N_mode; }

    const std::vector<std::string> & getLibs();
    
    unsigned int getDebugLevel() const { return debug_level; }

    std::string getRadioType() const { return radio_type; }

    std::string getGPSHostName() const { return gps_hostname; }

    std::string getGPSPortName() const { return gps_portname; }    

    std::string getLockFileName() const { return lock_file_name; }


    bool isRadioType(const std::string & rtype) {
      std::string rt = rtype;

      std::transform(rt.begin(), rt.end(), rt.begin(), ::toupper);
      std::transform(radio_type.begin(), radio_type.end(), radio_type.begin(), ::toupper);
      bool res = (rt == radio_type);

      return res; 
    }

  private:
    
    SoDa::Options cmd;

    std::string gps_hostname; 
    std::string gps_portname; 
    std::string radio_type; 
    std::string radio_args;
    std::string config_filename;
    std::vector<std::string> load_list;
    bool load_list_env_appended;

    std::string lock_file_name; 

    bool clock_source_internal;
    double rx_rate, tx_rate;
    std::string rx_ant, tx_ant;

    double tx_sample_rate;
    double rx_sample_rate;
    unsigned int tx_rf_buffer_size, rx_rf_buffer_size;
    unsigned int tx_af_buffer_size, rx_af_buffer_size;
    
    // message socket parameters
    std::string server_sock_basename;

    // audio port/device name
    std::string audio_portname; 

    // synthesizer mode switches (over-rides local determination)
    bool force_frac_N_mode;
    bool force_integer_N_mode;

    unsigned int debug_level; 
  };
}
#endif
