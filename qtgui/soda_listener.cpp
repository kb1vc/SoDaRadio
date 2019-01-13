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

#include "soda_listener.hpp"
#include <boost/format.hpp>

GUISoDa::Listener::Listener(QObject * parent, const QString & _socket_basename) : QObject(parent) {
  quit = false;
  socket_basename = _socket_basename; 
}

bool GUISoDa::Listener::init()
{
  tx_gain_min = 0; 
  tx_gain_max = 100; 
  cmd_socket = new QLocalSocket(this);
  // first wait for the file to be created
  QString cmd_socket_name = socket_basename + "_cmd"; 
  int wcount = 0;
  while(!QFile::exists(cmd_socket_name)) {
    QThread::sleep(5);
    wcount++; 
    if(wcount > 30) {
      qDebug() << QString("Waited %1 seconds for socket file [%2] to be created.  Is the radio process dead?").arg(wcount * 5).arg(cmd_socket_name);
      emit(fatalError(QString("No socket file [%1] found after timeout of %2 seconds").arg(cmd_socket_name).arg(wcount * 5)));
      return false;       
    }
  }

  cmd_socket->connectToServer(cmd_socket_name); 
  while(!cmd_socket->waitForConnected(1000)) {
    qDebug() << QString("Waited for connection on local socket [%1_cmd]. Is something wrong?").arg(socket_basename);
    qDebug() << cmd_socket->errorString();
    QThread::sleep(5); // sleep for 5 seconds...    
  }
    
  connect(cmd_socket, SIGNAL(readyRead()), 
	  this, SLOT(processCmd())); 
  connect(cmd_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	  this, SLOT(cmdErrorHandler(QLocalSocket::LocalSocketError)));


  spect_socket = new QLocalSocket(this);
  spect_socket->connectToServer(socket_basename + "_wfall"); 
  while(!spect_socket->waitForConnected(30000)) {
    qDebug() << QString("Waited 30 seconds for connection on local socket [%1_wfall]. Is something wrong?").arg(socket_basename);
    qDebug() << cmd_socket->errorString();
    QThread::sleep(5); // sleep for 5 seconds...    
  }
    
  connect(spect_socket, SIGNAL(readyRead()), 
	  this, SLOT(processSpectrum())); 
  connect(spect_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	  this, SLOT(cmdErrorHandler(QLocalSocket::LocalSocketError)));

  spect_buffer_len = 0; 

  return true; 
}

void GUISoDa::Listener::start()
{
  put(SoDa::Command(SoDa::Command::GET, SoDa::Command::HWMB_REP));
  put(SoDa::Command(SoDa::Command::GET, SoDa::Command::TX_GAIN_RANGE));  
  return; 
}

int GUISoDa::Listener::put(const char * buf, int len)
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

int GUISoDa::Listener::get(char * buf, int maxlen)
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

bool GUISoDa::Listener::get(SoDa::Command & cmd)
{
  int len = get((char*) &cmd, sizeof(SoDa::Command));
  return len > 0; 
}

void GUISoDa::Listener::setupSpectrumBuffer(double cfreq, double span, long buflen)
{
  (void) span; 
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

void GUISoDa::Listener::processSpectrum() {
  unsigned int rlen = spect_buffer_len * sizeof(float);
  while(((unsigned int) spect_socket->bytesAvailable()) > (sizeof(int) + rlen)) {
    unsigned int len; 
    spect_socket->read((char*) & len, sizeof(unsigned int));

    if(rlen != len) {
      char * nbuf = new char[len];
      // throw it away. 
      spect_socket->read(nbuf, len); 
      delete[] nbuf; 
    }
    else {
      spect_socket->read((char*) spect_buffer, rlen); 
      emit(updateData(spect_center_freq, spect_buffer)); 
    }
  }
}

void GUISoDa::Listener::processCmd() {
  SoDa::Command incmd; 
 
  while(((unsigned int) cmd_socket->bytesAvailable()) > sizeof(SoDa::Command)) {
    get(incmd);    
    
    if(incmd.cmd == SoDa::Command::REP) handleREP(incmd);
    else if(incmd.cmd == SoDa::Command::GET) handleGET(incmd);
    else if(incmd.cmd == SoDa::Command::SET) handleSET(incmd);    
  }
}


bool GUISoDa::Listener::put(const SoDa::Command & cmd)
{
  int len = put((char*) &cmd, sizeof(SoDa::Command));
  return len > 0;
}

void GUISoDa::Listener::setRXFreq(double freq) {
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_RETUNE_FREQ, freq))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
  current_rx_freq = freq;   
}

void GUISoDa::Listener::setTXFreq(double freq) {
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_RETUNE_FREQ, freq))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
  current_tx_freq = freq; 
}

void GUISoDa::Listener::setRXGain(int gain) {
  double dgain = gain;   
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_RF_GAIN, dgain))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setTXGain(int gain) {
  // gain is relative to max -- so we subtract from max gain.  
  double dgain = ((double) gain);  
  
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_RF_GAIN, dgain))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setAFGain(int gain) {
  double dgain = gain; 
  // this is a little complex...
  dgain = 50.0 * (log10(dgain) / log10(100.0));
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_GAIN, dgain))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setAFSidetoneGain(int gain) {
  double dgain = gain;   
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_SIDETONE_GAIN, dgain))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}


void GUISoDa::Listener::setModulation(int mod_id)
{

  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_MODE, mod_id))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_MODE, mod_id))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setAFFilter(int id)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_FILTER, id))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

bool GUISoDa::Listener::handleREP(const SoDa::Command & cmd) 
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
  case SoDa::Command::GPS_UTC:
    emit(repGPSTime(cmd.iparms[0], cmd.iparms[1], cmd.iparms[2]));
    break; 
  case SoDa::Command::GPS_LATLON:
    emit(repGPSLatLon(cmd.dparms[0], cmd.dparms[1]));
    break; 
  case SoDa::Command::TX_GAIN_RANGE:
    emit(repGainRange(cmd.dparms[0], cmd.dparms[1])); 
    tx_gain_min = cmd.dparms[0]; 
    tx_gain_max = cmd.dparms[1]; 
    qDebug() << QString("TX Gain Range [%1] to [%2]").arg(cmd.dparms[0], 10, 'f').arg(cmd.dparms[1], 10, 'f');
    break;
  default:
    break; 
  }
  return true; 
}

void GUISoDa::Listener::setRXAnt(const QString & antname)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_ANT, antname.toStdString()))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setTXAnt(const QString & antname)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_ANT, antname.toStdString()))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setSpectrumCenter(double freq) 
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_CENTER_FREQ, freq))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setSpectrumUpdateRate(int rate)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_UPDATE_RATE, rate))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}  

void GUISoDa::Listener::setSpectrumAvgWindow(int window)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::SPEC_AVG_WINDOW, window))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setCWSpeed(int speed)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_SPEED, speed))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setSidetoneVolume(int vol)
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RX_AF_SIDETONE_GAIN, (double) vol))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}


void GUISoDa::Listener::setClockRef(int external)
{
  int clock_source = (external != Qt::Unchecked) ? 1 : 0;
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::CLOCK_SOURCE, clock_source))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::setPTT(bool on)
{
  int tx_state = on ? 1 : 0;
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_STATE, tx_state))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::recordRF(int checkbox_state)
{
  if(checkbox_state == Qt::Checked) {
    QString fname = QString("%1.cf").arg(QDateTime::currentDateTime().toString("dd-MMM-yy_HHmmss"));
    if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RF_RECORD_START, fname.toStdString()))) {
      perror((boost::format("Failed to send SET START command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
    }
  }
  else if(checkbox_state == Qt::Unchecked) {
    if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::RF_RECORD_STOP))) {
      perror((boost::format("Failed to send SET STOP command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
    }
  }
}

void GUISoDa::Listener::setCarrier(bool on)
{
  int carrier_state = on ? 1 : 0;
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_BEACON, carrier_state))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::clearCWBuffer()
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::TX_CW_FLUSHTEXT))) {
    perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
  }
}

void GUISoDa::Listener::sendCW(const QString & txt)
{
  char cwbuf[SoDa::Command::getMaxStringLen()]; 
  int i, j; 
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
	perror((boost::format("Failed to send SET command in function %s\n") % __PRETTY_FUNCTION__).str().c_str());;
      }
      j = 0; 
    }
  }
}

bool GUISoDa::Listener::handleSET(const SoDa::Command & cmd)
{
  switch(cmd.target) {
  default:
    break; 
  }
  
  return true; 
}

bool GUISoDa::Listener::handleGET(const SoDa::Command & cmd)
{
  switch(cmd.target) {
  default:
    break; 
  }
  return true; 
}

void GUISoDa::Listener::closeRadio()
{
  if(!put(SoDa::Command(SoDa::Command::SET, SoDa::Command::STOP, 0))) {
    perror("What happened here  -- listener closeRadio?");
  }
}

