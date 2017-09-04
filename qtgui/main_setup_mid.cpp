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

void MainWindow::setupMidControls()
{
  // These controls include band selection, PTT, TX display,
  // antenna selection, CW QSO buttons, and the navigation panel

  connect(listener, &GUISoDa::Listener::addRXAntName, 
	  [this](const QString & v){
	    ui->RXAnt_sel->addItem(v); });
  connect(listener, &GUISoDa::Listener::addTXAntName, 
	  [this](const QString & v){
	    ui->TXAnt_sel->addItem(v); });

  connect(ui->RXAnt_sel, SIGNAL(currentTextChanged(const QString &)),
	  listener, SLOT(setRXAnt(const QString &)));
  connect(ui->TXAnt_sel, SIGNAL(currentTextChanged(const QString &)),
	  listener, SLOT(setTXAnt(const QString &)));

  connect(ui->PTT_btn, SIGNAL(toggled(bool)), 
	  listener, SLOT(setPTT(bool)));

  connect(listener, &GUISoDa::Listener::repPTT,
	  [=](bool on) { ui->TXState_lab->setText(on ? "TX ON" : "TX OFF"); });

  connect(ui->CWCurLine_le, &QLineEdit::returnPressed, 
	  [=]() 
	  { ui->CWOutBound_te->appendPlainText(ui->CWCurLine_le->text());
	    listener->sendCW(ui->CWCurLine_le->text());
	    ui->CWCurLine_le->clear(); });
	    

  connect(ui->ClrBuff_btn, SIGNAL(released()), 
	  listener, SLOT(clearCWBuffer()));

  connect(ui->MyCall_btn, &QPushButton::pressed,
	  [=]() {
	    int i;
	    for(i = 0; i < ui->RptCount_spin->value(); i++) {
	      sendCannedCW(ui->FromCall_le->text()); 
	    }
	  });

  connect(ui->MyGrid_btn, &QPushButton::pressed,
	  [=]() {
	    int i;
	    for(i = 0; i < ui->RptCount_spin->value(); i++) {
	      sendCannedCW(ui->FromGrid_le->text()); 
	    }
	  });
  
  connect(ui->MyInfo_btn, &QPushButton::pressed,
	  [=]() {
	    int i;
	    for(i = 0; i < ui->RptCount_spin->value(); i++) {
	      sendCannedCW(" " + ui->FromCall_le->text());	    	      
	    }
	    for(i = 0; i < ui->RptCount_spin->value(); i++) {
	      sendCannedCW(" " + ui->FromGrid_le->text());
	    }
	  });	  

  connect(ui->Exchange_btn, &QPushButton::pressed,
	  [=]() {
	    int i;
	    for(i = 0; i < ui->RptCount_spin->value(); i++) {
	      sendCannedCW(" " + ui->ToCall_le->text());	    	      
	    }
	    sendCannedCW(" de ");
	    for(i = 0; i < ui->RptCount_spin->value(); i++) {
	      sendCannedCW(" " + ui->FromCall_le->text());	    	      
	    }
	    for(i = 0; i < ui->RptCount_spin->value(); i++) {
	      sendCannedCW(" " + ui->FromGrid_le->text());
	    }
	  });	  
  
  connect(ui->CWV_btn, &QPushButton::pressed,
	  [=]() {
	    int i; 
	    for(i = 0; i < ui->RptCount_spin->value(); i++) {
	      sendCannedCW("VVVVVVVVVVVV"); 
	    }
	  });	  

  connect(ui->CW73_btn, &QPushButton::pressed,
	  [=]() {
	    sendCannedCW(" TNX es 73 de " + ui->FromCall_le->text());
	  });

  connect(ui->CWQSL_btn, &QPushButton::pressed,
	  [=]() {
	    sendCannedCW(" R R R  QSL QSL QSL de " + ui->FromCall_le->text()); 
	  });	  

  connect(ui->CWBK_btn, &QPushButton::pressed,
	  [=]() {
	    sendCannedCW(" _bk ");
	  });	  
}

void MainWindow::sendCannedCW(const QString & txt) 
{
  listener->sendCW(txt); 
  ui->CWOutBound_te->appendPlainText(txt); 
}
