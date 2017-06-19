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

  void setFreq(double freq); 

  double getFreq() const {
      std::cerr << "int_freq = [" << int_freq << "] frac_freq = [" << frac_freq << "]" << std::endl;
      double dif = ((double) int_freq);
      double dff = 1e-6 * ((double) frac_freq);
      double ret = dif + dff;
      std::cerr << "return freq = [" << ret << "] dif = [" << dif << "] dff = [" << dff << "]" << std::endl;
      return ret;
  }

signals:
  void newFreq(double freq);
  
protected:
  void mousePressEvent(QMouseEvent * event);
  
private:
  long int_freq, frac_freq;

  QString freq2String();

  int incdec_position; 

  int disp_w, disp_h; // width and height of actual string.
}; 

#endif
