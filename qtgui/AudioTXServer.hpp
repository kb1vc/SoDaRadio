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


#include <QObject>
#include <QIODevice>
#include <QAudioInput>
#include <QString>
#include <QtNetwork/QtNetwork>

namespace GUISoDa {
  /**
   * @class AudioTXServer
   *
   * @brief class to listen on a device providing audio samples, and
   * pass them to the radio TX audio input stream. 
   *
   * AudioTXServer is an IODevice. 
   *
   * AudioTXServer owns a socket (to send samples to the radio) and an
   * audio input device (to capture input audio).
   *
   * on creation it calls initServer
   *   - creates the audio_tx_socket by waiting for the Radio to create it.
   *     The socket name is socket_basename + "_txa"
   *   - creates an AudioInput device and connects it to the AudioTXServer
   * 
   * On IODevice::write: 
   * 
   *   - It eats the data and
   *   - puts it in an outbound message on audio_tx_socket
   *   - and then returns.
   *
   * Use the "put" object in RadioListener as a pattern. Each
   * call will send the number of samples available. It is up to the
   * radio to buffer or collect samples into the appropriate buffer
   * size for the radio's guts. 
   *
   * On changeDevice:
   *
   *   - Stop the audio device
   *   - disconnect the audio input
   *   - create a new audio input device
   *
   * Creation of an audio device (from changeDevice or from the constructor)
   * is done by initAudioDevice()
   * 
   * There's not much else to do.
   * 
   * AudioTXServer must be created by some QThread. Perhaps the main thread? Someone
   * who sleeps all the time.
   *
   * AudioTXServer provides two slots: changeDevice and shutdown
   */  
  class AudioTXServer : public QIODevice {
    Q_OBJECT

  public:
    /**
     * @brief Create the AudioTXServer IO device. See class description.
     *
     * @param parent someone who cares
     * @param socket_basename the radio will create a socket called basename + _txa
     */ 
    AudioTXServer(QObject * parent = 0, 
		  const QString & socket_basename = "soda");

    ~AudioTXServer() {
      shutdown();
      delete audio_tx_socket;
    }

    /**
     * @brief connect to radio server socket
     *
     *   - creates the audio_tx_socket by waiting for the Radio to create it.
     *     The socket name is socket_basename + "_txa"
     *   - creates an AudioInput device and connects it to the AudioTXServer
     * 
     * @return true on success, false on some fatal problem. 
     */
    bool initServer(const QString & socket_basename);   

    /**
     * @brief connect to radio server socket
     *
     *   - creates the audio_tx_socket by waiting for the Radio to create it.
     *     The socket name is socket_basename + "_txa"
     * 
     * @param tx_socket_name the full name for the socket (basename + _txa)
     * @return true on success, false on some fatal problem. 
     */
    bool createSocket(const QString & tx_socket_name);

    
    /**
     * @brief initialize audio interface
     *
     *  - Create a (new) audio device. 
     *  - tell it about us (we'll field the write calls...)
     *
     * @param dev_info a descriptor for the device
     * @return true on success, false on some fatal problem 
     */
    bool initAudioDevice(const QAudioDeviceInfo & dev_info);

    /**
     * @brief write method for a write only device
     * 
     *   - It eats the data and
     *   - puts it in an outbound message on audio_tx_socket
     *   - and then returns.
     *
     *
     * @param data a buffer that we will pass on to the radio
     * @param len the length of the buffer
     * @return the number of bytes taken from the buffer
     */
    qint64 writeData(const char * data, qint64 len) override;
    
    /**
     * @brief read method to for a write only device
     *
     * @param data buffer to be ignored
     * @param maxlen length of the buffer (in bytes). 
     *
     * @return the number of bytes written to the buffer (maxlen)
     */
    qint64 readData(char * data, qint64 maxlen) override; 


    /**
     * @brief This server only handles devices that provide 32 bit float
     * samples in the audio_pcm form.
     *
     * @param dev_info a Qt audio device descritor.
     * @return true if dev_info supports the 32/fload/pcm format, false otherwise. 
     */
    static bool audioFormatIsSupported(QAudioDeviceInfo & dev_info);
    
  protected:    
    /**
     * @brief implementation of QIODevice start method -- announce that we're open for busines
     */
    void start() {
      open(QIODevice::WriteOnly); 
    }

    /**
     * @brief shut down the QIODevice. 
     */
    void stop() {
      close();
    }

    /**
     * @brief create an audio format object that we can test against
     * a device descriptor or use to initialize and audio input device.
     *
     * @return a format descriptor specifying 32/float/pcm/le sample format. 
     */ 
    static QAudioFormat createAudioFormat();
    
  signals:
    void fatalError(const QString & error_string);

  public slots:
    /**
     * @brief On startup we connect to the default audio input device.
     * But the UI may select from any of the available IO devices.
     *
     * @param dev_info describes the audio device that will provide data
     */
    void changeDevice(QAudioDeviceInfo & dev_info);

    /**
     * @brief This closes out the audio device and the socket. 
     */
    void shutdown();
    
  private:

    QString socket_name; 
    QLocalSocket * audio_tx_socket;     

    QAudioFormat audio_format;
    QScopedPointer<QAudioInput> audio_input_p;

    bool is_active;

    // Let's make sure nothing gets ripped out from under anyone
    QMutex mutex; 
  };
}
