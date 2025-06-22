/*
Copyright (c) 2012, 2025 Matthew H. Reilly (kb1vc)
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
#pragma once

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"

#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include "UDSockets.hxx"
#include "Spectrogram.hxx"

#include <SoDa/MailBox.hxx>
#include <memory>


namespace SoDa {
  class UI;
  typedef std::shared_ptr<UI> UIPtr;

  
  class UI : public SoDa::Thread {
  private:
    UI(ParamsPtr params);

  public:
    static UIPtr make(ParamsPtr params) {
      auto ret = std::shared_ptr<UI>(new UI(params));
      ret->registerThread(ret);
      return ret; 
    }


    ~UI();

    /**
     * @brief connect to useful mailboxes. 
     * 
     * @param mailboxes list of mailboxes to which we might subscribe.
     */
    void subscribeToMailBoxes(const std::vector<MailBoxBasePtr> & mailboxes);  
    
    void run();

  private:
    void sendCommandSocket(CommandPtr ptr);
    
    // Do an FFT on an rx buffer and send the positive
    // frequencies to any network listeners. 
    void sendFFT(SoDa::CBufPtr buf);

    // the internal communications paths -- between the SoDa threads. 
    CmdMBoxPtr cwtxt_stream, cmd_stream, gps_stream;
    CDatMBoxPtr if_stream; 
    CmdMBox::Subscription cmd_subs, gps_subs;
    CDatMBox::Subscription if_subs;


    // these are the pieces of the posix message queue interface to the GUI or whatever.
    SoDa::UD::ServerSocket * server_socket, * wfall_socket; 

    // we ship a spectrogram of the RX IF stream to the GUI
    SpectrogramPtr spectrogram;
    unsigned int spectrogram_buckets; 

    SpectrogramPtr lo_spectrogram; 
    unsigned int lo_spectrogram_buckets;
    double lo_hz_per_bucket;
    std::vector<float> lo_spectrum; 

    // the spectrum runs from -100kHz below to 100kHz above the center freq. 
    static const double spectrum_span; // = 200e3; 
    
    double baseband_rx_freq;
    double spectrum_center_freq;
    double hz_per_bucket; 
    int required_spect_buckets;

    std::vector<float> spectrum, log_spectrum;
    bool new_spectrum_setting;

    float fft_acc_gain;
    unsigned int fft_update_interval;

    unsigned int fft_send_counter;
    
    // flag to signal that we're in microwave LO search mode.
    bool lo_check_mode;

    void updateSpectrumState();
    void execSetCommand(CommandPtr cmd);
    void execGetCommand(CommandPtr cmd);
    void execRepCommand(CommandPtr cmd);

    void reportSpectrumCenterFreq();
  }; 
}


