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

#include "soda_hamlib_listener.hpp"
#include "soda_hamlib_handler.hpp"


GUISoDa::HamlibListener::HamlibListener(qintptr desc, 
				       GUISoDa::HamlibHandler * _handler,
				       QObject *parent) : QThread(parent), 
							  handler_p(_handler), 
							  socket_desc(desc) 
{
  // not much else to do. 
}

void GUISoDa::HamlibListener::run()
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

void GUISoDa::HamlibListener::readyRead() {
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

void GUISoDa::HamlibListener::disconnected() {
  qDebug() << "\n\nHamlib listener got disconnect!!!\n\n";
  socket_p->deleteLater();
}
