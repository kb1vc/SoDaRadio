/*
Copyright (c) 2018,2023 Matthew H. Reilly (kb1vc)
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

#include "soda_audio_listener.hpp"
#include <QMessageBox>
#include <cstring>
#include <QDateTime>
#include <QFileDialog>
#include <QByteArray>

GUISoDa::AudioListener::AudioListener(QObject * parent,
				      const QString & socket_basename,
				      unsigned int _sample_rate) {
  rx_listener = new AudioRXListener(parent, socket_basename, _sample_rate); 
  rx_recorder = new AudioRecorder(_sample_rate); 
  
  connect(rx_listener, SIGNAL(pendAudioBuffer(float*, qint64)), 
	  rx_recorder, SLOT(saveData(float*, qint64)));
  this->setObjectName(QString("GUISoDa::AudioListener"));
}

GUISoDa::AudioRXListener::AudioRXListener(QObject * parent, const QString & _socket_basename, unsigned int _sample_rate) : QIODevice(parent) {
  quit = false;
  socket_basename = _socket_basename; 
  sample_rate = _sample_rate; 

  // allocate a silence buffer for one second's worth of samples; 
  silence = new float[sample_rate]; 
  for(int i = 0; i < sample_rate; i++) {
    silence[i] = 0.0; 
  }

  status_update_count = 0; 

  max_slack_time = 0.2; // 200ms starts to become a problem for FT8... 
}

bool GUISoDa::AudioRXListener::init()
{
  // create the circular buffer -- make it small. This limits 
  // excess latency caused by mismatches between the radio and the audio
  // fast radio vs. slow audio will eventually wrap the circular buffer, 
  // but not in a way that will hurt. 
  // slow radio vs. fast audio will trigger an under-run on occasion. 
  // we will recover in the audioOutputError handler. 
  audio_cbuffer_p = new SoDa::CircularBuffer<char>(sample_rate * sizeof(float) * 10); 

  // create the rx input buffer
  rx_in_buf_len = 16 * 1024; // bigger than the largest anticipated packet
  rx_in_buf = new char[rx_in_buf_len]; 

  audio_rx_socket = new QLocalSocket(this);
  QString rx_socket_name = socket_basename + "_rxa"; 

  int wcount = 0; 
  while(!QFile::exists(rx_socket_name)) {
    QThread::sleep(5);
    wcount++; 
    if(wcount > 30) {
      qDebug() << QString("Waited %1 seconds for socket file [%2] to be created.  Is the radio process dead?").arg(wcount * 5).arg(rx_socket_name);
      emit(fatalError(QString("No socket file [%1] found after timeout of %2 seconds").arg(rx_socket_name).arg(wcount * 5)));
      return false;       
    }
  }
  
  audio_rx_socket->connectToServer(rx_socket_name);
  while(!audio_rx_socket->waitForConnected(1000)) {
    qDebug() << QString("AudioRXListener Waited for connection on local socket\n[%1]. Is something wrong?").arg(rx_socket_name);
    qDebug() << audio_rx_socket->errorString();
    QThread::sleep(5); // sleep for 5 seconds...    
  }
  connect(audio_rx_socket, SIGNAL(readyRead()), 
	  this, SLOT(processRXAudio())); 
  connect(audio_rx_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), 
	  this, SLOT(audioSocketError(QLocalSocket::LocalSocketError)));


  return true; 
}

void GUISoDa::AudioRXListener::processRXAudio() {
  // we've got an incoming buffer. 
  // copy it to the circular buffer.  (Note that this copy 
  // is not the most time-efficient choice, but the simplicity
  // and reduced bug count from this approach is compelling. 
  // The total transfer load to/from this buffer is about 
  // 400KB/sec.  The CircularBuffer object has been measured
  // at way above 300MB/sec on a really old Intel desktop 
  // (a 2010 edition i7). 

  qint64 len = audio_rx_socket->bytesAvailable();

  
  while(len > 0) {
    // get the data from the socket
    qint64 tlen = (len > rx_in_buf_len) ? rx_in_buf_len : len;
    qint64 rlen = audio_rx_socket->read(rx_in_buf, tlen);

    if((status_update_count & 0x1f) == 0) {
      float * fp = (float*) rx_in_buf;
      float delay;
      size_t num_elts = audio_cbuffer_p->numElements();
      delay = ((float) (num_elts / sizeof(float))) / ((float) sample_rate); 
      emit(bufferSlack(QString("%1").arg(delay, 4, 'F', 2)));

      if(delay > max_slack_time) {
	// we may be way too far ahead.  
	qInfo() << QString("Audio RX stream has fallen behind -- clearing outbound buffers of [%1] seconds").arg(delay);
	cleanBuffer();
      }
    }
    status_update_count++; 

    if(rlen > 0) {
      audio_cbuffer_p->put(rx_in_buf, rlen);
      // send the buffer to anyone else who is listening. 
      emit(pendAudioBuffer((float*) rx_in_buf, rlen / sizeof(float)));
      len = len - rlen; 
    }
    else {
      return; 
    }
  }
}


void GUISoDa::AudioRXListener::closeRadio()
{
  audioRX->stop();
  audioRX->disconnect(this);
}

void GUISoDa::AudioRXListener::cleanBuffer() 
{
  audio_cbuffer_p->clear();
}

QAudioFormat GUISoDa::AudioRXListener::createAudioFormat(unsigned int sample_rate) {
  QAudioFormat format;
  format.setSampleRate(sample_rate);
  format.setChannelCount(1);
  format.setSampleSize(32);
  format.setCodec("audio/pcm");
  format.setByteOrder(QAudioFormat::LittleEndian);
  format.setSampleType(QAudioFormat::Float);
  
  return format; 
}

bool GUISoDa::AudioRXListener::initAudio(const QAudioDeviceInfo & dev_info)
{
  QAudioFormat format = createAudioFormat();
  
  if(!dev_info.isFormatSupported(format)) {
    qDebug() << QString("Sound system will not support [%1] floating point samples/sec").arg(sample_rate); 
  }
  audioRX.reset(new QAudioOutput(dev_info, format));
  
  audioRX->setBufferSize((sizeof(float) * sample_rate) >> 2); // buffer up 1/4 second


  
  // react to errors when they happen. 
  connect(audioRX.data(), SIGNAL(stateChanged(QAudio::State)), 
	  this, SLOT(audioOutError(QAudio::State)));

  // start this IO device -- does this need to be here? 
  this->start(); 

  // tell the audio device where to find the QIODevice.
  audioRX->start(this);
  return true; 
}	

void  GUISoDa::AudioRXListener::setAudioGain(float gain)
{
  audioRX->setVolume(qreal(gain));
}

void  GUISoDa::AudioRXListener::setRXDevice(const QAudioDeviceInfo & dev_info)
{
  if(audioRX != NULL) {
    audioRX->stop();
    audioRX->disconnect(this); 
  }

  QList<QAudioDeviceInfo> devs = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
  initAudio(dev_info);
}


qint64 GUISoDa::AudioRXListener::readData(char * data, qint64 max_len) 
{
  // we may have run out of data.  If so, return silence. 
  // and stuff silence into the output stream until we get ahead of the game
  // a little bit. 
  size_t avail = audio_cbuffer_p->numElements();

  // Qt Audio under Mac doesn't go through ALSA, so is much better
  // behaved. It won't call readData if we have nothing to offer.
  // and will buffer what it gets. 
  if((MACOSX == 0) && (avail < max_len)) {
    // we're below the acceptable reserve... stuff some silence
    // into the output buffers until we're 
    qInfo() << QString("[%3] Audio device attempts to read [%1] bytes, only [%2] available.")
      .arg(max_len).arg(avail).arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz t"));
    // stuff some silence in here.. 
    qint64 fill_len = max_len >> 2; 
    memset(data, 0, fill_len); 
    return fill_len;
  }
  else {
    int ret = (qint64) audio_cbuffer_p->get(data, max_len);
    
    return ret; 
  }
}

qint64 GUISoDa::AudioRXListener::bytesAvailable() const {
  qint64 ret = audio_cbuffer_p->numElements();
  return ret; 
}

void GUISoDa::AudioRXListener::audioOutError(QAudio::State new_state) {
  if(new_state == QAudio::StoppedState) {
    switch (audioRX->error()) {
    case QAudio::UnderrunError:
      qDebug() << QString("AudioRXListener under-run. Attempting reset.");
      audioRX->reset();
      break; 
    case QAudio::IOError:
      qDebug() << QString("AudioRXListener IO error. Attempting reset.");
      audioRX->reset();
      break; 
    case QAudio::OpenError:
      qFatal("AudioRXListener got a OpenError of some sort on the audio output device.");      
      break; 
    default:
      // all other errors are fatal, except the crap from audio alsa.. 
      qInfo("AudioRXListener got a bothersome error (not fatal, not io, not under-run) of some sort on the audio output device.");
      break; 
    }
  }
}

GUISoDa::AudioRecorder::AudioRecorder(int _sample_rate)
{
  sample_rate = _sample_rate; 
  snd_file = NULL; 

  record_directory = QString("./");

  // store 5 seconds worth of audio
  rec_buffer = new SoDa::CircularBuffer<float>(sample_rate * 5);   
}

void GUISoDa::AudioRecorder::openSoundFile(const QString & fname)
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

void GUISoDa::AudioRecorder::record(bool on) 
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

void GUISoDa::AudioRecorder::saveData(float * buf, qint64 len) 
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

void GUISoDa::AudioRecorder::getRecDirectory(QWidget * par)
{
  QString rec_dir = QFileDialog::getExistingDirectory(par, tr("Select Recording Directory"), 
						      record_directory,
						      QFileDialog::ShowDirsOnly | 
						      QFileDialog::DontResolveSymlinks);
  if(!(rec_dir.isNull() || rec_dir.isEmpty())) {
    record_directory = rec_dir; 
  }
}
