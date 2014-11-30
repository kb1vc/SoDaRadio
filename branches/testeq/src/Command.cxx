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

int SoDa::Command::command_sequence_number = 0;
bool SoDa::Command::table_needs_init = true; 
std::map<std::string, SoDa::Command::cmd_target> SoDa::Command::target_map_s2v;
std::map<SoDa::Command::cmd_target, std::string *> SoDa::Command::target_map_v2s;

void SoDa::Command::initTables()
{
  table_needs_init = true; 
  // created from the buildmap file produced by enum_body2map.sh
  target_map_s2v[std::string("RX_TUNE_FREQ")] = RX_TUNE_FREQ;
  target_map_s2v[std::string("TX_TUNE_FREQ")] = TX_TUNE_FREQ;
  target_map_s2v[std::string("RX_RETUNE_FREQ")] = RX_RETUNE_FREQ;
  target_map_s2v[std::string("TX_RETUNE_FREQ")] = TX_RETUNE_FREQ;
  target_map_s2v[std::string("RX_FE_FREQ")] = RX_FE_FREQ;
  target_map_s2v[std::string("TX_FE_FREQ")] = TX_FE_FREQ;
  target_map_s2v[std::string("RX_LO3_FREQ")] = RX_LO3_FREQ;
  target_map_s2v[std::string("RX_SAMP_RATE")] = RX_SAMP_RATE;
  target_map_s2v[std::string("TX_SAMP_RATE")] = TX_SAMP_RATE;
  target_map_s2v[std::string("RX_ANT")] = RX_ANT;
  target_map_s2v[std::string("TX_ANT")] = TX_ANT;
  target_map_s2v[std::string("RX_RF_GAIN")] = RX_RF_GAIN;
  target_map_s2v[std::string("RX_RF_CHAN_GAIN")] = RX_RF_CHAN_GAIN;
  target_map_s2v[std::string("TX_RF_GAIN")] = TX_RF_GAIN;
  target_map_s2v[std::string("TX_RF_CHAN_GAIN")] = TX_RF_CHAN_GAIN;
  target_map_s2v[std::string("RX_AF_GAIN")] = RX_AF_GAIN;
  target_map_s2v[std::string("RX_AF_SIDETONE_GAIN")] = RX_AF_SIDETONE_GAIN;
  target_map_s2v[std::string("TX_AF_GAIN")] = TX_AF_GAIN;
  target_map_s2v[std::string("TX_STATE")] = TX_STATE;
  target_map_s2v[std::string("RX_STATE")] = RX_STATE;
  target_map_s2v[std::string("RX_MODE")] = RX_MODE;
  target_map_s2v[std::string("TX_MODE")] = TX_MODE;
  target_map_s2v[std::string("RX_BW")] = RX_BW;
  target_map_s2v[std::string("RBW")] = RBW;
  target_map_s2v[std::string("CLOCK_SOURCE")] = CLOCK_SOURCE;
  target_map_s2v[std::string("LO_CHECK")] = LO_CHECK;
  target_map_s2v[std::string("LO_OFFSET")] = LO_OFFSET;
  target_map_s2v[std::string("RX_AF_FILTER")] = RX_AF_FILTER;
  target_map_s2v[std::string("TX_BEACON")] = TX_BEACON; 
  target_map_s2v[std::string("TX_CW_SPEED")] = TX_CW_SPEED; 
  target_map_s2v[std::string("TX_CW_TEXT")] = TX_CW_TEXT; 
  target_map_s2v[std::string("TX_CW_FLUSHTEXT")] = TX_CW_FLUSHTEXT; 

  target_map_s2v[std::string("SPEC_CENTER_FREQ")] = SPEC_CENTER_FREQ; 
  target_map_s2v[std::string("SPEC_RANGE_LOW")] = SPEC_RANGE_LOW; 
  target_map_s2v[std::string("SPEC_RANGE_HI")] = SPEC_RANGE_HI; 
  target_map_s2v[std::string("SPEC_STEP")] = SPEC_STEP; 
  target_map_s2v[std::string("SPEC_BUF_LEN")] = SPEC_BUF_LEN;
  target_map_s2v[std::string("DBG_REP")] = DBG_REP; 

  target_map_s2v[std::string("HWMB_REP")] = HWMB_REP;

  target_map_s2v[std::string("TVRT_LO_ENABLE")] = TVRT_LO_ENABLE;
  target_map_s2v[std::string("TVRT_LO_DISABLE")] = TVRT_LO_DISABLE;
  target_map_s2v[std::string("TVRT_LO_CONFIG")] = TVRT_LO_CONFIG;

  target_map_s2v[std::string("RX_FE_BASEBAND_FREQ")] = RX_FE_BASEBAND_FREQ;
  target_map_s2v[std::string("RX_BUFFER_SIZE")] = RX_BUFFER_SIZE;
  target_map_s2v[std::string("SPEC_DOWNSAMP_RATE")] = SPEC_DOWNSAMP_RATE;
  target_map_s2v[std::string("RX_COLLECT_BUFFER")] = RX_COLLECT_BUFFER;
  target_map_s2v[std::string("TRIGGER_COLLECT")] = TRIGGER_COLLECT;

  target_map_s2v[std::string("ENV_POWER")] = ENV_POWER;
  target_map_s2v[std::string("CLEAR_ENV_POWER")] = CLEAR_ENV_POWER;
  target_map_s2v[std::string("NF_THERM")] = NF_THERM;
  target_map_s2v[std::string("SPEC_ENA_A")] = SPEC_ENA_A;
  target_map_s2v[std::string("SPEC_ENA_B")] = SPEC_ENA_B; 

  target_map_s2v[std::string("START_TX_SWEEP")] = START_TX_SWEEP;
  target_map_s2v[std::string("STOP_TX_SWEEP")] = STOP_TX_SWEEP;
  target_map_s2v[std::string("STEP_TX_SWEEP")] = STEP_TX_SWEEP;

  target_map_s2v[std::string("STOP")] = STOP;
  
  target_map_v2s[RX_TUNE_FREQ] = new std::string("RX_TUNE_FREQ");
  target_map_v2s[TX_TUNE_FREQ] = new std::string("TX_TUNE_FREQ");
  target_map_v2s[RX_RETUNE_FREQ] = new std::string("RX_RETUNE_FREQ");
  target_map_v2s[TX_RETUNE_FREQ] = new std::string("TX_RETUNE_FREQ");
  target_map_v2s[RX_FE_FREQ] = new std::string("RX_FE_FREQ");
  target_map_v2s[TX_FE_FREQ] = new std::string("TX_FE_FREQ");
  target_map_v2s[RX_LO3_FREQ] = new std::string("RX_LO3_FREQ");
  target_map_v2s[RX_SAMP_RATE] = new std::string("RX_SAMP_RATE");
  target_map_v2s[TX_SAMP_RATE] = new std::string("TX_SAMP_RATE");
  target_map_v2s[RX_ANT] = new std::string("RX_ANT");
  target_map_v2s[TX_ANT] = new std::string("TX_ANT");
  target_map_v2s[RX_RF_GAIN] = new std::string("RX_RF_GAIN");
  target_map_v2s[RX_RF_CHAN_GAIN] = new std::string("RX_RF_CHAN_GAIN");
  target_map_v2s[TX_RF_GAIN] = new std::string("TX_RF_GAIN");
  target_map_v2s[TX_RF_CHAN_GAIN] = new std::string("TX_RF_CHAN_GAIN");
  target_map_v2s[RX_AF_GAIN] = new std::string("RX_AF_GAIN");
  target_map_v2s[RX_AF_SIDETONE_GAIN] = new std::string("RX_AF_SIDETONE_GAIN");
  target_map_v2s[TX_AF_GAIN] = new std::string("TX_AF_GAIN");
  target_map_v2s[TX_STATE] = new std::string("TX_STATE");
  target_map_v2s[RX_STATE] = new std::string("RX_STATE");
  target_map_v2s[RX_MODE] = new std::string("RX_MODE");
  target_map_v2s[TX_MODE] = new std::string("TX_MODE");
  target_map_v2s[RX_BW] = new std::string("RX_BW");
  target_map_v2s[RBW] = new std::string("RBW");
  target_map_v2s[CLOCK_SOURCE] = new std::string("CLOCK_SOURCE");
  target_map_v2s[LO_CHECK] = new std::string("LO_CHECK");
  target_map_v2s[LO_OFFSET] = new std::string("LO_OFFSET");
  target_map_v2s[RX_AF_FILTER] = new std::string("RX_AF_FILTER");
  target_map_v2s[TX_BEACON] = new std::string("TX_BEACON");
  target_map_v2s[TX_CW_SPEED] = new std::string("TX_CW_SPEED");
  target_map_v2s[TX_CW_TEXT] = new std::string("TX_CW_TEXT");
  target_map_v2s[TX_CW_FLUSHTEXT] = new std::string("TX_CW_FLUSHTEXT");

  target_map_v2s[SPEC_CENTER_FREQ] = new std::string("SPEC_CENTER_FREQ");
  target_map_v2s[SPEC_RANGE_LOW] = new std::string("SPEC_RANGE_LOW");
  target_map_v2s[SPEC_RANGE_HI] = new std::string("SPEC_RANGE_HI");
  target_map_v2s[SPEC_STEP] = new std::string("SPEC_STEP");
  target_map_v2s[SPEC_BUF_LEN] = new std::string("SPEC_BUF_LEN");

  target_map_v2s[HWMB_REP] = new std::string("HWMB_REP");
  
  target_map_v2s[DBG_REP] = new std::string("DBG_REP");
  target_map_v2s[TVRT_LO_ENABLE] = new std::string("TVRT_LO_ENABLE");
  target_map_v2s[TVRT_LO_DISABLE] = new std::string("TVRT_LO_DISABLE");
  target_map_v2s[TVRT_LO_CONFIG] = new std::string("TVRT_LO_CONFIG");

  target_map_v2s[RX_FE_BASEBAND_FREQ] = new std::string("RX_FE_BASEBAND_FREQ");
  target_map_v2s[RX_BUFFER_SIZE] = new std::string("RX_BUFFER_SIZE");
  target_map_v2s[SPEC_DOWNSAMP_RATE] = new std::string("SPEC_DOWNSAMP_RATE");
  target_map_v2s[RX_COLLECT_BUFFER] = new std::string("RX_COLLECT_BUFFER");
  target_map_v2s[TRIGGER_COLLECT] = new std::string("TRIGGER_COLLECT");

  target_map_v2s[ENV_POWER] = new std::string("ENV_POWER");
  target_map_v2s[CLEAR_ENV_POWER] = new std::string("CLEAR_ENV_POWER");
  target_map_v2s[NF_THERM] = new std::string("NF_THERM");
  target_map_v2s[SPEC_ENA_A] = new std::string("SPEC_ENA_A");
  target_map_v2s[SPEC_ENA_B] = new std::string("SPEC_ENA_B"); 

  target_map_v2s[START_TX_SWEEP] = new std::string("START_TX_SWEEP");
  target_map_v2s[STEP_TX_SWEEP] = new std::string("STEP_TX_SWEEP");
  target_map_v2s[STOP_TX_SWEEP] = new std::string("STOP_TX_SWEEP");

  target_map_v2s[STOP] = new std::string("STOP");
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

  SoDa::Command::cmd_type ct;
  SoDa::Command::cmd_target targ;
  
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

std::string & SoDa::Command::toString()
{
  if(table_needs_init) {
    initTables(); 
  }
  std::string *sp = new std::string("");
  std::ostringstream oss;
  switch(cmd) {
  case SET: oss << "SET ";
    break; 
  case GET: oss << "GET ";
    break; 
  case REP: oss <<  "REP ";
    break; 
  }

  if(target_map_v2s[target] != NULL) { 
    std::string tg(*target_map_v2s[target]); 
    oss << tg;
  }
  else {
    oss << " [?" << target << "?] ";
  }


  
  switch(parm_type) {
  case 'I':
    oss << " I " << iparms[0]; 
    break; 
  case 'D':
    oss << " D " << dparms[0]; 
    break; 
  case 'S':
    oss << " S \"" << sparm << "\"";
    break; 
  }
  *sp += oss.str();
  
  return *sp; 
}
