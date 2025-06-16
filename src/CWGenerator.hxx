/*
Copyright (c) 2012,2013,2014, 2025 Matthew H. Reilly (kb1vc)
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
#include "Command.hxx"
#include "Params.hxx"
#include <map>

namespace SoDa {
  /**
   * A text to morse envelope converter
   *
   * Methods accept a character and encode it into wiggles in the output
   * envelope stream. 
   */
  class CWGenerator {
  public:
    /**
     * @brief Constructor
     * @param cw_env_stream envelope stream from text-to-CW converter
     * @param _samp_rate sample rate for outbound envelope
     * @param _env_buf_len length of outbound buffer
     */
    CWGenerator(DatMBoxPtr cw_env_stream, double _samp_rate, unsigned int _env_buf_len);

    /**
     * @brief set the speed of the cw stream in words per minute
     * @param wpm words per minute
     */
    void setCWSpeed(unsigned int wpm);

    /**
     * @brief tell us what the current CW speed is
     * @return words per minute
     */
    unsigned int getCWSpeed() { return words_per_minute; }

    /**
     * @brief check envelope stream to see if we have less than 1 second's worth of stuff enqueued
     * @return true if we need to encode more characters
     */
    bool readyForMore();

    /**
     * @brief encode a character into the envelope buffer
     * @param c the character to be sent
     * @return true if c was a morse-encodable character
     */
    bool sendChar(char c);
    
  private:

    /**
     * @brief add a buffer of envelope pieces to the outgoing envelope buffer
     * @param v vector of floating point envelope amplitudes
     */
    void appendToOut(std::vector<float> & v);

    /**
     * @brief push the current buffer out to the transmitter, filling it with zeros
     */
    void flushBuffer();

    /**
     * @brief empty the current envelope, don't send it along to the TX unit
     */
    void clearBuffer();

    /**
     * @brief setup the mapping from ascii character to morse sequence
     */
    void initMorseMap();
    
    // configuration params. 
    DatMBoxPtr env_stream;  ///< this is the stream we send envelope buffers into. 
    double sample_rate;  ///< we need to know how long a sample is (in time)
    unsigned int env_buf_len; ///< the length of an envelope buffer 

    // timing parameters
    unsigned int words_per_minute; 
    unsigned int edge_sample_count; ///< edges are 'pre-built' this is the length of an edge, in samples
 
    unsigned int bufs_per_sec; ///< number of envelope buffers required per second.

    // envelopes are float buffers.
    std::vector<float> dit; ///< prototype dit buffer
    std::vector<float> dah; ///< prototype dah buffer
    std::vector<float> inter_char_space; ///< prototype space between characters
    std::vector<float> inter_word_space; ///< prototype space between words
    
    // edges are float buffers too
    std::vector<float> rising_edge; ///< a gentle shape for the leading edge of a pulse
    std::vector<float> falling_edge; ///< a gentle shape for the trailing edge of a pulse

    static std::map<char, std::string> morse_map; ///< map from ascii character to dits-and-dahs

    // current output buffer
    SoDa::BufPtr cur_buf; ///< the current envelope to be filled in
    unsigned int cur_buf_idx; ///< where are we in the buffer? 
    
    // state of the translator
    bool in_digraph; ///< if true, we're sending a two-character (no inter-char space) sequence (like _AR)
    bool last_was_space; ///< if true, next interword space should be a little short
  }; 
}

