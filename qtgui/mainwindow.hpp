#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSettings>
#include "soda_listener.hpp"
#include "../common/GuiParams.hxx"
#include "soda_band.hpp"
#include "soda_hamlib_server.hpp"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent, SoDa::GuiParams & params);
  ~MainWindow();

public slots:
  void setRXFreq(double freq);
  void setTXFreq(double freq);
  void restoreSettings(); 

  void changeBand(const QString & band);
  void writeBandMapEntry(bool);
  void fillBandMapEntry(const QString & band);

  void handleFatalError(const QString & err_string);

  void logContact(bool); 

protected:
  void setupTopControls();
  void setupMidControls();
  void setupLogGPS();

  void setupSettings();
  void setupBandConfig();
  void setupLogEditor();
  
  void setupWaterFall();
  void setupSpectrum();

  void setupHamlib(); 

  void widgetSaveRestore(QObject * op, const QString & par, bool save);

  void bandMapSaveRestore(SoDaBandMap & bmap, bool save);
  void saveCurrentFreqs();
  
  void sendCannedCW(const QString & txt);
  
private:
  void closeEvent(QCloseEvent * event) {
    std::cerr << "In window close event\n";
    listener->closeRadio();
    event->accept();
  }

  QSettings * settings_p; 

  // Band map
  QMap<QString, SoDaBand> band_map; 
  QString current_band_selector; 

  void setRXFreq_nocross(double freq);
  void setTXFreq_nocross(double freq);   
  
  Ui::MainWindow *ui;

  SoDaListener * listener; 

  SoDaHamlibServer * hlib_server; 
};

#endif // MAINWINDOW_H
