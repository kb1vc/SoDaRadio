/*
Copyright (c) 2018 Matthew H. Reilly (kb1vc)
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

#ifndef IFRECORDER_HDR
#define IFRECORDER_HDR
#include "SoDaBase.hxx"
#include "SoDaThread.hxx"
#include "Params.hxx"
#include "MultiMBox.hxx"
#include "Command.hxx"

#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <mutex>
#include <fstream>
#include <string>
#include <fstream>


namespace SoDa {
  /**
   * IFRecorder -- this is the audio processing chain for the recieve path
   *
   * @image html SoDa_Radio_RX_Signal_Path.svg
   *
   * The IFRecorder unit accepts the 3rd IF (nominally 80kHz to 200kHz)
   * from the USRPRX thread on the rx_stream.
   *
   * In most cases (all but Wide Band FM) the rx stream is downselected
   * by a 625 to 48 resampler before being passed through an audio filter
   * and finally demodulated.
   *
   * As each buffer/timeslice is demodulated, it
   * is placed on a queue of outbound audio blocks for the host processor's
   * audio system. Since there is some slop in the timing (specifically, the
   * clock governing the radio is not necessarily in sync with the audio
   * system clock) the IFRecorder unit monitors the backlog of outbound audio
   * buffers.  When this backlog gets longer than about 400mS, the unit will
   * "drop out" a sample here and there until the downstream sound system
   * catches up.
   *
   * IFRecorder supports CW_U (upper sideband CW), CW_L (lower sideband CW),
   * USB, and LSB modulation via the phasing method, since both I and Q
   * channels are available. AM is performed with a simple magnitude detector.
   */
  class IFRecorder : public SoDa::Thread {
  public:
    /**
     * @brief the constructor
     *
     * @param params command line parameter object
     **/
    IFRecorder(Params * params);

    /// implement the subscription method
    void subscribeToMailBox(const std::string & mbox_name, BaseMBox * mbox_p);
    
    /**
     * @brief the run method -- does the work of the audio receiver process
     */
    void run();

  private:
    /**
     * @brief execute GET commands from the command channel
     * @param cmd the incoming command
     */
    void execGetCommand(Command * cmd); 
    /**
     * @brief handle SET commands from the command channel
     * @param cmd the incoming command
     */
    void execSetCommand(Command * cmd); 
    /**
     * @brief handle Report commands from the command channel
     * @param cmd the incoming command
     */
    void execRepCommand(Command * cmd); 

    /**
     * @brief open an output stream to receive the RF samples
     * @param ofile_name name of the output file to be created. 
     */
    void openOutStream(char * ofile_name);

    /**
     * @brief close the current output stream
     */
    void closeOutStream();

    // parameters
    unsigned int rf_buffer_size; ///< size of input RF buffer chunk
    double rf_sample_rate; ///< sample rate of RF input from USRP -- assumed 625KHz

    double current_rx_center_freq; 

    DatMBox * rx_stream; ///< mailbox producing rx sample stream from USRP
    CmdMBox * cmd_stream; ///< mailbox producing command stream from user
    unsigned int rx_subs; ///< mailbox subscription ID for rx data stream
    unsigned int cmd_subs; ///< mailbox subscription ID for command stream

    std::ofstream ostr; ///< raw (binary) output stream.
    bool write_stream_on; ///< when true, write each incoming buffer to the output stream. 
  };
}


#endif
