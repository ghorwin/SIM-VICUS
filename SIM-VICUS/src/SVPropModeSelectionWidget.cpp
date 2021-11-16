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

#include "SVPropModeSelectionWidget.h"
#include "ui_SVPropModeSelectionWidget.h"

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVUndoModifySiteData.h"

SVPropModeSelectionWidget::SVPropModeSelectionWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropModeSelectionWidget)
{
	// make us known to the world
	SVViewStateHandler::instance().m_propModeSelectionWidget = this;

	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->comboBoxBuildingProperties->blockSignals(true);
	m_ui->comboBoxBuildingProperties->addItem(tr("Component"), BT_Components);
	m_ui->comboBoxBuildingProperties->addItem(tr("Sub-Surfaces"), BT_SubSurfaceComponents);
	m_ui->comboBoxBuildingProperties->addItem(tr("Construction orientation"), BT_ComponentOrientation);
	m_ui->comboBoxBuildingProperties->addItem(tr("Boundary conditions"), BT_BoundaryConditions);
	m_ui->comboBoxBuildingProperties->addItem(tr("Surface connections/component instances"), BT_InterlinkedSurfaces);
	m_ui->comboBoxBuildingProperties->addItem(tr("Building levels"), BT_FloorManager);
	m_ui->comboBoxBuildingProperties->addItem(tr("Zone templates"), BT_ZoneTemplates);
	m_ui->comboBoxBuildingProperties->addItem(tr("Surface heating"), BT_SurfaceHeating);
	m_ui->comboBoxBuildingProperties->blockSignals(false);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropModeSelectionWidget::onModified);

	// we are in "Site" mode initially, so the rest is hidden
	updateWidgetVisibility();
}


SVPropModeSelectionWidget::~SVPropModeSelectionWidget() {
	delete m_ui;
}


void SVPropModeSelectionWidget::setBuildingPropertyType(BuildingPropertyTypes pt) {
	// This function is triggered from main window, when the building property type combo box
	// shall be modified program-wise and not by user with mouse click
	m_ui->pushButtonSite->blockSignals(true);
	m_ui->pushButtonNetwork->blockSignals(true);
	m_ui->pushButtonBuilding->blockSignals(true);

	m_ui->pushButtonSite->setChecked(false);
	m_ui->pushButtonNetwork->setChecked(false);
	m_ui->pushButtonBuilding->setChecked(true);

	m_ui->pushButtonSite->blockSignals(false);
	m_ui->pushButtonNetwork->blockSignals(false);
	m_ui->pushButtonBuilding->blockSignals(false);

	m_ui->comboBoxBuildingProperties->blockSignals(true);
	m_ui->comboBoxBuildingProperties->setCurrentIndex( m_ui->comboBoxBuildingProperties->findData(pt) );
	m_ui->comboBoxBuildingProperties->blockSignals(false);

	updateWidgetVisibility();
}


void SVPropModeSelectionWidget::onModified(int modificationType, ModificationInfo * ) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
		case SVProjectHandler::NetworkModified:
		case SVProjectHandler::AllModified:
		case SVProjectHandler::NodeStateModified:
			// Based on current selection, switch into a specific building/network edit mode. For example, when
			// "Network" mode is selected and a node is selected, we automatically switch to "Node" edit mode.
			selectionChanged();
			// Note: the call above may results in a view state change, when the selection causes
			//		 the network property selection to change.
			break;

		default: ; // just to make compiler happy
	}
}


void SVPropModeSelectionWidget::selectionChanged() {
	if (m_ui->pushButtonNetwork->isChecked()) {
		// update combo box with network selection, based on current selection

		// get list of selected network-related objects
		std::set<const VICUS::Object*> sel;
		project().selectObjects(sel, VICUS::Project::SG_Network, true, true);

		std::set<const VICUS::Network*> selectedNetworks;
		// for all objects follow parent links up to top level and if this is a network, remember network in set of network objets
		for (const VICUS::Object* o : sel) {
			if (o->m_parent == nullptr) {
				const VICUS::Network* n = dynamic_cast<const VICUS::Network*>(o);
				Q_ASSERT(n != nullptr);
				selectedNetworks.insert(n);
			}
			else {
				const VICUS::Network* n = dynamic_cast<const VICUS::Network*>(o->m_parent);
				Q_ASSERT(n != nullptr);
				selectedNetworks.insert(n);
			}
		}

		// rules:

		// no network selected -> show all available networks and let user select one
		// 1 network selected -> show network name and disable combo box
		// more networks selected -> clear combo box and disable

		m_ui->comboBoxSelectedNetwork->blockSignals(true);
		m_ui->comboBoxSelectedNetwork->clear();
		if (selectedNetworks.empty()) {
			for (const VICUS::Network & n : project().m_geometricNetworks)
				m_ui->comboBoxSelectedNetwork->addItem(n.m_displayName, n.m_id);
			m_ui->comboBoxSelectedNetwork->setEnabled(true);
		}
		else if (selectedNetworks.size() == 1) {
			m_ui->comboBoxSelectedNetwork->addItem((*selectedNetworks.begin())->m_displayName, (*selectedNetworks.begin())->m_id);
			m_ui->comboBoxSelectedNetwork->setEnabled(false);
		}
		else {
			m_ui->comboBoxSelectedNetwork->setEnabled(false);
		}
		m_ui->comboBoxSelectedNetwork->blockSignals(false);

		// if selected network has changed (due to modification in combo box), also update the rest of the view
		updateViewState();
	}
}


BuildingPropertyTypes SVPropModeSelectionWidget::currentBuildingPropertyType() const {
	return (BuildingPropertyTypes)m_ui->comboBoxBuildingProperties->currentData().toInt();
}


int SVPropModeSelectionWidget::currentNetworkPropertyType() const {
	return m_ui->comboBoxNetworkProperties->currentIndex();
}


void SVPropModeSelectionWidget::viewStateProperties(SVViewState & vs) const {
	if (m_ui->pushButtonBuilding->isChecked())
		vs.m_propertyWidgetMode = SVViewState::PM_BuildingProperties;
	else if (m_ui->pushButtonNetwork->isChecked())
		vs.m_propertyWidgetMode = SVViewState::PM_NetworkProperties;
	else
		vs.m_propertyWidgetMode = SVViewState::PM_SiteProperties;

	// also set the scene coloring mode
	switch (vs.m_propertyWidgetMode) {
		case SVViewState::PM_SiteProperties:
			// clear scene coloring
			vs.m_objectColorMode = SVViewState::OCM_None;
		break;

		case SVViewState::PM_BuildingProperties:
			switch ((BuildingPropertyTypes)m_ui->comboBoxBuildingProperties->currentData().toInt()) {
				case BT_Components:				vs.m_objectColorMode = SVViewState::OCM_Components; break;
				case BT_SubSurfaceComponents:	vs.m_objectColorMode = SVViewState::OCM_SubSurfaceComponents; break;
				case BT_ComponentOrientation:	vs.m_objectColorMode = SVViewState::OCM_ComponentOrientation; break;
				case BT_BoundaryConditions:		vs.m_objectColorMode = SVViewState::OCM_BoundaryConditions; break;
				case BT_InterlinkedSurfaces:	vs.m_objectColorMode = SVViewState::OCM_InterlinkedSurfaces; break;
				case BT_FloorManager:			vs.m_objectColorMode = SVViewState::OCM_None; break;
				case BT_ZoneTemplates:			vs.m_objectColorMode = SVViewState::OCM_ZoneTemplates; break;
				case BT_SurfaceHeating:			vs.m_objectColorMode = SVViewState::OCM_SurfaceHeating; break;
			}
		break;

		case SVViewState::PM_NetworkProperties:
			switch (m_ui->comboBoxNetworkProperties->currentIndex()) {
				// network
				case 0 : vs.m_objectColorMode = SVViewState::OCM_Network; break;

				// node: show node association
				case 1 : vs.m_objectColorMode = SVViewState::OCM_NetworkNode; break;

				// pipe : show pipe association
				case 2 : vs.m_objectColorMode = SVViewState::OCM_NetworkEdge; break;

				// component: show network component association
				case 3 : vs.m_objectColorMode = SVViewState::OCM_NetworkSubNetworks; break;

				// component: show network component association
				case 4 : vs.m_objectColorMode = SVViewState::OCM_NetworkHeatExchange; break;
			}
		break;

		default:; // just to make compiler happy
	}
}


unsigned int SVPropModeSelectionWidget::currentNetworkId() const {
	return m_ui->comboBoxSelectedNetwork->currentData().toUInt();
}


void SVPropModeSelectionWidget::setCurrentNetwork(unsigned networkId) {
	return m_ui->comboBoxSelectedNetwork->setCurrentIndex(
		m_ui->comboBoxSelectedNetwork->findData(networkId));
}


void SVPropModeSelectionWidget::setDefaultViewState() {
	// this function is called when we return from any intermediate mode like "align
	// coordinate system" mode and get back to what we where doing before

	// if we are in geometry mode, we need to distinguish between PlaceVertex mode or SelectedGeometry mode
	// or NUM mode (nothing)

	SVViewState vs = SVViewStateHandler::instance().viewState();

	if (vs.m_viewMode == SVViewState::VM_PropertyEditMode) {
		// we always have scene operation mode none
		vs.m_sceneOperationMode = SVViewState::NUM_OM;
		SVViewStateHandler::instance().setViewState(vs);
		return;
	}

	switch (vs.m_propertyWidgetMode) {
		case SVViewState::PM_AddEditGeometry:
		case SVViewState::PM_SiteProperties:
		case SVViewState::PM_BuildingProperties:
		case SVViewState::PM_NetworkProperties: {
			// do we have any selected geometries
			std::set<const VICUS::Object *> sel;
			project().selectObjects(sel, VICUS::Project::SG_All, true, true);
			if (sel.empty())
				vs.m_sceneOperationMode = SVViewState::NUM_OM;
			else
				vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
			SVViewStateHandler::instance().setViewState(vs);
			return;
		}

		case SVViewState::PM_VertexList:
			vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
			SVViewStateHandler::instance().setViewState(vs);
			return;

		case SVViewState::PM_AddSubSurfaceGeometry:
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
			SVViewStateHandler::instance().setViewState(vs);
			return;
	}

}


void SVPropModeSelectionWidget::on_pushButtonBuilding_toggled(bool on) {
	// Note: this slot is also called when the button is turned off due to another
	//       button being turned on. But we don't want to change the view state twice,
	//       so we only handle the signal if the button is turned on
	if (!on) return;

	updateWidgetVisibility();

	// block signals in this widget, since we do not want to let selectionChanged() change the viewstate already
	blockSignals(true);
	selectionChanged();
	blockSignals(false);

	updateViewState(); // tell the world that our edit mode has changed
}


void SVPropModeSelectionWidget::on_pushButtonNetwork_toggled(bool on) {
	// Note: this slot is also called when the button is turned off due to another
	//       button being turned on. But we don't want to change the view state twice,
	//       so we only handle the signal if the button is turned on
	if (!on) return;

	updateWidgetVisibility();

	// block signals in this widget, since we do not want to let selectionChanged() change the viewstate already
	blockSignals(true);
	selectionChanged();
	blockSignals(false);

	updateViewState(); // tell the world that our edit mode has changed
}


void SVPropModeSelectionWidget::on_pushButtonSite_toggled(bool on) {
	// Note: this slot is also called when the button is turned off due to another
	//       button being turned on. But we don't want to change the view state twice,
	//       so we only handle the signal if the button is turned on
	if (!on) return;

	updateWidgetVisibility();

	updateViewState(); // tell the world that our edit mode has changed
}



void SVPropModeSelectionWidget::on_comboBoxNetworkProperties_currentIndexChanged(int) {
	updateViewState();
}


void SVPropModeSelectionWidget::on_comboBoxBuildingProperties_currentIndexChanged(int) {
	updateViewState();
}


void SVPropModeSelectionWidget::updateWidgetVisibility() {
	bool showBuildingProps = m_ui->pushButtonBuilding->isChecked();
	bool showNetworkProps = m_ui->pushButtonNetwork->isChecked();
	m_ui->labelNetworkProperties->setVisible(showNetworkProps);
	m_ui->comboBoxNetworkProperties->setVisible(showNetworkProps);

	m_ui->labelSelectedNetwork->setVisible(showNetworkProps);
	m_ui->comboBoxSelectedNetwork->setVisible(showNetworkProps);

	m_ui->labelBuildingProperties->setVisible(showBuildingProps);
	m_ui->comboBoxBuildingProperties->setVisible(showBuildingProps);
}


void SVPropModeSelectionWidget::updateViewState() {
	// this function is only called, when indeed there was a change (one of the buttons was pressed,
	// user has selected a property in the combo box, property combo was changed due to selection change)

	SVViewState vs = SVViewStateHandler::instance().viewState();
	viewStateProperties(vs); // select correct coloring mode based on current selection in combo box and type buttons

	// now set the new viewstate to update property widgets and scene coloring at the same time
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropModeSelectionWidget::on_comboBoxSelectedNetwork_currentIndexChanged(int /*index*/) {
	updateViewState();
}
