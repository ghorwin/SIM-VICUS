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
#include "SVDBComponentEditDialog.h"

#include "SVUndoTreeNodeState.h"
#include "SVUndoModifyComponentInstances.h"

SVPropBuildingEditWidget::SVPropBuildingEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingEditWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);
	m_ui->verticalLayoutComponents->setMargin(0);
	m_ui->verticalLayoutComponentOrientation->setMargin(0);
	m_ui->verticalLayoutBoundaryConditions->setMargin(0);

	// configure tables
	m_ui->tableWidgetComponents->setColumnCount(2);
	m_ui->tableWidgetComponents->setHorizontalHeaderLabels(QStringList() << QString() << tr("Component"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetComponents);
	m_ui->tableWidgetComponents->setSortingEnabled(false);
	m_ui->tableWidgetComponents->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetComponents->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetComponents->horizontalHeader()->setStretchLastSection(true);

	m_ui->tableWidgetBoundaryConditions->setColumnCount(2);
	m_ui->tableWidgetBoundaryConditions->setHorizontalHeaderLabels(QStringList() << QString() << tr("Boundary condition"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetBoundaryConditions);
	m_ui->tableWidgetBoundaryConditions->setSortingEnabled(false);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetBoundaryConditions->horizontalHeader()->setStretchLastSection(true);

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

	// update widget to current project's content
	onModified(SVProjectHandler::AllModified, nullptr);
}


SVPropBuildingEditWidget::~SVPropBuildingEditWidget() {
	delete m_ui;
}


void SVPropBuildingEditWidget::setPropertyType(int buildingPropertyType) {

	// TODO : Andreas, there is quite some overhead and duplicate work involved in
	//        this function. When this becomes a performance issue, improve this
	//        by moving all the data collection stuff to front and let the UI update
	//        only based on cached quantities.

	m_propertyType = buildingPropertyType;

	switch ((BuildingPropertyTypes)buildingPropertyType) {
		case BT_Components				: m_ui->stackedWidget->setCurrentIndex(1); break;
		case BT_ComponentOrientation	: m_ui->stackedWidget->setCurrentIndex(2); break;
		case BT_BoundaryConditions		: m_ui->stackedWidget->setCurrentIndex(3); break;
	}
}


void SVPropBuildingEditWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// react on selection changes only, then update properties
	switch ((SVProjectHandler::ModificationTypes)modificationType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::ComponentInstancesModified:
		case SVProjectHandler::NodeStateModified:
			updateUi(); // we do not change the property type here
		break;
	}
}


void SVPropBuildingEditWidget::on_pushButtonEditComponents_clicked() {
	const VICUS::Component * comp = currentlySelectedComponent();
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	SVMainWindow::instance().dbComponentEditDialog()->edit(comp->m_id);
	// TODO : signal a "recoloring needed" signal to scene in case any of the colors have changed
	// update table (in case user has deleted some components or changed colors
	setPropertyType(BT_Components);
	// Note: project data isn't modified, since only user DB data was changed.
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
	// TODO Andreas: signal a "recoloring needed" signal to scene in case any of the colors have changed
	// update table (in case user has deleted some components or changed colors
	setPropertyType(BT_Components); // Note: this invalidates "comp"!

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
	Q_ASSERT(comp != nullptr); // if nullptr, the button should be disabled!
	// compose set of objects to be selected
	std::set<unsigned int> objs;
	for (auto s : m_componentSurfacesMap[comp])
		objs.insert(s->uniqueID());

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Select objects with component '%1'")
		.arg(QtExt::MultiLangString2QString(comp->m_displayName)), SVUndoTreeNodeState::SelectedState,
														 objs, true);
	undo->push();
}


void SVPropBuildingEditWidget::on_tableWidgetComponents_itemSelectionChanged() {
	// enable/disable buttons based on selection changed signal
	bool enabled = (currentlySelectedComponent() != nullptr);
	m_ui->pushButtonEditComponents->setEnabled(enabled);
	m_ui->pushButtonExchangeComponents->setEnabled(enabled);
	m_ui->pushButtonSelectObjectsWithComponent->setEnabled(enabled);
}


void SVPropBuildingEditWidget::on_pushButtonAssignComponent_clicked() {
	assignComponent(false);
}


void SVPropBuildingEditWidget::on_pushButtonAssignInsideComponent_clicked() {
	assignComponent(true);
}


void SVPropBuildingEditWidget::on_checkBoxShowAllComponentOrientations_toggled(bool checked) {
	m_ui->labelComponentSelection->setEnabled(!checked);
	m_ui->comboBoxComponentSelection->setEnabled(!checked);
	if (checked) {
		SVViewState vs = SVViewStateHandler::instance().viewState();
		vs.m_colorModePropertyID = 0; // disable filter
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
	if (m_ui->comboBoxComponentSelection->currentIndex() == -1)
		vs.m_colorModePropertyID = 0;
	else
		vs.m_colorModePropertyID = m_ui->comboBoxComponentSelection->currentData().toInt();
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


void SVPropBuildingEditWidget::updateUi() {
	const SVDatabase & db = SVSettings::instance().m_db;

	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true);
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



	// *** Update Component Page ***

	// now put the data of the map into the table
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
	// process all selected surfaces and determine which component they have assigned
	std::vector<const VICUS::Surface*> surfaces;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_Building);
	if (surfaces.empty()) {
		m_ui->labelSelectedComponents->setText("");
		m_ui->groupBoxSelectedComponent->setEnabled(false);
	}
	else {
		m_ui->groupBoxSelectedComponent->setEnabled(true);
	}
	m_ui->pushButtonAssignInsideComponent->setEnabled(surfaces.size() == 2);

	// update selection-related info
	std::set<const VICUS::Component *> selectedComponents;
	for (const VICUS::Surface* s : surfaces) {
		if (s->m_componentInstance != nullptr) {
			const VICUS::Component * surfcomp = db.m_components[s->m_componentInstance->m_componentID];
			selectedComponents.insert(surfcomp);
		}
	}
	if (selectedComponents.empty()) {
		m_ui->labelSelectedComponents->setText(tr("No components"));
	}
	else if (selectedComponents.size() == 1) {
		if (*selectedComponents.begin() == nullptr)
			m_ui->labelSelectedComponents->setText(tr("Component with invalid/unknown ID"));
		else
			m_ui->labelSelectedComponents->setText(tr("Component %1 [%2]")
			   .arg(QtExt::MultiLangString2QString((*selectedComponents.begin())->m_displayName)).arg((*selectedComponents.begin())->m_id));
	}
	else {
		m_ui->labelSelectedComponents->setText(tr("%1 different components")
		   .arg(selectedComponents.size()));
	}
	// update table related button states
	on_tableWidgetComponents_itemSelectionChanged();



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
	m_ui->tableWidgetBoundaryConditions->clearContents();
	m_ui->tableWidgetBoundaryConditions->setRowCount(m_bcSurfacesMap.size());
	row=0;
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
			item->setText(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetBoundaryConditions->setItem(row, 1, item);
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


void SVPropBuildingEditWidget::assignComponent(bool insideWall) {
	// ask user to select a new component
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignComponent",
		tr("Assign component"), tr("You may now select a component from the component database, which will then be"
								   "assigned to the selected surfaces."));
	int selectedComponentId = SVMainWindow::instance().dbComponentEditDialog()->select(0);
	if (selectedComponentId == -1)
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

