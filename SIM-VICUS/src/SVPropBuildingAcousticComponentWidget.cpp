#include "SVPropBuildingAcousticComponentWidget.h"
#include "ui_SVPropBuildingAcousticComponentWidget.h"


#include "SVStyle.h"
#include "SVMainWindow.h"
#include <SVConversions.h>
#include <SVProjectHandler.h>
#include <VICUS_utilities.h>
#include "SVDatabaseEditDialog.h"
#include "SVUndoModifyComponentInstances.h"
#include "SVUndoTreeNodeState.h"


SVPropBuildingAcousticComponentWidget::SVPropBuildingAcousticComponentWidget(QWidget *parent) :
    QWidget(parent),
	m_ui(new Ui::SVPropBuildingAcousticComponentWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->tableWidgetAcousticComponents->setColumnCount(2);
	m_ui->tableWidgetAcousticComponents->setHorizontalHeaderLabels(QStringList() << QString() << tr("Component"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetAcousticComponents);
	m_ui->tableWidgetAcousticComponents->setSortingEnabled(false);
	m_ui->tableWidgetAcousticComponents->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetAcousticComponents->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetAcousticComponents->horizontalHeader()->setStretchLastSection(true);
}

SVPropBuildingAcousticComponentWidget::~SVPropBuildingAcousticComponentWidget()
{
	delete m_ui;
}


void SVPropBuildingAcousticComponentWidget::updateUi() {
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
	std::map<const VICUS::AcousticComponent *, std::set<const VICUS::Surface*> > surfaceComponentMap;

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
		const VICUS::AcousticComponent * acComp = SVSettings::instance().m_db.m_acousticComponents[ci.m_idAcousticComponent];
		if (acComp == nullptr) {
			// no component or invalid component, remember in set "without component"
			surfacesWithoutComponent.insert(visibleSideASurf);
			surfacesWithoutComponent.insert(visibleSideBSurf);
			continue; // nothing else to do
		}

		surfaceComponentMap[acComp].insert(visibleSideASurf);
		surfaceComponentMap[acComp].insert(visibleSideBSurf);
	}

	// remove nullptr (added for any non-surf objects)
	surfacesNotInComponentInterface.erase(nullptr);
	surfacesWithoutComponent.erase(nullptr);

	// Note: in case of invalid CI with surface IDs that do not exist, we
	//       may have a nullptrs for surfaces, so we need to remove those
	for (std::map<const VICUS::AcousticComponent *, std::set<const VICUS::Surface*> >::iterator it = surfaceComponentMap.begin();
		 it != surfaceComponentMap.end(); ++it)
	{
		it->second.erase(nullptr);
	}

	// now store the data into the vector that is used for generating the table
	m_acousticComponentTable.clear();

	if (!surfacesNotInComponentInterface.empty()) {
		AcousticComponentLegendEntry ale;
		ale.m_type = 0; // not connected/no component
		ale.m_surfaces = std::vector<const VICUS::Surface*>(surfacesNotInComponentInterface.begin(), surfacesNotInComponentInterface.end());
		// component ID is not needed
		m_acousticComponentTable.push_back(ale);
	}

	if (!surfacesWithoutComponent.empty()) {
		AcousticComponentLegendEntry ale;
		ale.m_type = 1; // connected, but without component
		ale.m_surfaces = std::vector<const VICUS::Surface*>(surfacesWithoutComponent.begin(), surfacesWithoutComponent.end());
		// component ID is not needed
		m_acousticComponentTable.push_back(ale);
	}

	// finally add all the rest
	Q_ASSERT(surfaceComponentMap.find(nullptr) == surfaceComponentMap.end());
	for (std::map<const VICUS::AcousticComponent *, std::set<const VICUS::Surface*> >::const_iterator it = surfaceComponentMap.begin();
		 it != surfaceComponentMap.end(); ++it)
	{
		AcousticComponentLegendEntry ale;
		ale.m_type = 2; // connected with valid component
		ale.m_surfaces = std::vector<const VICUS::Surface*>(it->second.begin(), it->second.end());
		ale.m_acousticComponent = it->first;
		m_acousticComponentTable.push_back(ale);
	}

	// ** legend table **

	int currentRow = m_ui->tableWidgetAcousticComponents->currentRow();
	m_ui->tableWidgetAcousticComponents->blockSignals(true);
	m_ui->tableWidgetAcousticComponents->clearContents();
	m_ui->tableWidgetAcousticComponents->setRowCount(m_acousticComponentTable.size());
	for (unsigned int row = 0; row < m_acousticComponentTable.size(); ++row) {
		QTableWidgetItem * colorItem = new QTableWidgetItem();
		QTableWidgetItem * item = new QTableWidgetItem();
		// special handling for components with "invalid" component id
		const AcousticComponentLegendEntry & ale = m_acousticComponentTable[row];
		switch (ale.m_type) {
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
				colorItem->setBackground(ale.m_acousticComponent->m_color);
				item->setText(QtExt::MultiLangString2QString(ale.m_acousticComponent->m_displayName));
			break;
		}
		colorItem->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		m_ui->tableWidgetAcousticComponents->setItem((int)row, 0, colorItem);

		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetAcousticComponents->setItem((int)row, 1, item);
	}
	// reselect row
	m_ui->tableWidgetAcousticComponents->blockSignals(false);
	m_ui->tableWidgetAcousticComponents->selectRow(std::min(currentRow, m_ui->tableWidgetAcousticComponents->rowCount()-1));


	// ** selected surfaces group box **

	// process all selected surfaces and determine which component they have assigned
	if (m_selectedSurfaces.empty()) {
		m_ui->labelSelectedComponents->setText("");
		m_ui->groupBoxSelectedComponent->setEnabled(false);
	}
	else {
		m_ui->groupBoxSelectedComponent->setEnabled(true);
	}
	m_ui->pushButtonAssignInsideAcousticComponent->setEnabled(m_selectedSurfaces.size() == 2);

	// update selection-related info

	std::set<const VICUS::AcousticComponent *> selectedComponents;
	const SVDatabase & db = SVSettings::instance().m_db;
	for (const VICUS::Surface * s : m_selectedSurfaces) {
		if (s->m_componentInstance != nullptr) {
			const VICUS::AcousticComponent * surfcomp = db.m_acousticComponents[s->m_componentInstance->m_idAcousticComponent];
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
	on_tableWidgetAcousticComponents_itemSelectionChanged();
}


void SVPropBuildingAcousticComponentWidget::on_tableWidgetAcousticComponents_itemSelectionChanged() {
	// disable all by default
	m_ui->pushButtonEditAcousticComponents->setEnabled(false);
	m_ui->pushButtonExchangeAcousticComponents->setEnabled(false);
	m_ui->pushButtonSelectObjectsWithAcousticComponent->setEnabled(false);
	m_ui->pushButtonAssignAcousticComponentFromTable->setEnabled(false);

	// check if the table is empty or there is no currently selected row
	int r = m_ui->tableWidgetAcousticComponents->currentRow();
	if (r == -1)
		return;

	// exchanging/assigning a component is always possible
	m_ui->pushButtonExchangeAcousticComponents->setEnabled(true);

	// the select button is always active, even if no component is assigned, yet
	m_ui->pushButtonSelectObjectsWithAcousticComponent->setEnabled(true);

	Q_ASSERT(r < (int)m_acousticComponentTable.size());
	const AcousticComponentLegendEntry & e = m_acousticComponentTable[(unsigned int)r];
	if (e.m_type == 2) {
		// valid component, can be edited
		m_ui->pushButtonEditAcousticComponents->setEnabled(true);
		// the assign-from-table button is only available when there is at least one surface selected
		m_ui->pushButtonAssignAcousticComponentFromTable->setEnabled(!m_selectedSurfaces.empty());
	}

}


void SVPropBuildingAcousticComponentWidget::on_pushButtonEditAcousticComponents_clicked() {
	openEditComponentDialog();
}




void SVPropBuildingAcousticComponentWidget::on_tableWidgetAcousticComponents_cellDoubleClicked(/*int row, int column*/) {
	// check if the acoustic component is valid
	int r = m_ui->tableWidgetAcousticComponents->currentRow();
	if (r == -1)
		return;
	Q_ASSERT(r < (int)m_acousticComponentTable.size());
	const AcousticComponentLegendEntry & e = m_acousticComponentTable[(unsigned int)r];
	if (e.m_type == 2) {
		openEditComponentDialog();
	}
}


void SVPropBuildingAcousticComponentWidget::assignAcousticComponent(bool insideWall, bool fromSurfaceSelection, unsigned int selectedAcousticComponentId) {
	// ask user to select a new component, unless given
	if (selectedAcousticComponentId == VICUS::INVALID_ID) {
		SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignComponent",
			tr("Assign acoustic component"), tr("You may now select an acoustic component from the database, which will then be "
									   "assigned to the selected surfaces."));
		selectedAcousticComponentId = SVMainWindow::instance().dbAcousticComponentEditDialog()->select(VICUS::INVALID_ID);
		if (selectedAcousticComponentId == VICUS::INVALID_ID)
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
		// selects all surfaces of one component (for exchange function)
		int row = m_ui->tableWidgetAcousticComponents->currentRow();
		Q_ASSERT(row != -1); // otherwise we shouldn't ever get here
		Q_ASSERT(row < (int)m_acousticComponentTable.size());
		const AcousticComponentLegendEntry & e = m_acousticComponentTable[(unsigned int)row];
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
				newCi.m_idAcousticComponent = (unsigned int)selectedAcousticComponentId;
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
			newCi.m_idComponent = (unsigned int)selectedAcousticComponentId;
			newCi.m_idSideASurface = (*selectedSurfaces.begin())->m_id;
			newCi.m_idSideBSurface = (*selectedSurfaces.rbegin())->m_id;
			compInstances.push_back(newCi);
		}

	}
	else {
		// now create a copy of the currently defined componentInstances
		compInstances = project().m_componentInstances;

		// lookup component based on selection
		const VICUS::AcousticComponent * newAcComp = SVSettings::instance().m_db.m_acousticComponents[selectedAcousticComponentId];
		Q_ASSERT(newAcComp != nullptr);

		// process all componentInstances and for all that reference any of the selected surfaces, replace component
		for (VICUS::ComponentInstance & ci : compInstances) {
			std::set<const VICUS::Surface*>::iterator itA = selectedSurfaces.find(ci.m_sideASurface);
			std::set<const VICUS::Surface*>::iterator itB = selectedSurfaces.find(ci.m_sideBSurface);
			if (itA != selectedSurfaces.end()) {
				ci.m_idAcousticComponent = (unsigned int)selectedAcousticComponentId;
				// if new component instance does not have an active layer, also remove the surface heating and control zone
				// properties
//				if (newComp->m_activeLayerIndex == VICUS::INVALID_ID) {
//					ci.m_idSurfaceHeatingControlZone = VICUS::INVALID_ID;
//					ci.m_idSurfaceHeating = VICUS::INVALID_ID;
//				}
				selectedSurfaces.erase(itA);
				// don't use continue here, we may also have sideBSurface in this component, so we want to erase the sideB surface as well
			}

			if (itB != selectedSurfaces.end()) {
				ci.m_idAcousticComponent = (unsigned int)selectedAcousticComponentId;
				// if new component instance does not have an active layer, also remove the surface heating and control zone
				// properties
//				if (newComp->m_activeLayerIndex == VICUS::INVALID_ID) {
//					ci.m_idSurfaceHeatingControlZone = VICUS::INVALID_ID;
//					ci.m_idSurfaceHeating = VICUS::INVALID_ID;
//				}
				selectedSurfaces.erase(itB);
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
				c.m_idAcousticComponent = (unsigned int)selectedAcousticComponentId;
				compInstances.push_back(c);
			}
		}

	}
	// create the undo action and modify project
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Acoustic Components assigned"), compInstances);
	undo->push();
}



void SVPropBuildingAcousticComponentWidget::openEditComponentDialog() {
	const VICUS::AcousticComponent * acComp = currentlySelectedComponent();

	// when we have an invalid component we assign a new component instead of editing
	if(acComp == nullptr){
		on_pushButtonAssignAcousticComponent_clicked();
		return;
	}

	int currentRow = m_ui->tableWidgetAcousticComponents->currentRow();
	SVMainWindow::instance().dbAcousticComponentEditDialog()->edit(acComp->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
	// reselect row, because closing the edit dialog recreates the table (due to possible color change)
	if (m_ui->tableWidgetAcousticComponents->rowCount() > currentRow)
		m_ui->tableWidgetAcousticComponents->selectRow(currentRow);
}

const VICUS::AcousticComponent * SVPropBuildingAcousticComponentWidget::currentlySelectedComponent() const {
	int row = m_ui->tableWidgetAcousticComponents->currentRow();
	if (row == -1)
		return nullptr;
	Q_ASSERT(row < (int)m_acousticComponentTable.size());
	const AcousticComponentLegendEntry & e = m_acousticComponentTable[(unsigned int)row];
	if (e.m_type != 2)
		return nullptr; // not a valid component assigned
	return e.m_acousticComponent;
}


void SVPropBuildingAcousticComponentWidget::on_pushButtonExchangeAcousticComponents_clicked() {
	// we have to distinguish between the different cases:
	// type 0 = not connected surfaces -> assign component to surface (i.e. create new componentInstances)
	// all other types, modify component instances to point to new component

	// now compose an undo action and modify the project
	int r = m_ui->tableWidgetAcousticComponents->currentRow();
	Q_ASSERT(r != -1);
	const AcousticComponentLegendEntry & e = m_acousticComponentTable[(unsigned int)r];

	if (e.m_type == 0) {
		assignAcousticComponent(false, false); // no inside wall, not using selected surfaces, but surfaces from component table entry
		return;
	}

	const VICUS::AcousticComponent * acComp = e.m_acousticComponent;
	// acComp may be invalid, in which case we actually "assign" a component, rather than replace it
	unsigned int oldId = VICUS::INVALID_ID;
	QString msg;
	if (acComp != nullptr) {
		oldId = acComp->m_id;
		msg = tr("This will replace all associations with component '%1 [%2]' with another component.")
				.arg(QtExt::MultiLangString2QString(acComp->m_displayName)).arg(oldId);
	}
	else {
		msg = tr("You may now select a component to be assigned to the surfaces.");
	}

	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoReplaceComponent",
		tr("Replace component"), msg);
	unsigned int newId = SVMainWindow::instance().dbAcousticComponentEditDialog()->select(oldId);
	if (newId == VICUS::INVALID_ID)
		return; // user has aborted the dialog

	const VICUS::AcousticComponent * newAcComp = SVSettings::instance().m_db.m_acousticComponents[newId];
	Q_ASSERT(newAcComp != nullptr);

	// now process all component instances and modify the component
	std::vector<VICUS::ComponentInstance> modCI = project().m_componentInstances;
	for (VICUS::ComponentInstance & ci : modCI) {
		// we identify the component instance to modify in two ways
		if (e.m_type == 2) {
			// for valid component instances, we use the old Id as identifier
			if (ci.m_idAcousticComponent == oldId) {

				bool surfA = ci.m_sideASurface != nullptr;
				bool isSurfAVisible = ci.m_sideASurface != nullptr && ci.m_sideASurface->m_visible;
				bool surfB = ci.m_sideBSurface != nullptr;
				bool isSurfBVisible = ci.m_sideBSurface != nullptr && ci.m_sideBSurface->m_visible;

				if( (surfA && !isSurfAVisible) || (surfB && !isSurfBVisible) )
					continue;

				ci.m_idAcousticComponent = (unsigned int)newId;
			}
		}
		else {
			// for invalid/missing component assignments
			if (ci.m_idAcousticComponent == VICUS::INVALID_ID ||
				SVSettings::instance().m_db.m_acousticComponents[ci.m_idAcousticComponent] == nullptr)
			{
				ci.m_idAcousticComponent = (unsigned int)newId;
//				}
			}
		}
	}
	// create the undo action and modify project
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Components exchanged"), modCI);
	undo->push();

}


void SVPropBuildingAcousticComponentWidget::on_pushButtonAssignAcousticComponent_clicked() {
	assignAcousticComponent(false, true);
}


void SVPropBuildingAcousticComponentWidget::on_pushButtonAssignInsideAcousticComponent_clicked() {
	assignAcousticComponent(true, true);
}


void SVPropBuildingAcousticComponentWidget::on_pushButtonAssignAcousticComponentFromTable_clicked() {
	// find out which component is selected in table
	int r = m_ui->tableWidgetAcousticComponents->currentRow();
	Q_ASSERT(r != -1);
	Q_ASSERT(r < (int)m_acousticComponentTable.size());
	const AcousticComponentLegendEntry & e = m_acousticComponentTable[(unsigned int)r];
	Q_ASSERT(e.m_type == 2); // ensure we have a valid component
	// get component ID
	unsigned int componentID = e.m_acousticComponent->m_id;
	assignAcousticComponent(false, true, componentID);
}


void SVPropBuildingAcousticComponentWidget::on_pushButtonSelectObjectsWithAcousticComponent_clicked() {
	const VICUS::AcousticComponent * acComp = nullptr;
	int row = m_ui->tableWidgetAcousticComponents->currentRow();
	Q_ASSERT(row != -1);
	const AcousticComponentLegendEntry & e = m_acousticComponentTable[(unsigned int)row];
	if (e.m_type == 2)
		acComp = e.m_acousticComponent;

	QString undoText;
	if (acComp != nullptr)
		undoText = tr("Select objects with component '%1'").arg(QtExt::MultiLangString2QString(acComp->m_displayName));
	else
		undoText = tr("Select objects with invalid/missing component.");

	std::set<unsigned int> surfIds;
	for (const VICUS::Surface * s : e.m_surfaces)
		surfIds.insert(s->m_id);
	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, surfIds, true);
	undo->push();
}

