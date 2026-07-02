/*
  Copyright (c) 2013, 2026 Matthew H. Reilly (kb1vc)
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

#include "UI.hxx"
#include "version.h"

#include <SoDa/MailBox.hxx>
#include <deque>

namespace SoDa {

const double UI::spectrum_span = 200e3;  
UI::UI(ParamsPtr params) : SoDa::Thread("UI")
{
  // connect to our message streams.
  cwtxt_stream = NULL;
  if_stream = NULL;
  cmd_stream = NULL;


  // create the network ports
  // This UI object is a server.
  server_socket = new SoDa::UD::ServerSocket(params->getServerSocketBasename() + "_cmd");
  wfall_socket = new SoDa::UD::ServerSocket(params->getServerSocketBasename() + "_wfall");

  baseband_rx_freq = 144e6; // just a filler to avoid divide by zero. 
  spectrum_center_freq = 144.2e6;
  
  // create the spectrogram object -- it eats RX IF buffers and produces
  // power spectral density plots.
  spectrogram_buckets = 4 * 4096;
  spectrogram = Spectrogram::make(spectrogram_buckets);

  // we also need an LO check spectrogram.  In particular we want
  // something with really bodacious resolution.
  lo_spectrogram_buckets = 16384;
  lo_spectrogram = Spectrogram::make(lo_spectrogram_buckets);
  lo_spectrum.resize(lo_spectrogram_buckets * 4);
  for(unsigned int i = 0; i < lo_spectrogram_buckets; i++) {
    lo_spectrum[i] = 0.0; 
  }

  // Now  how wide is a 200KHz wide chunk of spectrum, given
  // spectrogram_buckets frequency buckets in the RF sample rate
  double rxrate = params->getRXRate();
  hz_per_bucket = rxrate / ((float) spectrogram_buckets);
  required_spect_buckets = (int) (floor(0.5 + spectrum_span / hz_per_bucket));
  lo_hz_per_bucket = rxrate / ((float) lo_spectrogram_buckets);
  
  // now allocate the buffer that we'll send to the UI
  spectrum.resize(spectrogram_buckets * 4);
  log_spectrum.resize(spectrogram_buckets * 4);
  // make it a little large, and "zero" it out to account for walking off the end...
  for(unsigned int i = 0; i < spectrogram_buckets * 4; i++) {
    spectrum[i] = 1e-20; 
    log_spectrum[i] = -200.0; 
  }

  fft_send_counter = 0;
  fft_update_interval = 4;
  new_spectrum_setting = true;
  fft_acc_gain = 0.9;
  // we are not yet in lo check mode
  lo_check_mode = false;
}

void UI::updateSpectrumState()
{
  cmd_stream->put(SoDa::Command::make(Command::REP, Command::SPEC_STEP,
				    hz_per_bucket));
  cmd_stream->put(SoDa::Command::make(Command::REP, Command::SPEC_BUF_LEN,
				    required_spect_buckets));
  cmd_stream->put(SoDa::Command::make(Command::REP, Command::SPEC_RANGE_LOW,
				    spectrum_center_freq - 0.5 * spectrum_span));
  cmd_stream->put(SoDa::Command::make(Command::REP, Command::SPEC_RANGE_HI,
				    spectrum_center_freq + 0.5 * spectrum_span));
  cmd_stream->put(SoDa::Command::make(Command::REP, Command::SPEC_DIMS, 
				    spectrum_center_freq, 
				    spectrum_span, 
				    ((double) required_spect_buckets)));
}

UI::~UI()
{
  delete server_socket;
  delete wfall_socket; 
}

void UI::run()
{
  SoDa::CommandPtr net_cmd, ring_cmd;
  
  net_cmd = NULL;
  ring_cmd = NULL;
  
  cmd_stream->put(SoDa::Command::make(Command::SET, Command::RX_TUNE_FREQ, 144.2e6));
  cmd_stream->put(SoDa::Command::make(Command::SET, Command::TX_TUNE_FREQ, 144.2e6));
  cmd_stream->put(SoDa::Command::make(Command::SET, Command::RX_AF_FILTER, 1));
  sleep_ms(100);
  cmd_stream->put(SoDa::Command::make(Command::SET, Command::TX_STATE, Command::TX_OFF_0));

  updateSpectrumState(); 

  unsigned int socket_read_count = 0;
  unsigned int socket_empty_count = 0;
  unsigned int iter_count = 0;
  bool new_connection = true; ;

  // Buffer for REP commands that arrive before the GUI client connects.
  // Slow radio backends (e.g., USRP UHD probe) may finish their initial
  // setup -- including INIT_SETUP_COMPLETE, antenna names, and other
  // one-shot reports -- before the client socket is ready; without this
  // buffer those messages are silently dropped by ServerSocket::put.
  std::deque<SoDa::CommandPtr> pending_rep;
  const size_t pending_rep_cap = 4096;

  while(1) {
    iter_count++;
    bool didwork = false;
    bool got_new_netmsg = false;
    bool sock_ready = server_socket->isReady();

    if(sock_ready) {
      if(new_connection) {
	updateSpectrumState();

	std::string vers= SoDa::Format("%0 Git %1")
	  .addS(SoDaRadio_VERSION)
	  .addS(SoDaRadio_GIT_ID).str();

	SoDa::CommandPtr vers_cmd = SoDa::Command::make(Command::REP,
						     Command::SDR_VERSION,
						     vers.c_str());
	server_socket->put(vers_cmd.get(), sizeof(SoDa::Command));

	// Flush any REP commands that arrived before the client was ready.
	while(!pending_rep.empty()) {
	  SoDa::CommandPtr c = pending_rep.front();
	  pending_rep.pop_front();
	  server_socket->put(c.get(), sizeof(SoDa::Command));
	}

	new_connection = false;
      }

      if(net_cmd == NULL) {
	net_cmd = SoDa::Command::make();
      }
      int stat = server_socket->get(net_cmd.get(), sizeof(SoDa::Command));
      if(stat <= 0) {
	socket_empty_count++;
      }
      else {
	socket_read_count++;
	got_new_netmsg = true;
      }
    }
    else {
      new_connection = true;
    }

    // if there are commands arriving from the socket port, handle them.
    if(got_new_netmsg) {
      debugMsg(SoDa::Format("UI net->CMD: %0\n").addS(net_cmd->toString()), 6);
      cmd_stream->put(net_cmd);
      didwork = true;
      if(net_cmd->target == SoDa::Command::TX_CW_EMPTY) {
       	debugMsg("got TX_CW_EMPTY command from socket.\n");
      }
      if(net_cmd->target == SoDa::Command::STOP) {
	break;
      }
      net_cmd = NULL;
    }

    while(cmd_stream->get(cmd_subs, ring_cmd)) {
      debugMsg(SoDa::Format("UI CMD ring: %0\n").addS(ring_cmd->toString()), 6);
      if(ring_cmd->cmd == SoDa::Command::REP) {
	if(sock_ready) {
	  server_socket->put(ring_cmd.get(), sizeof(SoDa::Command));
	}
	else if(pending_rep.size() < pending_rep_cap) {
	  pending_rep.push_back(ring_cmd);
	}
      }
      execCommand(ring_cmd);
      didwork = true;
    }

    // listen ont the IF stream
    int bcount;
    SoDa::CBufPtr if_buf; 
    for(bcount = 0;
	(bcount < 4) && if_stream->get(if_subs, if_buf); 
	bcount++) {
      sendFFT(if_buf);
    }

    // 
    // if there are any socket listeners on the waterfall channel,
    // clue them in.
    
    // if there are any socket listeners on the status channel,
    // clue them in.
    
    // if there is nothing to do, sleep for a little while. -- 
    // do small sleep, to speedup spectrum update.
    if(!didwork) sleep_us(5000);
  }


  return; 
}

void UI::reportSpectrumCenterFreq()
{
    server_socket->put(SoDa::Command::make(Command::REP, Command::SPEC_RANGE_LOW,
					   spectrum_center_freq - 0.5 * spectrum_span).get(),
		       sizeof(SoDa::Command));
    server_socket->put(SoDa::Command::make(Command::REP, Command::SPEC_RANGE_HI,
					   spectrum_center_freq + 0.5 * spectrum_span).get(),
		       sizeof(SoDa::Command));
    server_socket->put(SoDa::Command::make(Command::REP, Command::SPEC_STEP,
					   hz_per_bucket).get(),
		       sizeof(SoDa::Command));
    server_socket->put(SoDa::Command::make(Command::REP, Command::SPEC_BUF_LEN,
					   required_spect_buckets).get(),
		       sizeof(SoDa::Command));
    server_socket->put(SoDa::Command::make(Command::REP, Command::SPEC_DIMS, 
					   spectrum_center_freq, 
					   spectrum_span, 
					   ((double) required_spect_buckets)).get(), 
		       sizeof(SoDa::Command));
    
}


void UI::execSetCommand(CommandPtr cmd)
{
  // when we get a SET SPEC_CENTER_FREQ
  switch(cmd->target) {
  case SoDa::Command::SPEC_CENTER_FREQ:
    spectrum_center_freq = cmd->dparms[0];
    new_spectrum_setting = true;
    reportSpectrumCenterFreq();
    break;
  case SoDa::Command::SPEC_AVG_WINDOW:
    fft_acc_gain = 1.0 - (1.0 / ((double) cmd->iparms[0]));
    new_spectrum_setting = true;
    break; 
  case SoDa::Command::SPEC_UPDATE_RATE:
    fft_update_interval = 11 - cmd->iparms[0];
    if(cmd->iparms[0] > 11) fft_update_interval = 0;
    if(cmd->iparms[0] < 0) fft_update_interval = 11;
    new_spectrum_setting = true;
    debugMsg(SoDa::Format("Updated SPEC_UPDATE_RATE = %0 -> interval = %1\n")
	     .addI(cmd->iparms[0])
	     .addI(fft_update_interval));
    break; 
  default:
    break; 
  }
}

void UI::execGetCommand(CommandPtr cmd)
{
  switch(cmd->target) {
  case SoDa::Command::LO_OFFSET: // remember that we want to report
    // the offset of the LO microwave oscillator on the next FFT event.
    lo_check_mode = true;
    fft_send_counter = 0; 
    break; 
  default:
    break;
  }
}

void UI::execRepCommand(CommandPtr cmd)
{
  switch(cmd->target) {
  case SoDa::Command::RX_LO_FREQ:
    // save the front end baseband frequency
    baseband_rx_freq = cmd->dparms[0];
    break;
  default:
    break;
  }
}


static unsigned int dbgctrfft = 0;
static bool first_ready = true;
static bool calc_max_first = true;

void UI::sendFFT(SoDa::CBufPtr buf)
{
  fft_send_counter++; 
  dbgctrfft++; 
  if(!wfall_socket->isReady()) {
    return;
  }
  if(first_ready == true) dbgctrfft = 0;
  first_ready = false; 
  
  
  // Do an FFT on the buffer
  // Note that we'll only send over on buffer every
  // fft_update_interval times that we're called -- this will keep
  // the IP traffic to something reasonable. 
  if(lo_check_mode) {
    lo_spectrogram->apply_acc(buf->getBuf(), lo_spectrum, (fft_send_counter == 0) ? 0.0 : 0.1);
  }
  else {
    spectrogram->apply_acc(buf->getBuf(), spectrum,
			   (new_spectrum_setting) ? 0.0 : fft_acc_gain);
  }
  new_spectrum_setting = false; 
  calc_max_first = false; 

  float * slice = spectrum.data();
  
  if(!lo_check_mode) {
    // find the right slice
    int idx;
    // first the index of the center point
    // this is the bucket for the baseband rx freq
    idx = (spectrogram_buckets / 2); 
    idx += (int) round((spectrum_center_freq - baseband_rx_freq) / hz_per_bucket);
    // now we've got the index for the center.
    // correct it to be the start...
    idx -= required_spect_buckets / 2; 
    int sbuck_target = (int) spectrogram_buckets;
    // debugMsg(Format("UI::sendFFT spectrogram_buckets %0 spectrum_center_freq %1 baseband_rx_freq %2 hz_per_bucket %3 idx %4 sbucket_target %5\n")
    // 	     .addI(spectrogram_buckets)
    // 	     .addF(spectrum_center_freq, 'e', 10, 6)
    // 	     .addF(baseband_rx_freq, 'e', 10, 6)
    // 	     .addI(hz_per_bucket)
    // 	     .addI(idx)
    // 	     .addI(sbuck_target)
    // 	     );
    if((idx < 0) || (idx > sbuck_target)) {
      slice = NULL; 
    }
    else {
      float * bp = spectrum.data();
      slice = &(bp[idx]);
    }
  }

  if(lo_check_mode && (fft_send_counter >= 8)) {
    // scan the buffer. Then find the peak.
    // scan from lo_spectrum midpoint minus 2KHz to plus 2KHz
    float magmax = 0.0;
    int maxi = 0;
    int idxrange = ((int) (2000.0 / lo_hz_per_bucket));
    int i, j; 
    for(i = -idxrange, j = (lo_spectrogram_buckets / 2) - idxrange; i < idxrange; i++, j++) {
      std::complex<float> v = lo_spectrum[j];
      float mag = v.real() * v.real() + v.imag() * v.imag();
      if(mag > magmax) {
	magmax = mag;
	maxi = i; 
      }
    }
    lo_check_mode = false;
    // send the report
    double freq = ((float) maxi) * lo_hz_per_bucket;
    debugMsg(SoDa::Format("offset = %0\n").addF(freq, 10, 6, 'e')); 
    cmd_stream->put(SoDa::Command::make(Command::REP, Command::LO_OFFSET,
				      freq)); 
    cmd_stream->put(SoDa::Command::make(Command::REP, Command::SPEC_RANGE_LOW,
				      spectrum_center_freq - 0.5 * spectrum_span));
    cmd_stream->put(SoDa::Command::make(Command::REP, Command::SPEC_RANGE_HI,
				      spectrum_center_freq + 0.5 * spectrum_span));
    cmd_stream->put(SoDa::Command::make(Command::REP, Command::SPEC_DIMS, 
				      spectrum_center_freq, 
				      spectrum_span, 
				      ((double) required_spect_buckets)));
    // send the end-of-calib command
    cmd_stream->put(SoDa::Command::make(Command::SET, Command::LO_CHECK,
				      0.0)); 
  }
  else if((fft_send_counter >= fft_update_interval) && (slice != NULL)) {
    // send the buffer over to the XY plotter.
    for(int i = 0; i < required_spect_buckets; i++) {
      log_spectrum[i] = 10.0 * log10(slice[i] * 0.05); 
    }
    wfall_socket->put(log_spectrum.data(), 
		      sizeof(float) * required_spect_buckets);
    fft_send_counter = 0;
    calc_max_first = true; 
    float maxmag = 0.0;

    for(unsigned int ii = 0; ii < spectrogram_buckets; ii++) {
      if(spectrum[ii] > maxmag) {
	maxmag = spectrum[ii];
      }
    }
  }
}


void UI::subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes) {
    for(auto mbox_p : mailboxes) {
      SoDa::MailBoxBase::connect<MailBox<CommandPtr>>(mbox_p,
					       "CMDstream",
					       cmd_stream);
      
      // SoDa::MailBoxBase(connect<MailBox<CommandPtr>>(mbox_p,
      // 						     "CMDstream",
      // 						     cmd_stream));
      SoDa::MailBoxBase::connect<MailBox<CommandPtr>>(mbox_p,
						     "CWTXTstream",
						     cwtxt_stream);
      SoDa::MailBoxBase::connect<MailBox<CBufPtr>>(mbox_p,
						   "IFstream",
						   if_stream);
    }

    if(cmd_stream == nullptr) {
      throw MissingMailBox("CMD", getSelfPtr());
    }
    else {
      cmd_subs = cmd_stream->subscribe();
    }

    if(cwtxt_stream == nullptr) {
      throw MissingMailBox("CWTX", getSelfPtr());
    }

    if(if_stream == nullptr) {
      throw MissingMailBox("IF", getSelfPtr());
    }
    else {
      if_subs = if_stream->subscribe();
    }
}
}
