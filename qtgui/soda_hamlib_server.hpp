#ifndef SODA_HAMLIB_SERVER_HEADER
#define SODA_HAMLIB_SERVER_HEADER
#include <QObject>
#include <QString>
#include <QtNetwork/QtNetwork>
#include <iostream>
#include <boost/format.hpp>
#include <errno.h>
#include "soda_hamlib_listener.hpp"
#include "soda_hamlib_handler.hpp"
#include "../src/Command.hxx"

// This, and soda_hamlib_listener are taken 
// from a pattern at https://gist.github.com/lamprosg/4587087
// nicely done and documented. 
// 


class SoDaHamlibServer : public QTcpServer {
  Q_OBJECT

public:
  SoDaHamlibServer(QObject * parent = 0, int _port_num = 4575);

  ~SoDaHamlibServer();

  SoDaHamlibHandler * getHandler() { return handler_p; }

signals:
  void stopListeners();
		      
public slots:
  // start listening for the first incoming connection.
  bool start(); 


protected:
  void incomingConnection(qintptr desc);
  int port_num;

  SoDaHamlibHandler * handler_p; 

  std::list<SoDaHamlibListener *> listener_list;
};

#endif
