/*
  Copyright (c) 2018, 2025 Matthew H. Reilly (kb1vc)
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

#include "AudioRXListener.hpp"
#include <QMessageBox>
#include <cstring>
#include <QDateTime>
#include <QFileDialog>
#include <QByteArray>

namespace GUISoDa {
  
  AudioRXListener::AudioRXListener(QObject * parent, const QString & _socket_basename, unsigned int _sample_rate) : QIODevice(parent) {
    quit = false;
    socket_basename = _socket_basename; 
    sample_rate = _sample_rate; 

    // allocate a silence buffer for one second's worth of samples; 
    silence = new float[sample_rate]; 
    for(int i = 0; i < sample_rate; i++) {
      silence[i] = 0.0; 
    }

    status_update_count = 0;
    bytes_sent_count = 0;

    max_slack_time = 0.2; // 200ms starts to become a problem for FT8...
  }

  bool AudioRXListener::init()
  {
    // create the circular buffer -- make it small. This limits 
    // excess latency caused by mismatches between the radio and the audio
    // fast radio vs. slow audio will eventually wrap the circular buffer, 
    // but not in a way that will hurt. 
    // slow radio vs. fast audio will trigger an under-run on occasion. 
    // we will recover in the audioOutputError handler.

    audio_cbuffer_p = new SoDa::CircularBuffer<char>(sample_rate * sizeof(float) * 10); 

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
	    this, SLOT(audioSocketError(QLocalSocket::LocalSocketError)));

    return true; 
  }

  qint64 AudioRXListener::writeData(const char * data, qint64 maxlen) {
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0; 
  }


  void AudioRXListener::processRXAudio() {
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

      if((status_update_count & 0x1f) == 0) {
	float * fp = (float*) rx_in_buf;
	float delay;
	size_t num_elts = audio_cbuffer_p->numElements();
	delay = ((float) (num_elts / sizeof(float))) / ((float) sample_rate); 
	emit(bufferSlack(QString("%1").arg(delay, 4, 'F', 2)));

	if(delay > max_slack_time) {
	  // we may be way too far ahead.  
	  qInfo() << QString("Audio RX stream has fallen behind -- clearing outbound buffers of [%1] seconds").arg(delay);
	  cleanBuffer();
	}
      }

      status_update_count++;
    
      if(rlen > 0) {
	bytes_sent_count += rlen;       
	audio_cbuffer_p->put(rx_in_buf, rlen);
	// send the buffer to anyone else who is listening.
	emit(pendAudioBuffer((float*) rx_in_buf, rlen / sizeof(float)));
	len = len - rlen; 
      }
      else {
	return; 
      }
    }
  }


  void AudioRXListener::closeRadio()
  {
    audio_rx_output->stop();
    audio_rx_output->disconnect(this);
  }

  void AudioRXListener::cleanBuffer() 
  {
    audio_cbuffer_p->clear();
  }

  QAudioFormat AudioRXListener::createAudioFormat(unsigned int sample_rate) {
    QAudioFormat format;
    format.setSampleRate(sample_rate);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);
  
    return format; 
  }

  qaudiodeviceinfo is now qaudiodevice...
  bool AudioRXListener::initAudio(const QAudioDeviceInfo & dev_info)
  {
    QAudioFormat format = createAudioFormat();

    
    if(!dev_info.isFormatSupported(format)) {
      qDebug() << QString("Sound system will not support [%1] floating point samples/sec").arg(sample_rate); 
    }
    audio_rx_output.reset(new QAudioOutput(dev_info, format));
  
    audio_rx_output->setBufferSize((sizeof(float) * sample_rate) >> 2); // buffer up 1/4 second

    // react to errors when they happen. 
    connect(audio_rx_output.data(), SIGNAL(stateChanged(QAudio::State)), 
	    this, SLOT(audioOutError(QAudio::State)));

    // start this IO device -- does this need to be here? 
    this->start(); 

    // tell the audio device where to find the QIODevice.
    audio_rx_output->start(this);
    return true; 
  }	


  void  AudioRXListener::setAudioGain(float gain)
  {
    audio_rx_output->setVolume(qreal(gain));
  }

  void  AudioRXListener::setRXDevice(const QAudioDeviceInfo & dev_info)
  {
    if(audio_rx_output != NULL) {
      audio_rx_output->stop();
      audio_rx_output->disconnect(this); 
    }

    QList<QAudioDeviceInfo> devs = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    initAudio(dev_info);
  }


  qint64 AudioRXListener::readData(char * data, qint64 max_len) 
  {
    // we may have run out of data.  If so, return silence. 
    // and stuff silence into the output stream until we get ahead of the game
    // a little bit. 
    size_t avail = audio_cbuffer_p->numElements();

    // Qt Audio under Mac doesn't go through ALSA, so is much better
    // behaved. It won't call readData if we have nothing to offer.
    // and will buffer what it gets. 
    if((MACOSX == 0) && (avail < max_len)) {
      // we're below the acceptable reserver... stuff some silence
      // into the output buffers until we're 
      //    qInfo() << QString("[%3] Audio device attempts to read [%1] bytes, only [%2] available.")
      //      .arg(max_len).arg(avail).arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz t"));
      // stuff some silence in here.. 
      qint64 fill_len = max_len >> 2; 
      memset(data, 0, fill_len); 
      return fill_len;
    }
    else {
      int ret = (qint64) audio_cbuffer_p->get(data, max_len);
      float * fdat = (float*) data;
      float sum = 0.0; 
      for(int i = 0; i < ret / 4; i++) {
	sum += fdat[i]; 
      }
      return ret; 
    }
  }

  qint64 AudioRXListener::bytesAvailable() const {
    qint64 ret = audio_cbuffer_p->numElements();
    return ret; 
  }

  void AudioRXListener::audioOutError(QAudio::State new_state) {
    if(new_state == QAudio::StoppedState) {
      switch (audio_rx_output->error()) {
      case QAudio::UnderrunError:
	qDebug() << QString("AudioRXListener under-run. Attempting reset.");
	audio_rx_output->reset();
	break; 
      case QAudio::IOError:
	qDebug() << QString("AudioRXListener IO error. Attempting reset.");
	audio_rx_output->reset();
	break; 
      case QAudio::OpenError:
	qFatal("AudioRXListener got a OpenError of some sort on the audio output device.");      
	break; 
      default:
	// all other errors are fatal, except the crap from audio alsa.. 
	qInfo("AudioRXListener got a bothersome error (not fatal, not io, not under-run) of some sort on the audio output device.");
	break; 
      }
    }
  }

}
