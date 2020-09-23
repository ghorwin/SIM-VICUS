#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <qmainwindow.h>

class QwtPlotRescaler;
class QLabel;
class Plot;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    enum RescaleMode
    {
        KeepScales,
        Fixed,
        Expanding,
        Fitting
    };

    MainWindow();

private Q_SLOTS:
    void setRescaleMode( int );
    void showRatio( double, double );

private:
    QWidget *createPanel( QWidget * );
    Plot *createPlot( QWidget * );

    QwtPlotRescaler *d_rescaler;
    QLabel *d_rescaleInfo;

    Plot *d_plot;
};

#endif
