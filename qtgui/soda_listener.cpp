#include "soda_listener.hpp"

SoDaListener::SoDaListener(QObject * parent, QString socket_basename) : QObject(parent) {
  quit = false;
  std::cerr << boost::format("Connecting to server socket [%s_cmd]\n") % socket_basename.toStdString(); 
  cmd_socket = new QLocalSocket(this);
  cmd_socket->connectToServer(socket_basename + "_cmd"); 

  if(cmd_socket->waitForConnected(3000)) {
    std::cerr << "Got connected.\n"; 
  }
  else {
    std::cerr << "No connection.\n";
  }
    
  connect(cmd_socket, SIGNAL(readyRead()), 
	  this, SLOT(processCmd())); 
  connect(cmd_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	  this, SLOT(cmdErrorHandler(QLocalSocket::LocalSocketError)));


  std::cerr << boost::format("Connecting to spectrum socket [%s_wfall]\n") % socket_basename.toStdString(); 
  spect_socket = new QLocalSocket(this);
  spect_socket->connectToServer(socket_basename + "_wfall"); 

  if(spect_socket->waitForConnected(3000)) {
    std::cerr << "Got connected.\n"; 
  }
  else {
    std::cerr << "No connection.\n";
  }
    
  connect(spect_socket, SIGNAL(readyRead()), 
	  this, SLOT(processSpectrum())); 
  connect(spect_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	  this, SLOT(cmdErrorHandler(QLocalSocket::LocalSocketError)));

  spect_buffer_len = 0; 
}

void SoDaListener::start()
{
  put(SoDa::Command(SoDa::Command::GET, SoDa::Command::HWMB_REP));
  return; 
}

int SoDaListener::put(const char * buf, int len)
{
  cmd_socket->write((char*) &len, sizeof(int)); 

  unsigned int to_go = len; 
  
  const char * cp = buf; 
  while(to_go > 0) {
    int l = cmd_socket->write(cp, to_go); 
    if(l < 0) return l; 
    else {
      to_go -= l; 
      cp += l; 
    }
  }

  cmd_socket->flush();  
  return len; 
}

int SoDaListener::get(char * buf, int maxlen)
{
  int len; 
  int stat = cmd_socket->read((char*) & len, sizeof(int));

  if(stat < 0) return stat; 
 
  if(len > maxlen) len = maxlen;

  unsigned int to_go = len; 
  
  char * cp = buf; 
  while(to_go > 0) {
    int l = cmd_socket->read(cp, to_go); 
    if(l < 0) return l; 
    else {
      to_go -= l; 
      cp += l; 
    }
  }

  return len; 
}

bool SoDaListener::get(SoDa::Command & cmd)
{
  int len = get((char*) &cmd, sizeof(SoDa::Command));
  return len > 0; 
}

void SoDaListener::setupSpectrumBuffer(double cfreq, double span, long buflen)
{
  spect_center_freq = cfreq; 
  std::cerr << boost::format("Got spectrum buffer update  buflen = %d\n") % buflen; 
  if(spect_buffer_len < buflen) {
    if(spect_buffer_len > 0) delete[] spect_buffer; 

    spect_buffer_len = buflen; 
    spect_buffer = new float[buflen]; 
  }
  else {
    spect_buffer_len = buflen; 
  }
}

void SoDaListener::processSpectrum() {
  int rlen = spect_buffer_len * sizeof(float);
  while(spect_socket->bytesAvailable() > (sizeof(int) + rlen)) {
    int len; 
    int stat = spect_socket->read((char*) & len, sizeof(int));

    if(rlen != len) {
      char * nbuf = new char[len];
      // throw it away. 
      spect_socket->read(nbuf, len); 
      delete[] nbuf; 
    }
    else {
      stat = spect_socket->read((char*) spect_buffer, rlen); 
      emit(updateData(spect_center_freq, spect_buffer)); 
    }
  }
}

void SoDaListener::processCmd() {
  SoDa::Command incmd; 
  std::cerr << "In processCmd\n";
 
  while(cmd_socket->bytesAvailable() > sizeof(SoDa::Command)) {
    int len = get(incmd);    
    std::cerr << boost::format("Got message size = %d content = [%s]\n") 
      % len % incmd.toString();
    if(incmd.cmd == SoDa::Command::REP) handleREP(incmd);
    else if(incmd.cmd == SoDa::Command::GET) handleGET(incmd);
    else if(incmd.cmd == SoDa::Command::SET) handleSET(incmd);    
  }
  std::cerr << "Leaving processCmd\n";  
}


bool SoDaListener::put(const SoDa::Command & cmd)
{
  std::cerr << boost::format("listener sending [%s]\n") % cmd.toString();
  
  int len = put((char*) &cmd, sizeof(SoDa::Command));
  return len > 0;
}

void SoDaListener::setRXFreq(double freq) {
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_RETUNE_FREQ, freq))) {
    perror("What happened here?");
  }
  current_rx_freq = freq;   
}

void SoDaListener::setTXFreq(double freq) {
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_RETUNE_FREQ, freq))) {
    perror("What happened here?");
  }
  current_tx_freq = freq; 
}

void SoDaListener::setRXGain(int gain) {
  double dgain = gain;   
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_RF_GAIN, dgain))) {
    perror("What happened here?");
  }
}

void SoDaListener::setTXGain(int gain) {
  double dgain = gain;   
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_RF_GAIN, dgain))) {
    perror("What happened here?");
  }
}

void SoDaListener::setAFGain(int gain) {
  double dgain = gain; 
  // this is a little complex...
  dgain = 50.0 * (log10(dgain) / log10(100.0));
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_GAIN, dgain))) {
    perror("What happened here?");
  }
}

void SoDaListener::setAFSidetoneGain(int gain) {
  double dgain = gain;   
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_SIDETONE_GAIN, dgain))) {
    perror("What happened here?");
  }
}


void SoDaListener::setModulation(int mod_id)
{
  std::cerr << "in listener setModulation\n";  
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_MODE, mod_id))) {
    perror("What happened here?");
  }
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_MODE, mod_id))) {
    perror("What happened here?");
  }
}

void SoDaListener::setAFFilter(int id)
{
  std::cerr << "in listener setAFFilter\n";
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_FILTER, id))) {
    perror("What happened here?");
  }
}

bool SoDaListener::handleREP(const SoDa::Command & cmd) 
{
  switch(cmd.target) {
  case SoDa::Command::MOD_SEL_ENTRY:
    emit(addModulation(QString(cmd.sparm), cmd.tag));
    break; 
  case SoDa::Command::AF_FILT_ENTRY:
    emit(addFilterName(QString(cmd.sparm), cmd.tag));
    break; 
  case SoDa::Command::RX_ANT_NAME:
    emit(addRXAntName(QString(cmd.sparm)));
    break; 
  case SoDa::Command::TX_ANT_NAME:
    emit(addTXAntName(QString(cmd.sparm)));
    break; 
  case SoDa::Command::SPEC_DIMS:
    emit(configureSpectrum(cmd.dparms[0], cmd.dparms[1], ((long) cmd.dparms[2])));
    setupSpectrumBuffer(cmd.dparms[0], cmd.dparms[1], (long) cmd.dparms[2]);
    break; 
  case SoDa::Command::HWMB_REP:
    emit(repHWMBVersion(QString(cmd.sparm)));
    break; 
  case SoDa::Command::SDR_VERSION:
    emit(repSDRVersion(QString(cmd.sparm)));
    break; 
  case SoDa::Command::RX_AF_FILTER_SHAPE:
    emit(repMarkerOffset(cmd.dparms[0], cmd.dparms[1])); 
    break; 
  default:
    std::cerr << boost::format("Ignoring incoming REP command: [%s]\n") % cmd.toString();
    break; 
  }
  return true; 
}

void SoDaListener::setRXAnt(const QString & antname)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_ANT, antname.toStdString()))) {
    perror("What happened here?");
  }
}

void SoDaListener::setTXAnt(const QString & antname)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_ANT, antname.toStdString()))) {
    perror("What happened here?");
  }
}

void SoDaListener::setSpectrumCenter(double freq) 
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_CENTER_FREQ, freq))) {
    perror("What happened here?");
  }
}

void SoDaListener::setSpectrumUpdateRate(int rate)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_UPDATE_RATE, rate))) {
    perror("What happened here?");
  }
}  

void SoDaListener::setSpectrumAvgWindow(int window)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_AVG_WINDOW, window))) {
    perror("What happened here?");
  }
}

bool SoDaListener::handleSET(const SoDa::Command & cmd)
{
  switch(cmd.target) {
  default:
    std::cerr << boost::format("Ignoring incoming SET command: [%s]\n") % cmd.toString();
    break; 
  }
  
  return true; 
}

bool SoDaListener::handleGET(const SoDa::Command & cmd)
{
  switch(cmd.target) {
  default:
    std::cerr << boost::format("Ignoring incoming GET command: [%s]\n") % cmd.toString();
    break; 
  }
  return true; 
}

void SoDaListener::closeRadio()
{
  std::cerr << "Listener is closing the radio connection\n";
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::STOP, 0))) {
    perror("What happened here?");
  }
}

