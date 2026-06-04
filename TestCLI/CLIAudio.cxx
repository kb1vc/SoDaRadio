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

#include "CLIAudio.hxx"

#include <QAudioSink>
#include <QAudioSource>
#include <QAudioFormat>
#include <QLocalSocket>
#include <QMediaDevices>
#include <QTimer>
#include <QIODevice>
#include <QString>
#include <QDebug>

#include <iostream>

namespace SoDaCLI {

static constexpr int   SAMPLE_RATE   = 48000;
static constexpr int   SAMPLE_BYTES  = static_cast<int>(sizeof(float));
// half-second ring buffer gives headroom without ballooning latency
static constexpr int   SINK_BUF_BYTES = SAMPLE_RATE * SAMPLE_BYTES / 2;

// -------------------------------------------------------------------
// Device discovery

static QAudioFormat makeFormat()
{
  QAudioFormat fmt;
  fmt.setSampleRate(SAMPLE_RATE);
  fmt.setChannelCount(1);
  fmt.setSampleFormat(QAudioFormat::Float);
  return fmt;
}

QAudioDevice findOutputDevice(const std::string & name)
{
  if(name.empty() || name == "default") {
    return QMediaDevices::defaultAudioOutput();
  }
  QString target = QString::fromStdString(name).toLower();
  for(const auto & dev : QMediaDevices::audioOutputs()) {
    if(dev.description().toLower().contains(target)) return dev;
  }
  qWarning() << "Audio output device [" << QString::fromStdString(name)
             << "] not found -- using default";
  return QMediaDevices::defaultAudioOutput();
}

QAudioDevice findInputDevice(const std::string & name)
{
  if(name.empty() || name == "default") {
    return QMediaDevices::defaultAudioInput();
  }
  QString target = QString::fromStdString(name).toLower();
  for(const auto & dev : QMediaDevices::audioInputs()) {
    if(dev.description().toLower().contains(target)) return dev;
  }
  qWarning() << "Audio input device [" << QString::fromStdString(name)
             << "] not found -- using default";
  return QMediaDevices::defaultAudioInput();
}

// -------------------------------------------------------------------
// AudioOutThread

AudioOutThread::AudioOutThread(const std::string & sockPath,
                               const QAudioDevice & dev,
                               float initVolume,
                               QObject * parent)
  : QThread(parent),
    sock_path(sockPath),
    device(dev),
    pending_volume(initVolume),
    stop_flag(false)
{
}

AudioOutThread::~AudioOutThread()
{
  stopAudio();
}

void AudioOutThread::stopAudio()
{
  stop_flag.store(true);
  quit();
  wait(3000);
}

void AudioOutThread::run()
{
  QAudioFormat fmt = makeFormat();

  if(!device.isFormatSupported(fmt)) {
    qCritical() << "AudioOut: device does not support float32/48kHz/mono";
    return;
  }

  QLocalSocket socket;
  socket.connectToServer(QString::fromStdString(sock_path));
  if(!socket.waitForConnected(10000)) {
    qCritical() << "AudioOut: cannot connect to socket ["
                << QString::fromStdString(sock_path) << "]:"
                << socket.errorString();
    return;
  }

  QAudioSink sink(device, fmt);
  sink.setBufferSize(SINK_BUF_BYTES);
  sink.setVolume(static_cast<qreal>(pending_volume.load()));
  QIODevice * io = sink.start();

  if(!io) {
    qCritical() << "AudioOut: QAudioSink::start() returned null";
    return;
  }

  // Forward any audio arriving from the server straight to the sink.
  QObject::connect(&socket, &QLocalSocket::readyRead, [&]() {
    QByteArray data = socket.readAll();
    if(io && sink.state() != QAudio::StoppedState) {
      io->write(data);
    }
  });

  // Periodic: apply volume changes and check stop flag.
  QTimer vol_timer;
  QObject::connect(&vol_timer, &QTimer::timeout, [&]() {
    sink.setVolume(static_cast<qreal>(pending_volume.load()));
    if(stop_flag.load()) quit();
  });
  vol_timer.start(50);

  exec();  // run the event loop

  sink.stop();
  socket.disconnectFromServer();
}

// -------------------------------------------------------------------
// AudioInThread

AudioInThread::AudioInThread(const std::string & sockPath,
                             const QAudioDevice & dev,
                             float initGain,
                             QObject * parent)
  : QThread(parent),
    sock_path(sockPath),
    device(dev),
    pending_gain(initGain),
    stop_flag(false)
{
}

AudioInThread::~AudioInThread()
{
  stopAudio();
}

void AudioInThread::stopAudio()
{
  stop_flag.store(true);
  quit();
  wait(3000);
}

void AudioInThread::run()
{
  QAudioFormat fmt = makeFormat();

  if(!device.isFormatSupported(fmt)) {
    qCritical() << "AudioIn: device does not support float32/48kHz/mono";
    return;
  }

  QLocalSocket socket;
  socket.connectToServer(QString::fromStdString(sock_path));
  if(!socket.waitForConnected(10000)) {
    qCritical() << "AudioIn: cannot connect to socket ["
                << QString::fromStdString(sock_path) << "]:"
                << socket.errorString();
    return;
  }

  QAudioSource source(device, fmt);
  QIODevice * io = source.start();  // pull mode: we read from io

  if(!io) {
    qCritical() << "AudioIn: QAudioSource::start() returned null";
    return;
  }

  // Poll for captured samples, scale, and forward to server.
  QTimer poll_timer;
  QObject::connect(&poll_timer, &QTimer::timeout, [&]() {
    if(stop_flag.load()) { quit(); return; }

    float g = pending_gain.load();
    qint64 avail = io->bytesAvailable();
    if(avail <= 0) return;

    QByteArray data = io->read(avail);
    if(data.isEmpty() || g == 0.0f) return;

    if(g != 1.0f) {
      float * fp = reinterpret_cast<float *>(data.data());
      int n = data.size() / SAMPLE_BYTES;
      for(int i = 0; i < n; i++) fp[i] *= g;
    }

    socket.write(data);
    socket.flush();
  });
  poll_timer.start(5);  // 5 ms — well under the 48 ms frame budget

  exec();  // run the event loop

  source.stop();
  socket.disconnectFromServer();
}

} // namespace SoDaCLI
