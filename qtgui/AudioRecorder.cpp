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

#include "AudioRecorder.hpp"
#include <QDataStream>
#include <QDateTime>
#include <QFileDialog>
#include <QByteArray>
#include <algorithm>

namespace GUISoDa {

  AudioRecorder::AudioRecorder(int _sample_rate)
    : sample_rate(_sample_rate), data_bytes_written(0),
      record_directory("./")
  {
    // Pre-record circular buffer: keep the last 5 seconds so recording
    // starts slightly before the user hits the button.
    rec_buffer = new SoDa::CircularBuffer<float>(sample_rate * 5);
  }

  AudioRecorder::~AudioRecorder()
  {
    finalizeWavFile();
    delete rec_buffer;
  }

  // Write a 44-byte RIFF/WAV header with placeholder sizes (filled on close).
  // Format: 16-bit signed PCM, mono, little-endian.
  void AudioRecorder::openWavFile(const QString & fname)
  {
    wav_file.setFileName(fname);
    if (!wav_file.open(QIODevice::WriteOnly)) return;

    data_bytes_written = 0;

    QDataStream ds(&wav_file);
    ds.setByteOrder(QDataStream::LittleEndian);

    // RIFF chunk
    ds.writeRawData("RIFF", 4);
    ds << quint32(0);                       // placeholder: file size - 8
    ds.writeRawData("WAVE", 4);
    // fmt sub-chunk
    ds.writeRawData("fmt ", 4);
    ds << quint32(16);                      // sub-chunk size (PCM)
    ds << quint16(1);                       // audio format: PCM
    ds << quint16(1);                       // channels: mono
    ds << quint32(sample_rate);
    ds << quint32(sample_rate * 2);         // byte rate (16-bit = 2 bytes/sample)
    ds << quint16(2);                       // block align
    ds << quint16(16);                      // bits per sample
    // data sub-chunk
    ds.writeRawData("data", 4);
    ds << quint32(0);                       // placeholder: data size in bytes
  }

  // Seek back and fill in the two size fields, then close.
  void AudioRecorder::finalizeWavFile()
  {
    if (!wav_file.isOpen()) return;

    quint32 data_size = data_bytes_written;
    quint32 riff_size = 36 + data_size;    // 44-byte header - 8 = 36

    // Patch RIFF size at offset 4
    wav_file.seek(4);
    QDataStream ds(&wav_file);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds << riff_size;

    // Patch data size at offset 40
    wav_file.seek(40);
    ds << data_size;

    wav_file.close();
  }

  // Convert float [-1,1] samples to 16-bit signed PCM and append to file.
  void AudioRecorder::writeSamples(const float * buf, int len)
  {
    QByteArray pcm(len * 2, Qt::Uninitialized);
    qint16 * out = reinterpret_cast<qint16 *>(pcm.data());
    for (int i = 0; i < len; i++) {
      float s = std::max(-1.0f, std::min(1.0f, buf[i]));
      out[i] = static_cast<qint16>(s * 32767.0f);
    }
    wav_file.write(pcm);
    data_bytes_written += static_cast<quint32>(len * 2);
  }

  void AudioRecorder::record(bool on)
  {
    // Always close any open file first.
    finalizeWavFile();

    if (on) {
      QString fname = QString("%1/%2.wav")
        .arg(record_directory)
        .arg(QDateTime::currentDateTime().toString("dd-MMM-yy_HHmmss"));
      openWavFile(fname);
    }
  }

  void AudioRecorder::saveData(float * buf, qint64 len)
  {
    if (wav_file.isOpen()) {
      // First buffer after record() — flush the pre-record circular buffer.
      if (rec_buffer->numElements() != 0) {
        float ibuf[4096];
        int n;
        while ((n = rec_buffer->get(ibuf, 4096)) > 0)
          writeSamples(ibuf, n);
      }
      writeSamples(buf, static_cast<int>(len));
    } else {
      // Not recording: keep a rolling window of recent audio.
      rec_buffer->put(buf, len);
    }
  }

  void AudioRecorder::getRecDirectory(QWidget * par)
  {
    QString dir = QFileDialog::getExistingDirectory(par,
      tr("Select Recording Directory"),
      record_directory,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isNull() && !dir.isEmpty())
      record_directory = dir;
  }
}
