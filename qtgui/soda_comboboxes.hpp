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
      std::cerr << "WFallDynRangeComboBox added values\n";
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
      std::cerr << "WFallSpanComboBox added values\n";
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
