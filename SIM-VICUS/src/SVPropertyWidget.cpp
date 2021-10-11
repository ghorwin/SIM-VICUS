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
#include "SVPropEditGeometry.h"
#include "SVPropSiteWidget.h"
#include "SVPropNetworkEditWidget.h"
#include "SVPropModeSelectionWidget.h"
#include "SVPropBuildingEditWidget.h"
#include "SVPropFloorManagerWidget.h"
#include "SVPropAddWindowWidget.h"


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

	m_propModeSelectionWidget = new SVPropModeSelectionWidget(this);
	m_layout->addWidget(m_propModeSelectionWidget);


	setMinimumWidth(200);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropertyWidget::onViewStateChanged);

	onViewStateChanged();
}



void SVPropertyWidget::onViewStateChanged() {

	// hide all already created widgets
	for (QWidget * w : m_propWidgets) {
		if (w != nullptr)
			w->setVisible(false);
	}

	SVViewState vs = SVViewStateHandler::instance().viewState();

	switch (vs.m_viewMode) {
		case SVViewState::VM_GeometryEditMode:
			m_propModeSelectionWidget->setVisible(false);
		break;
		case SVViewState::VM_PropertyEditMode:
			m_propModeSelectionWidget->setVisible(true);
		break;
		case SVViewState::NUM_VM: break; // just to make compiler happy
	}

	// now show the respective property widget
	SVViewState::PropertyWidgetMode m = vs.m_propertyWidgetMode;
	switch (m) {
		case SVViewState::PM_AddEditGeometry :
			showPropertyWidget<SVPropEditGeometry>(M_Geometry);
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
			// select highlighting/edit mode -> this will send a signal to update the scene's geometry coloring
			BuildingPropertyTypes buildingPropertyType = m_propModeSelectionWidget->currentBuildingPropertyType();
			if (buildingPropertyType == BT_FloorManager) {
				showPropertyWidget<SVPropFloorManagerWidget>(M_FloorManager);
			}
			else {
				showPropertyWidget<SVPropBuildingEditWidget>(M_BuildingProperties);
				qobject_cast<SVPropBuildingEditWidget*>(m_propWidgets[M_BuildingProperties])->setPropertyType(buildingPropertyType);
			}
		} break;

		case SVViewState::PM_NetworkProperties : {
			showPropertyWidget<SVPropNetworkEditWidget>(M_NetworkProperties);
			SVPropNetworkEditWidget *propNetworkEditWidget = qobject_cast<SVPropNetworkEditWidget*>(m_propWidgets[M_NetworkProperties]);
			propNetworkEditWidget->m_propModeSelectionWidget = m_propModeSelectionWidget;
			// select highlighting/edit mode -> this will send a signal to update the scene's geometry coloring
			int networkPropertyType = m_propModeSelectionWidget->currentNetworkPropertyType();
			propNetworkEditWidget->setPropertyMode(networkPropertyType);
			// set current network id from combobox selection
			unsigned int networkId  = m_propModeSelectionWidget->currentNetworkId();
			propNetworkEditWidget->selectionChanged(networkId);
		} break;
	}
}


