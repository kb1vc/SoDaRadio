/*
Copyright (c) 2018 Matthew H. Reilly (kb1vc)
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

#include "soda_audio_listener.hpp"
#include <boost/format.hpp>
#include <QMessageBox>

GUISoDa::AudioRXListener::AudioRXListener(QObject * parent, const QString & _socket_basename, unsigned int _sample_rate) : QIODevice(parent) {
  quit = false;
  socket_basename = _socket_basename; 
  sample_rate = _sample_rate; 

  init(); 
  initAudio(QAudioDeviceInfo::defaultOutputDevice());
}

bool GUISoDa::AudioRXListener::init()
{
  // create the circular buffer
  audio_cbuffer_p = new SoDa::CircularBuffer<char>(sample_rate * 10); 

  // create the rx input buffer
  rx_in_buf_len = 16 * 1024; // bigger than the largest anticipated packet
  rx_in_buf = new char(rx_in_buf_len); 
  
  audio_rx_socket = new QLocalSocket(this);
  audio_rx_socket->connectToServer(socket_basename + "_rx_audio"); 
  while(!audio_rx_socket->waitForConnected(30000)) {
    qDebug() << QString("AudioRXListener Waited 30 seconds for connection on local socket [%1_wfall]. Is something wrong?").arg(socket_basename);
    qDebug() << audio_rx_socket->errorString();
    QThread::sleep(5); // sleep for 5 seconds...    
  }
  connect(audio_rx_socket, SIGNAL(readyRead()), 
	  this, SLOT(processRXAudio())); 
  connect(audio_rx_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	  this, SLOT(cmdErrorHandler(QLocalSocket::LocalSocketError)));

  return true; 
}

void GUISoDa::AudioRXListener::processRXAudio() {
  // we've got an incoming buffer. 
  // copy it to the circular buffer.  (Note that this copy 
  // is not the most time-efficient choice, but the simplicity
  // and reduced bug count from this approach is compelling. 
  // The total transfer load to/from this buffer is about 
  // 400KB/sec.  The CircularBuffer object has been measured
  // at way above 300MB/sec on a really old Intel desktop 
  // (a 2010 edition i7). 
  
  qint64 len = audio_rx_socket->bytesAvailable(); 
  while(len > 0) {
    // get the data from the socket
    qint64 tlen = (len > rx_in_buf_len) ? rx_in_buf_len : len; 
    qint64 rlen = audio_rx_socket->read(rx_in_buf, tlen);
    
    if(rlen > 0) {
      // now pend it to the circular buffer
      audio_cbuffer_p->put(rx_in_buf, rlen); 
      
      len = len - rlen; 
    }
    else {
      // nothing to do here.  Should we complain? 
      return; 
    }
  }
}


void GUISoDa::AudioRXListener::closeRadio()
{
  // stop the audio device. 
  audioRX->stop();
  audioRX->disconnect(this);
}


bool GUISoDa::AudioRXListener::initAudio(const QAudioDeviceInfo & dev_info)
{
  QAudioFormat format; 
  format.setSampleRate(sample_rate);
  format.setChannelCount(1);
  format.setSampleSize(32);
  format.setCodec("audio/pcm");
  format.setByteOrder(QAudioFormat::LittleEndian);
  format.setSampleType(QAudioFormat::Float);
  
  if(!dev_info.isFormatSupported(format)) {
    QMessageBox mbox(QMessageBox::Critical, 
		     "Fatal Error", 
		     "Sound system will not support 48000 floating point samples/sec", 
		     QMessageBox::Ok, NULL);
    mbox.exec();
  }

  audioRX.reset(new QAudioOutput(dev_info, format));
  this->start(); 
  // tell the audio device where to find the QIODevice.
  audioRX->start(this);

  return true; 
}	

void  GUISoDa::AudioRXListener::setAudioGain(float gain)
{
  audioRX->setVolume(qreal(gain));
}

void  GUISoDa::AudioRXListener::setAudioDevice(QAudioDeviceInfo & dev_info)
{
  audioRX->stop();
  audioRX->disconnect(this); 
  initAudio(dev_info);
}


qint64 GUISoDa::AudioRXListener::readData(char * data, qint64 maxlen) 
{
  return (qint64) audio_cbuffer_p->get(data, maxlen); 
}

qint64 GUISoDa::AudioRXListener::bytesAvailable() const {
  return audio_cbuffer_p->numElements();
}
