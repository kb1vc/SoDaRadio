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

  last_used_row = 0; 
  
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
    QMessageBox::information(this, tr("Unable to open file"),
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
}

void LogTable::readLogReportDlg()
{
  QString fname = QFileDialog::getOpenFileName(this, 
					       tr("Read Log Report from File"), 
					       "", 
					       tr("SoDa Log (*.soda_log);;All Files(*)"));
  if(!fname.isEmpty()) readLogReport(fname);
}

void LogTable::writeLogReportDlg()
{
  QString fname = QFileDialog::getSaveFileName(this, 
					       tr("Write Log Report to File"), 
					       "", 
					       tr("SoDa Log (*.soda_log);;All Files(*)"));
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

  setField(last_used_row, "Date", utc_time.toString("d-MMM-yyyy"));
  setField(last_used_row, "Time", utc_time.toString("hh:mm:ss"));  
  setField(last_used_row, "From Call", from_call);
  setField(last_used_row, "From Grid", from_grid);   
  setField(last_used_row, "To Call", to_call);
  setField(last_used_row, "To Grid", to_grid);
  setField(last_used_row, "Mode", mode);
  setField(last_used_row, "Comment", comment);
  setField(last_used_row, "RX Freq", rx_freq);
  setField(last_used_row, "TX Freq", tx_freq);
  last_used_row++;  
}

void LogTable::setLogFile(const QString & fname)
{
}
