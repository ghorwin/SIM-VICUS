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

#include "SVLocalCoordinateView.h"
#include "ui_SVLocalCoordinateView.h"

#include <QDebug>
#include <QKeyEvent>

#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "SVStyle.h"
#include "SVProjectHandler.h"

SVLocalCoordinateView::SVLocalCoordinateView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVLocalCoordinateView)
{
	m_ui->setupUi(this);
	m_ui->horizontalLayout->setMargin(0);

	// make us known to the world
	SVViewStateHandler::instance().m_localCoordinateViewWidget = this;
}


SVLocalCoordinateView::~SVLocalCoordinateView() {
	delete m_ui;
}


void SVLocalCoordinateView::setCoordinates(const Vic3D::Transform3D &t) {

	// is being call from local coordinate system object, whenever this has changed location (regardless of
	// its own visibility)
	m_ui->lineEditXValue->setText( QString("%L1").arg( (double)t.translation().x(), 0, 'f', 3 ) );
	m_ui->lineEditYValue->setText( QString("%L1").arg( (double)t.translation().y(), 0, 'f', 3 ) );
	m_ui->lineEditZValue->setText( QString("%L1").arg( (double)t.translation().z(), 0, 'f', 3 ) );
}

void SVLocalCoordinateView::clearCoordinates() {
	m_ui->lineEditXValue->setText( QString("%L1").arg( 0.0, 0, 'f', 3 ) );
	m_ui->lineEditYValue->setText( QString("%L1").arg( 0.0, 0, 'f', 3 ) );
	m_ui->lineEditZValue->setText( QString("%L1").arg( 0.0, 0, 'f', 3 ) );
}

void SVLocalCoordinateView::setBoundingBoxDimension(const IBKMK::Vector3D& bb) {
	// is being call from local coordinate system object, whenever this has changed location (regardless of
	// its own visibility)
	m_ui->lineEditBoundingBoxDimensionX->setText( QString("%L1").arg( bb.m_x, 0, 'f', 3 ) );
	m_ui->lineEditBoundingBoxDimensionY->setText( QString("%L1").arg( bb.m_y, 0, 'f', 3 ) );
	m_ui->lineEditBoundingBoxDimensionZ->setText( QString("%L1").arg( bb.m_z, 0, 'f', 3 ) );
}

void SVLocalCoordinateView::setAlignCoordinateSystemButtonChecked(bool checked) {
	m_ui->toolButtonAlignCoordinateSystem->setChecked(checked);
}


void SVLocalCoordinateView::setMoveCoordinateSystemButtonChecked(bool checked) {
	m_ui->toolButtonMoveCoordinateSystem->setChecked(checked);
}


void SVLocalCoordinateView::on_toolButtonAlignCoordinateSystem_clicked() {
	SVGeometryView * geoView = SVViewStateHandler::instance().m_geometryView;
	QKeyEvent e(QKeyEvent::KeyPress, Qt::Key_F4, Qt::NoModifier);
	geoView->handleGlobalKeyPressEvent(&e);
}


void SVLocalCoordinateView::on_toolButtonMoveCoordinateSystem_clicked() {
	SVGeometryView * geoView = SVViewStateHandler::instance().m_geometryView;
	QKeyEvent e(QKeyEvent::KeyPress, Qt::Key_F5, Qt::NoModifier);
	geoView->handleGlobalKeyPressEvent(&e);
}


void SVLocalCoordinateView::on_toolButtonInformation_clicked() {
	showInformation();
}


void SVLocalCoordinateView::showInformation() {
	// update our selection lists
	std::set<const VICUS::Object*> sel;

	// first we get how many surfaces are selected
	project().selectObjects(sel, VICUS::Project::SG_All, true, true);

	std::vector<const VICUS::Surface*>			selSurfaces;
	std::vector<const VICUS::SubSurface*>		selSubSurfaces;
	// we also have to cache all existing names, so we take alle existing objects
	selSurfaces.clear();
	selSubSurfaces.clear();

	unsigned int numSurfs = 0;
	unsigned int numSubSurfs = 0;

	double areaSurfs = 0;
	double areaSubSurfs = 0;

	std::set<unsigned int> handledSubSurfsIds;

	// process all selected objects and sort them into vectors
	for (const VICUS::Object * o : sel) {
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			if (s->m_selected && s->m_visible) {
				areaSurfs += s->geometry().area(2);
				++numSurfs;
				for (const VICUS::SubSurface &ss : s->subSurfaces()) {
					if(handledSubSurfsIds.find(ss.m_id) != handledSubSurfsIds.end())
						continue;

					// we need to calculate the 3D Points of the Sub Surface
					std::vector<IBKMK::Vector3D> subSurf3D;
					for (unsigned int i=0; i<ss.m_polygon2D.vertexes().size(); ++i) {
						const IBKMK::Vector2D &vertex = ss.m_polygon2D.vertexes()[i];
						subSurf3D.push_back(s->geometry().offset() + s->geometry().localX()*vertex.m_x
											+ s->geometry().localY()*vertex.m_y);
					}
					IBKMK::Polygon3D poly(subSurf3D);

					double area = poly.polyline().area(2);
					areaSurfs -= area;

					if(ss.m_visible && ss.m_selected) {
						handledSubSurfsIds.insert(ss.m_id);
						areaSubSurfs += area;
						++numSubSurfs;
					}
					//			qDebug() << i << "\t" << subSurf3D[i].m_x << "\t" << subSurf3D[i].m_y << "\t" << subSurf3D[i].m_z;
				}
			}
		}
		const VICUS::SubSurface * ss = dynamic_cast<const VICUS::SubSurface *>(o);
		if (ss != nullptr) {
			if (ss->m_selected && ss->m_visible) {
				if(handledSubSurfsIds.find(ss->m_id) != handledSubSurfsIds.end())
					continue;

				VICUS::Surface *surf = dynamic_cast<VICUS::Surface*>(ss->m_parent);
				if (surf == nullptr)
					continue;

				// we need to calculate the 3D Points of the Sub Surface
				std::vector<IBKMK::Vector3D> subSurf3D;
				for (unsigned int i=0; i<ss->m_polygon2D.vertexes().size(); ++i) {


					const IBKMK::Vector2D &vertex = ss->m_polygon2D.vertexes()[i];
					subSurf3D.push_back(surf->geometry().offset() + surf->geometry().localX()*vertex.m_x
										+ surf->geometry().localY()*vertex.m_y);
				}
				IBKMK::Polygon3D poly(subSurf3D);

				double area = poly.polyline().area(2);
				if(surf->m_visible && surf->m_selected)
					areaSurfs -= area;
				handledSubSurfsIds.insert(ss->m_id);
				areaSubSurfs += area;

				++numSubSurfs;
				//			qDebug() << i << "\t" << subSurf3D[i].m_x << "\t" << subSurf3D[i].m_y << "\t" << subSurf3D[i].m_z;
			}
		}
	}
	QString surfaceInfo = QString ("Selected surfaces:\t%1\tArea:\t%2 m²\nSelected sub-surfaces:\t%3\tArea:\t%4 m²")
			.arg(numSurfs).arg(areaSurfs).arg(numSubSurfs).arg(areaSubSurfs);
	QMessageBox msg(QMessageBox::Information, tr("Selection Information"), surfaceInfo, QMessageBox::Ok, this);
	msg.exec();

}
