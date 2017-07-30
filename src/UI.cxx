/*
  Copyright (c) 2013, Matthew H. Reilly (kb1vc)
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

SoDa::UI::UI(Params * params, CmdMBox * _cwtxt_stream,
	     DatMBox * _rx_stream, DatMBox * _if_stream, 
	     CmdMBox * _cmd_stream, CmdMBox * _gps_stream) : SoDa::SoDaThread("UI")
{
  // connect to our message streams.
  cwtxt_stream = _cwtxt_stream;
  rx_stream = _rx_stream;
  if_stream = _if_stream; 
  cmd_stream = _cmd_stream; 
  gps_stream = _gps_stream; 

  // subscribe to them.
  cmd_subs = cmd_stream->subscribe();
  gps_subs = gps_stream->subscribe();
  if_subs = if_stream->subscribe(); 

  // create the network ports
  // This UI object is a server.
  server_socket = new SoDa::UD::ServerSocket(params->getServerSocketBasename() + "_cmd");
  wfall_socket = new SoDa::UD::ServerSocket(params->getServerSocketBasename() + "_wfall");

  baseband_rx_freq = 144e6; // just a filler to avoid divide by zero. 
  spectrum_center_freq = 144.2e6;
  
  // create the spectrogram object -- it eats RX IF buffers and produces
  // power spectral density plots.
  spectrogram_buckets = 4 * 4096;
  spectrogram = new Spectrogram(spectrogram_buckets);

  // we also need an LO check spectrogram.  In particular we want
  // something with really bodacious resolution.
  lo_spectrogram_buckets = 16384;
  lo_spectrogram = new Spectrogram(lo_spectrogram_buckets);
  lo_spectrum = new float[lo_spectrogram_buckets * 4];
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
  spectrum = new float[spectrogram_buckets * 4];
  log_spectrum = new float[spectrogram_buckets * 4];
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

void SoDa::UI::updateSpectrumState()
{
  cmd_stream->put(new SoDa::Command(Command::REP, Command::SPEC_STEP,
				    hz_per_bucket));
  cmd_stream->put(new SoDa::Command(Command::REP, Command::SPEC_BUF_LEN,
				    required_spect_buckets));
  cmd_stream->put(new SoDa::Command(Command::REP, Command::SPEC_RANGE_LOW,
				    spectrum_center_freq - 0.5 * spectrum_span));
  cmd_stream->put(new SoDa::Command(Command::REP, Command::SPEC_RANGE_HI,
				    spectrum_center_freq + 0.5 * spectrum_span));
  cmd_stream->put(new SoDa::Command(Command::REP, Command::SPEC_DIMS, 
				    spectrum_center_freq, 
				    spectrum_span, 
				    ((double) required_spect_buckets)));
}

SoDa::UI::~UI()
{
  delete server_socket;
  delete wfall_socket; 
}

void SoDa::UI::run()
{
  SoDa::Command * net_cmd, * ring_cmd;

  net_cmd = NULL;
  ring_cmd = NULL;
  
  cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_FE_FREQ, 144.2e6));
  cmd_stream->put(new SoDa::Command(Command::SET, Command::TX_FE_FREQ, 144.2e6));
  cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_LO3_FREQ, 100e3));
  cmd_stream->put(new SoDa::Command(Command::SET, Command::RX_AF_FILTER, 1));
  usleep(100000);
  cmd_stream->put(new SoDa::Command(Command::SET, Command::TX_STATE, 0));

  updateSpectrumState(); 

  unsigned int socket_read_count = 0;
  unsigned int socket_empty_count = 0;
  unsigned int iter_count = 0;
  bool new_connection = true; ;
  while(1) {
    iter_count++;
    bool didwork = false;
    bool got_new_netmsg = false; 
    // listen on the socket.

    if(server_socket->isReady()) {
      if(new_connection) {
	updateSpectrumState();

	std::string vers= (boost::format("%s GIT %s") % SoDaRadio_VERSION % SoDaRadio_GIT_ID).str(); 
	SoDa::Command * vers_cmd = new SoDa::Command(Command::REP,
						     Command::SDR_VERSION,
						     vers.c_str());
	std::cerr << "About to send sdr version info\n";
	server_socket->put(vers_cmd, sizeof(SoDa::Command));
	std::cerr << "Sent sdr version info\n";
	new_connection = false; 
      }
      
      if(net_cmd == NULL) {
	net_cmd = new SoDa::Command();
      }
      int stat = server_socket->get(net_cmd, sizeof(SoDa::Command));
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
      debugMsg(boost::format("UI got message [%s]\n") % net_cmd->toString());
      cmd_stream->put(net_cmd);
      didwork = true;
      if(net_cmd->target == SoDa::Command::TX_CW_EMPTY) {
       	debugMsg("got TX_CW_EMPTY command from socket.\n"); 
      }
      if(net_cmd->target == SoDa::Command::STOP) {
	// relay "stop" commands to the GPS unit. 
	gps_stream->put(new SoDa::Command(Command::SET, Command::STOP, 0));
	break;
      }
      net_cmd = NULL; 
    }

    while((ring_cmd = cmd_stream->get(cmd_subs)) != NULL) {
      if(ring_cmd->cmd == SoDa::Command::REP) {
	server_socket->put(ring_cmd, sizeof(SoDa::Command));
      }
      // if(net_cmd->target == SoDa::Command::TX_CW_EMPTY) {
      // 	debugMsg("send TX_CW_EMPTY report to socket.\n"); 
      // }
      
      execCommand(ring_cmd); 
      cmd_stream->free(ring_cmd);
      didwork = true; 
    }

    while((ring_cmd = gps_stream->get(gps_subs)) != NULL) {
      if(ring_cmd->cmd == SoDa::Command::REP) {
	server_socket->put(ring_cmd, sizeof(SoDa::Command));
      }
      execCommand(ring_cmd); 
      gps_stream->free(ring_cmd);
      didwork = true; 
    }
      
    
    // listen ont the IF stream
    int bcount;
    SoDaBuf * if_buf; 
    for(bcount = 0;
	(bcount < 4) && ((if_buf = if_stream->get(if_subs)) != NULL);
	bcount++) {
      sendFFT(if_buf);
      if_stream->free(if_buf); 
    }

    // 
    // if there are any socket listeners on the waterfall channel,
    // clue them in.
    
    // if there are any socket listeners on the status channel,
    // clue them in.
    
    // if there is nothing to do, sleep for a little while. -- 
    // do small sleep, to speedup spectrum update.
    if(!didwork) usleep(5000);
  }


  return; 
}

void SoDa::UI::reportSpectrumCenterFreq()
{
    server_socket->put(new SoDa::Command(Command::REP, Command::SPEC_RANGE_LOW,
					 spectrum_center_freq - 0.5 * spectrum_span),
		       sizeof(SoDa::Command));
    server_socket->put(new SoDa::Command(Command::REP, Command::SPEC_RANGE_HI,
					 spectrum_center_freq + 0.5 * spectrum_span),
		       sizeof(SoDa::Command));
    server_socket->put(new SoDa::Command(Command::REP, Command::SPEC_STEP,
					 hz_per_bucket),
		       sizeof(SoDa::Command));
    server_socket->put(new SoDa::Command(Command::REP, Command::SPEC_BUF_LEN,
					 required_spect_buckets),
		       sizeof(SoDa::Command));
    server_socket->put(new SoDa::Command(Command::REP, Command::SPEC_DIMS, 
					 spectrum_center_freq, 
					 spectrum_span, 
					 ((double) required_spect_buckets)), 
		       sizeof(SoDa::Command));
    
}


void SoDa::UI::execSetCommand(Command * cmd)
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
    fft_update_interval = 20 - cmd->iparms[0];
    if(fft_update_interval < 1) fft_update_interval = 1;
    if(fft_update_interval > 20) fft_update_interval = 20;
    new_spectrum_setting = true;
    debugMsg(boost::format("Updated SPEC_UPDATE_RATE = %d -> interval = %d\n")
	     % cmd->iparms[0] % fft_update_interval);
    break; 
  default:
    break; 
  }
}

void SoDa::UI::execGetCommand(Command * cmd)
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

void SoDa::UI::execRepCommand(Command * cmd)
{
  switch(cmd->target) {
  case SoDa::Command::RX_FE_FREQ:
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

void SoDa::UI::sendFFT(SoDa::SoDaBuf * buf)
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
    lo_spectrogram->apply_acc(buf->getComplexBuf(), buf->getComplexLen(), lo_spectrum, (fft_send_counter == 0) ? 0.0 : 0.1);
  }
  else {
    spectrogram->apply_acc(buf->getComplexBuf(), buf->getComplexLen(), spectrum,
			   (new_spectrum_setting) ? 0.0 : fft_acc_gain);
  }
  new_spectrum_setting = false; 
  calc_max_first = false; 

  float * slice = spectrum;
  
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
    if((idx < 0) || (idx > sbuck_target)) {
      slice = NULL; 
    }
    else {
      slice = &(spectrum[idx]);
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
    debugMsg(boost::format("offset = %g\n") % freq); 
    cmd_stream->put(new SoDa::Command(Command::REP, Command::LO_OFFSET,
				      freq)); 
    cmd_stream->put(new SoDa::Command(Command::REP, Command::SPEC_RANGE_LOW,
				      spectrum_center_freq - 0.5 * spectrum_span));
    cmd_stream->put(new SoDa::Command(Command::REP, Command::SPEC_RANGE_HI,
				      spectrum_center_freq + 0.5 * spectrum_span));
    cmd_stream->put(new SoDa::Command(Command::REP, Command::SPEC_DIMS, 
				      spectrum_center_freq, 
				      spectrum_span, 
				      ((double) required_spect_buckets)));
    // send the end-of-calib command
    cmd_stream->put(new SoDa::Command(Command::SET, Command::LO_CHECK,
				      0.0)); 
  }
  else if((fft_send_counter >= fft_update_interval) && (slice != NULL)) {
    // send the buffer over to the XY plotter.
    for(int i = 0; i < required_spect_buckets; i++) {
      log_spectrum[i] = 10.0 * log10(slice[i] * 0.05); 
    }
    wfall_socket->put(log_spectrum, sizeof(float) * required_spect_buckets);
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
