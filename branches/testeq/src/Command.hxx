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

#ifndef COMMAND_HDR
#define COMMAND_HDR

#include <string>
#include "MultiMBox.hxx"
#include <string.h>
#include <set> 

namespace SoDa {
  /** This is a list of all the commands that can "do something"
   * to one or more components in the SoDa radio.
   * The base class defines all the command types in an
   * enum.  Each command can take up to 4 double and 4 integer
   * parameters.  (This isn't very OO, but the intent is to
   * keep the command packet format simple, stable, and
   * universal.)
   *
   */
  class Command : public MBoxMessage {
  public:
    /**
     * @brief Commands are of the form, SET, GET, or REPort some
     * parameter.
     */
    enum cmd_type { SET, GET, REP, NONE };

    /**
     * @brief Each command has a "target" state that it is meant to modify, query, or report
     */
    enum cmd_target {
      /**
       * Set the RX front end (1st LO, the 2nd IF LO), and the 3rd LO
       * in a more-or-less optimal way to position the requested
       * frequency at least 80 kHz above the target frequency. 
       *
       * param is frequency as a double
       *
       * @see SoDa::USRPCtrl
       */
      RX_TUNE_FREQ,

      /**
       * Set the RX front end 1st LO and the 2nd IF LO.
       * The 1st LO is an analog synthesizer and the 2nd LO
       * drives the digital down converter in the USRP FPGA.
       *
       * param is frequency as a double
       *
       * @see SoDa::USRPCtrl SoDa::UI
       */
      RX_FE_FREQ,

      /**
       * Tune the 3rd LO, if the current FE frequency
       * is such that the desired frequency is between 
       * RX_FE_FREQ + 50KHz and RX_FE_FREQ + 300KHz.
       * Otherwise, promote this to an RX_TUNE_FREQ request.
       *
       * param is frequency as a double
       */
      RX_RETUNE_FREQ,

      /**
       * Tune the 3rd LO (in SoDa::USRPRX).
       *
       * param is frequency as a double
       */
      RX_LO3_FREQ,

      
      /**
       * Set the TX front end (1st LO, the 2nd IF LO), and the 3rd LO
       * in a more-or-less optimal way to position the requested
       * frequency at least 80 kHz above the target frequency. 
       *
       * param is frequency as a double
       *
       * @see SoDa::USRPCtrl
       */
      TX_TUNE_FREQ,
      
      /**
       * Same effect as TX_TUNE_FREQ 
       *
       * param is frequency as a double
       */
      TX_FE_FREQ,
      
      /**
       * Same effect as TX_TUNE_FREQ 
       *
       * param is frequency as a double
       */
      TX_RETUNE_FREQ,

      /**
       * Sample rate for RX is typically 600 KHz or better to allow
       * a reasonable span for the waterfall and periodogram.  It also
       * gets us reasonably far away from the high noise region near
       * the oscillators.
       *
       * param is a double 
       */
      RX_SAMP_RATE,

      /**
       * Sample rate for TX needs to be fast enough for reasonable FM
       * We set it to 625000 just because there doesn't seem to be much
       * utility to go any slower.
       *
       * param is a double
       */
      TX_SAMP_RATE,

      /**
       * RX ant choices are TX/RX, and RX2
       *
       * param is a string
       */
      RX_ANT,
      /**
       * Not many choices for TX ant, just TX
       *
       * param is a string 
       */
      TX_ANT, 

      /**
       * Set the RX front end attenuator/amp
       *
       * gain param is a double 0.0 < gain < 40.0
       */
      RX_RF_GAIN,
      /**
       * Set the TX final amplifier
       *
       * gain param is a double 0.0 < gain < 100.0
       */
      TX_RF_GAIN,

      /**
       * RX audio gain setting
       *
       * gain param is a double
       * actual gain factor is 10^(0.1*param - 50)
       */
      RX_AF_GAIN,
      /**
       * RX audio gain for sidetone (CW) monitor
       *
       * gain param is a double
       * actual gain factor is 10^(0.1*param - 50)
       */
      RX_AF_SIDETONE_GAIN,

      /**
       * TX mic gain
       *
       * gain param is a double
       * actual gain factor is 10^(0.1*param - 50)
       */
      TX_AF_GAIN,

      /**
       * turn transmitter on and off.
       *
       * param is integer 0 disables TX, 3 enables TX
       */
      TX_STATE,

      
      /**
       * RX IF stream processing enable -- used only by SoDaBench test equipment
       * param is integer
       *     0 disables RX processing,
       *     1 enables RX stream to UI,
       *     2 enables RX stream to UI, bypasses DDC downconverter, and puts the
       *       RX unit in "collect single buffer" mode (for spectrum analyzers)... 
       */
      RX_STATE,

      /**
       * TX Carrier Control -- send a dead carrier
       *
       * param is integer -- nonzero enables beacon mode. @see SoDa::USRPTX
       */
      TX_BEACON,

      /**
       * TX CW text control -- a buffer of up to 8 characters
       *
       * param is a string of 8 chars @see SoDa::CWTX
       */
      TX_CW_TEXT,
      /**
       * Set speed of CW generator
       *
       * param is integer in WPM @see SoDa::CWTX
       */
      TX_CW_SPEED,
      /**
       * Flush outstanding CW text strings from pending buffer
       *
       * no parameter
       */
      TX_CW_FLUSHTEXT,
      /**
       * Put a marker in the CW text stream, report its "passing"
       *
       * parameter is integer tag that will be reported  in a TX_CW_MARKER REP @see CWTX
       */
      TX_CW_MARKER,

      /**
       * Report when CW TX envelope buffer was empty (cmd enables report)
       *
       * no parameter
       */
      TX_CW_EMPTY,
      
      /**
       * Set the modulation mode for the receive chain.
       *
       * param integer -- (int) conversion of SoDa::Command::ModulationType
       *
       * @see USRPRX
       */

      RX_MODE,
      /**
       * Set the modulation mode for the transmit chain.
       *
       * param integer -- (int) conversion of SoDa::Command::ModulationType
       *
       * @see USRPTX       
       */
      TX_MODE,

      /**
       * tweak the AF chain -- filter settings
       */
      RX_BW,

      /**
       * tweak the waterfall display parameters
       * like resolution bandwidth
       */
      RBW, 

      // and spectrum start/stop limits
      SPEC_CENTER_FREQ,  ///< the center frequency (command from GUI)
      SPEC_RANGE_LOW,  ///< low spectrum frequency range
      SPEC_RANGE_HI,   ///< high spectrum frequency range
      SPEC_STEP, ///< freqency step
      SPEC_BUF_LEN,    ///< number of samples in the buffer. 

      /**
       * The master clock oscillator source
       *Reference oscilator selector
       * set to 1 for external, 0 for internal
       * rep = 1 for internal lock, 0 for unlock
       * 3 for external lock, 2 for external unlocked.
       */
      CLOCK_SOURCE,

      /**
       * This is an LO check command - use it for
       * finding the actual microwave LO frequency.
       * if the parameter is > 0, set the rx_lo to the
       * dparm arg, and remember that we're in LOcheck mode.
       */
      LO_CHECK,
      /**
       * this is a GET/REP command -- AudioRX takes FFT
       * centered around 0 and reports largest peak within
       * 50KHz.
       */
      LO_OFFSET, 
      
      RX_AF_FILTER, ///< Audio Filter

      /**
       * Report LAT and LON from GPS receiver
       *
       * params are double Latitude, Longitude
       *
       * forms: REP
       */
      GPS_LATLON,
      /**
       * Report UTC (time) from GPS receiver
       *
       * params are int HH, MM, SS
       *
       * forms: REP
       */
      GPS_UTC,

      /**
       * Report when GPS is locked.
       *
       * param is int -- 0 for unlocked, 1 for locked
       *
       * forms: REP
       */
      GPS_LOCK,

      /**
       * Report the SDR (SoDa server program) version info
       *
       * no param
       *
       * forms: REP
       */
      SDR_VERSION,

      /**
       * Initiate a debug dump
       *
       * param (int) ordinal of UnitSelector
       *
       * forms: GET
       */
      DBG_REP,

      /**
       * Report the motherboard name (the model name of the USRP)
       *
       * no param
       *
       * forms: GET, REP
       */
      HWMB_REP,
      
      /**
       * On receipt of a STOP command, all threads should exit their run loop.
       *
       * no param
       *
       * forms: SET
       */
      STOP,

      /**
       * On receipt of a TVRT_LO_ENABLE command dump a perpetual constant IF stream
       * of (1.0, 0.0) into the tx2 channel to get a steady output.
       *
       * Ignore this command unless the radio is a B210.
       *
       * forms: SET
       */
      TVRT_LO_ENABLE, 

      /**
       * On receipt of a TVRT_LO_DISABLE command, turn the LO output on TX2 off. 
       * Ignore this command unless the radio is a B210.
       *
       * no param
       *
       * forms: SET
       */
      TVRT_LO_DISABLE, 

      /**
       * On receipt of a TVRT_LO_CONFIG command , set the TX2 channel
       * frequency to dparam[0] and the TX2 output gain to dparam[1].
       *
       * Ignore this command unless the radio is a B210.
       *
       * param (double) output frequency
       * param (double) output gain setting
       *
       * forms: SET, REP
       */
      TVRT_LO_CONFIG, 
      
      /**
       * Set the RX front end (1st LO, the 2nd IF LO) ensuring that
       * the DDC LO is at least as large as the second parameter. 
       *
       * param 0 is target freq, 1 is minimum LO offset
       *
       * @see SoDa::USRPCtrl
       */
      RX_FE_BASEBAND_FREQ,

      /**
       * buffer size for RX is normally 625k for SoDaRadio.
       * Test equipment may use a different buffer size. 
       *
       * param is an unsigned int
       */
      RX_BUFFER_SIZE,

      /**
       * SpectrumAnalyzer needs to know whether it should
       * downsample the input RF stream before FFT. 
       *
       * param is an unsigned int 
       */
      SPEC_DOWNSAMP_RATE, 
      
      /**
       * RX unit can be put in "single buffer" mode (via SET RX_STATE=2) where
       * it collects and processes one buffer at a time, on
       * command from the USRPCtrl unit. This is the command to the USRPRX
       * that does that. 
       * USPRCtrl sends this command when a retune has completed (in RX_STATE=2 mode)
       * or when the remote GUI sends a TRIGGER_COLLECT command
       *
       * param is a double -- the current center frequency.
       */
      RX_COLLECT_BUFFER,

      /**
       * This is the command that a GUI sends to trigger a new collection.
       * It will only ever be processed by the USRPCtrl unit, and forwarded as
       * RX_COLLECT_BUFFER
       *
       */
      TRIGGER_COLLECT,


      /**
       * This is a report of the envelope power since the last CLEAR env power. 
       * This is created by the AudioRX thread
       * the parameter is a float that constitutes the mean squared amplitude in the
       * demodulated signal.  The second paramter is the peak value in the evenlope
       *
       */
      ENV_POWER, 

      /**
       * This is a command to reset the envelope power statistics. 
       * This is handled by the AudioRX thread
       */
      CLEAR_ENV_POWER,

      /**
       * This is a report of the thermal sensors (used in the noise figure meter)
       * This is created by the NFThermometer thread
       * the parameter is a string that is the content of the temperature report
       * from the arduino NF_Meter_Thermometer sketch.
       *
       */
      NF_THERM,

      /**
       * This command enables the spectrum analyzer for channel A -- used in SoDaBench...
       */
      SPEC_ENA_A, 

      /**
       * This command enables the spectrum analyzer for channel B where available
       * -- used in SoDaBench...
       */
      SPEC_ENA_B, 

      /**
       * Start the sweep oscillator
       * param 0 -- the start frequency (Hz)
       * param 1 -- the end frequency (Hz)
       * param 2 -- the step in Hz
       * param 3 -- the delay between steps (in microsec)
       */
      START_TX_SWEEP,

      /**
       * Stop the sweep oscillator
       */
      STOP_TX_SWEEP,

      /**
       * Do the next sweep step
       */
      STEP_TX_SWEEP, 
      
      /**
       * Won't do anything
       */
      NULL_CMD
      
    };

    /**
     * @brief modulation selector targets take one of these values
     * NF is noise figure meter
     */
    enum ModulationType { LSB, USB, CW_U, CW_L, AM, WBFM, NBFM, NF };

    /**
     * @brief these are the possible audio filter bandwidths
     */
    enum AudioFilterBW { BW_100, BW_500, BW_2000, BW_6000, BW_PASS, BW_NULL };

    /**
     * @brief a selector to identify a particular unit for debug reports
     */
    enum UnitSelector { AudioRX, AudioTX, RFRX, RFTX, CWTX, CTRL };

    /**
     * Constructor for commands with no parameters
     *
     * @param _ct the command type (SET, GET, REPort)
     * @param _tgt the state that we're setting, getting, reporting
     */
    Command(cmd_type _ct, cmd_target _tgt) 
    {
      cmd = _ct;
      target = _tgt;
      parm_type = ' ';
      id = command_sequence_number++;
    }

    /**
     * Constructor for commands with integer parameters
     *
     * @param _ct the command type (SET, GET, REPort)
     * @param _tgt the state that we're setting, getting, reporting
     * @param p0 first integer parameter
     * @param p1 second integer parameter
     * @param p2 third integer parameter
     * @param p3 fourth integer parameter
     */
    Command(cmd_type _ct, cmd_target _tgt,
	    int p0,
	    int p1 = 0,
	    int p2 = 0,
	    int p3 = 0) 
    {
      cmd = _ct;
      target = _tgt;
      iparms[0] = p0; 
      iparms[1] = p1; 
      iparms[2] = p2; 
      iparms[3] = p3; 
      parm_type = 'I';
      id = command_sequence_number++;
    }

    /**
     * Constructor for commands with double float parameters
     *
     * @param _ct the command type (SET, GET, REPort)
     * @param _tgt the state that we're setting, getting, reporting
     * @param p0 first double float parameter
     * @param p1 second double float parameter
     * @param p2 third double float parameter
     * @param p3 fourth double float parameter
     */
    Command(cmd_type _ct, cmd_target _tgt,
	    double p0,
	    double p1 = 0.0,
	    double p2 = 0.0,
	    double p3 = 0.0)
    {
      cmd = _ct;
      target = _tgt; 
      dparms[0] = p0; 
      dparms[1] = p1; 
      dparms[2] = p2; 
      dparms[3] = p3; 
      parm_type = 'D';
      id = command_sequence_number++;
    }

    /**
     * Constructor for commands with a string parameter
     *
     * @param _ct the command type (SET, GET, REPort)
     * @param _tgt the state that we're setting, getting, reporting
     * @param _str_arg the string we're passing
     */
    Command(cmd_type _ct, cmd_target _tgt, const std::string & _str_arg)
    {
      cmd = _ct;
      target = _tgt;
      const char * cp = _str_arg.c_str();
      int i;
      for(i = 0; i < 64; i++) {
	sparm[i] = *cp;
	if(*cp == '\000') break;
	cp++; 
      }
      parm_type = 'S';
      id = command_sequence_number++;
    }

    /**
     * Constructor for commands with a string parameter
     *
     * @param _ct the command type (SET, GET, REPort)
     * @param _tgt the state that we're setting, getting, reporting
     * @param cp the string we're passing
     */
    Command(cmd_type _ct, cmd_target _tgt, const char * cp)
    {
      cmd = _ct;
      target = _tgt;
      const char * ocp = cp; 
      int i;
      for(i = 0; i < 64; i++) {
	sparm[i] = *cp;
	if(*cp == '\000') break;
	cp++; 
      }
      parm_type = 'S';
      char * p = sparm; 
      id = command_sequence_number++;
    }

    /**
     * Copy Constructor
     *
     * @param cc the command we're copying
     */
    Command(const Command & cc) {
      cmd = cc.cmd;
      target = cc.target;
      switch (cc.parm_type) {
	
      case 'D':
	dparms[0] = cc.dparms[0];
	dparms[1] = cc.dparms[1];
	dparms[2] = cc.dparms[2];
	dparms[3] = cc.dparms[3];
	break; 
      case 'I':
	iparms[0] = cc.iparms[0];
	iparms[1] = cc.iparms[1];
	iparms[2] = cc.iparms[2];
	iparms[3] = cc.iparms[3];
	break;
      case 'S':
      default:
	strncpy(sparm, cc.sparm, 64);
	break; 
      }
      id = -1 * command_sequence_number++;
      parm_type = cc.parm_type; 
    }

    /**
     * Constructor -- create an empty command
     */
    Command() {
      cmd = NONE;
      target = NULL_CMD;
      parm_type = 'I';
      iparms[0] = 0;
    }

    /**
     * Destructor
     */
    ~Command() {
    }


    /**
     * @brief convert a string to a command
     * @param str the string to be parsed
     * @return a pointer to a new command
     */
    static Command * parseCommandString(std::string str);

    /**
     * @brief return a string that displays the command
     * @return the string
     */
    std::string & toString();

    /**
     * @brief how long can a string parameter to a command be?
     * @return the length of the longest string command argument
     */
    static int getMaxStringLen() { return 64; }

    union {
      int iparms[4]; ///< integer parameters
      double dparms[4]; ///< double float parameters
      char sparm[64]; ///< a buffer holding the string
    };
    cmd_type cmd; ///< the command type (SET, GET, REP)
    cmd_target target; ///< the thing we're touching

    int id; ///< a sequential ID for each command -- used in debugging and sequencing
    char parm_type; ///< is this a double, int, string? 

    static int command_sequence_number; ///< sequential ID applied to each command
    
    static bool table_needs_init; ///< if true, we need to call initTables()
    static std::map<std::string, cmd_target> target_map_s2v; ///< mapping for parseCommandString
    static std::map<cmd_target, std::string *> target_map_v2s; ///< mapping for toString
    /**
     * @brief setup maps to support parseCommandString and toString
     */
    static void initTables(); 
  };

  /**
   * This is a generic event queue class for commands.  
   */
  template <typename T> class CommandQueue {
  public:
    typedef std::pair<T, Command *> QEntry;
    
    CommandQueue() {};
    
    void put(Command * cmd, T time) {
      cmd_queue.insert(QEntry(time, cmd));
    }

    Command * getNext(T tim = T(0)) {
      if(cmd_queue.empty()) return NULL; 
      QEntry n = (*cmd_queue.begin());
      if(n.first > tim) return NULL; 
      else {
	cmd_queue.erase(cmd_queue.begin());
	return n.second;
      }
    }

  private:
    std::multiset<QEntry> cmd_queue; 
  }; 
}


#endif
