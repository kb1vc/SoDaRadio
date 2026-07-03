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

#include "soda_comboboxes.hpp"



void GUISoDa::ValComboBox::addValue(QString lab, double val) {
  valmap[lab] = val;
  addItem(lab, QVariant(val));
  setCurrentIndex(0);
  this->show();
}

void GUISoDa::ValComboBox::setValue(double v) {
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


void GUISoDa::ValComboBox::textChanged(const QString & txt) {
  emit valueChanged(valmap[txt]);
}



void GUISoDa::IntValComboBox::addValue(QString lab, int val) {
  valmap[lab] = val;
  addItem(lab, QVariant(val));
  setCurrentIndex(0);
  this->show();
}

void GUISoDa::IntValComboBox::setValue(int v) {
  for(int i = 0; i < count(); i++) {
    QString key = itemText(i);
    if(v == valmap[key]) {
      setCurrentIndex(i); 
      return; 
    }
  }
}

void GUISoDa::IntValComboBox::setValue(const QString & v) {
  for(int i = 0; i < count(); i++) {
    QString key = itemText(i);
    if(v == key) {
      setCurrentIndex(i); 
      return; 
    }
  }
}


void GUISoDa::IntValComboBox::textChanged(const QString & txt) {
  emit valueChanged(valmap[txt]);
}
