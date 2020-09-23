#ifndef PLOT_H
#define PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    enum ColorMap
    {
        RGBMap,
        HueMap,
        SaturationMap,
        ValueMap,
        SVMap,
        AlphaMap
    };

    Plot( QWidget * = NULL );

Q_SIGNALS:
    void rendered( const QString& status );

public Q_SLOTS:
    void showContour( bool on );
    void showSpectrogram( bool on );

    void setColorMap( int );
    void setColorTableSize( int );
    void setAlpha( int );

#ifndef QT_NO_PRINTER
    void printPlot();
#endif

private:
    virtual void drawItems( QPainter *, const QRectF &,
        const QwtScaleMap maps[axisCnt] ) const;

    QwtPlotSpectrogram *d_spectrogram;

    int d_mapType;
    int d_alpha;
};

#endif
