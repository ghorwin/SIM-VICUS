#include "SVPropSupplySystemsWidget.h"

#include <SVConversions.h>

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "SVDatabase.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVUndoModifyComponentInstances.h"
#include "SVUndoTreeNodeState.h"

SVPropSupplySystemsWidget::SVPropSupplySystemsWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropSupplySystemsWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->tableWidgetSupplySystems->setColumnCount(2);
	m_ui->tableWidgetSupplySystems->setHorizontalHeaderLabels(QStringList() << QString() << tr("Supply systems"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSupplySystems);
	m_ui->tableWidgetSupplySystems->setSortingEnabled(false);
	m_ui->tableWidgetSupplySystems->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetSupplySystems->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetSupplySystems->horizontalHeader()->setStretchLastSection(true);
}


SVPropSupplySystemsWidget::~SVPropSupplySystemsWidget() {
	delete m_ui;
}


void SVPropSupplySystemsWidget::updateUi() {
	// get list of selected surfaces
	std::vector<const VICUS::Surface*> surfs;
	project().selectedSurfaces(surfs, VICUS::Project::SG_Building);

	// insert all referenced component instances into set
	m_selectedComponentInstances.clear();
	for (const VICUS::Surface* s : surfs)
		m_selectedComponentInstances.insert(s->m_componentInstance);

	// get all visible "building" type objects in the scene
	std::set<const VICUS::Object * > objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, false, true); // visible (selected and not selected)

	const SVDatabase & db = SVSettings::instance().m_db;

	// now build a map of referenced boundary conditions versus visible surfaces
	m_supplySysSurfacesMap.clear();
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {

		// we don't consider cis that have no supply system assigned
		if (ci.m_idSupplySystem == VICUS::INVALID_ID)
			continue;

		// lookup supply system in DB
		const VICUS::SupplySystem * supplySystem = db.m_supplySystems[ci.m_idSupplySystem];

		// supplySystem can still be a null pointer in case there is no supply system with given id ...

		// side A
		if (ci.m_sideASurface != nullptr) {
			// is this surface visible?
			std::set<const VICUS::Object * >::const_iterator it_A = objs.find(ci.m_sideASurface);
			if (it_A != objs.end()) {
				// we allow null pointer
				m_supplySysSurfacesMap[supplySystem].insert(ci.m_sideASurface);
			}
		}
		// side B
		if (ci.m_sideBSurface != nullptr) {
			std::set<const VICUS::Object * >::const_iterator it_B = objs.find(ci.m_sideBSurface);
			if (it_B != objs.end())
				// we allow null pointer
				m_supplySysSurfacesMap[supplySystem].insert(ci.m_sideBSurface);
		}
	}

	// now put the data of the map into the table
	int currentRow = m_ui->tableWidgetSupplySystems->currentRow();
	m_ui->tableWidgetSupplySystems->blockSignals(true);
	m_ui->tableWidgetSupplySystems->clearContents();
	m_ui->tableWidgetSupplySystems->setRowCount(m_supplySysSurfacesMap.size());
	int row=0;
	for (std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator
		 it = m_supplySysSurfacesMap.begin(); it != m_supplySysSurfacesMap.end(); ++it, ++row) {

		QTableWidgetItem * item = new QTableWidgetItem();
		// special handling for surfaces with invalid supply system assigned
		if (it->first == nullptr) {
			item->setBackground(QColor(64,64,64)); // gray = invalid
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetSupplySystems->setItem(row, 0, item);

			item = new QTableWidgetItem();
			item->setText(tr("Invalid/missing supply system"));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetSupplySystems->setItem(row, 1, item);
		}
		else {
			item->setBackground(it->first->m_color);
			item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
			m_ui->tableWidgetSupplySystems->setItem(row, 0, item);

			item = new QTableWidgetItem();
			item->setText(QtExt::MultiLangString2QString(it->first->m_displayName) );
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetSupplySystems->setItem(row, 1, item);
		}
	}
	// reselect row
	m_ui->tableWidgetSupplySystems->blockSignals(false);
	m_ui->tableWidgetSupplySystems->selectRow(std::min(currentRow, m_ui->tableWidgetSupplySystems->rowCount()-1));

	// we will check vakidity of defiting and selecting here
	on_tableWidgetSupplySystems_itemSelectionChanged();
}


void SVPropSupplySystemsWidget::on_tableWidgetSupplySystems_itemSelectionChanged() {

	bool editingEnabled = false;
	bool selectingEnabled = m_ui->tableWidgetSupplySystems->currentRow() != -1;

	if(selectingEnabled) {
		int currentRow = m_ui->tableWidgetSupplySystems->currentRow();
		std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator it = m_supplySysSurfacesMap.begin();
		std::advance(it, currentRow);
		const VICUS::SupplySystem* supplySys = it->first;
		editingEnabled = supplySys != nullptr;
	}

	// we need a valid surface choice
	bool assigningEnabled = false;
	// we need a valid surface choice with minimum one supply system assigend
	bool removingEnabled = false;
	// only activate if valid components are chosen
	std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;

	// process all selected components
	for (VICUS::ComponentInstance & ci : cis) {
		// check if current ci is in list of selected component instances
		std::set<const VICUS::ComponentInstance*>::const_iterator ciIt = m_selectedComponentInstances.begin();

		for (; ciIt != m_selectedComponentInstances.end(); ++ciIt) {
			// component instance may be nullptr
			if(*ciIt == nullptr)
				continue;

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
		// check if no surface heating is assigned
		if(ci.m_idSurfaceHeating == VICUS::INVALID_ID)
			continue;

		assigningEnabled = true;

		// we only can remove an existing supply system
		if (ci.m_idSupplySystem != VICUS::INVALID_ID) {
			removingEnabled = true;
			break;
		}
	}

	m_ui->pushButtonEditSupplySystem->setEnabled(editingEnabled);
	m_ui->pushButtonExchangeSupplySystem->setEnabled(editingEnabled);
	m_ui->pushButtonSelectSupplySystem->setEnabled(selectingEnabled);
	m_ui->pushButtonAssignSupplySystem->setEnabled(assigningEnabled);
	m_ui->pushButtonRemoveSupplySystem->setEnabled(removingEnabled);
}


void SVPropSupplySystemsWidget::on_pushButtonEditSupplySystem_clicked() {
	openEditSupplySystemsDialog();
}

void SVPropSupplySystemsWidget::on_tableWidgetSupplySystems_cellDoubleClicked(int /*row*/, int /*column*/) {
	openEditSupplySystemsDialog();

}

void SVPropSupplySystemsWidget::openEditSupplySystemsDialog() {
	int currentRow = m_ui->tableWidgetSupplySystems->currentRow();
	std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator it = m_supplySysSurfacesMap.begin();
	std::advance(it, currentRow);
	const VICUS::SupplySystem* supplySys = it->first;

	// edit nutton is only activated for valid supply system
	Q_ASSERT(supplySys != nullptr);

	// start DB editor for selected boundary condition
	SVMainWindow::instance().dbSupplySystemEditDialog()->edit(supplySys->m_id);
	// Note: project data isn't modified, since only user DB data was changed.
	// reselect row, because closing the edit dialog recreates the table (due to possible color change)
	if (m_ui->tableWidgetSupplySystems->rowCount() > currentRow)
		m_ui->tableWidgetSupplySystems->selectRow(currentRow);
}


void SVPropSupplySystemsWidget::on_pushButtonExchangeSupplySystem_clicked()
{
	int r = m_ui->tableWidgetSupplySystems->currentRow();
	Q_ASSERT(r != -1);
	std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator sit = m_supplySysSurfacesMap.begin();
	std::advance(sit, r);
	// exchange button is only activated for valid supply system
	Q_ASSERT(sit->first != nullptr);
	unsigned int currentID = sit->first->m_id;

	unsigned int selectedID = SVMainWindow::instance().dbSupplySystemEditDialog()->select(VICUS::INVALID_ID);
	if (selectedID == VICUS::INVALID_ID)
		return;
	// nothing changed
	if (selectedID == currentID)
		return;

	// if component instance does not have an active layer assigned, skip
	VICUS::SupplySystem * supplySys = SVSettings::instance().m_db.m_supplySystems[selectedID];
	if (supplySys == nullptr)
		return;

	// select component instances

	std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;

	// process all selected components
	for (VICUS::ComponentInstance & ci : cis) {
		// if component instance does not have an active layer assigned, skip
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci.m_idComponent];
		if (comp == nullptr)
			continue;
		// check if no active layer is present
		if (comp->m_activeLayerIndex == VICUS::INVALID_ID)
			continue;
		// check if no surface heating is assigned
		if(ci.m_idSurfaceHeating == VICUS::INVALID_ID)
			continue;
		// filter component instance with old id
		if(ci.m_idSupplySystem != currentID)
			continue;

		ci.m_idSupplySystem = selectedID;
	}

	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Assigned supply systems"), cis);
	undo->push();
}

void SVPropSupplySystemsWidget::on_pushButtonSelectSupplySystem_clicked() {
	int r = m_ui->tableWidgetSupplySystems->currentRow();
	Q_ASSERT(r != -1);
	std::map<const VICUS::SupplySystem*, std::set<const VICUS::Surface *> >::const_iterator sit = m_supplySysSurfacesMap.begin();
	std::advance(sit, r);
	const VICUS::SupplySystem * supplySys = sit->first;
	Q_ASSERT(m_supplySysSurfacesMap.find(supplySys) != m_supplySysSurfacesMap.end());
	// select button is also activated for missing supply system

	std::set<unsigned int> surfaceObjs;
	for (const VICUS::Surface * s : sit->second)
		surfaceObjs.insert(s->m_id);

	QString undoText;
	if (supplySys != nullptr)
		undoText = tr("Select objects with supply system '%1'").arg(QtExt::MultiLangString2QString(supplySys->m_displayName));
	else
		undoText = tr("Select objects with invalid supply system");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, surfaceObjs, true);
	undo->push();
}


void SVPropSupplySystemsWidget::on_pushButtonAssignSupplySystem_clicked() {
	// popup supply system DB dialog and if user selects one, assign it to all selected component instances
	unsigned int selectedID = SVMainWindow::instance().dbSupplySystemEditDialog()->select(VICUS::INVALID_ID);
	if (selectedID == VICUS::INVALID_ID)
		return;

	// if component instance does not have an active layer assigned, skip
	VICUS::SupplySystem * supplySys = SVSettings::instance().m_db.m_supplySystems[selectedID];
	if (supplySys == nullptr)
		return;

	std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;

	// process all selected components
	for (VICUS::ComponentInstance & ci : cis) {
		// check if current ci is in list of selected component instances
		std::set<const VICUS::ComponentInstance*>::const_iterator ciIt = m_selectedComponentInstances.begin();
		for (; ciIt != m_selectedComponentInstances.end(); ++ciIt) {
			// component instance may be nullptr
			if(*ciIt == nullptr)
				continue;
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
		// check if no surface heating is assigned
		if(ci.m_idSurfaceHeating == VICUS::INVALID_ID)
			continue;

		ci.m_idSupplySystem = selectedID;
	}

	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Assigned supply systems"), cis);
	undo->push();
}


void SVPropSupplySystemsWidget::on_pushButtonRemoveSupplySystem_clicked() {
	// popup supply system DB dialog and if user selects one, assign it to all selected component instances
	std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;

	// process all selected components
	for (VICUS::ComponentInstance & ci : cis) {
		// check if current ci is in list of selected component instances
		std::set<const VICUS::ComponentInstance*>::const_iterator ciIt = m_selectedComponentInstances.begin();
		for (; ciIt != m_selectedComponentInstances.end(); ++ciIt) {
			// component instance may be nullptr
			if(*ciIt == nullptr)
				continue;
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
		// check if no surface heating is assigned
		if (ci.m_idSurfaceHeating == VICUS::INVALID_ID)
			continue;

		ci.m_idSupplySystem = VICUS::INVALID_ID;
	}

	// TODO : Feedback to user, if one/none of the selected components could have the supply system removed. Otherwise
	//        the user clicks on the button and nothing happens....

	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Removed supply systems"), cis);
	undo->push();
}



