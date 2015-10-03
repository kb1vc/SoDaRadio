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

#include <boost/format.hpp>
#include <boost/program_options.hpp>

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
    std::string & getUHDArgs() { return uhd_args; }

    /**
     * @brief where does the reference come from?
     * @return "EXTERNAL" or "INTERNAL" 
     */
    std::string & getClockSource() { return clock_source; }


    /**
     * @brief which port is the RX channel?
     * @return string RX2 or TX/RX
     */
    std::string & getRXAnt() { return rx_ant; }
    /**
     * @brief which port is the TX channel?
     * @return string TX
     */
    std::string & getTXAnt() { return tx_ant; }

    /**
     * @brief read a configuration file (for now, just setup the default config.)
     */
    void readConfigFile(std::string &cf_name );
    /**
     * @brief save config data to a configuration file (for now, do nothing.)
     */
    void saveConfigFile(std::string &cf_name ); 

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
    double getRXRate() { return 625000; }
    /**
     * @brief TX rate will always be 625K
     */
    double getTXRate() { return 625000; }

    double getAudioSampleRate() { return 48000.0 ; }
    unsigned int getRFBufferSize() { return (unsigned int) 30000; }
    unsigned int getAFBufferSize() { return (unsigned int) 2304; }

    std::string getServerSocketBasename() { return server_sock_basename; }

    std::string getAudioPortName() { return audio_portname; }
    
    std::string getGPSDev() { return "/dev/ttyGPS"; }

    bool forceFracN() { return force_frac_N_mode && !force_integer_N_mode; }
    bool forceIntN() { return !force_frac_N_mode && force_integer_N_mode; }

    unsigned int getDebugLevel() { return debug_level; }

  private:
    
    boost::program_options::variables_map pmap;

    std::string uhd_args;
    std::string config_filename;

    std::string clock_source;
    double rx_rate, tx_rate;
    std::string rx_ant, tx_ant;

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
