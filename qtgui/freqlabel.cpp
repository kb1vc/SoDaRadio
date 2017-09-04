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

#include <cmath>
#include "freqlabel.hpp"
#include <iostream>

using namespace GUISoDa; 

GUISoDa::FreqLabel::FreqLabel(QWidget * parent,
		     Qt::WindowFlags f)
  : QLabel(parent), incdec_position(2)
{
    (void) f;
  setFreq(144.295e6);

  setTextFormat(Qt::RichText);
}

GUISoDa::FreqLabel::~FreqLabel() {}

QString GUISoDa::FreqLabel::freq2String() {
  QString ret; 
  int num_digs; 
  unsigned long ifrac = frac_freq;
  // start at the left hand side
  bool first_nonzero = false; 
  for(num_digs = 0; num_digs < 6; num_digs++) {
    int dig = ifrac % 10;
    ifrac = ifrac / 10;
    if(num_digs == 3) ret = " " + ret; 
    if(incdec_position == num_digs) {
      ret = "<u>" + QString::number(dig) + "</u>" + ret;      
    }
    else {
      ret = QString::number(dig) + ret;
    }
  }

  // add the decimal point
  ret = "." + ret; 

  unsigned long ifreq = int_freq;
  for(; num_digs < 11; num_digs++) {
    int dig = ifreq % 10;
    ifreq = ifreq / 10;

    if(((num_digs % 3) == 0) && (num_digs > 6)) {
      ret = "," + ret; 
    }
    if(incdec_position == num_digs) {
      ret = "<u>" + QString::number(dig) + "</u>" + ret;      
    }
    else {
      ret = QString::number(dig) + ret;
    }   
  }

  while(ret.size() < 14) {
      ret = " " + ret;
  }
  return ret; 
}

void GUISoDa::FreqLabel::setFreq(double hzfreq)
{
  double freq = hzfreq * 1e-6; 
  frequency = hzfreq; 

  int_freq = lround(floor(freq));
  double fr = freq - floor(freq);
  frac_freq = lround(fr * 1e6);
  QString flab = freq2String();

  setText(flab);

  // now set the text width/height
  QRect r = fontMetrics().boundingRect(flab);
  disp_w = r.width();
  disp_h = r.height();
}

void GUISoDa::FreqLabel::mousePressEvent(QMouseEvent * event) 
{
    int px = event->x();
    int py = event->y();
    int wh = height();
    int ww = width();

    unsigned long incr = 0;
    if(event->button() == Qt::LeftButton) {
        if(py > (wh >> 1)) {
            incr = incr - 1;
        }
        else if(py < (wh >> 1)) {
            incr = incr + 1;
        }
    }

    if(event->button() == Qt::RightButton) {
        if(px < (ww >> 1)) {
            incdec_position += 1;
            if(incdec_position > 10) incdec_position = 10;
        }
        else if(px > (ww >> 1)) {
            incdec_position -= 1;
            if(incdec_position < 0) incdec_position = 0;
        }
    }

    if(incr != 0) {
        if(incdec_position < 6) {
            for(int i = 0; i < incdec_position; i++) {
                incr = incr * 10;
            }
            frac_freq += incr;
        }
        else {
            for(int i = 6; i < incdec_position; i++) {
                incr = incr * 10;
            }
            int_freq += incr;
            if(int_freq < 0) {
                int_freq -= incr;
                // move adj to right
                incdec_position -= 1;
            }
        }
    }
    if(frac_freq >= 1000000) {
        frac_freq -= 1000000;
        int_freq += 1;
    }
    if(frac_freq < 0) {
        frac_freq += 1000000;
        int_freq -= 1;
    }

    if(int_freq > 99999) int_freq = 99999;
    if(int_freq < 0) int_freq = 0;

    setFreq(updateFrequency());

    emit newFreq(frequency);
}
