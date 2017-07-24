#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>
#include "soda_comboboxes.h"
#include "soda_listener.h"

void MainWindow::setupTopControls()
{
  // setup the top row of controls under the tabbed pages.
  // these are the basic radio settings... gain, frequency and such
  // we connect the report side and then the command side. 
  connect(listener, SIGNAL(addModulation(QString, int)), 
	  ui->Mode_cb, SLOT(addValue(QString, int)));
  connect(ui->Mode_cb, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setModulation(int)));

  connect(listener, SIGNAL(addFilterName(QString, int)), 
	  ui->AFBw_cb, SLOT(addValue(QString, int)));
  connect(ui->AFBw_cb, SIGNAL(valueChanged(int)),
	  listener, SLOT(setAFFilter(int)));

  connect(ui->RFGain_slide, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setRXGain(int)));
  connect(ui->AFGain_slide, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setAFGain(int)));

  connect(ui->RXFreq_lab, SIGNAL(newFreq(double)), 
	  listener, SLOT(setRXFreq(double))); 

  connect(ui->TXFreq_lab, SIGNAL(newFreq(double)), 
	  listener, SLOT(setTXFreq(double))); 

  connect(ui->RXFreq_lab, &FreqLabel::newFreq, 
	  this, [=] (double freq) { 
	    if(ui->TXRXLock_chk->isChecked()) 
	      ui->TXFreq_lab->setFreq(freq); 
	  });

  connect(ui->TXFreq_lab, &FreqLabel::newFreq, 
	  this, [=] (double freq) { 
	    if(ui->TXRXLock_chk->isChecked()) 
	      ui->RXFreq_lab->setFreq(freq); 
	  });

  // connect RX freq setting to waterfall and spectrum if "spectrum track" is checked

  // connect rx->cfreq from rx freq to waterfall/spectrum

  // connect waterfall/spectrum freq setting to rxfreq 
  
  connect(ui->RXfreq2TXfreq_btn, &QPushButton::clicked, 
	  this, [=] (bool checked) {
	    ui->TXFreq_lab->setFreq(ui->RXFreq_lab->getFreq());
	  });
  connect(ui->TXfreq2RXfreq_btn, &QPushButton::clicked, 
	  this, [=] (bool checked) {
	    ui->RXFreq_lab->setFreq(ui->TXFreq_lab->getFreq());
	  });
}
