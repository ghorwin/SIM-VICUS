#ifndef BAR_CHART_H
#define BAR_CHART_H

#include <qwt_plot.h>
#include <qstringlist.h>

class DistroChartItem;

class BarChart: public QwtPlot
{
    Q_OBJECT

public:
    BarChart( QWidget * = NULL );

public Q_SLOTS:
    void setOrientation( int );
    void exportChart();
    void doScreenShot();

private:
    void populate();
    void exportPNG( int width, int height );
    void render( QPainter* painter, const QRectF & targetRect );

    DistroChartItem *d_barChartItem;
    QStringList d_distros;
};

#endif
