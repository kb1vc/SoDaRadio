#include "soda_comboboxes.hpp"



void ValComboBox::addValue(QString lab, double val) {
  valmap[lab] = val;
  addItem(lab, QVariant(val));
  setCurrentIndex(0);
  this->show();
}

void ValComboBox::setSelection(double v) {
  double diff = 1e23;
  std::cerr << boost::format("Looking for value %g\n") % v;
  int bestidx;
  for(int i = 0; i < count(); i++) {
    QString key = itemText(i);
    double nd = fabs(v - valmap[key]);
    std::cerr << boost::format("\t comparing to valmap[%s] = %g at idx %d\n")
      % key.toStdString() % valmap[key] % i;
    if(nd < diff) {
      diff = nd;
      bestidx = i;
    }
  }
  std::cerr << boost::format("\tBest idx = %d\n") % bestidx;
  setCurrentIndex(bestidx);
}


void ValComboBox::textChanged(const QString & txt) {
  std::cerr << boost::format("Text changed to %s\n") % txt.toStdString();
  emit valueChanged(valmap[txt]);
}



void IntValComboBox::addValue(QString lab, int val) {
  valmap[lab] = val;
  addItem(lab, QVariant(val));
  setCurrentIndex(0);
  this->show();
}

void IntValComboBox::setSelection(int v) {
  std::cerr << boost::format("Looking for value %d\n") % v;
  for(int i = 0; i < count(); i++) {
    QString key = itemText(i);
    if(v == valmap[key]) {
      setCurrentIndex(i); 
      return; 
    }
  }
}

void IntValComboBox::setSelection(const QString & v) {
  for(int i = 0; i < count(); i++) {
    QString key = itemText(i);
    if(v == key) {
      setCurrentIndex(i); 
      return; 
    }
  }
}


void IntValComboBox::textChanged(const QString & txt) {
  emit valueChanged(valmap[txt]);
}
