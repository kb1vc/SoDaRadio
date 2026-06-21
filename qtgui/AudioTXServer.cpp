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

  
    auto default_device = QMediaDevices::defaultAudioInput();

    initAudioDevice(default_device);
    
    return true; 
  }

  bool AudioTXServer::createSocket(const QString & tx_socket_name) {
    socket_name = tx_socket_name;
    socket_retry_count = 0;

    connect(audio_tx_socket, SIGNAL(errorOccurred(QLocalSocket::LocalSocketError)),
            this, SLOT(audioSocketError(QLocalSocket::LocalSocketError)));

    socket_poll_timer = new QTimer(this);
    connect(socket_poll_timer, &QTimer::timeout, this, &AudioTXServer::tryConnectSocket);
    socket_poll_timer->start(5000);
    tryConnectSocket();

    return true;
  }

  void AudioTXServer::tryConnectSocket() {
    if (audio_tx_socket->state() == QLocalSocket::ConnectedState) {
      socket_poll_timer->stop();
      return;
    }

    if (QFile::exists(socket_name)) {
      socket_poll_timer->stop();
      audio_tx_socket->connectToServer(socket_name);
    } else {
      socket_retry_count++;
      if (socket_retry_count > 30) {
        socket_poll_timer->stop();
        emit fatalError(QString("No socket file [%1] found after timeout of %2 seconds")
                        .arg(socket_name).arg(socket_retry_count * 5));
      }
    }
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
    QMutexLocker locker(&mutex);

    auto format = createAudioFormat();

    if (!dev_info.isFormatSupported(format)) {
      qCritical() << "Default format (float 32, 48000 Hz, pcm) not supported - Don't know how this happened.";
      return false;
    }

    // QIODevice must be open before QAudioSource::start() will call writeData()
    if (!isOpen()) {
      open(QIODevice::WriteOnly);
    }
    audio_input_p.reset(new QAudioSource(dev_info, format));
    audio_input_p->start(this);

    return true;
  }

  qint64 AudioTXServer::readData(char * data, qint64 maxlen) {
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
  }

  quint64 debug_count = 0;
  qint64 samp_count = 0;
  qint64 AudioTXServer::writeData(const char * data, qint64 maxlen) {
    QMutexLocker locker(&mutex);

    if (audio_tx_socket->state() != QLocalSocket::ConnectedState) {
      return maxlen;
    }

    samp_count += maxlen;
    if(debug_count % 512 == 0) {
      qInfo() << QString("sample count %1").arg(samp_count);
    }
    debug_count++;

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
