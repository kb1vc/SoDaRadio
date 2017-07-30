#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <iostream>
#include <boost/format.hpp>
#include "soda_comboboxes.hpp"
#include "soda_listener.hpp"

void MainWindow::setupMidControls()
{
  // These controls include band selection, PTT, TX display,
  // antenna selection, CW QSO buttons, and the navigation panel

  connect(listener, &SoDaListener::addRXAntName, 
	  [this](const QString & v){
	    ui->RXAnt_sel->addItem(v); });
  connect(listener, &SoDaListener::addTXAntName, 
	  [this](const QString & v){
	    ui->TXAnt_sel->addItem(v); });

  connect(ui->RXAnt_sel, SIGNAL(activated(const QString &)),
	  listener, SLOT(setRXAnt(const QString &)));
  connect(ui->TXAnt_sel, SIGNAL(activated(const QString &)),
	  listener, SLOT(setTXAnt(const QString &)));


}
