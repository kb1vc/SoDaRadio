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

#include <QString>
#include <iostream>
#include <boost/format.hpp>

#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"

void MainWindow::setupWaterFall()
{
    connect(ui->waterfall_plt,SIGNAL(xClick(double)), this, SLOT(setRXFreq(double)));
    connect(ui->waterfall_plt, &GUISoDa::WFall::xClick, listener, &GUISoDa::Listener::setRXFreq);
    connect(ui->wf_moveRight_btn, SIGNAL(clicked(bool)), 
	    ui->waterfall_plt, SLOT(scrollRight(bool)));
    connect(ui->wf_moveLeft_btn, SIGNAL(clicked(bool)), 
	    ui->waterfall_plt, SLOT(scrollLeft(bool)));
    connect(ui->wf_dynRange_cb, SIGNAL(valueChanged(double)), 
	    ui->waterfall_plt, SLOT(setDynamicRange(double)));
    connect(ui->wf_floorLevel_spb, SIGNAL(valueChanged(int)), 
	    ui->waterfall_plt, SLOT(setRefLevel(int)));
    connect(ui->wf_freqSpan_cb, SIGNAL(valueChanged(double)), 
	    ui->waterfall_plt, SLOT(setFreqSpanKHz(double)));

    ui->waterfall_plt->setMarkerOffset(-100, 1e3);

    ui->wf_dynRange_cb->setCurrentIndex(0);
    ui->wf_freqSpan_cb->setCurrentIndex(0);

    connect(listener, SIGNAL(configureSpectrum(double, double, long)),
	    ui->waterfall_plt, SLOT(configureSpectrum(double, double, long)));
    connect(listener, SIGNAL(updateData(double, float*)), 
	    ui->waterfall_plt, SLOT(updateData(double, float*)));

    connect(ui->wf_RX2Center_btn, &QPushButton::clicked, 
	    [this](bool v) { 
	      (void) v;
	      listener->setSpectrumCenter(listener->getRXFreq()); });

    connect(ui->wf_updateRate_sl, SIGNAL(valueChanged(int)), 
	    listener, SLOT(setSpectrumUpdateRate(int))); 
    connect(ui->wf_avgWindow_sl, SIGNAL(valueChanged(int)), 
	    listener, SLOT(setSpectrumAvgWindow(int))); 

    connect(listener, SIGNAL(repMarkerOffset(double, double)), 
	    ui->waterfall_plt, SLOT(setMarkerOffset(double, double)));
}

void MainWindow::setupSpectrum()
{
  ui->spectrum_plt->setRefLevel(10);
  ui->spectrum_plt->setDynamicRange(100.0);
  ui->spectrum_plt->setFreqCenter(144.2e6);
  connect(ui->spectrum_plt, SIGNAL(xClick(double)), this, SLOT(setRXFreq(double)));
  connect(ui->sp_moveRight_btn, SIGNAL(clicked(bool)), 
	  ui->spectrum_plt, SLOT(scrollRight(bool)));
  connect(ui->sp_moveLeft_btn, SIGNAL(clicked(bool)), 
	  ui->spectrum_plt, SLOT(scrollLeft(bool)));
  connect(ui->sp_dynRange_cb, SIGNAL(valueChanged(double)), 
	  ui->spectrum_plt, SLOT(setDynamicRange(double)));
  connect(ui->sp_freqSpan_cb, SIGNAL(valueChanged(double)), 
	  ui->spectrum_plt, SLOT(setFreqSpanKHz(double)));
  connect(ui->sp_ceilLevel_spb, SIGNAL(valueChanged(int)), 
	  ui->spectrum_plt, SLOT(setRefLevel(int)));

  ui->spectrum_plt->setMarkerOffset(0,1e3);


  connect(listener, SIGNAL(configureSpectrum(double, double, long)),
	  ui->spectrum_plt, SLOT(configureSpectrum(double, double, long)));
  connect(listener, SIGNAL(updateData(double, float*)), 
	  ui->spectrum_plt, SLOT(updateData(double, float*)));

  connect(ui->sp_RX2Center_btn, &QPushButton::clicked, 
	  [this](bool v) { 
	    (void) v; 
	    listener->setSpectrumCenter(listener->getRXFreq()); });
  connect(ui->sp_avgWindow_sl, SIGNAL(valueChanged(int)), 
	    listener, SLOT(setSpectrumAvgWindow(int))); 

  connect(listener, SIGNAL(repMarkerOffset(double, double)), 
	  ui->spectrum_plt, SLOT(setMarkerOffset(double, double)));
  
}
