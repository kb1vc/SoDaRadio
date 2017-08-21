#include "soda_hamlib_server.hpp"
#include <QMessageBox>

SoDaHamlibServer::SoDaHamlibServer(QObject * parent, int _port_num) :
  QTcpServer(parent), port_num(_port_num)  {
    connect( &server_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(tcpError(QAbstractSocket::SocketError)) );
    connect( &server_socket, SIGNAL(readyRead()),
             this, SLOT(tcpReady()) );
    server_socket.setSocketOption(QAbstractSocket::KeepAliveOption, true );
}


  
SoDaHamlibServer::~SoDaHamlibServer() {
    server_socket.disconnectFromHost();
    server_socket.waitForDisconnected();
}

void SoDaHamlibServer::tcpReady() {
    QByteArray array = server_socket.read(server_socket.bytesAvailable());

    char * as = array.data();

    int lim = array.count();
    std::cerr << "NEW MESSAGE:" << std::endl; 
    for(int i = 0; i < lim; i++) {
      if((i % 32) == 0) std::cerr << std::endl; 
      if(isprint(as[i])) {
	std::cerr << as[i]; 
      }
      else {
	unsigned int kk = as[i]; 
	std::cerr << " " << kk << " "; 
      }
    }
    std::cerr << std::endl;
}

void SoDaHamlibServer::tcpError(QAbstractSocket::SocketError error) {
    QMessageBox::warning( (QWidget *)this->parent(), tr("Error"),tr("TCP error: %1").arg( server_socket.errorString() ) );
}

bool SoDaHamlibServer::start() {
    if( !this->listen( QHostAddress::Any, port_num ) ) {
        QMessageBox::warning( (QWidget *)this->parent(), tr("Error!"), tr("Cannot listen to port %1").arg(port_num) );
    }
    else
        return true;
}

void SoDaHamlibServer::incomingConnection(int descriptor) {
    if( !server_socket.setSocketDescriptor( descriptor ) ) {
        QMessageBox::warning( (QWidget *)this->parent(), tr("Error!"), tr("Socket error!") );
        return;
    }
}
