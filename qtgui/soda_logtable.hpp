#ifndef SODA_LOGTABLE_HDR
#define SODA_LOGTABLE_HDR

#include <Qt>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QTableWidget>
#include <QFileDialog>

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
  
protected:
  void openLogFile(); 
  void readLogFile(const QString & fname);  
  void recordEdit(int row, int col);
  
  QStringList current_headers;
};

#endif // SODA_LOGTABLE_HDR
