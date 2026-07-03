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

#include <QThread>
#include <QFile>
#include <QString>
#include <QWidget>
#include "../common/CircularBuffer.hxx"

namespace GUISoDa {

  // Records received audio to a WAV file.  Runs in its own thread so a
  // slow filesystem write can't stall the GUI.
  class AudioRecorder : public QThread {
    Q_OBJECT

  public:
    AudioRecorder(int _sample_rate);
    ~AudioRecorder();

  public slots:
    void getRecDirectory(QWidget * par);

    // Buffer audio continuously; flush to file when recording is active.
    void saveData(float * buf, qint64 len);

    // Start (on=true) or stop (on=false) recording.
    void record(bool on);

  protected:
    void openWavFile(const QString & fname);
    void finalizeWavFile();
    void writeSamples(const float * buf, int len);

    QString record_directory;

    QFile   wav_file;
    quint32 data_bytes_written;
    int     sample_rate;

    SoDa::CircularBuffer<float> * rec_buffer;
  };
}
