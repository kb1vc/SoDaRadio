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

#ifndef SODA_LIST_SPINBOX_H
#define SODA_LIST_SPINBOX_H

#include <QWidget>
#include <QAbstractSpinBox>
#include <QSpinBox>
#include <QKeyEvent>

#include <iostream>
#include <boost/format.hpp>

#include <cmath>
#include <vector>

class SoDaNoEditSpinbox : public QSpinBox {
public:
  SoDaNoEditSpinbox(QWidget * parent = 0) : QSpinBox(parent) { }

protected:

  void keyPressEvent(QKeyEvent * event) {
    event->ignore();
  }
}; 

class SoDaListSpinbox : public QSpinBox
{
public:
  SoDaListSpinbox(QWidget * parent = 0) : QSpinBox(parent)  {
    init();
  }

  SoDaListSpinbox(std::vector<int> vals)  {
    init();
    setValues(vals); 
  }

  void init() {
    cur_index = 0; 
    setAccelerated(false);
    setReadOnly(true); 
  }
  
  void setValues(std::vector<int> vals) {
    values = vals;
    setValue(values[cur_index]);    
    setMinimum(values[0]);
    setMaximum(values[values.size() - 1]);
    cur_index = values.size() - 1; 
  }

  void stepBy(int steps) {
    cur_index += steps;
    std::cerr << boost::format("stepBy(%d) -> cur_index = %d\n") % steps % cur_index;     
    if(cur_index < 0) cur_index = 0; 
    if(cur_index >= ((int) values.size())) cur_index = values.size() - 1;
    std::cerr << boost::format("\tvalues[%d] = %d\n") % cur_index % values[cur_index];         
    setValue(values[cur_index]);
    std::cerr << boost::format("\tnew value = %d\n") % value();
  }

protected:
  StepEnabled stepEnabled() const {
    if(values.empty()) return StepEnabled(StepNone);
    StepEnabled ret;
    if(cur_index > 0) ret |= StepDownEnabled;
    if((cur_index + 1) < ((int) values.size())) ret |= StepUpEnabled; 
    return ret; 
  }
  
  std::vector<int> values; 
  int cur_index; 
}; 

#endif
