/*
  Copyright (c) 2018,2022 Matthew H. Reilly (kb1vc)
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

#include "IFRecorder.hxx"
#include <SoDa/OSFilter.hxx>
#include "MailBoxRegistry.hxx"
#include "Radio.hxx"

#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SoDa/Format.hxx>

namespace SoDa {

  IFRecorder::IFRecorder(Params_p params) : Thread("IFRecorder")
  {
    // setup the streams
    rx_stream = NULL;
    cmd_stream = NULL;

    // what is the default sample rate and buffer size?
    rf_sample_rate = params->getRXRate(); 
    rf_buffer_size = params->getRXRFBufferSize();

    // we aren't recording right now
    write_stream_on = false; 

    // we don't know the current center frequency
    current_rx_center_freq = 0.0; 
  }



  void IFRecorder::execSetCommand(Command * cmd)
  {
    Command::AudioFilterBW fbw;
    Command::ModulationType txmod; 
    switch (cmd->target) {
    case Command::RF_RECORD_START:
      openOutStream(cmd->sparm);
      break;
    case Command::RF_RECORD_STOP:
      closeOutStream();
      break; 
    default:
      break; 
    }
  }

  void IFRecorder::execGetCommand(Command * cmd)
  {
    (void) cmd;
  }

  void IFRecorder::execRepCommand(Command * cmd)
  {
    switch (cmd->target) {
    case Command::RX_FE_FREQ:
      current_rx_center_freq = cmd->dparms[0];
      break;
    default:
      // do nothing. 
      break; 
    }
  }

  void IFRecorder::run()
  {
    bool exitflag = false;
    CFBuf rxbuf;
    CmdMsg cmd; 

    if((cmd_stream == NULL) || (rx_stream == NULL)) {
      throw Radio::Exception(std::string("Missing a stream connection.\n"), 
			     this);	
    }
  
  
    while(!exitflag) {
      bool did_work = false;

      if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
	// process the command.
	execCommand(cmd);
	did_work = true; 
	exitflag |= (cmd->target == Command::STOP); 
      }

      // now look for incoming buffers from the rx_stream.
      // but keep it to two buffers before we look back at the cmd stream 
      int bcount = 0; 
      for(bcount = 0; (bcount < 2) && ((rxbuf = rx_stream->get(rx_subs)) != NULL); bcount++) {
	if(rxbuf == NULL) break; 
	did_work = true; 
      
	if(write_stream_on) {
	  ostr.write((char*) rxbuf->data(), rxbuf->size() * sizeof(std::complex<float>));
	}
      }

      if(!did_work) {
	sleep_us(1000); 
      }
    }

    // we get here when the server tells us the game is over... (we get a STOP command)
    if(write_stream_on) {
      ostr.close();
    }
  }

  void IFRecorder::openOutStream(char * ofile_name)
  {
    std::cerr << Format("IFRecorder: about to open file [%0] for writing\n")
      .addS(ofile_name); 
    if(ostr.is_open()) {
      ostr.close();
    }
    ostr.open(ofile_name, std::ofstream::out | std::ofstream::binary); 

    // write the RX front end frequency
    ostr.write((char*) &current_rx_center_freq, sizeof(double));
    write_stream_on = true; 
  }

  void IFRecorder::closeOutStream()
  {
    std::cerr << "IFRecorder: closing RF file" << std::endl; 
    if(ostr.is_open()) {
      ostr.close();
    }
    write_stream_on = false;
  }

  /// implement the subscription method
  void IFRecorder::subscribe() 
  {
    auto reg = MailBoxRegistry::getRegistrar();
  
    cmd_stream = MailBoxBase::convert<MsgMBox>(reg->get("CMD"));
    cmd_subs = cmd_stream->subscribe();

    rx_stream = MailBoxBase::convert<CFMBox>(reg->get("RX"));
    rx_subs = rx_stream->subscribe();
  }
}
