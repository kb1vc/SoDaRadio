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
#include <QMessageBox>
#include <cstring>
#include <QDateTime>
#include <QFileDialog>
#include <QByteArray>

namespace GUISoDa {

  AudioRecorder::AudioRecorder(int _sample_rate)
  {
    sample_rate = _sample_rate; 
    snd_file = NULL; 

    record_directory = QString("./");

    // store 5 seconds worth of audio
    rec_buffer = new SoDa::CircularBuffer<float>(sample_rate * 5);   
  }

  void AudioRecorder::openSoundFile(const QString & fname)
  {
    //// NOTE!  Since this manipulates snd_file, it must interlock against
    //// saveData -- this works just fine as long as the two methods are 
    //// only ever called as or from a slot.  

    SF_INFO info; 
    info.samplerate = sample_rate; 
    info.channels = 1; 
    // FLAC is lossless and a bit more compact than ulaw/wave
    // info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16 | SF_FORMAT_ULAW;
    info.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
    QByteArray ba = fname.toLatin1(); 
    snd_file = sf_open(ba.data(), SFM_WRITE, &info); 
    // fail silently. 
  }

  void AudioRecorder::record(bool on) 
  {
    //// NOTE!  Since this manipulates snd_file, it must interlock against
    //// saveData -- this works just fine as long as the two methods are 
    //// only ever called as slots.  
    // stop recording, regardless of what just happened. 
    if(snd_file != NULL) {
      sf_close(snd_file); 
      snd_file = NULL; 
    }
  
    if(on) {
      // if we're turning the recorder on, 
      // create a new file name. 

      QString fname = QString("%1/%2.%3").arg(record_directory).arg(QDateTime::currentDateTime().toString("dd-MMM-yy_HHmmss")).arg("flac");
    
      // open the sound file.     
      openSoundFile(fname); 
    }
  }

  void AudioRecorder::saveData(float * buf, qint64 len) 
  {
    if(snd_file != NULL) {
      // we're writing a sound file -- is this the first buffer 
      // to arrive?  If so, dump the circular buffer first...
      if (rec_buffer->numElements() != 0) {
	// dump the circular buffer to the sound file. 
	// 4K elements at a time. 
	float ibuf[4096]; 
	int len; 
	while((len = rec_buffer->get(ibuf, 4096)) > 0) {
	  int rv = sf_write_float(snd_file, ibuf, len); 
	}
      }

      // now the circular buffer is empty (and it will stay that
      // way for a while. 
      // dump the incoming buffer to the sound file.
      int rv = sf_write_float(snd_file, buf, len); 
    }
    else {
      // we aren't recording, save the last samples. 
      rec_buffer->put(buf, len); 
    }
  }

  void AudioRecorder::getRecDirectory(QWidget * par)
  {
    QString rec_dir = QFileDialog::getExistingDirectory(par, tr("Select Recording Directory"), 
							record_directory,
							QFileDialog::ShowDirsOnly | 
							QFileDialog::DontResolveSymlinks);
    if(!(rec_dir.isNull() || rec_dir.isEmpty())) {
      record_directory = rec_dir; 
    }
  }
}
