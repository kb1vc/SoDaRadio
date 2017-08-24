#include "soda_hamlib_server.hpp"
#include <QMessageBox>
#include <boost/format.hpp>

#include <hamlib/rig.h>

SoDaHamlibServer::SoDaHamlibServer(QObject * parent, int _port_num) :
  QTcpServer(parent), port_num(_port_num)  {

  // setup the tables
  initModTables(); 

  // setup the command tables
  initCommandTables();

  // make the connections
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
  for(int i = 0; i < lim; i++) {
    if(as[i] == '\n') {
      std::cerr << QString("Process this: [%1]\n").arg(current_command).toStdString();
      processCommand();
      current_command = QString("");
    }
    else {
      current_command.append(as[i]);
    }
  }
  std::cerr << std::endl;
}

void SoDaHamlibServer::tcpError(QAbstractSocket::SocketError error) {
  QMessageBox::warning( (QWidget *)this->parent(), tr("Error"),tr("TCP error: %1").arg( server_socket.errorString() ) );
}

bool SoDaHamlibServer::start() {
  std::cerr << "Got to START in hamlib server.\n";
  
  if( !this->listen( QHostAddress::LocalHost, port_num ) ) {
    QMessageBox::warning( (QWidget *)this->parent(), tr("Error!"), tr("Cannot listen to port %1").arg(port_num) );
  }
  else {
    std::cerr << "HAMLIB SERVER is running on port " << port_num << "  "  << this->serverPort() << std::endl; 
    return true;
  }
}

void SoDaHamlibServer::incomingConnection(qintptr descriptor) {
  std::cerr << boost::format("HAMLIB SERVER got incoming connection descriptor = %d\n") % descriptor; 
  if( !server_socket.setSocketDescriptor( descriptor ) ) {
    QMessageBox::warning( (QWidget *)this->parent(), tr("Error!"), tr("Socket error!") );
    return;
  }
}

void SoDaHamlibServer::initModTables()
{
  // setup the mode map
  soda2hl_modmap[SoDa::Command::LSB] = QString("LSB");
  soda2hl_modmap[SoDa::Command::USB] = QString("USB");
  soda2hl_modmap[SoDa::Command::AM] = QString("AMS");
  soda2hl_modmap[SoDa::Command::NBFM] = QString("FM");
  soda2hl_modmap[SoDa::Command::WBFM] = QString("FMS");
  soda2hl_modmap[SoDa::Command::CW_U] = QString("CW");
  soda2hl_modmap[SoDa::Command::CW_L] = QString("CWR");  
  
  hl2soda_modmap[QString("LSB")] = SoDa::Command::LSB;
  hl2soda_modmap[QString("USB")] = SoDa::Command::USB;
  hl2soda_modmap[QString("AMS")] = SoDa::Command::AM;
  hl2soda_modmap[QString("FM")] = SoDa::Command::NBFM;
  hl2soda_modmap[QString("CW")] = SoDa::Command::CW_U;
  hl2soda_modmap[QString("CWR")]  = SoDa::Command::CW_L;
}

void SoDaHamlibServer::initCommandTables()
{
  // setup all the commands
  registerCommand("", "\\dump_state", &SoDaHamlibServer::cmdDumpState, true);
  registerCommand("v", "get_vfo", &SoDaHamlibServer::cmdVFO, true);
  registerCommand("V", "set_vfo", &SoDaHamlibServer::cmdVFO, false);  
  current_VFO = QString("Main");
  registerCommand("f", "get_freq", &SoDaHamlibServer::cmdFreq, true);
  registerCommand("F", "set_freq", &SoDaHamlibServer::cmdFreq, false);  
  registerCommand("m", "get_mode", &SoDaHamlibServer::cmdMode, true);
  registerCommand("M", "set_mode", &SoDaHamlibServer::cmdMode, false);  
  registerCommand("t", "get_ptt", &SoDaHamlibServer::cmdPTT, true);
  registerCommand("T", "set_ptt", &SoDaHamlibServer::cmdPTT, false);  
}

void SoDaHamlibServer::registerCommand(const char * shortname, 
		       const char * longname,
		       cmdHandler_t handler, 
		       bool is_get)
{
  QString sn(shortname); 
  QString ln(longname);
  if(is_get) {
    get_command_map[sn] = handler; 
    get_command_map[ln] = handler; 
  }
  else {
    set_command_map[sn] = handler; 
    set_command_map[ln] = handler; 
  }
}


bool SoDaHamlibServer::processCommand()
{
  // first chop the current command up into tokens
  QStringList cmd_list = current_command.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);

  qDebug() << "got cmd list [" << cmd_list << "]\n";
  if (cmd_list.size() == 0) return false; 

  QString cmdkey = cmd_list.at(0);
  qDebug() << QString("cmdkey = [%1]\n").arg(cmdkey);
  if(set_command_map.count(cmdkey) != 0) {
    qDebug() << "In set_command\n";
    (this->*set_command_map[cmdkey])(cmd_list, false);
  }
  else if(get_command_map.count(cmdkey) != 0) {
    qDebug() << "In get_command\n";    
    (this->*get_command_map[cmdkey])(cmd_list, true);
  }
  else {
    std::cerr << QString("HAMLIB server can't deal with this command [%1]\n").arg(current_command).toStdString();
  }
}

bool SoDaHamlibServer::cmdDumpState(const QStringList & cmd_vec, bool getval)
{
  std::cerr << "\n\n!!!!IN CMD DUMP STATE!!!!!!!\n\n";
  server_socket.write("Hello world!\n");
#if 0  
  std::string resp; 

  resp = "0\n"; // protocol version
  resp += "1 \n"; // seems to be ignored...
  resp += "2 \n"; // ITU region



  // rmode_t vfo_t ant_t

  // now the frequency ranges
  // RX
  resp += (boost::format("%15f %15f 0x%x %d %d 0x%x 0x%x\n")
	   % rx_freq_min
	   % rx_freq_max
	   % (RIG_MODE_AM | RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_FM | RIG_MODE_WFM | RIG_MODE_CWR )
	   % -1  // this is rx  low_power
	   % -1  // this is rx high_power
	   % (RIG_VFO_A | RIG_VFO_B) // vfo_mask
	   % (RIG_ANT_1 | RIG_ANT_2)
	   ).str();
  resp += "0 0 0 0 0 0 0\n";

  // TX
  resp += (boost::format("%15f %15f 0x%x %d %d 0x%x 0x%x\n")
	   % tx_freq_min
	   % tx_freq_max
	   % (RIG_MODE_AM | RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_FM | RIG_MODE_WFM | RIG_MODE_CWR )
	   % 1  // this is TX 1 mW
	   % 200  // this is TX 200 mW
	   % (RIG_VFO_A | RIG_VFO_B) // vfo_mask
	   % RIG_ANT_1
	   ).str();

  resp += "0 0 0 0 0 0 0\n";

  // now tuning steps
  resp += (boost::format("0x%x %d\n")
	   % (RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_CWR )
	   % 1
	   ).str();
  resp += (boost::format("0x%x %d\n")
	   % (RIG_MODE_AM | RIG_MODE_FM | RIG_MODE_WFM)
	   % 100
	   ).str();
  resp += "0 0\n";

  // now filters
  resp += (boost::format("0x%x %d\n") 
	   % (RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_CWR )  
	   % 100).str(); 
  resp += (boost::format("0x%x %d\n") 
	   % (RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_CWR )  
	   % 500).str(); 
  resp += (boost::format("0x%x %d\n") 
	   % (RIG_MODE_AM | RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_FM | RIG_MODE_CWR )  
	   % 2000).str(); 
  resp += (boost::format("0x%x %d\n") 
	   % (RIG_MODE_AM | RIG_MODE_CW | RIG_MODE_USB | RIG_MODE_LSB | RIG_MODE_FM | RIG_MODE_WFM | RIG_MODE_CWR )  
	   % 6000).str(); 

  resp += "0 0\n";

  // max RIT
  // mast XIT
  // max IF shift
  resp += "1000\n1000\n1000\n";

  // we don't have a speech synthesizer to announce frequencies
  resp += "0\n";

  // preamp list
  resp += "0\n";

  // attenuator list
  resp += "5 10 15 20 25 30 35\n";

  // has_get_func
  resp += (boost::format("0x%x\n") 
	   % (RIG_FUNC_NONE)).str();
  // has_set_func
  resp += (boost::format("0x%x\n") 
	   % (RIG_FUNC_NONE)).str();
  // has get level
  resp += (boost::format("0x%x\n") 
	   % (RIG_LEVEL_NONE)).str();
  // has set level
  resp += (boost::format("0x%x\n") 
	   % (RIG_LEVEL_NONE)).str();

  // has get_param
  resp += (boost::format("0x%x\n") 
	   % (RIG_PARM_NONE)).str();

  // has set param
  resp += (boost::format("0x%x\n") 
	   % (RIG_PARM_NONE)).str();

  QString qresp(resp.c_str());

#endif  

  return true;
  
}

bool SoDaHamlibServer::cmdVFO(const QStringList & cmd_vec, bool getval)
{
}

bool SoDaHamlibServer::cmdFreq(const QStringList & cmd_vec, bool getval)
{
}

bool SoDaHamlibServer::cmdMode(const QStringList & cmd_vec, bool getval)
{
}

bool SoDaHamlibServer::cmdPTT(const QStringList & cmd_vec, bool getval)
{
}

