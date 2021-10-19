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
#include <QDialogButtonBox>

#include <VICUS_Component.h>
#include <VICUS_utilities.h>

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
#include "SVPropSurfaceHeatingDelegate.h"
#include "SVZoneSelectionDialog.h"
#include "SVUndoModifyComponentInstances.h"

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
	m_ui->verticalLayoutInterlinkedSurfaces->setMargin(0);
	m_ui->verticalLayoutZoneTemplates->setMargin(0);
	m_ui->verticalLayoutSurfaceHeating->setMargin(0);

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

	m_ui->tableWidgetInterlinkedSurfaces->setColumnCount(5);
	m_ui->tableWidgetInterlinkedSurfaces->setHorizontalHeaderLabels(QStringList() << tr("") << tr("CI id") << tr("Surface Side A") << tr("Surface Side B") << tr("Component") );
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetInterlinkedSurfaces);
	m_ui->tableWidgetInterlinkedSurfaces->setSortingEnabled(false);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(0,10);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(1,40);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(2,100);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(3,100);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->resizeSection(4,100);
//	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
//	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
//	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
	m_ui->tableWidgetInterlinkedSurfaces->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetInterlinkedSurfaces->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ui->tableWidgetInterlinkedSurfaces->setSelectionBehavior(QAbstractItemView::SelectItems);

	m_ui->tableWidgetSurfaceHeating->setColumnCount(6);
	m_ui->tableWidgetSurfaceHeating->setHorizontalHeaderLabels(QStringList() << QString() << QString() << tr("Heating") << tr("Control zone") << tr("Surfaces, side A/B") << tr("Network"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSurfaceHeating);
	m_ui->tableWidgetSurfaceHeating->setSortingEnabled(false);
	m_ui->tableWidgetSurfaceHeating->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetSurfaceHeating->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	m_ui->tableWidgetSurfaceHeating->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetSurfaceHeating->horizontalHeader()->resizeSection(1,20);
	m_ui->tableWidgetSurfaceHeating->horizontalHeader()->resizeSection(2,100);
	m_ui->tableWidgetSurfaceHeating->horizontalHeader()->resizeSection(3,120);
	m_ui->tableWidgetSurfaceHeating->horizontalHeader()->resizeSection(4,120);
	m_ui->tableWidgetSurfaceHeating->horizontalHeader()->setStretchLastSection(true);
	m_ui->tableWidgetSurfaceHeating->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ui->tableWidgetSurfaceHeating->setSelectionBehavior(QAbstractItemView::SelectRows);

	// Mind: parent of the item delegate must be its widget!
	m_ui->tableWidgetSurfaceHeating->setItemDelegate(new SVPropSurfaceHeatingDelegate(m_ui->tableWidgetSurfaceHeating));

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

	m_ui->lineEditConnectSurfacesMaxDistance->setup(0, 10000, tr("Maximum distances between surfaces to enable connection in [m]."),
												 false, true);
	m_ui->lineEditConnectSurfacesMaxDistance->setValue(0.8);
	m_ui->lineEditConnectSurfacesMaxAngle->setup(0, 45, tr("Maximum angle between surfaces to enable connection in [Deg]."),
												 false, true);
	m_ui->lineEditConnectSurfacesMaxAngle->setValue(5);

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
		case BT_InterlinkedSurfaces		: m_ui->stackedWidget->setCurrentIndex(5); break;
		case BT_ZoneTemplates			: m_ui->stackedWidget->setCurrentIndex(6); break;
		case BT_SurfaceHeating			: m_ui->stackedWidget->setCurrentIndex(7); break;
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
		case SVProjectHandler::SubSurfaceComponentInstancesModified:
		case SVProjectHandler::NodeStateModified:

			updateUi(); // we do not change the property type here
		break;

		case SVProjectHandler::ObjectRenamed: // we only show zone names in surface heating
			updateSurfaceHeatingPage();
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
		m_ui->pushButtonAssignSelectedComponent->setEnabled(false);
		m_ui->pushButtonSelectObjectsWithComponent->setEnabled(false);
		return;
	}
	// enable/disable buttons that require valid components
	bool enabled = (currentlySelectedComponent() != nullptr);
	m_ui->pushButtonEditComponents->setEnabled(enabled);
	m_ui->pushButtonExchangeComponents->setEnabled(enabled);
	m_ui->pushButtonAssignSelectedComponent->setEnabled(enabled);

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
		if (ci.m_idComponent == oldId)
			ci.m_idComponent = (unsigned int)newId;
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

	// ask user to select a new component
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignComponent",
		tr("Assign component"), tr("You may now select a component from the database, which will then be "
								   "assigned to the selected surfaces."));

	unsigned int selectedComponentId = SVMainWindow::instance().dbComponentEditDialog()->select(0);

	if (selectedComponentId == VICUS::INVALID_ID)
		return; // user aborted the dialog

	assignComponent(false, selectedComponentId);
}


void SVPropBuildingEditWidget::on_pushButtonAssignInsideComponent_clicked() {

	// ask user to select a new component
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignComponent",
		tr("Assign component"), tr("You may now select a component from the database, which will then be "
								   "assigned to the selected surfaces."));

	unsigned int selectedComponentId = SVMainWindow::instance().dbComponentEditDialog()->select(0);

	if (selectedComponentId == VICUS::INVALID_ID)
		return; // user aborted the dialog

	assignComponent(true, selectedComponentId);
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
	m_ui->pushButtonAssignSelectedComponent->setEnabled(enabled);

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
		if (ci.m_idSubSurfaceComponent == oldId)
			ci.m_idSubSurfaceComponent = (unsigned int)newId;
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

	// filter-out selected objects and group by type
	std::vector<const VICUS::Object * > selObjs;
	std::vector<const VICUS::Surface * > selSurfs;
	std::vector<const VICUS::SubSurface * > subSurfObjs;
	for (const VICUS::Object * o : objs) {
		if (!o->m_selected) continue;
		selObjs.push_back(o);
		const VICUS::Surface * surf = dynamic_cast<const VICUS::Surface *>(o);
		if (surf != nullptr)
			selSurfs.push_back(surf);
		const VICUS::SubSurface * subsurf = dynamic_cast<const VICUS::SubSurface *>(o);
		if (subsurf != nullptr)
			subSurfObjs.push_back(subsurf);
	}
	m_selectedSurfaces = std::set<const VICUS::Surface*>(selSurfs.begin(), selSurfs.end());

	// ** Components data structure update **
	// now build a map of component IDs versus visible surfaces
	m_componentSurfacesMap.clear();
	m_selectedComponentInstances.clear();

	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// component ID assigned?
		if (ci.m_idComponent == VICUS::INVALID_ID)
			continue; // no component, skip
		// lookup component in DB
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];
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
		if (ci.m_idSubSurfaceComponent == VICUS::INVALID_ID)
			continue; // no component, skip
		// lookup component in DB
		const VICUS::SubSurfaceComponent * comp = SVSettings::instance().m_db.m_subSurfaceComponents[ci.m_idSubSurfaceComponent];
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
				item->setBackground(QColor(64,64,64));
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
		if (selSurfs.empty()) {
			m_ui->labelSelectedComponents->setText("");
			m_ui->groupBoxSelectedComponent->setEnabled(false);
		}
		else {
			m_ui->groupBoxSelectedComponent->setEnabled(true);
		}
		m_ui->pushButtonAssignInsideComponent->setEnabled(selSurfs.size() == 2);

		// update selection-related info
		std::set<const VICUS::Component *> selectedComponents;
		for (const VICUS::Surface* s : selSurfs) {
			if (s->m_componentInstance != nullptr) {
				const VICUS::Component * surfcomp = db.m_components[s->m_componentInstance->m_idComponent];
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
				item->setBackground(QColor(64,64,64));
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
				const VICUS::SubSurfaceComponent * surfcomp = db.m_subSurfaceComponents[s->m_subSurfaceComponentInstance->m_idSubSurfaceComponent];
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
			if (ci.m_idComponent == VICUS::INVALID_ID)
				continue; // no component, skip
			// lookup component in DB
			const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];
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
				item->setBackground(QColor(64,64,64));
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

	updateInterlinkedSurfacesPage();
	updateSurfaceHeatingPage();
}


void SVPropBuildingEditWidget::updateInterlinkedSurfacesPage() {

	m_ui->tableWidgetInterlinkedSurfaces->blockSignals(true);
	m_ui->tableWidgetInterlinkedSurfaces->selectionModel()->blockSignals(true);
	m_ui->tableWidgetInterlinkedSurfaces->setRowCount(0);

	const SVDatabase & db = SVSettings::instance().m_db;

	// process all component instances
	QList<QTableWidgetItem *> selectedItems;
	std::set<const VICUS::Surface*> referencedSurfaces;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// skip all without two surfaces
		if (ci.m_sideASurface == nullptr || ci.m_sideBSurface == nullptr)
			continue;

		// add new row
		int row = m_ui->tableWidgetInterlinkedSurfaces->rowCount();
		m_ui->tableWidgetInterlinkedSurfaces->setRowCount(row + 1);

		// column 1 - ID of this component instance

		QTableWidgetItem * item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled);
		item->setData(Qt::UserRole, ci.m_id);
		item->setText(QString("%1").arg(ci.m_id));
		item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 1, item);

		// column 2 - surface name A

		item = new QTableWidgetItem(ci.m_sideASurface->m_displayName);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, ci.m_sideASurface->uniqueID()); // uniqueID is the user role
		if (m_selectedSurfaces.find(ci.m_sideASurface) != m_selectedSurfaces.end())
			selectedItems.append(item);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 2, item);

		// column 3 - surface name B

		item = new QTableWidgetItem(ci.m_sideBSurface->m_displayName);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, ci.m_sideBSurface->uniqueID()); // uniqueID is the user role
		if (m_selectedSurfaces.find(ci.m_sideBSurface) != m_selectedSurfaces.end())
			selectedItems.append(item);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 3, item);

		// column 4 - component
		const VICUS::Component * comp = db.m_components[ci.m_idComponent];
		QString compName;
		if (comp == nullptr)
			compName = "---";
		else
			compName = QtExt::MultiLangString2QString(comp->m_displayName);

		item = new QTableWidgetItem(compName);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 4, item);

		// we also have to check all components, so that we have not only
		// connected surfaces but also boundary conditions that fits to them

		// we already now that both sides are connected

		//QIcon("://gfx/actions/16x16/error.png");
		item = new QTableWidgetItem();
		item->setIcon(QIcon("://gfx/actions/16x16/ok.png"));

		// we collect all relevent information as text in the tooltip
		QString toolTip;

		// check that neither of the two surfaces was previously used in another component instance
		if (referencedSurfaces.find(ci.m_sideASurface) != referencedSurfaces.end()) {
			item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
			toolTip += "Component references surface at side A that was already previously referenced somewhere else.";
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
		}
		if (referencedSurfaces.find(ci.m_sideBSurface) != referencedSurfaces.end()) {
			item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
			toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
			toolTip += "Component references surface at side B that was already previously referenced somewhere else.";
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 3)->setForeground(Qt::red);
		}
		// remember surfaces are "referenced"
		referencedSurfaces.insert(ci.m_sideASurface);
		referencedSurfaces.insert(ci.m_sideBSurface);

		// accces DB

		if (comp != nullptr) {
			const VICUS::BoundaryCondition * bcLeft = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
			if (bcLeft == nullptr) {
				item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
				toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
				toolTip += "Component has no valid boundary condition at surface side A.";

				m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
			}
			else {
				// check that bc does not reference constant zone
				if (bcLeft->m_heatConduction.m_otherZoneType != VICUS::InterfaceHeatConduction::OZ_Standard) {
					item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
					toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
					toolTip += "Boundary condition at surface side A is associated with constant/scheduled zone.";

					m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
				}
			}
		}
		else {
			item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
			toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
			toolTip += "Invalid/unassigned component";
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
		}


		if (comp != nullptr) {
			const VICUS::BoundaryCondition * bcRight = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
			if (bcRight == nullptr) {
				item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
				toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
				toolTip += "Component has no valid boundary condition at surface side B.";

				m_ui->tableWidgetInterlinkedSurfaces->item(row, 3)->setForeground(Qt::red);
			}
			else {
				// check that bc does not reference constant zone
				if (bcRight->m_heatConduction.m_otherZoneType != VICUS::InterfaceHeatConduction::OZ_Standard) {
					item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
					toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
					toolTip += "Boundary condition at surface side B is associated with constant/scheduled zone.";

					m_ui->tableWidgetInterlinkedSurfaces->item(row, 3)->setForeground(Qt::red);
				}
			}
		}

		// must not reference the same surface on both sides
		if (ci.m_sideASurface == ci.m_sideBSurface) {
			item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
			toolTip.size() > 0 ? toolTip += "\n" : toolTip = "";
			toolTip += "Same surface referenced on both sides.";
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 2)->setForeground(Qt::red);
			m_ui->tableWidgetInterlinkedSurfaces->item(row, 3)->setForeground(Qt::red);
		}

		item->setToolTip(toolTip);
		m_ui->tableWidgetInterlinkedSurfaces->setItem(row, 0, item);

	}
	m_ui->tableWidgetInterlinkedSurfaces->blockSignals(false);

	// once table is complete (and won't be modified anylonger), select items
	for (QTableWidgetItem * item : selectedItems)
		item->setSelected(true);

	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 0, true);
	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 1, true);
	SVStyle::resizeTableColumnToContents(m_ui->tableWidgetInterlinkedSurfaces, 2, true);

	m_ui->tableWidgetInterlinkedSurfaces->selectionModel()->blockSignals(false);

	// enable/disable button based on available selections
	m_ui->pushButtonRemoveComponentInstance->setEnabled(!selectedItems.isEmpty());
	m_ui->groupBoxConnectSurfaces->setEnabled(!m_selectedSurfaces.empty());
}


void SVPropBuildingEditWidget::updateSurfaceHeatingPage() {
	// populate table with all components that are currently selected by filter
	// we only show assigned components with active layers

	const SVDatabase & db = SVSettings::instance().m_db;

	// populate combo box with components
	m_ui->comboBoxSurfaceHeatingComponentFilter->blockSignals(true);
	unsigned int currentComponent = m_ui->comboBoxSurfaceHeatingComponentFilter->currentData().toUInt(); // unique ID of currently selected component
	int selectedIndex = -1;
	m_ui->comboBoxSurfaceHeatingComponentFilter->clear();
	m_ui->comboBoxSurfaceHeatingComponentFilter->addItem(tr("All components"), VICUS::INVALID_ID);
	for (const auto & compEntry : db.m_components) {
		if (compEntry.second.m_activeLayerIndex == VICUS::INVALID_ID)
			continue;
		if (compEntry.second.m_id == currentComponent)
			selectedIndex = m_ui->comboBoxSurfaceHeatingComponentFilter->count();
		m_ui->comboBoxSurfaceHeatingComponentFilter->addItem( QtExt::MultiLangString2QString(compEntry.second.m_displayName), compEntry.second.m_id);
	}
	if (selectedIndex == -1)
		m_ui->comboBoxSurfaceHeatingComponentFilter->setCurrentIndex(0);
	else
		m_ui->comboBoxSurfaceHeatingComponentFilter->setCurrentIndex(selectedIndex);
	m_ui->comboBoxSurfaceHeatingComponentFilter->blockSignals(false);

	m_ui->tableWidgetSurfaceHeating->blockSignals(true);
	m_ui->tableWidgetSurfaceHeating->selectionModel()->blockSignals(true);
	m_ui->tableWidgetSurfaceHeating->setRowCount(0);


	// process all component instances
	unsigned int componentFilterID = m_ui->comboBoxSurfaceHeatingComponentFilter->currentData().toUInt();
	std::set<const VICUS::ComponentInstance *> selectedCI;					// all selected component instance
	std::set<const VICUS::ComponentInstance *> selectedSurfaceHeatingCI;	// only selected component instance with surface heatings assigned
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// skip all without components - these should be removed as invalid from the start
		const VICUS::Component * comp = db.m_components[ci.m_idComponent];
		if (comp == nullptr)
			continue;
		// skip all that do not have active layers
		if (comp->m_activeLayerIndex == VICUS::INVALID_ID)
			continue;

		// skip if not in combo box filter
		if (componentFilterID != VICUS::INVALID_ID && comp->m_id != componentFilterID)
			continue;

		// skip all component instances where _all_ assigned surfaces are invisible
		bool visible = false;
		if (ci.m_sideASurface != nullptr && ci.m_sideASurface->m_visible)
			visible = true;
		if (ci.m_sideBSurface != nullptr && ci.m_sideBSurface->m_visible)
			visible = true;
		if (!visible)
			continue;

		bool selected = false;
		if (ci.m_sideASurface != nullptr && ci.m_sideASurface->m_selected)
			selected = true;
		if (ci.m_sideBSurface != nullptr && ci.m_sideBSurface->m_selected)
			selected = true;
		// store selected component instances (with active layer)
		if (selected) {
			selectedCI.insert(&ci);
			// if already configured surface heating?
			if (ci.m_idSurfaceHeating != VICUS::INVALID_ID)
				selectedSurfaceHeatingCI.insert(&ci);
		}

		// add new row
		int row = m_ui->tableWidgetSurfaceHeating->rowCount();
		m_ui->tableWidgetSurfaceHeating->setRowCount(row + 1);

		// look-up surface heating system
		const VICUS::SurfaceHeating * surfHeat = db.m_surfaceHeatings[ci.m_idSurfaceHeating];


		// column 0 - valid icon, also stores unique ID of this component instance

		QTableWidgetItem * item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled);
		item->setData(Qt::UserRole, ci.m_id);
		if (comp->isValid(db.m_materials, db.m_constructions, db.m_boundaryConditions))
			item->setIcon(QIcon("://gfx/actions/16x16/ok.png"));
		else
			item->setIcon(QIcon("://gfx/actions/16x16/error.png"));
		m_ui->tableWidgetSurfaceHeating->setItem(row, 0, item);

		// column 1 - color of heating
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled);
		if (surfHeat == nullptr) {
			item->setBackground(QColor("#758eb3")); // color in blue to indicate potential for surface heating
		}
		else
			item->setBackground(surfHeat->m_color);
		m_ui->tableWidgetSurfaceHeating->setItem(row, 1, item);

		// column 2 - heating name
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (surfHeat == nullptr) {
			item->setText("---");
			item->setData(Qt::UserRole, VICUS::INVALID_ID);
		}
		else {
			item->setText(QtExt::MultiLangString2QString(surfHeat->m_displayName));
			item->setData(Qt::UserRole, surfHeat->m_id);
		}
		m_ui->tableWidgetSurfaceHeating->setItem(row, 2, item);

		// column 3 - control zone ID name
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (ci.m_surfaceHeatingControlZone == nullptr)
			item->setText("---");
		else {
			item->setText(ci.m_surfaceHeatingControlZone->m_displayName);
			item->setData(Qt::UserRole, ci.m_surfaceHeatingControlZone->m_id);
		}
		m_ui->tableWidgetSurfaceHeating->setItem(row, 3, item);

		// column 4 - surface names
		QString surfaceNames;
		if (ci.m_sideASurface != nullptr)
			surfaceNames = ci.m_sideASurface->m_displayName;
		surfaceNames += "/";
		if (ci.m_sideBSurface != nullptr)
			surfaceNames += ci.m_sideBSurface->m_displayName;

		item = new QTableWidgetItem(surfaceNames);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetSurfaceHeating->setItem(row, 4, item);

		// column 5 - associated network


	}
	m_ui->tableWidgetSurfaceHeating->blockSignals(false);
	m_ui->tableWidgetSurfaceHeating->selectionModel()->blockSignals(false);


	// enable/disable selection-based buttons
	on_tableWidgetSurfaceHeating_itemSelectionChanged();
	if (selectedCI.empty()) {
		m_ui->labelSelectedCIWithActiveLayer->setText(tr("None"));
		m_ui->pushButtonAssignSurfaceHeating->setEnabled(false);
		m_ui->pushButtonAssignSurfaceHeatingControlZone->setEnabled(false);
		m_ui->pushButtonAssignSurfaceHeatingNetwork->setEnabled(false);
		m_ui->pushButtonRemoveSelectedSurfaceHeating->setEnabled(false);
	}
	else {
		m_ui->labelSelectedCIWithActiveLayer->setText(tr("%1 component instance(s)").arg(selectedCI.size()));
		m_ui->pushButtonAssignSurfaceHeating->setEnabled(true);

		// other buttons are only active if a configured surface heating component instances is selected
		m_ui->pushButtonAssignSurfaceHeatingControlZone->setEnabled(!selectedSurfaceHeatingCI.empty());
		m_ui->pushButtonAssignSurfaceHeatingNetwork->setEnabled(!selectedSurfaceHeatingCI.empty());
		m_ui->pushButtonRemoveSelectedSurfaceHeating->setEnabled(!selectedSurfaceHeatingCI.empty());
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
			std::swap(it->m_idSideASurface, it->m_idSideBSurface);
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


void SVPropBuildingEditWidget::assignComponent(bool insideWall, unsigned int selectedComponentId) {

	Q_ASSERT(!m_selectedSurfaces.empty());

	std::vector<VICUS::ComponentInstance> compInstances;
	if (insideWall) {
		Q_ASSERT(m_selectedSurfaces.size() == 2); // must have exactly two
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
			std::set<const VICUS::Surface*>::iterator it = m_selectedSurfaces.find(ci.m_sideASurface);
			std::set<const VICUS::Surface*>::iterator it2 = m_selectedSurfaces.find(ci.m_sideBSurface);
			if (it != m_selectedSurfaces.end() || it2 != m_selectedSurfaces.end()) {
				// yes, but did we already have a component instance with these surfaces?
				if (compInstanceFound) {
					// already handled, so we can just skip this component instance
					continue;
				}
				// remember, that we handled a componentInstance already
				compInstanceFound = true;
				VICUS::ComponentInstance newCi(ci);
				// set the selected component
				newCi.m_idComponent = (unsigned int)selectedComponentId;
				// no, first time a component instance references the selected surfaces, modify it such,
				// that any original assignment remains untouched
				if (it != m_selectedSurfaces.end()) {
					// side A connected?, set side B to the other surface
					if (it == m_selectedSurfaces.begin())
						newCi.m_idSideBSurface = (*m_selectedSurfaces.rbegin())->m_id;
					else
						newCi.m_idSideBSurface = (*m_selectedSurfaces.begin())->m_id;
				}
				else {
					// must be side B connected, set side A to the other surface
					if (it2 == m_selectedSurfaces.begin())
						newCi.m_idSideASurface = (*m_selectedSurfaces.rbegin())->m_id;
					else
						newCi.m_idSideASurface = (*m_selectedSurfaces.begin())->m_id;
				}
				// check that both surfaces do not belong to the same room
				const VICUS::Surface * surfA = project().surfaceByID(newCi.m_idSideASurface);
				const VICUS::Surface * surfB = project().surfaceByID(newCi.m_idSideBSurface);
				Q_ASSERT(surfA != nullptr && surfB != nullptr);
				if (surfA->m_parent == surfB->m_parent) {
					QMessageBox::critical(this, QString(), tr("Both surfaces belong to the same room."));
					return;
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
			unsigned int nextId = VICUS::largestUniqueId(compInstances);
			newCi.m_id = nextId;
			newCi.m_idComponent = (unsigned int)selectedComponentId;
			newCi.m_idSideASurface = (*m_selectedSurfaces.begin())->m_id;
			newCi.m_idSideBSurface = (*m_selectedSurfaces.rbegin())->m_id;
			compInstances.push_back(newCi);
		}

	}
	else {
		// now create a copy of the currently defined componentInstances
		compInstances = project().m_componentInstances;

		// process all componentInstances and for all that reference any of the selected surfaces, replace component
		for (VICUS::ComponentInstance & ci : compInstances) {
			std::set<const VICUS::Surface*>::iterator itA = m_selectedSurfaces.find(ci.m_sideASurface);
			std::set<const VICUS::Surface*>::iterator itB= m_selectedSurfaces.find(ci.m_sideBSurface);
			if (itA != m_selectedSurfaces.end()) {
				ci.m_idComponent = (unsigned int)selectedComponentId;
				m_selectedSurfaces.erase(itA);

				// we also want to know if the selection contains
				// our already assigned component. If this is the case
				// we erase also side B surface
				if (itB != m_selectedSurfaces.end())
					m_selectedSurfaces.erase(itB);

				continue;
			}
			if (itB != m_selectedSurfaces.end()) {
				ci.m_idComponent = (unsigned int)selectedComponentId;
				m_selectedSurfaces.erase(itB);

				// we also want to know if the selection contains
				// our already assigned component. If this is the case
				// we erase also side B surface
				if (itA != m_selectedSurfaces.end())
					m_selectedSurfaces.erase(itA);
			}
		}

		// now all surfaces still left in selSurfaces are not yet referenced in ComponentInstances
		// hence, we need to create a new componentInstance for each of them
		if (!m_selectedSurfaces.empty()) {
			unsigned int nextId = VICUS::largestUniqueId(compInstances);
			for (const VICUS::Surface * s : m_selectedSurfaces) {
				VICUS::ComponentInstance c;
				c.m_id = nextId++;
				c.m_idSideASurface = s->m_id;
				c.m_idComponent = (unsigned int)selectedComponentId;
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
				newCi.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
				// no, first time a component instance references the selected surfaces, modify it such,
				// that any original assignment remains untouched
				if (it != surfaces.end()) {
					// side A connected?, set side B to the other surface
					if (it == surfaces.begin())
						newCi.m_idSideBSurface = (*surfaces.rbegin())->m_id;
					else
						newCi.m_idSideBSurface = (*surfaces.begin())->m_id;
				}
				else {
					// must be side B connected, set side A to the other surface
					if (it2 == surfaces.begin())
						newCi.m_idSideASurface = (*surfaces.rbegin())->m_id;
					else
						newCi.m_idSideASurface = (*surfaces.begin())->m_id;
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
			unsigned int nextId = VICUS::largestUniqueId(compInstances);
			newCi.m_id = nextId;
			newCi.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
			newCi.m_idSideASurface = (*surfaces.begin())->m_id;
			newCi.m_idSideBSurface = (*surfaces.rbegin())->m_id;
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
				ci.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
				surfaces.erase(it);
				continue;
			}
			it = surfaces.find(ci.m_sideBSubSurface);
			if (it != surfaces.end()) {
				ci.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
				surfaces.erase(it);
			}
		}

		// now all surfaces still left in selSurfaces are not yet referenced in ComponentInstances
		// hence, we need to create a new componentInstance for each of them
		if (!surfaces.empty()) {
			unsigned int nextId = VICUS::largestUniqueId(compInstances);
			for (const VICUS::SubSurface * s : surfaces) {
				VICUS::SubSurfaceComponentInstance c;
				c.m_id = nextId++;
				c.m_idSideASurface = s->m_id;
				c.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
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


void SVPropBuildingEditWidget::on_comboBoxSurfaceHeatingComponentFilter_currentIndexChanged(int /*index*/) {
	updateSurfaceHeatingPage();
}


void SVPropBuildingEditWidget::on_tableWidgetSurfaceHeating_itemChanged(QTableWidgetItem *item) {
	if (item->column() == 2 || item->column() == 3) {
		QTableWidgetItem * firstItem = m_ui->tableWidgetSurfaceHeating->item(item->row(), 0);
		unsigned int ciID = firstItem->data(Qt::UserRole).toUInt();
		std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;
		for (unsigned int i=0; i<cis.size(); ++i)
			if (cis[i].m_id == ciID) {
				if (item->column() == 2)
					cis[i].m_idSurfaceHeating = item->data(Qt::UserRole).toUInt();
				else
					cis[i].m_idSurfaceHeatingControlZone = item->data(Qt::UserRole).toUInt();
				break;
			}
		SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Assigned surface heating"), cis);
		undo->push();
	}
}


void SVPropBuildingEditWidget::on_pushButtonRemoveSurfaceHeating_clicked() {
	// process all selected elements, modify component instances and issue undo action

	std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;
	for (int row=0; row<m_ui->tableWidgetSurfaceHeating->rowCount(); ++row) {
		// is any of the two editable cells selected?
		if (m_ui->tableWidgetSurfaceHeating->item(row, 2)->isSelected() ||
			m_ui->tableWidgetSurfaceHeating->item(row, 3)->isSelected())
		{
			// get unique ID of component instance
			QTableWidgetItem * firstItem = m_ui->tableWidgetSurfaceHeating->item(row, 0);
			unsigned int ciID = firstItem->data(Qt::UserRole).toUInt();
			// find matching component instance
			for (unsigned int i=0; i<cis.size(); ++i)
				if (cis[i].m_id == ciID) {
					if (m_ui->tableWidgetSurfaceHeating->item(row, 2)->isSelected())
						cis[i].m_idSurfaceHeating = VICUS::INVALID_ID;
					if (m_ui->tableWidgetSurfaceHeating->item(row, 3)->isSelected())
						cis[i].m_idSurfaceHeatingControlZone = VICUS::INVALID_ID;
					break;
				}
		}
	}
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Removed surface heating assignment"), cis);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonAssignSurfaceHeating_clicked() {
	// popup surface heating DB dialog and if user selects one, assign it to all selected component instances
	unsigned int selectedID = SVMainWindow::instance().dbSurfaceHeatingSystemEditDialog()->select(VICUS::INVALID_ID);
	if (selectedID == VICUS::INVALID_ID)
		return;

	std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;

	// process all selected components
	for (VICUS::ComponentInstance & ci : cis) {
		// check if current ci is in list of selected component instances
		std::set<const VICUS::ComponentInstance*>::const_iterator ciIt = m_selectedComponentInstances.begin();
		for (; ciIt != m_selectedComponentInstances.end(); ++ciIt) {
			if ((*ciIt)->m_id == ci.m_id)
				break;
		}
		if (ciIt == m_selectedComponentInstances.end())
			continue;
		// if component instance does not have an active layer assigned, skip
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];
		if (comp == nullptr)
			continue;
		// check if no active layer is present
		if (comp->m_activeLayerIndex == VICUS::INVALID_ID)
			continue;
		// now get room associated with selected component
		const VICUS::Surface * s = ci.m_sideASurface;
		if (s == nullptr)
			s = ci.m_sideBSurface;
		Q_ASSERT(s != nullptr);
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(s->m_parent);
		ci.m_idSurfaceHeating = selectedID;
		ci.m_idSurfaceHeatingControlZone = room->m_id;
	}
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Assigned surface heatings"), cis);
	undo->push();
}

void SVPropBuildingEditWidget::on_tableWidgetSurfaceHeating_currentCellChanged(int /*currentRow*/, int /*currentColumn*/, int previousRow, int previousColumn) {
	QTableWidgetItem * item = m_ui->tableWidgetSurfaceHeating->item(previousRow, previousColumn);
	if (item != nullptr)
		m_ui->tableWidgetSurfaceHeating->closePersistentEditor(item);
}


void SVPropBuildingEditWidget::on_pushButtonRemoveComponentInstance_clicked() {

	// create set with selected and connected surfaces
	std::set<unsigned int> connectedSurfacesIDs;
	for (QTableWidgetItem * item : m_ui->tableWidgetInterlinkedSurfaces->selectedItems())
		connectedSurfacesIDs.insert(item->data(Qt::UserRole).toUInt());

	// now process all ComponentInstances and handle those with selected and connected surfaces
	std::vector<VICUS::ComponentInstance> newInstances;
	unsigned int newID = VICUS::largestUniqueId(project().m_componentInstances);
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// must have both sides connected
		if (ci.m_sideASurface == nullptr || ci.m_sideBSurface == nullptr) {
			newInstances.push_back(ci); // just keep unmodified
			continue;
		}

		if ( (ci.m_sideASurface != nullptr &&
				connectedSurfacesIDs.find(ci.m_sideASurface->uniqueID()) != connectedSurfacesIDs.end()) ||
			(ci.m_sideBSurface != nullptr &&
							connectedSurfacesIDs.find(ci.m_sideBSurface->uniqueID()) != connectedSurfacesIDs.end()) )
		{
			// create two copies of the ComponentInstance and remove sideA and sideB in either one
			VICUS::ComponentInstance ci1(ci);
			ci1.m_id = newID++;
			ci1.m_idSideBSurface = VICUS::INVALID_ID;
			VICUS::ComponentInstance ci2(ci);
			ci2.m_id = newID++;
			ci2.m_idSideASurface = VICUS::INVALID_ID;
			newInstances.push_back(ci1);
			newInstances.push_back(ci2);
		}
		else {
			// not currently selected
			newInstances.push_back(ci); // just keep unmodified
			continue;
		}
	}

	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Removed surface-surface connection"),
																			   newInstances);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonConnectSurfaces_clicked() {
	// check input
	if (!m_ui->lineEditConnectSurfacesMaxDistance->isValid()) {
		QMessageBox::critical(this, QString(), tr("Please enter valid parameters!"));
		m_ui->lineEditConnectSurfacesMaxDistance->selectAll();
		m_ui->lineEditConnectSurfacesMaxDistance->setFocus();
		return;
	}
	// check input
	if (!m_ui->lineEditConnectSurfacesMaxAngle->isValid()) {
		QMessageBox::critical(this, QString(), tr("Please enter valid parameters!"));
		m_ui->lineEditConnectSurfacesMaxAngle->selectAll();
		m_ui->lineEditConnectSurfacesMaxAngle->setFocus();
		return;
	}
	double maxDist = m_ui->lineEditConnectSurfacesMaxDistance->value();
	double maxAngle = m_ui->lineEditConnectSurfacesMaxAngle->value();

	// the actual work is implemented in the project class, because it is needed elsewhere as well
	std::vector<VICUS::ComponentInstance> newInstances;

	if (project().connectSurfaces(maxDist, maxAngle, m_selectedSurfaces, newInstances)) {

		SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Created surface-surface connection"),
																				   newInstances);
		undo->push();
	}
	else {
		QMessageBox::information(this, QString(), tr("With the given selection and algorithm parameters, no new "
													 "connections could be created."));
	}
}


void SVPropBuildingEditWidget::on_tableWidgetInterlinkedSurfaces_itemSelectionChanged() {
	// get selected items, compose undo-action and fire
	// compose set of objects to be selected
	std::set<unsigned int> selectedObjs;

	for (QTableWidgetItem * item : m_ui->tableWidgetInterlinkedSurfaces->selectedItems())
		selectedObjs.insert(item->data(Qt::UserRole).toUInt());

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selected connected surfaces"),
														 SVUndoTreeNodeState::SelectedState, selectedObjs,
														 true, true /* exclusive */);
	undo->push();
}


void SVPropBuildingEditWidget::on_tableWidgetSurfaceHeating_itemSelectionChanged() {
	// based on selection in surface heating table, enable/disable "Remove button"

	// we can only remove surface heatings that are actually configured
	// process all selected rows and check if the surface heating association is set
	// UserRole of column 2 stores ID of surface heating object; VICUS::INVALID_ID if not assigned
	bool haveSurfaceHeating = false;

	for (QModelIndex idx : m_ui->tableWidgetSurfaceHeating->selectionModel()->selectedRows()) {
		// construct model index of second column
		int row = idx.row();
		// retrieve item and userrole from second column
		const QTableWidgetItem * item = m_ui->tableWidgetSurfaceHeating->item(row, 2);
		if (item->data(Qt::UserRole).toUInt() != VICUS::INVALID_ID) {
			haveSurfaceHeating = true;
			break;
		}
	}
	m_ui->pushButtonRemoveSurfaceHeating->setEnabled(haveSurfaceHeating);
}


void SVPropBuildingEditWidget::on_pushButtonRemoveSelectedSurfaceHeating_clicked() {
	std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;

	// process all selected components
	for (VICUS::ComponentInstance & ci : cis) {
		// check if current ci is in list of selected component instances
		std::set<const VICUS::ComponentInstance*>::const_iterator ciIt = m_selectedComponentInstances.begin();
		for (; ciIt != m_selectedComponentInstances.end(); ++ciIt) {
			if ((*ciIt)->m_id == ci.m_id)
				break;
		}
		if (ciIt == m_selectedComponentInstances.end())
			continue;
		// clear surface heating
		ci.m_idSurfaceHeating = VICUS::INVALID_ID;
		ci.m_idSurfaceHeatingControlZone = VICUS::INVALID_ID;
	}

	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Removed surface heatings"), cis);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonAssignSurfaceHeatingControlZone_clicked() {
	// popup dialog with zone selection

	// create dialog - only locally, this ensures that in constructor the zone is is updated
	SVZoneSelectionDialog dlg(this);

	// start dialog
	int res = dlg.exec();
	if (res != QDialog::Accepted)
		return; // user canceled the dialog

	std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;

	for (VICUS::ComponentInstance & ci : cis) {
		// check if current ci is in list of selected component instances
		std::set<const VICUS::ComponentInstance*>::const_iterator ciIt = m_selectedComponentInstances.begin();
		for (; ciIt != m_selectedComponentInstances.end(); ++ciIt) {
			if ((*ciIt)->m_id == ci.m_id)
				break;
		}
		if (ciIt == m_selectedComponentInstances.end())
			continue;
		// if component instance does not have an active layer assigned, skip
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];
		if (comp == nullptr)
			continue;
		// check if no active layer is present
		if (comp->m_activeLayerIndex == VICUS::INVALID_ID)
			continue;
		ci.m_idSurfaceHeatingControlZone = dlg.m_idZone;
	}
	// perform an undo action in order to redo/revert current operation
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Changed surface heatings control zone"), cis);
	undo->push();
}


void SVPropBuildingEditWidget::on_pushButtonAssignSelComponent_clicked() {
	// we take the currently selected component
	const VICUS::Component * comp = currentlySelectedComponent();
	// assign it to our selected surfaces
	assignComponent(false, comp->m_id);
}
