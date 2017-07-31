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

  connect(ui->RXAnt_sel, SIGNAL(currentTextChanged(const QString &)),
	  listener, SLOT(setRXAnt(const QString &)));
  connect(ui->TXAnt_sel, SIGNAL(currentTextChanged(const QString &)),
	  listener, SLOT(setTXAnt(const QString &)));

  connect(ui->PTT_btn, SIGNAL(toggled(bool)), 
	  listener, SLOT(setPTT(bool)));

  connect(listener, &SoDaListener::repPTT,
	  [=](bool on) { ui->TXState_lab->setText(on ? "TX ON" : "TX OFF"); });

  connect(ui->CWCurLine_le, &QLineEdit::returnPressed, 
	  [=]() 
	  { ui->CWOutBound_te->appendPlainText(ui->CWCurLine_le->text());
	    listener->sendCW(ui->CWCurLine_le->text());
	    ui->CWCurLine_le->clear(); });
	    

  connect(ui->ClrBuff_btn, SIGNAL(released()), 
	  listener, SLOT(clearCWBuffer()));
  
}
