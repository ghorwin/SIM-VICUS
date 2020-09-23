#include "plot.h"
#include <qwt_plot_legenditem.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "legenditemmover.h"

Plot::Plot( QWidget *parent ):
    QwtPlot( parent )
{
    setAutoReplot( false );

    setTitle( "Movable internal legend" );

    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setFrameStyle( QFrame::Box);
    canvas->setLineWidth(1);

    QPalette pal = canvas->palette();
    pal.setBrush( QPalette::Window, Qt::white );
    canvas->setPalette(pal);
    setCanvas( canvas );

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    grid->setMajorPen( Qt::gray, 0, Qt::DashDotLine );
    grid->setMinorPen( Qt::lightGray, 0 , Qt::DotLine );
    grid->attach( this );

    // axes
    setAxisTitle( QwtPlot::xBottom, "Time [min]" );
    setAxisTitle( QwtPlot::yLeft, "Temperature [C]" );

    // curves

    QPolygonF points1;
    points1 << QPointF( 0.2, 4.4 ) << QPointF( 1.2, 3.0 )
        << QPointF( 2.7, 4.5 ) << QPointF( 3.5, 6.8 )
        << QPointF( 4.7, 7.9 ) << QPointF( 5.8, 7.1 );

    insertCurve( "Curve 1", "Navy", points1 );

    QPolygonF points2;
    points2 << QPointF( 0.4, 8.7 ) << QPointF( 1.4, 7.8 )
        << QPointF( 2.3, 5.5 ) << QPointF( 3.3, 4.1 )
        << QPointF( 4.4, 5.2 ) << QPointF( 5.6, 5.7 );

    insertCurve( "Curve 2", "DodgerBlue", points2 );

    const int margin = 5;
    setContentsMargins( margin, margin, margin, margin );

    setAutoFillBackground( true );


    // legend item

    d_legendItem = new QwtPlotLegendItem();
    d_legendItem->setBackgroundBrush( QColor(255,220,194) );
    d_legendItem->setBorderPen(QPen(Qt::black));
    d_legendItem->setMaxColumns(1);
    d_legendItem->attach( this );

    // *** Add the legend item mover ***
    // The constructor will automatically register the LegendItemMover
    // as event filter for the plot.
    d_legendItemMover = new LegendItemMover( this );
}


void Plot::insertCurve( const QString &title,
    const QColor &color, const QPolygonF &points )
{
    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setTitle( title );
    curve->setPen( color, 1 ),
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Rect,
        QBrush( Qt::white ), QPen( color, 0 ), QSize( 8, 8 ) );
    curve->setSymbol( symbol );

    curve->setSamples( points );

    curve->attach( this );
}

