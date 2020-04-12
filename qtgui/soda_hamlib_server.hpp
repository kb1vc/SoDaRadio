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

#ifndef SODA_HAMLIB_SERVER_HEADER
#define SODA_HAMLIB_SERVER_HEADER
#include <QObject>
#include <QString>
#include <QtNetwork/QtNetwork>
#include <iostream>
#include <errno.h>
#include "soda_hamlib_listener.hpp"
#include "soda_hamlib_handler.hpp"
#include "../src/Command.hxx"

 // This, and soda_hamlib_listener are taken 
 // from a pattern at https://gist.github.com/lamprosg/4587087
 // nicely done and documented. 
 // 

namespace GUISoDa {
  class HamlibServer : public QTcpServer {
    Q_OBJECT

  public:
    HamlibServer(QObject * parent = 0, int _port_num = 4575);

    ~HamlibServer();

    HamlibHandler * getHandler() { return handler_p; }

  signals:
    void stopListeners();
		      
  public slots:
    // start listening for the first incoming connection.
    bool start(); 


  protected:
    void incomingConnection(qintptr desc);
    int port_num;

    HamlibHandler * handler_p; 

    std::list<HamlibListener *> listener_list;
  };
}
#endif
