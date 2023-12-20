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

#include <QtExt_ToolBox.h>

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
#include "SVPropSupplySystemsWidget.h"
#include "SVMainWindow.h"
#include "SVPreferencesDialog.h"
#include "SVPreferencesPageStyle.h"

SVPropBuildingEditWidget::SVPropBuildingEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->toolBox->layout()->setMargin(0);

	// add pages to tool box:
	// Note: NEVER change the order, it must correspond to BuildingPropertyType enum.
	m_ui->toolBox->blockSignals(true);
	m_ui->toolBox->addPage(tr("Component"), new SVPropBuildingComponentsWidget(this));
	m_ui->toolBox->addPage(tr("Sub-Surfaces"), new SVPropBuildingSubComponentsWidget(this));
	m_ui->toolBox->addPage(tr("Construction orientation"), new SVPropBuildingComponentOrientationWidget(this));
	m_ui->toolBox->addPage(tr("Boundary conditions"), new SVPropBuildingBoundaryConditionsWidget(this));
	m_ui->toolBox->addPage(tr("Surface connections/component instances"), new SVPropBuildingSurfaceConnectionWidget(this));
	m_ui->toolBox->addPage(tr("Zone templates"), new SVPropBuildingZoneTemplatesWidget(this));
	m_ui->toolBox->addPage(tr("Surface heating"), new SVPropBuildingSurfaceHeatingWidget(this));
	m_ui->toolBox->addPage(tr("Supply Systems"), new SVPropSupplySystemsWidget(this));
	m_ui->toolBox->addPage(tr("Room properties"), new SVPropBuildingZoneProperty(this));
	m_ui->toolBox->addPage(tr("Building levels"), new SVPropFloorManagerWidget(this));

	m_ui->toolBox->blockSignals(false);
	m_ui->toolBox->setCurrentIndex(BT_Components);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropBuildingEditWidget::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::colorRefreshNeeded,
			this, &SVPropBuildingEditWidget::onColorRefreshNeeded);

	connect(m_ui->toolBox, &QtExt::ToolBox::indexChanged,
			this, &SVPropBuildingEditWidget::onCurrentBuildingPropertyTypeChanged);

	connect(SVMainWindow::instance().preferencesDialog()->pageStyle(), &SVPreferencesPageStyle::styleChanged,
			this, &SVPropBuildingEditWidget::onStyleChanged);

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);

	// update color view initially
	onCurrentBuildingPropertyTypeChanged((BuildingPropertyType)m_ui->toolBox->currentIndex());
}


SVPropBuildingEditWidget::~SVPropBuildingEditWidget() {
	delete m_ui;
}


void SVPropBuildingEditWidget::setPropertyType(int buildingPropertyType) {
	m_ui->toolBox->setCurrentIndex((unsigned int)buildingPropertyType);
}


void SVPropBuildingEditWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// react on selection changes only, then update properties
	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		// This is meant to update all widgets when e.g. a new project has been loaded
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::BuildingTopologyChanged: // used when zone templates are assigned
		case SVProjectHandler::ComponentInstancesModified:
		case SVProjectHandler::SubSurfaceComponentInstancesModified:
			updateUi(false);
		break;
		case SVProjectHandler::NodeStateModified:
			updateUi(true);
		break;

		case SVProjectHandler::ObjectRenamed: // we only show zone names in surface heating
			dynamic_cast<SVPropBuildingSurfaceHeatingWidget*>(m_ui->toolBox->widget(BT_SurfaceHeating))->updateUi(false);
			dynamic_cast<SVPropBuildingZoneProperty*>(m_ui->toolBox->widget(BT_ZoneProperty))->updateUi();
		break;

		// nothing to do for the remaining modification types
		case SVProjectHandler::SolverParametersModified:
		case SVProjectHandler::ClimateLocationModified:
		case SVProjectHandler::GridModified:
		case SVProjectHandler::NetworkGeometryChanged:
		case SVProjectHandler::NetworkDataChanged:
		case SVProjectHandler::ClimateLocationAndFileModified:
		case SVProjectHandler::OutputsModified:
		case SVProjectHandler::StructuralUnitsModified:
		case SVProjectHandler::DrawingModified:
		case SVProjectHandler::LcaLccModified:
		break;
	}
}


void SVPropBuildingEditWidget::onColorRefreshNeeded() {
	// for now we just rebuild the widgets... this might be changed in the future, if performance issues arise
	// since updating the table color rows is usually much faster than rebuilding the entire UI
	updateUi(false);
}

void SVPropBuildingEditWidget::onCurrentBuildingPropertyTypeChanged(int propertyType) {
	BuildingPropertyType buildingPropType = BuildingPropertyType(propertyType);
	// set coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (buildingPropType) {
		case BT_Components				: vs.m_objectColorMode = SVViewState::OCM_Components ; break;
		case BT_SubSurfaceComponents	: vs.m_objectColorMode = SVViewState::OCM_SubSurfaceComponents; break;
		case BT_ComponentOrientation	: vs.m_objectColorMode = SVViewState::OCM_ComponentOrientation; break;
		case BT_BoundaryConditions		: {
				unsigned int ocm = dynamic_cast<SVPropBuildingBoundaryConditionsWidget*>(m_ui->toolBox->widget(BT_BoundaryConditions))->currentObjectColorMode();
				vs.m_objectColorMode = SVViewState::ObjectColorMode(ocm);
			} break;
		case BT_SurfaceConnection		: vs.m_objectColorMode = SVViewState::OCM_InterlinkedSurfaces; break;
		case BT_ZoneTemplates			: vs.m_objectColorMode = SVViewState::OCM_ZoneTemplates; break;
		case BT_SurfaceHeating			: vs.m_objectColorMode = SVViewState::OCM_SurfaceHeating; break;
		case BT_SupplySystems			: vs.m_objectColorMode = SVViewState::OCM_SupplySystems; break;
		case BT_ZoneProperty			: vs.m_objectColorMode = SVViewState::OCM_ZoneTemplates; break;
		case BT_FloorManager			: vs.m_objectColorMode = SVViewState::OCM_None; break;
	}
	SVViewStateHandler::instance().setViewState(vs);
}

unsigned int SVPropBuildingEditWidget::currentPropertyType() {
	return m_ui->toolBox->currentIndex();
}


void SVPropBuildingEditWidget::onStyleChanged() {
	m_ui->toolBox->updatePageBackgroundColorFromStyle();
}


// *** PRIVATE FUNCTIONS ***

void SVPropBuildingEditWidget::updateUi(bool onlyNodeStateModified) {
	// TODO Andreas : this function currently updates all widgets in the stacked widget, regardless of which
	//                is currently visible. This makes switching property modes very fast, but whenever the project
	//                data changes, it takes a bit more time. If this becomes a performance issue at some point,
	//                modify the update logic.
	// Note: It is not meaningful to update the widgets based on their visibility.
	// It could be that project data changes and then the user switches to a different widget, which has then not be updated yet.
	dynamic_cast<SVPropBuildingComponentsWidget*>(m_ui->toolBox->widget(BT_Components))->updateUi();
	dynamic_cast<SVPropBuildingSubComponentsWidget*>(m_ui->toolBox->widget(BT_SubSurfaceComponents))->updateUi();
	dynamic_cast<SVPropBuildingComponentOrientationWidget*>(m_ui->toolBox->widget(BT_ComponentOrientation))->updateUi();
	dynamic_cast<SVPropBuildingBoundaryConditionsWidget*>(m_ui->toolBox->widget(BT_BoundaryConditions))->updateUi();
	dynamic_cast<SVPropBuildingSurfaceConnectionWidget*>(m_ui->toolBox->widget(BT_SurfaceConnection))->updateUi(onlyNodeStateModified);
	dynamic_cast<SVPropBuildingZoneTemplatesWidget*>(m_ui->toolBox->widget(BT_ZoneTemplates))->updateUi();
	dynamic_cast<SVPropBuildingSurfaceHeatingWidget*>(m_ui->toolBox->widget(BT_SurfaceHeating))->updateUi(onlyNodeStateModified);
	dynamic_cast<SVPropSupplySystemsWidget*>(m_ui->toolBox->widget(BT_SupplySystems))->updateUi();
	dynamic_cast<SVPropBuildingZoneProperty*>(m_ui->toolBox->widget(BT_ZoneProperty))->updateUi();

	// SVPropFloorManagerWidget has its own onModified() slot, no need to handle that here
}

