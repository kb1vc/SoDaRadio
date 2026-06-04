#pragma once
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

/**
 * @file CLIAudio.hxx
 * @brief Qt-based audio I/O threads for SoDaCLI.
 *
 * Each direction (RX audio out, TX audio in) runs in its own QThread with a
 * full Qt event loop so that QLocalSocket signals and QTimer callbacks fire
 * correctly.  The main REPL thread controls them through thread-safe setters.
 *
 * Audio format: 32-bit float PCM, 48 000 Hz, mono.  Samples are exchanged
 * with SoDaServer over Unix-domain sockets without a length prefix:
 *   {socket_basename}_rxa — server sends demodulated audio to CLI (play)
 *   {socket_basename}_txa — CLI sends microphone audio to server (transmit)
 */

#include <QThread>
#include <QAudioDevice>
#include <atomic>
#include <string>

namespace SoDaCLI {

  /**
   * @brief Find an audio output device by name.
   *
   * Matches case-insensitively against the device description.
   * Returns the system default if name is "default", empty, or not found.
   *
   * @param name  Partial or full device description, or "default".
   * @return Matching QAudioDevice.
   */
  QAudioDevice findOutputDevice(const std::string & name);

  /**
   * @brief Find an audio input device by name.
   *
   * Same matching rules as findOutputDevice.
   *
   * @param name  Partial or full device description, or "default".
   * @return Matching QAudioDevice.
   */
  QAudioDevice findInputDevice(const std::string & name);

  // -------------------------------------------------------------------

  /**
   * @class AudioOutThread
   * @brief Receives demodulated audio from SoDaServer and plays it.
   *
   * Connects as a QLocalSocket client to @c sockPath (the server's
   * @c _rxa socket).  Incoming float32 samples are forwarded directly
   * to a QAudioSink in push mode.  Volume is adjustable at any time.
   *
   * Create, call start(), then use setVolume() to unmute.
   */
  class AudioOutThread : public QThread {
    Q_OBJECT
  public:
    /**
     * @param sockPath     Full path of the _rxa socket.
     * @param dev          Audio output device.
     * @param initVolume   Initial volume [0.0 = muted, 1.0 = full].
     * @param parent       Optional Qt parent.
     */
    explicit AudioOutThread(const std::string & sockPath,
                            const QAudioDevice & dev,
                            float initVolume = 0.0f,
                            QObject * parent = nullptr);
    ~AudioOutThread();

    /**
     * @brief Set playback volume (thread-safe).
     * @param v  Linear volume in [0.0, 1.0].
     */
    void setVolume(float v) { pending_volume.store(v); }

    /**
     * @brief Stop the thread cleanly.
     */
    void stopAudio();

  protected:
    void run() override;

  private:
    std::string sock_path;
    QAudioDevice device;
    std::atomic<float> pending_volume;
    std::atomic<bool>  stop_flag;
  };

  // -------------------------------------------------------------------

  /**
   * @class AudioInThread
   * @brief Captures microphone audio and sends it to SoDaServer.
   *
   * Connects as a QLocalSocket client to @c sockPath (the server's
   * @c _txa socket).  Samples captured from a QAudioSource are scaled
   * by the current gain and forwarded to the socket.
   *
   * Create, call start(), then use setGain() to enable audio.
   */
  class AudioInThread : public QThread {
    Q_OBJECT
  public:
    /**
     * @param sockPath   Full path of the _txa socket.
     * @param dev        Audio input device.
     * @param initGain   Initial gain [0.0 = muted, 1.0 = unity].
     * @param parent     Optional Qt parent.
     */
    explicit AudioInThread(const std::string & sockPath,
                           const QAudioDevice & dev,
                           float initGain = 0.0f,
                           QObject * parent = nullptr);
    ~AudioInThread();

    /**
     * @brief Set capture gain (thread-safe).
     * @param g  Linear gain factor applied to each sample.
     */
    void setGain(float g) { pending_gain.store(g); }

    /**
     * @brief Stop the thread cleanly.
     */
    void stopAudio();

  protected:
    void run() override;

  private:
    std::string sock_path;
    QAudioDevice device;
    std::atomic<float> pending_gain;
    std::atomic<bool>  stop_flag;
  };

} // namespace SoDaCLI
