/*
  Copyright (c) 2013,2022 Matthew H. Reilly (kb1vc)
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

namespace SoDa {
  
  const double UI::spectrum_span = 200e3;

  UI::UI(Params * params) : Thread("UI")
  {
    // connect to our message streams.
    cwtxt_stream = nullptr;
    if_stream = nullptr;
    cmd_stream = nullptr;
    gps_stream = nullptr;


    // create the network ports
    // This UI object is a server.
    server_socket = new UD::ServerSocket(params->getServerSocketBasename() + "_cmd");
    wfall_socket = new UD::ServerSocket(params->getServerSocketBasename() + "_wfall");

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

  void UI::updateSpectrumState()
  {
    cmd_stream->put(Command::make(Command::REP, Command::SPEC_STEP,
				hz_per_bucket));
    cmd_stream->put(Command::make(Command::REP, Command::SPEC_BUF_LEN,
				required_spect_buckets));
    cmd_stream->put(Command::make(Command::REP, Command::SPEC_RANGE_LOW,
				spectrum_center_freq - 0.5 * spectrum_span));
    cmd_stream->put(Command::make(Command::REP, Command::SPEC_RANGE_HI,
				spectrum_center_freq + 0.5 * spectrum_span));
    cmd_stream->put(Command::make(Command::REP, Command::SPEC_DIMS, 
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
    CmdMsg net_cmd, ring_cmd;

    if((cwtxt_stream == nullptr) || 
       (if_stream == nullptr) || 
       (cmd_stream == nullptr) || 
       (gps_stream == nullptr)) {
    
      throw Radio::Exception("Missing a stream connection.\n", 
			     this);	
    }
  
    net_cmd = nullptr;
    ring_cmd = nullptr;
  
    cmd_stream->put(Command::make(Command::SET, Command::RX_FE_FREQ, 144.2e6));
    cmd_stream->put(Command::make(Command::SET, Command::TX_FE_FREQ, 144.2e6));
    cmd_stream->put(Command::make(Command::SET, Command::RX_LO3_FREQ, 100e3));
    cmd_stream->put(Command::make(Command::SET, Command::RX_AF_FILTER, 1));
    sleep_ms(100);
    cmd_stream->put(Command::make(Command::SET, Command::TX_STATE, 0));

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

	  std::string vers= SoDa::Format("%0 Git %1")
	    .addS(SoDaRadio_VERSION)
	    .addS(SoDaRadio_GIT_ID).str();
	
	  Command * vers_cmd = new Command(Command::REP,
					   Command::SDR_VERSION,
					   vers.c_str());
	  server_socket->put(vers_cmd, sizeof(Command));
	  new_connection = false; 
	}
      
	if(net_cmd == nullptr) {
	  net_cmd = Command::make();
	}
	int stat = server_socket->get(net_cmd.get(), sizeof(Command));
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
	debugMsg(SoDa::Format("UI got message [%0]\n").addS(net_cmd->toString()));
	cmd_stream->put(net_cmd);
	didwork = true;
	if(net_cmd->target == Command::TX_CW_EMPTY) {
	  debugMsg("got TX_CW_EMPTY command from socket.\n"); 
	}
	if(net_cmd->target == Command::STOP) {
	  // relay "stop" commands to the GPS unit. 
	  gps_stream->put(Command::make(Command::SET, Command::STOP, 0));
	  break;
	}
	net_cmd = nullptr; 
      }

      while((ring_cmd = cmd_stream->get(cmd_subs)) != nullptr) {
	if(ring_cmd->cmd == Command::REP) {
	  server_socket->put(ring_cmd.get(), sizeof(Command));
	}
	// if(net_cmd->target == Command::TX_CW_EMPTY) {
	// 	debugMsg("send TX_CW_EMPTY report to socket.\n"); 
	// }
      
	execCommand(ring_cmd); 
	ring_cmd = nullptr; 
	didwork = true; 
      }

      while((ring_cmd = gps_stream->get(gps_subs)) != nullptr) {
	if(ring_cmd->cmd == Command::REP) {
	  server_socket->put(ring_cmd.get(), sizeof(Command));
	}
	execCommand(ring_cmd); 
	ring_cmd = nullptr; 
	didwork = true; 
      }
      
    
      // listen ont the IF stream
      int bcount;
      CFBuf if_buf; 
      for(bcount = 0;
	  (bcount < 4) && ((if_buf = if_stream->get(if_subs)) != nullptr);
	  bcount++) {
	sendFFT(if_buf);
	if_buf = nullptr; 
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
    server_socket->put(new Command(Command::REP, Command::SPEC_RANGE_LOW,
				   spectrum_center_freq - 0.5 * spectrum_span),
		       sizeof(Command));
    server_socket->put(new Command(Command::REP, Command::SPEC_RANGE_HI,
				   spectrum_center_freq + 0.5 * spectrum_span),
		       sizeof(Command));
    server_socket->put(new Command(Command::REP, Command::SPEC_STEP,
				   hz_per_bucket),
		       sizeof(Command));
    server_socket->put(new Command(Command::REP, Command::SPEC_BUF_LEN,
				   required_spect_buckets),
		       sizeof(Command));
    server_socket->put(new Command(Command::REP, Command::SPEC_DIMS, 
				   spectrum_center_freq, 
				   spectrum_span, 
				   ((double) required_spect_buckets)), 
		       sizeof(Command));
    cmd_stream->put(Command::make(Command::REP, Command::COMMENT, SoDa::Format("UI::rSCF %0").addF(spectrum_center_freq).str(), 0));
  }


  void UI::execSetCommand(Command * cmd)
  {
    // when we get a SET SPEC_CENTER_FREQ
    switch(cmd->target) {
    case Command::SPEC_CENTER_FREQ:
      spectrum_center_freq = cmd->dparms[0];
      new_spectrum_setting = true;
      reportSpectrumCenterFreq();
      break;
    case Command::SPEC_AVG_WINDOW:
      fft_acc_gain = 1.0 - (1.0 / ((double) cmd->iparms[0]));
      new_spectrum_setting = true;
      break; 
    case Command::SPEC_UPDATE_RATE:
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

  void UI::execGetCommand(Command * cmd)
  {
    switch(cmd->target) {
    case Command::LO_OFFSET: // remember that we want to report
      // the offset of the LO microwave oscillator on the next FFT event.
      lo_check_mode = true;
      fft_send_counter = 0; 
      break; 
    default:
      break;
    }
  }

  void UI::execRepCommand(Command * cmd)
  {
    switch(cmd->target) {
    case Command::RX_FE_FREQ:
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

  void UI::sendFFT(CFBuf & buf)
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
      lo_spectrogram->apply_acc(buf->data(), buf->size(), lo_spectrum, (fft_send_counter == 0) ? 0.0 : 0.1);
    }
    else {
      spectrogram->apply_acc(buf->data(), buf->size(), spectrum,
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
	slice = nullptr; 
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
      debugMsg(SoDa::Format("offset = %0\n").addF(freq, 'e', 10, 6)); 
      cmd_stream->put(Command::make(Command::REP, Command::LO_OFFSET,
				  freq)); 
      cmd_stream->put(Command::make(Command::REP, Command::SPEC_RANGE_LOW,
				  spectrum_center_freq - 0.5 * spectrum_span));
      cmd_stream->put(Command::make(Command::REP, Command::SPEC_RANGE_HI,
				  spectrum_center_freq + 0.5 * spectrum_span));
      cmd_stream->put(Command::make(Command::REP, Command::SPEC_DIMS, 
				  spectrum_center_freq, 
				  spectrum_span, 
				  ((double) required_spect_buckets)));
      // send the end-of-calib command
      cmd_stream->put(Command::make(Command::SET, Command::LO_CHECK,
				  0.0)); 
    }
    else if((fft_send_counter >= fft_update_interval) && (slice != nullptr )) {
      // send the buffer over to the GUI
      for(int i = 0; i < required_spect_buckets; i++) {
	log_spectrum[i] = 10.0 * log10(slice[i] * 0.05);  // what's with the 0.05 ?  and the 10.0 vs. 20 .. is slice mag^2
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

  /// implement the subscription method
  void UI::subscribe()
  {
    auto reg = MailBoxRegistry::getRegistrar();
    cmd_stream = MailBoxBase::convert<MsgMBox>(reg->get("CMD"));
    cmd_subs = cmd_stream->subscribe();

    cwtxt_stream = MailBoxBase::convert<MsgMBox>(reg->get("CW_TXT"));

    if_stream = MailBoxBase::convert<CFMBox>(reg->get("IF"));
    if_subs = if_stream->subscribe();

    gps_stream = MailBoxBase::convert<MsgMBox>(reg->get("GPS"));
    gps_subs = gps_stream->subscribe();
    
  }
}
