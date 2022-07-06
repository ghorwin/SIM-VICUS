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

#ifndef SVLocalCoordinateViewH
#define SVLocalCoordinateViewH

#include <QWidget>

#include <Vic3DTransform3D.h>

#include <IBKMK_Vector3D.h>

namespace Ui {
class SVLocalCoordinateView;
}

/*! A widget that shows the location and orientation of the local coordinate system. */
class SVLocalCoordinateView : public QWidget {
	Q_OBJECT
public:
	explicit SVLocalCoordinateView(QWidget *parent = nullptr);
	~SVLocalCoordinateView();

	/*! Sets the Coordinates of the Center Point of the local Coordinate System. */
	void setCoordinates(const Vic3D::Transform3D &t);
	/*! Sets the Dimension of Bounding Box Dimension. */
	void setBoundingBoxDimension(const IBKMK::Vector3D& bb);

	/*! Called from GeometryView when view state changes. */
	void setAlignCoordinateSystemButtonChecked(bool checked);
	/*! Called from GeometryView when view state changes. */
	void setMoveCoordinateSystemButtonChecked(bool checked);

public slots:
	/*! Connected to preference dialog and is called when style is changed from dark to white theme. */
	void onStyleChanged();

private slots:
	void on_toolButtonAlignCoordinateSystem_clicked();
    void on_toolButtonMoveCoordinateSystem_clicked();
    void on_toolButtonInformation_clicked();

private:
    /*! Shows Information of selected surfaces and subsurfaces. */
    void showInformation();

	Ui::SVLocalCoordinateView *m_ui;

	QVector3D				m_xAxis;
	QVector3D				m_yAxis;
	QVector3D				m_zAxis;
};

#endif // SVLocalCoordinateViewH
