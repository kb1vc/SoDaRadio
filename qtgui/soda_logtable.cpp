#include "soda_logtable.hpp"
#include <iostream>
#include <QFile>
#include <QMessageBox>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>

LogTable::LogTable(QWidget *parent) :
  QTableWidget(parent)
{
  horizontalHeader()->setStretchLastSection(true);
  setShowGrid(true);

  QStringList headers;
  headers << "Date" << "Time" << "From Call" << "From Grid" << "To Call" << "To Grid"
	  << "Mode" << "Comment" << "RX Freq" << "TX Freq";
  setKeys(headers);

  // hide the last two columns
  hideColumn(columnCount() - 1);
  hideColumn(columnCount() - 1);

  connect(this, &LogTable::cellChanged,
	  [this](int row, int col) {
	    emit entryUpdated(row, 
			      this->current_headers.at(col), 
			      this->item(row, col)->text()); });

  connect(this, SIGNAL(cellChanged(int, int)),
	  this, SLOT(recordChange(int, int)));
}

void LogTable::setKeys(QStringList headers)
{
  current_headers = headers; // save the header list -- they'll be used for keys.
  setColumnCount(headers.size());
  setHorizontalHeaderLabels(headers);
}

LogTable::~LogTable()
{
  // write the log report
  // close the log file.
  
}

void LogTable::writeLogReport(const QString & fname)
{
  QFile file(fname); 
  if(!file.open(QIODevice::WriteOnly)) {
    QMessageBox::information(this, tr("Unable to open file"),
			     file.errorString());
    return; 
  }

  int rows = rowCount();
  int cols = columnCount();

  QXmlStreamWriter xmlo(&file);
  xmlo.setAutoFormatting(true);
  xmlo.writeStartDocument();
  for (int i = 0; i < rows; i++) {
    xmlo.writeStartElement("Entry");
    xmlo.writeAttribute("row", QString::number(i));
    for (int j = 0; j < cols; j++) {
      xmlo.writeTextElement(current_headers.at(j), item(i, j)->text());
    }
    xmlo.writeEndElement();
  }
  xmlo.writeEndDocument();
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
  QString fname = QFileDialog::getOpenFileName(this, 
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
}

void LogTable::setLogFile(const QString & fname)
{
}
