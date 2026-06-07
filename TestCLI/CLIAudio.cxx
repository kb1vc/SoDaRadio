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
#include <QSocketNotifier>
#include <QMediaDevices>
#include <QTimer>
#include <QIODevice>
#include <QString>
#include <QDebug>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <iostream>
#include <SoDa/Format.hxx>

namespace SoDaCLI {

static constexpr int   SAMPLE_RATE    = 48000;
static constexpr int   SAMPLE_BYTES   = static_cast<int>(sizeof(float));
// Half-second ring buffer: plenty of headroom without ballooning latency.
static constexpr int   SINK_BUF_BYTES = SAMPLE_RATE * SAMPLE_BYTES / 2;

// -------------------------------------------------------------------
// Helpers

static QAudioFormat makeFormat()
{
  QAudioFormat fmt;
  fmt.setSampleRate(SAMPLE_RATE);
  fmt.setChannelCount(1);
  fmt.setSampleFormat(QAudioFormat::Float);
  return fmt;
}

// Open a client-side AF_UNIX SOCK_STREAM connection to path.
// Returns the connected fd on success, -1 on failure (errno set).
static int connectUnixSocket(const std::string & path)
{
  int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
  if(fd < 0) return -1;

  struct sockaddr_un addr;
  ::memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  ::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  if(::connect(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
    int saved = errno;
    ::close(fd);
    errno = saved;
    return -1;
  }
  return fd;
}

// -------------------------------------------------------------------
// Device discovery

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
void listAudioDevices()
{
  auto defaultOut = QMediaDevices::defaultAudioOutput();
  auto defaultIn  = QMediaDevices::defaultAudioInput();

  std::cout << SoDa::Format("Audio output devices:\n");
  for(const auto & dev : QMediaDevices::audioOutputs()) {
    bool isDefault = (dev.id() == defaultOut.id());
    std::cout << SoDa::Format("%0%1\n")
                 .addS(isDefault ? "  * " : "    ")
                 .addS(dev.description().toStdString());
  }

  std::cout << SoDa::Format("Audio input devices:\n");
  for(const auto & dev : QMediaDevices::audioInputs()) {
    bool isDefault = (dev.id() == defaultIn.id());
    std::cout << SoDa::Format("%0%1\n")
                 .addS(isDefault ? "  * " : "    ")
                 .addS(dev.description().toStdString());
  }

  std::cout << SoDa::Format("  (* = system default)\n");
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

  // Use a native AF_UNIX socket so there is no Qt path-translation layer
  // between us and the server's UD::ServerSocket.
  int sockfd = connectUnixSocket(sock_path);
  if(sockfd < 0) {
    qCritical() << "AudioOut: connect to [" << QString::fromStdString(sock_path)
                << "] failed:" << ::strerror(errno);
    return;
  }
  qDebug() << "AudioOut: connected to" << QString::fromStdString(sock_path);

  QAudioSink sink(device, fmt);
  sink.setBufferSize(SINK_BUF_BYTES);
  sink.setVolume(static_cast<qreal>(pending_volume.load()));
  QIODevice * io = sink.start();

  if(!io) {
    qCritical() << "AudioOut: QAudioSink::start() returned null";
    ::close(sockfd);
    return;
  }

  // RMS accumulator — updated in the read callback, reported every second.
  double rms_sum   = 0.0;
  long   rms_count = 0;

  // QSocketNotifier fires in the event loop when the socket fd is readable.
  // This is the proper way to integrate a POSIX fd with a Qt event loop.
  QSocketNotifier notifier(sockfd, QSocketNotifier::Read);
  QObject::connect(&notifier, &QSocketNotifier::activated, [&](QSocketDescriptor, QSocketNotifier::Type) {
    char buf[16384];
    ssize_t n = ::read(sockfd, buf, sizeof(buf));
    if(n > 0) {
      // Accumulate RMS before writing to the sink.
      const float * fp = reinterpret_cast<const float *>(buf);
      long nf = n / SAMPLE_BYTES;
      for(long i = 0; i < nf; i++) rms_sum += fp[i] * fp[i];
      rms_count += nf;

      if(io && sink.state() != QAudio::StoppedState) {
        auto written = io->write(buf, static_cast<qint64>(n));
        if(written != static_cast<qint64>(n)) {
          std::cerr << SoDa::Format("AudioOut: short write to sink: %0 of %1 bytes\n")
                       .addI((int)written).addI((int)n);
        }
      }
    } else if(n == 0) {
      std::cerr << SoDa::Format("AudioOut: server closed the audio socket\n");
      quit();
    } else if(errno != EAGAIN && errno != EWOULDBLOCK) {
      std::cerr << SoDa::Format("AudioOut: read error: %0\n").addS(::strerror(errno));
      quit();
    }
  });

  // Every second: print RMS of samples received, then reset the accumulator.
  QTimer rms_timer;
  QObject::connect(&rms_timer, &QTimer::timeout, [&]() {
    if(rms_count > 0) {
      double rms = std::sqrt(rms_sum / static_cast<double>(rms_count));
      std::cerr << SoDa::Format("[AudioOut] RMS=%0  (%1 samples,  %2.%3 s)\n")
                   .addF(rms, 'e', 0, 6)
                   .addI((int)rms_count)
                   .addI((int)(rms_count / SAMPLE_RATE))
                   .addI((int)((rms_count % SAMPLE_RATE) * 10 / SAMPLE_RATE));
    } else {
      std::cerr << SoDa::Format("[AudioOut] no samples received from server\n");
    }
    rms_sum   = 0.0;
    rms_count = 0;
  });
  rms_timer.start(1000);

  // Periodic: apply volume changes and check stop flag.
  QTimer vol_timer;
  QObject::connect(&vol_timer, &QTimer::timeout, [&]() {
    sink.setVolume(static_cast<qreal>(pending_volume.load()));
    if(stop_flag.load()) quit();
  });
  vol_timer.start(50);

  exec();  // run the event loop

  sink.stop();
  ::close(sockfd);
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

  int sockfd = connectUnixSocket(sock_path);
  if(sockfd < 0) {
    qCritical() << "AudioIn: connect to [" << QString::fromStdString(sock_path)
                << "] failed:" << ::strerror(errno);
    return;
  }
  qDebug() << "AudioIn: connected to" << QString::fromStdString(sock_path);

  QAudioSource source(device, fmt);
  QIODevice * io = source.start();  // pull mode: we read from io

  if(!io) {
    qCritical() << "AudioIn: QAudioSource::start() returned null";
    ::close(sockfd);
    return;
  }

  // Poll for captured samples, scale, and forward to the server socket.
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

    ::write(sockfd, data.constData(), static_cast<size_t>(data.size()));
  });
  poll_timer.start(5);  // 5 ms — well under the 48 ms frame budget

  exec();  // run the event loop

  source.stop();
  ::close(sockfd);
}

// -------------------------------------------------------------------
// AudioToneThread

AudioToneThread::AudioToneThread(const QAudioDevice & dev, QObject * parent)
  : QThread(parent),
    device(dev),
    stop_flag(false)
{
}

AudioToneThread::~AudioToneThread()
{
  stopAudio();
}

void AudioToneThread::stopAudio()
{
  stop_flag.store(true);
  quit();
  wait(3000);
}

void AudioToneThread::run()
{
  QAudioFormat fmt = makeFormat();

  if(!device.isFormatSupported(fmt)) {
    qCritical() << "AudioTone: device does not support float32/48kHz/mono";
    return;
  }

  QAudioSink sink(device, fmt);
  sink.setVolume(1.0);
  QIODevice * io = sink.start();

  if(!io) {
    qCritical() << "AudioTone: QAudioSink::start() returned null";
    return;
  }

  constexpr double FREQ      = 440.0;
  constexpr float  AMP       = 0.2f;
  constexpr int    CHUNK     = SAMPLE_RATE / 100;  // 10 ms at 48 kHz

  double phase     = 0.0;
  double phase_inc = 2.0 * M_PI * FREQ / SAMPLE_RATE;

  QTimer timer;
  QObject::connect(&timer, &QTimer::timeout, [&]() {
    if(stop_flag.load()) { quit(); return; }

    QByteArray data(CHUNK * SAMPLE_BYTES, 0);
    float * fp = reinterpret_cast<float *>(data.data());
    for(int i = 0; i < CHUNK; i++) {
      fp[i] = AMP * static_cast<float>(std::sin(phase));
      phase += phase_inc;
      if(phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
    }

    if(io && sink.state() != QAudio::StoppedState) io->write(data);
  });
  timer.start(10);

  exec();

  sink.stop();
}

} // namespace SoDaCLI
