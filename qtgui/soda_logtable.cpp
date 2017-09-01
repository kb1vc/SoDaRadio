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

#include "soda_logtable.hpp"
#include <iostream>
#include <QFile>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QDateTime>

LogTable::LogTable(QWidget *parent) :
  QTableWidget(parent)
{
  horizontalHeader()->setStretchLastSection(true);

  QStringList headers;
  headers << "Date" << "Time" << "From Call" << "From Grid" << "To Call" << "To Grid"
	  << "Mode" << "RX Freq" << "TX Freq" << "Comment";
  setColumnCount(headers.size());  
  qDebug() << "header list size = " << headers.size();
  qDebug() << "Column count is now = " << columnCount(); 
  
  setKeys(headers);

  setShowGrid(true);


  connect(this, &LogTable::cellChanged,
	  [this](int row, int col) {
	    emit entryUpdated(row, 
			      this->current_headers.at(col), 
			      this->item(row, col)->text()); });

  connect(this, SIGNAL(cellChanged(int, int)),
	  this, SLOT(recordChange(int, int)));

  next_used_row = 0; 
  
  log_file_out = NULL; 
}

void LogTable::setKeys(QStringList headers)
{
  current_headers = headers; // save the header list -- they'll be used for keys.
  setHorizontalHeaderLabels(headers);
}

LogTable::~LogTable()
{
  // write the log report
  // close the log file.
  if(log_file_out != NULL) {
    log_file_out->close();
  }
}

bool LogTable::emptyRow(int r) {
  for(int i = 0; i < columnCount(); i++) {
    if(item(r, i)) return false; 
  }
  return true; 
}

void LogTable::writeLogReport(const QString & fname)
{
  if(log_file_out != NULL) {
    log_file_out->close();
  }

  log_file_out = new QFile(fname); 
  if(!log_file_out->open(QIODevice::WriteOnly)) {
    QMessageBox::information(this, tr("Unable to open file for writing"),
			     log_file_out->errorString());
    return; 
  }

  int rows = rowCount();
  int cols = columnCount();

  // we're going to write row,col,value streams to the output
  QTextStream out(log_file_out);
  for(int i = 0; i < rows; i++) {

    if(emptyRow(i)) continue; 

    for(int j = 0; j < cols; j++) {
      QTableWidgetItem * itm(item(i,j));
      if(itm) {
	out << QString("%1:%2:%3").arg(i).arg(j).arg(itm->text()) << endl;
      }
    }
  }
  log_file_out->flush();
}

void LogTable::recordChange(int r, int c)
{
  if((log_file_out != NULL) && log_file_out->isOpen()) {
    QTextStream out(log_file_out);
    QTableWidgetItem * itm(item(r,c));    
    if(itm) {
      out << QString("%1:%2:%3").arg(r).arg(c).arg(item(r,c)->text()) << endl;    
    }
    else {
      out << QString("%1:%2: ").arg(r).arg(c) << endl;
    }
    log_file_out->flush();
  }
}


void LogTable::readLogReport(const QString & fname)
{
  // load the table from the input stream. 
  qDebug() << QString("Reading log file [%1]").arg(fname);
  QFile infile(fname);

  if(!infile.open(QFile::ReadOnly)) {
    QMessageBox::information(this, tr("Unable to open file for reading"),
			     infile.errorString());
    return; 
  }

  QString lbuf;
  QTextStream instr(&infile); 
  int row_start = next_used_row; 
  int max_row = -1; 
  while(!(lbuf = instr.readLine()).isNull()) {
    int co0 = lbuf.indexOf(':');
    int co1 = lbuf.indexOf(':',co0+1);
    
    // strip out row, col.
    int row = lbuf.mid(0,co0).toInt() + row_start;
    int col = lbuf.mid(co0+1,co1 - (co0+1)).toInt();    
    QString val = lbuf.mid(co1+1); 
    if((row + 10) > rowCount()) setRowCount(rowCount() + 20);
    setItem(row, col, new QTableWidgetItem(val));
    if(row > max_row) max_row = row; 
  }
  next_used_row = max_row + 1; 
  infile.close();
}

void LogTable::readLogReportDlg()
{
  QString fname = QFileDialog::getOpenFileName(this, 
					       tr("Read Log Report from File"), 
					       "", 
					       tr("*.soda_log (*.soda_log);;All Files(*)"));
  if(!fname.isEmpty()) readLogReport(fname);
}

void LogTable::writeLogReportDlg()
{
  QString fname = QFileDialog::getSaveFileName(this, 
					       tr("Write Log Report to File"), 
					       "", 
					       tr("*.soda_log (*.soda_log);;All Files(*)"));
  if(!fname.isEmpty()) writeLogReport(fname);
}


void LogTable::logContact(const QString & from_call,
			  const QString & from_grid,
			  const QString & to_call,
			  const QString & to_grid,
			  const QString & mode,		  
			  const QString & comment,
			  double rx_freq,
			  double tx_freq)
{
  QDateTime utc_time(QDateTime::currentDateTime().toUTC());

  setField(next_used_row, "Date", utc_time.toString("d-MMM-yyyy"));
  setField(next_used_row, "Time", utc_time.toString("hh:mm:ss"));  
  setField(next_used_row, "From Call", from_call);
  setField(next_used_row, "From Grid", from_grid);   
  setField(next_used_row, "To Call", to_call);
  setField(next_used_row, "To Grid", to_grid);
  setField(next_used_row, "Mode", mode);
  setField(next_used_row, "Comment", comment);
  setField(next_used_row, "RX Freq", rx_freq);
  setField(next_used_row, "TX Freq", tx_freq);
  // scroll so the edit log window shows the last line in the log
  scrollToItem(item(next_used_row, 1));

  // bump the pointer to the next row
  next_used_row++;
  // if we're almost out, allocate a bunch of new rows. 
  if((next_used_row + 10) > rowCount()) {
    setRowCount(rowCount() + 20);
  }
}

void LogTable::setLogFile(const QString & fname)
{
}
