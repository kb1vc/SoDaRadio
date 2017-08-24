#include "soda_listener.hpp"

SoDaListener::SoDaListener(QObject * parent, QString socket_basename) : QObject(parent) {
  quit = false;
  std::cerr << boost::format("Connecting to server socket [%s_cmd]\n") % socket_basename.toStdString(); 
  cmd_socket = new QLocalSocket(this);
  cmd_socket->connectToServer(socket_basename + "_cmd"); 

  if(!cmd_socket->waitForConnected(3000)) {
    std::cerr << "No connection for command socket.\n";
  }
    
  connect(cmd_socket, SIGNAL(readyRead()), 
	  this, SLOT(processCmd())); 
  connect(cmd_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	  this, SLOT(cmdErrorHandler(QLocalSocket::LocalSocketError)));


  spect_socket = new QLocalSocket(this);
  spect_socket->connectToServer(socket_basename + "_wfall"); 

  if(!spect_socket->waitForConnected(3000)) {
    std::cerr << "No connection for spectrum socket.\n";
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
 
  while(cmd_socket->bytesAvailable() > sizeof(SoDa::Command)) {
    int len = get(incmd);    

    if(incmd.cmd == SoDa::Command::REP) handleREP(incmd);
    else if(incmd.cmd == SoDa::Command::GET) handleGET(incmd);
    else if(incmd.cmd == SoDa::Command::SET) handleSET(incmd);    
  }
}


bool SoDaListener::put(const SoDa::Command & cmd)
{
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

  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_MODE, mod_id))) {
    perror("What happened here?");
  }
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_MODE, mod_id))) {
    perror("What happened here?");
  }
}

void SoDaListener::setAFFilter(int id)
{
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
  case SoDa::Command::INIT_SETUP_COMPLETE:
    emit(initSetupComplete());
    break;
  case SoDa::Command::TX_STATE:
    emit(repPTT(cmd.iparms[0] == 1));
    break; 
  default:
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

void SoDaListener::setCWSpeed(int speed)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_SPEED, speed))) {
    perror("What happened here?");
  }
}

void SoDaListener::setSidetoneVolume(int vol)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_SIDETONE_GAIN, (double) vol))) {
    perror("What happened here?");
  }
}

void SoDaListener::setTXPower(int power)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_RF_GAIN, (double) power))) {
    perror("What happened here?");
  }
}

void SoDaListener::setPTT(bool on)
{
  int tx_state = on ? 1 : 0;
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_STATE, tx_state))) {
    perror("What happened here?");
  }
}

void SoDaListener::clearCWBuffer()
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_FLUSHTEXT))) {
    perror("What happened here?");
  }
}

void SoDaListener::sendCW(const QString & txt)
{
  char cwbuf[SoDa::Command::getMaxStringLen()]; 
  unsigned int i, j; 
  for(i = 0, j = 0; i <= txt.size(); i++) {
    if(i == txt.size()) {
      cwbuf[j] = '\000';
    }
    else {
      char c = txt.at(i).toLatin1();
      if(isprint(c)) {
	cwbuf[j] = c; 
      }
      else {
	cwbuf[j] = ' '; 
      }
    }

    j++; 

    if((j >= SoDa::Command::getMaxStringLen()) || (i == txt.size())) {
      if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_TEXT, cwbuf))) {
	perror("What happened here?");
      }
      j = 0; 
    }
  }
}

bool SoDaListener::handleSET(const SoDa::Command & cmd)
{
  switch(cmd.target) {
  default:
    break; 
  }
  
  return true; 
}

bool SoDaListener::handleGET(const SoDa::Command & cmd)
{
  switch(cmd.target) {
  default:
    break; 
  }
  return true; 
}

void SoDaListener::closeRadio()
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::STOP, 0))) {
    perror("What happened here  -- listener closeRadio?");
  }
}

