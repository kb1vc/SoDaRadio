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

void MainWindow::setupHamlib()
{
  // connect updates or status reports to the hamlib handler 
  // so that it can maintain its own view of the radio state.
  connect(ui->spectrum_plt, SIGNAL(xClick(double)), hlib_server->getHandler(), SLOT(reportRXFreq(double)));    

  connect(ui->waterfall_plt, SIGNAL(xClick(double)), hlib_server->getHandler(), SLOT(reportRXFreq(double)));    
  
  
  connect(ui->RXFreq_lab, SIGNAL(newFreq(double)), 
	  hlib_server->getHandler(), SLOT(reportRXFreq(double))); 

  connect(ui->TXFreq_lab, SIGNAL(newFreq(double)), 
	  hlib_server->getHandler(), SLOT(reportTXFreq(double))); 

  connect(ui->Mode_cb, SIGNAL(valueChanged(int)), 
	  hlib_server->getHandler(), SLOT(reportModulation(int)));

  connect(hlib_server->getHandler(), SIGNAL(setRXFreq(double)),
	  this, SLOT(setRXFreq(double)));

  connect(hlib_server->getHandler(), SIGNAL(setRXFreq(double)),
	  this, SLOT(updateBandDisplay(double)));

  connect(hlib_server->getHandler(), SIGNAL(setTXFreq(double)),
	  this, SLOT(setTXFreq(double)));

  connect(hlib_server->getHandler(), SIGNAL(setRXFreq(double)),
	  this->listener, SLOT(setSpectrumCenter(double)));

  connect(hlib_server->getHandler(), SIGNAL(setTXFreq(double)),
	  this, SLOT(setTXFreq(double)));

  connect(hlib_server->getHandler(), SIGNAL(setTXOn(bool)), 
	  this->listener, SLOT(setPTT(bool)));
	  
}


