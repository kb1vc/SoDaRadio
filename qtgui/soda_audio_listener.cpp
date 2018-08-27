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

  // allocate a silence buffer for one second's worth of samples; 
  silence = new float[sample_rate]; 
  for(int i = 0; i < sample_rate; i++) {
    silence[i] = 0.0; 
  }

  debug_count = 0; 
}

bool GUISoDa::AudioRXListener::init()
{
  // create the circular buffer
  audio_cbuffer_p = new SoDa::CircularBuffer<char>(sample_rate * 10); 

  // create the rx input buffer
  rx_in_buf_len = 16 * 1024; // bigger than the largest anticipated packet
  rx_in_buf = new char[rx_in_buf_len]; 

  audio_rx_socket = new QLocalSocket(this);
  QString rx_socket_name = socket_basename + "_rxa"; 

  int wcount = 0; 
  while(!QFile::exists(rx_socket_name)) {
    QThread::sleep(5);
    wcount++; 
    if(wcount > 30) {
      qDebug() << QString("Waited %1 seconds for socket file [%2] to be created.  Is the radio process dead?").arg(wcount * 5).arg(rx_socket_name);
      emit(fatalError(QString("No socket file [%1] found after timeout of %2 seconds").arg(rx_socket_name).arg(wcount * 5)));
      return false;       
    }
  }
  
  audio_rx_socket->connectToServer(rx_socket_name);
  while(!audio_rx_socket->waitForConnected(1000)) {
    qDebug() << QString("AudioRXListener Waited for connection on local socket\n[%1]. Is something wrong?").arg(rx_socket_name);
    qDebug() << audio_rx_socket->errorString();
    QThread::sleep(5); // sleep for 5 seconds...    
  }
  connect(audio_rx_socket, SIGNAL(readyRead()), 
	  this, SLOT(processRXAudio())); 
  connect(audio_rx_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	  this, SLOT(audioErrorHandler(QLocalSocket::LocalSocketError)));

  // is the audio socket ok? 
  qDebug() << QString("AUDIO SOCKET STATE: [%1] error string [%2]")
    .arg(audio_rx_socket->state()).arg(audio_rx_socket->errorString());
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
  // qDebug() << QString("processRXAudio bytesAvail = [%1] cbuffer pointer [%2] rx_buf_ptr [%3]")
  //   .arg(len)
  //   .arg((quintptr) audio_cbuffer_p, QT_POINTER_SIZE * 2, 16, QChar('0'))
  //   .arg((quintptr) rx_in_buf, QT_POINTER_SIZE * 2, 16, QChar('0'));   

  // we may have run out of data -- if so, stuff it with 1/4 second of audio
  size_t num_elts = audio_cbuffer_p->numElements();
  size_t quarter_sec = (sample_rate * sizeof(float)) >> 2;
  if(num_elts < quarter_sec) {
    size_t slen = quarter_sec - num_elts; 
    slen = slen & (~0x3); // a number of whole floats..
    audio_cbuffer_p->put((char*) silence, slen); 
  }
  
  while(len > 0) {
    // get the data from the socket
    qint64 tlen = (len > rx_in_buf_len) ? rx_in_buf_len : len;
    //    qDebug() << QString("ba [%1] tlen [%2]").arg(len).arg(tlen);
    qint64 rlen = audio_rx_socket->read(rx_in_buf, tlen);

    if((debug_count & 0xff) == 0) {
      float * fp = (float*) rx_in_buf; 
      qDebug() << QString("dbg_count [%1] two samples in buffer [%2] [%3]")
	.arg(debug_count, 6, 16).arg(fp[0]).arg(fp[81]);
    }
    debug_count++; 

    if(rlen > 0) {
      // qDebug() << QString("P [%1] [%2]^").arg(rlen).arg(tlen);      
      // now pend it to the circular buffer
      audio_cbuffer_p->put(rx_in_buf, rlen); 
      //      qDebug() << QString("P!");            
      len = len - rlen; 
    }
    else {
      // nothing to do here.  Should we complain? 
      // qDebug() << QString("Leaving processRXAudio(A) queue has [%1] bytes available")
      // 	.arg(audio_cbuffer_p->numElements());;
      return; 
    }
  }
  //   qDebug() << QString("Leaving processRXAudio(B) queue has [%1] bytes available")
  //   .arg(audio_cbuffer_p->numElements());;
}


void GUISoDa::AudioRXListener::closeRadio()
{
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
    qDebug() << QString("Sound system will not support [%1] floating point samples/sec").arg(sample_rate); 
  }

  audioRX.reset(new QAudioOutput(dev_info, format));
  
  audioRX->setBufferSize((sizeof(float) * sample_rate) >> 2); // buffer up 1/4 second

  qDebug() << QString("audioRX periodSize() = [%1] bufferSize() = [%2] sample rate = [%3]").arg(audioRX->periodSize()).arg(audioRX->bufferSize()).arg(sample_rate);

  this->start(); 
  // tell the audio device where to find the QIODevice.
  audioRX->start(this);
  qDebug() << QString("period size is now [%1]").arg(audioRX->periodSize());
  return true; 
}	

void  GUISoDa::AudioRXListener::setAudioGain(float gain)
{
  audioRX->setVolume(qreal(gain));
}

void  GUISoDa::AudioRXListener::setRXDevice(const QAudioDeviceInfo & dev_info)
{
  qDebug() << QString("Setting RX Device to [%1]").arg(dev_info.deviceName());
  
  if(audioRX != NULL) {
    audioRX->stop();
    audioRX->disconnect(this); 
  }
  else {
    qDebug() << QString("Audio RX was non-null");
  }

  initAudio(dev_info);
  qDebug() << QString("Did initAudio");
}


qint64 GUISoDa::AudioRXListener::readData(char * data, qint64 maxlen) 
{
  // qDebug() << QString("in readData maxlen = [%1] cbuffer has [%2] bytes available")
  //   .arg(maxlen).arg(audio_cbuffer_p->numElements());
  int ret = (qint64) audio_cbuffer_p->get(data, maxlen); 
  // qDebug() << QString("returning [%1] from readData now have [%2] bytes available")
  //   .arg(ret).arg(audio_cbuffer_p->numElements()); 
  return ret; 
}

qint64 GUISoDa::AudioRXListener::bytesAvailable() const {
  // qDebug() << QString("in bytesAvailable");
  qint64 ret = audio_cbuffer_p->numElements();
  // qDebug() << QString("returning [%1] from bytesAvailable")
  //   .arg(ret);
  return ret; 
}
