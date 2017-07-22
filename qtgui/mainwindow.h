#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void newFreq(double freq);
    void closeRadio();

protected:
  void setupWaterFall();
  void setupSpectrum();
  
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
