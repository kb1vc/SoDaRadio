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

// methods and such for the bandconfig panel.
// This includes saving the band config list to the
// settings object, and restoring the band config list
// from the settings object. 
void MainWindow::setupBandConfig()
{
  connect(ui->bandSel_cb, SIGNAL(currentTextChanged(const QString &)), 
	  this, SLOT(changeBand(const QString &)));

  connect(ui->BCOK_btn, SIGNAL(clicked(bool)), 
	  this, SLOT(writeBandMapEntry(bool)));
  connect(ui->BCCancel_btn, &QPushButton::clicked, 
	  [=](bool v){ fillBandMapEntry(ui->BCBandSel_cb->currentText()); });
  connect(ui->BCBandSel_cb, SIGNAL(currentTextChanged(const QString &)),
	  this, SLOT(fillBandMapEntry(const QString &)));

  // setup validators.
  ui->BCMinFreq_le->setValidator(new QDoubleValidator(this));
  ui->BCMaxFreq_le->setValidator(new QDoubleValidator(this));
  ui->BCLOFreq_le->setValidator(new QDoubleValidator(this));

  connect(listener, &GUISoDa::Listener::addRXAntName, 
	  [this](const QString & v){
	    ui->BCRXAnt_cb->addItem(v); });
  connect(listener, &GUISoDa::Listener::addTXAntName, 
	  [this](const QString & v){
	    ui->BCTXAnt_cb->addItem(v); });

  connect(listener, SIGNAL(addModulation(QString, int)), 
	  ui->BCDefMode_cb, SLOT(addValue(QString, int)));
  
  
}

void MainWindow::saveCurrentFreqs()
{
  if(band_map.count(current_band_selector) > 0) {
    double rxfreq = ui->RXFreq_lab->getFreq() * 1.0e-6;
    double txfreq = ui->TXFreq_lab->getFreq() * 1.0e-6;
    GUISoDa::Band * bp = &band_map[current_band_selector]; 
    if((rxfreq >= bp->minFreq()) && (rxfreq <= bp->maxFreq())) {
      band_map[current_band_selector].setLastRXFreq(1e-6 * ui->RXFreq_lab->getFreq());
    }
    if((txfreq >= bp->minFreq()) && (txfreq <= bp->maxFreq())) {    
      band_map[current_band_selector].setLastTXFreq(1e-6 * ui->TXFreq_lab->getFreq());
    }
  }
}

void MainWindow::bandMapSaveRestore(GUISoDa::BandMap & bmap, bool save)
{
  if(save) {
    saveCurrentFreqs();
    bmap.saveBands(settings_p);
  }
  else {
    bmap.restoreBands(settings_p);
    // now load the comboboxes
    GUISoDa::BandMap::BandMapIterator bmi(bmap); 
    // clear the two band selectors.
    ui->BCBandSel_cb->clear();
    ui->bandSel_cb->clear();
    while(bmi.hasNext()) {
      bmi.next();            
      ui->BCBandSel_cb->addItem(bmi.key());
      ui->bandSel_cb->addItem(bmi.key());
    }
    // add a new band at the end of the band config selector
    ui->BCBandSel_cb->addItem("Create Band");
  }
}

// called when we change the band selector in the band setup box.
// or hit cancel. 
void MainWindow::fillBandMapEntry(const QString & band)
{
  ui->BCStatus_lbl->setText("");
  if(band_map.count(band)) {
    GUISoDa::Band b = band_map[band]; 
    ui->BCBandName_le->setText("");

    ui->BCBandSel_cb->setCurrentText(band); 

    ui->BCIndex_sb->setValue(b.index());;
    ui->BCRXAnt_cb->setCurrentText(b.defRXAnt());
    ui->BCTXAnt_cb->setCurrentText(b.defTXAnt());

    ui->BCMinFreq_le->setText(QString("%1").arg(b.minFreq(), 14, 'f', 6));
    ui->BCMaxFreq_le->setText(QString("%1").arg(b.maxFreq(), 14, 'f', 6));    

    ui->BCDefMode_cb->setCurrentText(b.defMode());
    
    ui->BCLOFreq_le->setText(QString("%1").arg(b.tvLOFreq(), 14, 'f', 6));
    ui->BCLOMult_sb->setValue((int) b.tvLOMult());

    ui->BCEnableTX_cb->setChecked(b.txEna());
    ui->BCTransMode_cb->setChecked(b.tverterEna());
    ui->BCLowInj_cb->setChecked(b.tverterLowInjection());    
  }
}

// called when we hit the OK button in the band setup box.
void MainWindow::writeBandMapEntry(bool v)
{
  GUISoDa::Band b; 
  (void) v;

  ui->BCStatus_lbl->setText("");
  QString bname = ui->BCBandName_le->text();
  if(bname == "") {
    bname = ui->BCBandSel_cb->currentText();
  }
  
  if(bname == "Create Band") {
    // there's a problem here... we need a band name.
    // launch an error message
    ui->BCStatus_lbl->setText("<font color='red'>Please supply a band name to create a new band.</font>");
    return; 
  }

  // if we get to here, it is time to create a new band or modify an old one
  b.setName(bname); 
  b.setIndex(ui->BCIndex_sb->value());
  b.setDefRXAnt(ui->BCRXAnt_cb->currentText());
  b.setDefTXAnt(ui->BCTXAnt_cb->currentText());

  b.setDefMode(ui->BCDefMode_cb->currentText());
  
  double min_freq = ui->BCMinFreq_le->text().toDouble();
  double max_freq = ui->BCMaxFreq_le->text().toDouble();  
  b.setMinFreq(min_freq);
  b.setMaxFreq(max_freq);  

  b.setTvLOFreq(ui->BCLOFreq_le->text().toDouble());
  b.setTvLOMult(ui->BCLOMult_sb->value());
  b.setTxEna(ui->BCEnableTX_cb->isChecked());
  b.setTverterEna(ui->BCTransMode_cb->isChecked());
  b.setTverterLowInjection(ui->BCLowInj_cb->isChecked());
  
  b.setLastRXFreq((min_freq + max_freq) * 0.5);
  b.setLastTXFreq((min_freq + max_freq) * 0.5);    

  band_map[bname] = b; 

  // now add the name to the rolling list.
  if(ui->BCBandSel_cb->findText(bname) < 0) {
    ui->BCBandSel_cb->addItem(bname);
  }
  if(ui->bandSel_cb->findText(bname) < 0) {
    ui->bandSel_cb->addItem(bname);
  }
}

// called when the selection changes in the band selector
void MainWindow::changeBand(const QString & band)
{
  // first save the current frequency in the current band, if and only
  // if it is "in range" and if the band exists. 
  if(band != current_band_selector) saveCurrentFreqs();

  // now find the new band.
  if(band_map.count(band)) {
    // and set the UI widgets.
    if((band != auto_bandswitch_target)) {
      // but only change these if this was the result of user band input.
      if (auto_bandswitch_target != "") {
        ui->Mode_cb->setValue(band_map[band].defMode());
      }
      double rx_freq = band_map[band].lastRXFreq() * 1e6;
      setRXFreq(rx_freq);
      setTXFreq(band_map[band].lastTXFreq() * 1e6);
      listener->setSpectrumCenter(rx_freq);      
    }
    else {
      auto_bandswitch_target = ""; 
    }
    
    ui->RXAnt_sel->setCurrentText(band_map[band].defRXAnt());
    ui->TXAnt_sel->setCurrentText(band_map[band].defTXAnt());    
    
    current_band_selector = band; 
  }
}
