#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>
#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"

void MainWindow::setupLogGPS()
{
  // connect the log contact button to something that will do us some good.  
  connect(ui->LogContact_btn, SIGNAL(clicked(bool)), 
	  this, SLOT(logContact(bool))); 
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
