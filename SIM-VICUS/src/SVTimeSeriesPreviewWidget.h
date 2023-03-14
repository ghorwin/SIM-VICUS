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


#ifndef SVTimeSeriesPreviewWidgetH
#define SVTimeSeriesPreviewWidgetH

#include <QWidget>

#include <NANDRAD_LinearSplineParameter.h>

class QTextBrowser;
class QwtPlot;
class QwtPlotCurve;

/*! This widget takes data from a TSV-time-series and displays a preview diagram.
*/
class SVTimeSeriesPreviewWidget : public QWidget {
	Q_OBJECT
public:
	explicit SVTimeSeriesPreviewWidget(QWidget *parent = nullptr);
	~SVTimeSeriesPreviewWidget();

	/*! Updates internal data structure of widget and displayed data. */
	void setData(const NANDRAD::LinearSplineParameter & data, const std::string &xTitle, const std::string &yTitle);

	/*! Hides the diagram and shows the error label with the given text. */
	void setErrorMessage(const QString& errmsg);

private:
	/*! The chart, owned by widget. */
	QwtPlot								*m_chart;
	/*! The curve used to plot the preview data. */
	QwtPlotCurve						*m_curve = nullptr;
	/*! Shows error messages when reading of CCD files fails. */
	QTextBrowser						*m_errorTextBrowser;
	/*! Temporary copy of the data to be visualized. */
	NANDRAD::LinearSplineParameter		m_data;
};

#endif // SVTimeSeriesPreviewWidgetH
