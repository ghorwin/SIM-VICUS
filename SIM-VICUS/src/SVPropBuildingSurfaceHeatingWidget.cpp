#include "SVPropBuildingSurfaceHeatingWidget.h"
#include "ui_SVPropBuildingSurfaceHeatingWidget.h"

#include <SV_Conversions.h>

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include "SVStyle.h"
#include "SVPropSurfaceHeatingDelegate.h"
#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVUndoModifyComponentInstances.h"
#include "SVUndoModifyProject.h"
#include "SVUndoModifyNetwork.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVDBSupplySystemEditWidget.h"
#include "SVZoneSelectionDialog.h"

SVPropBuildingSurfaceHeatingWidget::SVPropBuildingSurfaceHeatingWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingSurfaceHeatingWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

	m_ui->tableWidgetSurfaceHeating->setColumnCount(6);
	m_ui->tableWidgetSurfaceHeating->setHorizontalHeaderLabels(QStringList() << QString() << QString() << tr("Heating") << tr("Control zone") << tr("Surfaces, side A/B") << tr("Supply system"));
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
}


SVPropBuildingSurfaceHeatingWidget::~SVPropBuildingSurfaceHeatingWidget() {
	delete m_ui;
}


void SVPropBuildingSurfaceHeatingWidget::updateUi() {

	// get list of selected surfaces
	std::vector<const VICUS::Surface*> surfs;
	project().selectedSurfaces(surfs, VICUS::Project::SG_Building);

	// insert all referenced component instances into set
	m_selectedComponentInstances.clear();
	for (const VICUS::Surface* s : surfs)
		m_selectedComponentInstances.insert(s->m_componentInstance);

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
		if (comp->isValid(db.m_materials, db.m_constructions, db.m_boundaryConditions, db.m_schedules))
			item->setIcon(QIcon(":/gfx/actions/16x16/ok.png"));
		else
			item->setIcon(QIcon(":/gfx/actions/16x16/error.png"));
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

		// column 5 - associated supply network
		// look-up supply system
		const VICUS::SupplySystem * supplySys = db.m_supplySystems[ci.m_idSupplySystem];

		item = new QTableWidgetItem(surfaceNames);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

		if (surfHeat == nullptr || supplySys == nullptr) {
			item->setText("---");
			item->setData(Qt::UserRole, VICUS::INVALID_ID);
		}
		else {
			item->setText(QtExt::MultiLangString2QString(supplySys->m_displayName));
			item->setData(Qt::UserRole, ci.m_idSupplySystem);
		}
		m_ui->tableWidgetSurfaceHeating->setItem(row, 5, item);
	}
	m_ui->tableWidgetSurfaceHeating->blockSignals(false);
	m_ui->tableWidgetSurfaceHeating->selectionModel()->blockSignals(false);


	// enable/disable selection-based buttons
	on_tableWidgetSurfaceHeating_itemSelectionChanged();
	if (selectedCI.empty()) {
		m_ui->labelSelectedCIWithActiveLayer->setText(tr("None"));
		m_ui->pushButtonAssignSurfaceHeating->setEnabled(false);
		m_ui->pushButtonAssignSurfaceHeatingControlZone->setEnabled(false);
		m_ui->pushButtonRemoveSelectedSurfaceHeating->setEnabled(false);
	}
	else {
		m_ui->labelSelectedCIWithActiveLayer->setText(tr("%1 component instance(s)").arg(selectedCI.size()));
		m_ui->pushButtonAssignSurfaceHeating->setEnabled(true);

		// other buttons are only active if a configured surface heating component instances is selected
		m_ui->pushButtonAssignSurfaceHeatingControlZone->setEnabled(!selectedSurfaceHeatingCI.empty());

		m_ui->pushButtonRemoveSelectedSurfaceHeating->setEnabled(!selectedSurfaceHeatingCI.empty());
	}
}



void SVPropBuildingSurfaceHeatingWidget::on_comboBoxSurfaceHeatingComponentFilter_currentIndexChanged(int /*index*/) {
	updateUi();
}


void SVPropBuildingSurfaceHeatingWidget::on_tableWidgetSurfaceHeating_itemChanged(QTableWidgetItem *item) {
	if (item->column() == 2 || item->column() == 3 || item->column() == 5) {
		QTableWidgetItem * firstItem = m_ui->tableWidgetSurfaceHeating->item(item->row(), 0);
		unsigned int ciID = firstItem->data(Qt::UserRole).toUInt();
		std::vector<VICUS::ComponentInstance> cis = project().m_componentInstances;
		for (unsigned int i=0; i<cis.size(); ++i)
			if (cis[i].m_id == ciID) {
				if (item->column() == 2)
					cis[i].m_idSurfaceHeating = item->data(Qt::UserRole).toUInt();
				else if(item->column() == 3)
					cis[i].m_idSurfaceHeatingControlZone = item->data(Qt::UserRole).toUInt();
				else
					cis[i].m_idSupplySystem = item->data(Qt::UserRole).toUInt();
				break;
			}
		SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Assigned surface heating"), cis);
		undo->push();
	}
}


void SVPropBuildingSurfaceHeatingWidget::on_pushButtonRemoveSurfaceHeating_clicked() {
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


void SVPropBuildingSurfaceHeatingWidget::on_pushButtonAssignSurfaceHeating_clicked() {
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

void SVPropBuildingSurfaceHeatingWidget::on_tableWidgetSurfaceHeating_currentCellChanged(int /*currentRow*/, int /*currentColumn*/, int previousRow, int previousColumn) {
	QTableWidgetItem * item = m_ui->tableWidgetSurfaceHeating->item(previousRow, previousColumn);
	if (item != nullptr)
		m_ui->tableWidgetSurfaceHeating->closePersistentEditor(item);
}


void SVPropBuildingSurfaceHeatingWidget::on_tableWidgetSurfaceHeating_itemSelectionChanged() {
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


void SVPropBuildingSurfaceHeatingWidget::on_pushButtonRemoveSelectedSurfaceHeating_clicked() {
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
		ci.m_idSupplySystem = VICUS::INVALID_ID;
	}

	SVUndoModifyComponentInstances * undo = new SVUndoModifyComponentInstances(tr("Removed surface heatings"), cis);
	undo->push();
}


void SVPropBuildingSurfaceHeatingWidget::on_pushButtonAssignSurfaceHeatingControlZone_clicked() {
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


