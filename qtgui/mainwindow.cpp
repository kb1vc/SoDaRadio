#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>

#include <QString>
#include <QAudioDeviceInfo>

#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"
#include "../common/GuiParams.hxx"

MainWindow::MainWindow(QWidget *parent, SoDa::GuiParams & params) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // setup the listener. 
  listener = new SoDaListener(this, QString::fromStdString(params.getServerSocketBasename())); 

  setupSpectrum();
  setupWaterFall();
    
  setupTopControls();
  setupMidControls();
  setupLogGPS();

  setupSettings();
  setupBandConfig();
  setupLogEditor();


  connect(ui->LogView, &LogTable::entryUpdated,
	  [](int row, const QString & key, const QString & val) {
	    std::cerr << boost::format("row = %d key = [%s] val = [%s]\n") % row % key.toStdString() % val.toStdString();
	  });

  connect(listener, SIGNAL(repHWMBVersion(const QString &)), 
	  this, SLOT(setWindowTitle(const QString &)));

  connect(listener, SIGNAL(initSetupComplete()), 
	  this, SLOT(restoreSettings()));
  listener->start();

  settings_p = new QSettings("kb1vc.org", "SoDaRadioQT", this);

  current_band_selector = ui->bandSel_cb->currentText(); 

  hlib_server = new SoDaHamlibServer(this, 4575);
  
  hlib_server->start();

  setupHamlib();  
}

MainWindow::~MainWindow()
{
  settings_p->beginGroup("Radio");
  bandMapSaveRestore(band_map, true);    
  widgetSaveRestore(this, "SoDaRadioQT.", true);
  settings_p->endGroup();
  delete ui;
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
    else if(my_class == "FreqLabel") {
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
    else if(my_class == "SoDaWFall") {
      if(SoDaWFall * cb = qobject_cast<SoDaWFall*>(*cp)) {      
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
    else if(my_class == "SoDaSpect") {
      if(SoDaSpect * cb = qobject_cast<SoDaSpect*>(*cp)) {      
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
    else if(my_class == "IntValComboBox") {
      if(IntValComboBox * cb = qobject_cast<IntValComboBox*>(*cp)) {      
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
    else if((my_class == "WFallSpanComboBox") || 
	    (my_class == "WFallDynRangeComboBox")) {
      if(ValComboBox * cb = qobject_cast<ValComboBox*>(*cp)) {      
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
    else if((my_class == "QLabel") ||
	    (my_class == "QVBoxLayout") ||
	    (my_class == "QHBoxLayout") ||
	    (my_class == "QPushButton") ||
	    (my_class == "QWidgetTextControl") ||
	    (my_class == "QTextDocument") ||
	    (my_class == "QTextDocumentLayout") ||
	    (my_class == "QTextImageHandler") ||
	    
	    (my_class == "QWidgetLineControl") ||
	    (my_class == "QAction") ||
	    (my_class == "QGroupBox") ||	    
	    (my_class == "QLocalSocket") ||
	    (my_class == "QTcpSocket") ||
	    (my_class == "QDoubleValidator") ||
	    (my_class == "QStandardItemModel") ||
	    (my_class == "QDoubleValidator") ||
	    (my_class == "QSocketNotifier") ||
	    (my_class == "QNativeSocketEngine")) {
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

