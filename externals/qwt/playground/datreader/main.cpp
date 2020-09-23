#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_vectorfield.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_legend.h>
#include <qpen.h>
#include <qbrush.h>
#include <qshortcut.h>

#include "DatFileReader.h"

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

        setMagnitudeScaleFactor( 0.02 );
        setRasterSize( QSizeF( 10, 10 ) );
#if 1
        setIndicatorOrigin( QwtPlotVectorField::OriginHead );
#else
        setIndicatorOrigin( QwtPlotVectorField::OriginTail );
#endif

        setSamples( samples() );
        setRasterSize( QSizeF( 10, 10 ) );
    }

private:
    QVector<QwtVectorFieldSample> samples() const
    {
        DatFileReader reader;
        reader.read("/disk0/project/qwt/qwt-git/playground/datreader/Heatflux_elements_2Dvector.dat");

        QVector<QwtVectorFieldSample> samples;

        for ( uint i = 0; i < reader.x.size(); i++ )
        {
            samples += QwtVectorFieldSample( 
                reader.x[i], reader.y[i], reader.u[i], reader.v[i] );
        }

        return samples;
    }
};

class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot( QWidget *parent = NULL ):
        QwtPlot( parent )
    {
        setTitle( "Vector Field" );
        setCanvasBackground( Qt::white );
        insertLegend( new QwtLegend() );

        QwtPlotGrid *grid = new QwtPlotGrid();
        grid->attach( this );

        d_vectorField = new VectorField();
        d_vectorField->attach( this );
    }

private Q_SLOTS:
    void toggleFilterMode()
    {
        const bool on = d_vectorField->testPaintAttribute( 
            QwtPlotVectorField::FilterVectors );

        d_vectorField->setPaintAttribute(
            QwtPlotVectorField::FilterVectors, !on );

        replot();
    }

private:
    VectorField *d_vectorField;
};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    Plot plot;

    // navigation

    QwtPlotZoomer *zoomer = new QwtPlotZoomer( plot.canvas() );
    zoomer->setRubberBandPen( QPen( Qt::red ) );

    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
        Qt::RightButton );

    QwtPlotPanner *panner = new QwtPlotPanner( plot.canvas() );
    panner->setAxisEnabled( QwtPlot::yRight, false );
    panner->setMouseButton( Qt::MidButton );

    plot.resize( 600, 400 );
    plot.show();

    (void) new QShortcut( Qt::CTRL + Qt::Key_R,
        &plot, SLOT( toggleFilterMode() ) );

    return a.exec();
}

#include "main.moc"
