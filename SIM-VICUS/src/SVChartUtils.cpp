/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

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


