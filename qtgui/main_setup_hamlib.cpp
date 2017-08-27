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
  connect(ui->RXFreq_lab, SIGNAL(newFreq(double)), 
	  hlib_server->getHandler(), SLOT(reportRXFreq(double))); 

  connect(ui->TXFreq_lab, SIGNAL(newFreq(double)), 
	  hlib_server->getHandler(), SLOT(reportTXFreq(double))); 

  connect(ui->Mode_cb, SIGNAL(valueChanged(int)), 
	  hlib_server->getHandler(), SLOT(reportModulation(int)));

  connect(hlib_server->getHandler(), SIGNAL(setRXFreq(double)),
	  this, SLOT(setRXFreq(double)));

  connect(hlib_server->getHandler(), SIGNAL(setTXFreq(double)),
	  this, SLOT(setTXFreq(double)));

  // connect(hlib_server->getHandler(), &SoDaHamlibHandler::setRXFreq,
  // 	  this, [=] (double f) {
  // 	    qDebug() << "Recentering spectrum displays to frequency: " << f; 
  // 	    this->listener->setSpectrumCenter(f);
  // 	  });
  connect(hlib_server->getHandler(), SIGNAL(setRXFreq(double)),
	  this->listener, SLOT(setSpectrumCenter(double)));

  connect(hlib_server->getHandler(), SIGNAL(setTXFreq(double)),
	  this, SLOT(setTXFreq(double)));

  connect(hlib_server->getHandler(), SIGNAL(setTXOn(bool)), 
	  this->listener, SLOT(setPTT(bool)));
	  
}


