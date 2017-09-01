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

#include "soda_hamlib_server.hpp"
#include "soda_hamlib_handler.hpp"
#include "soda_hamlib_listener.hpp"

#include <QMessageBox>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <hamlib/rig.h>

SoDaHamlibServer::SoDaHamlibServer(QObject * parent, int _port_num) :
  QTcpServer(parent), port_num(_port_num)  {

  handler_p = new SoDaHamlibHandler(this);
}
  
SoDaHamlibServer::~SoDaHamlibServer() {

  emit stopListeners();

  BOOST_FOREACH(SoDaHamlibListener * l, listener_list) {
    l->wait();
  }
}

bool SoDaHamlibServer::start() {
  if( !this->listen( QHostAddress::LocalHost, port_num ) ) {
    QMessageBox::critical( (QWidget *)this->parent(), tr("Error!"), tr("Cannot listen to port %1").arg(port_num) );
  }
}

void SoDaHamlibServer::incomingConnection(qintptr descriptor) {
  std::cerr << boost::format("HAMLIB SERVER got incoming connection descriptor = %d\n") % descriptor; 
  
  // create a new listener
  SoDaHamlibListener * listener = new SoDaHamlibListener(descriptor, handler_p, this);

  // make sure the thread terminates itself
  connect(listener, SIGNAL(finished()), listener, SLOT(deleteLater()));
  connect(this, SIGNAL(stopListeners()), listener, SLOT(setFinished()));

  listener_list.push_back(listener); 

  listener->start(); // start the thread -- it will wait for data from the client.
}


