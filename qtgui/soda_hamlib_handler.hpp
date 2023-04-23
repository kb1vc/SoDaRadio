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

#ifndef SODA_HAMLIB_HANDLER_HEADER
#define SODA_HAMLIB_HANDLER_HEADER
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QtNetwork/QtNetwork>
#include <iostream>
#include <errno.h>

#include "../src/Command.hxx"

namespace GUISoDa {
  
  class HamlibHandler : public QObject {
    Q_OBJECT

  public:
    HamlibHandler(QObject * parent = 0);

    ~HamlibHandler();

    void processCommand(const QString & cmd, QTcpSocket * socket_p); 
		      
  public slots:
    void reportRXFreq(double f);
    void reportTXFreq(double f);
    void reportModulation(int mod_id);
    void reportTXOn(bool tx_on);
  
  signals:
    void setRXFreq(double f);
    void setTXFreq(double f);  
    void setModulation(SoDa::Command::ModulationType mod);
    void setTXOn(bool tx_on);

  protected:
    double rx_freq;
    double tx_freq;
    bool tx_on;
    SoDa::Command::ModulationType modulation; 

  protected:
  
    // command handlers
    typedef bool(HamlibHandler::*cmdHandler_t)(QTextStream &, QTextStream &, bool);
    std::map<QString,  cmdHandler_t> set_command_map;
    std::map<QString,  cmdHandler_t> get_command_map;     
    void registerCommand(const char * shortname, 
			 const char * longname,
			 cmdHandler_t handler, 
			 bool is_get);

    std::map<SoDa::Command::ModulationType, QString> soda2hl_modmap;
    std::map<QString, SoDa::Command::ModulationType> hl2soda_modmap;

    bool cmdDumpState(QTextStream & out, QTextStream & in, bool getval);
    bool cmdVFO(QTextStream & out, QTextStream & in, bool getval);
    bool cmdFreq(QTextStream & out, QTextStream & in, bool getval);
    bool cmdSplitFreq(QTextStream & out, QTextStream & in, bool getval);  
    bool cmdMode(QTextStream & out, QTextStream & in, bool getval);
    bool cmdPTT(QTextStream & out, QTextStream & in, bool getval);
    bool cmdSplitVFO(QTextStream & out, QTextStream & in, bool getval);  
    bool cmdCheckVFO(QTextStream & out, QTextStream & in, bool getval);  
    bool cmdQuit(QTextStream & out, QTextStream & in, bool getval);  

    // state of the radio
    QString current_VFO, tx_VFO;
    bool split_enabled; 
    double current_rx_freq; 
    double current_tx_freq; 
    SoDa::Command::ModulationType  current_modtype; 
    bool current_tx_on; 
  
  private:
    void initModTables();
    void initCommandTables();    
  };
}
#endif
