#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>

class RectItem;
class QwtInterval;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget *parent, const QwtInterval & );
    virtual void updateLayout();

    void setRectOfInterest( const QRectF & );

Q_SIGNALS:
    void resized( double xRatio, double yRatio );

private:
    RectItem *d_rectOfInterest;
};

#endif
