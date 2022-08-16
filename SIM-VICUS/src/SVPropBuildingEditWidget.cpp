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

#include "SVPropBuildingEditWidget.h"
#include "ui_SVPropBuildingEditWidget.h"

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVConstants.h"

#include "SVPropBuildingComponentsWidget.h"
#include "SVPropBuildingSubComponentsWidget.h"
#include "SVPropBuildingComponentOrientationWidget.h"
#include "SVPropBuildingBoundaryConditionsWidget.h"
#include "SVPropBuildingSurfaceConnectionWidget.h"
#include "SVPropBuildingZoneTemplatesWidget.h"
#include "SVPropBuildingSurfaceHeatingWidget.h"
#include "SVPropBuildingZoneProperty.h"
#include "SVPropFloorManagerWidget.h"

SVPropBuildingEditWidget::SVPropBuildingEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->mainLayout->setMargin(0);

	// populate combobox
	m_ui->comboBoxBuildingProperties->blockSignals(true);
	m_ui->comboBoxBuildingProperties->addItem(tr("Component"), BT_Components);
	m_ui->comboBoxBuildingProperties->addItem(tr("Sub-Surfaces"), BT_SubSurfaceComponents);
	m_ui->comboBoxBuildingProperties->addItem(tr("Construction orientation"), BT_ComponentOrientation);
	m_ui->comboBoxBuildingProperties->addItem(tr("Boundary conditions"), BT_BoundaryConditions);
	m_ui->comboBoxBuildingProperties->addItem(tr("Surface connections/component instances"), BT_SurfaceConnection);
	m_ui->comboBoxBuildingProperties->addItem(tr("Zone templates"), BT_ZoneTemplates);
	m_ui->comboBoxBuildingProperties->addItem(tr("Surface heating"), BT_SurfaceHeating);
	m_ui->comboBoxBuildingProperties->addItem(tr("Room properties"), BT_ZoneProperty);
	m_ui->comboBoxBuildingProperties->addItem(tr("Building levels"), BT_FloorManager);
	m_ui->comboBoxBuildingProperties->blockSignals(false);
	m_ui->comboBoxBuildingProperties->setCurrentIndex(0);

	// Mind: order in which the widgets are added is important, see setPropertyType()
	// Note: keep this order in sync with the enum and the combobox
	m_ui->stackedWidget->addWidget(new SVPropBuildingComponentsWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingSubComponentsWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingComponentOrientationWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingBoundaryConditionsWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingSurfaceConnectionWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingZoneTemplatesWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingSurfaceHeatingWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingZoneProperty(this));
	m_ui->stackedWidget->addWidget(new SVPropFloorManagerWidget(this));

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropBuildingEditWidget::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::colorRefreshNeeded,
			this, &SVPropBuildingEditWidget::onColorRefreshNeeded);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);
}


SVPropBuildingEditWidget::~SVPropBuildingEditWidget() {
	delete m_ui;
}


void SVPropBuildingEditWidget::setPropertyType(int buildingPropertyType) {
	// if type has not changed we need to manually trigger the slot to get color update
	if (buildingPropertyType != currentPropertyType())
		m_ui->comboBoxBuildingProperties->setCurrentIndex(buildingPropertyType);
	else
		on_comboBoxBuildingProperties_currentIndexChanged(buildingPropertyType);
}


int SVPropBuildingEditWidget::currentPropertyType() {
	return m_ui->comboBoxBuildingProperties->currentIndex();
}


void SVPropBuildingEditWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// react on selection changes only, then update properties
	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::BuildingTopologyChanged: // used when zone templates are assigned
		case SVProjectHandler::ComponentInstancesModified:
		case SVProjectHandler::SubSurfaceComponentInstancesModified:
		case SVProjectHandler::NodeStateModified:
			updateUi();
		break;

		case SVProjectHandler::ObjectRenamed: // we only show zone names in surface heating
			dynamic_cast<SVPropBuildingSurfaceHeatingWidget*>(m_ui->stackedWidget->widget(BT_SurfaceHeating))->updateUi();
			dynamic_cast<SVPropBuildingZoneProperty*>(m_ui->stackedWidget->widget(BT_ZoneProperty))->updateUi();
		break;

		// nothing to do for the remaining modification types
		case SVProjectHandler::SolverParametersModified:
		case SVProjectHandler::ClimateLocationModified:
		case SVProjectHandler::GridModified:
		case SVProjectHandler::NetworkGeometryChanged:
		case SVProjectHandler::NetworkDataChanged:
		break;
	}
}


void SVPropBuildingEditWidget::onColorRefreshNeeded() {
	// for now we just rebuild the widgets... this might be changed in the future, if performance issues arise
	// since updating the table color rows is usually much faster than rebuilding the entire UI
	updateUi();
}


// *** PRIVATE FUNCTIONS ***

void SVPropBuildingEditWidget::updateUi() {
	BuildingPropertyTypes propType = BuildingPropertyTypes(m_ui->comboBoxBuildingProperties->currentData().toUInt());
	switch (propType) {
		case BT_Components:
			dynamic_cast<SVPropBuildingComponentsWidget*>(m_ui->stackedWidget->widget(BT_Components))->updateUi();
		break;
		case BT_SubSurfaceComponents:
			dynamic_cast<SVPropBuildingSubComponentsWidget*>(m_ui->stackedWidget->widget(BT_SubSurfaceComponents))->updateUi();
		break;
		case BT_ComponentOrientation:
			dynamic_cast<SVPropBuildingComponentOrientationWidget*>(m_ui->stackedWidget->widget(BT_ComponentOrientation))->updateUi();
		break;
		case BT_BoundaryConditions:
			dynamic_cast<SVPropBuildingBoundaryConditionsWidget*>(m_ui->stackedWidget->widget(BT_BoundaryConditions))->updateUi();
		break;
		case BT_SurfaceConnection:
			dynamic_cast<SVPropBuildingSurfaceConnectionWidget*>(m_ui->stackedWidget->widget(BT_SurfaceConnection))->updateUi();
		break;
		case BT_ZoneTemplates:
			dynamic_cast<SVPropBuildingZoneTemplatesWidget*>(m_ui->stackedWidget->widget(BT_ZoneTemplates))->updateUi();
		break;
		case BT_SurfaceHeating:
			dynamic_cast<SVPropBuildingSurfaceHeatingWidget*>(m_ui->stackedWidget->widget(BT_SurfaceHeating))->updateUi();
		break;
		case BT_ZoneProperty:
			dynamic_cast<SVPropBuildingZoneProperty*>(m_ui->stackedWidget->widget(BT_ZoneProperty))->updateUi();
		break;
		case BT_FloorManager: break;
		// SVPropFloorManagerWidget has its own onModified() slot, no need to handle that here
	}

}


void SVPropBuildingEditWidget::on_comboBoxBuildingProperties_currentIndexChanged(int index) {

	BuildingPropertyTypes buildingPropType = (BuildingPropertyTypes)m_ui->comboBoxBuildingProperties->currentData().toUInt();

	// show property page
	switch (buildingPropType) {
		case BT_Components				: m_ui->stackedWidget->setCurrentIndex(BT_Components); break;
		case BT_SubSurfaceComponents	: m_ui->stackedWidget->setCurrentIndex(BT_SubSurfaceComponents); break;
		case BT_ComponentOrientation	: m_ui->stackedWidget->setCurrentIndex(BT_ComponentOrientation); break;
		case BT_BoundaryConditions		: m_ui->stackedWidget->setCurrentIndex(BT_BoundaryConditions); break;
		case BT_SurfaceConnection		: m_ui->stackedWidget->setCurrentIndex(BT_SurfaceConnection); break;
		case BT_ZoneTemplates			: m_ui->stackedWidget->setCurrentIndex(BT_ZoneTemplates); break;
		case BT_SurfaceHeating			: m_ui->stackedWidget->setCurrentIndex(BT_SurfaceHeating); break;
		case BT_ZoneProperty			: m_ui->stackedWidget->setCurrentIndex(BT_ZoneProperty); break;
		case BT_FloorManager			: m_ui->stackedWidget->setCurrentIndex(BT_FloorManager); break;
	}

	// set coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (buildingPropType) {
		case BT_Components				: vs.m_objectColorMode = SVViewState::OCM_Components ; break;
		case BT_SubSurfaceComponents	: vs.m_objectColorMode = SVViewState::OCM_SubSurfaceComponents; break;
		case BT_ComponentOrientation	: vs.m_objectColorMode = SVViewState::OCM_ComponentOrientation; break;
		case BT_BoundaryConditions		: vs.m_objectColorMode = SVViewState::OCM_BoundaryConditions; break;
		case BT_SurfaceConnection		: vs.m_objectColorMode = SVViewState::OCM_InterlinkedSurfaces; break;
		case BT_ZoneTemplates			: vs.m_objectColorMode = SVViewState::OCM_ZoneTemplates; break;
		case BT_SurfaceHeating			: vs.m_objectColorMode = SVViewState::OCM_SurfaceHeating; break;
		case BT_ZoneProperty			: vs.m_objectColorMode = SVViewState::OCM_ZoneTemplates; break;
		case BT_FloorManager			: vs.m_objectColorMode = SVViewState::OCM_None; break;
	}
	SVViewStateHandler::instance().setViewState(vs);
}

