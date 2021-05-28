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

#include <QMessageBox>

#include <VICUS_Component.h>
#include <QtExt_Conversions.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVConstants.h"
#include "SVStyle.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVDBZoneTemplateEditDialog.h"

#include "SVUndoTreeNodeState.h"
#include "SVUndoModifyComponentInstances.h"
#include "SVUndoModifySubSurfaceComponentInstances.h"
#include "SVUndoModifyRoomZoneTemplateAssociation.h"
#include "SVUndoTreeNodeState.h"

SVPropBuildingEditWidget::SVPropBuildingEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);
	m_ui->verticalLayoutComponents->setMargin(0);
	m_ui->verticalLayoutSubSurfaceComponents->setMargin(0);
	m_ui->verticalLayoutComponentOrientation->setMargin(0);
	m_ui->verticalLayoutBoundaryConditions->setMargin(0);
	m_ui->verticalLayoutZoneTemplates->setMargin(0);

	// configure tables
	m_ui->tableWidgetComponents->setColumnCount(2);
	m_ui->tableWidgetComponents->setHorizontalHeaderLabels(QStringList() << QString() << tr("Component"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetComponents);
	m_ui->tableWidgetComponents->setSortingEnabled(false);
	m_ui->tableWidgetComponents->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetComponents->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetComponents->horizontalHeader()->setStretchLastSection(true);

	m_ui->tableWidgetSubSurfaceComponents->setColumnCount(3);
	m_ui->tableWidgetSubSurfaceComponents->setHorizontalHeaderLabels(QStringList() << QString() << tr("Type") << tr("Sub-Surface Component") );
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSubSurfaceComponents);
	m_ui->tableWidgetSubSurfaceComponents->setSortingEnabled(false);
	m_ui->tableWidgetSubSurfaceComponents->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetSubSurfaceComponents->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetSubSurfaceComponents->horizontalHeader()->setStretchLastSection(true);

	m_ui->tableWidgetBoundaryConditions->setColumnCount(2);
	m_ui->tableWidgetBoundaryConditions->setHorizontalHeaderLabels(QStringList() << QString() << tr("Boundary condition"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetBoundaryConditions);
	m_ui->tableWidgetBoundaryConditions->setSortingEnabled(false);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->setStretchLastSection(true);

	m_ui->tableWidgetZoneTemplates->setColumnCount(2);
	m_ui->tableWidgetZoneTemplates->setHorizontalHeaderLabels(QStringList() << QString() << tr("Zone template"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetZoneTemplates);
	m_ui->tableWidgetZoneTemplates->setSortingEnabled(false);
	m_ui->tableWidgetZoneTemplates->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetZoneTemplates->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetZoneTemplates->horizontalHeader()->setStretchLastSection(true);

	// init widget to correct initial state
	m_ui->labelComponentSelection->setEnabled(false);
	m_ui->comboBoxComponentSelection->setEnabled(false);

	m_ui->frameSideA->setStyleSheet(".QFrame { background-color: #2f7dd4; }");
	m_ui->frameSideB->setStyleSheet(".QFrame { background-color: #ffce30; }");

	QPalette p;
	p.setColor(QPalette::Window, QColor(47,125,212));
	m_ui->frameSideA->setPalette(p);
	p.setColor(QPalette::Window, QColor(255, 206, 48));
	m_ui->frameSideB->setPalette(p);

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
		case BT_Components				: m_ui->stackedWidget->setCurrentIndex(1); break;
		case BT_SubSurfaceComponents	: m_ui->stackedWidget->setCurrentIndex(2); break;
		case BT_ComponentOrientation	: m_ui->stackedWidget->setCurrentIndex(3); break;
		case BT_BoundaryConditions		: m_ui->stackedWidget->setCurrentIndex(4); break;
		case BT_ZoneTemplates			: m_ui->stackedWidget->setCurrentIndex(5); break;
		case BT_FloorManager : break; // just to remove compiler warning, FloorManager is not handled here
	}
}


void SVPropBuildingEditWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// react on selection changes only, then update properties
	switch ((SVProjectHandler::ModificationTypes)modificationType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::BuildingTopologyChanged: // used when zone templates are assigned
		case SVProjectHandler::ComponentInstancesModified:
		case SVProjectHandler::NodeStateModified:

			updateUi(); // we do not change the property type here
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
	updateUi();
}



// *** Component property mode ***

void SVPropBuildingEditWidget::on_tableWidgetComponents_itemSelectionChanged() {
	// check if the table is empty or there is no currently selected row
	int r = m_ui->tableWidgetComponents->currentRow();
	if (r == -1 || m_componentSurfacesMap.empty()) {
		m_ui->pushButtonEditComponents->setEnabled(false);
		m_ui->pushButtonExchangeComponents->setEnabled(false);
		m_ui->pushButtonSelectObjectsWithComponent->setEnabled(false);
		return;
	}
	// enable/disable buttons that require valid components
	bool enabled = (currentlySelectedComponent() != nullptr);
	m_ui->pushButtonEditComponents->setEnabled(enabled);
	m_ui->pushButtonExchangeComponents->setEnabled(enabled);

	// the select buttons are always active, even if no component is assigned, yet
	m_ui->pushButtonSelectObjectsWithComponent->setEnabled(true);
}


void SVPropBuildingEditWidget::on_pushButtonEditComponents_clicked() {
	const VICUS::Component * comp = currentlySelectedComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	int currentRow = m_ui->tableWidgetComponents->currentRow();
	SVMainWindow::instance().dbComponentEditDialog()->edit(comp->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
	// reselect row, because closing the edit dialog recreates the table (due to possible color change)
	if (m_ui->tableWidgetComponents->rowCount() > currentRow)
		m_ui->tableWidgetComponents->selectRow(currentRow);
}


void SVPropBuildingEditWidget::on_pushButtonExchangeComponents_clicked() {
	const VICUS::Component * comp = currentlySelectedComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoReplaceComponent",
		tr("Replace component"), tr("This will replace all associations with component '%1 [%2]' with another component.")
			 .arg(QtExt::MultiLangString2QString(comp->m_displayName)).arg(comp->m_id));
	unsigned int oldId = comp->m_id;
	unsigned int newId = SVMainWindow::instance().dbComponentEditDialog()->select(oldId);
	if (newId == VICUS::INVALID_ID)
		return; // user has aborted the dialog

	// now compose an undo action and modify the project

	// first, we need to find the component instances which reference the "old" id
	comp = SVSettings::instance().m_db.m_components[oldId];
	if (comp == nullptr) {
		// the user has done something stupid and deleted the component that he wanted to replace. In this
		// case there is no longer a component with this ID and we have a nullptr
		QMessageBox::critical(this, tr("Replace component"), tr("Component with id %1 no longer found in component DB.").arg(oldId));
		return;
	}
	// now process all component instances and modify the component
	std::vector<VICUS::ComponentInstance> modCI = project().m_componentInstances;
	for (VICUS::ComponentInstance & ci : modCI) {
		if (ci.m_componentID == oldId)
			ci.m_componentID = (unsigned int)newId;
	}
	// create the undo action and modify project
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Components exchanged"), modCI);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonSelectObjectsWithComponent_clicked() {
	const VICUS::Component * comp = currentlySelectedComponent();
	Q_ASSERT(m_componentSurfacesMap.find(comp) != m_componentSurfacesMap.end());
	// compose set of objects to be selected
	std::set<unsigned int> objs;
	for (auto s : m_componentSurfacesMap[comp])
		objs.insert(s->uniqueID());

	QString undoText;
	if (comp != nullptr)
		undoText = tr("Select objects with component '%1'").arg(QtExt::MultiLangString2QString(comp->m_displayName));
	else
		undoText = tr("Select objects with invalid component.");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, objs, true);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonAssignComponent_clicked() {
	assignComponent(false);
}


void SVPropBuildingEditWidget::on_pushButtonAssignInsideComponent_clicked() {
	assignComponent(true);
}


// *** Sub-Surface component property mode ***

void SVPropBuildingEditWidget::on_tableWidgetSubSurfaceComponents_itemSelectionChanged() {
	// check if the table is empty or there is no currently selected row
	int r = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	if (r == -1 || m_subComponentSurfacesMap.empty()) {
		m_ui->pushButtonEditSubSurfaceComponents->setEnabled(false);
		m_ui->pushButtonExchangeSubSurfaceComponents->setEnabled(false);
		m_ui->pushButtonSelectObjectsWithSubSurfaceComponent->setEnabled(false);
		return;
	}
	// enable/disable buttons that require valid components
	bool enabled = (currentlySelectedSubSurfaceComponent() != nullptr);
	m_ui->pushButtonEditSubSurfaceComponents->setEnabled(enabled);
	m_ui->pushButtonExchangeSubSurfaceComponents->setEnabled(enabled);

	// the select buttons are always active, even if no component is assigned, yet
	m_ui->pushButtonSelectObjectsWithSubSurfaceComponent->setEnabled(true);
}


void SVPropBuildingEditWidget::on_pushButtonEditSubSurfaceComponents_clicked() {
	const VICUS::SubSurfaceComponent * comp = currentlySelectedSubSurfaceComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	int currentRow = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	SVMainWindow::instance().dbSubSurfaceComponentEditDialog()->edit(comp->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
	// reselect row, because closing the edit dialog recreates the table (due to possible color change)
	if (m_ui->tableWidgetSubSurfaceComponents->rowCount() > currentRow)
		m_ui->tableWidgetSubSurfaceComponents->selectRow(currentRow);
}


void SVPropBuildingEditWidget::on_pushButtonExchangeSubSurfaceComponents_clicked() {
	const VICUS::SubSurfaceComponent * comp = currentlySelectedSubSurfaceComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoReplaceComponent",
		tr("Replace component"), tr("This will replace all associations with component '%1 [%2]' with another component.")
			 .arg(QtExt::MultiLangString2QString(comp->m_displayName)).arg(comp->m_id));
	unsigned int oldId = comp->m_id;
	unsigned int newId = SVMainWindow::instance().dbSubSurfaceComponentEditDialog()->select(oldId);
	if (newId == VICUS::INVALID_ID)
		return; // user has aborted the dialog

	// now compose an undo action and modify the project

	// first, we need to find the component instances which reference the "old" id
	comp = SVSettings::instance().m_db.m_subSurfaceComponents[oldId];
	if (comp == nullptr) {
		// the user has done something stupid and deleted the component that he wanted to replace. In this
		// case there is no longer a component with this ID and we have a nullptr
		QMessageBox::critical(this, tr("Replace component"), tr("Sub-surface component with id %1 no longer found in DB.").arg(oldId));
		return;
	}
	// now process all component instances and modify the component
	std::vector<VICUS::SubSurfaceComponentInstance> modCI = project().m_subSurfaceComponentInstances;
	for (VICUS::SubSurfaceComponentInstance & ci : modCI) {
		if (ci.m_subSurfaceComponentID == oldId)
			ci.m_subSurfaceComponentID = (unsigned int)newId;
	}
	// create the undo action and modify project
	SVUndoModifySubSurfaceComponentInstances * undo = new SVUndoModifySubSurfaceComponentInstances(tr("Sub-surface components exchanged"), modCI);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonSelectObjectsWithSubSurfaceComponent_clicked() {
	const VICUS::SubSurfaceComponent * comp = currentlySelectedSubSurfaceComponent();
	Q_ASSERT(m_subComponentSurfacesMap.find(comp) != m_subComponentSurfacesMap.end());
	// compose set of objects to be selected
	std::set<unsigned int> objs;
	for (auto s : m_subComponentSurfacesMap[comp])
		objs.insert(s->uniqueID());

	QString undoText;
	if (comp != nullptr)
		undoText = tr("Select objects with sub-surface component '%1'").arg(QtExt::MultiLangString2QString(comp->m_displayName));
	else
		undoText = tr("Select objects with invalid sub-surface component.");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, objs, true);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonAssignSubSurfaceComponent_clicked() {
	assignSubSurfaceComponent(false);
}


void SVPropBuildingEditWidget::on_pushButtonAssignInsideSubSurfaceComponent_clicked() {
	assignSubSurfaceComponent(true);
}


// *** Component orientation property mode ***

void SVPropBuildingEditWidget::on_checkBoxShowAllComponentOrientations_toggled(bool checked) {
	m_ui->labelComponentSelection->setEnabled(!checked);
	m_ui->comboBoxComponentSelection->setEnabled(!checked);
	if (checked) {
		SVViewState vs = SVViewStateHandler::instance().viewState();
		vs.m_colorModePropertyID = VICUS::INVALID_ID; // disable filter
		SVViewStateHandler::instance().setViewState(vs);
	}
	else {
		on_comboBoxComponentSelection_currentIndexChanged(m_ui->comboBoxComponentSelection->currentIndex());
	}
}


void SVPropBuildingEditWidget::on_pushButtonAlignComponentToSideA_clicked() {
	alignSelectedComponents(true);
}


void SVPropBuildingEditWidget::on_pushButtonAlignComponentToSideB_clicked() {
	alignSelectedComponents(false);
}


void SVPropBuildingEditWidget::on_comboBoxComponentSelection_currentIndexChanged(int index) {
	// set index/id of selected component in view manager and update coloring
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_colorModePropertyID = m_ui->comboBoxComponentSelection->currentData().toUInt();
	SVViewStateHandler::instance().setViewState(vs);
	m_ui->comboBoxComponentSelection->blockSignals(true);
	m_ui->comboBoxComponentSelection->setCurrentIndex(index);
	m_ui->comboBoxComponentSelection->blockSignals(false);
}


// *** PRIVATE FUNCTIONS ***


const VICUS::Component * SVPropBuildingEditWidget::currentlySelectedComponent() const {
	// check if selected "component" is actually a missing component
	int r = m_ui->tableWidgetComponents->currentRow();
	if (r == -1 || m_componentSurfacesMap.empty())
		return nullptr;
	std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >::const_iterator cit = m_componentSurfacesMap.begin();
	std::advance(cit, r);
	return cit->first;
}


const VICUS::SubSurfaceComponent * SVPropBuildingEditWidget::currentlySelectedSubSurfaceComponent() const {
	// check if selected "sub-surface component" is actually missing
	int r = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	if (r == -1 || m_subComponentSurfacesMap.empty())
		return nullptr;
	std::map<const VICUS::SubSurfaceComponent*, std::vector<const VICUS::SubSurface *> >::const_iterator cit = m_subComponentSurfacesMap.begin();
	std::advance(cit, r);
	return cit->first;
}


const VICUS::ZoneTemplate * SVPropBuildingEditWidget::currentlySelectedZoneTemplate() const {
	// check if selected "template" is actually missing
	int r = m_ui->tableWidgetZoneTemplates->currentRow();
	if (r == -1 || m_zoneTemplateAssignments.empty())
		return nullptr;
	std::map<const VICUS::ZoneTemplate*, std::vector<const VICUS::Room *> >::const_iterator cit = m_zoneTemplateAssignments.begin();
	std::advance(cit, r);
	return cit->first;
}


void SVPropBuildingEditWidget::updateUi() {
	const SVDatabase & db = SVSettings::instance().m_db;

	// TODO Andreas : this function currently updates all widgets in the stacked widget, regardless of which
	//                is currently visible. This makes switching property modes very fast, but whenever the project
	//                data changes, it takes a bit more time. If this becomes a performance issue at some point,
	//                modify the update logic.

	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true); // just visible
	std::vector<const VICUS::Object * > selObjs;
	std::vector<const VICUS::Surface * > surfObjs;
	std::vector<const VICUS::SubSurface * > subSurfObjs;
	for (const VICUS::Object * o : objs) {
		if (!o->m_selected) continue;
		selObjs.push_back(o);
		const VICUS::Surface * surf = dynamic_cast<const VICUS::Surface *>(o);
		if (surf != nullptr)
			surfObjs.push_back(surf);
		const VICUS::SubSurface * subsurf = dynamic_cast<const VICUS::SubSurface *>(o);
		if (subsurf != nullptr)
			subSurfObjs.push_back(subsurf);
	}

	// ** Components data structure update **
	// now build a map of component IDs versus visible surfaces
	m_componentSurfacesMap.clear();
	m_selectedComponentInstances.clear();

	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// component ID assigned?
		if (ci.m_componentID == VICUS::INVALID_ID)
			continue; // no component, skip
		// lookup component in DB
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_componentID];
		if (comp == nullptr) {
			// invalid component ID... should we notify the user about that somehow?
			// for now we keep the nullptr and use this to identify "invalid component" in the table
		}

		// now test the surfaces associated with this component instance

		// side A
		if (ci.m_sideASurface != nullptr) {
			// is this surface visible? then it must be in the set 'obj'
			std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASurface);
			if (it_A != objs.end()) {
				m_componentSurfacesMap[comp].push_back(ci.m_sideASurface);
				if (ci.m_sideASurface->m_selected)
					m_selectedComponentInstances.insert(&ci);
			}
		}
		// side B
		if (ci.m_sideBSurface != nullptr) {
			std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
			if (it_B != objs.end()) {
				m_componentSurfacesMap[comp].push_back(ci.m_sideBSurface);
				if (ci.m_sideBSurface->m_selected)
					m_selectedComponentInstances.insert(&ci);
			}
		}
	}

	// ** Sub-Surface Components data structure update **
	m_subComponentSurfacesMap.clear();
	m_selectedSubComponentInstances.clear();

	for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
		// component ID assigned?
		if (ci.m_subSurfaceComponentID == VICUS::INVALID_ID)
			continue; // no component, skip
		// lookup component in DB
		const VICUS::SubSurfaceComponent * comp = SVSettings::instance().m_db.m_subSurfaceComponents[ci.m_subSurfaceComponentID];
		if (comp == nullptr) {
			// invalid component ID... should we notify the user about that somehow?
			// for now we keep the nullptr and use this to identify "invalid component" in the table
		}

		// now test the surfaces associated with this component instance

		// side A
		if (ci.m_sideASubSurface != nullptr) {
			// is this surface visible? then it must be in the set 'obj'
			std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASubSurface);
			if (it_A != objs.end()) {
				m_subComponentSurfacesMap[comp].push_back(ci.m_sideASubSurface);
				if (ci.m_sideASubSurface->m_selected)
					m_selectedSubComponentInstances.insert(&ci);
			}
		}
		// side B
		if (ci.m_sideBSubSurface != nullptr) {
			std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSubSurface);
			if (it_B != objs.end()) {
				m_subComponentSurfacesMap[comp].push_back(ci.m_sideBSubSurface);
				if (ci.m_sideBSubSurface->m_selected)
					m_selectedSubComponentInstances.insert(&ci);
			}
		}
	}

	m_zoneTemplateAssignments.clear();
	const VICUS::Database<VICUS::ZoneTemplate> & db_zt = SVSettings::instance().m_db.m_zoneTemplates;
	// loop over all rooms and store zone template associations
	for (const VICUS::Object * o : objs) {
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
		if (room == nullptr) continue; // skip all but rooms
		// skip rooms without zone template
		if (room->m_idZoneTemplate == VICUS::INVALID_ID)
			continue;
		// lookup zone template in DB
		const VICUS::ZoneTemplate * zt = db_zt[room->m_idZoneTemplate];
		// Note: might be a nullptr if id is invalid
		m_zoneTemplateAssignments[zt].push_back(room);
	}


	// *** Update Component Page ***
	{

		// now put the data of the map into the table
		int currentRow = m_ui->tableWidgetComponents->currentRow();
		m_ui->tableWidgetComponents->blockSignals(true);
		m_ui->tableWidgetComponents->clearContents();
		m_ui->tableWidgetComponents->setRowCount(m_componentSurfacesMap.size());
		int row=0;
		for (std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >::const_iterator
			 it = m_componentSurfacesMap.begin(); it != m_componentSurfacesMap.end(); ++it, ++row)
		{
			QTableWidgetItem * item = new QTableWidgetItem();
			// special handling for components with "invalid" component id
			if (it->first == nullptr)
				item->setBackground(QColor(255,128,128));
			else
				item->setBackground(it->first->m_color);
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetComponents->setItem(row, 0, item);

			item = new QTableWidgetItem();
			if (it->first == nullptr)
				item->setText(tr("<invalid component id>"));
			else
				item->setText(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetComponents->setItem(row, 1, item);
		}
		// reselect row
		m_ui->tableWidgetComponents->blockSignals(false);
		m_ui->tableWidgetComponents->selectRow(std::min(currentRow, m_ui->tableWidgetComponents->rowCount()-1));
		// process all selected surfaces and determine which component they have assigned
		if (surfObjs.empty()) {
			m_ui->labelSelectedComponents->setText("");
			m_ui->groupBoxSelectedComponent->setEnabled(false);
		}
		else {
			m_ui->groupBoxSelectedComponent->setEnabled(true);
		}
		m_ui->pushButtonAssignInsideComponent->setEnabled(surfObjs.size() == 2);

		// update selection-related info
		std::set<const VICUS::Component *> selectedComponents;
		for (const VICUS::Surface* s : surfObjs) {
			if (s->m_componentInstance != nullptr) {
				const VICUS::Component * surfcomp = db.m_components[s->m_componentInstance->m_componentID];
				selectedComponents.insert(surfcomp);
			}
		}
		if (selectedComponents.empty()) {
			m_ui->labelSelectedComponents->setText(tr("None"));
		}
		else if (selectedComponents.size() == 1) {
			if (*selectedComponents.begin() == nullptr)
				m_ui->labelSelectedComponents->setText(tr("Component with invalid/unknown ID"));
			else
				m_ui->labelSelectedComponents->setText(tr("%1 [%2]")
				   .arg(QtExt::MultiLangString2QString((*selectedComponents.begin())->m_displayName)).arg((*selectedComponents.begin())->m_id));
		}
		else {
			m_ui->labelSelectedComponents->setText(tr("%1 different components")
			   .arg(selectedComponents.size()));
		}
		// update table related button states
		on_tableWidgetComponents_itemSelectionChanged();
	}


	// *** Update Sub-Surface Component Page ***

	{
		// now put the data of the map into the table
		int currentRow = m_ui->tableWidgetSubSurfaceComponents->currentRow();
		m_ui->tableWidgetSubSurfaceComponents->blockSignals(true);
		m_ui->tableWidgetSubSurfaceComponents->clearContents();
		m_ui->tableWidgetSubSurfaceComponents->setRowCount(m_subComponentSurfacesMap.size());
		int row=0;
		for (std::map<const VICUS::SubSurfaceComponent*, std::vector<const VICUS::SubSurface *> >::const_iterator
			 it = m_subComponentSurfacesMap.begin(); it != m_subComponentSurfacesMap.end(); ++it, ++row)
		{
			QTableWidgetItem * item = new QTableWidgetItem();
			// special handling for components with "invalid" component id
			if (it->first == nullptr)
				item->setBackground(QColor(255,128,128));
			else
				item->setBackground(it->first->m_color);
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetSubSurfaceComponents->setItem(row, 0, item);

			// insert type
			item = new QTableWidgetItem();
			if (it->first == nullptr) {
				item->setText(tr("<invalid>"));
			}
			else {
				switch (it->first->m_type) {
					case VICUS::SubSurfaceComponent::CT_Window : item->setText(tr("Window")); break;
					case VICUS::SubSurfaceComponent::CT_Door : item->setText(tr("Door")); break;
					case VICUS::SubSurfaceComponent::CT_Miscellaneous : item->setText(tr("Miscellaneous")); break;
					default: item->setText(tr("<invalid>"));
				}
			}
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetSubSurfaceComponents->setItem(row, 1, item);

			item = new QTableWidgetItem();
			if (it->first == nullptr)
				item->setText(tr("<invalid sub-surface component id>"));
			else
				item->setText(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetSubSurfaceComponents->setItem(row, 2, item);
		}
		// reselect row
		m_ui->tableWidgetSubSurfaceComponents->blockSignals(false);
		m_ui->tableWidgetSubSurfaceComponents->selectRow(std::min(currentRow, m_ui->tableWidgetSubSurfaceComponents->rowCount()-1));
		// process all selected sub surfaces and determine which component they have assigned
		if (subSurfObjs.empty()) {
			m_ui->labelSelectedSubSurfaceComponents->setText("");
			m_ui->groupBoxSelectedSubComponent->setEnabled(false);
		}
		else {
			m_ui->groupBoxSelectedSubComponent->setEnabled(true);
		}
		m_ui->pushButtonAssignInsideSubSurfaceComponent->setEnabled(subSurfObjs.size() == 2);

		// update selection-related info
		std::set<const VICUS::SubSurfaceComponent *> selectedComponents;
		for (const VICUS::SubSurface* s : subSurfObjs) {
			if (s->m_subSurfaceComponentInstance != nullptr) {
				const VICUS::SubSurfaceComponent * surfcomp = db.m_subSurfaceComponents[s->m_subSurfaceComponentInstance->m_subSurfaceComponentID];
				selectedComponents.insert(surfcomp);
			}
		}
		if (selectedComponents.empty()) {
			m_ui->labelSelectedSubSurfaceComponents->setText(tr("None"));
		}
		else if (selectedComponents.size() == 1) {
			if (*selectedComponents.begin() == nullptr)
				m_ui->labelSelectedSubSurfaceComponents->setText(tr("Sub-surface component with invalid/unknown ID"));
			else
				m_ui->labelSelectedSubSurfaceComponents->setText(tr("%1 [%2]")
				   .arg(QtExt::MultiLangString2QString((*selectedComponents.begin())->m_displayName)).arg((*selectedComponents.begin())->m_id));
		}
		else {
			m_ui->labelSelectedSubSurfaceComponents->setText(tr("%1 different sub-surface components")
			   .arg(selectedComponents.size()));
		}
		// update table related button states
		on_tableWidgetSubSurfaceComponents_itemSelectionChanged();
	}


	// *** Update ComponentOrientation Page ***

	m_ui->comboBoxComponentSelection->blockSignals(true);
	m_ui->comboBoxComponentSelection->clear();
	for (std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >::const_iterator
		 it = m_componentSurfacesMap.begin(); it != m_componentSurfacesMap.end(); ++it)
	{
		if (it->first == nullptr)
			continue;
		m_ui->comboBoxComponentSelection->addItem(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")), it->first->m_id);
	}
	m_ui->comboBoxComponentSelection->setCurrentIndex(m_ui->comboBoxComponentSelection->count()-1);
	m_ui->comboBoxComponentSelection->blockSignals(false);

	// selection-related info
	if (m_selectedComponentInstances.empty()) {
		m_ui->labelComponentOrientationInfo->setText(tr("No surfaces with components selected"));
		m_ui->pushButtonAlignComponentToSideA->setEnabled(false);
		m_ui->pushButtonAlignComponentToSideB->setEnabled(false);
	}
	else {
		m_ui->labelComponentOrientationInfo->setText(tr("%1 surfaces with components selected").arg(m_selectedComponentInstances.size()));
		m_ui->pushButtonAlignComponentToSideA->setEnabled(true);
		m_ui->pushButtonAlignComponentToSideB->setEnabled(true);
	}


	// *** Update BoundaryCondition Page ***

	{
		// now build a map of component IDs versus visible surfaces
		m_bcSurfacesMap.clear();
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			// component ID assigned?
			if (ci.m_componentID == VICUS::INVALID_ID)
				continue; // no component, skip
			// lookup component in DB
			const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_componentID];
			const VICUS::BoundaryCondition * bcSideA = nullptr;
			const VICUS::BoundaryCondition * bcSideB = nullptr;
			if (comp != nullptr) {
				// lookup boundary condition pointers
				if (comp->m_idSideABoundaryCondition != VICUS::INVALID_ID)
					bcSideA = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
				if (comp->m_idSideBBoundaryCondition != VICUS::INVALID_ID)
					bcSideB = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
			}
			// side A
			if (ci.m_sideASurface != nullptr) {
				std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASurface);
				if (it_A != objs.end()) {
					m_bcSurfacesMap[bcSideA].push_back(ci.m_sideASurface);
				}
			}
			// side B
			if (ci.m_sideBSurface != nullptr) {
				std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
				if (it_B != objs.end())
					m_bcSurfacesMap[bcSideB].push_back(ci.m_sideBSurface);
			}
		}
		// now put the data of the map into the table
		int currentRow = m_ui->tableWidgetBoundaryConditions->currentRow();
		m_ui->tableWidgetBoundaryConditions->blockSignals(true);
		m_ui->tableWidgetBoundaryConditions->clearContents();
		m_ui->tableWidgetBoundaryConditions->setRowCount(m_bcSurfacesMap.size());
		int row=0;
		for (std::map<const VICUS::BoundaryCondition*, std::vector<const VICUS::Surface *> >::const_iterator
			 it = m_bcSurfacesMap.begin(); it != m_bcSurfacesMap.end(); ++it, ++row)
		{
			QTableWidgetItem * item = new QTableWidgetItem();
			// special handling for surfaces without bc assigned
			if (it->first == nullptr)
				item->setBackground(QColor(64,64,64));
			else
				item->setBackground(it->first->m_color);
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetBoundaryConditions->setItem(row, 0, item);

			item = new QTableWidgetItem();
			if (it->first == nullptr)
				item->setText(tr("<no/invalid boundary condition>"));
			else
				item->setText(QtExt::MultiLangString2QString(it->first->m_displayName) );
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetBoundaryConditions->setItem(row, 1, item);
		}
		// reselect row
		m_ui->tableWidgetBoundaryConditions->blockSignals(false);
		m_ui->tableWidgetBoundaryConditions->selectRow(std::min(currentRow, m_ui->tableWidgetBoundaryConditions->rowCount()-1));
	}


	// *** Update ZoneTemplates Page ***

	{
		// now put the data of the map into the table
		int currentRow = m_ui->tableWidgetZoneTemplates->currentRow();
		m_ui->tableWidgetZoneTemplates->blockSignals(true);
		m_ui->tableWidgetZoneTemplates->clearContents();
		m_ui->tableWidgetZoneTemplates->setRowCount(m_zoneTemplateAssignments.size());
		int row=0;
		for (std::map<const VICUS::ZoneTemplate*, std::vector<const VICUS::Room *> >::const_iterator
			 it = m_zoneTemplateAssignments.begin(); it != m_zoneTemplateAssignments.end(); ++it, ++row)
		{
			QTableWidgetItem * item = new QTableWidgetItem();
			// special handling for zone template with "invalid" id
			if (it->first == nullptr)
				item->setBackground(QColor(255,128,128));
			else
				item->setBackground(it->first->m_color);
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetZoneTemplates->setItem(row, 0, item);

			item = new QTableWidgetItem();
			if (it->first == nullptr)
				item->setText(tr("<invalid zone template id>"));
			else
				item->setText(QtExt::MultiLangString2QString(it->first->m_displayName) );
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetZoneTemplates->setItem(row, 1, item);
		}
		m_ui->tableWidgetZoneTemplates->blockSignals(false);
		m_ui->tableWidgetZoneTemplates->selectRow(std::min(currentRow, m_ui->tableWidgetZoneTemplates->rowCount()-1));

		// process all selected rooms and determine which zone template they have assigned
		std::vector<const VICUS::Room*> rooms;
		project().selectedRooms(rooms);
		if (rooms.empty()) {
			m_ui->labelSelectedZoneTemplates->setText("");
			m_ui->groupBoxSelectedRooms->setEnabled(false);
		}
		else {
			m_ui->groupBoxSelectedRooms->setEnabled(true);
		}

		// update selection-related info
		std::set<const VICUS::ZoneTemplate *> selectedZoneTemplate;
		// loop over all selected rooms and store pointer to assigned zone template
		for (const VICUS::Room* r : rooms) {
			if (r->m_idZoneTemplate != VICUS::INVALID_ID) {
				const VICUS::ZoneTemplate * zt= db.m_zoneTemplates[r->m_idZoneTemplate];
				selectedZoneTemplate.insert(zt); // when ID is invalid/unknown, we store a nullptr
			}
		}
		if (selectedZoneTemplate.empty()) {
			m_ui->labelSelectedZoneTemplates->setText(tr("None"));
		}
		else if (selectedZoneTemplate.size() == 1) {
			const VICUS::ZoneTemplate * zt = *selectedZoneTemplate.begin();
			// special handling: exactly one room with invalid zone template ID is selected
			if (zt == nullptr)
				m_ui->labelSelectedZoneTemplates->setText(tr("Zone template with invalid/unknown ID"));
			else // otherwise show info about the selected zone template
				m_ui->labelSelectedZoneTemplates->setText(tr("%1 [%2]")
				   .arg(QtExt::MultiLangString2QString(zt->m_displayName)).arg(zt->m_id) );
		}
		else {
			m_ui->labelSelectedZoneTemplates->setText(tr("%1 different templates")
			   .arg(selectedZoneTemplate.size()));
		}
		// update table related button states
		on_tableWidgetZoneTemplates_itemSelectionChanged();
	}
}


void SVPropBuildingEditWidget::alignSelectedComponents(bool toSideA) {
	// create a copy of the component instances
	std::vector<VICUS::ComponentInstance> compInstances = project().m_componentInstances;

	std::set<unsigned int> surfacesToDDeselect;
	// loop over all components and look for a selected side - if there is more than one side of a component
	// instance selected, show an error message
	for (const VICUS::ComponentInstance * c : m_selectedComponentInstances) {
		// both sides selected?
		bool sideASelected = (c->m_sideASurface != nullptr && c->m_sideASurface->m_selected);
		bool sideBSelected = (c->m_sideBSurface != nullptr && c->m_sideBSurface->m_selected);
		if (sideASelected && sideBSelected) {
			QMessageBox::critical(this, QString(), tr("You must not select both surfaces of the same component!"));
			return;
		}
		// now lookup copied componentInstance by ID and swap sides if:
		// - the selected side is side B and should be switched to side A
		// - the selected side is side A and should be switched to side B
		if ((toSideA && sideBSelected) ||
			(!toSideA && sideASelected))
		{
			std::vector<VICUS::ComponentInstance>::iterator it = std::find(compInstances.begin(), compInstances.end(), c->m_id);
			Q_ASSERT(it != compInstances.end());

			if (sideASelected)
				surfacesToDDeselect.insert(it->m_sideASurface->uniqueID()); // Mind: use uniqueID here for selection change!
			if (sideBSelected)
				surfacesToDDeselect.insert(it->m_sideBSurface->uniqueID());
			std::swap(it->m_sideASurfaceID, it->m_sideBSurfaceID);
		}
	}

	// if there was no change, inform the user and abort
	if (surfacesToDDeselect.empty()) {
		QMessageBox::information(this, QString(), tr("All of the selected surfaces are already aligned as requested."));
		return;
	}

	// compose undo action
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Aligning component sides"), compInstances);
	undo->push(); // Note: this invalidates all pointers in the project, calls onModified() and rebuilds our maps

	// now deselect all surfaces that have been touched
	SVUndoTreeNodeState * undoSelect = new SVUndoTreeNodeState(tr("Deselecting modified surfaces"),
															   SVUndoTreeNodeState::SelectedState, surfacesToDDeselect, false);
	undoSelect->push();
}


void SVPropBuildingEditWidget::zoneTemplateVisibilityChanged() {
	// set currently selected zone template ID in view state and trigger a recoloring
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (m_ui->checkBoxZoneTemplateColorOnlyActive->isChecked()) {
		const VICUS::ZoneTemplate * selectedZoneTemplate = currentlySelectedZoneTemplate();
		if (selectedZoneTemplate == nullptr)
			vs.m_colorModePropertyID = VICUS::INVALID_ID;
		else
			vs.m_colorModePropertyID = selectedZoneTemplate->m_id;
	}
	else {
		// disable coloring filter
		vs.m_colorModePropertyID = VICUS::INVALID_ID;
	}
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropBuildingEditWidget::zoneTemplateSelectionChanged() {
	// do nothing, if checkbox isn't active; that also means that when the checkbox is unchecked,
	// we will not automatically show all hidden objects again
	if (!m_ui->checkBoxZoneTemplateShowOnlyActive->isChecked())
		return;

	// compose node states for all rooms and their surfaces based on template association

	// get currently selected zone template
	const VICUS::ZoneTemplate * zt = currentlySelectedZoneTemplate();

	// if not a valid template, do nothing here
	if (zt == nullptr)
		return;

	// compose a list of unique room IDs and respective on/off visibility states
	std::set<unsigned int> nodeIDs;

	const VICUS::Project & p = project();

	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				// skip all rooms that do not have the same zone template as we have
				if (r.m_idZoneTemplate != zt->m_id) continue;
				nodeIDs.insert(r.uniqueID()); // Mind: unique IDs!
				for (const VICUS::Surface & s : r.m_surfaces)
					nodeIDs.insert(s.uniqueID()); // Mind: unique IDs!
			}
		}
	}

	// trigger undo-action
	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Exclusive selection by zone template"), SVUndoTreeNodeState::VisibilityState, nodeIDs, true, true);
	undo->push();
}


void SVPropBuildingEditWidget::assignComponent(bool insideWall) {
	// ask user to select a new component
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignComponent",
		tr("Assign component"), tr("You may now select a component from the database, which will then be "
								   "assigned to the selected surfaces."));
	unsigned int selectedComponentId = SVMainWindow::instance().dbComponentEditDialog()->select(0);
	if (selectedComponentId == VICUS::INVALID_ID)
		return; // user aborted the dialog

	std::vector<const VICUS::Surface*> surfaces;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_Building);
	Q_ASSERT(!surfaces.empty());
	std::set<const VICUS::Surface*> selSurfaces(surfaces.begin(), surfaces.end());


	std::vector<VICUS::ComponentInstance> compInstances;
	if (insideWall) {
		Q_ASSERT(surfaces.size() == 2); // must have exactly two
		// we handle the following cases
		// - both surfaces are not yet used in a componentInstance -> we create a new one
		// - both surfaces are already used in the same componentInstance -> we just replace the component
		// - both surfaces are already used in different componentInstances -> we modify the first and delete the second
		// - only one surface is already used in a componentInstance -> we modify it

		// we achieve all that with just one loop
		bool compInstanceFound = false;
		// process all component instances
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			// does the component instance reference either of our selected surfaces/or both?
			std::set<const VICUS::Surface*>::iterator it = selSurfaces.find(ci.m_sideASurface);
			std::set<const VICUS::Surface*>::iterator it2 = selSurfaces.find(ci.m_sideBSurface);
			if (it != selSurfaces.end() || it2 != selSurfaces.end()) {
				// yes, but did we already have a component instance with these surfaces?
				if (compInstanceFound) {
					// already handled, so we can just skip this component instance
					continue;
				}
				// remember, that we handled a componentInstance already
				compInstanceFound = true;
				VICUS::ComponentInstance newCi(ci);
				// set the selected component
				newCi.m_componentID = (unsigned int)selectedComponentId;
				// no, first time a component instance references the selected surfaces, modify it such,
				// that any original assignment remains untouched
				if (it != selSurfaces.end()) {
					// side A connected?, set side B to the other surface
					if (it == selSurfaces.begin())
						newCi.m_sideBSurfaceID = (*selSurfaces.rbegin())->m_id;
					else
						newCi.m_sideBSurfaceID = (*selSurfaces.begin())->m_id;
				}
				else {
					// must be side B connected, set side A to the other surface
					if (it2 == selSurfaces.begin())
						newCi.m_sideASurfaceID = (*selSurfaces.rbegin())->m_id;
					else
						newCi.m_sideASurfaceID = (*selSurfaces.begin())->m_id;
				}
				// remember modified component instance
				compInstances.push_back(newCi);
			}
			else {
				// not related to our surfaces, just dump to the new compInstances vector
				compInstances.push_back(ci);
			}
		}
		// if we didn't have a component instance yet, create a new one
		if (!compInstanceFound) {
			VICUS::ComponentInstance newCi;
			unsigned int nextId = VICUS::Project::largestUniqueId(compInstances);
			newCi.m_id = nextId;
			newCi.m_componentID = (unsigned int)selectedComponentId;
			newCi.m_sideASurfaceID = (*selSurfaces.begin())->m_id;
			newCi.m_sideBSurfaceID = (*selSurfaces.rbegin())->m_id;
			compInstances.push_back(newCi);
		}

	}
	else {
		// now create a copy of the currently defined componentInstances
		compInstances = project().m_componentInstances;

		// process all componentInstances and for all that reference any of the selected surfaces, replace component
		for (VICUS::ComponentInstance & ci : compInstances) {
			std::set<const VICUS::Surface*>::iterator it = selSurfaces.find(ci.m_sideASurface);
			if (it != selSurfaces.end()) {
				ci.m_componentID = (unsigned int)selectedComponentId;
				selSurfaces.erase(it);
				continue;
			}
			it = selSurfaces.find(ci.m_sideBSurface);
			if (it != selSurfaces.end()) {
				ci.m_componentID = (unsigned int)selectedComponentId;
				selSurfaces.erase(it);
			}
		}

		// now all surfaces still left in selSurfaces are not yet referenced in ComponentInstances
		// hence, we need to create a new componentInstance for each of them
		if (!selSurfaces.empty()) {
			unsigned int nextId = VICUS::Project::largestUniqueId(compInstances);
			for (const VICUS::Surface * s : selSurfaces) {
				VICUS::ComponentInstance c;
				c.m_id = nextId++;
				c.m_sideASurfaceID = s->m_id;
				c.m_componentID = (unsigned int)selectedComponentId;
				compInstances.push_back(c);
			}
		}

	}
	// create the undo action and modify project
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Components assigned"), compInstances);
	undo->push();
}


void SVPropBuildingEditWidget::assignSubSurfaceComponent(bool connectTwoSurfaces) {
	// ask user to select a new component
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignComponent",
		tr("Assign component"), tr("You may now select a component from the database, which will then be "
								   "assigned to the selected surfaces."));
	unsigned int selectedComponentId = SVMainWindow::instance().dbSubSurfaceComponentEditDialog()->select(0);
	if (selectedComponentId == VICUS::INVALID_ID)
		return; // user aborted the dialog

	std::set<const VICUS::Object*> selObjs;
	project().selectObjects(selObjs, VICUS::Project::SG_Building, true, true);
	std::set<const VICUS::SubSurface*> surfaces;
	for (const VICUS::Object* o : selObjs) {
		const VICUS::SubSurface* surf = dynamic_cast<const VICUS::SubSurface*>(o);
		if (surf != nullptr)
			surfaces.insert(surf);
	}

	Q_ASSERT(!surfaces.empty());

	std::vector<VICUS::SubSurfaceComponentInstance> compInstances;
	if (connectTwoSurfaces) {
		Q_ASSERT(surfaces.size() == 2); // must have exactly two
		// we handle the following cases
		// - both surfaces are not yet used in a componentInstance -> we create a new one
		// - both surfaces are already used in the same componentInstance -> we just replace the component
		// - both surfaces are already used in different componentInstances -> we modify the first and delete the second
		// - only one surface is already used in a componentInstance -> we modify it

		// we achieve all that with just one loop
		bool compInstanceFound = false;
		// process all component instances
		for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
			// does the component instance reference either of our selected surfaces/or both?
			std::set<const VICUS::SubSurface*>::iterator it = surfaces.find(ci.m_sideASubSurface);
			std::set<const VICUS::SubSurface*>::iterator it2 = surfaces.find(ci.m_sideBSubSurface);
			if (it != surfaces.end() || it2 != surfaces.end()) {
				// yes, but did we already have a component instance with these surfaces?
				if (compInstanceFound) {
					// already handled, so we can just skip this component instance
					continue;
				}
				// remember, that we handled a componentInstance already
				compInstanceFound = true;
				VICUS::SubSurfaceComponentInstance newCi(ci);
				// set the selected component
				newCi.m_subSurfaceComponentID = (unsigned int)selectedComponentId;
				// no, first time a component instance references the selected surfaces, modify it such,
				// that any original assignment remains untouched
				if (it != surfaces.end()) {
					// side A connected?, set side B to the other surface
					if (it == surfaces.begin())
						newCi.m_sideBSurfaceID = (*surfaces.rbegin())->m_id;
					else
						newCi.m_sideBSurfaceID = (*surfaces.begin())->m_id;
				}
				else {
					// must be side B connected, set side A to the other surface
					if (it2 == surfaces.begin())
						newCi.m_sideASurfaceID = (*surfaces.rbegin())->m_id;
					else
						newCi.m_sideASurfaceID = (*surfaces.begin())->m_id;
				}
				// remember modified component instance
				compInstances.push_back(newCi);
			}
			else {
				// not related to our surfaces, just dump to the new compInstances vector
				compInstances.push_back(ci);
			}
		}
		// if we didn't have a component instance yet, create a new one
		if (!compInstanceFound) {
			VICUS::SubSurfaceComponentInstance newCi;
			unsigned int nextId = VICUS::Project::largestUniqueId(compInstances);
			newCi.m_id = nextId;
			newCi.m_subSurfaceComponentID = (unsigned int)selectedComponentId;
			newCi.m_sideASurfaceID = (*surfaces.begin())->m_id;
			newCi.m_sideBSurfaceID = (*surfaces.rbegin())->m_id;
			compInstances.push_back(newCi);
		}

	}
	else {
		// now create a copy of the currently defined componentInstances
		compInstances = project().m_subSurfaceComponentInstances;

		// process all componentInstances and for all that reference any of the selected surfaces, replace component
		for (VICUS::SubSurfaceComponentInstance & ci : compInstances) {
			std::set<const VICUS::SubSurface*>::iterator it = surfaces.find(ci.m_sideASubSurface);
			if (it != surfaces.end()) {
				ci.m_subSurfaceComponentID = (unsigned int)selectedComponentId;
				surfaces.erase(it);
				continue;
			}
			it = surfaces.find(ci.m_sideBSubSurface);
			if (it != surfaces.end()) {
				ci.m_subSurfaceComponentID = (unsigned int)selectedComponentId;
				surfaces.erase(it);
			}
		}

		// now all surfaces still left in selSurfaces are not yet referenced in ComponentInstances
		// hence, we need to create a new componentInstance for each of them
		if (!surfaces.empty()) {
			unsigned int nextId = VICUS::Project::largestUniqueId(compInstances);
			for (const VICUS::SubSurface * s : surfaces) {
				VICUS::SubSurfaceComponentInstance c;
				c.m_id = nextId++;
				c.m_sideASurfaceID = s->m_id;
				c.m_subSurfaceComponentID = (unsigned int)selectedComponentId;
				compInstances.push_back(c);
			}
		}

	}
	// create the undo action and modify project
	SVUndoModifySubSurfaceComponentInstances * undo = new SVUndoModifySubSurfaceComponentInstances(tr("Sub-surface components assigned"), compInstances);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonAssignZoneTemplate_clicked() {
	// ask user to select a the zone template to assign
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignZoneTemplate",
		tr("Assign zone template"), tr("You may now select a zone template from the database, which will then be "
								   "assigned to the selected rooms."));
	unsigned int selectedId = SVMainWindow::instance().dbZoneTemplateEditDialog()->select(0);
	if (selectedId == VICUS::INVALID_ID)
		return; // user aborted the dialog

	// get all visible _and_ selected "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, true, true);

	std::vector<unsigned int> modifiedRoomIDs; // unique IDs!!!
	// loop over all rooms and store zone template associations
	for (const VICUS::Object * o : objs) {
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
		if (room == nullptr) continue; // skip all but rooms
		modifiedRoomIDs.push_back(room->uniqueID());
	}
	// now create an undo action for modifying zone template assignments
	SVUndoModifyRoomZoneTemplateAssociation * undo = new SVUndoModifyRoomZoneTemplateAssociation(
				tr("Assigned zone template"),
				modifiedRoomIDs, selectedId);
	undo->push();
}


void SVPropBuildingEditWidget::on_tableWidgetZoneTemplates_itemSelectionChanged() {
	// enable/disable buttons based on selection changed signal
	bool enabled = (currentlySelectedZoneTemplate() != nullptr);
	m_ui->pushButtonEditZoneTemplates->setEnabled(enabled);
	m_ui->pushButtonExchangeZoneTemplates->setEnabled(enabled);
}


void SVPropBuildingEditWidget::on_pushButtonEditZoneTemplates_clicked() {
	const VICUS::ZoneTemplate * zt = currentlySelectedZoneTemplate();
	Q_ASSERT(zt != nullptr); // if nullptr, the button should be disabled!
	SVMainWindow::instance().dbZoneTemplateEditDialog()->edit(zt->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
}


void SVPropBuildingEditWidget::on_pushButtonExchangeZoneTemplates_clicked() {
	const VICUS::ZoneTemplate * zt = currentlySelectedZoneTemplate();
	Q_ASSERT(zt != nullptr); // if nullptr, the button should be disabled!
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoReplaceComponent",
		tr("Replace template"), tr("This will replace all associations with zone template '%1 [%2]' with another template.")
			 .arg(QtExt::MultiLangString2QString(zt->m_displayName)).arg(zt->m_id));
	unsigned int oldId = zt->m_id;
	unsigned int newId = SVMainWindow::instance().dbZoneTemplateEditDialog()->select(oldId);
	if (newId == VICUS::INVALID_ID)
		return; // user has aborted the dialog

	// now compose an undo action and modify the project

	// first, we need to find the component instances which reference the "old" id
	zt = SVSettings::instance().m_db.m_zoneTemplates[oldId];
	if (zt == nullptr) {
		// the user has done something stupid and deleted the component that he wanted to replace. In this
		// case there is no longer a component with this ID and we have a nullptr
		QMessageBox::critical(this, tr("Replace template"), tr("Zone template with id %1 no longer found in DB.").arg(oldId));
		return;
	}

	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true);

	std::vector<unsigned int> modifiedRoomIDs; // unique IDs!!!
	// loop over all rooms and if their current zone template matches the oldId, remember the room to be modified
	for (const VICUS::Object * o : objs) {
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(o);
		if (room == nullptr) continue; // skip all but rooms
		if (room->m_idZoneTemplate == oldId)
			modifiedRoomIDs.push_back(room->uniqueID()); // Mind: unique IDs!
	}


	// create the undo action and modify project
	// now create an undo action for modifying zone template assignments
	SVUndoModifyRoomZoneTemplateAssociation * undo = new SVUndoModifyRoomZoneTemplateAssociation(
				tr("Exchange zone template"),
				modifiedRoomIDs, newId);
	undo->push();
}



void SVPropBuildingEditWidget::on_checkBoxZoneTemplateColorOnlyActive_toggled(bool) {
	zoneTemplateVisibilityChanged();
}


void SVPropBuildingEditWidget::on_checkBoxZoneTemplateShowOnlyActive_toggled(bool) {
	zoneTemplateSelectionChanged();
}


void SVPropBuildingEditWidget::on_tableWidgetZoneTemplates_itemClicked(QTableWidgetItem *) {
	if (m_ui->checkBoxZoneTemplateColorOnlyActive->isChecked())
		zoneTemplateVisibilityChanged();
	if (m_ui->checkBoxZoneTemplateShowOnlyActive->isChecked())
		zoneTemplateSelectionChanged();
}


