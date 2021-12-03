#include "SVPropBuildingComponentsWidget.h"
#include "ui_SVPropBuildingComponentsWidget.h"

#include <QtExt_Conversions.h>

#include <VICUS_utilities.h>

#include "SVStyle.h"
#include "SVUndoModifyComponentInstances.h"
#include "SVUndoTreeNodeState.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

SVPropBuildingComponentsWidget::SVPropBuildingComponentsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingComponentsWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->tableWidgetComponents->setColumnCount(2);
	m_ui->tableWidgetComponents->setHorizontalHeaderLabels(QStringList() << QString() << tr("Component"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetComponents);
	m_ui->tableWidgetComponents->setSortingEnabled(false);
	m_ui->tableWidgetComponents->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetComponents->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetComponents->horizontalHeader()->setStretchLastSection(true);
}


SVPropBuildingComponentsWidget::~SVPropBuildingComponentsWidget() {
	delete m_ui;
}


void SVPropBuildingComponentsWidget::updateUi() {
	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true); // visible (selected and not selected)

	// filter out surfaces and create set with surfaces that are not referenced by any component instance
	// (will be adjusted in the component instance loop below)
	std::set<const VICUS::Surface * > surfacesNotInComponentInterface; // note: we use a set, because we want to quickly remove items later
	// we need to create a vector with entries for the table, but first create a map with surfaces belonging to
	// one of the four categories
	std::set<const VICUS::Surface *> surfacesWithoutComponent; // component interface referencing this surface does not have a component assigned

	// this map holds the association of a surface to its assigned component
	// Note: if a surface is referenced multiple times with different components in the ComponentInstance, it will also show up several times in the map
	std::map<const VICUS::Component *, std::set<const VICUS::Surface*> > surfaceComponentMap;

	// collect all selected surfaces
	m_selectedSurfaces.clear();
	for (const VICUS::Object * o : objs) {
		const VICUS::Surface * surf = dynamic_cast<const VICUS::Surface * >(o);
		if (surf != nullptr) {
			surfacesNotInComponentInterface.insert(surf);
			if (surf->m_selected)
				m_selectedSurfaces.push_back(surf);
		}
	}

	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// lookup visible surfaces
		const VICUS::Surface * visibleSideASurf = nullptr;
		// side A
		if (ci.m_sideASurface != nullptr) {
			// is this surface visible?
			if (ci.m_sideASurface->m_visible)
				visibleSideASurf = ci.m_sideASurface;
		}
		// side B
		const VICUS::Surface * visibleSideBSurf = nullptr;
		if (ci.m_sideBSurface != nullptr) {
			// is this surface visible?
			if (ci.m_sideBSurface->m_visible)
				visibleSideBSurf = ci.m_sideBSurface;
		}

		// ignore component instances, where both surfaces are invisible
		if (visibleSideASurf == nullptr && visibleSideBSurf == nullptr)
			continue;

		// remove from visibleSurfaces set since this surface was referenced in componentInstance
		surfacesNotInComponentInterface.erase(visibleSideASurf);
		surfacesNotInComponentInterface.erase(visibleSideBSurf);

		// valid component assigned?
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];
		if (comp == nullptr) {
			// no component or invalid component, remember in set "without component"
			surfacesWithoutComponent.insert(visibleSideASurf);
			surfacesWithoutComponent.insert(visibleSideBSurf);
			continue; // nothing else to do
		}

		surfaceComponentMap[comp].insert(visibleSideASurf);
		surfaceComponentMap[comp].insert(visibleSideBSurf);
	}

	// remove nullptr (added for any non-surf objects)
	surfacesNotInComponentInterface.erase(nullptr);
	surfacesWithoutComponent.erase(nullptr);

	// Note: in case of invalid CI with surface IDs that do not exist, we
	//       may have a nullptrs for surfaces, so we need to remove those
	for (std::map<const VICUS::Component *, std::set<const VICUS::Surface*> >::iterator it = surfaceComponentMap.begin();
		 it != surfaceComponentMap.end(); ++it)
	{
		it->second.erase(nullptr);
	}

	// now store the data into the vector that is used for generating the table
	m_componentTable.clear();

	if (!surfacesNotInComponentInterface.empty()) {
		ComponentLegendEntry le;
		le.m_type = 0; // not connected/no component
		le.m_surfaces = std::vector<const VICUS::Surface*>(surfacesNotInComponentInterface.begin(), surfacesNotInComponentInterface.end());
		// component ID is not needed
		m_componentTable.push_back(le);
	}

	if (!surfacesWithoutComponent.empty()) {
		ComponentLegendEntry le;
		le.m_type = 1; // connected, but without component
		le.m_surfaces = std::vector<const VICUS::Surface*>(surfacesWithoutComponent.begin(), surfacesWithoutComponent.end());
		// component ID is not needed
		m_componentTable.push_back(le);
	}

	// finally add all the rest
	Q_ASSERT(surfaceComponentMap.find(nullptr) == surfaceComponentMap.end());
	for (std::map<const VICUS::Component *, std::set<const VICUS::Surface*> >::const_iterator it = surfaceComponentMap.begin();
		 it != surfaceComponentMap.end(); ++it)
	{
		ComponentLegendEntry le;
		le.m_type = 2; // connected with valid component
		le.m_surfaces = std::vector<const VICUS::Surface*>(it->second.begin(), it->second.end());
		le.m_component = it->first;
		m_componentTable.push_back(le);
	}

	// ** legend table **

	int currentRow = m_ui->tableWidgetComponents->currentRow();
	m_ui->tableWidgetComponents->blockSignals(true);
	m_ui->tableWidgetComponents->clearContents();
	m_ui->tableWidgetComponents->setRowCount(m_componentTable.size());
	for (unsigned int row = 0; row < m_componentTable.size(); ++row) {
		QTableWidgetItem * colorItem = new QTableWidgetItem();
		QTableWidgetItem * item = new QTableWidgetItem();
		// special handling for components with "invalid" component id
		const ComponentLegendEntry & le = m_componentTable[row];
		switch (le.m_type) {
			// not assigned - dark gray
			case 0 :
				colorItem->setBackground(QColor(64,64,64));
				item->setText(tr("Unused surfaces (not connected/assigned)"));
			break;
			// connected, yet no component or invalid component
			case 1 :
				colorItem->setBackground(QColor(148,64,64));
				item->setText(tr("Surfaces without component"));
			break;
			// connected, valid
			case 2 :
				colorItem->setBackground(le.m_component->m_color);
				item->setText(QtExt::MultiLangString2QString(le.m_component->m_displayName));
			break;
		}
		colorItem->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		m_ui->tableWidgetComponents->setItem((int)row, 0, colorItem);

		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetComponents->setItem((int)row, 1, item);
	}
	// reselect row
	m_ui->tableWidgetComponents->blockSignals(false);
	m_ui->tableWidgetComponents->selectRow(std::min(currentRow, m_ui->tableWidgetComponents->rowCount()-1));


	// ** selected surfaces group box **

	// process all selected surfaces and determine which component they have assigned
	if (m_selectedSurfaces.empty()) {
		m_ui->labelSelectedComponents->setText("");
		m_ui->groupBoxSelectedComponent->setEnabled(false);
	}
	else {
		m_ui->groupBoxSelectedComponent->setEnabled(true);
	}
	m_ui->pushButtonAssignInsideComponent->setEnabled(m_selectedSurfaces.size() == 2);

	// update selection-related info

	std::set<const VICUS::Component *> selectedComponents;
	const SVDatabase & db = SVSettings::instance().m_db;
	for (const VICUS::Surface * s : m_selectedSurfaces) {
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


void SVPropBuildingComponentsWidget::on_tableWidgetComponents_itemSelectionChanged() {
	// disable all by default
	m_ui->pushButtonEditComponents->setEnabled(false);
	m_ui->pushButtonExchangeComponents->setEnabled(false);
	m_ui->pushButtonSelectObjectsWithComponent->setEnabled(false);
	m_ui->pushButtonAssignComponentFromTable->setEnabled(false);

	// check if the table is empty or there is no currently selected row
	int r = m_ui->tableWidgetComponents->currentRow();
	if (r == -1)
		return;

	// exchanging/assigning a component is always possible
	m_ui->pushButtonExchangeComponents->setEnabled(true);

	// the select button is always active, even if no component is assigned, yet
	m_ui->pushButtonSelectObjectsWithComponent->setEnabled(true);

	Q_ASSERT(r < (int)m_componentTable.size());
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)r];
	if (e.m_type == 2) {
		// valid component, can be edited
		m_ui->pushButtonEditComponents->setEnabled(true);
		// the assign-from-table button is only available when there is at least one surface selected
		m_ui->pushButtonAssignComponentFromTable->setEnabled(!m_selectedSurfaces.empty());
	}

}


void SVPropBuildingComponentsWidget::on_pushButtonEditComponents_clicked() {
	const VICUS::Component * comp = currentlySelectedComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	int currentRow = m_ui->tableWidgetComponents->currentRow();
	SVMainWindow::instance().dbComponentEditDialog()->edit(comp->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
	// reselect row, because closing the edit dialog recreates the table (due to possible color change)
	if (m_ui->tableWidgetComponents->rowCount() > currentRow)
		m_ui->tableWidgetComponents->selectRow(currentRow);
}


void SVPropBuildingComponentsWidget::on_pushButtonExchangeComponents_clicked() {
	// we have to distinguish between the different cases:
	// type 0 = not connected surfaces -> assign component to surface (i.e. create new componentInstances)
	// all other types, modify component instances to point to new component

	// now compose an undo action and modify the project
	int r = m_ui->tableWidgetComponents->currentRow();
	Q_ASSERT(r != -1);
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)r];

	if (e.m_type == 0) {
		assignComponent(false, false); // no inside wall, not using selected surfaces, but surfaces from component table entry
		return;
	}

	const VICUS::Component * comp = e.m_component;
	// comp may be invalid, in which case we actually "assign" a component, rather than replace it
	unsigned int oldId = VICUS::INVALID_ID;
	QString msg;
	if (comp != nullptr) {
		oldId = comp->m_id;
		QString msg = tr("This will replace all associations with component '%1 [%2]' with another component.")
				.arg(QtExt::MultiLangString2QString(comp->m_displayName)).arg(oldId);
	}
	else {
		msg = tr("You may now select a component to be assigned to the surfaces.");
	}

	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoReplaceComponent",
		tr("Replace component"), msg);
	unsigned int newId = SVMainWindow::instance().dbComponentEditDialog()->select(oldId);
	if (newId == VICUS::INVALID_ID)
		return; // user has aborted the dialog

	// now process all component instances and modify the component
	std::vector<VICUS::ComponentInstance> modCI = project().m_componentInstances;
	for (VICUS::ComponentInstance & ci : modCI) {
		// we identify the component instance to modify in two ways
		if (e.m_type == 2) {
			// for valid component instances, we use the old Id as identifier
			if (ci.m_idComponent == oldId)
				ci.m_idComponent = (unsigned int)newId;
		}
		else {
			// for invalid/missing component assignments
			if (ci.m_idComponent == VICUS::INVALID_ID ||
				SVSettings::instance().m_db.m_components[ci.m_idComponent] == nullptr)
			{
				ci.m_idComponent = (unsigned int)newId;
			}
		}
	}
	// create the undo action and modify project
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Components exchanged"), modCI);
	undo->push();
}


void SVPropBuildingComponentsWidget::on_pushButtonSelectObjectsWithComponent_clicked() {
	const VICUS::Component * comp = nullptr;
	int row = m_ui->tableWidgetComponents->currentRow();
	Q_ASSERT(row != -1);
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)row];
	if (e.m_type == 2)
		comp = e.m_component;

	QString undoText;
	if (comp != nullptr)
		undoText = tr("Select objects with component '%1'").arg(QtExt::MultiLangString2QString(comp->m_displayName));
	else
		undoText = tr("Select objects with invalid/missing component.");

	std::set<unsigned int> surfIds;
	for (const VICUS::Surface * s : e.m_surfaces)
		surfIds.insert(s->uniqueID());
	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, surfIds, true);
	undo->push();
}


void SVPropBuildingComponentsWidget::on_pushButtonAssignComponent_clicked() {
	assignComponent(false, true);
}


void SVPropBuildingComponentsWidget::on_pushButtonAssignInsideComponent_clicked() {
	assignComponent(true, true);
}


const VICUS::Component * SVPropBuildingComponentsWidget::currentlySelectedComponent() const {
	int row = m_ui->tableWidgetComponents->currentRow();
	if (row == -1)
		return nullptr;
	Q_ASSERT(row < (int)m_componentTable.size());
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)row];
	if (e.m_type != 2)
		return nullptr; // not a valid component assigned
	return e.m_component;
}


void SVPropBuildingComponentsWidget::assignComponent(bool insideWall, bool fromSurfaceSelection, unsigned int selectedComponentId) {
	// ask user to select a new component, unless given
	if (selectedComponentId == VICUS::INVALID_ID) {
		SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignComponent",
			tr("Assign component"), tr("You may now select a component from the database, which will then be "
									   "assigned to the selected surfaces."));
		selectedComponentId = SVMainWindow::instance().dbComponentEditDialog()->select(VICUS::INVALID_ID);
		if (selectedComponentId == VICUS::INVALID_ID)
			return; // user aborted the dialog
	}

	// we either use surfaces from component table with invalid components, or we use
	// the selection, depending on argument 'fromSurfaceSelection'
	const std::vector<const VICUS::Surface * >	*selectedSurfaceVector;
	if (fromSurfaceSelection) {
		Q_ASSERT(!m_selectedSurfaces.empty());
		selectedSurfaceVector = &m_selectedSurfaces; // use surfaces from user selection
	}
	else {
		int row = m_ui->tableWidgetComponents->currentRow();
		Q_ASSERT(row != -1); // otherwise we shouldn't ever get here
		Q_ASSERT(row < (int)m_componentTable.size());
		const ComponentLegendEntry & e = m_componentTable[(unsigned int)row];
		selectedSurfaceVector = &e.m_surfaces; // use surfaces from component table
	}

	// transform vector into a set, since we need to search for IDs repeatedly
	std::set<const VICUS::Surface*> selectedSurfaces(selectedSurfaceVector->begin(), selectedSurfaceVector->end());

	std::vector<VICUS::ComponentInstance> compInstances;
	if (insideWall) {
		Q_ASSERT(selectedSurfaces.size() == 2); // must have exactly two
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
			std::set<const VICUS::Surface*>::iterator it = selectedSurfaces.find(ci.m_sideASurface);
			std::set<const VICUS::Surface*>::iterator it2 = selectedSurfaces.find(ci.m_sideBSurface);
			if (it != selectedSurfaces.end() || it2 != selectedSurfaces.end()) {
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
				if (it != selectedSurfaces.end()) {
					// side A connected?, set side B to the other surface
					if (it == selectedSurfaces.begin())
						newCi.m_idSideBSurface = (*selectedSurfaces.rbegin())->m_id;
					else
						newCi.m_idSideBSurface = (*selectedSurfaces.begin())->m_id;
				}
				else {
					// must be side B connected, set side A to the other surface
					if (it2 == selectedSurfaces.begin())
						newCi.m_idSideASurface = (*selectedSurfaces.rbegin())->m_id;
					else
						newCi.m_idSideASurface = (*selectedSurfaces.begin())->m_id;
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
			newCi.m_idSideASurface = (*selectedSurfaces.begin())->m_id;
			newCi.m_idSideBSurface = (*selectedSurfaces.rbegin())->m_id;
			compInstances.push_back(newCi);
		}

	}
	else {
		// now create a copy of the currently defined componentInstances
		compInstances = project().m_componentInstances;

		// process all componentInstances and for all that reference any of the selected surfaces, replace component
		for (VICUS::ComponentInstance & ci : compInstances) {
			std::set<const VICUS::Surface*>::iterator it = selectedSurfaces.find(ci.m_sideASurface);
			if (it != selectedSurfaces.end()) {
				ci.m_idComponent = (unsigned int)selectedComponentId;
				selectedSurfaces.erase(it);
				continue;
			}
			it = selectedSurfaces.find(ci.m_sideBSurface);
			if (it != selectedSurfaces.end()) {
				ci.m_idComponent = (unsigned int)selectedComponentId;
				selectedSurfaces.erase(it);
			}
		}

		// now all surfaces still left in selSurfaces are not yet referenced in ComponentInstances
		// hence, we need to create a new componentInstance for each of them
		if (!selectedSurfaces.empty()) {
			unsigned int nextId = VICUS::largestUniqueId(compInstances);
			for (const VICUS::Surface * s : selectedSurfaces) {
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


void SVPropBuildingComponentsWidget::on_pushButtonAssignComponentFromTable_clicked() {
	// find out which component is selected in table
	int r = m_ui->tableWidgetComponents->currentRow();
	Q_ASSERT(r != -1);
	Q_ASSERT(r < (int)m_componentTable.size());
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)r];
	Q_ASSERT(e.m_type == 2); // ensure we have a valid component
	// get component ID
	unsigned int componentID = e.m_component->m_id;
	assignComponent(false, true, componentID);
}
