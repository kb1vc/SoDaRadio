/*
Copyright (c) 2012,2024 Matthew H. Reilly (kb1vc)
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

#include <memory>

#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"
#include "Params.hxx"
#include "UI.hxx"
#include "UDSockets.hxx"
#include "Spectrogram.hxx"

namespace SoDa {
  class UI : public SoDa::Thread {
  public:
    UI(Params * params);
    ~UI();

    /// implement the subscription method
    void subscribeToMailBoxList(CmdMailBoxMap & cmd_boxes,
				DatMailBoxMap & dat_boxes); 
    
    void run();

  private:
    // Do an FFT on an rx buffer and send the positive
    // frequencies to any network listeners. 
    void sendFFT(SoDa::BufPtr  buf);

    // the internal communications paths -- between the SoDa threads. 
    std::shared_ptr<CmdMBox> cwtxt_stream, cmd_stream, gps_stream;
    std::shared_ptr<DatMBox> if_stream; 


    // these are the pieces of the posix message queue interface to the GUI or whatever.
    SoDa::UD::ServerSocketPtr server_socket, wfall_socket; 

    // we ship a spectrogram of the RX IF stream to the GUI
    Spectrogram * spectrogram;
    unsigned int spectrogram_buckets; 

    Spectrogram * lo_spectrogram; 
    unsigned int lo_spectrogram_buckets;
    double lo_hz_per_bucket;
    float * lo_spectrum; 

    // the spectrum runs from -100kHz below to 100kHz above the center freq. 
    static const double spectrum_span; // = 200e3; 
    
    double baseband_rx_freq;
    double spectrum_center_freq;
    double hz_per_bucket; 
    int required_spect_buckets;

    float * spectrum, * log_spectrum;
    bool new_spectrum_setting;

    float fft_acc_gain;
    unsigned int fft_update_interval;

    unsigned int fft_send_counter;
    
    // flag to signal that we're in microwave LO search mode.
    bool lo_check_mode;

    void updateSpectrumState();
    void execSetCommand(CommandPtr cmd);
    void execGetCommand(CommandPtr  cmd);
    void execRepCommand(CommandPtr cmd);

    void reportSpectrumCenterFreq();
  }; 
}

