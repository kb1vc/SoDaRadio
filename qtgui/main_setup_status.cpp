/*
Copyright (c) 2018, 2024 Matthew H. Reilly (kb1vc)
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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <iostream>
#include <QAudioDeviceInfo>

QString MainWindow::secondsToElapsed(int seconds)
{
  int sec, min, hr; 
  int t = seconds;
  hr = t / 3600; 
  t = t % 3600; 
  min = t / 60; 
  sec = t % 60;
  return QString("%1:%2:%3").arg(hr, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));
}

void MainWindow::setupStatus()
{
  elapsed_seconds = 0; 
  transmit_seconds = 0; 
  transmit_intervals = 0; 
  
  ui->StartTime_lbl->setText(QDateTime::currentDateTime().toString("dd-MMM-yy HH:mm:ss t"));
  
  connect(listener, &GUISoDa::Listener::repPTT,
	  [=](bool fl) {
	    if(fl) {
	      transmit_intervals++; 
	      transmit_time.start();
	    }
	    else {
	      transmit_seconds += ((transmit_time.elapsed() + 500) / 1000);
	      ui->TransTime_lbl->setText(secondsToElapsed(transmit_seconds)); 
	      ui->TransInt_lbl->setText(QString("%1").arg(transmit_intervals));
	    }
	  }); 

  connect(&one_second_timer, &QTimer::timeout, 
	  [=]() {
	    elapsed_seconds++; 
	    ui->ElapsedTime_lbl->setText(secondsToElapsed(elapsed_seconds));
	  }); 
  
  one_second_timer.start(1000); 
  
}

