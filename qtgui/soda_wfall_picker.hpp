#pragma once
/*
Copyright (c) 2017,2023 Matthew H. Reilly (kb1vc)
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
#include <QBrush>
#include <QColor>
#include <QPen>
#include <qwt/qwt_text.h>
#include <qwt/qwt_picker_machine.h>
namespace GUISoDa {

  class WFallPickerClient {
  public:
    virtual void handleMouseEvent(const QMouseEvent * ev) = 0; 
  };
  
  class SoDaWFallQwtPickerClickPointMachine : public QwtPickerClickPointMachine {
  public:
    SoDaWFallQwtPickerClickPointMachine(WFallPickerClient * client) {
      picker_client = client; 
    }
    QList<QwtPickerMachine::Command> transition(const QwtEventPattern & event_pattern, const QEvent * e) {
      QList<QwtPickerMachine::Command> cmdList; 
      if(e->type() == QEvent::MouseButtonPress) {
	const QMouseEvent * m_e = static_cast<const QMouseEvent *>(e); 
	if(event_pattern.mouseMatch(QwtEventPattern::MouseSelect2, m_e)) {
	    // if(m_e->button() == Qt::RightButton) {
	  picker_client->handleMouseEvent(m_e);
	  cmdList += Begin;
	  cmdList += Append;
	  cmdList += End;
	  return cmdList; 	  
	}
      }

      // if we get there, this was not a QtRightButton press. 
      return QwtPickerClickPointMachine::transition(event_pattern, e);  
    }

  protected:
    WFallPickerClient * picker_client;
    
  }; 
     
  class WFallPicker : public QwtPlotPicker {
  public:
    WFallPicker(int xAxis, int yAxis, QWidget * canvas, WFallPickerClient * client) : QwtPlotPicker(xAxis, yAxis, canvas)
    {
      setStateMachine(new SoDaWFallQwtPickerClickPointMachine(client));
      setTrackerMode(QwtPicker::AlwaysOn);
      setTrackerPen(QPen(Qt::white));
    }

    QwtText trackerTextF(const QPointF & pos) const {
      QColor trbgcolor(Qt::black);
      trbgcolor.setAlpha(200); // translucent highlight

      QString str = QString("%1 MHz").arg(pos.x() * 1e-6, 0, 'f', 4);

      QwtText text(str);
      text.setBackgroundBrush(QBrush(trbgcolor));

      return text;
    }
  };
}
