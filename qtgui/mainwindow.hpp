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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSettings>
#include "soda_listener.hpp"
#include "../common/GuiParams.hxx"
#include "soda_band.hpp"
#include "soda_hamlib_server.hpp"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent, SoDa::GuiParams & params);
  ~MainWindow();

public slots:
  void setRXFreq(double freq);
  void setTXFreq(double freq);
  void restoreSettings(); 

  void changeBand(const QString & band);
  void writeBandMapEntry(bool);
  void fillBandMapEntry(const QString & band);

  void handleFatalError(const QString & err_string);

  void logContact(bool); 

  void saveConfig();
  void saveConfigAs_dlg();
  void restoreConfig_dlg();

protected:
  void setupTopControls();
  void setupMidControls();
  void setupLogGPS();

  void setupSettings();
  void setupBandConfig();
  void setupLogEditor();
  
  void setupWaterFall();
  void setupSpectrum();

  void setupHamlib(); 

  void widgetSaveRestore(QObject * op, const QString & par, bool save);

  void bandMapSaveRestore(SoDaBandMap & bmap, bool save);
  void saveCurrentFreqs();
  
  void sendCannedCW(const QString & txt);
  
private:
  void closeEvent(QCloseEvent * event) {
    listener->closeRadio();
    event->accept();
  }

  QSettings * settings_p; 

  // Band map
  QMap<QString, SoDaBand> band_map; 
  QString current_band_selector; 

  void setRXFreq_nocross(double freq);
  void setTXFreq_nocross(double freq);   
  
  Ui::MainWindow *ui;

  SoDaListener * listener; 

  SoDaHamlibServer * hlib_server; 
};

#endif // MAINWINDOW_H
