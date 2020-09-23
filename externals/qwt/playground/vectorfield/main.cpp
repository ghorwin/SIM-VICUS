#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_vectorfield.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qpen.h>
#include <qbrush.h>
#include <cmath>

class VectorField: public QwtPlotVectorField
{
public:
    VectorField():
        QwtPlotVectorField( "Vector Field" )
    {
        setRenderHint( QwtPlotItem::RenderAntialiased, true );
        setLegendIconSize( QSize( 20, 10 ) );

        setPen( Qt::NoPen );
        setBrush( Qt::black );

        // a magnitude of 1.0 becomes 5 times the width of the tail
        setMagnitudeScaleFactor( 1.0 );
#if 1
        setIndicatorOrigin( QwtPlotVectorField::OriginHead );
#else
        setIndicatorOrigin( QwtPlotVectorField::OriginTail );
#endif

#if 0
        setRasterSize( QSizeF( 20, 20 ) );
#endif
        setSamples( samples() );
    }

private:
    QVector<QwtVectorFieldSample> samples() const
    {
        const int dim = 10;

        QVector<QwtVectorFieldSample> samples;

        for ( int x = -dim; x < dim; x++ )
        {
            for ( int y = -dim; y < dim; y++ )
            {
                samples += QwtVectorFieldSample( x, y, y, -x );
            }
        }

        return samples;
    }
};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QwtPlot plot;
    plot.setTitle( "Vector Field" );
    plot.setCanvasBackground( Qt::white );

    plot.insertLegend( new QwtLegend() );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach( &plot );

    VectorField *fieldItem = new VectorField();
    fieldItem->attach( &plot );

    const QRectF r = fieldItem->boundingRect();

#if 1
    plot.setAxisScale( QwtPlot::xBottom, r.left() - 1.0, r.right() + 1.0 );
#else
    plot.setAxisScale( QwtPlot::xBottom, r.right() + 1.0, r.left() - 1.0 );
#endif

#if 1
    plot.setAxisScale( QwtPlot::yLeft, r.top() - 1.0, r.bottom() + 1.0 );
#else
    plot.setAxisScale( QwtPlot::yLeft, r.bottom() + 1.0, r.top() - 1.0 );
#endif

    plot.resize( 600, 400 );
    plot.show();

    return a.exec();
}
