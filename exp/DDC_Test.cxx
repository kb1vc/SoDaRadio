/*
  Copyright (c) 2014, Matthew H. Reilly (kb1vc)
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

/**
 * @file DDC_Test.cxx
 *
 * @brief The MAIN process that creates and supervises all the threads that make
 * up an experiment that looks for birdies in the DDC oscillator. 
 *
 * @author Matt Reilly (kb1vc)
 */
#include <uhd/usrp/multi_usrp.hpp>

#include "SoDaBase.hxx"
#include "MultiMBox.hxx"

// the radio parts. 
#include "Params.hxx"
#include "USRPCtrl.hxx"
#include "USRPRX.hxx"
#include "Command.hxx"
#include "Debug.hxx"
#include "Spectrogram.hxx"
#include <fstream>
#include <math.h>

#define BINARY_MODE_OUTPUT 1

namespace SoDa {
  class ExpParams : public Params {
  public:
    ExpParams(int argc, char * argv[]) : Params() {
      desc->add_options()
	("lo_freq", po::value<double>(&lo_base_freq)->default_value(157.0e6),
	 "Local Oscillator Frequency")
	("file", po::value<std::string>(&out_filename)->default_value("sweep.dat"),
	 "Output filename");
      
      parseCommandLine(argc, argv); 
    }

    double getLOFreq() { return lo_base_freq; }
    std::string getOutFilename() { return out_filename; }
    
  private:
    double lo_base_freq;
    std::string out_filename; 
  };
  
  class DDCExp : public SoDaThread {
  public:
    /**
     * constructor
     * @param params a pointer to a params object that will tell us
     *        about sample rates and other configuration details.
     * @param usrp a pointer to the UHD USRP object that we are streaming data f
rom.
     * @param _if_stream data mailbox used to pass the IF2 data to the spectrum 
units
     * @param _cmd_stream data mailbox used to carry command, query, and report 
messages
     */
    DDCExp(ExpParams * params,
	   DatMBox * _if_stream,
	   CmdMBox * _cmd_stream) : SoDa::SoDaThread("DDCExp") {

      if_stream = _if_stream;
      if_subs = if_stream->subscribe();
     
      cmd_stream = _cmd_stream;
      cmd_subs = cmd_stream->subscribe();

      got_new_ddc_freq = false;

      cur_fe_freq = params->getLOFreq();

      spectrogram_buckets = 16 * 1024;
      spect = new Spectrogram(spectrogram_buckets);

      //      spectrum_buf = new float[8*spectrogram_buckets];
      //      spectrum_buf = spectrum_buf + 4 * spectrogram_buckets;
      spectrum_buf = new float[spectrogram_buckets];

      for(int i = 0; i < spectrogram_buckets; i++) {
	spectrum_buf[i] = 0.0; 
      }

      std::string datfile = params->getOutFilename() + std::string(".dat"); 
      std::string binfile = params->getOutFilename() + std::string(".bin"); 
      ofb.open(binfile.c_str(), std::ios::out | std::ios::binary);
      of.open(datfile.c_str(), std::ios::out);


      collect_count_limit = 72;
      ignore_count = 8;
      spectrum_acc_gain = 1.0 - (1.0 / ((float) (collect_count_limit - ignore_count)));
      
      writeHeader();
    }

    void run() {
      bool exitflag = false;

      usleep(5 * 1e6);

      // setup the intial freq
      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_FE_FREQ, cur_fe_freq));
      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_RF_GAIN, 100.0));

      usleep(1e6);
      // enable the receiver. 
      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_STATE, 1));


      enum {INIT, WAIT_FOR_FREQ, COLLECT, WAIT_FOR_RXEND } curstate;
      curstate = INIT;

      SoDaBuf * ifbuf; // this is the if buffer. 

      cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_DDC_FREQ, 1.0));
      
      while(!exitflag) {
	Command * cmd;
	// process all the incoming commands
	while((cmd = cmd_stream->get(cmd_subs)) != NULL) {
	  execCommand(cmd);
	  exitflag |= (cmd->target == Command::STOP);
	  debugMsg(boost::format("Got command: %s\n") % cmd->toString());
	  cmd_stream->free(cmd);
	}

	// now do the state machine
	switch (curstate) {
	case INIT:
	  if(got_new_ddc_freq) {
	    ddc_freq_test = 0.0;
	    got_new_ddc_freq = false; 
	    cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_DDC_FREQ, ddc_freq_test));
	    curstate = WAIT_FOR_FREQ;
	    debugMsg("curstate = WAIT_FOR_FREQ\n");
	  }
	  break;

	case WAIT_FOR_FREQ:
	  if(got_new_ddc_freq) {
	    got_new_ddc_freq = false;
	    collect_count = 0; 
	    curstate = COLLECT;
	    cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_STATE, 1));	    
	    debugMsg("curstate = COLLECT\n");
	  }
	  break;

	case COLLECT:
	  if(collect_count == collect_count_limit) {
	    // turn off the receiver.
	    cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_STATE, 0));
	    curstate = WAIT_FOR_RXEND;
	    debugMsg("curstate = WAIT_FOR_RXEND\n");
	  }
	  if((ifbuf = if_stream->get(if_subs)) != NULL) {
	    processBuffer(ifbuf, collect_count);
	    collect_count++; 	  
	    if_stream->free(ifbuf); 
	  }
	  else {
	    usleep(100);
	  }
	  break;

	case WAIT_FOR_RXEND:
	  if(!rx_enabled) {
	    // drain the if stream.
	    while((ifbuf = if_stream->get(if_subs)) != NULL) {
	      if_stream->free(ifbuf);
	    }
	    // now dump the buffer.
	    writeResultDB();
	    writeSummaryResult();
	    // set a new frequency
	    if(ddc_freq_test >  -10.0) {
	      ddc_freq_test -= 1.0;
	    }
	    else if(ddc_freq_test > -100.0) {
	      ddc_freq_test -= 10.0;
	    }
	    else if(ddc_freq_test > -0.1e6) {
	      ddc_freq_test -= 10.0;	      
	    }
	    else if(ddc_freq_test > -1.0e6) {
	      ddc_freq_test -= 1000.0;	      
	    }
	    else {
	      // do nothing -- we're on our way out.
	      exitflag = true; 
	    }

	    debugMsg(boost::format("setting new ddcfreq %g\n") % ddc_freq_test);
	    cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_DDC_FREQ, ddc_freq_test));

	    // new state waits for freq confirmation
	    curstate = WAIT_FOR_FREQ; 
	    debugMsg("curstate = WAIT_FOR_FREQ\n");
	  }
	  break; 
	}
      }

      of.close();
      ofb.close();

      cmd_stream->put(new SoDa::Command(Command::SET, Command::STOP));
    }

    void processBuffer(SoDaBuf * ifbuf, unsigned int count) {
      if(count < ignore_count) return; 
      std::complex<float> * cbuf = ifbuf->getComplexBuf();

      spect->apply_acc(ifbuf->getComplexBuf(), ifbuf->getComplexLen(),
		       spectrum_buf, (count == ignore_count) ? 0.0 : spectrum_acc_gain);
    }

    void writeResultDB() {
      // write out everything we know..
      // The format is 1stLO freq <double>, ddc freq <double>, num_points <int>, vector of floats
      ofb.write((char*) &cur_fe_freq, sizeof(double));
      ofb.write((char*) &cur_ddc_freq, sizeof(double));
      ofb.write((char*) &spectrogram_buckets, sizeof(int));
      ofb.write((char*) spectrum_buf, sizeof(float) * spectrogram_buckets);
      ofb.flush();
    }
    
    void writeSummaryResult() {
      // write the frequency, the floor (in dB), the DC magnitude (in dB) 
      // the count of the number of spurs > 1dB above the floor
      // and the next highest non DC spur. (at least 4 buckets from
      // the midband point.
      // first, calculate the mean power. use the harmonic mean
      int i;

      float invsum = 0.0;
      for(i = 0; i < spectrogram_buckets; i++) {
	invsum += 1.0 / spectrum_buf[i]; 
      }
      float mean = ((float) spectrogram_buckets) / invsum;
      
      float high_spur_freq = 0.0;
      int high_spur_idx = 0;
      float high_spur = 0.0;
      int spur_count = 0;
      float threshold = 1.25 * mean;
      int half = spectrogram_buckets / 2;
      for(i = 0; i < spectrogram_buckets; i++) {
	if((i < (half - 4)) || (i > (half + 4))) {
	  if(spectrum_buf[i] > threshold) {
	    spur_count++; 
	  }

	  if(spectrum_buf[i] > high_spur) {
	    high_spur = spectrum_buf[i];
	    high_spur_idx = i; 
	  }
	}
      }


      high_spur_freq = ((float)((high_spur_idx - half) * 625000)) / ((float)spectrogram_buckets);
      boost::format vout = boost::format("%g %g %g %d %g %g %d   %g %g\n") % ddc_freq_test %
	(10.0 * log10(mean)) %
	(10.0 * log10(spectrum_buf[spectrogram_buckets/2])) %
	spur_count %
	(10.0 * log10(high_spur)) %
	high_spur_freq %
	high_spur_idx %
	cur_ddc_freq %
	cur_fe_freq; 
      of << vout;
      of.flush();
      std::cout << vout;
    }
    
    void writeSpectResult() {
      // now we dump the buffer....
      // first, calculate the mean power. use the harmonic mean
      float invsum = 0.0;
      int i;
      for(i = 0; i < spectrogram_buckets; i++) {
	invsum += 1.0 / spectrum_buf[i]; 
      }
      float hmean = ((float) spectrogram_buckets) / invsum;

      std::cerr << boost::format("%g %g\n") % ddc_freq_test % hmean;

      float ddc = ddc_freq_test; 
#if 0      
      of.write((char*) &ddc, sizeof(float));

      int s = sizeof(float) * (spectrogram_buckets); 
      of.write((char*) spectrum_buf, s);
#else
      for(i = 0; i < spectrogram_buckets; i++) {
	of << " " << spectrum_buf[i]; 
      }
      of << std::endl; 
#endif

      of.flush();
    }

    void writeHeader() {
#if 1
      of << "# ddc_freq avg_amp (dB) DC_mag spur_count high_spur\n";
#else      
      float Np1 = (float) (spectrogram_buckets + 1);

      float yv;
      of.write((char*) &Np1, sizeof(Np1));

      
      for(int i = 0; i < spectrogram_buckets; i++) {
	float fi = i;
	of.write((char*) &fi, sizeof(fi)); 
      }
#endif
    }
    
    /// Dispatch an incoming REPort command
    /// @param cmd a command record
    void execRepCommand(Command * cmd) {
      switch (cmd->target) {
      case Command::RX_DDC_FREQ:
	got_new_ddc_freq = true;
	cur_fe_freq = cmd->dparms[0];
	cur_ddc_freq = cmd->dparms[1];
	debugMsg(boost::format("DDC report fe = %g  ddc = %g\n") % cur_fe_freq % cur_ddc_freq);
	break;
      case Command::RX_STATE:
	if(cmd->iparms[0] == 1) {
	  // RX is on... 
	  rx_enabled = true;
	  debugMsg("REP RX on\n");
	}
	else if(cmd->iparms[0] == 0) {
	  // RX is off...
	  rx_enabled = false; 
	  debugMsg("REP RX off\n");
	}
      default:
	break;
      }
    }

  private:
    DatMBox * if_stream;
    CmdMBox * cmd_stream;
    unsigned int if_subs;
    unsigned int cmd_subs;

    Spectrogram * spect;
    unsigned int spectrogram_buckets;
    float * spectrum_buf;

    bool got_new_ddc_freq;

    bool rx_enabled; 

    double cur_fe_freq;
    double cur_ddc_freq;

    double ddc_freq_test;

    int collect_count;
    int collect_count_limit; /// number of buffers to accumulate
    int ignore_count;
    float spectrum_acc_gain; 

    std::ofstream of, ofb;

  };
}

/// do the work of creating the SoDa threads
/// @param argc number of command line arguments
/// @param argv command line arguments
int doWork(int argc, char * argv[])
{
  /// create the components of the radio

  /// the command line parameter list is used to
  /// create a param object that holds configuration
  /// information from the command line and from
  /// the stored configuration files.
  /// @see SoDa::Params
  SoDa::ExpParams params(argc, argv);

  SoDa::Debug d(params.getDebugLevel(), "DDC_Test");
  d.setDefaultLevel(params.getDebugLevel());
  
  // These are the mailboxes that connect
  // the various widgets
  // the rx and tx streams are vectors of complex floats.
  // we don't declare the extent here, as it will be set
  // by a negotiation.  
  SoDa::DatMBox rx_stream(true, std::string("RXStream")), if_stream(true, std::string("IFStream"));
  SoDa::CmdMBox cmd_stream(false);


  /// doWork creates the USRP Control, RX Streamer, and analysis threads
  /// @see SoDa::USRPCtrl @see SoDa::USRPRX @see SoDa::USRPTX
  SoDa::USRPCtrl ctrl(&params, &cmd_stream);
  SoDa::USRPRX rx(&params, ctrl.getUSRP(), &rx_stream, &if_stream, &cmd_stream);
  SoDa::DDCExp exp(&params, &if_stream, &cmd_stream);

  d.debugMsg("Created units.");
  
  // Now start each of the activities -- they may or may not
  // implement the "start" method -- not all objects need to be threads.

  ctrl.start();
  rx.start();
  exp.start();

  // now the gps...
  d.debugMsg("Starting gps");

  // wait for the user interface to tell us that it is time to quit.
  ctrl.join();
  rx.join();
  exp.join();
  d.debugMsg("Exit");
  
  // when we get here, we are done... (UI should not return until it gets an "exit/quit" command.)

  return 0; 
}

/// main entrypoint
///
/// Start the SoDa USRP radio server threads
/// @param argc number of command line arguments
/// @param argv command line arguments @see doWork
int main(int argc, char * argv[])
{
  try {
    return doWork(argc, argv); 
  }
  catch (SoDa::SoDaException * exc) {
    std::cerr << "Exception caught at SoDa main: " << std::endl;
    std::cerr << "\t" << exc->toString() << std::endl; 
  }
}
