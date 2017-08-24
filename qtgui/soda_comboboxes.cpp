#include "soda_comboboxes.hpp"



void ValComboBox::addValue(QString lab, double val) {
  valmap[lab] = val;
  addItem(lab, QVariant(val));
  setCurrentIndex(0);
  this->show();
}

void ValComboBox::setValue(double v) {
  double diff = 1e23;
  int bestidx;
  for(int i = 0; i < count(); i++) {
    QString key = itemText(i);
    double nd = fabs(v - valmap[key]);
    if(nd < diff) {
      diff = nd;
      bestidx = i;
    }
  }
  setCurrentIndex(bestidx);
}


void ValComboBox::textChanged(const QString & txt) {
  emit valueChanged(valmap[txt]);
}



void IntValComboBox::addValue(QString lab, int val) {
  valmap[lab] = val;
  addItem(lab, QVariant(val));
  setCurrentIndex(0);
  this->show();
}

void IntValComboBox::setValue(int v) {
  for(int i = 0; i < count(); i++) {
    QString key = itemText(i);
    if(v == valmap[key]) {
      setCurrentIndex(i); 
      return; 
    }
  }
}

void IntValComboBox::setValue(const QString & v) {
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
