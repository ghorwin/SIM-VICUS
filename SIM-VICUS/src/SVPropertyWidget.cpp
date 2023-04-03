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

#include "SVPropertyWidget.h"

#include <QVBoxLayout>
#include <QToolBox>

#include <IBKMK_Vector3D.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVConstants.h"

#include "SVPropVertexListWidget.h"
#include "SVPropAddGeometry.h"
#include "SVPropEditGeometry.h"
#include "SVPropSiteWidget.h"
#include "SVPropBuildingEditWidget.h"
#include "SVPropFloorManagerWidget.h"
#include "SVPropAddWindowWidget.h"
#include "SVPropNetworkEditWidget.h"
#include "SVPropBuildingAcousticTemplatesWidget.h"
#include "SVPropStructuralUnitEditWidget.h"
#include "SVPropResultsWidget.h"

#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DWireFrameObject.h"

SVPropertyWidget::SVPropertyWidget(QWidget * parent) :
	QWidget(parent)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	setLayout(m_layout);
	for (QWidget * &w : m_propWidgets)
		w = nullptr;

	setMinimumWidth(200);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropertyWidget::onViewStateChanged);

	// create edit geometry widget - this is needed since other widgets like the coordinate system widget send
	// coordinate changes to the widget
	showPropertyWidget<SVPropEditGeometry>(M_EditGeometry);

	// for default we start with add geometry mode
	setPropertyWidgetVisible(SVViewState::PM_AddGeometry);

	SVViewStateHandler::instance().m_propertyWidget = this;
}


void SVPropertyWidget::setBuildingPropertyType(int buildingPropertyType) {
	showPropertyWidget<SVPropBuildingEditWidget>(M_BuildingProperties);
	qobject_cast<SVPropBuildingEditWidget*>(m_propWidgets[M_BuildingProperties])->setPropertyType(buildingPropertyType);
}


void SVPropertyWidget::setStructuralUnitPropertyType(int buildingPropertyType) {
	showPropertyWidget<SVPropStructuralUnitEditWidget>(M_BuildingStructuralUnitProperties);
	qobject_cast<SVPropStructuralUnitEditWidget*>(m_propWidgets[M_BuildingStructuralUnitProperties])->setPropertyType(buildingPropertyType);
}


void SVPropertyWidget::setNetworkPropertyType(int networkPropertyType) {
	showPropertyWidget<SVPropNetworkEditWidget>(M_NetworkProperties);
	qobject_cast<SVPropNetworkEditWidget*>(m_propWidgets[M_NetworkProperties])->setPropertyType(networkPropertyType);
}


void SVPropertyWidget::updateColorMode() {
	switch (m_propertyWidgetMode) {
		case SVViewState::PM_BuildingProperties : {
			// enforce color update
			SVPropBuildingEditWidget *widget = qobject_cast<SVPropBuildingEditWidget*>(m_propWidgets[M_BuildingProperties]);
			widget->setPropertyType((int)widget->currentPropertyType());
		} break;
		case SVViewState::PM_NetworkProperties : {
			// enforce color update
			SVPropNetworkEditWidget *widget = qobject_cast<SVPropNetworkEditWidget*>(m_propWidgets[M_NetworkProperties]);
			widget->setPropertyType((int)widget->currentPropertyType());
		} break;
		case SVViewState::PM_BuildingStructuralUnitProperties : {
			// enforce color update
			SVPropStructuralUnitEditWidget *widget = qobject_cast<SVPropStructuralUnitEditWidget*>(m_propWidgets[M_BuildingStructuralUnitProperties]);
			widget->setPropertyType((int)widget->currentPropertyType());
		} break;

		default: break;
	}
}


void SVPropertyWidget::setPropertyWidgetVisible(SVViewState::PropertyWidgetMode propertyWidgetMode) {

	// hide all already created widgets
	for (QWidget * w : m_propWidgets) {
		if (w != nullptr)
			w->setVisible(false);
	}

	switch (propertyWidgetMode) {
		case SVViewState::PM_AddGeometry :
			showPropertyWidget<SVPropAddGeometry>(M_AddGeometry);
		break;

		case SVViewState::PM_EditGeometry :
			showPropertyWidget<SVPropEditGeometry>(M_EditGeometry);
		break;

		case SVViewState::PM_VertexList:
			showPropertyWidget<SVPropVertexListWidget>(M_VertexListWidget);
			setMinimumWidth(500);
		break;

		case SVViewState::PM_AddSubSurfaceGeometry:
			showPropertyWidget<SVPropAddWindowWidget>(M_AddWindowWidget);
			setMinimumWidth(500);
		break;

		case SVViewState::PM_SiteProperties :
			showPropertyWidget<SVPropSiteWidget>(M_SiteProperties);
		break;

		case SVViewState::PM_BuildingProperties : {
			showPropertyWidget<SVPropBuildingEditWidget>(M_BuildingProperties);
		} break;

		case SVViewState::PM_BuildingAcousticProperties : {
			showPropertyWidget<SVPropBuildingAcousticTemplatesWidget>(M_BuildingAcousticProperties);
		} break;
		case SVViewState::PM_BuildingStructuralUnitProperties : {
			showPropertyWidget<SVPropStructuralUnitEditWidget>(M_BuildingStructuralUnitProperties);
		} break;
		case SVViewState::PM_NetworkProperties : {
			showPropertyWidget<SVPropNetworkEditWidget>(M_NetworkProperties);
		} break;

		case SVViewState::PM_ResultsProperties : {
			showPropertyWidget<SVPropResultsWidget>(M_ResultsWidget);
			qobject_cast<SVPropResultsWidget*>(m_propWidgets[M_ResultsWidget])->refreshDirectory();
		} break;
	}
}


void SVPropertyWidget::onViewStateChanged() {

	// get property widget mode from view state
	SVViewState vs = SVViewStateHandler::instance().viewState();

	// If property widget mode has changed: we update it and make the new widget visible
	// Note: This check prevents us from running into an endless loop
	if (vs.m_propertyWidgetMode != m_propertyWidgetMode) {
		m_propertyWidgetMode = vs.m_propertyWidgetMode;
		setPropertyWidgetVisible(m_propertyWidgetMode);
	}
}

