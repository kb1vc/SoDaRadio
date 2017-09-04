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

#ifndef FREQLABEL_HDR
#define FREQLABEL_HDR

#include <QLabel>
#include <QWidget>
#include <Qt>
#include <QMouseEvent>
#include <iostream>

namespace GUISoDa {

  class FreqLabel : public QLabel {
    Q_OBJECT
 
  public:
    explicit FreqLabel(QWidget * parent = Q_NULLPTR,
		       Qt::WindowFlags f = Qt::WindowFlags());
    ~FreqLabel();


    double getFreq() const {
      return frequency; 
    }

    void setFreqUpdate(double freq) {
      setFreq(freq); 
      emit(newFreq(frequency));
    }
  signals:
    void newFreq(double freq);

  public slots:
    void setFreq(double freq); 

  protected:
    void mousePressEvent(QMouseEvent * event);

    double updateFrequency() {
      double dif = ((double) int_freq);
      double dff = 1e-6 * ((double) frac_freq);
      frequency = (dif + dff) * 1e6;
      return frequency; 
    }
  
  private:
    double frequency;  // in hz
    long int_freq, frac_freq; // in MHz. 

    QString freq2String();

    int incdec_position; 

    int disp_w, disp_h; // width and height of actual string.
  }; 
}

#endif
