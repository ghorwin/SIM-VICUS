#include "SVPropBuildingEditWidget.h"
#include "ui_SVPropBuildingEditWidget.h"

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
	m_propertyType = buildingPropertyType;
	m_ui->stackedWidget->setCurrentIndex(0);
	const SVDatabase & db = SVSettings::instance().m_db;

	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true);
	// now build a map of component IDs versus visible surfaces
	m_componentSurfacesMap.clear();
	unsigned int selectedSurfacesWithComponent = 0;
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
					++selectedSurfacesWithComponent;
			}
		}
		// side B
		if (ci.m_sideBSurface != nullptr) {
			std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
			if (it_B != objs.end()) {
				m_componentSurfacesMap[comp].push_back(ci.m_sideBSurface);
				if (ci.m_sideBSurface->m_selected)
					++selectedSurfacesWithComponent;
			}
		}
	}

	switch ((BuildingPropertyTypes)buildingPropertyType) {
		case BT_Components: {
			m_ui->stackedWidget->setCurrentIndex(1);
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
			// update button states
			on_tableWidgetComponents_itemSelectionChanged();
		} break;

		case BT_ComponentOrientation:
			m_ui->stackedWidget->setCurrentIndex(2);
			m_ui->comboBoxComponentSelection->blockSignals(true);
			m_ui->comboBoxComponentSelection->clear();
			for (std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >::const_iterator
				 it = m_componentSurfacesMap.begin(); it != m_componentSurfacesMap.end(); ++it)
			{
				if (it->first == nullptr)
					return;
				m_ui->comboBoxComponentSelection->addItem(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")), it->first->m_id);
			}
			m_ui->comboBoxComponentSelection->setCurrentIndex(m_ui->comboBoxComponentSelection->count()-1);
			m_ui->comboBoxComponentSelection->blockSignals(false);
			if (selectedSurfacesWithComponent == 0) {
				m_ui->labelComponentOrientationInfo->setText(tr("No surfaces with components selected"));
				m_ui->pushButtonAlignComponentToSideA->setEnabled(false);
				m_ui->pushButtonAlignComponentToSideB->setEnabled(false);
			}
			else {
				m_ui->labelComponentOrientationInfo->setText(tr("%1 surfaces with components selected").arg(selectedSurfacesWithComponent));
				m_ui->pushButtonAlignComponentToSideA->setEnabled(true);
				m_ui->pushButtonAlignComponentToSideB->setEnabled(true);
			}
		break;

		case BT_BoundaryConditions: {
			m_ui->stackedWidget->setCurrentIndex(3);
			// get all visible "building" type objects in the scene
			std::set<const VICUS::Object * > objs;
			project().selectObjects(objs, VICUS::Project::SG_Building, false, true);
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
					item->setText(QString::fromStdString(it->first->m_displayName.string(IBK::MultiLanguageString::m_language, "en")));
				item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				m_ui->tableWidgetBoundaryConditions->setItem(row, 1, item);
			}
		} break;
	}
	// finally update everything related to selection
	objectSelectionChanged();
}


void SVPropBuildingEditWidget::onModified(int modificationType, ModificationInfo * data) {
	// react on selection changes only, then update properties
	switch ((SVProjectHandler::ModificationTypes)modificationType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::ComponentInstancesModified:
		case SVProjectHandler::NodeStateModified:
			// the selection has changed, update widgets related to selection
			setPropertyType(m_propertyType);
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
	int newId = SVMainWindow::instance().dbComponentEditDialog()->select(oldId);
	if (newId == -1)
		return; // user has aborted the dialog
	// TODO : signal a "recoloring needed" signal to scene in case any of the colors have changed
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


const VICUS::Component * SVPropBuildingEditWidget::currentlySelectedComponent() const {
	// check if selected "component" is actually a missing component
	int r = m_ui->tableWidgetComponents->currentRow();
	if (r == -1 || m_componentSurfacesMap.empty())
		return nullptr;
	std::map<const VICUS::Component*, std::vector<const VICUS::Surface *> >::const_iterator cit = m_componentSurfacesMap.begin();
	std::advance(cit, r);
	return cit->first;
}


void SVPropBuildingEditWidget::objectSelectionChanged() {
	switch ((BuildingPropertyTypes)m_propertyType) {
		case BT_Components: {
			// process all selected surfaces and determine which component they have assigned
			std::vector<const VICUS::Surface*> surfaces;
			project().selectedSurfaces(surfaces, VICUS::Project::SG_Building);
			if (surfaces.empty()) {
				m_ui->labelSelectedComponents->setText("");
				m_ui->groupBoxSelectedComponent->setEnabled(false);
				return;
			}
			m_ui->groupBoxSelectedComponent->setEnabled(true);
			std::set<const VICUS::Component *> selectedComponents;
			const SVDatabase & db = SVSettings::instance().m_db;
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
				m_ui->labelSelectedComponents->setText(tr("Component %1 [%2]")
				   .arg(QtExt::MultiLangString2QString((*selectedComponents.begin())->m_displayName)).arg((*selectedComponents.begin())->m_id));
			}
			else {
				m_ui->labelSelectedComponents->setText(tr("%1 different components")
				   .arg(selectedComponents.size()));
			}
		} break;

		case BT_ComponentOrientation:
			// TODO : Andreas
		break;

		case BT_BoundaryConditions:
			// TODO : Andreas
		break;
	}

}


void SVPropBuildingEditWidget::on_pushButtonAssignComponent_clicked() {
	// ask user to select a new component
	SVSettings::instance().showDoNotShowAgainMessage(this, "PropertyWidgetInfoAssignComponent",
		tr("Assign component"), tr("You may now select a component from the component database, which will then be"
								   "assigned to all selected surfaces."));
	int newId = SVMainWindow::instance().dbComponentEditDialog()->select(0);
	if (newId == -1)
		return; // user aborted the dialog

	std::vector<const VICUS::Surface*> surfaces;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_Building);
	Q_ASSERT(!surfaces.empty());
	std::set<const VICUS::Surface*> selSurfaces(surfaces.begin(), surfaces.end());

	// now create a copy of the currently defined componentInstances
	std::vector<VICUS::ComponentInstance> compInstances = project().m_componentInstances;
	// process all componentInstances and for all that reference any of the selected surfaces, replace component
	for (VICUS::ComponentInstance & ci : compInstances) {
		std::set<const VICUS::Surface*>::iterator it = selSurfaces.find(ci.m_sideASurface);
		if (it != selSurfaces.end()) {
			ci.m_componentID = (unsigned int)newId;
			selSurfaces.erase(it);
			continue;
		}
		it = selSurfaces.find(ci.m_sideBSurface);
		if (it != selSurfaces.end()) {
			ci.m_componentID = (unsigned int)newId;
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
			c.m_componentID = (unsigned int)newId;
			compInstances.push_back(c);
		}
	}

	// create the undo action and modify project
	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Components assigned"), compInstances);
	undo->push();

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
	// create a copy of the component instances
	std::vector<VICUS::ComponentInstance> compInstances = project().m_componentInstances;

	// loop over all components and look for a selected side - if there is more than one side of a component
	// instance selected, show an error message



}


void SVPropBuildingEditWidget::on_pushButtonAlignComponentToSideB_clicked() {

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
