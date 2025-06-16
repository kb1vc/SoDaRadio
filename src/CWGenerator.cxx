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

#include "CWGenerator.hxx"
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

using namespace SoDa;

std::map<char, std::string> CWGenerator::morse_map; 

CWGenerator::CWGenerator(DatMBoxPtr cw_env_stream, double _samp_rate, unsigned int _env_buf_len)
{
  env_stream = cw_env_stream;
  sample_rate = _samp_rate;
  env_buf_len = _env_buf_len; 

  if(morse_map.empty()) initMorseMap();

  // we want a rising/trailing edge attack that
  // has a 5mS rise/fall time.
  // Thats
  edge_sample_count = (int) round(0.005 * sample_rate);

  rising_edge.resize(edge_sample_count);
  falling_edge.resize(edge_sample_count);
  
  // make a soft rising edge
  // and a soft falling edge
  float ang = 0.0;
  float ang_incr = M_PI / ((float) edge_sample_count); 
  for(unsigned int i = 0; i < edge_sample_count; i++) {
    rising_edge[i] = 0.5 * (1.0 - cos(ang));
    falling_edge[i] = 0.5 * (1.0 + cos(ang));
    ang += ang_incr; 
  }


  // and set a default speed
  setCWSpeed(10);

  // allocate our first outbound buffer
  cur_buf = SoDa::Buf::make(env_buf_len);
  cur_buf_idx = 0; 
  // we really want a buffer that is the complex length. 

  // how big is this buffer, and how many of them do I need for 1 second of TX time.
  bufs_per_sec = (unsigned int) (sample_rate / ((double) env_buf_len));

  // we aren't in the middle of a digraph right now. 
  in_digraph = false; 
}


bool CWGenerator::readyForMore()
{
  // if we've got less than 1 second's worth of
  // elements buffered up, then return true;
  unsigned int ifc = env_stream->minReadyCount();

  return (ifc < bufs_per_sec); 
}

void CWGenerator::initMorseMap()
{
  morse_map['a'] = ".-";
  morse_map['b'] = "-...";
  morse_map['c'] = "-.-.";
  morse_map['d'] = "-..";
  morse_map['e'] = ".";
  morse_map['f'] = "..-.";
  morse_map['g'] = "--.";
  morse_map['h'] = "....";
  morse_map['i'] = "..";
  morse_map['j'] = ".---";
  morse_map['k'] = "-.-";
  morse_map['l'] = ".-..";
  morse_map['m'] = "--";
  morse_map['n'] = "-.";
  morse_map['o'] = "---";
  morse_map['p'] = ".--.";
  morse_map['q'] = "--.-";
  morse_map['r'] = ".-.";
  morse_map['s'] = "...";
  morse_map['t'] = "-";
  morse_map['u'] = "..-";
  morse_map['v'] = "...-";
  morse_map['w'] = ".--";
  morse_map['x'] = "-..-";
  morse_map['y'] = "-.--";
  morse_map['z'] = "--..";
  morse_map['0'] = "-----";
  morse_map['1'] = ".----";
  morse_map['2'] = "..---";
  morse_map['3'] = "...--";
  morse_map['4'] = "....-";
  morse_map['5'] = ".....";
  morse_map['6'] = "-....";
  morse_map['7'] = "--...";
  morse_map['8'] = "---..";
  morse_map['9'] = "----.";
  morse_map['.'] = ".-.-.-";
  morse_map[','] = "--..--";
  morse_map['?'] = "..--..";
  morse_map['-'] = "-...-";
  morse_map['/'] = "-..-.";
}

void CWGenerator::setCWSpeed(unsigned int wpm)
{
  if(wpm < 1) wpm = 1;
  if(wpm > 50) wpm = 50; 
  words_per_minute = wpm;

  float dot_time_s = 1.20 / ((float) wpm);
  unsigned int dot_samples = (int) round(sample_rate * dot_time_s);
  unsigned int dah_samples = dot_samples * 3; 

  // clear out the vectors
  dit.clear();
  dah.clear();
  inter_word_space.clear();
  inter_char_space.clear();
    
  
  // now create the dit, dah, and space pattern.
  unsigned int i, j;
  for(i = 0; i < edge_sample_count; i++) {
    dit.push_back(rising_edge[i]);
  }
  for(; i < (dot_samples - edge_sample_count); i++) {
    dit.push_back(1.0); 
  }
  for(j = 0; i < dot_samples; i++, j++) {
    dit.push_back(falling_edge[j]); 
  }
  for(; i < dot_samples * 2; i++) {
    dit.push_back(0.0); 
  }

  for(i = 0; i < edge_sample_count; i++) {
    dah.push_back(rising_edge[i]);
  }
  for(; i < (dah_samples - edge_sample_count); i++) {
    dah.push_back(1.0); 
  }
  for(j = 0; i < dah_samples; i++, j++) {
    dah.push_back(falling_edge[j]); 
  }
  for(; i < dah_samples + dot_samples; i++) {
    dah.push_back(0.0); 
  }

  for(i = 0; i < dit.size(); i++) {
    inter_char_space.push_back(0.0);
  }
  for(i = 0; i < dit.size() * 3; i++) {
    inter_word_space.push_back(0.0); 
  }
}

void CWGenerator::appendToOut(std::vector<float> & v)
{
  // ok.  What we're trying to do here is to fill "cur_buf" with
  // the samples in v.  Once cur_buf is filled, we'll put it on
  // the message ring, and create a new buffer.

  // because of the way the messaging works, cur_buf is really
  // a pointer to a buffer message object. It contains an actual
  // vector of floats (or complex floats).  That's the thing
  // that we're filling up.

  // We'll do this the simplest way possible first.
  std::vector<float> & curvec = cur_buf->getFloatBuf();
  for(auto vv : v) {
    curvec[cur_buf_idx] = vv;
    cur_buf_idx++;
    if(cur_buf_idx == env_buf_len) {
      // send the buffer
      env_stream->put(cur_buf);
      cur_buf = SoDa::Buf::make(env_buf_len);
      cur_buf_idx = 0;
      curvec = cur_buf->getFloatBuf();
    }
  }
}

void CWGenerator::flushBuffer()
{
  if(cur_buf_idx > 0) {
    auto & dbuf = cur_buf->getFloatBuf();
    for(; cur_buf_idx < env_buf_len; cur_buf_idx++) {
      // fill the rest with zeros
      dbuf[cur_buf_idx] = 0.0; 
    }
    // post the buffer.
    env_stream->put(cur_buf);
    cur_buf = SoDa::Buf::make(env_buf_len);    
    cur_buf_idx = 0;
  }
}

void CWGenerator::clearBuffer()
{
  if(cur_buf != NULL) {
    cur_buf = SoDa::Buf::make(env_buf_len);
    cur_buf_idx = 0;
  }
}

// returns true if the operation resulted in
// something that took time... (an actual element)
bool CWGenerator::sendChar(char c)
{
  c = tolower(c);
  if(c == '_') {
    in_digraph = true;
    return false; 
  }

  if(c == ' ') {
    in_digraph = false;
    if(last_was_space) {
      appendToOut(inter_word_space);
    }
    else {
      appendToOut(inter_word_space);
    }
    last_was_space = true;
    return true; 
  }
  else {
    last_was_space = false;
  }

  if(morse_map.find(c) == morse_map.end()) return false;
  
  std::string symb = morse_map[c];

  for(auto si : symb) {
    if(si == '.') {
      appendToOut(dit);
    }
    else if(si == '-') {
      appendToOut(dah);
    }
  }

  if(!in_digraph) appendToOut(inter_char_space);
  else in_digraph = false; 
  
  return true; 
}



