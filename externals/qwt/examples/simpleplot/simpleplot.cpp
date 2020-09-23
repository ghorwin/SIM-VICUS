#include <QApplication>
#include <QwtPlot>         // oder <qwt_plot.h>
#include <QwtPlotCurve>    // oder <qwt_plot_curve.h>
#include <QwtPlotGrid>     // oder <qwt_plot_grid.h>
#include <QwtSymbol>       // oder <qwt_symbol.h>
#include <QwtLegend>       // oder <qwt_legend.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    // create plot as main widget
    QwtPlot plot;
    plot.setTitle( "Plot Demo" );
    plot.setCanvasBackground( Qt::white );
    QwtLegend * legend = new QwtLegend();
    plot.insertLegend( legend , QwtPlot::BottomLegend);

    // create a new curve to be shown in the plot and set some properties
    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setTitle( "Some Points" ); // will later be used in legend
    curve->setPen( Qt::blue, 4 ), // color and thickness in pixels
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // use antialiasing

    // data points
    QPolygonF points;
    points << QPointF( 0.0, 4.4 ) << QPointF( 1.0, 3.0 )
        << QPointF( 2.0, 4.5 ) << QPointF( 3.0, 6.8 )
        << QPointF( 4.0, 7.9 ) << QPointF( 5.0, 7.1 );

    // give some points to the curve
    curve->setSamples( points );

    // set the curve in the plot
    curve->attach( &plot );

    plot.setAxisScale( QwtPlot::yLeft, 0.0, 10.0 );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(Qt::DotLine));
    grid->attach( &plot );

    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
        QBrush( Qt::yellow ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
    curve->setSymbol( symbol );

    plot.resize( 600, 400 );
    plot.show();

    return a.exec();
}
