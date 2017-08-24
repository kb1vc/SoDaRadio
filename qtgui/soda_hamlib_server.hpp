#ifndef SODA_HAMLIB_SERVER_HEADER
#define SODA_HAMLIB_SERVER_HEADER
#include <QObject>
#include <QString>
#include <QtNetwork/QtNetwork>
#include <iostream>
#include <boost/format.hpp>
#include <errno.h>

#include "../src/Command.hxx"


class SoDaHamlibServer : public QTcpServer {
  Q_OBJECT

public:
  SoDaHamlibServer(QObject * parent = 0, int _port_num = 4575);

  ~SoDaHamlibServer();

  QTcpSocket server_socket;

public slots:
  void tcpReady();
  void tcpError( QAbstractSocket::SocketError error);
  
  // start listening for the first incoming connection.
  bool start(); 

signals:
  void setFreq(double f);
  void setModulation(SoDa::Command::ModulationType mod);
  void transmitOn(bool tx_on);

protected:
  void incomingConnection(qintptr desc);
  int port_num;

  
  // command handlers
  typedef bool(SoDaHamlibServer::*cmdHandler_t)(const QStringList &, bool);
  std::map<QString,  cmdHandler_t> set_command_map;
  std::map<QString,  cmdHandler_t> get_command_map;     
  void registerCommand(const char * shortname, 
		       const char * longname,
		       cmdHandler_t handler, 
		       bool is_get);

  std::map<SoDa::Command::ModulationType, QString> soda2hl_modmap;
  std::map<QString, SoDa::Command::ModulationType> hl2soda_modmap;

  bool processCommand(); 
  QString current_command; 

  bool cmdDumpState(const QStringList & cmd_vec, bool getval);
  bool cmdVFO(const QStringList & cmd_vec, bool getval);
  bool cmdFreq(const QStringList & cmd_vec, bool getval);
  bool cmdMode(const QStringList & cmd_vec, bool getval);
  bool cmdPTT(const QStringList & cmd_vec, bool getval);


  // state of the radio
  QString current_VFO;
  double current_rx_freq; 
  double current_tx_freq; 
  SoDa::Command::ModulationType  current_modtype; 
  bool current_tx_on; 
  
private:
  void initModTables();
  void initCommandTables();    
};

#endif
