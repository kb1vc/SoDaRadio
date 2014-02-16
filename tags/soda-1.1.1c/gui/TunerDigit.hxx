#ifndef TUNER_HDR
#define TUNER_HDR
/*
  Copyright (c) 2012,2013,2014 Matthew H. Reilly (kb1vc)
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

#include <iostream>
namespace SoDaRadio_GUI {
  class TunerDigit {
  public:
    TunerDigit(double * _freqp, int _digit_pos, TunerDigit * _next, wxTextCtrl * dtxt) {
      freqp = _freqp;
      pos = _digit_pos;
      next = _next;
      digtxt = dtxt; 

      int i;
      decade = 1.0; 
      for(i = 0; i < pos; i++) {
	decade *= 10.0; 
      }
    
      freq2Val(*freqp); 
    }
    void setNext(TunerDigit * _next) {
      next = _next; 
    }
    void setLast(TunerDigit * _last) {
      last = _last; 
    }
    void up()
    {
      if(curval >= 9) {
	curval = 0;
	if(next != NULL) next->up(); 
      }
      else {
	curval++; 
      }
      setVal(); 
    }
  
    void down()
    {
      if(curval == 0) {
	curval = 9;
	if(next != NULL) next->down(); 
      }
      else {
	curval--; 
      }
      setVal(); 
    }

    void update(double acc) {
      // go to the last (0) digit and build the
      // frequency value.
      acc += decade * ((double) curval);
      if(next == NULL) {
	*freqp = acc; 
      }
      else {
	next->update(acc);
      }
    }

    void newFreq() {
      freq2Val(*freqp);
      if(next != NULL) next->newFreq(); 
    }
  
  private:
    void freq2Val(double dfreq) {
      unsigned long ifreq;

      int i = pos;
      while(i != 0) {
	i--;
	dfreq = dfreq / 10; 
      }
      ifreq = (unsigned long) dfreq;
      curval = ifreq % 10;
      setVal(); 
    }
  
    void setVal() {
      // turn the digit to a string
      static wxChar cmap[] = { wxChar('0'), wxChar('1'), wxChar('2'), wxChar('3'), 
			       wxChar('4'), wxChar('5'), wxChar('6'), wxChar('7'), 
			       wxChar('8'), wxChar('9') };
      digtxt->SetValue(cmap[curval % 10]);
    }
  
    double * freqp; 
    int pos;
    double decade; 
    TunerDigit * last, * next;
    unsigned char curval;
    wxTextCtrl * digtxt; 
  }; 
}
#endif
