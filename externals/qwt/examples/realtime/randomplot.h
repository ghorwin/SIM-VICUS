#ifndef RANDOM_PLOT_H
#define RANDOM_PLOT_H

#include "incrementalplot.h"
#include <qdatetime.h>

class QTimer;

class RandomPlot: public IncrementalPlot
{
    Q_OBJECT

public:
    RandomPlot( QWidget *parent );

    virtual QSize sizeHint() const;

Q_SIGNALS:
    void running( bool );
    void elapsed( int ms );

public Q_SLOTS:
    void clear();
    void stop();
    void append( int timeout, int count );

private Q_SLOTS:
    void appendPoint();

private:
    void initCurve();

    QTimer *d_timer;
    int d_timerCount;

    QTime d_timeStamp;
};

#endif
