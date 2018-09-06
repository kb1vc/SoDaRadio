/*
Copyright (c) 2017,2018 Matthew H. Reilly (kb1vc)
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
#include "version.h"
#include <boost/version.hpp>
#include <uhd/version.hpp>
#include <iostream>
#include <boost/format.hpp>

#include <QString>
#include <QMessageBox>
#include <QtCoreVersion>
#include <QtGlobal>

#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"
#include "../common/GuiParams.hxx"

using namespace GUISoDa;

MainWindow::MainWindow(QWidget *parent, SoDa::GuiParams & params) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // setup the listener. 
  listener = new GUISoDa::Listener(this, QString::fromStdString(params.getServerSocketBasename())); 

  // setup the audio listener
  audio_listener = new GUISoDa::AudioListener(this, QString::fromStdString(params.getServerSocketBasename()));
  
  setupSpectrum();
  setupWaterFall();
    
  setupTopControls();
  setupMidControls();
  setupLogGPS();

  setupSettings();
  setupBandConfig();
  setupLogEditor();
  setupStatus();

  // connect(listener, SIGNAL(repHWMBVersion(const QString &)), 
  // 	  this, SLOT(setWindowTitle(const QString &)));
  connect(listener, &GUISoDa::Listener::repHWMBVersion,
	  [=](const QString & hw) {
	    this->setWindowTitle(QString("SoDa Radio V %1 -- SDR %2").arg(SoDaRadio_VERSION).arg(hw));
	  });

  connect(listener, SIGNAL(initSetupComplete()), 
	  this, SLOT(restoreSettings()));

  connect(listener, SIGNAL(fatalError(const QString &)), 
	  this, SLOT(handleFatalError(const QString &)));

  // connect the audio listener to the rx selector combobox
  connect(ui->audioOut_cb, QOverload<int>::of(&QComboBox::currentIndexChanged),
	  [=](int index) {
	    audio_listener->getRX()->setRXDevice(ui->audioOut_cb->itemData(index).value<QAudioDeviceInfo>());
	  });

  connect(audio_listener->getRX(), SIGNAL(bufferSlack(const QString &)), 
	  ui->slack_lab, SLOT(setText(const QString &)));

  connect(ui->Record_chk, &QCheckBox::stateChanged,
	  [=](int changed) {
	    audio_listener->getRec()->record(changed != Qt::Unchecked);
	  });

  connect(ui->recDir_btn, &QPushButton::clicked,
	  [=]() {
	    audio_listener->getRec()->getRecDirectory(this); 
	  });

  
  connect(ui->aboutSoDa_btn, SIGNAL(clicked(bool)), 
	  this, SLOT(displayAppInfo(bool)));

  settings_p = new QSettings("kb1vc.org", "SoDaRadioQT", this);

  current_band_selector = ui->bandSel_cb->currentText(); 
  auto_bandswitch_target = QString("");


  listener->init();
  listener->start();
  audio_listener->init();
  audio_listener->start();
  
  hlib_server = new HamlibServer(this, params.getHamlibPortNumber());
  
  hlib_server->start();

  setupHamlib();  
}

MainWindow::~MainWindow()
{
  saveConfig();
  delete ui;
}

void MainWindow::displayAppInfo(bool dummy)
{
  (void) dummy; 

  QMessageBox::about(this, QString("SoDaRadio"), 
		     QString("<h1>SoDaRadio</h1> \
<p>An all-mode SDR application for the Ettus USRP platform.</p> \
<ul> \
<li>SoDaRadio Version: %1</li><li>Git ID: %2</li>\
<li>USRP Hardware Driver Version: %3</li>\
<li>Qt Version: %4</li> \
<li>Boost Version: %5</li> \
<li>Sources and Such: https://kb1vc.github.io/SoDaRadio/</li> \
<li>Maintainer: kb1vc@kb1vc.org</li> \
</ul>\
<h2>License:</h2> \
<p> \
Copyright (c) 2017, 2018 Matthew H. Reilly (kb1vc) \
All rights reserved.</p> \
<p>Redistribution and use in source and binary forms, with or without \
modification, are permitted provided that the following conditions are \
met:</p> \
<p><b>Redistributions of source code must retain the above copyright	\
    notice, this list of conditions and the following disclaimer. \
    Redistributions in binary form must reproduce the above copyright \
    notice, this list of conditions and the following disclaimer in \
    the documentation and/or other materials provided with the \
    distribution.</b></p> \
<p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \
\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT \
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR \
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT \
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, \
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT \
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, \
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY \
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT \
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE \
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. \
</p>").arg(SoDaRadio_VERSION).arg(SoDaRadio_GIT_ID).arg(UHD_VERSION_ABI_STRING).arg(QTCORE_VERSION_STR).arg(BOOST_LIB_VERSION));
}

void MainWindow::widgetSaveRestore(QObject * op, const QString & par, bool save)
{
  QObjectList wlist = op->children();
  for(QObjectList::iterator cp = wlist.begin(); cp != wlist.end(); ++cp) {
    QString my_name = (*cp)->objectName();
    QString my_class = (*cp)->metaObject()->className();

    QString my_pathname = par + "." + my_name;     
    if(my_class == "QComboBox") {
      if(QComboBox * cb = qobject_cast<QComboBox*>(*cp)) {
	if(save) {
	  int ci = cb->currentIndex();
	  settings_p->setValue(my_pathname, ci);
	}
	else {
	  // restore!
	  int nvalue = settings_p->value(my_pathname, 0).toInt();
	  cb->setCurrentIndex(nvalue);
	}
      }
    }
    else if(my_class == "QSlider") {
      if(QSlider * cb = qobject_cast<QSlider*>(*cp)) {      
	if(save) {
	  double cv = cb->value();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  double nvalue = settings_p->value(my_pathname, 0).toDouble();
	  cb->setValue(nvalue);
	}
      }
    }
    else if(my_class == "GUISoDa::FreqLabel") {
      if(FreqLabel * cb = qobject_cast<FreqLabel *>(*cp)) {      
	if(save) {
	  double cv = cb->getFreq();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  double nvalue = settings_p->value(my_pathname, 144.2e6).toDouble();
	  cb->setFreqUpdate(nvalue);
	}
      }
    }
    else if(my_class == "QCheckBox") {
      if(QCheckBox * cb = qobject_cast<QCheckBox *>(*cp)) {      
	if(save) {
	  bool cv = cb->isChecked();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  bool nvalue = settings_p->value(my_pathname, 0).toBool();
	  cb->setChecked(nvalue);
	}
      }
    }
    else if((my_class == "QSpinBox") || (my_class == "SoDaNoEditSpinbox")) {
      if(QSpinBox * cb = qobject_cast<QSpinBox*>(*cp)) {      
	if(save) {
	  int cv = cb->value();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  int nvalue = settings_p->value(my_pathname, 0).toInt();
	  cb->setValue(nvalue);
	}
      }
    }
    else if(my_class == "QLineEdit") {
      if(QLineEdit * cb = qobject_cast<QLineEdit*>(*cp)) {      
	if(save) {
	  QString cv = cb->text();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  QString nvalue = settings_p->value(my_pathname, "").toString();
	  cb->setText(nvalue);
	}
      }
    }
    else if(my_class == "GUISoDa::WFall") {
      if(GUISoDa::WFall * cb = qobject_cast<GUISoDa::WFall*>(*cp)) {      
	if(save) {
	  double cv = cb->freqCenter();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  double nvalue = settings_p->value(my_pathname, 0).toDouble();
	  cb->setFreqCenter(nvalue);
	}
      }
    }
    else if(my_class == "GUISoDa::Spect") {
      if(GUISoDa::Spect * cb = qobject_cast<GUISoDa::Spect*>(*cp)) {      
	if(save) {
	  double cv = cb->freqCenter();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  double nvalue = settings_p->value(my_pathname, 0).toDouble();
	  cb->setFreqCenter(nvalue);
	}
      }
    }
    else if(my_class == "GUISoDa::IntValComboBox") {
      if(GUISoDa::IntValComboBox * cb = qobject_cast<GUISoDa::IntValComboBox*>(*cp)) {      
	if(save) {
	  int cv = cb->value();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  int nvalue = settings_p->value(my_pathname, 0).toInt();
	  cb->setValue(nvalue);
	}
      }
    }
    else if((my_class == "GUISoDa::WFallSpanComboBox") || 
	    (my_class == "GUISoDa::WFallDynRangeComboBox")) {
      if(GUISoDa::ValComboBox * cb = qobject_cast<GUISoDa::ValComboBox*>(*cp)) {      
	if(save) {
	  double cv = cb->value();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  double nvalue = settings_p->value(my_pathname, 0).toDouble();
	  cb->setValue(nvalue);
	}
      }
    }
    else if(my_class == "QTabBar") {
      if(QTabBar * cb = qobject_cast<QTabBar*>(*cp)) {      
	if(save) {
	  int cv = cb->currentIndex();
	  settings_p->setValue(my_pathname, cv);
	}
	else {
	  // restore!
	  int nvalue = settings_p->value(my_pathname, 0).toInt();
	  cb->setCurrentIndex(nvalue);
	}
      }
    }
    else if((my_class == "QAction") ||
	    (my_class == "QBoxLayout") ||
	    (my_class == "QDoubleValidator") ||
	    (my_class == "QFormLayout") ||	    
	    (my_class == "QGroupBox") ||	    
	    (my_class == "QHBoxLayout") ||
	    (my_class == "QHeaderView") ||	    
	    (my_class == "QItemSelectionModel") ||	    
	    (my_class == "QLabel") ||
	    (my_class == "QLocalSocket") ||
	    (my_class == "QMainWindowLayout") ||
	    (my_class == "QNativeSocketEngine") ||
	    (my_class == "QPlainTextDocumentLayout") ||	    
	    (my_class == "QPlainTextEdit") ||
	    (my_class == "QPlainTextEditControl") ||	    
	    (my_class == "QPropertyAnimation") ||	    
	    (my_class == "QPushButton") ||
	    (my_class == "QScrollBar") ||
	    (my_class == "QSettings") ||
	    (my_class == "QSocketNotifier") ||
	    (my_class == "QSplitter") ||
	    (my_class == "QSplitterHandle") ||
	    (my_class == "QStackedLayout") ||	    
	    (my_class == "QStackedWidget") ||
	    (my_class == "QStandardItemModel") ||
	    (my_class == "QStyledItemDelegate") ||	    
	    (my_class == "QTableCornerButton") ||	    
	    (my_class == "QTableModel") ||	    
	    (my_class == "QTabWidget") ||
	    (my_class == "QTcpSocket") ||
	    (my_class == "QTextDocument") ||
	    (my_class == "QTextDocumentLayout") ||
	    (my_class == "QTextFrame") ||
	    (my_class == "QTextImageHandler") ||
	    (my_class == "QToolButton") ||
	    (my_class == "QValidator") ||
	    (my_class == "QVBoxLayout") ||
	    (my_class == "QWidget") ||	    
	    (my_class == "QWidgetLineControl") ||
	    (my_class == "QWidgetTextControl") ||
	    (my_class == "QwtPlotCanvas") ||
	    (my_class == "QwtPlotPicker") ||	    	    
	    (my_class == "QwtScaleWidget") ||
	    (my_class == "QwtTextLabel")) {
      // do nothing. 
    }
    widgetSaveRestore((*cp), my_pathname, save);
  }
}

void MainWindow::restoreSettings()
{
  settings_p->beginGroup("Radio");
  bandMapSaveRestore(band_map, false);  
  widgetSaveRestore(this, "SoDaRadioQT.", false);
  settings_p->endGroup();
}

void MainWindow::handleFatalError(const QString & err_string) 
{
  QMessageBox mbox(QMessageBox::Critical, 
		   tr("Fatal Error"), 
		   tr("%1 has encountered an error that is beyond safe recovery.\n"
		      "Please press OK button to quit. (Though this is -not- OK.\n"
		      "Send a note when you get a chance to kb1vc@kb1vc.org").arg(qApp->applicationDisplayName()), 
		   QMessageBox::Ok, this);
  mbox.setDetailedText(err_string); 
  mbox.exec();
  qApp->quit();
}
