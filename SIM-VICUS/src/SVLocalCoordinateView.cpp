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

#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "SVStyle.h"

SVLocalCoordinateView::SVLocalCoordinateView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVLocalCoordinateView)
{
	m_ui->setupUi(this);
	m_ui->horizontalLayout->setMargin(0);

	// make us known to the world
	SVViewStateHandler::instance().m_localCoordinateViewWidget = this;

	// to set colors on start
	onStyleChanged();
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


void SVLocalCoordinateView::setAlignCoordinateSystemButtonChecked(bool checked) {
	m_ui->toolButtonAlignCoordinateSystem->setChecked(checked);
}


void SVLocalCoordinateView::setMoveCoordinateSystemButtonChecked(bool checked) {
	m_ui->toolButtonMoveCoordinateSystem->setChecked(checked);
}

void SVLocalCoordinateView::onStyleChanged() {
	SVStyle::instance().formatLineEditReadOnly(m_ui->lineEditXValue);
	SVStyle::instance().formatLineEditReadOnly(m_ui->lineEditYValue);
	SVStyle::instance().formatLineEditReadOnly(m_ui->lineEditZValue);
}


void SVLocalCoordinateView::on_toolButtonAlignCoordinateSystem_clicked() {
	SVGeometryView * geoView = SVViewStateHandler::instance().m_geometryView;
	geoView->handleGlobalKeyPress(Qt::Key_F4);
}

void SVLocalCoordinateView::on_toolButtonMoveCoordinateSystem_clicked() {
	SVGeometryView * geoView = SVViewStateHandler::instance().m_geometryView;
	geoView->handleGlobalKeyPress(Qt::Key_F5);
}
