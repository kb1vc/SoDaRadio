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
#include "help_path.h"
#include <uhd/version.hpp>
#include <iostream>

#include <QString>
#include <QMessageBox>
#include <QtCoreVersion>
#include <QtGlobal>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFontMetrics>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTableView>
#include <QTextEdit>
#include <QLabel>
#include <QMenu>
#include <QActionGroup>
#include <QShortcut>
#include <QKeySequence>
#include <QApplication>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDialog>
#include <QFile>
#include <QPushButton>
#include <QTextBrowser>
#include <QUrl>
#ifdef HAVE_QT6_WEBENGINE
#include <QWebEngineView>
#endif
#include <QPixmap>
#include <QProcess>

#include "soda_comboboxes.hpp"
#include "RadioListener.hpp"
#include "../common/GuiParams.hxx"

#include "SoDaLogo_Big.xpm"

using namespace GUISoDa;

MainWindow::MainWindow(QWidget *parent, SoDa::GuiParams & params) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  qDebug() << QString("MainWIndow about to start UI");
  ui->setupUi(this);
  qDebug() << QString("MainWIndow started UI");

  // Re-arrange the inherited qtgui layout for the ShortSoDa tabbed touchscreen UI.
  reorganizeForShortSoDa();

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
  setupSpectControls();
    
  setupTopControls();
  setupMidControls();
  setupLogGPS();

  setupSettings();
  setupLogEditor();
  setupStatus();

  qDebug() << QString("Setup all the settings and UI components.");

  // ShortSoDa_Lib is a static library, so the global static initializer
  // that AUTORCC emits in qrc_shortsoda_resources.cpp can be stripped by
  // the linker.  Force resource registration explicitly.
  Q_INIT_RESOURCE(shortsoda_resources);

  // Modern Plasma / Gnome / Wayland trays use StatusNotifierItem over
  // D-Bus and resolve the icon through the freedesktop icon theme by
  // application name.  setDesktopFileName + setApplicationName make the
  // tray host look up an installed "ShortSoDa.png" (or .desktop entry)
  // instead of falling back to the generic "missing application" tile.
  qApp->setApplicationName("ShortSoDa");
  qApp->setOrganizationName("kb1vc.org");
  qApp->setDesktopFileName("ShortSoDa");

  // Build the application/tray icon.  Prefer the high-quality 128x128 RGBA
  // PNG that is compiled in via shortsoda_resources.qrc; fall back to the
  // legacy XPM if the resource isn't available for some reason.  We add
  // several pre-scaled sizes so the StatusNotifier / xembed tray on KDE,
  // Gnome, XFCE etc. picks a sharp version instead of dropping back to a
  // generic placeholder.
  QIcon app_icon;
  QPixmap big;
  bool from_resource = big.load(":/SoDaLogo_Big.png");
  if (!from_resource) {
    qWarning() << "ShortSoDa: failed to load :/SoDaLogo_Big.png; falling back to XPM";
    big = QPixmap(SoDaLogo_Big);
  }
  if (!big.isNull()) {
    for (int sz : {16, 22, 24, 32, 48, 64, 128, 256}) {
      app_icon.addPixmap(big.scaled(sz, sz, Qt::KeepAspectRatio,
				    Qt::SmoothTransformation));
    }
  }
  // Status-tray icons must be drawn as full-color images, not as
  // monochrome masks.  Some Qt/KDE combinations default to mask mode and
  // render the icon as a flat grey rectangle if this isn't set false.
  app_icon.setIsMask(false);

  if (app_icon.isNull()) {
    qWarning() << "ShortSoDa: tray icon QIcon is empty -- nothing to draw";
  }

  this->setWindowIcon(app_icon);
  qApp->setWindowIcon(app_icon);  // also sets the default app icon

  // Drop the PNG at every standard hicolor size into ~/.local/share/icons so
  // the Wayland compositor (GNOME Shell, Plasma) can find the icon regardless
  // of which size it requests first.  GNOME typically looks for 48x48 and
  // won't fall back to 128x128 unless the theme index says otherwise.
  // Use 48x48 as the sentinel: if it already exists we assume the full set
  // was written on a previous run.
  if (from_resource) {
    QString genShare = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    static const int kSizes[] = { 16, 22, 24, 32, 48, 64, 128, 256 };
    bool anyWritten = false;
    QPixmap srcPix(":/SoDaLogo_Big.png");
    for (int sz : kSizes) {
      QString dir  = genShare + QString("/icons/hicolor/%1x%1/apps").arg(sz);
      QString path = dir + "/ShortSoDa.png";
      if (!QDir().exists(dir)) QDir().mkpath(dir);
      if (!QFile::exists(path) && !srcPix.isNull()) {
        QPixmap scaled = srcPix.scaled(sz, sz,
                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (scaled.save(path, "PNG")) {
          QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner |
                                      QFile::ReadGroup  | QFile::ReadOther);
          anyWritten = true;
        }
      }
    }
    if (anyWritten) {
      // Refresh the icon cache so the compositor sees the new files.
      QProcess::startDetached("gtk-update-icon-cache",
                              {"-f", "-t", genShare + "/icons/hicolor"});
    }

    QString appDir  = genShare + "/applications";
    QString appPath = appDir + "/ShortSoDa.desktop";
    if (QDir().mkpath(appDir) && !QFile::exists(appPath)) {
      QFile::copy(":/ShortSoDa.desktop", appPath);
      QFile::setPermissions(appPath, QFile::ReadOwner | QFile::WriteOwner |
                                     QFile::ReadGroup  | QFile::ReadOther);
      QProcess::startDetached("update-desktop-database", {appDir});
    }
  }

  tray_icon = new QSystemTrayIcon(this);
  tray_icon->setIcon(app_icon);
  tray_icon->setToolTip(QString("ShortSoDa"));
  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    tray_icon->show();
    // Re-assert the icon after show().  Some SNI hosts cache the first
    // icon they see at registration time and ignore subsequent
    // pixmap-only updates; setting it again here forces a refresh.
    tray_icon->setIcon(app_icon);
  }
  
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

  // ShortSoDa: when PTT engages, auto-switch to the Transmit tab; when
  // PTT releases, return to Waterfall.  Honors RX-only radios via the
  // PTT_btn enabled state.
  connect(ui->PTT_btn, &QPushButton::toggled,
	  [this](bool on) {
	    if (!isTXCapable()) return;
	    if (on && tab_idx_transmit >= 0) {
	      ui->Display_tabs->setCurrentIndex(tab_idx_transmit);
	    } else if (!on && tab_idx_waterfall >= 0) {
	      ui->Display_tabs->setCurrentIndex(tab_idx_waterfall);
	    }
	  });

  // Show/hide the QSO-info and frequency-control bars depending on the
  // active tab.  Those bars only belong above the operating tabs
  // (Waterfall, Periodogram, Transmit) per the spec.
  connect(ui->Display_tabs, &QTabWidget::currentChanged,
	  [this](int idx) {
	    bool show_bars = (idx == tab_idx_waterfall) ||
	                     (idx == tab_idx_periodogram) ||
	                     (idx == tab_idx_transmit);
	    if (qso_info_bar)  qso_info_bar->setVisible(show_bars);
	    if (freq_ctrl_bar) freq_ctrl_bar->setVisible(show_bars);
	  });

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

  // VU meter: floating window, toggled by Alt+V (shortcut registered in reorganizeForShortSoDa)
  vu_meter = new GUISoDa::VUMeter(this);
  connect(audio_listener->getRX(), &GUISoDa::AudioRXListener::pendAudioBuffer,
	  vu_meter, &GUISoDa::VUMeter::updateLevel);

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

void MainWindow::displayHelp(bool) {
  QStringList candidates = {
    QString(SODARADIO_HTML_BUILD_PATH) + "/index.html",
    QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../share/doc/html/index.html"),
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

  // Populate the spectrum-panel band selectors from the now-loaded bandSel_cb.
  for (QComboBox * cb : {wf_band_cb, sp_band_cb}) {
    if (!cb) continue;
    QSignalBlocker blk(cb);
    cb->clear();
    for (int i = 0; i < ui->bandSel_cb->count(); ++i)
      cb->addItem(ui->bandSel_cb->itemText(i));
    cb->setCurrentText(ui->bandSel_cb->currentText());
  }

  // Block mirror-widget signals so widgetSaveRestore doesn't fire changeBand
  // or setAFGain/setRXGain mid-restore and corrupt the band/gain state.
  QWidget * mirrors[] = {
    wf_band_cb, sp_band_cb,
    wf_af_gain_sl, sp_af_gain_sl,
    wf_rf_gain_sl, sp_rf_gain_sl
  };
  for (QWidget * w : mirrors) if (w) w->blockSignals(true);

  widgetSaveRestore(this, "SoDaRadioQT.", false);

  for (QWidget * w : mirrors) if (w) w->blockSignals(false);

  // Re-sync mirrors from masters (widgetSaveRestore restored the masters).
  if (wf_band_cb) { QSignalBlocker b(wf_band_cb); wf_band_cb->setCurrentText(ui->bandSel_cb->currentText()); }
  if (sp_band_cb) { QSignalBlocker b(sp_band_cb); sp_band_cb->setCurrentText(ui->bandSel_cb->currentText()); }
  if (wf_af_gain_sl) { QSignalBlocker b(wf_af_gain_sl); wf_af_gain_sl->setValue(ui->AFGain_slide->value()); }
  if (sp_af_gain_sl) { QSignalBlocker b(sp_af_gain_sl); sp_af_gain_sl->setValue(ui->AFGain_slide->value()); }
  if (wf_rf_gain_sl) { QSignalBlocker b(wf_rf_gain_sl); wf_rf_gain_sl->setValue(ui->RFGain_slide->value()); }
  if (sp_rf_gain_sl) { QSignalBlocker b(sp_rf_gain_sl); sp_rf_gain_sl->setValue(ui->RFGain_slide->value()); }

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
    ui->TXFreq_box,       // hidden in ShortSoDa, but disable it anyway
    ui->TXFreq_lab,       // extracted from GroupBox in ShortSoDa layout
    ui->TXfreq2RXfreq_btn,
    ui->TXRXLock_chk,
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

// ---------------------------------------------------------------------------
// ShortSoDa-specific layout reorganization for 7" touchscreen.
//
// The qtgui .ui file is reused as-is.  Here we re-parent the widgets from the
// original toolbar-style layout into a clean two-row header (QSO info + freq
// control) above a 7-tab QTabWidget: Waterfall, Periodogram, Transmit,
// Settings, Config Band, Edit Log, Status.
// ---------------------------------------------------------------------------
void MainWindow::reorganizeForShortSoDa()
{
  // Resize to fit a 7" LCD (1024x600 is the common touchscreen panel).
  setMinimumSize(800, 480);
  resize(1024, 600);
  // Override the .ui-imposed central-widget size constraints.
  if (ui->centralWidget) {
    ui->centralWidget->setMinimumSize(0, 0);
    ui->centralWidget->setMaximumSize(16777215, 16777215);
  }

  // --- Build the new Transmit tab (CW text editor + macro buttons). -------
  QWidget * transmitTab = new QWidget(ui->Display_tabs);
  QVBoxLayout * txLayout = new QVBoxLayout(transmitTab);

  // CW text editor above; a mirror view of the log below.
  txLayout->addWidget(ui->CWTXBox, 1);

  // Read-only mirror of the log table.  Shares the same QAbstractItemModel
  // as ui->LogView so it updates automatically as contacts are logged or
  // edited on the Edit Log tab.
  QTableView * tx_log_view = new QTableView(transmitTab);
  tx_log_view->setModel(ui->LogView->model());
  tx_log_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tx_log_view->setSelectionMode(QAbstractItemView::NoSelection);
  tx_log_view->setFocusPolicy(Qt::NoFocus);
  tx_log_view->verticalHeader()->hide();
  tx_log_view->horizontalHeader()->setStretchLastSection(true);
  // Fix the view to exactly 5 rows + header + frame.
  {
    int rowH  = tx_log_view->verticalHeader()->defaultSectionSize();
    int hdrH  = tx_log_view->horizontalHeader()->sizeHint().height();
    int frame = 2 * tx_log_view->frameWidth();
    tx_log_view->setFixedHeight(rowH * 5 + hdrH + frame);
  }
  // LogTable::logContact pre-allocates rows in chunks of 20 (setRowCount),
  // so scrollToBottom() would scroll to an empty pre-allocated row.  Track
  // the largest row that has actually had a cell written and scroll to it.
  //   fewer than 5 filled → scrollToTop (rows fill in from the top)
  //   5+ filled           → scrollTo(last, PositionAtBottom) so the newest
  //                         5 fill the viewport.
  auto max_filled = std::make_shared<int>(-1);
  auto updateScroll = [tx_log_view, max_filled]() {
    if (*max_filled < 4) {
      tx_log_view->scrollToTop();
    } else {
      QModelIndex idx = tx_log_view->model()->index(*max_filled, 0);
      tx_log_view->scrollTo(idx, QAbstractItemView::PositionAtBottom);
    }
  };
  connect(ui->LogView->model(), &QAbstractItemModel::dataChanged,
          tx_log_view,
          [max_filled, updateScroll](const QModelIndex &,
                                     const QModelIndex & br,
                                     const QList<int> &) {
    if (br.row() > *max_filled) *max_filled = br.row();
    updateScroll();
  });
  connect(ui->LogView->model(), &QAbstractItemModel::modelReset,
          tx_log_view,
          [max_filled, updateScroll]() {
    *max_filled = -1;
    updateScroll();
  });
  updateScroll();
  txLayout->addWidget(tx_log_view);

  // CW macro buttons split across two rows of touch-friendly buttons.
  QHBoxLayout * cwRow1 = new QHBoxLayout();
  cwRow1->addWidget(ui->Exchange_btn);
  cwRow1->addWidget(ui->MyInfo_btn);
  cwRow1->addWidget(ui->MyCall_btn);
  cwRow1->addWidget(ui->MyGrid_btn);
  txLayout->addLayout(cwRow1);

  QHBoxLayout * cwRow2 = new QHBoxLayout();
  cwRow2->addWidget(ui->CWQSL_btn);
  cwRow2->addWidget(ui->CW73_btn);
  cwRow2->addWidget(ui->CWBK_btn);
  cwRow2->addWidget(ui->CWV_btn);
  cwRow2->addWidget(ui->Carrier_btn);
  cwRow2->addWidget(ui->ClrBuff_btn);
  cwRow2->addWidget(ui->RptCount_lab);
  ui->RptCount_spin->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  cwRow2->addWidget(ui->RptCount_spin);
  cwRow2->addStretch(1);
  txLayout->addLayout(cwRow2);

  // Ctrl+key macro shortcuts, active only when a widget on the Transmit tab
  // has focus.  The event filter above auto-focuses CWTXBox on this tab, so
  // these fire whenever the tab is active.  Labels show the binding.
  ui->Exchange_btn->setText(tr("Exchange (^E)"));
  ui->MyInfo_btn->setText(tr("My Info (^I)"));
  ui->MyCall_btn->setText(tr("My Call (^K)"));
  ui->MyGrid_btn->setText(tr("My Grid (^G)"));
  ui->CWBK_btn->setText(tr("BK (^B)"));
  ui->CWQSL_btn->setText(tr("QSL (^R)"));
  // Ctrl+E is intercepted in eventFilter() instead — QLineEdit accepts its
  // ShortcutOverride, so a QShortcut for Ctrl+E would never fire here.
  struct TxShortcut { Qt::Key key; QPushButton * btn; };
  for (const TxShortcut & s : {
        TxShortcut{Qt::Key_I, ui->MyInfo_btn},
        TxShortcut{Qt::Key_K, ui->MyCall_btn},
        TxShortcut{Qt::Key_G, ui->MyGrid_btn},
        TxShortcut{Qt::Key_B, ui->CWBK_btn},
        TxShortcut{Qt::Key_R, ui->CWQSL_btn}}) {
    auto * sc = new QShortcut(QKeyCombination(Qt::ControlModifier, s.key), transmitTab);
    sc->setContext(Qt::WidgetWithChildrenShortcut);
    connect(sc, &QShortcut::activated, s.btn, &QPushButton::click);
  }

  // --- Inject the radio-controls strip at the top of the Settings tab. ---
  // TXRXLock_chk lives in the freq bar, not here.
  QGroupBox * radioCtl = new QGroupBox(tr("Radio Controls"), ui->Settings);
  QHBoxLayout * rcLay = new QHBoxLayout(radioCtl);
  // Mode_box / AFBw_box / AFG_box / RFG_box / groupBox (Band Select) all live
  // on the Waterfall / Periodogram panels now (see setupSpectControls());
  // hide the masters so they don't render on the Settings tab.
  ui->Mode_box->hide();
  ui->AFBw_box->hide();
  ui->AFG_box->hide();
  ui->RFG_box->hide();
  ui->groupBox->hide();               // "Band Select"
  // Record_chk / RecordRF_chk are relocated to the Waterfall/Periodogram
  // bottom strip (see setupSpectControls()); hide the masters here.
  ui->Record_chk->hide();
  ui->RecordRF_chk->hide();
  rcLay->addWidget(ui->groupBox_16);  // "RX Ant." group
  rcLay->addWidget(ui->groupBox_17);  // "TX Ant." group
  rcLay->addStretch(1);               // push antennas to the left
  // Cap the RX/TX antenna selectors: at least 1.5× the "TX Ant." title width,
  // and expand to fit the widest entry.  Keep the wrapping group boxes from
  // stretching horizontally in the Radio Controls strip.
  {
    QFontMetrics fm(ui->TXAnt_sel->font());
    int minW = fm.horizontalAdvance(tr("TX Ant.")) * 3 / 2;
    for (QComboBox * cb : { ui->RXAnt_sel, ui->TXAnt_sel }) {
      cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
      cb->setMinimumWidth(minW);
      cb->setSizePolicy(QSizePolicy::Preferred, cb->sizePolicy().verticalPolicy());
    }
    for (QGroupBox * gb : { ui->groupBox_16, ui->groupBox_17 }) {
      QSizePolicy sp = gb->sizePolicy();
      sp.setHorizontalPolicy(QSizePolicy::Fixed);
      gb->setSizePolicy(sp);
    }
  }

  // --- Restructure the Settings panel body. --------------------------------
  // Delete the old layout tree.  All QGroupBox widgets survive (parented to
  // ui->Settings); spacers and nested layouts are freed.
  delete ui->Settings->layout();

  // "Use GPS Grid" belongs in the Status panel, not here.
  ui->useGPS_ck->hide();

  // Pull recDir_btn and aboutSoDa_btn out of their boxes; they become
  // standalone items in col 3.  Also remove any orphaned spacers.
  {
    auto cleanSpacer = [](QLayout* lay) {
      for (int i = lay->count() - 1; i >= 0; --i) {
        QLayoutItem* it = lay->itemAt(i);
        if (it && it->spacerItem()) { delete lay->takeAt(i); break; }
      }
    };
    ui->groupBox_4->layout()->removeWidget(ui->recDir_btn);
    ui->groupBox_4->layout()->removeWidget(ui->openConfig_btn);
    ui->groupBox_4->layout()->removeWidget(ui->saveConfig_btn);
    ui->groupBox_4->layout()->removeWidget(ui->saveConfigAs_btn);
    cleanSpacer(ui->groupBox_4->layout());   // remove leftover spacer in Config
    ui->groupBox_4->hide();
    ui->groupBox_5->layout()->removeWidget(ui->help_btn);
    ui->groupBox_5->layout()->removeWidget(ui->aboutSoDa_btn);
    ui->groupBox_5->layout()->removeWidget(ui->openLog_btn);
    ui->groupBox_5->layout()->removeWidget(ui->writeLogReport_btn);
    cleanSpacer(ui->groupBox_5->layout());   // remove leftover spacer in Log
    ui->groupBox_5->hide();
  }

  // Col 2: QSO Settings stacked above Reference Oscillator.
  QWidget * col2 = new QWidget(ui->Settings);
  {
    QVBoxLayout * l = new QVBoxLayout(col2);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(4);
    l->addWidget(ui->groupBox_2);   // QSO Settings
    l->addWidget(ui->groupBox_6);   // Reference Oscillator
    l->addStretch(1);
  }

  // Move the log report buttons to the bottom of the Edit Log tab.
  if (auto * elLay = qobject_cast<QVBoxLayout*>(ui->EditLog->layout())) {
    QHBoxLayout * logBtnRow = new QHBoxLayout();
    logBtnRow->setContentsMargins(4, 2, 4, 4);
    logBtnRow->addWidget(ui->openLog_btn);
    logBtnRow->addWidget(ui->writeLogReport_btn);
    logBtnRow->addStretch(1);
    elLay->addLayout(logBtnRow);
  }

  // Col 3: Configuration buttons + Record Directory.  Buttons live directly
  // in the column layout so their left edges align (no QGroupBox border/pad).
  QWidget * col3 = new QWidget(ui->Settings);
  {
    QVBoxLayout * l = new QVBoxLayout(col3);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(4);
    l->addWidget(new QLabel(tr("Configuration"), col3), 0, Qt::AlignLeft);
    l->addWidget(ui->openConfig_btn,   0, Qt::AlignLeft);
    l->addWidget(ui->saveConfig_btn,   0, Qt::AlignLeft);
    l->addWidget(ui->saveConfigAs_btn, 0, Qt::AlignLeft);
    l->addWidget(ui->recDir_btn,       0, Qt::AlignLeft);
    l->addStretch(1);
  }

  // Flush all QPushButtons in VBoxLayouts under Settings to the left so they
  // don't stretch across the full column width.
  std::function<void(QWidget*)> leftAlignBtns = [&](QWidget* root) {
    if (auto* vl = qobject_cast<QVBoxLayout*>(root->layout())) {
      for (int i = 0; i < vl->count(); ++i) {
        QLayoutItem* it = vl->itemAt(i);
        if (!it) continue;
        if (auto* btn = qobject_cast<QPushButton*>(it->widget())) {
          btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
          vl->setAlignment(btn, Qt::AlignLeft);
        }
      }
    }
    for (QObject* ch : root->children())
      if (auto* cw = qobject_cast<QWidget*>(ch))
        leftAlignBtns(cw);
  };
  leftAlignBtns(ui->Settings);

  // Pull SquelchBox out of AudioGroupBox so it can sit on the far left of
  // the settings row.
  if (ui->SquelchBox && ui->AudioGroupBox && ui->AudioGroupBox->layout()) {
    ui->AudioGroupBox->layout()->removeWidget(ui->SquelchBox);
    ui->SquelchBox->setParent(ui->Settings);
  }
  // SquelchBox: don't let it stretch vertically to match tall neighboring
  // columns; otherwise its single-item QVBoxLayout centers the slider in the
  // excess height, opening a gap under the title.  Match Sidetone_box's
  // structure by converting the internal layout to a QHBoxLayout too.
  if (ui->SquelchBox && ui->SquelchBox->layout()) {
    QLayout * oldLay = ui->SquelchBox->layout();
    QLayoutItem * sliderItem = oldLay->takeAt(0);
    delete oldLay;
    QHBoxLayout * hlay = new QHBoxLayout(ui->SquelchBox);
    if (sliderItem) hlay->addItem(sliderItem);
  }

  // Build the new Settings tab layout.
  QVBoxLayout * newSettingsLay = new QVBoxLayout(ui->Settings);
  newSettingsLay->addWidget(radioCtl);
  QHBoxLayout * settingsRow = new QHBoxLayout();
  settingsRow->setSpacing(8);
  // Col 0: Squelch — wrap in a VBox with a bottom stretch so the group box
  // doesn't get stretched vertically to match taller neighboring columns.
  QWidget * col0 = new QWidget(ui->Settings);
  {
    QVBoxLayout * l = new QVBoxLayout(col0);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(ui->SquelchBox);
    l->addStretch(1);
  }
  settingsRow->addWidget(col0);
  settingsRow->addWidget(ui->groupBox_3);    // Col 1: TX Settings
  settingsRow->addWidget(col2);              // Col 2: QSO Settings + Ref Osc
  settingsRow->addWidget(col3);              // Col 3: Config + RecDir
  settingsRow->addWidget(ui->AudioGroupBox); // Col 4: Audio
  settingsRow->addStretch(1);
  newSettingsLay->addLayout(settingsRow, 1);

  // Bottom-right footer row: Help + About.
  QHBoxLayout * settingsFooter = new QHBoxLayout();
  settingsFooter->addStretch(1);
  settingsFooter->addWidget(ui->help_btn);
  settingsFooter->addWidget(ui->aboutSoDa_btn);
  newSettingsLay->addLayout(settingsFooter);

  // --- Build the shared QSO-info header row. ------------------------------
  qso_info_bar = new QWidget(ui->centralWidget);
  QHBoxLayout * qsoLay = new QHBoxLayout(qso_info_bar);
  qsoLay->setContentsMargins(4, 2, 4, 2);
  qsoLay->addWidget(ui->TG_llab);
  qsoLay->addWidget(ui->ToGrid_le);
  qsoLay->addWidget(ui->TC_llab);
  qsoLay->addWidget(ui->ToCall_le);
  qsoLay->addWidget(ui->FG_llab);
  qsoLay->addWidget(ui->FromGrid_lab);
  qsoLay->addWidget(ui->B_llab);
  qsoLay->addWidget(ui->Bearing_lab);
  qsoLay->addWidget(ui->Rng_llab);
  qsoLay->addWidget(ui->Range_lab);
  qsoLay->addWidget(ui->GPST_lab);   // "UTC:" label
  qsoLay->addWidget(ui->UTC_lab);

  // --- Flatten the frequency row. ------------------------------------------
  // Desired order: [RX->TX] [RX freq] [TX->RX] [TX freq] [Log Contact] [PTT] [TX state]
  // The .ui wraps each FreqLabel in a GroupBox; we don't need those wrappers
  // in the compact touchscreen layout, so pull the children out and hide the boxes.
  delete ui->RXFreq_box->layout();   // widgets stay parented to GroupBox, just detached
  delete ui->TXFreq_box->layout();
  ui->RXFreq_box->hide();
  ui->TXFreq_box->hide();

  // --- Build the shared frequency/PTT control row. -------------------------
  freq_ctrl_bar = new QWidget(ui->centralWidget);
  QHBoxLayout * freqLay = new QHBoxLayout(freq_ctrl_bar);
  freqLay->setContentsMargins(4, 2, 4, 2);
  freqLay->setSpacing(4);

  // Order: [RX freq] [RX->TX] [TX=RX lock] [TX->RX] [TX freq] [Log Contact] [PTT] [TX ON/OFF]
  freqLay->addWidget(ui->RXFreq_lab, 3);

  ui->RXfreq2TXfreq_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  freqLay->addWidget(ui->RXfreq2TXfreq_btn);

  ui->TXRXLock_chk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  freqLay->addWidget(ui->TXRXLock_chk);

  ui->TXfreq2RXfreq_btn->setText(tr("RX←TX"));
  ui->TXfreq2RXfreq_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  freqLay->addWidget(ui->TXfreq2RXfreq_btn);

  freqLay->addWidget(ui->TXFreq_lab, 3);

  ui->LogContact_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  ui->LogContact_btn->setText(tr("Log Contact (^L)"));
  freqLay->addWidget(ui->LogContact_btn);

  freqLay->addWidget(ui->PTT_btn);

  ui->TXState_lab->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  freqLay->addWidget(ui->TXState_lab);

  // --- Rebuild the central widget's outer layout. -------------------------
  // The original verticalLayout_29 still owns Display_tabs plus the leftover
  // empty Controls_box / MidCtrls_lay / LogGPS_lay sub-layouts (their widgets
  // are now reparented).  Discard it cleanly and install our own.
  QLayout * oldCentralLay = ui->centralWidget->layout();
  if (oldCentralLay) {
    // Make sure Display_tabs is not destroyed when we drop the layout.
    ui->Display_tabs->setParent(ui->centralWidget);
  }
  delete oldCentralLay;

  QVBoxLayout * centralLay = new QVBoxLayout(ui->centralWidget);
  centralLay->setContentsMargins(2, 2, 2, 2);
  centralLay->setSpacing(2);
  centralLay->addWidget(freq_ctrl_bar);
  centralLay->addWidget(ui->Display_tabs, 1);
  centralLay->addWidget(qso_info_bar);

  // --- Rewrite the tab order to match the ShortSoDa spec. -----------------
  // The .ui created tabs in this order:
  //   0 Waterfall, 1 Periodogram, 2 Settings, 3 BandConfig, 4 EditLog, 5 Status
  // We insert Transmit at index 2 and rename the bottom tabs.
  ui->Display_tabs->insertTab(2, transmitTab, tr("Transmit"));

  // Find indices by widget pointer so renames or reordering can't drift.
  tab_idx_waterfall   = ui->Display_tabs->indexOf(ui->Waterfall);
  tab_idx_periodogram = ui->Display_tabs->indexOf(ui->Periodogram);
  tab_idx_transmit    = ui->Display_tabs->indexOf(transmitTab);
  tab_idx_settings    = ui->Display_tabs->indexOf(ui->Settings);
  tab_idx_bandconfig  = ui->Display_tabs->indexOf(ui->BandConfig);
  tab_idx_editlog     = ui->Display_tabs->indexOf(ui->EditLog);
  tab_idx_status      = ui->Display_tabs->indexOf(ui->Status);

  if (tab_idx_waterfall   >= 0) ui->Display_tabs->setTabText(tab_idx_waterfall,   tr("Waterfall"));
  if (tab_idx_periodogram >= 0) ui->Display_tabs->setTabText(tab_idx_periodogram, tr("Periodogram"));
  if (tab_idx_settings    >= 0) ui->Display_tabs->setTabText(tab_idx_settings,    tr("Settings"));
  if (tab_idx_bandconfig  >= 0) ui->Display_tabs->setTabText(tab_idx_bandconfig,  tr("Config Band"));
  if (tab_idx_editlog     >= 0) ui->Display_tabs->setTabText(tab_idx_editlog,     tr("Edit Log"));
  if (tab_idx_status      >= 0) ui->Display_tabs->setTabText(tab_idx_status,      tr("Status"));

  ui->Display_tabs->setCurrentIndex(tab_idx_waterfall);
  ui->Display_tabs->tabBar()->hide();

  // Grid, Lat, Lon, Bearing, and Comment removed from the Status panel per
  // ShortSoDa spec.  Hide the orphaned label widgets so they don't float in
  // the central widget.
  for (QWidget * w : {
        (QWidget*)ui->GPSg_lab, (QWidget*)ui->GRID_lab,
        (QWidget*)ui->GPSlat_lab, (QWidget*)ui->LAT_lab,
        (QWidget*)ui->GPSlon_lab, (QWidget*)ui->LON_lab,
        (QWidget*)ui->RevBearing_lab,
        (QWidget*)ui->label_22, (QWidget*)ui->LogComment_txt }) {
    if (w) w->hide();
  }

  // Keep buffer/slack indicators on the Status tab.
  if (ui->Status && ui->Status->layout()) {
    QHBoxLayout * statusRow = new QHBoxLayout();
    statusRow->addWidget(ui->buflab);
    statusRow->addWidget(ui->slack_lab);
    statusRow->addStretch(1);
    static_cast<QBoxLayout*>(ui->Status->layout())->addLayout(statusRow);
  }

  // PTT button: MinimumExpanding from the .ui → Fixed so it doesn't fill the
  // row; keep a generous minimum width for touchscreen tapping.
  ui->PTT_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  ui->PTT_btn->setMinimumWidth(96);
  ui->PTT_btn->setText(tr("PTT (^T)"));

  // Hide widgets that were containers in the old toolbar layout and are
  // now empty after we moved their children into the new tabs.  Deleting
  // the parent layout doesn't reparent these orphans, so they would
  // otherwise hover at (0,0) in the central widget.
  QWidget * orphans[] = {
    ui->Exch_splt, ui->CWCG_splt, ui->CWAuto_splt, ui->CarClr_splt,
    ui->R_llab
  };
  for (QWidget * w : orphans) { if (w) w->hide(); }

  // Catch right-click anywhere in the window for the panel chooser.
  ui->centralWidget->installEventFilter(this);
  ui->Display_tabs->installEventFilter(this);
  // Application-wide filter so ordinary typing on the Transmit tab lands in
  // CWTXBox regardless of which widget currently has focus.
  qApp->installEventFilter(this);

  // Waterfall and Periodogram: the .ui puts a vertical spacer ABOVE
  // "Center RX Freq" so the controls float to the bottom of the column.
  // Move the spacer to the END so controls sit at the top.
  // Path: tab(VBox)[0] → HBox[0] → control VBox[0] = spacer.
  for (QWidget * tab : { (QWidget*)ui->Waterfall, (QWidget*)ui->Periodogram }) {
    if (!tab) continue;
    auto* tabVBox = qobject_cast<QBoxLayout*>(tab->layout());
    if (!tabVBox) continue;
    auto* hlay = qobject_cast<QBoxLayout*>(tabVBox->itemAt(0)->layout());
    if (!hlay) continue;
    auto* ctrlCol = qobject_cast<QBoxLayout*>(hlay->itemAt(0)->layout());
    if (!ctrlCol) continue;
    QLayoutItem* top = ctrlCol->itemAt(0);
    if (top && top->spacerItem()) {
      ctrlCol->removeItem(top);
      delete top;
      ctrlCol->addStretch(1);
    }
  }

  // Keep UTC_lab current from the system clock.  GPS updates via updateTime()
  // write the same format, so either source can win without conflict.
  connect(&one_second_timer, &QTimer::timeout, this, [this]() {
    QTime t = QDateTime::currentDateTimeUtc().time();
    QChar z('0');
    ui->UTC_lab->setText(
      QString("%1:%2:%3").arg(t.hour(), 2, 10, z)
                         .arg(t.minute(), 2, 10, z)
                         .arg(t.second(), 2, 10, z));
  });

  // Application-wide Alt+key shortcuts.  We register two dispatch paths:
  //   1. QShortcut with ApplicationShortcut context for the normal case.
  //   2. An entry in alt_actions_ so eventFilter() can fire the action ahead
  //      of any widget that would swallow the key via ShortcutOverride
  //      (QLineEdit does this for several Alt+letter combinations).
  auto mkAlt = [this](Qt::Key k, std::function<void()> fn) {
    auto* sc = new QShortcut(QKeyCombination(Qt::AltModifier, k), this);
    sc->setContext(Qt::ApplicationShortcut);
    connect(sc, &QShortcut::activated, this, fn);
    alt_actions_.insert(static_cast<int>(k), fn);
  };

  mkAlt(Qt::Key_T, [this]() {
    if (tab_idx_transmit >= 0) ui->Display_tabs->setCurrentIndex(tab_idx_transmit);
  });
  mkAlt(Qt::Key_W, [this]() {
    if (tab_idx_waterfall >= 0) ui->Display_tabs->setCurrentIndex(tab_idx_waterfall);
  });
  mkAlt(Qt::Key_P, [this]() {
    if (tab_idx_periodogram >= 0) ui->Display_tabs->setCurrentIndex(tab_idx_periodogram);
  });
  mkAlt(Qt::Key_S, [this]() {
    if (tab_idx_settings >= 0) ui->Display_tabs->setCurrentIndex(tab_idx_settings);
  });
  mkAlt(Qt::Key_L, [this]() {
    if (tab_idx_editlog >= 0) ui->Display_tabs->setCurrentIndex(tab_idx_editlog);
  });
  mkAlt(Qt::Key_C, [this]() { showPanelChooser(); });
  mkAlt(Qt::Key_V, [this]() { vu_meter->setVisible(!vu_meter->isVisible()); });
  mkAlt(Qt::Key_H, [this]() {
    QMessageBox::information(this, tr("Keyboard Shortcuts"),
      tr("<b>Alt+T</b> — Transmit panel<br>"
         "<b>Alt+W</b> — Waterfall panel<br>"
         "<b>Alt+P</b> — Periodogram panel<br>"
         "<b>Alt+S</b> — Settings panel<br>"
         "<b>Alt+L</b> — Log panel<br>"
         "<b>Alt+C</b> — Panel chooser (also middle-click)<br>"
         "<b>Alt+V</b> — VU meter (toggle)<br>"
         "<b>Alt+H</b> — This help"));
  });
}

// True if this radio reports any TX antenna (i.e. is not RX-only).
// PTT_btn is force-disabled at construction and re-enabled by
// setTXEnabled(true) when addTXAntName fires.
bool MainWindow::isTXCapable() const
{
  return ui->PTT_btn && ui->PTT_btn->isEnabled();
}

void MainWindow::selectTabByName(const QString & name)
{
  int idx = -1;
  if      (name == "waterfall")   idx = tab_idx_waterfall;
  else if (name == "periodogram") idx = tab_idx_periodogram;
  else if (name == "transmit")    idx = tab_idx_transmit;
  else if (name == "settings")    idx = tab_idx_settings;
  else if (name == "bandconfig")  idx = tab_idx_bandconfig;
  else if (name == "editlog")     idx = tab_idx_editlog;
  else if (name == "status")      idx = tab_idx_status;
  if (idx >= 0) ui->Display_tabs->setCurrentIndex(idx);
}

// Pop a radio-button-style chooser at the cursor for quick tab switching.
// Bound to right-click (MB3) and Alt-D.
void MainWindow::showPanelChooser()
{
  QMenu menu(this);
  QActionGroup * grp = new QActionGroup(&menu);
  grp->setExclusive(true);

  struct Item { const char * label; int idx; };
  QList<Item> items = {
    {"Waterfall",   tab_idx_waterfall},
    {"Periodogram", tab_idx_periodogram},
    {"Transmit",    tab_idx_transmit},
    {"Settings",    tab_idx_settings},
    {"Config Band", tab_idx_bandconfig},
    {"Edit Log",    tab_idx_editlog},
    {"Status",      tab_idx_status},
  };

  int cur = ui->Display_tabs->currentIndex();
  for (const Item & it : items) {
    if (it.idx < 0) continue;
    if (it.idx == tab_idx_transmit && !isTXCapable()) continue;
    QAction * a = menu.addAction(tr(it.label));
    a->setCheckable(true);
    a->setChecked(it.idx == cur);
    grp->addAction(a);
    int target = it.idx;
    connect(a, &QAction::triggered,
	    [this, target]() { ui->Display_tabs->setCurrentIndex(target); });
  }
  menu.exec(QCursor::pos());
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
  // Alt+key actions are handled by ApplicationShortcut QShortcuts registered
  // in reorganizeForShortSoDa(); they fire on any panel regardless of focus.
  QMainWindow::keyPressEvent(event);
}

bool MainWindow::eventFilter(QObject * obj, QEvent * event)
{
  // Middle-click on the central widget / tab widget opens the panel chooser.
  if (event->type() == QEvent::MouseButtonPress &&
      (obj == ui->centralWidget || obj == ui->Display_tabs)) {
    QMouseEvent * me = static_cast<QMouseEvent *>(event);
    if (me->button() == Qt::MiddleButton) {
      showPanelChooser();
      return true;
    }
  }

  // Global Ctrl+key shortcuts.  Dispatched here (not via QShortcut) because
  // QLineEdit accepts ShortcutOverride for several readline-style Ctrl+letter
  // combos when a text-entry line has focus.
  //   Ctrl+T  — toggle PTT       (any panel)
  //   Ctrl+L  — Log Contact      (any panel)
  //   Ctrl+E  — Exchange macro   (Transmit tab only)
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent * ke = static_cast<QKeyEvent *>(event);
    if (ke->modifiers() == Qt::ControlModifier) {
      switch (ke->key()) {
        case Qt::Key_T:
          if (ui->PTT_btn && ui->PTT_btn->isEnabled()) {
            ui->PTT_btn->toggle();
            return true;
          }
          break;
        case Qt::Key_L:
          if (ui->LogContact_btn && ui->LogContact_btn->isEnabled()) {
            ui->LogContact_btn->click();
            return true;
          }
          break;
        case Qt::Key_E:
          if (tab_idx_transmit >= 0 &&
              ui->Display_tabs->currentIndex() == tab_idx_transmit &&
              ui->Exchange_btn && ui->Exchange_btn->isEnabled()) {
            ui->Exchange_btn->click();
            return true;
          }
          break;
        default: break;
      }
    }
  }

  // Alt+key global actions.  Dispatch here so QLineEdit / other widgets can't
  // swallow them via ShortcutOverride when the CW entry line has focus.
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent * ke = static_cast<QKeyEvent *>(event);
    if (ke->modifiers() == Qt::AltModifier) {
      auto it = alt_actions_.constFind(ke->key());
      if (it != alt_actions_.constEnd()) {
        it.value()();
        return true;
      }
    }
  }

  // Transmit-tab typing redirect: ordinary character keypresses land in
  // the CW text-entry line — but only when focus isn't already on some
  // other text-input widget (To Call / To Grid in the QSO info bar, etc.).
  // Ctrl/Alt/Meta modified keys and keys with no text (Esc, arrows, function
  // keys, etc.) fall through to their normal handlers.  A re-entry guard
  // stops sendEvent from re-triggering this branch via qApp's filter chain.
  static thread_local bool tx_redirect_active = false;
  if (!tx_redirect_active &&
      event->type() == QEvent::KeyPress &&
      tab_idx_transmit >= 0 &&
      ui->Display_tabs->currentIndex() == tab_idx_transmit &&
      ui->CWCurLine_le) {
    QWidget * fw = QApplication::focusWidget();
    bool text_focused = fw && (
      qobject_cast<QLineEdit*>(fw) ||
      qobject_cast<QTextEdit*>(fw) ||
      qobject_cast<QPlainTextEdit*>(fw));
    if (!text_focused) {
      QKeyEvent * ke = static_cast<QKeyEvent *>(event);
      const auto mods = ke->modifiers();
      if (!(mods & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))
          && !ke->text().isEmpty()) {
        tx_redirect_active = true;
        ui->CWCurLine_le->setFocus();
        QCoreApplication::sendEvent(ui->CWCurLine_le, event);
        tx_redirect_active = false;
        return true;
      }
    }
  }

  return QMainWindow::eventFilter(obj, event);
}
