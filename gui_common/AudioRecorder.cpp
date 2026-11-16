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
#include <QDateTime>
#include <QFileDialog>
#include <algorithm>
#include <vector>

namespace GUISoDa {

  AudioRecorder::AudioRecorder(int _sample_rate)
    : sample_rate(_sample_rate), flac_encoder(nullptr),
      record_directory("./")
  {
    // Pre-record circular buffer: keep the last 5 seconds so recording
    // starts slightly before the user hits the button.
    rec_buffer = new SoDa::CircularBuffer<float>(sample_rate * 5);
  }

  AudioRecorder::~AudioRecorder()
  {
    finalizeFlacFile();
    delete rec_buffer;
  }

  void AudioRecorder::openFlacFile(const QString & fname)
  {
    flac_encoder = FLAC__stream_encoder_new();
    if (!flac_encoder) return;

    FLAC__stream_encoder_set_channels(flac_encoder, 1);
    FLAC__stream_encoder_set_bits_per_sample(flac_encoder, 16);
    FLAC__stream_encoder_set_sample_rate(flac_encoder, static_cast<uint32_t>(sample_rate));

    FLAC__StreamEncoderInitStatus status =
      FLAC__stream_encoder_init_file(flac_encoder,
                                     fname.toLocal8Bit().constData(),
                                     nullptr, nullptr);
    if (status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
      FLAC__stream_encoder_delete(flac_encoder);
      flac_encoder = nullptr;
    }
  }

  void AudioRecorder::finalizeFlacFile()
  {
    if (!flac_encoder) return;
    FLAC__stream_encoder_finish(flac_encoder);
    FLAC__stream_encoder_delete(flac_encoder);
    flac_encoder = nullptr;
  }

  // Convert float [-1,1] samples to 16-bit signed PCM and feed to FLAC encoder.
  void AudioRecorder::writeSamples(const float * buf, int len)
  {
    if (!flac_encoder) return;
    std::vector<FLAC__int32> pcm(len);
    for (int i = 0; i < len; i++) {
      float s = std::max(-1.0f, std::min(1.0f, buf[i]));
      pcm[i] = static_cast<FLAC__int32>(s * 32767.0f);
    }
    FLAC__stream_encoder_process_interleaved(flac_encoder, pcm.data(),
                                             static_cast<uint32_t>(len));
  }

  void AudioRecorder::record(bool on)
  {
    // Always close any open file first.
    finalizeFlacFile();

    if (on) {
      // Capture the start-of-recording timestamp before opening the file so
      // the filename reflects when recording actually began.
      QDateTime now = QDateTime::currentDateTime();
      // Format: SoDaAF_dd_mm_yy_HH_MM_SS.flac
      // Qt format codes: dd=day, MM=month, yy=2-digit year, HH=hour, mm=minute, ss=second
      QString fname = QString("%1/SoDaAF_%2.flac")
        .arg(record_directory)
        .arg(now.toString("dd_MM_yy_HH_mm_ss"));
      openFlacFile(fname);
    }
  }

  void AudioRecorder::saveData(float * buf, qint64 len)
  {
    if (flac_encoder) {
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
