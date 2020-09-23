#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>

class QColor;
class QSizeF;
class QPointF;
class QwtPlotLegendItem;
class LegendItemMover;

// Just a test plot with dummy data to test the legend item mover
class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget *parent = NULL );

private:
    void insertCurve( const QString &title,
        const QColor &, const QPolygonF & );

    QwtPlotLegendItem *d_legendItem;
    LegendItemMover *d_legendItemMover;
};

#endif
