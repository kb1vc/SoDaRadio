#ifndef SODA_HAMLIB_HANDLER_HEADER
#define SODA_HAMLIB_HANDLER_HEADER
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QtNetwork/QtNetwork>
#include <iostream>
#include <boost/format.hpp>
#include <errno.h>

#include "../src/Command.hxx"


class SoDaHamlibHandler : public QObject {
  Q_OBJECT

public:
  SoDaHamlibHandler(QObject * parent = 0);

  ~SoDaHamlibHandler();

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
  typedef bool(SoDaHamlibHandler::*cmdHandler_t)(QTextStream &, QTextStream &, bool);
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

#endif
