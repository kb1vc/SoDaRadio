#pragma once
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

#include <QObject>
#include <QIODevice>
#include <QAudioOutput>
#include <QString>
#include <QtNetwork/QtNetwork>

#include <iostream>
#include <fstream>
#include <errno.h>
#include <sndfile.h>
#include "../common/CircularBuffer.hxx"

namespace GUISoDa {

  // put the recorder in its own thread, so if something goes wrong
  // we don't zorch the rest of the radio. 
  class AudioRecorder : public QThread {
    Q_OBJECT
    
  public:
    AudioRecorder(int _sample_rate);
    ~AudioRecorder() {
      if(snd_file != NULL) {
	sf_close(snd_file); 
	delete rec_buffer; 
      }
    }

  public slots:
    void getRecDirectory(QWidget * par); 
    
    // write buffer to circular buffer or to file. 
    void saveData(float * buf, qint64 len); 

    // start/stop recording
    void record(bool on); 
    
  protected:
    void openSoundFile(const QString & fname); 
    
    QString record_directory; 
    QString current_file; 
    
    SNDFILE * snd_file; 
    int sample_rate; 
    
    SoDa::CircularBuffer<float> * rec_buffer; 
  }; 
}
