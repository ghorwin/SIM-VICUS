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

#ifndef SVMeasurementWidgetH
#define SVMeasurementWidgetH

#include <QWidget>
#include <QVector3D>

class SVPreferencesDialog;

namespace Ui {
	class SVMeasurementWidget;
}

namespace Vic3D {
	class MeasurementObject;
};

/*! This is the measurement widget which show all data needed for measurements inside the scene. */
class SVMeasurementWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVMeasurementWidget(QWidget *parent = nullptr);
	~SVMeasurementWidget();

	/*! Resets all line edits in measurement widget. */
	void reset();

	/*! Moves the widget according to the point position and maps it to the parent widgets position.
		\param position The bottom-right corner position of the widget.
	*/
	void setPosition(const QPoint &position);

	/*! Sets the start point based on the measurement object. */
	void showStartPoint(const QVector3D &sp);
	/*! Sets the end point based on the measurement object. */
	void showEndPoint(const QVector3D &ep);

	/*! Updates the measurement data and switches between local/global. */
	void showMeasurement();

	void setLocalAxes(const QVector3D &xAxis, const QVector3D &yAxis, const QVector3D &zAxis);

	/*! Color of measurementLine. */
	QColor							m_color = Qt::red;

	/*! Local X Axis */
	QVector3D						m_localXAxis;

	/*! Local Y Axis */
	QVector3D						m_localYAxis;

	/*! Local Z Axis */
	QVector3D						m_localZAxis;


private slots:
	void on_pushButtonCopyInformation_clicked();

	void on_pushButtonColor_colorChanged();

	void on_checkBoxLocalMeasurement_toggled(bool);

private:
	Ui::SVMeasurementWidget			*m_ui;

	/*! Cached value of start point, updated in showStartPoint. */
	QVector3D						m_startPoint;
	/*! Cached value of start point, updated in showEndPoint. */
	QVector3D						m_endPoint;



	/*! User preferences. */
	SVPreferencesDialog				*m_preferencesDialog									= nullptr;
};

#endif // SVMeasurementWidgetH
