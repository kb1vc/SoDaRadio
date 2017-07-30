#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>
#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"

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
	  this, SLOT(setRXFreq(double))); 

  connect(ui->TXFreq_lab, SIGNAL(newFreq(double)), 
	  this, SLOT(setTXFreq(double))); 
  
  connect(ui->RXfreq2TXfreq_btn, &QPushButton::clicked, 
	  this, [=] (bool checked) {
	    setTXFreq(ui->RXFreq_lab->getFreq());
	  });
  connect(ui->TXfreq2RXfreq_btn, &QPushButton::clicked, 
	  this, [=] (bool checked) {
	    setRXFreq(ui->TXFreq_lab->getFreq());
	  });
}


void MainWindow::setRXFreq(double freq)
{
  // coordinate all settings for new frequencies. 
  setRXFreq_nocross(freq); 
  // if we are TX/RX frequency locked, tell the TX unit
  if(ui->TXRXLock_chk->isChecked()) setTXFreq_nocross(freq); 
}

void MainWindow::setTXFreq(double freq)
{
  setTXFreq_nocross(freq); 
  if(ui->TXRXLock_chk->isChecked()) setRXFreq_nocross(freq); 
}

void MainWindow::setRXFreq_nocross(double freq)
{
  // tell the radio. 
  listener->setRXFreq(freq); 
  
  // tell the waterfall
  ui->waterfall_plt->setFreqMarker(freq); 

  // tell the spectrum plot
  ui->spectrum_plt->setFreqMarker(freq);
  
  // tell the RX freq display
  ui->RXFreq_lab->setFreq(freq); 
}

void MainWindow::setTXFreq_nocross(double freq)
{
  // tell the radio. 
  listener->setTXFreq(freq); 
  
  // tell the RX freq display
  ui->TXFreq_lab->setFreq(freq); 
}


