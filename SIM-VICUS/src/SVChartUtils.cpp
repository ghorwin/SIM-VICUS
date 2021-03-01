#include "SVChartUtils.h"


#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>

void configureChart(QwtPlot * plot) {

	// title

	QwtText title;
	plot->setTitle(title);

	// background

	QwtPlotCanvas * canvas = new QwtPlotCanvas(plot);
	canvas->setPalette(Qt::white);
	canvas->setFrameStyle(QFrame::Box | QFrame::Plain );
	canvas->setLineWidth(1);
	plot->setCanvas(canvas);

	// axes

	plot->setAxisScale( QwtPlot::yLeft, 0.0, 10.0 );
	plot->setAxisScale( QwtPlot::xBottom, 0.0, 7.0 ); // 7 days

	// grid

	QwtPlotGrid *grid = new QwtPlotGrid();
	grid->setMajorPen(QPen(Qt::DotLine));
	grid->attach( plot );

	// no legend

	// lines
	// add curves
	QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");
	curve1->setTitle( "Some Points" ); // will later be used in legend
	curve1->setPen( Qt::blue, 1 ); // color and thickness in pixels
	curve1->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // use antialiasing

	QwtPlotCurve *curve2 = new QwtPlotCurve("Curve 2");

	// connect or copy the data to the curves
	QPolygonF points;
	curve1->setSamples(points);
	curve2->setSamples(points);

	curve1->attach(plot);
	curve2->attach(plot);
}


