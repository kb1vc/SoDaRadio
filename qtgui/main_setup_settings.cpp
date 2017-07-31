#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>
#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"

void MainWindow::setupSettings()
{
  connect(ui->CWSpeed_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setCWSpeed(int)));
  connect(ui->Sidetone_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setSidetoneVolume(int)));
  connect(ui->TXPower_sli, SIGNAL(valueChanged(int)), 
	  listener, SLOT(setTXPower(int)));

  connect(ui->FromGrid_le, SIGNAL(textChanged(const QString &)),
	  ui->FromGrid_lab, SLOT(setText(const QString &)));
}
