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

#include "SVPropAddGeometry.h"
#include "ui_SVPropAddGeometry.h"

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "SVPropVertexListWidget.h"
#include "Vic3DNewGeometryObject.h"
#include "SVPropAddWindowWidget.h"
#include "Vic3DCoordinateSystemObject.h"

SVPropAddGeometry::SVPropAddGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropAddGeometry)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutMaster->setMargin(0);
	//	m_ui->verticalLayoutPage1->setMargin(0);
	//	m_ui->verticalLayoutPage2->setMargin(0);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified, this, &SVPropAddGeometry::onModified);
}


SVPropAddGeometry::~SVPropAddGeometry() {
	delete m_ui;
}


void SVPropAddGeometry::onModified(int modificationType, ModificationInfo *) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::NodeStateModified:
			// When the building geometry has changed, we need to update the geometrical info
			// in the widget based on the current selection.

			// Also, if the selection has changed, we need to distinguish between no selection and selection
			// and update the buttons accordingly.
			updateUi();
		break;

		default: ; // just to make compiler happy
	} // switch

}


// *** slots ***

void SVPropAddGeometry::on_pushButtonAddPolygon_clicked() {
	// reset new polygon object and set it into polygon mode
	SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_Polygon);
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Polygon);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropAddGeometry::on_pushButtonAddRect_clicked() {
	// reset new polygon object and set it into rect mode
	SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_Rect);
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Rect);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropAddGeometry::on_pushButtonAddZone_clicked() {
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Zone);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropAddGeometry::on_pushButtonAddRoof_clicked() {
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Roof);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropAddGeometry::on_pushButtonAddWindow_clicked() {
	// set property widget into "add window/door" mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_propertyWidgetMode = SVViewState::PM_AddSubSurfaceGeometry;
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propAddWindowWidget->setup();
}


void SVPropAddGeometry::updateUi() {
	// update our selection lists
	std::set<const VICUS::Object*> sel;

	// first we get how many surfaces are selected
	project().selectObjects(sel, VICUS::Project::SG_All, false, false);

	// check if we have any surfaces selected?


	// process all selected objects and sort them into vectors
	unsigned int surfaceCount = 0;
	for (const VICUS::Object * o : sel) {
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr )
			++surfaceCount;
	}

	// handling if surfaces are selected
	if (surfaceCount == 1) {
		// enable "add subsurface" button
		m_ui->pushButtonAddWindow->setEnabled(true);
	}
	else {
		m_ui->pushButtonAddWindow->setEnabled(false);
	}
}

