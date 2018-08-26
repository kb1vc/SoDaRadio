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

#ifndef SODA_AUDIO_LISTENER_HEADER
#define SODA_AUDIO_LISTENER_HEADER
#include <QObject>
#include <QIODevice>
#include <QAudioOutput>
#include <QString>
#include <QtNetwork/QtNetwork>

#include <iostream>
#include <errno.h>
#include "../src/Command.hxx"
#include "../common/CircularBuffer.hxx"

namespace GUISoDa {
  /**
   * @brief class to listen on a socket carrying audio samples, and
   * pass them to a Qt audio device. 
   */
  class AudioRXListener : public QIODevice {
    Q_OBJECT

  public:
    AudioRXListener(QObject * parent = 0, const QString & socket_basename = "tmp", 
		    unsigned int _sample_rate = 48000);
    ~AudioRXListener() {
    }

    /**
     * @brief connect to radio server sockets and initialize listener state
     * 
     * @return true on success, false on some fatal problem. 
     */
    bool init();   

    /**
     * @brief initialize audio interface
     * @return true on success, false on some fatal problem 
     */
    bool initAudio(const QAudioDeviceInfo & dev_info);

    /*
     * This is a QIODevice, so it must implement some means of providing data 
     * to a reader and sinking data from a writer.  This is a read-only device, 
     * so the writer part is easy
     */

    /**
     * @brief write method for a write only device
     *
     * @param data a buffer that we will ignore
     * @param len the length of the buffer that we just ignored
     * @return 0 all the time. 
     */
    qint64 writeData(const char * data, qint64 len) {
      Q_UNUSED(data);
      Q_UNUSED(len);
      return 0; 
    }
    
    /**
     * @brief read method to supply a sequence of samples to the audio 
     * device. 
     *
     * @param data buffer to be filled from enqueued samples
     * @param maxlen length of the buffer (in bytes). 
     *
     * @return the number of bytes written to the buffer
     */
    qint64 readData(char * data, qint64 maxlen); 

    /**
     * @brief how many bytes are ready for reading
     */
    qint64 bytesAvailable() const; 

    /**
     * @brief implementation of QIODevice start method -- announce that we're open for business
     */
    void start() {
      open(QIODevice::ReadOnly); 
    }

    /**
     * @brief shut down the QIODevice. 
     */
    void stop() {
      close();
    }


    
  signals:
    void fatalError(const QString & error_string);
					      
  public slots:
    void setAudioGain(float gain); 
    void setAudioDevice(QAudioDeviceInfo & dev_info);
    void closeRadio();


  protected slots:  
    void processRXAudio();
  
  private:
    QString socket_basename; 
    QLocalSocket * audio_rx_socket;     
    bool quit; 

    QScopedPointer<QAudioOutput> audioRX; 
    
    // the audio samples arrive as floats, but
    // the packets move back and forth as bytes
    SoDa::CircularBuffer<char> * audio_cbuffer_p;

    // we need a reasonably large input buffer for the 
    // rx socket interface. 
    char * rx_in_buf;
    qint64 rx_in_buf_len; 
    qint64 sample_rate; ///< Audio output samples per second.
  };
}
#endif
