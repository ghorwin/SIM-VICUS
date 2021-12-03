#include "SVPropBuildingSubComponentsWidget.h"
#include "ui_SVPropBuildingSubComponentsWidget.h"

#include <QtExt_Conversions.h>

#include <VICUS_utilities.h>

#include "SVStyle.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVUndoTreeNodeState.h"
#include "SVUndoModifySubSurfaceComponentInstances.h"

SVPropBuildingSubComponentsWidget::SVPropBuildingSubComponentsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingSubComponentsWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->tableWidgetSubSurfaceComponents->setColumnCount(3);
	m_ui->tableWidgetSubSurfaceComponents->setHorizontalHeaderLabels(QStringList() << QString() << tr("Type") << tr("Sub-Surface Component") );
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSubSurfaceComponents);
	m_ui->tableWidgetSubSurfaceComponents->setSortingEnabled(false);
	m_ui->tableWidgetSubSurfaceComponents->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetSubSurfaceComponents->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetSubSurfaceComponents->horizontalHeader()->setStretchLastSection(true);
}


SVPropBuildingSubComponentsWidget::~SVPropBuildingSubComponentsWidget() {
	delete m_ui;
}


void SVPropBuildingSubComponentsWidget::updateUi() {
	// see documentation in SVPropBuildingComponentsWidget::updateUi()

	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true); // visible (selected and not selected)

	std::set<const VICUS::SubSurface * > surfacesNotInComponentInterface; // note: we use a set, because we want to quickly remove items later
	std::set<const VICUS::SubSurface *> surfacesWithoutComponent;

	m_selectedSurfaces.clear();
	for (const VICUS::Object * o : objs) {
		const VICUS::SubSurface * surf = dynamic_cast<const VICUS::SubSurface * >(o);
		if (surf != nullptr) {
			surfacesNotInComponentInterface.insert(surf);
			if (surf->m_selected)
				m_selectedSurfaces.push_back(surf);
		}
	}

	m_surfaceComponentMap.clear();
	for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
		const VICUS::SubSurface * visibleSideASurf = nullptr;
		// side A
		if (ci.m_sideASubSurface != nullptr) {
			if (ci.m_sideASubSurface->m_visible)
				visibleSideASurf = ci.m_sideASubSurface;
		}
		// side B
		const VICUS::SubSurface * visibleSideBSurf = nullptr;
		if (ci.m_sideBSubSurface != nullptr) {
			if (ci.m_sideBSubSurface->m_visible)
				visibleSideBSurf = ci.m_sideBSubSurface;
		}

		// ignore sub-component instances, where both surfaces are invisible
		if (visibleSideASurf == nullptr && visibleSideBSurf == nullptr)
			continue;

		// remove from visibleSurfaces set since this surface was referenced in componentInstance
		surfacesNotInComponentInterface.erase(visibleSideASurf);
		surfacesNotInComponentInterface.erase(visibleSideBSurf);

		// valid component assigned?
		const VICUS::SubSurfaceComponent * comp = SVSettings::instance().m_db.m_subSurfaceComponents[ci.m_idSubSurfaceComponent];
		if (comp == nullptr) {
			// no component or invalid component, remember in set "without component"
			surfacesWithoutComponent.insert(visibleSideASurf);
			surfacesWithoutComponent.insert(visibleSideBSurf);
			continue; // nothing else to do
		}

		m_surfaceComponentMap[comp].insert(visibleSideASurf);
		m_surfaceComponentMap[comp].insert(visibleSideBSurf);
	}

	// remove nullptr (added for any non-surf objects)
	surfacesNotInComponentInterface.erase(nullptr);
	surfacesWithoutComponent.erase(nullptr);

	// Note: in case of invalid CI with surface IDs that do not exist, we
	//       may have a nullptrs for surfaces, so we need to remove those
	for (std::map<const VICUS::SubSurfaceComponent *, std::set<const VICUS::SubSurface*> >::iterator it = m_surfaceComponentMap.begin();
		 it != m_surfaceComponentMap.end(); ++it)
	{
		it->second.erase(nullptr);
	}

	// now store the data into the vector that is used for generating the table
	m_componentTable.clear();

	if (!surfacesNotInComponentInterface.empty()) {
		ComponentLegendEntry le;
		le.m_type = 0; // not connected/no component
		le.m_surfaces = std::vector<const VICUS::SubSurface*>(surfacesNotInComponentInterface.begin(), surfacesNotInComponentInterface.end());
		// component ID is not needed
		m_componentTable.push_back(le);
	}

	if (!surfacesWithoutComponent.empty()) {
		ComponentLegendEntry le;
		le.m_type = 1; // connected, but without component
		le.m_surfaces = std::vector<const VICUS::SubSurface*>(surfacesWithoutComponent.begin(), surfacesWithoutComponent.end());
		// component ID is not needed
		m_componentTable.push_back(le);
	}

	// finally add all the rest
	Q_ASSERT(m_surfaceComponentMap.find(nullptr) == m_surfaceComponentMap.end());
	for (std::map<const VICUS::SubSurfaceComponent *, std::set<const VICUS::SubSurface*> >::const_iterator it = m_surfaceComponentMap.begin();
		 it != m_surfaceComponentMap.end(); ++it)
	{
		ComponentLegendEntry le;
		le.m_type = 2; // connected with valid component
		le.m_surfaces = std::vector<const VICUS::SubSurface*>(it->second.begin(), it->second.end());
		le.m_component = it->first;
		m_componentTable.push_back(le);
	}

	// ** legend table **

	int currentRow = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	m_ui->tableWidgetSubSurfaceComponents->blockSignals(true);
	m_ui->tableWidgetSubSurfaceComponents->clearContents();
	m_ui->tableWidgetSubSurfaceComponents->setRowCount(m_componentTable.size());
	for (unsigned int row = 0; row < m_componentTable.size(); ++row) {
		QTableWidgetItem * colorItem = new QTableWidgetItem();
		QTableWidgetItem * item = new QTableWidgetItem();
		// special handling for components with "invalid" component id
		const ComponentLegendEntry & le = m_componentTable[row];
		switch (le.m_type) {
			// not assigned - dark gray
			case 0 :
				colorItem->setBackground(QColor(64,64,64));
				item->setText(tr("Unused sub-surfaces (not connected/assigned)"));
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
		m_ui->tableWidgetSubSurfaceComponents->setItem((int)row, 0, colorItem);

		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetSubSurfaceComponents->setItem((int)row, 1, item);
	}
	// reselect row
	m_ui->tableWidgetSubSurfaceComponents->blockSignals(false);
	m_ui->tableWidgetSubSurfaceComponents->selectRow(std::min(currentRow, m_ui->tableWidgetSubSurfaceComponents->rowCount()-1));

	// ** selected surfaces group box **

	// process all selected surfaces and determine which component they have assigned
	if (m_selectedSurfaces.empty()) {
		m_ui->labelSelectedSubSurfaceComponents->setText("");
		m_ui->groupBoxSelectedSubComponent->setEnabled(false);
	}
	else {
		m_ui->groupBoxSelectedSubComponent->setEnabled(true);
	}
	m_ui->pushButtonAssignInsideSubSurfaceComponent->setEnabled(m_selectedSurfaces.size() == 2);

	// update selection-related info

	std::set<const VICUS::SubSurfaceComponent *> selectedComponents;
	const SVDatabase & db = SVSettings::instance().m_db;
	for (const VICUS::SubSurface * s : m_selectedSurfaces) {
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
			m_ui->labelSelectedSubSurfaceComponents->setText(tr("Component with invalid/unknown ID"));
		else
			m_ui->labelSelectedSubSurfaceComponents->setText(tr("%1 [%2]")
				.arg(QtExt::MultiLangString2QString((*selectedComponents.begin())->m_displayName)).arg((*selectedComponents.begin())->m_id));
	}
	else {
		m_ui->labelSelectedSubSurfaceComponents->setText(tr("%1 different components")
			.arg(selectedComponents.size()));
	}
	// update table related button states
	on_tableWidgetSubSurfaceComponents_itemSelectionChanged();
}


void SVPropBuildingSubComponentsWidget::on_tableWidgetSubSurfaceComponents_itemSelectionChanged() {
	// disable all by default
	m_ui->pushButtonEditSubSurfaceComponents->setEnabled(false);
	m_ui->pushButtonExchangeSubSurfaceComponents->setEnabled(false);
	m_ui->pushButtonSelectObjectsWithSubSurfaceComponent->setEnabled(false);

	// check if the table is empty or there is no currently selected row
	int r = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	if (r == -1)
		return;

	// exchanging/assigning a component is always possible
	m_ui->pushButtonExchangeSubSurfaceComponents->setEnabled(true);

	// the select buttons are always active, even if no component is assigned, yet
	m_ui->pushButtonSelectObjectsWithSubSurfaceComponent->setEnabled(true);

	// enable/disable buttons that require valid components
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)r];
	if (e.m_type == 2) {
		// valid component, can be edited
		m_ui->pushButtonEditSubSurfaceComponents->setEnabled(true);
	}
}


void SVPropBuildingSubComponentsWidget::on_pushButtonEditSubSurfaceComponents_clicked() {
	const VICUS::SubSurfaceComponent * comp = currentlySelectedSubSurfaceComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	int currentRow = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	SVMainWindow::instance().dbSubSurfaceComponentEditDialog()->edit(comp->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
	// reselect row, because closing the edit dialog recreates the table (due to possible color change)
	if (m_ui->tableWidgetSubSurfaceComponents->rowCount() > currentRow)
		m_ui->tableWidgetSubSurfaceComponents->selectRow(currentRow);
}


void SVPropBuildingSubComponentsWidget::on_pushButtonExchangeSubSurfaceComponents_clicked() {
	// we have to distinguish between the different cases:
	// type 0 = not connected surfaces -> assign component to surface (i.e. create new componentInstances)
	// all other types, modify component instances to point to new component

	// now compose an undo action and modify the project
	int r = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	Q_ASSERT(r != -1);
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)r];

	if (e.m_type == 0) {
		assignSubSurfaceComponent(false, false);
		return;
	}

	const VICUS::SubSurfaceComponent * comp = e.m_component;
	// comp may be invalid, in which case we actually "assign" a component, rather than replace it
	unsigned int oldId = VICUS::INVALID_ID;
	QString msg;
	if (comp != nullptr) {
		oldId = comp->m_id;
		QString msg = tr("This will replace all associations with sub-surface component '%1 [%2]' with another sub-surface component.")
				.arg(QtExt::MultiLangString2QString(comp->m_displayName)).arg(oldId);
	}
	else {
		msg = tr("You may now select a sub-surface component to be assigned to the surfaces.");
	}

	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoReplaceSubSurfaceComponent",
		tr("Replace sub-surface component"), msg);
	unsigned int newId = SVMainWindow::instance().dbSubSurfaceComponentEditDialog()->select(oldId);
	if (newId == VICUS::INVALID_ID)
		return; // user has aborted the dialog

	// now process all component instances and modify the component
	std::vector<VICUS::SubSurfaceComponentInstance> modCI = project().m_subSurfaceComponentInstances;
	for (VICUS::SubSurfaceComponentInstance & ci : modCI) {
		// we identify the component instance to modify in two ways
		if (e.m_type == 2) {
			// for valid component instances, we use the old Id as identifier
			if (ci.m_idSubSurfaceComponent == oldId)
				ci.m_idSubSurfaceComponent = (unsigned int)newId;
		}
		else {
			// for invalid/missing component assignments
			if (ci.m_idSubSurfaceComponent == VICUS::INVALID_ID ||
				SVSettings::instance().m_db.m_subSurfaceComponents[ci.m_idSubSurfaceComponent] == nullptr)
			{
				ci.m_idSubSurfaceComponent = (unsigned int)newId;
			}
		}
	}
	// create the undo action and modify project
	SVUndoModifySubSurfaceComponentInstances * undo = new SVUndoModifySubSurfaceComponentInstances(tr("Sub-surface components exchanged"), modCI);
	undo->push();
}


void SVPropBuildingSubComponentsWidget::on_pushButtonSelectObjectsWithSubSurfaceComponent_clicked() {
	const VICUS::SubSurfaceComponent * comp = nullptr;
	int row = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	Q_ASSERT(row != -1);
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)row];
	if (e.m_type == 2)
		comp = e.m_component;

	QString undoText;
	if (comp != nullptr)
		undoText = tr("Select objects with sub-surface component '%1'").arg(QtExt::MultiLangString2QString(comp->m_displayName));
	else
		undoText = tr("Select objects with invalid/missing sub-surface component.");

	std::set<unsigned int> surfIds;
	for (const VICUS::SubSurface * s : e.m_surfaces)
		surfIds.insert(s->uniqueID());

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, surfIds, true);
	undo->push();
}


void SVPropBuildingSubComponentsWidget::on_pushButtonAssignSubSurfaceComponent_clicked() {
	assignSubSurfaceComponent(false, true);
}


void SVPropBuildingSubComponentsWidget::on_pushButtonAssignInsideSubSurfaceComponent_clicked() {
	assignSubSurfaceComponent(true, true);
}


void SVPropBuildingSubComponentsWidget::assignSubSurfaceComponent(bool insideWall, bool fromSurfaceSelection) {
	// ask user to select a new component
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignSubComponent",
		tr("Assign component"), tr("You may now select a sub-surface component from the database, which will then be "
								   "assigned to the selected sub-surfaces."));
	unsigned int selectedComponentId = SVMainWindow::instance().dbSubSurfaceComponentEditDialog()->select(VICUS::INVALID_ID);
	if (selectedComponentId == VICUS::INVALID_ID)
		return; // user aborted the dialog

	// we either use surfaces from component table with invalid components, or we use
	// the selection, depending on argument 'fromSurfaceSelection'
	const std::vector<const VICUS::SubSurface * >	*selectedSurfaceVector;
	if (fromSurfaceSelection) {
		Q_ASSERT(!m_selectedSurfaces.empty());
		selectedSurfaceVector = &m_selectedSurfaces; // use surfaces from user selection
	}
	else {
		int row = m_ui->tableWidgetSubSurfaceComponents->currentRow();
		Q_ASSERT(row != -1); // otherwise we shouldn't ever get here
		Q_ASSERT(row < (int)m_componentTable.size());
		const ComponentLegendEntry & e = m_componentTable[(unsigned int)row];
		selectedSurfaceVector = &e.m_surfaces; // use surfaces from component table
	}

	// transform vector into a set, since we need to search for IDs repeatedly
	std::set<const VICUS::SubSurface*> selectedSurfaces(selectedSurfaceVector->begin(), selectedSurfaceVector->end());

	std::vector<VICUS::SubSurfaceComponentInstance> compInstances;
	if (insideWall) {
		Q_ASSERT(selectedSurfaces.size() == 2); // must have exactly two

		// we achieve all that with just one loop
		bool compInstanceFound = false;
		// process all component instances
		for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
			// does the component instance reference either of our selected surfaces/or both?
			std::set<const VICUS::SubSurface*>::iterator it = selectedSurfaces.find(ci.m_sideASubSurface);
			std::set<const VICUS::SubSurface*>::iterator it2 = selectedSurfaces.find(ci.m_sideBSubSurface);
			if (it != selectedSurfaces.end() || it2 != selectedSurfaces.end()) {
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
				const VICUS::SubSurface * surfA = project().subSurfaceByID(newCi.m_idSideASurface);
				const VICUS::SubSurface * surfB = project().subSurfaceByID(newCi.m_idSideBSurface);
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
			VICUS::SubSurfaceComponentInstance newCi;
			unsigned int nextId = VICUS::largestUniqueId(compInstances);
			newCi.m_id = nextId;
			newCi.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
			newCi.m_idSideASurface = (*selectedSurfaces.begin())->m_id;
			newCi.m_idSideBSurface = (*selectedSurfaces.rbegin())->m_id;
			compInstances.push_back(newCi);
		}

	}
	else {
		// now create a copy of the currently defined componentInstances
		compInstances = project().m_subSurfaceComponentInstances;

		// process all componentInstances and for all that reference any of the selected surfaces, replace component
		for (VICUS::SubSurfaceComponentInstance & ci : compInstances) {
			std::set<const VICUS::SubSurface*>::iterator it = selectedSurfaces.find(ci.m_sideASubSurface);
			if (it != selectedSurfaces.end()) {
				ci.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
				selectedSurfaces.erase(it);
				continue;
			}
			it = selectedSurfaces.find(ci.m_sideBSubSurface);
			if (it != selectedSurfaces.end()) {
				ci.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
				selectedSurfaces.erase(it);
			}
		}

		// now all surfaces still left in selSurfaces are not yet referenced in ComponentInstances
		// hence, we need to create a new componentInstance for each of them
		if (!selectedSurfaces.empty()) {
			unsigned int nextId = VICUS::largestUniqueId(compInstances);
			for (const VICUS::SubSurface * s : selectedSurfaces) {
				VICUS::SubSurfaceComponentInstance c;
				c.m_id = nextId++;
				c.m_idSideASurface = s->m_id;
				c.m_idSubSurfaceComponent = (unsigned int)selectedComponentId;
				compInstances.push_back(c);
			}
		}

	}
	// create the undo action and modify project
	SVUndoModifySubSurfaceComponentInstances * undo = new SVUndoModifySubSurfaceComponentInstances(tr("Sub-Components assigned"), compInstances);
	undo->push();
}


const VICUS::SubSurfaceComponent * SVPropBuildingSubComponentsWidget::currentlySelectedSubSurfaceComponent() const {
	int row = m_ui->tableWidgetSubSurfaceComponents->currentRow();
	if (row == -1)
		return nullptr;
	Q_ASSERT(row < (int)m_componentTable.size());
	const ComponentLegendEntry & e = m_componentTable[(unsigned int)row];
	if (e.m_type != 2)
		return nullptr; // not a valid component assigned
	return e.m_component;
}

