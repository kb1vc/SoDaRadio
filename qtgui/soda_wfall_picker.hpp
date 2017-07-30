#ifndef SODAWFALL_PICKER_H
#define SODAWFALL_PICKER_H
#include <QBrush>
#include <QColor>
#include <QPen>
#include <qwt/qwt_picker_machine.h>
#include <boost/format.hpp>

class SoDaWFallPicker : public QwtPlotPicker {
public:
    SoDaWFallPicker(int xAxis, int yAxis, QWidget * canvas) : QwtPlotPicker(xAxis, yAxis, canvas)
    {
        setStateMachine(new QwtPickerClickPointMachine);
        setTrackerMode(QwtPicker::AlwaysOn);
        setTrackerPen(QPen(Qt::white));
    }

    QwtText trackerTextF(const QPointF & pos) const {
        QColor trbgcolor(Qt::black);
        trbgcolor.setAlpha(200); // translucent highlight

        QwtText text((boost::format("%'.4f MHz") % (pos.x() * 1e-6)).str().c_str());
        text.setBackgroundBrush(QBrush(trbgcolor));

        return text;
    }
};

#endif // SODAWFALL_PICKER_H
