#ifndef SODAPLOTPICKER_H
#define SODAPLOTPICKER_H
#include <QBrush>
#include <QColor>
#include <QPen>
#include <qwt/qwt_picker_machine.h>
#include <boost/format.hpp>

class SoDaPlotPicker : public QwtPlotPicker {
public:
    SoDaPlotPicker(int xAxis, int yAxis, QWidget * canvas) : QwtPlotPicker(xAxis, yAxis, canvas)
    {
        setStateMachine(new QwtPickerClickPointMachine);
        setTrackerMode(QwtPicker::AlwaysOn);
        setTrackerPen(QPen(Qt::white));
    }

    QwtText trackerTextF(const QPointF & pos) const {
        QColor trbgcolor(Qt::white);
        trbgcolor.setAlpha(128); // translucent highlight

        QwtText text((boost::format("%.4f MHz, %.1f dB") % (pos.x() * 1e-6) % pos.y()).str().c_str());
        text.setBackgroundBrush(QBrush(trbgcolor));

        return text;
    }
};

#endif // SODAPLOTPICKER_H
