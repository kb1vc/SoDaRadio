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

#ifndef SODA_LOGTABLE_HDR
#define SODA_LOGTABLE_HDR

#include <Qt>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QTableWidget>
#include <QFileDialog>
#include <QDebug>
#include <iostream>

namespace Ui {
  class LogTable;
}

namespace GUISoDa {
  class LogTable : public QTableWidget
  {
    Q_OBJECT

  public:
    explicit LogTable(QWidget *parent = Q_NULLPTR);

    ~LogTable();

  signals:
    void entryUpdated(int row, const QString & key, const QString & val);

  public slots:
    void logContact(const QString & from_call,
		    const QString & from_grid,
		    const QString & to_call,
		    const QString & to_grid,
		    const QString & mode,		  
		    const QString & comment,
		    double rx_freq,
		    double tx_freq);


    void setKeys(QStringList headers);
    void setLogFile(const QString & fname);

    void writeLogReport(const QString & fname);
    void readLogReport(const QString & fname);  

    void writeLogReportDlg();
    void readLogReportDlg();

  protected slots:
    void recordChange(int r, int c);
  
  protected:
    void openLogFile(); 
    void readLogFile(const QString & fname);  
    void recordEdit(int row, int col);

    bool emptyRow(int r); 

    void setField(int row, const QString & key_st, const QString & st) {
      int ncol = current_headers.indexOf(key_st);
      if(ncol < 0) {
	qDebug() << QString("Could not find column key [%1] in log table column headers").arg(key_st);
	return; 
      }
      setItem(row, ncol, new QTableWidgetItem(st));
    }

    void setField(int row, const QString & key_st, double val) {
      int ncol = current_headers.indexOf(key_st);
      if(ncol < 0) {
	qDebug() << QString("Could not find column key [%1] in log table column headers").arg(key_st);
	return; 
      }
      setItem(row, ncol, new QTableWidgetItem(QString("%1").arg(val, 15, 'f')));
    }

  
    int next_used_row; 
    QStringList current_headers;

    QFile * log_file_out;
  };
}

#endif // SODA_LOGTABLE_HDR
