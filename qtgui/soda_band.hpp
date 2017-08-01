#ifndef SODA_BAND_HDR
#define SODA_BAND_HDR
#include <QString>
#include <QObject>
#include <QSettings>

class SoDaBand {
public:
  SoDaBand() {
  }

  void restore(QSettings * set_p) {
    band_name = set_p->value("Name").toString();
    band_index = set_p->value("Index").toInt();
    def_rx_ant = set_p->value("DefRXAnt").toString();
    def_tx_ant = set_p->value("DefTXAnt").toString();
    min_freq = set_p->value("MinFreq").toDouble();
    max_freq = set_p->value("MaxFreq").toDouble();
    tv_LO_freq = set_p->value("TVLOFreq").toDouble();
    tv_LO_mult = set_p->value("TVLOMult").toDouble();
    tx_enabled = set_p->value("TXEna").toBool();
    tverter_mode = set_p->value("TVMode").toBool();
    lowside_injection = set_p->value("TVLowInjection").toBool();    
  }

  void save(QSettings * set_p) const {
    set_p->setValue("Name", band_name);
    set_p->setValue("Index", band_index);
    set_p->setValue("DefRXAnt", def_rx_ant);
    set_p->setValue("DefTXAnt", def_tx_ant);
    set_p->setValue("MinFreq", min_freq);
    set_p->setValue("MaxFreq", max_freq);
    set_p->setValue("TVLOFreq", tv_LO_freq);
    set_p->setValue("TVLOMult", tv_LO_mult);
    set_p->setValue("TXEna", tx_enabled);
    set_p->setValue("TVMode", tverter_mode);
    set_p->setValue("TVLowInjection", lowside_injection);
  }

  QString & name() const  { return band_name; }
  int index() const  { return band_index; }
  QString & defRXAnt() const  { return def_rx_ant; }
  QString & defTXAnt() const  { return def_tx_ant; }
  double minFreq() const  { return min_freq; }
  double maxFreq() const  { return max_freq; }
  double tvLOFreq() const  { return tv_LO_freq; }
  double tvLOMult() const  { return tv_LO_mult; }  
  bool txEna() const  { return tx_enabled; }
  bool tverterEna() const  { return tverter_mode; }
  bool tverterLowInjection() const  { return lowside_injection; }

  void setName(const QString & v) { band_name= v; }
  void setIndex(int v) { band_index= v; }
  void setDefRXAnt(const QString & v) { def_rx_ant= v; }
  void setDefTXAnt(const QString & v) { def_tx_ant= v; }
  void setMinFreq(double v) { min_freq= v; }
  void setMaxFreq(double v) { max_freq= v; }
  void setTvLOFreq(double v) { tv_LO_freq= v; }
  void setTvLOMult(double v) { tv_LO_mult= v; }  
  void setTxEna(bool v) { tx_enabled= v; }
  void setTverterEna(bool v) { tverter_mode= v; }
  void setTverterLowInjection(bool v) { lowside_injection= v; }

  static QList<SoDaBand> restoreBandList(QSettings * set_p) {
    QList<SoDaBand> bandlist; 
    int size = set_p->beginReadArray("Bands");
    for(int i = 0; i < size; i++) {
      SoDaBand b; 
      set_p->setArrayIndex(i);
      b.restore(set_p); 
    }
  }

  static void restoreBandList(QSettings * set_p, const QList<SoDaBand> & bandlist) {
    set_p->beginWriteArray("Bands");
    for(int i = 0; i < bandlist.size(); i++) {
      set_p->setArrayIndex(i);
      bandlist.at(i).save(set_p); 
    }
  }
  
protected:
  QString band_name; 
  int band_index; 
  QString def_rx_ant;
  QString def_tx_ant;
  double min_freq; 
  double max_freq; 
  QString def_mode; 
  bool tx_enabled; 
  bool tverter_mode; 
  bool lowside_injection; 
  double tv_LO_freq; 
  double tv_LO_mult; 
}; 


#endif
