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

#ifndef SODA_WFALL_COMBO_BOXES_H
#define SODA_WFALL_COMBO_BOXES_H

#include <QWidget>
#include <QComboBox>
#include <QString>
#include <boost/format.hpp>
#include <vector>
#include <map> 
#include <cmath>
#include <iostream>

  class ValComboBox : public QComboBox { 
      Q_OBJECT
  public:
    explicit ValComboBox(QWidget * parent = 0) : QComboBox(parent) {

        clear();
        connect(this, &QComboBox::currentTextChanged, this, &ValComboBox::textChanged);
    }

    double value() { 
      return valmap[currentText()]; 
    }

  public slots:
    void addValue(QString lab, double val); 
    void setValue(double v);
    
  signals:
    void valueChanged(double v);

  protected:
    void textChanged(const QString & txt);
    
    std::map<QString, double> valmap;
  };

  class WFallDynRangeComboBox : public ValComboBox {
      Q_OBJECT
  public:
    WFallDynRangeComboBox(QWidget * parent = 0) : ValComboBox(parent) {
      
      addValue("50", 50.0);
      addValue("45", 45.0);
      addValue("40", 40.0);	    
      addValue("35", 35.0);
      addValue("30", 30.0);	          
      addValue("25", 25.0);
      addValue("15", 15.0);       
      addValue("10", 10.0);
      addValue("5", 5.0);                  
      setValue(25.0);
    }
  }; 

  class WFallSpanComboBox : public ValComboBox {
      Q_OBJECT
  public:
    WFallSpanComboBox(QWidget * parent = 0) : ValComboBox(parent) {
      addValue("200", 200.0);
      addValue("100", 100.0);
      addValue("50", 50.0);
      addValue("25", 25.0);
      addValue("10", 10.0);            
      setValue(200.0);
    }
  }; 


  class IntValComboBox : public QComboBox { 
      Q_OBJECT
  public:
    explicit IntValComboBox(QWidget * parent = 0) : QComboBox(parent) {

        clear();
        connect(this, &QComboBox::currentTextChanged, this, &IntValComboBox::textChanged);
    }

    int value() { 
      return valmap[currentText()]; 
    }

  public slots:
    void addValue(QString lab, int val); 

    void setValue(int v);
    void setValue(const QString & s);    
    
  signals:
    void valueChanged(int v);

  protected:
    void textChanged(const QString & txt);
    
    std::map<QString, int> valmap;
  };

#endif
