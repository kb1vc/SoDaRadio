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

    void addValue(QString lab, double val); 

    void setSelection(double v);
    
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
      
      addValue("100", 100.0);
      addValue("50", 50.0);
      addValue("25", 25.0); 
      addValue("10", 10.0);            
      std::cerr << "WFallDynRangeComboBox added values\n";
      setSelection(100.0);
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
      std::cerr << "WFallSpanComboBox added values\n";
      setSelection(200.0);
    }
  }; 

#endif
