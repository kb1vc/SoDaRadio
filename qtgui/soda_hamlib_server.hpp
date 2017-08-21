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

protected:
  void incomingConnection(int desc);
				      
  int port_num; 
};

#endif
