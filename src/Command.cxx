/*
  Copyright (c) 2012, Matthew H. Reilly (kb1vc)
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

#include "Command.hxx"
#include <string>
#include <sstream>
#include <map>
#include <iostream>
#include <SoDa/Format.hxx>

int SoDa::Command::command_sequence_number = 0;
bool SoDa::Command::table_needs_init = true; 
std::map<std::string, SoDa::Command::CmdTarget> SoDa::Command::target_map_s2v;
std::map<SoDa::Command::CmdTarget, std::string> SoDa::Command::target_map_v2s;

void SoDa::Command::initTableEntry(const std::string & st, CmdTarget tgt) {
  target_map_s2v[st] = tgt;
  target_map_v2s[tgt] = st;
}

void SoDa::Command::initTables()
{
  table_needs_init = false; 
  // created from the buildmap file produced by enum_body2map.sh
  initTableEntry(std::string("RX_TUNE_FREQ"), RX_TUNE_FREQ);
  initTableEntry(std::string("TX_TUNE_FREQ"), TX_TUNE_FREQ);
  initTableEntry(std::string("RX_RETUNE_FREQ"), RX_RETUNE_FREQ);
  initTableEntry(std::string("TX_RETUNE_FREQ"), TX_RETUNE_FREQ);
  initTableEntry(std::string("RX_FE_FREQ"), RX_FE_FREQ);
  initTableEntry(std::string("TX_FE_FREQ"), TX_FE_FREQ);
  initTableEntry(std::string("RX_LO3_FREQ"), RX_LO3_FREQ);
  initTableEntry(std::string("RX_SAMP_RATE"), RX_SAMP_RATE);
  initTableEntry(std::string("TX_SAMP_RATE"), TX_SAMP_RATE);
  initTableEntry(std::string("RX_ANT"), RX_ANT);
  initTableEntry(std::string("TX_ANT"), TX_ANT);
  initTableEntry(std::string("RX_RF_GAIN"), RX_RF_GAIN);
  initTableEntry(std::string("TX_RF_GAIN"), TX_RF_GAIN);
  initTableEntry(std::string("RX_AF_GAIN"), RX_AF_GAIN);
  initTableEntry(std::string("RX_AF_SIDETONE_GAIN"), RX_AF_SIDETONE_GAIN);
  initTableEntry(std::string("TX_AF_GAIN"), TX_AF_GAIN);
  initTableEntry(std::string("TX_STATE"), TX_STATE);
  initTableEntry(std::string("RX_STATE"), RX_STATE);
  initTableEntry(std::string("RX_MODE"), RX_MODE);
  initTableEntry(std::string("TX_MODE"), TX_MODE);
  initTableEntry(std::string("RX_BW"), RX_BW);
  initTableEntry(std::string("RBW"), RBW);
  initTableEntry(std::string("CLOCK_SOURCE"), CLOCK_SOURCE);
  initTableEntry(std::string("LO_CHECK"), LO_CHECK);
  initTableEntry(std::string("LO_OFFSET"), LO_OFFSET);
  initTableEntry(std::string("RX_AF_FILTER"), RX_AF_FILTER);
  initTableEntry(std::string("RX_AF_FILTER_SHAPE"), RX_AF_FILTER_SHAPE);
  initTableEntry(std::string("TX_BEACON"), TX_BEACON); 
  initTableEntry(std::string("TX_CW_SPEED"), TX_CW_SPEED); 
  initTableEntry(std::string("TX_CW_TEXT"), TX_CW_TEXT); 
  initTableEntry(std::string("TX_CW_FLUSHTEXT"), TX_CW_FLUSHTEXT); 

  initTableEntry(std::string("SPEC_CENTER_FREQ"), SPEC_CENTER_FREQ); 
  initTableEntry(std::string("SPEC_RANGE_LOW"), SPEC_RANGE_LOW); 
  initTableEntry(std::string("SPEC_RANGE_HI"), SPEC_RANGE_HI); 
  initTableEntry(std::string("SPEC_STEP"), SPEC_STEP); 
  initTableEntry(std::string("SPEC_BUF_LEN"), SPEC_BUF_LEN);
  initTableEntry(std::string("SPEC_DIMS"), SPEC_DIMS); 
  initTableEntry(std::string("SPEC_AVG_WINDOW"), SPEC_AVG_WINDOW);
  initTableEntry(std::string("SPEC_UPDATE_RATE"), SPEC_UPDATE_RATE);
  initTableEntry(std::string("SDR_VERSION"), SDR_VERSION); 
  initTableEntry(std::string("DBG_REP"), DBG_REP); 

  initTableEntry(std::string("HWMB_REP"), HWMB_REP);
  initTableEntry(std::string("INIT_SETUP_COMPLETE"), INIT_SETUP_COMPLETE);
  initTableEntry(std::string("TVRT_LO_ENABLE"), TVRT_LO_ENABLE);
  initTableEntry(std::string("TVRT_LO_DISABLE"), TVRT_LO_DISABLE);
  initTableEntry(std::string("TVRT_LO_CONFIG"), TVRT_LO_CONFIG);

  initTableEntry(std::string("STOP"), STOP);
  initTableEntry(std::string("STATUS_MESSAGE"), STATUS_MESSAGE);

  initTableEntry(std::string("TX_AUDIO_IN"), TX_AUDIO_IN);
  
  initTableEntry(std::string("RX_GAIN_RANGE"), RX_GAIN_RANGE);
  initTableEntry(std::string("TX_GAIN_RANGE"), TX_GAIN_RANGE);
  initTableEntry(std::string("RX_ANT_NAME"), RX_ANT_NAME);
  initTableEntry(std::string("TX_ANT_NAME"), TX_ANT_NAME);
  initTableEntry(std::string("MOD_SEL_ENTRY"), MOD_SEL_ENTRY);
  initTableEntry(std::string("AF_FILT_ENTRY"), AF_FILT_ENTRY);
  initTableEntry(std::string("CW_CHAR_SENT"), CW_CHAR_SENT);
  initTableEntry(std::string("RF_RECORD_START"), RF_RECORD_START);
  initTableEntry(std::string("RF_RECORD_STOP"), RF_RECORD_STOP);
  initTableEntry(std::string("NBFM_SQUELCH"), NBFM_SQUELCH);

  initTableEntry(std::string("RX_CENTER_FREQ"), RX_CENTER_FREQ);

  initTableEntry(std::string("COMMENT"), COMMENT);  
}

SoDa::Command * SoDa::Command::parseCommandString(std::string str)
{
  if(table_needs_init) {
    initTables(); 
  }
  // tokenize the string
  // [GET|REP] TARGET 
  // or
  // SET TARGET [I|D|S] DATA
  std::istringstream iss(str);

  std::string cmd_str, targ_str, typ_str, val_s;
  double val_d;
  int val_i;

  SoDa::Command::CmdType ct;
  SoDa::Command::CmdTarget targ;
  
  cmd_str = ""; targ_str = ""; 
  iss >> cmd_str >> targ_str; 
  
  if(cmd_str == "SET") {
    ct = Command::SET;
    if(target_map_s2v.find(targ_str) == target_map_s2v.end()) {
      std::cerr << "Command target was [" << targ_str << "]  can't parse that." << std::endl; 
      return NULL; 
    }
    targ = target_map_s2v[targ_str];
    iss >> typ_str;
    switch (typ_str[0]) {
    case 'I':
      iss >> val_i;
      return new Command(ct, targ, val_i); 
      break; 
    case 'D':
      iss >> val_d; 
      return new Command(ct, targ, val_d); 
      break; 
    case 'S':
      iss >> val_s; 
      return new Command(ct, targ, val_s); 
      break;
    default:
      std::cerr << "Can't parse dtype string [" << typ_str << "]" << std::endl; 
      break; 
    }
  }
  else if(cmd_str == "GET") {
  }
  else if(cmd_str == "REP") {
  }
  else {
    std::cerr << "Can't parse [" << str << "] first token was [" << cmd_str << "]" << std::endl; 
    return NULL; 
  }

  return NULL; 
}

std::string SoDa::Command::toString() const
{
  if(table_needs_init) {
    initTables(); 
  }
  std::string sp("");
  std::ostringstream oss;
  switch(cmd) {
  case SET: oss << "SET ";
    break; 
  case GET: oss << "GET ";
    break; 
  case REP: oss <<  "REP ";
    break; 
  default:
    break; 
  }

  if(target_map_v2s.find(target) != target_map_v2s.end()) { 
    oss << target_map_v2s[target];
  }
  else {
    oss << " [?" << target << "?] ";
  }


  SoDa::Format parmstr("%0 %1");
  parmstr.addC(parm_type);
  switch(parm_type) {
  case 'I':
    sp += parmstr.addI(iparms[0]).str();
    break; 
  case 'D':
    sp += parmstr.addF(dparms[0], 'e', 10, 6).str();    
    break; 
  case 'S':
    sp += parmstr.addS(sparm).str();
    break; 
  }

  return sp; 
}
