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

SVPropBuildingEditWidget::SVPropBuildingEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->mainLayout->setMargin(0);

	// Mind: order in which the widgets are added is important, see setPropertyType()
	m_ui->stackedWidget->addWidget(new SVPropBuildingComponentsWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingSubComponentsWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingComponentOrientationWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingBoundaryConditionsWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingSurfaceConnectionWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingZoneTemplatesWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingSurfaceHeatingWidget(this));
	m_ui->stackedWidget->addWidget(new SVPropBuildingZoneProperty(this));

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
	m_propertyType = buildingPropertyType;

	switch ((BuildingPropertyTypes)buildingPropertyType) {
		case BT_Components				: m_ui->stackedWidget->setCurrentIndex(0); break;
		case BT_SubSurfaceComponents	: m_ui->stackedWidget->setCurrentIndex(1); break;
		case BT_ComponentOrientation	: m_ui->stackedWidget->setCurrentIndex(2); break;
		case BT_BoundaryConditions		: m_ui->stackedWidget->setCurrentIndex(3); break;
		case BT_InterlinkedSurfaces		: m_ui->stackedWidget->setCurrentIndex(4); break;
		case BT_ZoneTemplates			: m_ui->stackedWidget->setCurrentIndex(5); break;
		case BT_SurfaceHeating			: m_ui->stackedWidget->setCurrentIndex(6); break;
		case BT_ZoneProperty			: m_ui->stackedWidget->setCurrentIndex(7); break;
		case BT_FloorManager : break; // just to remove compiler warning, FloorManager is not handled here
	}
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
			dynamic_cast<SVPropBuildingSurfaceHeatingWidget*>(m_ui->stackedWidget->widget(6))->updateUi();
			dynamic_cast<SVPropBuildingZoneProperty*>(m_ui->stackedWidget->widget(7))->updateUi();
		break;

		// nothing to do for the remaining modification types
		case SVProjectHandler::SolverParametersModified:
		case SVProjectHandler::ClimateLocationModified:
		case SVProjectHandler::GridModified:
		case SVProjectHandler::NetworkModified:
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
	// TODO Andreas : this function currently updates all widgets in the stacked widget, regardless of which
	//                is currently visible. This makes switching property modes very fast, but whenever the project
	//                data changes, it takes a bit more time. If this becomes a performance issue at some point,
	//                modify the update logic.

	dynamic_cast<SVPropBuildingComponentsWidget*>(m_ui->stackedWidget->widget(0))->updateUi();
	dynamic_cast<SVPropBuildingSubComponentsWidget*>(m_ui->stackedWidget->widget(1))->updateUi();
	dynamic_cast<SVPropBuildingComponentOrientationWidget*>(m_ui->stackedWidget->widget(2))->updateUi();
	dynamic_cast<SVPropBuildingBoundaryConditionsWidget*>(m_ui->stackedWidget->widget(3))->updateUi();
	dynamic_cast<SVPropBuildingSurfaceConnectionWidget*>(m_ui->stackedWidget->widget(4))->updateUi();
	dynamic_cast<SVPropBuildingZoneTemplatesWidget*>(m_ui->stackedWidget->widget(5))->updateUi();
	dynamic_cast<SVPropBuildingSurfaceHeatingWidget*>(m_ui->stackedWidget->widget(6))->updateUi();
	dynamic_cast<SVPropBuildingZoneProperty*>(m_ui->stackedWidget->widget(7))->updateUi();
}
