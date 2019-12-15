/*
  Copyright (c) 2018, Matthew H. Reilly (kb1vc)
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
#include "OSFilter.hxx"
#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

SoDa::IFRecorder::IFRecorder(Params * params) : SoDa::Thread("IFRecorder")
{
  // setup the streams
  rx_stream = NULL;
  cmd_stream = NULL;

  // what is the default sample rate and buffer size?
  rf_sample_rate = params->getRXRate(); 
  rf_buffer_size = params->getRFBufferSize();

  // we aren't recording right now
  write_stream_on = false; 

  // we don't know the current center frequency
  current_rx_center_freq = 0.0; 
}



void SoDa::IFRecorder::execSetCommand(SoDa::Command * cmd)
{
  SoDa::Command::AudioFilterBW fbw;
  SoDa::Command::ModulationType txmod; 
  switch (cmd->target) {
  case SoDa::Command::RF_RECORD_START:
    openOutStream(cmd->sparm);
    break;
  case SoDa::Command::RF_RECORD_STOP:
    closeOutStream();
    break; 
  default:
    break; 
  }
}

void SoDa::IFRecorder::execGetCommand(SoDa::Command * cmd)
{
  (void) cmd;
}

void SoDa::IFRecorder::execRepCommand(SoDa::Command * cmd)
{
  switch (cmd->target) {
  case SoDa::Command::RX_FE_FREQ:
    current_rx_center_freq = cmd->dparms[0];
    break;
  }
}

void SoDa::IFRecorder::run()
{
  bool exitflag = false;
  SoDa::Buf * rxbuf;
  Command * cmd; 

  if((cmd_stream == NULL) || (rx_stream == NULL)) {
      throw SoDa::Exception((boost::format("Missing a stream connection.\n")).str(), 
			  this);	
  }
  
  
  while(!exitflag) {
    bool did_work = false;

    if((cmd = cmd_stream->get(cmd_subs)) != NULL) {
      // process the command.
      execCommand(cmd);
      did_work = true; 
      exitflag |= (cmd->target == Command::STOP); 
      cmd_stream->free(cmd); 
    }

    // now look for incoming buffers from the rx_stream.
    // but keep it to two buffers before we look back at the cmd stream 
    int bcount = 0; 
    for(bcount = 0; (bcount < 2) && ((rxbuf = rx_stream->get(rx_subs)) != NULL); bcount++) {
      if(rxbuf == NULL) break; 
      did_work = true; 
      
      if(write_stream_on) {
	ostr.write((char*) rxbuf->getComplexBuf(), rxbuf->getComplexLen() * sizeof(std::complex<float>));
      }
      // now free the buffer up.
      rx_stream->free(rxbuf); 
    }

    if(!did_work) {
      usleep(1000); 
    }
  }

  // we get here when the server tells us the game is over... (we get a STOP command)
  if(write_stream_on) {
    ostr.close();
  }
}

void SoDa::IFRecorder::openOutStream(char * ofile_name)
{
  std::cerr << boost::format("IFRecorder: about to open file [%s] for writing\n") % ofile_name; 
  if(ostr.is_open()) {
    ostr.close();
  }
  ostr.open(ofile_name, std::ofstream::out | std::ofstream::binary); 

  // write the RX front end frequency
  ostr.write((char*) &current_rx_center_freq, sizeof(double));
  write_stream_on = true; 
}

void SoDa::IFRecorder::closeOutStream()
{
  std::cerr << "IFRecorder: closing RF file" << std::endl; 
  if(ostr.is_open()) {
    ostr.close();
  }
  write_stream_on = false;
}

/// implement the subscription method
void SoDa::IFRecorder::subscribeToMailBox(const std::string & mbox_name, SoDa::BaseMBox * mbox_p)
{
  if(SoDa::connectMailBox<SoDa::CmdMBox>(this, cmd_stream, "CMD", mbox_name, mbox_p)) {
    cmd_subs = cmd_stream->subscribe();
  }
  if(SoDa::connectMailBox<SoDa::DatMBox>(this, rx_stream, "RX", mbox_name, mbox_p)) {
    rx_subs = rx_stream->subscribe();
  }
}
