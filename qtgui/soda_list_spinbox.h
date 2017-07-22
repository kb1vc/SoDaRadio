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
