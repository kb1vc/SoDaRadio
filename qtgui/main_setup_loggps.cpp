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
#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"
#include "../common/Navigation.hxx"

void MainWindow::setupLogGPS()
{
  // connect the log contact button to something that will do us some good.  
  connect(ui->LogContact_btn, SIGNAL(clicked(bool)), 
	  this, SLOT(logContact(bool))); 

  connect(ui->FromGrid_le, SIGNAL(textChanged(const QString &)), 
	  this, SLOT(evalNav(const QString &)));
  connect(ui->ToGrid_le, SIGNAL(textChanged(const QString &)), 
	  this, SLOT(evalNav(const QString &)));

  connect(listener, SIGNAL(repGPSTime(int, int, int)), 
	  this, SLOT(updateTime(int, int, int)));
  connect(listener, SIGNAL(repGPSLatLon(double, double)),
	  this, SLOT(updatePosition(double, double)));
}

void MainWindow::evalNav(const QString & dummy)
{
  (void) dummy; 
  QString from_grid = ui->FromGrid_le->text();
  QString to_grid = ui->ToGrid_le->text();
  
  float bearing, rbearing, distance; 
  int stat = GetBearingDistance(from_grid.toStdString(), to_grid.toStdString(), bearing, rbearing, distance);

  if(stat == 0) {
    ui->Bearing_lab->setText(QString("%1").arg(bearing, 3, 'f', 0));
    ui->RevBearing_lab->setText(QString("%1").arg(rbearing, 3, 'f', 0));
    ui->Range_lab->setText(QString("%1").arg(distance, 4, 'f', 0));
  }
}
/**
 * @brief log the current contact by storing the grid, call, date, time, 
 * frequency, mode, and other stuff into the log object. 
 */
void MainWindow::logContact(bool dummy) 
{
  (void) dummy; 
  ui->LogView->logContact(ui->FromCall_le->text(), 
			  ui->FromGrid_le->text(), 
			  ui->ToCall_le->text(), 
			  ui->ToGrid_le->text(), 
			  ui->Mode_cb->currentText(), 
			  ui->LogComment_txt->text(), 
			  ui->RXFreq_lab->getFreq(),
			  ui->TXFreq_lab->getFreq());
}

void MainWindow::updateTime(int h, int m, int s)
{
  QChar fc('0');
  ui->UTC_lab->setText(QString("%1:%2:%3").arg(h,2,10,fc).arg(m,2,10,fc).arg(s,2,10,fc));
}

void MainWindow::updatePosition(double lat, double lon)
{
  QString grid = QString::fromStdString(GetGridSquare(lat, lon));

  ui->GRID_lab->setText(grid);

  ui->LAT_lab->setText(QString("%1").arg(lat, 5, 'f', 2));
  ui->LON_lab->setText(QString("%1").arg(lon, 6, 'f', 2));  

  if(ui->useGPS_ck->isChecked()) {
    ui->FromGrid_lab->setText(grid);
    ui->FromGrid_le->setText(grid);
  }
}
