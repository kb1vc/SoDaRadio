/*
  Copyright (c) 2025 Matthew H. Reilly (kb1vc)
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



#include <QString>
#include <QObject>
#include <QSettings>
#include <QMap>

#include "BandMap.hpp"

namespace GUISoDa {

  void BandMap::restoreBands(QSettings * set_p) {
    int size = set_p->beginReadArray("Bands");
    for(int i = 0; i < size; i++) {
      Band b;
      set_p->setArrayIndex(i);
      QString dname = set_p->value("Name").toString();
      b.restore(set_p);
      QString bn = b.name();
      (*this)[b.name()] = b;
    }
    set_p->endArray();
  }

  void BandMap::saveBands(QSettings * set_p) {
    set_p->beginWriteArray("Bands");
    BandMapIterator bmi(*this);    
    int i = 0;    
    while(bmi.hasNext()) {
      bmi.next();
      if(bmi.value().name() == "") continue;       
      set_p->setArrayIndex(i);
      bmi.value().save(set_p); 
      i++; 
    }
    set_p->endArray();
  }
    
  QString BandMap::findBand(double freq) const {
    BandMapIterator bmi(*this);
    while(bmi.hasNext()) {
      bmi.next();
      if(bmi.value().isInBand(freq)) return bmi.value().name();
    }
    return ""; 
  }
  
}
