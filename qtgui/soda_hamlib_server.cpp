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


