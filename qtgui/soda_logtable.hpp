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

  
  int last_used_row; 
  QStringList current_headers;

  QFile * log_file_out;
};

#endif // SODA_LOGTABLE_HDR
