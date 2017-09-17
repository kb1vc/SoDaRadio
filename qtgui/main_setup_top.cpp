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

void MainWindow::updateBandDisplay(double freq)
{
  // change the band setting, if we need to.
  // but don't modify any other settings. 
  QString band_name = band_map.findBand(freq);
  if(band_name != "") {
    int idx; 
    // make sure that we don't modify the MODE and such.
    auto_bandswitch_target = band_name; 
    if((idx = ui->bandSel_cb->findText(band_name)) >= 0) {
      // remember the mode 
      int cur_mode = ui->Mode_cb->value();

      ui->bandSel_cb->setCurrentIndex(idx);

      // undo the mode switching...
      ui->Mode_cb->setValue(cur_mode);
      
      listener->setSpectrumCenter(freq);      
    }
  }
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


