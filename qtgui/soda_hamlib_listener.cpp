#include "soda_hamlib_listener.hpp"
#include "soda_hamlib_handler.hpp"


SoDaHamlibListener::SoDaHamlibListener(qintptr desc, 
				       SoDaHamlibHandler * _handler,
				       QObject *parent) : QThread(parent), 
							  handler_p(_handler), 
							  socket_desc(desc) 
{
  // not much else to do. 
}

void SoDaHamlibListener::run()
{
  socket_p = new QTcpSocket(); 

  if(!socket_p->setSocketDescriptor(socket_desc)) {
    emit error(socket_p->error()); 
    return; 
  }

  connect(socket_p, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
  connect(socket_p, SIGNAL(disconnected()), this, SLOT(disconnected()));

  exec();
}

void SoDaHamlibListener::readyRead() {
  QByteArray array = socket_p->read(socket_p->bytesAvailable());

  char * as = array.data();

  int lim = array.count();
  for(int i = 0; i < lim; i++) {
    if(as[i] == '\n') {
      handler_p->processCommand(current_command, socket_p);
      current_command = QString("");
    }
    else {
      current_command.append(as[i]);
    }
  }
}

void SoDaHamlibListener::disconnected() {
  qDebug() << "\n\nHamlib listener got disconnect!!!\n\n";
  socket_p->deleteLater();
}
