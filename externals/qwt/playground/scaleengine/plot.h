#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>

class QwtTransform;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget *parent = NULL );

public Q_SLOTS:
    void setTransformation( QwtTransform * );

private:
    void populate();
};

#endif
