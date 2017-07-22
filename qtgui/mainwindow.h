#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "soda_listener.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0, QString socket_basename = "/tmp/sodasocket");
  ~MainWindow();

public slots:
  void newFreq(double freq);
  void closeRadio();

protected:
  void setupWaterFall();
  void setupSpectrum();
  
private:
  Ui::MainWindow *ui;

  SoDaListener * listener; 
};

#endif // MAINWINDOW_H
