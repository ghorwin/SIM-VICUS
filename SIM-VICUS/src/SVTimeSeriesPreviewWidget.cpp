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

#include "SVTimeSeriesPreviewWidget.h"

#include <QVBoxLayout>
#include <QTextBrowser>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "SVSettings.h"
#include "SVChartUtils.h"

SVTimeSeriesPreviewWidget::SVTimeSeriesPreviewWidget(QWidget *parent) : QWidget(parent) {
	// create layout and elements of layout
	QVBoxLayout *layout = new QVBoxLayout(this);

	// create chart and curve
	m_chart = new QwtPlot(this);
	configureChart(m_chart);

	m_curve = addConfiguredCurve(m_chart);

	m_errorTextBrowser = new QTextBrowser(this);

	// insert plot and button for update
	layout->addWidget(m_chart);
	layout->addWidget(m_errorTextBrowser);
	m_errorTextBrowser->setVisible(false);
	this->setLayout(layout);
}


SVTimeSeriesPreviewWidget::~SVTimeSeriesPreviewWidget(){
	delete m_curve;
}


void SVTimeSeriesPreviewWidget::setErrorMessage(const QString& errmsg) {
	m_chart->setVisible(false);
	m_errorTextBrowser->setVisible(true);
	m_errorTextBrowser->setText(errmsg);
}


void SVTimeSeriesPreviewWidget::setData(const NANDRAD::LinearSplineParameter & data) {

	// we need valid data, otherwise we clear the chart and show an error message
	try {
		m_data = data;
		m_data.checkAndInitialize(data.m_name, data.m_xUnit, data.m_yUnit,
								data.m_yUnit, std::numeric_limits<double>::lowest(), true, std::numeric_limits<double>::max(), true,
								"", true);
	} catch (IBK::Exception & ex) {
		// TODO : error message from checkAndInitialize is not translated... we may need a Qt-based check-and-initialize function
		setErrorMessage(tr("Invalid data: %1").arg(ex.what()));
		return;
	}
	m_chart->setVisible(true);
	m_errorTextBrowser->setVisible(false);

	m_curve->setRawSamples(m_data.m_values.x().data(), m_data.m_values.y().data(), (int)m_data.m_values.size());
	m_chart->replot();
}

