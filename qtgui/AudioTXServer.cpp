/*
  Copyright (c) 2026 Matthew H. Reilly (kb1vc)
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

#include "AudioTXServer.hpp"
#include <QMessageBox>
#include <cstring>
#include <QDateTime>
#include <QFileDialog>
#include <QByteArray>

namespace GUISoDa {
  AudioTXServer::AudioTXServer(QObject * parent, 
			       const QString & socket_basename) 
    : QIODevice(parent) {

    initServer(socket_basename);
    
    is_active = true; 
  }

  bool AudioTXServer::initServer(const QString & socket_basename)   {
    // create the outbound tx audio socket (to the radio)
    audio_tx_socket = new QLocalSocket(this);
    QString tx_socket_name = socket_basename + "_txa"; 

    createSocket(tx_socket_name);

  
    // create the default device
    // find it:
    auto default_device = QMediaDevices::defaultAudioOutput();
    
    initAudioDevice(default_device); 
    
    return true; 
  }

  bool AudioTXServer::createSocket(const QString & tx_socket_name) {
    // all sockets are "served" by the radio
    // so we need to wait until the radio creates
    // the socket. 
    int wcount = 0; 
    while(!QFile::exists(tx_socket_name)) {
      QThread::sleep(5);
      wcount++; 
      if(wcount > 30) {
	qDebug() << QString("Waited %1 seconds for socket file [%2] to be created.  Is the radio process dead?").arg(wcount * 5).arg(tx_socket_name);
	emit(fatalError(QString("No socket file [%1] found after timeout of %2 seconds").arg(tx_socket_name).arg(wcount * 5)));
	return false;       
      }
    }

    audio_tx_socket->connectToServer(tx_socket_name);
    while(!audio_tx_socket->waitForConnected(1000)) {
      qDebug() << QString("AudioTXServer Waited for connection on local socket\n[%1]. Is something wrong?").arg(tx_socket_name);
      qDebug() << audio_tx_socket->errorString();
      QThread::sleep(5); // sleep for 5 seconds...    
    }

    connect(audio_tx_socket, SIGNAL(errorOccured(QLocalSocket::LocalSocketError)), 
	    this, SLOT(audioSocketError(QLocalSocket::LocalSocketError)));

    return true;
  }

  QAudioFormat AudioTXServer::createAudioFormat() {
  
    // we're going to use a 32 bit float little endian pcm 48000 s/S
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Float);
    
    return format;
  }

  bool AudioTXServer::audioFormatIsSupported(QAudioDevice & dev_info) {
    auto format = createAudioFormat();
    
    // setup the format -- is it supported.  If not, we die
    return dev_info.isFormatSupported(format);
  }

  
  bool AudioTXServer::initAudioDevice(const QAudioDevice & dev_info) {
    // we don't want to do this while a transfer is in progress.
    mutex.lock();
    
    // create the audio device.
    auto format = createAudioFormat();

    // setup the format -- is it supported.  If not, we die
    if (!dev_info.isFormatSupported(format)) {
      qCritical() << "Default format (float 32, 48000 Hz, pcm) not supported - Don't know how this happened.";
      return false; 
    }
    
    // set the format
    audio_input_p.reset(new QAudioSource(dev_info, format)); 

    // now we've got a new audio input device.  Connect it to
    // us (we're a QIODevice and implement the WRITE operation.)
    audio_input_p->start(this);
    
    return true; 
  }

  qint64 AudioTXServer::readData(char * data, qint64 maxlen) {
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
  }

  qint64 AudioTXServer::writeData(const char * data, qint64 maxlen) {
    // we don't want anyone fooling around with devices and sockets
    // while we're doing this transfer
    mutex.lock();
      
    // just move the bits as they are.  Don't even look at them.
    auto to_go = maxlen;
    const char * cp = data; 
    while(to_go > 0) {
      int wl = audio_tx_socket->write(cp, to_go);
      if(wl < 0) {
	return maxlen - to_go;
      }
      else {
	to_go -= wl;
	cp += wl;
      }
    }

    audio_tx_socket->flush();
    
    return maxlen; 
  }
  
  void AudioTXServer::shutdown() {
    is_active = false;
    
    audio_input_p.reset(nullptr);
    audio_tx_socket->close();
  }
}
