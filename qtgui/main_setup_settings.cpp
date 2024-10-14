/*
Copyright (c) 2017 Matthew H. Reilly (kb1vc)
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
#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"
#include <QAudioDeviceInfo>

void MainWindow::setupSettings()
{
  connect(ui->CWSpeed_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setCWSpeed(int)));
  connect(ui->CWSpeed_sli, &QSlider::valueChanged,
	  [=](int s) {
	    ui->CWSpeed_lbl->setText(QString("%1").arg(s, 2));
	  }); 
  connect(ui->Sidetone_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setSidetoneVolume(int)));
  connect(ui->Sidetone_sli, &QSlider::valueChanged,
	  [=](int s) {
	    ui->Sidetone_lbl->setText(QString("%1").arg(s, 2));
	  }); 

  connect(ui->TXPower_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setTXGain(int)));
  connect(ui->TXPower_sli, &QSlider::valueChanged,
	  [=](int s) {
	    ui->TXPower_lbl->setText(QString("%1").arg(s, 3));
	  }); 
  
  connect(ui->FromGrid_le, SIGNAL(textChanged(const QString &)),
	  ui->FromGrid_lab, SLOT(setText(const QString &)));

  connect(ui->externalRefClock_ck, SIGNAL(stateChanged(int)), 
	  listener, SLOT(setClockRef(int)));

  connect(ui->openLog_btn, SIGNAL(clicked()),
	  ui->LogView, SLOT(readLogReportDlg()));
  connect(ui->writeLogReport_btn, SIGNAL(clicked()),
	  ui->LogView, SLOT(writeLogReportDlg()));

  connect(ui->saveConfig_btn, SIGNAL(clicked()), 
	  this, SLOT(saveConfig())); 

  connect(ui->saveConfigAs_btn, SIGNAL(clicked()), 
	  this, SLOT(saveConfigAs_dlg()));
  connect(ui->openConfig_btn, SIGNAL(clicked()), 
	  this, SLOT(restoreConfig_dlg()));  

  connect(ui->Squelch_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setSquelchLevel(int)));
  setupAudioDeviceList(); 
}

void MainWindow::setupAudioDeviceList() 
{
  const QAudioDeviceInfo & def_rx_dev_info = QAudioDeviceInfo::defaultOutputDevice(); 
  ui->audioOut_cb->addItem(def_rx_dev_info.deviceName(), QVariant::fromValue(def_rx_dev_info));
  QAudioFormat format = GUISoDa::AudioRXListener::createAudioFormat();
  for(auto &rx_dev_info: QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
    if(rx_dev_info.isFormatSupported(format) && (rx_dev_info.deviceName() != def_rx_dev_info.deviceName())) {
      ui->audioOut_cb->addItem(rx_dev_info.deviceName(), QVariant::fromValue(rx_dev_info));
    }
  }

  // set the initial choice
  ui->audioOut_cb->setCurrentIndex(0);

  const QAudioDeviceInfo & def_tx_dev_info = QAudioDeviceInfo::defaultInputDevice(); 
  ui->audioIn_cb->addItem(def_tx_dev_info.deviceName(), QVariant::fromValue(def_tx_dev_info));
  for(auto &tx_dev_info: QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
    qInfo() << "Input audio device [" << tx_dev_info.deviceName() << "]\n";
    if(tx_dev_info.deviceName() != def_tx_dev_info.deviceName()) {
      qInfo() << "Adding device [" << tx_dev_info.deviceName() << "]\n";
      ui->audioIn_cb->addItem(tx_dev_info.deviceName(), QVariant::fromValue(tx_dev_info));
    }
  }
  
  // set the initial choice
  ui->audioIn_cb->setCurrentIndex(0);
}

void MainWindow::saveConfig()
{
  settings_p->beginGroup("Radio");
  bandMapSaveRestore(band_map, true);    
  widgetSaveRestore(this, "SoDaRadioQT.", true);
  settings_p->endGroup();
}

void MainWindow::saveConfigAs_dlg()
{
  QString fname = QFileDialog::getSaveFileName(this, 
					       tr("Save Configuration to File"), 
					       "", 
					       tr("*.conf (*.conf);;All Files(*)"));
  if(!fname.isEmpty()) {
    settings_p = new QSettings(fname, QSettings::NativeFormat, this);
    saveConfig();
  }
}

void MainWindow::restoreConfig_dlg()
{
  QString fname = QFileDialog::getOpenFileName(this, 
					       tr("Read Configuration from File"), 
					       "", 
					       tr("*.conf (*.conf);;All Files(*)"));
  if(!fname.isEmpty()) {
    settings_p = new QSettings(fname, QSettings::NativeFormat, this);
    restoreSettings();
  }
  
}
