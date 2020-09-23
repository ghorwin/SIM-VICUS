#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>

class QPolygonF;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget * = NULL );

private:
    void insertCurve( const QString &title, 
        const QColor &, const QPolygonF & );
};

#endif

