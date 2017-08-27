#ifndef SODA_HAMLIB_LISTENER_DEF
#define SODA_HAMLIB_LISTENER_DEF

#include <QThread>
#include <QtNetwork/QtNetwork>
#include "soda_hamlib_handler.hpp"

class SoDaHamlibListener : public QThread {
  Q_OBJECT
public:
  explicit SoDaHamlibListener(qintptr desc, 
			      SoDaHamlibHandler * _handler,
			      QObject *parent = 0);
  
  void run();

	    
signals:
  void error(QTcpSocket::SocketError err);

public slots:
  void readyRead();
  void disconnected();
  void setFinished() { exit(0); }

protected:
  SoDaHamlibHandler * handler_p; 
  qintptr socket_desc;
  QString current_command;
  QTcpSocket * socket_p;
};

#endif
