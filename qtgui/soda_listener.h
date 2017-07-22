#ifndef SODA_LISTENER_HEADER
#define SODA_LISTENER_HEADER
#include <QObject>
#include <QString>
#include <QtNetwork/QtNetwork>
#include <iostream>
#include <boost/format.hpp>
#include <errno.h>

namespace Ui {
  class SoDaListener;
}

class SoDaListener : public QObject {
  Q_OBJECT

public:
  SoDaListener(QObject * parent = 0, QString socket_basename = "tmp") : QObject(parent) {
    quit = false;
    std::cerr << boost::format("Connecting to server socket [%s]\n") % socket_basename.toStdString(); 
    cmd_socket = new QLocalSocket(this);
    cmd_socket->connectToServer(socket_basename); 

    if(cmd_socket->waitForConnected(3000)) {
      std::cerr << "Got connected.\n"; 
    }
    else {
      std::cerr << "No connection.\n";
    }

    
    connect(cmd_socket, SIGNAL(readyRead()), 
	    this, SLOT(getCmd())); 
    connect(cmd_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	    this, SLOT(cmdErrorHandler(QLocalSocket::LocalSocketError)));

    cmd_stream = new QDataStream(cmd_socket);
    
  }

  ~SoDaListener() {
  }

  int get(char* buf, int maxlen); 

  int put(const char * buf, int len); 

signals:
  void addModulation(const QString & modtype);

public slots:
  void setRXFreq(double freq) {
    std::cerr << boost::format("In soda listener got setRXFreq command [%g] from gui\n") % freq;
    cmd_socket->flush();
    std::string msg("SET RX_FREQ D 1e9\n");
    int len = msg.size();
    int stat = put(msg.c_str(), len);
    std::cerr << boost::format("socket write returned status = %d\n") % stat;
    if(stat < 0) perror("What happened here?");
  }
  
  void getCmd();

  void cmdErrorHandler(QLocalSocket::LocalSocketError err) {
    std::cerr << "Error [" << err << "]\n";
  }

  void closeRadio() {
    quit = true; 
  }

  void setModulation(const QString & modtype);
  
private:
  QString host_name; 
  QLocalSocket * cmd_socket;
  QDataStream * cmd_stream; 
  bool quit; 
};

#endif
