/*
Copyright (c) 2017,2018,2019,2020,2025 Matthew H. Reilly (kb1vc)
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
#include <uhd/version.hpp>
#include <iostream>

#include <QString>
#include <QMessageBox>
#include <QtCoreVersion>
#include <QtGlobal>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>
#ifdef HAVE_QT6_WEBENGINE
#include <QWebEngineView>
#endif

#include "help_path.h"
#include "soda_comboboxes.hpp"
#include "RadioListener.hpp"
#include "../common/GuiParams.hxx"
#include "VUMeter.hpp"
#include <QShortcut>

#include "SoDaLogo_Big.xpm"

using namespace GUISoDa;

MainWindow::MainWindow(QWidget *parent, SoDa::GuiParams & params) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  qDebug() << QString("MainWIndow about to start UI");  
  ui->setupUi(this);
  qDebug() << QString("MainWIndow started UI");  
  // setup the listener. 
  radio_listener = new GUISoDa::RadioListener(this, QString::fromStdString(params.getServerSocketBasename())); 

  // setup the audio listener
  qDebug() << QString("About to start AudioListener");
  audio_listener = new GUISoDa::AudioListener(this, QString::fromStdString(params.getServerSocketBasename()));
  qDebug() << QString("Started AudioListener");

  // setup the TX audio server (captures mic/virtual-device audio and ships it
  // to the radio server's _txa unix socket)
  qDebug() << QString("About to start AudioTXServer");
  audio_tx_server = new GUISoDa::AudioTXServer(this, QString::fromStdString(params.getServerSocketBasename()));
  qDebug() << QString("Started AudioTXServer");

  setupBandConfig();  
  setupSpectrum();
  setupWaterFall();
    
  setupTopControls();
  setupMidControls();
  setupLogGPS();

  setupSettings();
  setupLogEditor();
  setupStatus();

  qDebug() << QString("Setup all the settings and UI components.");
  
  QPixmap logo_pixmap(SoDaLogo_Big);
  QIcon app_icon(logo_pixmap);
  this->setWindowIcon(app_icon);

  tray_icon = new QSystemTrayIcon(this);
  tray_icon->setIcon(app_icon);
  tray_icon->setToolTip(QString("SoDaRadio"));
  tray_icon->show();
  
  // connect(listener, SIGNAL(repHWMBVersion(const QString &)), 
  // 	  this, SLOT(setWindowTitle(const QString &)));
  connect(radio_listener, &GUISoDa::RadioListener::repHWMBVersion,
	  [=](const QString & hw) {
	    this->setWindowTitle(QString("SoDa Radio V %1 -- SDR %2").arg(SoDaRadio_VERSION).arg(hw));
	  });

  connect(radio_listener, SIGNAL(initSetupComplete()),
	  this, SLOT(restoreSettings()));

  // Start with all TX controls disabled.  If the radio reports any TX
  // antenna, we know it's a transmit-capable rig and re-enable them.
  setTXEnabled(false);
  connect(radio_listener, &GUISoDa::RadioListener::addTXAntName,
	  [this](const QString &) { setTXEnabled(true); });

  connect(radio_listener, SIGNAL(fatalError(const QString &)), 
	  this, SLOT(handleFatalError(const QString &)));

  // connect the audio listener to the rx selector combobox
  connect(ui->audioOut_cb, QOverload<int>::of(&QComboBox::currentIndexChanged),
	  [=](int index) {
	    int ridx = ((index >= 0) && (index < ui->audioOut_cb->count())) ? index : 0;
	    audio_listener->getRX()->setRXDevice(ui->audioOut_cb->itemData(ridx).value<QAudioDevice>());
	  });

  // connect the TX audio server to the tx input device combobox
  connect(ui->audioIn_cb, QOverload<int>::of(&QComboBox::currentIndexChanged),
	  [=](int index) {
	    int ridx = ((index >= 0) && (index < ui->audioIn_cb->count())) ? index : 0;
	    QAudioDevice dev = ui->audioIn_cb->itemData(ridx).value<QAudioDevice>();
	    audio_tx_server->changeDevice(dev);
	  });

  connect(audio_listener->getRX(), SIGNAL(bufferSlack(const QString &)),
	  ui->slack_lab, SLOT(setText(const QString &)));

  // VU meter: small floating window, toggled by Alt+V
  vu_meter = new GUISoDa::VUMeter(this);
  connect(new QShortcut(QKeySequence(Qt::ALT | Qt::Key_V), this), &QShortcut::activated,
	  [this]() { vu_meter->setVisible(!vu_meter->isVisible()); });
  connect(audio_listener->getRX(), &GUISoDa::AudioRXListener::pendAudioBuffer,
	  vu_meter, &GUISoDa::VUMeter::updateLevel);

  // Keyboard shortcuts reference: Alt+H and Alt+?
  auto showShortcuts = [this]() { displayKeyboardShortcuts(); };
  connect(new QShortcut(QKeySequence(Qt::ALT | Qt::Key_H),        this), &QShortcut::activated, showShortcuts);
  connect(new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Question), this), &QShortcut::activated, showShortcuts);

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
  connect(ui->Record_chk, &QCheckBox::checkStateChanged,
	  [=](Qt::CheckState state) {
	    audio_listener->getRec()->record(state != Qt::Unchecked);
	  });

  connect(ui->RecordRF_chk, &QCheckBox::checkStateChanged,
	  [=](Qt::CheckState state) { radio_listener->recordRF(static_cast<int>(state)); });
#else
  connect(ui->Record_chk, &QCheckBox::stateChanged,
	  [=](int changed) {
	    audio_listener->getRec()->record(changed != Qt::Unchecked);
	  });

  connect(ui->RecordRF_chk, SIGNAL(stateChanged(int)),
	  radio_listener, SLOT(recordRF(int)));
#endif

  connect(ui->recDir_btn, &QPushButton::clicked,
	  [=]() {
	    audio_listener->getRec()->getRecDirectory(this); 
	  });

  
  connect(ui->aboutSoDa_btn, SIGNAL(clicked(bool)),
	  this, SLOT(displayAppInfo(bool)));

  connect(ui->help_btn, SIGNAL(clicked(bool)),
	  this, SLOT(displayHelp(bool)));

  qDebug() << QString("connected stuff");
  QString radio_name = QString::fromStdString(params.getRadioName());
  QString settings_app = radio_name.isEmpty()
    ? QString("SoDaRadioQT")
    : radio_name.toUpper() + QString("_SoDaRadioQT");
  settings_p = new QSettings("kb1vc.org", settings_app, this);

  current_band_selector = ui->bandSel_cb->currentText(); 
  auto_bandswitch_target = QString("");

  qDebug() << QString("About to init radio_listener");  
  radio_listener->init();
  qDebug() << QString("About to start radio_listener");
  radio_listener->start();
  
  qDebug() << QString("About to init audo_listener");        
  audio_listener->init();
  if(ui->audioOut_cb->count() > 0) {
    audio_listener->getRX()->setRXDevice(
      ui->audioOut_cb->itemData(ui->audioOut_cb->currentIndex()).value<QAudioDevice>());
  }
  qDebug() << QString("About to start audo_listener");
  audio_listener->start();

  qDebug() << QString("About to starting hamlib server");        
  hlib_server = new HamlibServer(this, params.getHamlibPortNumber());
  hlib_server->start();
  setupHamlib();
  qDebug() << QString("setup hamlib server");          
}

MainWindow::~MainWindow()
{
  if (settings_loaded) saveConfig();
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
<li>Sources and Such: https://kb1vc.github.io/SoDaRadio/</li> \
<li>Maintainer: kb1vc@kb1vc.org</li> \
</ul>\
<h2>License:</h2> \
<p> \
Copyright (c) 2026 Matthew H. Reilly (kb1vc) \
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
</p>").arg(SoDaRadio_VERSION).arg(SoDaRadio_GIT_ID).arg(UHD_VERSION_ABI_STRING).arg(QTCORE_VERSION_STR));
}

void MainWindow::displayKeyboardShortcuts()
{
  QMessageBox::information(this, "SoDaRadio Keyboard Shortcuts",
    "Alt+V       Toggle VU meter\n"
    "Alt+H       This keyboard shortcuts reference\n"
    "Alt+?       This keyboard shortcuts reference");
}

void MainWindow::displayHelp(bool) {
  QStringList candidates = {
    QString(SODARADIO_HTML_BUILD_PATH) + "/index.html",
    QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../share/doc/SoDaRadio/html/index.html"),
  };

  QString indexPath;
  for (const auto &c : candidates) {
    if (QFile::exists(c)) { indexPath = c; break; }
  }

  if (indexPath.isEmpty()) {
    QMessageBox::information(this, "SoDaRadio Help",
      "Help documentation not found.\n\nRun 'make doc' from the build directory to generate it.");
    return;
  }

  auto *dlg = new QDialog(this);
  dlg->setWindowTitle("SoDaRadio Help");
  dlg->resize(1000, 750);
  auto *layout = new QVBoxLayout(dlg);

#ifdef HAVE_QT6_WEBENGINE
  auto *view = new QWebEngineView(dlg);
  view->load(QUrl::fromLocalFile(indexPath));
  layout->addWidget(view);
#else
  auto *browser = new QTextBrowser(dlg);
  browser->setSource(QUrl::fromLocalFile(indexPath));
  browser->setOpenExternalLinks(true);
  auto *backBtn = new QPushButton("< Back", dlg);
  auto *fwdBtn  = new QPushButton("Forward >", dlg);
  connect(backBtn, &QPushButton::clicked, browser, &QTextBrowser::backward);
  connect(fwdBtn,  &QPushButton::clicked, browser, &QTextBrowser::forward);
  auto *btnRow = new QHBoxLayout();
  btnRow->addWidget(backBtn);
  btnRow->addWidget(fwdBtn);
  btnRow->addStretch();
  layout->addLayout(btnRow);
  layout->addWidget(browser);
#endif

  dlg->setAttribute(Qt::WA_DeleteOnClose);
  dlg->show();
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
	  // restore! Fall back to the widget's current value (set from the .ui
	  // file) so new sliders don't get silently defaulted to 0.
	  double nvalue = settings_p->value(my_pathname, (double)cb->value()).toDouble();
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
      qDebug() << QString("my_class = [%1]\n").arg(my_class);
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
  settings_loaded = true;
}

void MainWindow::setTXEnabled(bool enabled)
{
  // Enable or grey out every transmit-related control.  Called with
  // false at startup; flipped to true when the radio reports any TX
  // antenna name (RX-only rigs never send one).
  QWidget * tx_widgets[] = {
    ui->PTT_btn,
    ui->TXState_lab,
    ui->TXFreq_box,
    ui->TXAnt_sel,
    ui->CWCurLine_le,
    ui->CWOutBound_te,
    ui->Exchange_btn,
    ui->MyInfo_btn,
    ui->MyCall_btn,
    ui->MyGrid_btn,
    ui->CWQSL_btn,
    ui->CW73_btn,
    ui->CWBK_btn,
    ui->CWV_btn,
    ui->RptCount_spin,
    ui->RptCount_lab,
    ui->Carrier_btn,
    ui->ClrBuff_btn,
    ui->groupBox_9,    // TX Input (settings panel)
    ui->TXPower_box,
    ui->TXAFGain_box,
    ui->CWSpeed_box,
    ui->Sidetone_box
  };

  for (QWidget * w : tx_widgets) {
    if (w) w->setEnabled(enabled);
  }
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
