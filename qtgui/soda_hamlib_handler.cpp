/*
Copyright (c) 2017 Matthew H. Reilly (kb1vc)
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

#include "soda_hamlib_handler.hpp"
#include <hamlib/rig.h>

// All the command handling here is according to what I learned from
// http://hamlib.sourceforge.net/manuals/hamlib.html#rigctld-Default-protocol
// and
// http://hamlib.sourceforge.net/pdf/rigctl.1.pdf

using namespace GUISoDa;

GUISoDa::HamlibHandler::HamlibHandler(QObject * parent) : QObject(parent)
{
  // setup the tables
  initModTables(); 

  // setup the command tables
  initCommandTables();

  modulation = SoDa::Command::AM;
  rx_freq = 1.0e9;
  tx_freq = 1.0e9;
  tx_on = false;
  current_VFO = QString("VFOA");  
  tx_VFO = QString("VFOB");
  split_enabled = true; 
}
  
GUISoDa::HamlibHandler::~HamlibHandler() {
}


void GUISoDa::HamlibHandler::initModTables()
{
  // setup the mode map
  soda2hl_modmap[SoDa::Command::LSB] = QString("LSB");
  soda2hl_modmap[SoDa::Command::USB] = QString("USB");
  soda2hl_modmap[SoDa::Command::AM] = QString("AMS");
  soda2hl_modmap[SoDa::Command::NBFM] = QString("FM");
  soda2hl_modmap[SoDa::Command::WBFM] = QString("FMS");
  soda2hl_modmap[SoDa::Command::CW_U] = QString("CW");
  soda2hl_modmap[SoDa::Command::CW_L] = QString("CWR");  
  
  hl2soda_modmap[QString("LSB")] = SoDa::Command::LSB;
  hl2soda_modmap[QString("USB")] = SoDa::Command::USB;
  hl2soda_modmap[QString("AMS")] = SoDa::Command::AM;
  hl2soda_modmap[QString("FM")] = SoDa::Command::NBFM;
  hl2soda_modmap[QString("CW")] = SoDa::Command::CW_U;
  hl2soda_modmap[QString("CWR")]  = SoDa::Command::CW_L;
}

void GUISoDa::HamlibHandler::initCommandTables()
{
  // setup all the commands
  registerCommand("dump_state", "\\dump_state", &GUISoDa::HamlibHandler::cmdDumpState, true);
  registerCommand("v", "get_vfo", &GUISoDa::HamlibHandler::cmdVFO, true);
  registerCommand("V", "set_vfo", &GUISoDa::HamlibHandler::cmdVFO, false);
  registerCommand("chk_vfo", "\\chk_vfo", &GUISoDa::HamlibHandler::cmdChkVFO, true);    
  registerCommand("f", "get_freq", &GUISoDa::HamlibHandler::cmdFreq, true);
  registerCommand("F", "set_freq", &GUISoDa::HamlibHandler::cmdFreq, false);  
  registerCommand("i", "get_split_freq", &GUISoDa::HamlibHandler::cmdSplitFreq, true);
  registerCommand("I", "set_split_freq", &GUISoDa::HamlibHandler::cmdSplitFreq, false);  
  registerCommand("x", "get_split_mode", &GUISoDa::HamlibHandler::cmdMode, true);
  registerCommand("X", "set_split_mode", &GUISoDa::HamlibHandler::cmdMode, false);  
  registerCommand("m", "get_mode", &GUISoDa::HamlibHandler::cmdMode, true);
  registerCommand("M", "set_mode", &GUISoDa::HamlibHandler::cmdMode, false);  
  registerCommand("t", "get_ptt", &GUISoDa::HamlibHandler::cmdPTT, true);
  registerCommand("T", "set_ptt", &GUISoDa::HamlibHandler::cmdPTT, false);
  registerCommand("s", "get_split_vfo", &GUISoDa::HamlibHandler::cmdSplitVFO, true);
  registerCommand("S", "set_split_vfo", &GUISoDa::HamlibHandler::cmdSplitVFO, false);  
  registerCommand("q", "exit", &GUISoDa::HamlibHandler::cmdQuit, true);
  registerCommand("Q", "quit", &GUISoDa::HamlibHandler::cmdQuit, true);    
}

void GUISoDa::HamlibHandler::registerCommand(const char * shortname, 
		       const char * longname,
		       cmdHandler_t handler, 
		       bool is_get)
{
  QString sn(shortname); 
  QString ln(longname);
  if(is_get) {
    qDebug() << QString("registering get command [%1]\n").arg(ln);    
    get_command_map[sn] = handler; 
    get_command_map[ln] = handler; 
  }
  else {
    qDebug() << QString("registering set command [%1]\n").arg(ln);
    set_command_map[sn] = handler; 
    set_command_map[ln] = handler; 
  }
}


void GUISoDa::HamlibHandler::processCommand(const QString & cmd, QTcpSocket * socket_p)
{
  // first chop the current command up into tokens
  QStringList cmd_list = cmd.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);

  if (cmd_list.size() == 0) return;

  QTextStream out(socket_p);
  QString lcmd = cmd; 
  QTextStream in(&lcmd, QIODevice::ReadOnly);

  // repeat as long as there's something in the input stream.
  while(!in.atEnd()) {
    QString cmdkey; 

    in >> cmdkey; 

    if(cmdkey.size() == 0) continue; 
    if((cmdkey[0] != 'g') && (cmdkey[0] != 's') && (cmdkey[0] != '\\')) {
      for(int i = 0; i < cmdkey.size(); i++) {
	QString cmdchar = cmdkey.mid(i,1);
	if(set_command_map.count(cmdchar) != 0) {
	  (this->*set_command_map[cmdchar])(out, in, false);
	}
	else if(get_command_map.count(cmdchar) != 0) {
	  (this->*get_command_map[cmdchar])(out, in, true);
	}
	else {
	  qDebug() << QString("HAMLIB handler can't deal with this char command [%1]").arg(cmdchar);
	  out << "RPRT " << RIG_EINVAL << endl;
	}
      }
    }
    else if(set_command_map.count(cmdkey) != 0) {
      (this->*set_command_map[cmdkey])(out, in, false);
    }
    else if(get_command_map.count(cmdkey) != 0) {
      (this->*get_command_map[cmdkey])(out, in, true);
    }
    else {
      qDebug() << QString("HAMLIB handler can't deal with this str command [%1]").arg(cmd);
      for(auto & el : set_command_map) {
	qDebug() << QString("setmap [%1]").arg(el.first);
      }
      for(auto el : get_command_map) {
	qDebug() << QString("getmap [%1]").arg(el.first);
      }
      out << "RPRT " << RIG_EINVAL << endl;
    }
  }
}

bool GUISoDa::HamlibHandler::cmdDumpState(QTextStream & out, QTextStream & in, bool getval)
{
  (void) in;
  (void) getval; 
  out << "0\n"; // protocol version
  out << "1 \n"; // seems to be ignored...
  out << "2 \n"; // ITU region

  double rx_freq_min = 1.0;
  double rx_freq_max = 1.0e12;
  double tx_freq_min = 1.0; 
  double tx_freq_max = 1.0e12;

  // rmode_t vfo_t ant_t

  // now the frequency ranges
  // RX
  int mode_mask = RIG_MODE_AM | RIG_MODE_CW | 
    RIG_MODE_USB | RIG_MODE_LSB | 
    RIG_MODE_FM | RIG_MODE_WFM | RIG_MODE_CWR;  

  out << QString("%1 %2 ").arg(rx_freq_min, 15, 'f').arg(rx_freq_max, 15, 'f');
  out << QString("0x%1 %2 %3 ").arg(mode_mask, 0, 16).arg(-1).arg(-1);
  out << QString("0x%1 0x%2\n").arg(RIG_VFO_A | RIG_VFO_B, 0, 16).arg(RIG_ANT_1 | RIG_ANT_2, 0, 16);
  out << "0 0 0 0 0 0 0\n";

  // TX
  out << QString("%1 %2 ").arg(tx_freq_min, 15, 'f').arg(tx_freq_max, 15, 'f');
  out << QString("0x%1 %2 %3 ").arg(mode_mask, 0, 16).arg(1).arg(200);
  out << QString("0x%1 0x%2\n").arg(RIG_VFO_A | RIG_VFO_B, 0, 16).arg(RIG_ANT_1, 0, 16);
  out << "0 0 0 0 0 0 0\n";

  // now tuning steps
  int ssb_mask = (RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_CWR );
  int amfm_mask = (RIG_MODE_AM | RIG_MODE_FM | RIG_MODE_WFM);
  out << QString("0x%1 %2\n").arg(ssb_mask, 0, 16).arg(1);
  out << QString("0x%1 %2\n").arg(amfm_mask, 0, 16).arg(100);
  out << "0 0\n";

  // now filters
  out << QString("0x%1 %2\n").arg(ssb_mask, 0, 16).arg(100);
  out << QString("0x%1 %2\n").arg(ssb_mask, 0, 16).arg(500);
  out << QString("0x%1 %2\n").arg(ssb_mask | amfm_mask, 0, 16).arg(2000);
  out << QString("0x%1 %2\n").arg(ssb_mask | amfm_mask, 0, 16).arg(6000);    
  out << "0 0\n";

  // max RIT
  // mast XIT
  // max IF shift
  out << "1000\n1000\n1000\n";

  // we don't have a speech synthesizer to announce frequencies
  out << "0\n";

  // preamp list
  out << "0\n";

  // attenuator list
  out << "5 10 15 20 25 30 35\n";

  // has_get_func has_set_func
  out << QString("0x%1\n0x%1\n").arg(RIG_FUNC_NONE, 0, 16);
  // has get/set level
  out << QString("0x%1\n0x%1\n").arg(RIG_LEVEL_NONE, 0, 16);
  // has set/get_param
  out << QString("0x%1\n0x%1\n").arg(RIG_PARM_NONE, 0, 16);

  return true;
}

bool GUISoDa::HamlibHandler::cmdVFO(QTextStream & out, QTextStream & in, bool getval)
{
  if(getval) {
    out << current_VFO << endl;
  }
  else {
    in >> current_VFO;
    out << "RPRT 0" << endl; 
  }
  return true;
}

bool GUISoDa::HamlibHandler::cmdChkVFO(QTextStream & out, QTextStream & in, bool getval)
{
  out << "CHKVFO 0" << endl;
  return true;
}

bool GUISoDa::HamlibHandler::cmdFreq(QTextStream & out, QTextStream & in, bool getval)
{
  if(getval) {
    QString resp = QString("%1").arg(rx_freq, 15, 'f');
    out << resp << endl; 
  }
  else {
    double setfreq; 
    in >> setfreq;
    if(!split_enabled || (current_VFO == "VFOA")) {
      rx_freq = setfreq;
      emit setRXFreq(setfreq);             
    }
    else {
      tx_freq = setfreq;
      emit setTXFreq(setfreq);       
    }
    out << "RPRT 0" << endl; 

  }
  return true;  
}

bool GUISoDa::HamlibHandler::cmdSplitFreq(QTextStream & out, QTextStream & in, bool getval)
{
  if(getval) {
    QString resp = QString("%1").arg(tx_freq, 15, 'f');
    out << resp << endl; 
  }
  else {
    double setfreq; 
    in >> setfreq; 
    tx_freq = setfreq;
    emit setTXFreq(setfreq);       
    out << "RPRT 0" << endl; 
  }
  return true;  
}


bool GUISoDa::HamlibHandler::cmdMode(QTextStream & out, QTextStream & in, bool getval)
{
  if(getval) {
    out << soda2hl_modmap[modulation] << endl << 6000 << endl;
  }
  else {
    QString req_mod; 
    int passband; 
    in >> req_mod; 
    if(req_mod == "?") {
      // tell them what we've got...
      QString delim = "";
      for(auto & mp : hl2soda_modmap)
      {
        out << delim << mp.first;
        delim = " ";
      }
      out << endl;
      out << "RPRT 0" << endl;
    }
    else {
      in >> passband;
      if (hl2soda_modmap.count(req_mod) != 0) {
	emit setModulation(hl2soda_modmap[req_mod]);
	out << "RPRT 0" << endl;
      }
      else {
	out << "RPRT " << RIG_EINVAL << endl; 
      }
    }
  }
  return true;  
}

bool GUISoDa::HamlibHandler::cmdPTT(QTextStream & out, QTextStream & in, bool getval)
{
  if(getval) {
    QString tx_state = tx_on ? "1" : "0";
    out << tx_state << endl; 
    out << "RPRT 0" << endl;
  }
  else {
    int tx_sel; 
    in >> tx_sel; 
    tx_on = (tx_sel != 0); 
    out << "RPRT 0" << endl;  
    emit setTXOn(tx_on); 
  }
  return true;  
}

bool GUISoDa::HamlibHandler::cmdSplitVFO(QTextStream & out, QTextStream & in, bool getval)
{
  if(getval) {
    QString se = split_enabled ? "1" : "0"; 
    out << se << endl << tx_VFO << endl; 
    out << "RPRT 0" << endl; 
  }
  else {
    QString split_ena; 
    in >> split_ena >> tx_VFO; 
    split_enabled = (split_ena == "1");
    out << "RPRT 0" << endl; 
  }
  return true; 
}



bool GUISoDa::HamlibHandler::cmdQuit(QTextStream & out, QTextStream & in, bool getval)
{
  (void) in;
  (void) getval; 
  out << "q" << endl;
  return false; 
}


void GUISoDa::HamlibHandler::reportRXFreq(double freq) {
  rx_freq = freq; 
}

void GUISoDa::HamlibHandler::reportTXFreq(double freq) {
  tx_freq = freq; 
}

void GUISoDa::HamlibHandler::reportModulation(int mod_id)
{
  modulation = (SoDa::Command::ModulationType) mod_id;
  
}

void GUISoDa::HamlibHandler::reportTXOn(bool _tx_on) 
{
  tx_on = _tx_on; 
}
