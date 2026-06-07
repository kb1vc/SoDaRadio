#pragma once
/*
  Copyright (c) 2012,2017, 2025, 2026 Matthew H. Reilly (kb1vc)
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

#include <string>
#include <memory>
#include <string.h>

#include <memory>
#include <SoDa/MailBox.hxx>

namespace SoDa
{

  class Command;
  typedef std::shared_ptr<Command> CommandPtr;
  
  /** This is a list of all the commands that can "do something"
   * to one or more components in the SoDa radio.
   * The base class defines all the command types in an
   * enum.  Each command can take up to 4 double and 4 integer
   * parameters.  (This isn't very OO, but the intent is to
   * keep the command packet format simple, stable, and
   * universal.)
   *
   */
  class Command {
  public:
    /**
     * @brief Commands are of the form, SET, GET, or REPort some
     * parameter.
     */
    enum CmdType
      {
	SET,
	GET,
	REP,
	NONE
      };

    /**
     * @brief some radios support two sources for the master oscillator reference.
     * (chosen by set/get ClockSource)
     */
    enum ClockSource
      {
	EXTERNAL,
	INTERNAL
      }; 
    /**
     * @brief Transmit/Receive state changes go through stages. See TX_STATE
     */
    enum RxTxState
      {
	TX_OFF_0,
	TX_OFF_1,
	TX_OFF_2,
	TX_ON_0,
	TX_ON_1,
	TX_ON_2, 	
      };
    /**
     * @brief Each command has a "target" state that it is meant to modify, query, or report
     */
    enum CmdTarget
      {
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
	 * Tune the RX hardware front end local oscillator 
	 *
	 * param is frequency as a double
	 */
	RX_LO_FREQ,


	/**
	 * Tune the software NCO (final LO) frequency to shift to baseband
	 *
	 * param is frequency as a double
	 */
	RX_IF_FREQ,
	
	/**
	 * The center frequency for IF buffers from USRPRX
	 *
	 * param is frequency as a double
	 */
	RX_CENTER_FREQ,
      
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
	 * Tune the RX hardware front end local oscillator 
	 *
	 * param is frequency as a double
	 */
	TX_LO_FREQ,
	
	/**
	 * Set/report the TX IF frequency 
	 *
	 * param is frequency as a double (normally 0)
	 */
	TX_IF_FREQ,

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
	 * gain param is a double less than 0 relative to max RX gain
	 */
	RX_RF_GAIN,
	/**
	 * Set the TX final amplifier
	 *
	 * gain param is a double less than 0 relative to max TX gain
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
	 * param 1 is integer from the RxTxState enum
	 * param 2 is integer 0 is half-duplex, 1 full-duplex
	 * (In full duplex mode, the state of the RX chain is
	 * unchanged when the transmitter is enabled.)
	 */
	TX_STATE,

	/**
	 * Ignored for now
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
	 * REP -- iparam[0] is the character count for the __last__ character
	 * dropped from the buffer. 
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
	SPEC_CENTER_FREQ, ///< the center frequency (command from GUI)
	SPEC_RANGE_LOW,   ///< low spectrum frequency range
	SPEC_RANGE_HI,    ///< high spectrum frequency range
	SPEC_STEP,        ///< freqency step
	SPEC_BUF_LEN,     ///< number of samples in the buffer.

	SPEC_DIMS, ///< all spec info in one call, cf, span, and buflen

	SPEC_AVG_WINDOW,  ///< how many FFT samples contribute to a spectrum report
	SPEC_UPDATE_RATE, ///< how many FFT samples between spectrum reports

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
	 * this is a GET/REP command -- BaseBandRX takes FFT
	 * centered around 0 and reports largest peak within
	 * 50KHz.
	 */
	LO_OFFSET,

	RX_AF_FILTER, ///< Audio Filter

	RX_AF_FILTER_SHAPE, ///< Audio Filter

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
	 * string param
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
	 * rep -- string param
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
	 * The STATUS_MESSAGE carries a payload of up to 64 characters.
	 * These will be displayed in a log window for the GUI.
	 *
	 * param char[64]
	 *
	 * forms: REP
	 */
	STATUS_MESSAGE,

	/**
	 * Select the transmit chain audio input (for SSB, AM, and FM)
	 */
	TX_AUDIO_IN,

	/**
	 * Enable the TX audio bandpass filter (limit to 2.5 kHz) for SSB/AM/FM
	 */
	TX_AUDIO_FILT_ENA,

	/** 
	 * Report min max RX Gain setting (dparm[0,1] = min, max)
	 */
	RX_GAIN_RANGE,

	/** 
	 * Report min max TX Gain setting (dparm[0,1] = min, max)
	 */
	TX_GAIN_RANGE,

	/** 
	 * Report RX antenna choice (asciiz string, uint tag)
	 */
	RX_ANT_NAME,

	/** 
	 * Report TX antenna choice (asciiz string, uint tag)
	 */
	TX_ANT_NAME,

	/**
	 * Report a string/name pair for modulation mode
	 */
	MOD_SEL_ENTRY,

	/**
	 * Report a string/name pair for AF filter bandwidth
	 */
	AF_FILT_ENTRY,

	/**
	 * indicate to GUI that we've sent all the initial configuration information
	 */
	INIT_SETUP_COMPLETE,

	/**
	 * send character count from start-of-time each time we send a 
	 * character. sparm[0] is the sent character, tag is count from start 
	 */
	CW_CHAR_SENT,

	/**
	 * Start recording raw IF stream to file
	 */
	RF_RECORD_START,

	/**
	 * Stop recording raw IF stream to file
	 */
	RF_RECORD_STOP,

	/**
	 * Set/Get NBMF squelch level
	 */
	NBFM_SQUELCH,

	/**
	 * Get a report of supported modes from the baseband rx module
	 */
	LIST_MODES,

	/**
	 * Get a report of supported filters from the baseband rx module
	 */
	LIST_AF_FILTERS,
	
	/**
	 * No comment
	 */
	NULL_CMD
      };

    /**
     * @brief modulation selector targets take one of these values
     */
    enum ModulationType
      {
	LSB,
	USB,
	CW_U,
	CW_L,
	AM,
	WBFM,
	NBFM
      };

    /**
     * @brief these are the possible audio filter bandwidths
     */
    enum AudioFilterBW
      {
	BW_100,
	BW_500,
	BW_2000,
	BW_6000,
	BW_PASS,
	BW_WSPR,
	BW_NULL
      };

    /**
     * @brief a selector to identify a particular unit for debug reports
     */
    enum UnitSelector
      {
	BaseBandRX,
	BaseBandTX,
	RFRX,
	RFTX,
	CWTX,
	CTRL
      };

    /**
     * @brief a selector to identify the Audio TX input (MIC, NOISE...)
     */
    enum TXAudioSelector
      {
	MIC,
	NOISE
      };


  public:
    
    
    /**
     * Constructor for commands with no parameters
     *
     * @param _ct the command type (SET, GET, REPort)
     * @param _tgt the state that we're setting, getting, reporting
     */
    Command(CmdType _ct, CmdTarget _tgt)
    {
      cmd = _ct;
      target = _tgt;
      parm_type = ' ';
      id = command_sequence_number++;
    }

    static CommandPtr make(CmdType _ct, CmdTarget _tgt) {
      return std::make_shared<Command>(_ct, _tgt);
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
    Command(CmdType _ct, CmdTarget _tgt,
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

    static CommandPtr make(CmdType _ct, CmdTarget _tgt,
			   int p0,
			   int p1 = 0,
			   int p2 = 0,
			   int p3 = 0)
    {
      return std::make_shared<Command>(_ct, _tgt, p0, p1, p2, p3);
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
    Command(CmdType _ct, CmdTarget _tgt,
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

    static CommandPtr make(CmdType _ct, CmdTarget _tgt,
			   double p0,
			   double p1 = 0.0,
			   double p2 = 0.0,
			   double p3 = 0.0) {
      return std::make_shared<Command>(_ct, _tgt, p0, p1, p2, p3);
    }
    
    /**
     * Constructor for commands with a string parameter
     *
     * @param _ct the command type (SET, GET, REPort)
     * @param _tgt the state that we're setting, getting, reporting
     * @param _str_arg the string we're passing
     * @param _tag an integer tag to associate with the string.
     */
    Command(CmdType _ct, CmdTarget _tgt, const std::string &_str_arg, unsigned int _tag = 0)
    {
      cmd = _ct;
      target = _tgt;
      tag = _tag;
      const char *cp = _str_arg.c_str();
      int i;
      for (i = 0; i < 64; i++)
	{
	  sparm[i] = *cp;
	  if (*cp == '\000')
	    break;
	  cp++;
	}
      parm_type = 'S';
      id = command_sequence_number++;
    }

    static CommandPtr make(CmdType _ct, CmdTarget _tgt, 
			   const std::string &_str_arg, unsigned int _tag = 0)
    {
      return std::make_shared<Command>(_ct, _tgt, _str_arg, _tag);
    }
    
    /**
     * Constructor for commands with a string parameter
     *
     * @param _ct the command type (SET, GET, REPort)
     * @param _tgt the state that we're setting, getting, reporting
     * @param cp the asciiz string we're passing
     * @param _tag an integer tag to associate with the string.
     */
    Command(CmdType _ct, CmdTarget _tgt, const char *cp, unsigned int _tag = 0)
    {
      cmd = _ct;
      target = _tgt;
      tag = _tag;
      int i;
      for (i = 0; i < 64; i++)
	{
	  sparm[i] = *cp;
	  if (*cp == '\000')
	    break;
	  cp++;
	}
      parm_type = 'S';
      id = command_sequence_number++;
    }

    static CommandPtr make(CmdType _ct, CmdTarget _tgt, const char *cp, unsigned int _tag = 0) {
      return std::make_shared<Command>(_ct, _tgt, cp, _tag);
    }
    
    /**
     * Copy Constructor
     *
     * @param cc the command we're copying
     */
    Command(const Command &cc)
    {
      cmd = cc.cmd;
      target = cc.target;
      strncpy(sparm, cc.sparm, 64);
      dparms[0] = cc.dparms[0];
      dparms[1] = cc.dparms[1];
      dparms[2] = cc.dparms[2];
      dparms[3] = cc.dparms[3];
      iparms[0] = cc.iparms[0];
      iparms[1] = cc.iparms[1];
      iparms[2] = cc.iparms[2];
      iparms[3] = cc.iparms[3];
      id = -1 * command_sequence_number++;
      parm_type = cc.parm_type;
    }

    /**
     * Constructor -- create an empty command
     */
    Command()
    {
      cmd = NONE;
      target = NULL_CMD;
      parm_type = 'I';
      iparms[0] = 0;
      tag = 0;
    }

    static CommandPtr make() {
      return std::make_shared<Command>();
    }
    
    /**
     * Destructor
     */
    ~Command()
    {
    }

    /**
     * @brief convert a string to a command
     * @param str the string to be parsed
     * @return a pointer to a new command
     */
    static Command *parseCommandString(std::string str);

    /**
     * @brief return a string that displays the command
     * @return the string
     */
    std::string toString() const;

    /**
     * @brief how long can a string parameter to a command be?
     * @return the length of the longest string command argument
     */
    static int getMaxStringLen() { return 64; }

    unsigned int tag; ///< used to pair an int with a string or other param.
    union {
      int iparms[4];    ///< integer parameters
      double dparms[4]; ///< double float parameters
      char sparm[64];   ///< a buffer holding the string
    };
    CmdType cmd;      ///< the command type (SET, GET, REP)
    CmdTarget target; ///< the thing we're touching

    int id;         ///< a sequential ID for each command -- used in debugging and sequencing
    char parm_type; ///< is this a double, int, string?

    static int command_sequence_number; ///< sequential ID applied to each command

    static bool table_needs_init;                           ///< if true, we need to call initTables()
    static std::map<std::string, CmdTarget> target_map_s2v; ///< mapping for parseCommandString
    static std::map<CmdTarget, std::string> target_map_v2s; ///< mapping for toString
    /**
     * @brief setup maps to support parseCommandString and toString
     */
    static void initTables();

  private:
    static void initTableEntry(const std::string &, CmdTarget tgt);
  };

  template<Command::CmdTarget Targ, int numargs> struct DoubleCommand : public Command {
    template<typename... Args>
    DoubleCommand(CmdType sgr, Args... dvs) : Command(sgr, Targ) {
      static_assert(sizeof...(dvs) == numargs);
      static_assert(numargs <= 4);
      std::vector<double> vals = { static_cast<double>(dvs)... };
      for(int i = 0; i < numargs; i++) {
	dparms[i] = (i >= numargs) ? 0.0 : vals[i];
      }
      parm_type = 'D';
    }
    
    template<typename... Args>
    static CommandPtr make(CmdType sgr, Args... dvs) {
      return std::make_shared<DoubleCommand<Targ, numargs>>(sgr, dvs...);
    }
  };

  template<Command::CmdTarget Targ, int numargs> struct IntCommand : public Command {
    template<typename... Args>
    IntCommand(CmdType sgr, Args... ivs) : Command(sgr, Targ) {
      static_assert(sizeof...(ivs) == numargs);
      static_assert(numargs <= 4);
      std::vector<int> vals = { static_cast<int>(ivs)... };
      for(int i = 0; i < numargs; i++) {
	iparms[i] = vals[i];
      }
      parm_type = 'I';
    }

    template<typename... Args>
    static CommandPtr make(CmdType sgr, Args... ivs) {
      return std::make_shared<IntCommand<Targ, numargs>>(sgr, ivs...);
    }
  };

  template<Command::CmdTarget Targ, typename enum_type> struct EnumCommand : public Command {
    EnumCommand(CmdType sgr, enum_type ev) : Command(sgr, Targ) {
      iparms[0] = static_cast<int>(ev);
      iparms[1] = iparms[2] = iparms[3] = 0;
      parm_type = 'E';
      std::string name = enumToString(ev);
      ::strncpy(sparm + 4, name.c_str(), 59);
      sparm[63] = '\0';
    }

    EnumCommand(CmdType sgr) : Command(sgr, Targ) {
      iparms[0] = iparms[1] = iparms[2] = iparms[3] = 0;      
    }
    
    static CommandPtr make(CmdType sgr, enum_type ev) {
      return std::make_shared<EnumCommand<Targ, enum_type>>(sgr, ev);
    }

    static CommandPtr make(CmdType sgr) {
      return std::make_shared<EnumCommand<Targ, enum_type>>(sgr);
    }

    std::string getEnumStr() {
      return enumToString(static_cast<enum_type>(iparms[0]));
    }
    
  };

  template<Command::CmdTarget Targ> struct StringCommand : public Command {
    StringCommand(CmdType sgr, const std::string & s, unsigned int tag = 0)
      : Command(sgr, Targ, s, tag) {}
    StringCommand(CmdType sgr) : Command(sgr, Targ) {}
    static CommandPtr make(CmdType sgr, const std::string & s, unsigned int tag = 0) {
      return std::make_shared<StringCommand<Targ>>(sgr, s, tag);
    }
    static CommandPtr make(CmdType sgr) {
      return std::make_shared<StringCommand<Targ>>(sgr);
    }
  };

  typedef DoubleCommand<Command::RX_TUNE_FREQ, 1> CmdRXTuneFreq;
  typedef DoubleCommand<Command::RX_LO_FREQ, 1> CmdRXLOFreq;
  typedef DoubleCommand<Command::RX_IF_FREQ, 1> CmdRXIFFreq;
  typedef DoubleCommand<Command::RX_CENTER_FREQ, 1> CmdRXCenterFreq;
  
  typedef DoubleCommand<Command::TX_TUNE_FREQ, 1> CmdTXTuneFreq;
  typedef DoubleCommand<Command::TX_LO_FREQ, 1> CmdTXLOFreq;
  typedef DoubleCommand<Command::TX_IF_FREQ, 1> CmdTXIFFreq;

  typedef EnumCommand<Command::RX_MODE, Command::ModulationType> CmdRXMode;
  typedef EnumCommand<Command::TX_MODE, Command::ModulationType> CmdTXMode;
  typedef EnumCommand<Command::TX_STATE, Command::RxTxState>     CmdTXState;
  typedef EnumCommand<Command::CLOCK_SOURCE, Command::ClockSource> CmdClockSource;
  typedef EnumCommand<Command::TX_AUDIO_IN, Command::TXAudioSelector> CmdTXAudioIn;

  typedef IntCommand<Command::TX_BEACON,       1> CmdTXBeacon;
  typedef IntCommand<Command::TX_CW_SPEED,     1> CmdTXCWSpeed;
  typedef IntCommand<Command::TX_CW_MARKER,    1> CmdTXCWMarker;
  typedef IntCommand<Command::TX_CW_FLUSHTEXT, 1> CmdTXCWFlushText;
  typedef IntCommand<Command::TX_CW_EMPTY,     1> CmdTXCWEmpty;
  typedef IntCommand<Command::TX_AUDIO_FILT_ENA, 1> CmdTXAudioFiltEna;
  typedef IntCommand<Command::RX_AF_FILTER,    1> CmdRXAFFilter;
  typedef IntCommand<Command::RX_AF_FILTER_SHAPE, 1> CmdRXAFFilterShape;
  typedef IntCommand<Command::RX_BW,           1> CmdRXBW;
  typedef IntCommand<Command::SPEC_BUF_LEN,    1> CmdSpecBufLen;
  typedef IntCommand<Command::SPEC_AVG_WINDOW, 1> CmdSpecAvgWindow;
  typedef IntCommand<Command::SPEC_UPDATE_RATE,1> CmdSpecUpdateRate;
  typedef IntCommand<Command::GPS_LOCK,        1> CmdGPSLock;
  typedef IntCommand<Command::DBG_REP,         1> CmdDbgRep;
  typedef IntCommand<Command::GPS_UTC,         3> CmdGPSUTC;

  typedef StringCommand<Command::RX_ANT>          CmdRXAnt;
  typedef StringCommand<Command::TX_ANT>          CmdTXAnt;
  typedef StringCommand<Command::TX_CW_TEXT>      CmdTXCWText;
  typedef StringCommand<Command::SDR_VERSION>     CmdSDRVersion;
  typedef StringCommand<Command::HWMB_REP>        CmdHWMBRep;
  typedef StringCommand<Command::STATUS_MESSAGE>  CmdStatusMessage;
  typedef StringCommand<Command::RF_RECORD_START> CmdRFRecordStart;
  typedef StringCommand<Command::RX_ANT_NAME>     CmdRXAntName;
  typedef StringCommand<Command::TX_ANT_NAME>     CmdTXAntName;
  typedef StringCommand<Command::MOD_SEL_ENTRY>   CmdModSelEntry;
  typedef StringCommand<Command::AF_FILT_ENTRY>   CmdAFFiltEntry;
  typedef StringCommand<Command::CW_CHAR_SENT>    CmdCWCharSent;

inline std::string enumToString(Command::CmdType v) {
  switch(v) {
  case Command::SET:  return "SET";
  case Command::GET:  return "GET";
  case Command::REP:  return "REP";
  case Command::NONE: return "NONE";
  default:            return "CmdType(" + std::to_string((int)v) + ")";
  }
}

inline std::string enumToString(Command::ClockSource v) {
  switch(v) {
  case Command::EXTERNAL: return "EXTERNAL";
  case Command::INTERNAL: return "INTERNAL";
  default:                return "ClockSource(" + std::to_string((int)v) + ")";
  }
}

inline std::string enumToString(Command::RxTxState v) {
  switch(v) {
  case Command::TX_OFF_0: return "TX_OFF_0";
  case Command::TX_OFF_1: return "TX_OFF_1";
  case Command::TX_OFF_2: return "TX_OFF_2";
  case Command::TX_ON_0:  return "TX_ON_0";
  case Command::TX_ON_1:  return "TX_ON_1";
  case Command::TX_ON_2:  return "TX_ON_2";
  default:                return "RxTxState(" + std::to_string((int)v) + ")";
  }
}

inline std::string enumToString(Command::ModulationType v) {
  switch(v) {
  case Command::LSB:  return "LSB";
  case Command::USB:  return "USB";
  case Command::CW_U: return "CW_U";
  case Command::CW_L: return "CW_L";
  case Command::AM:   return "AM";
  case Command::WBFM: return "WBFM";
  case Command::NBFM: return "NBFM";
  default:            return "ModulationType(" + std::to_string((int)v) + ")";
  }
}

inline std::string enumToString(Command::AudioFilterBW v) {
  switch(v) {
  case Command::BW_100:  return "BW_100";
  case Command::BW_500:  return "BW_500";
  case Command::BW_2000: return "BW_2000";
  case Command::BW_6000: return "BW_6000";
  case Command::BW_PASS: return "BW_PASS";
  case Command::BW_WSPR: return "BW_WSPR";
  case Command::BW_NULL: return "BW_NULL";
  default:               return "AudioFilterBW(" + std::to_string((int)v) + ")";
  }
}

inline std::string enumToString(Command::UnitSelector v) {
  switch(v) {
  case Command::BaseBandRX: return "BaseBandRX";
  case Command::BaseBandTX: return "BaseBandTX";
  case Command::RFRX:       return "RFRX";
  case Command::RFTX:       return "RFTX";
  case Command::CWTX:       return "CWTX";
  case Command::CTRL:       return "CTRL";
  default:                  return "UnitSelector(" + std::to_string((int)v) + ")";
  }
}

inline std::string enumToString(Command::TXAudioSelector v) {
  switch(v) {
  case Command::MIC:   return "MIC";
  case Command::NOISE: return "NOISE";
  default:             return "TXAudioSelector(" + std::to_string((int)v) + ")";
  }
}

} // namespace SoDa

