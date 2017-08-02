#ifndef SODA_BAND_HDR
#define SODA_BAND_HDR
#include <QString>
#include <QObject>
#include <QSettings>
#include <QMap>

class SoDaBand;

typedef QMap<QString, SoDaBand> SoDaBandMap;
typedef QMapIterator<QString, SoDaBand> SoDaBandMapIterator; 

class SoDaBand {
public:
  SoDaBand() {
  }

  void restore(QSettings * set_p) {
    band_name = set_p->value("Name").toString();
    band_index = set_p->value("Index").toInt();

    def_rx_ant = set_p->value("DefRXAnt").toString();
    def_tx_ant = set_p->value("DefTXAnt").toString();

    def_mode = set_p->value("DefMode").toString();    

    min_freq = set_p->value("MinFreq").toDouble();
    max_freq = set_p->value("MaxFreq").toDouble();

    last_rx_freq = set_p->value("LastRXFreq").toDouble();
    last_tx_freq = set_p->value("LastTXFreq").toDouble();

    tx_enabled = set_p->value("TXEna").toBool();    

    tv_LO_freq = set_p->value("TVLOFreq").toDouble();
    tv_LO_mult = set_p->value("TVLOMult").toDouble();
    tverter_mode = set_p->value("TVMode").toBool();
    lowside_injection = set_p->value("TVLowInjection").toBool();    
  }

  void save(QSettings * set_p) const {
    set_p->setValue("Name", band_name);
    set_p->setValue("Index", band_index);
    set_p->setValue("DefRXAnt", def_rx_ant);
    set_p->setValue("DefTXAnt", def_tx_ant);
    set_p->setValue("DefMode", def_mode);    
    set_p->setValue("MinFreq", min_freq);
    set_p->setValue("MaxFreq", max_freq);
    set_p->setValue("LastRXFreq", last_rx_freq);
    set_p->setValue("LastTXFreq", last_tx_freq);
    set_p->setValue("TVLOFreq", tv_LO_freq);
    set_p->setValue("TVLOMult", tv_LO_mult);
    set_p->setValue("TXEna", tx_enabled);
    set_p->setValue("TVMode", tverter_mode);
    set_p->setValue("TVLowInjection", lowside_injection);
  }

  const QString & name() const  { return band_name; }
  int index() const  { return band_index; }
  const QString & defRXAnt() const  { return def_rx_ant; }
  const QString & defTXAnt() const  { return def_tx_ant; }
  const QString & defMode() const  { return def_mode; }  
  double minFreq() const  { return min_freq; }
  double maxFreq() const  { return max_freq; }
  double lastRXFreq() const { return last_rx_freq; }
  double lastTXFreq() const { return last_tx_freq; }
  
  double tvLOFreq() const  { return tv_LO_freq; }
  double tvLOMult() const  { return tv_LO_mult; }  
  bool txEna() const  { return tx_enabled; }
  bool tverterEna() const  { return tverter_mode; }
  bool tverterLowInjection() const  { return lowside_injection; }

  void setName(const QString & v) { band_name = v; }
  void setIndex(int v) { band_index = v; }
  void setDefRXAnt(const QString & v) { def_rx_ant = v; }
  void setDefTXAnt(const QString & v) { def_tx_ant = v; }
  void setDefMode(const QString & v) { def_mode = v; }  
  
  void setMinFreq(double v) { min_freq = v; }
  void setMaxFreq(double v) { max_freq = v; }
  void setLastRXFreq(double v) { last_rx_freq = v; }
  void setLastTXFreq(double v) { last_tx_freq = v; }  
  void setTvLOFreq(double v) { tv_LO_freq = v; }
  void setTvLOMult(double v) { tv_LO_mult = v; }  
  void setTxEna(bool v) { tx_enabled = v; }
  void setTverterEna(bool v) { tverter_mode = v; }
  void setTverterLowInjection(bool v) { lowside_injection = v; }

  static void restoreBands(QSettings * set_p, SoDaBandMap & band_map) {
    int size = set_p->beginReadArray("Bands");
    qDebug() << QString("Band map will have %1 entries").arg(size);
    for(int i = 0; i < size; i++) {
      SoDaBand b; 
      set_p->setArrayIndex(i);
      QString dname = set_p->value("Name").toString();
      qDebug() << QString("Got direct name [%1]").arg(dname);
      b.restore(set_p); 
      qDebug() << "About to get b.name()";
      QString bn = b.name();
      qDebug() << QString("Band map entry %1 gets name [%2]").arg(i).arg(bn);
      band_map[b.name()] = b; 
    }
    qDebug() << "Left band map loop";
    set_p->endArray();
    qDebug() << "restoreBands returned";    
  }

  static void saveBands(QSettings * set_p, SoDaBandMap & band_map) {
    set_p->beginWriteArray("Bands");
    SoDaBandMapIterator bmi(band_map);
    int i = 0; 
    while(bmi.hasNext()) {
      bmi.next();
      set_p->setArrayIndex(i);
      bmi.value().save(set_p); 
      i++; 
    }
    set_p->endArray();
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
  double last_rx_freq; 
  double last_tx_freq; 
}; 


#endif
