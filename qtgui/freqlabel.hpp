#ifndef FREQLABEL_HDR
#define FREQLABEL_HDR

#include <QLabel>
#include <QWidget>
#include <Qt>
#include <QMouseEvent>
#include <iostream>

class FreqLabel : public QLabel {
  Q_OBJECT
 
public:
  explicit FreqLabel(QWidget * parent = Q_NULLPTR,
		    Qt::WindowFlags f = Qt::WindowFlags());
  ~FreqLabel();


  double getFreq() const {
      return frequency; 
  }

signals:
  void newFreq(double freq);

public slots:
  void setFreq(double freq); 

protected:
  void mousePressEvent(QMouseEvent * event);

  double updateFrequency() {
      std::cerr << "int_freq = [" << int_freq << "] frac_freq = [" << frac_freq << "]" << std::endl;
      double dif = ((double) int_freq);
      double dff = 1e-6 * ((double) frac_freq);
      frequency = (dif + dff) * 1e6;
      std::cerr << "return freq = [" << frequency << "] dif = [" << dif << "] dff = [" << dff << "]" << std::endl;
      return frequency; 
  }
  
private:
  double frequency;  // in hz
  long int_freq, frac_freq; // in MHz. 

  QString freq2String();

  int incdec_position; 

  int disp_w, disp_h; // width and height of actual string.
}; 

#endif
