#include "SVPropBuildingZoneProperty.h"
#include "ui_SVPropBuildingZoneProperty.h"

#include <SV_Conversions.h>

#include "SVStyle.h"
#include "SVPropZonePropertyDelegate.h"
#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVUndoModifyProject.h"
#include "SVUndoModifyBuildingLevel.h"
#include "SVUndoModifyRoom.h"
#include "SVUndoModifyBuildingTopology.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVZoneSelectionDialog.h"


SVPropBuildingZoneProperty::SVPropBuildingZoneProperty(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropBuildingZoneProperty)
{

	m_ui->setupUi(this);
	//m_ui->groupBox_7->setMargin(0);

	m_ui->tableWidgetZones->setColumnCount(4);
	m_ui->tableWidgetZones->setHorizontalHeaderLabels(QStringList() << tr("Id") << tr("Name") << tr("Area [m2]") << tr("Volume [m3]"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetZones);
	m_ui->tableWidgetZones->sortByColumn(1, Qt::SortOrder::AscendingOrder);
	m_ui->tableWidgetZones->setSortingEnabled(true);
	m_ui->tableWidgetZones->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetZones->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidgetZones->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
	m_ui->tableWidgetZones->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
	m_ui->tableWidgetZones->horizontalHeader()->resizeSection(0,100);
	m_ui->tableWidgetZones->horizontalHeader()->resizeSection(2,100);
	m_ui->tableWidgetZones->horizontalHeader()->resizeSection(3,100);
	m_ui->tableWidgetZones->horizontalHeader()->setStretchLastSection(false);
	m_ui->tableWidgetZones->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ui->tableWidgetZones->setSelectionBehavior(QAbstractItemView::SelectRows);

	// Mind: parent of the item delegate must be its widget!
	m_ui->tableWidgetZones->setItemDelegate(new SVPropZonePropertyDelegate(m_ui->tableWidgetZones));

	m_ui->pushButtonAddSurface->setEnabled(false);
}


SVPropBuildingZoneProperty::~SVPropBuildingZoneProperty() {
	delete m_ui;
}


void SVPropBuildingZoneProperty::updateUi() {


	// first update building and building level

	m_ui->comboBoxBuildingFilter->blockSignals(true);
	m_ui->comboBoxBuildingLevelFilter->blockSignals(true);

	unsigned int currentBuilding = m_ui->comboBoxBuildingFilter->currentData().toUInt();	// unique ID of currently selected building
	unsigned int currentBuildingLevel = m_ui->comboBoxBuildingLevelFilter->currentData().toUInt();	// unique ID of currently selected building
	int selectedBuildingIndex = -1 ;
	int selectedBuildingLevelIndex = -1 ;
	unsigned int currentBuildingVectorIdx = VICUS::INVALID_ID;
	unsigned int currentBuildingLevelVectorIdx = VICUS::INVALID_ID;

	m_ui->comboBoxBuildingFilter->clear();
	m_ui->comboBoxBuildingLevelFilter->clear();

	// set all building in combo box building
	m_ui->comboBoxBuildingFilter->addItem(tr("All buildings"), VICUS::INVALID_ID);
	for(unsigned int i=0; i<project().m_buildings.size(); ++i){
		const VICUS::Building & b = project().m_buildings[i];
		if(currentBuilding == b.m_id){
			selectedBuildingIndex = m_ui->comboBoxBuildingFilter->count();
			currentBuildingVectorIdx = i;
		}
		m_ui->comboBoxBuildingFilter->addItem(b.m_displayName, b.m_id);
	}

	if (selectedBuildingIndex == -1)
		m_ui->comboBoxBuildingFilter->setCurrentIndex(0);
	else
		m_ui->comboBoxBuildingFilter->setCurrentIndex(selectedBuildingIndex);

	m_ui->comboBoxBuildingLevelFilter->addItem(tr("All building levels"), VICUS::INVALID_ID);
	if(selectedBuildingIndex == -1){
		for(const VICUS::Building & b : project().m_buildings){
			for(const VICUS::BuildingLevel & bl : b.m_buildingLevels){
				if(currentBuildingLevel == bl.m_id)
					selectedBuildingLevelIndex = m_ui->comboBoxBuildingLevelFilter->count();
				m_ui->comboBoxBuildingLevelFilter->addItem(bl.m_displayName, bl.m_id);
			}
		}
	}
	else{
		for(unsigned int i=0; i<project().m_buildings[currentBuildingVectorIdx].m_buildingLevels.size(); ++i){
			const VICUS::BuildingLevel & bl = project().m_buildings[currentBuildingVectorIdx].m_buildingLevels[i];
			if(currentBuildingLevel == bl.m_id){
				selectedBuildingLevelIndex = m_ui->comboBoxBuildingLevelFilter->count();
				currentBuildingLevelVectorIdx = i;
			}
			m_ui->comboBoxBuildingLevelFilter->addItem(bl.m_displayName, bl.m_id);
		}
	}

	if(selectedBuildingLevelIndex == -1)
		m_ui->comboBoxBuildingLevelFilter->setCurrentIndex(0);
	else
		m_ui->comboBoxBuildingLevelFilter->setCurrentIndex(selectedBuildingLevelIndex);

	m_ui->comboBoxBuildingLevelFilter->blockSignals(false);
	m_ui->comboBoxBuildingFilter->blockSignals(false);


	// get list of all rooms by filter selection
	std::vector<const VICUS::Room*> rooms;

	if(selectedBuildingIndex == -1){
		for( const VICUS::Building &b : project().m_buildings){
			// put all rooms in
			for( const VICUS::BuildingLevel &bl : b.m_buildingLevels)
				for( const VICUS::Room &r : bl.m_rooms)
					rooms.push_back(&r);
		}

		m_ui->comboBoxBuildingLevelFilter->blockSignals(true);
		m_ui->comboBoxBuildingLevelFilter->setCurrentIndex(0);
		m_ui->comboBoxBuildingLevelFilter->blockSignals(false);
		m_ui->comboBoxBuildingLevelFilter->setEnabled(false);
	}
	else{
		if(selectedBuildingLevelIndex == -1){
			// put all rooms of this building in
			for( const VICUS::BuildingLevel &bl : project().m_buildings[currentBuildingVectorIdx].m_buildingLevels)
				for( const VICUS::Room &r : bl.m_rooms)
					rooms.push_back(&r);
		}
		else{
			// put only rooms of this building level in
			const VICUS::BuildingLevel &bl = project().m_buildings[currentBuildingVectorIdx].
					m_buildingLevels[currentBuildingLevelVectorIdx];
			for( const VICUS::Room &r : bl.m_rooms)
				rooms.push_back(&r);

		}
		m_ui->comboBoxBuildingLevelFilter->setEnabled(true);
	}

	// process all rooms
	// and fill table
	m_ui->tableWidgetZones->blockSignals(true);
	m_ui->tableWidgetZones->selectionModel()->blockSignals(true);
	m_ui->tableWidgetZones->setSortingEnabled(false);
	m_ui->tableWidgetZones->setRowCount(0);

	for(const VICUS::Room * r : rooms){
		// add new row
		int row = m_ui->tableWidgetZones->rowCount();
		m_ui->tableWidgetZones->setRowCount(row+1);

		// fill table

		// column 0 - room id
		QTableWidgetItem * item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if(r == nullptr){
			// skip
			/// ToDo Heiko ist das richtig?
			continue;
		}
		else{
			item->setText(QString::number(r->m_id));
			item->setData(Qt::UserRole,r->m_id);
		}
		m_ui->tableWidgetZones->setItem(row,0, item);

		// column 1 - room name
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setText(r->m_displayName);
		item->setData(Qt::UserRole,r->m_id);
		m_ui->tableWidgetZones->setItem(row,1, item);

		// column 2 - room floor area
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		//get parameter
		bool isValid = !r->m_para[VICUS::Room::P_Area].empty();
		double floorarea = 0;
		if(isValid){
			floorarea = r->m_para[VICUS::Room::P_Area].get_value("m2");
			item->setText(QString::number(floorarea,'f', 2));
		}
		else
			item->setText("---");

		item->setData(Qt::UserRole,r->m_id);
		item->setTextAlignment(Qt::AlignRight);
		m_ui->tableWidgetZones->setItem(row,2, item);

		// column 3 - room volume
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		//get parameter
		isValid = !r->m_para[VICUS::Room::P_Volume].empty();
		double volume = 0;
		if(isValid){
			volume = r->m_para[VICUS::Room::P_Volume].get_value("m3");
			item->setText(QString::number(volume,'f', 2));
		}
		else
			item->setText("---");

		item->setData(Qt::UserRole,r->m_id);
		item->setTextAlignment(Qt::AlignRight);
		m_ui->tableWidgetZones->setItem(row,3, item);
	}

	m_ui->tableWidgetZones->blockSignals(false);
	m_ui->tableWidgetZones->selectionModel()->blockSignals(false);
	m_ui->tableWidgetZones->setSortingEnabled(true);

	//	for (const VICUS::Room* r : rooms)
	//		m_selectedRooms.insert(r);

	// populate table with all components that are currently selected by filter
	// we only show assigned components with active layers

	const SVDatabase & db = SVSettings::instance().m_db;




	// enable/disable selection-based buttons
	//on_tableWidgetSurfaceHeating_itemSelectionChanged();

	// insert all referenced rooms into set
	std::vector<unsigned int> selectedRoomIds;
	bool roomsSelected = !selectedRoomIds.empty();
	for(auto *item : m_ui->tableWidgetZones->selectedItems()){
		if(item->column() != 0)
			continue;
		selectedRoomIds.push_back(item->text().toUInt());
	}
	/// ToDo Heiko->Dirk: Wie geht die Aktivierung richtig siehe Frage ganz unten.
	//m_ui->pushButtonFloorAreaSelectedRooms->setEnabled(roomsSelected);
	//m_ui->pushButtonVolumeSelectedRooms->setEnabled(roomsSelected);

}

#if 0

void SVPropBuildingZoneProperty::on_tableWidgetSurfaceHeating_itemChanged(QTableWidgetItem *item) {
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


void SVPropBuildingZoneProperty::on_pushButtonRemoveSurfaceHeating_clicked() {
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
			// find matching component instance | Qt::ItemIsEditable
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


void SVPropBuildingZoneProperty::on_pushButtonAssignSurfaceHeating_clicked() {
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

void SVPropBuildingZoneProperty::on_tableWidgetSurfaceHeating_currentCellChanged(int /*currentRow*/, int /*currentColumn*/, int previousRow, int previousColumn) {
	QTableWidgetItem * item = m_ui->tableWidgetSurfaceHeating->item(previousRow, previousColumn);
	if (item != nullptr)
		m_ui->tableWidgetSurfaceHeating->closePersistentEditor(item);
}


void SVPropBuildingZoneProperty::on_tableWidgetSurfaceHeating_itemSelectionChanged() {
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


void SVPropBuildingZoneProperty::on_pushButtonRemoveSelectedSurfaceHeating_clicked() {
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


void SVPropBuildingZoneProperty::on_pushButtonAssignSurfaceHeatingControlZone_clicked() {
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

#endif
void SVPropBuildingZoneProperty::on_comboBoxBuildingFilter_currentIndexChanged(int /*index*/){
	updateUi();
}



void SVPropBuildingZoneProperty::on_comboBoxBuildingLevelFilter_currentIndexChanged(int /*index*/) {
	updateUi();
}

void SVPropBuildingZoneProperty::on_tableWidgetZones_itemDoubleClicked(QTableWidgetItem *item) {
	if (item != nullptr)
		m_ui->tableWidgetZones->closePersistentEditor(item);
}

void SVPropBuildingZoneProperty::on_tableWidgetZones_cellChanged(int row, int column){


	if(column<2)
		return;

	QTableWidgetItem *current = m_ui->tableWidgetZones->item(row, column);
	// change floor area
	unsigned int roomId = m_ui->tableWidgetZones->item(row, 0)->data(Qt::UserRole).toUInt();

	//get value
	bool valueOk;
	double val = current->text().toDouble(&valueOk);

	if(!valueOk){
		QMessageBox::warning(this, QString(), tr("Only numbers are allowed. Input is not a number."));
		updateUi();
		return;
	}

	if(val<=0){
		QMessageBox::warning(this, QString(), tr("Value must greater zero. Value is smaller or equal zero."));
		updateUi();
		return;
	}

	std::vector<VICUS::Building> buildings = project().m_buildings;

	// first find the room
	bool foundRoom = false;
	for(unsigned int i=0; i<buildings.size(); ++i){
		const VICUS::Building & b = buildings[i];
		for(unsigned j=0; j<b.m_buildingLevels.size(); ++j){
			VICUS::BuildingLevel bl = b.m_buildingLevels[j];
			for(unsigned k=0; k<bl.m_rooms.size(); ++k){
				VICUS::Room & r = bl.m_rooms[k];
				if(r.m_id == roomId){
					QString text;

					if(current->column() == 2){
						VICUS::KeywordList::setParameter(r.m_para, "Room::para_t", VICUS::Room::P_Area, val);
						text = tr("Modified floor area");
					}
					// change volume
					else if(current->column() == 3){
						VICUS::KeywordList::setParameter(r.m_para, "Room::para_t", VICUS::Room::P_Volume, val);
						text = tr("Modified volume");
					}
					SVUndoModifyRoom * undo = new SVUndoModifyRoom(text, r, i, j, k);
					undo->push();

					foundRoom = true;
					break;
				}
			}
			if(foundRoom)
				break;
		}
		if(foundRoom)
			break;
	}
}

void SVPropBuildingZoneProperty::calculatedParameters(bool floorAreaCalc, bool onlySelected){

	std::vector<unsigned int>	roomIds;
	if(onlySelected){

		for(auto *item : m_ui->tableWidgetZones->selectedItems()){
			if(item->column() != 0)
				continue;
			roomIds.push_back(item->text().toUInt());
		}

		if(roomIds.empty())
			return;
	}
	else{
		for(int row=0; row<m_ui->tableWidgetZones->rowCount(); ++row)
			roomIds.push_back(m_ui->tableWidgetZones->item(row,0)->text().toUInt());

		if(roomIds.empty())
			return;
	}

	// make a copy of buildings data structure
	std::vector<VICUS::Building>	buildings = project().m_buildings;

	for(unsigned int idxBuilding=0; idxBuilding<buildings.size(); ++idxBuilding){
		for(unsigned int idxBuildingLevel=0; idxBuildingLevel<buildings[idxBuilding].m_buildingLevels.size(); ++idxBuildingLevel){
			for(unsigned int idxRoom=0; idxRoom<buildings[idxBuilding].m_buildingLevels[idxBuildingLevel].m_rooms.size(); ++idxRoom){
				VICUS::Room &r = buildings[idxBuilding].m_buildingLevels[idxBuildingLevel].m_rooms[idxRoom];
				for(unsigned int id : roomIds){
					if(id == r.m_id){
						// make calculation here
						if(floorAreaCalc)
							r.calculateFloorArea();
						else
							r.calculateVolume();
						break;
					}
				}
			}
		}
	}
	QString text = "Floor area calculation";
	if(!floorAreaCalc)
		text = "Volume calculation";

	SVUndoModifyBuildingTopology *undo = new SVUndoModifyBuildingTopology(text, buildings);
	undo->push();
	updateUi();
}


//void SVPropBuildingZoneProperty::on_tableWidgetZones_itemSelectionChanged() {
//	updateUi();
//}


//void SVPropBuildingZoneProperty::on_tableWidgetZones_cellPressed(int row, int column)
//{
//	updateUi();
//}

/// ToDo Heiko->Dirk: wie kann ich abfangen wenn einer eine zelle bzw. mehrere markiert. dann muss der Button freigeschaltet werden
/// derzeit passiert das zwar mit cellPressed oder itemSelectionChanged aber danach ist dann nicht mehr die Zeile markiert.
/// weitere sind auch nicht mehr zu markieren
/// was mach ich falsch?


void SVPropBuildingZoneProperty::on_pushButtonFloorArea_clicked() {
	calculatedParameters(true, m_ui->radioButtonSelected->isChecked());
}

void SVPropBuildingZoneProperty::on_pushButtonVolume_clicked() {
	calculatedParameters(false, m_ui->radioButtonSelected->isChecked());
}

void SVPropBuildingZoneProperty::on_pushButtonAddSurface_clicked() {

	VICUS::Project vp = project();
	vp.updatePointers();

	VICUS::Room *selectedRoom = vp.roomByID(m_selectedRoomID);
	Q_ASSERT(selectedRoom != nullptr);

	// we get the selected surfaces
	std::vector<const VICUS::Surface*> surfs;
	vp.selectedSurfaces(surfs, VICUS::Project::SG_Building);
	QString newRoomName, messageBoxText, oldRoomName;

	newRoomName = selectedRoom ->m_displayName;
	bool hadAtLeastOneChangedSurface = false;

	std::map<unsigned int, std::vector<unsigned int>> idTodeleteSurfPositions;

	// we have to mind plain geometry and already assigned surfaces
	for(const VICUS::Surface *surf : surfs) {

		// if surface has already a parent we have to delete the surface from the parent
		// and update its parent
		VICUS::Room* oldRoom = dynamic_cast<VICUS::Room*>(surf->m_parent);
		if(oldRoom->m_id == selectedRoom->m_id)
			continue; // skip if surface is already assigned to specified room

		selectedRoom->m_surfaces.push_back(*surf);
		selectedRoom->updateParents();
		hadAtLeastOneChangedSurface = true;

		if (oldRoom != nullptr) {
			for (unsigned int i=0; i<oldRoom->m_surfaces.size(); ++i) {
				VICUS::Surface &s = oldRoom->m_surfaces[i];
				if(s.m_id == surf->m_id) {
					idTodeleteSurfPositions[oldRoom->m_id].push_back(i);
					break;
				}
			}
//			oldRoom->m_surfaces.erase(oldRoom->m_surfaces.begin()+pos);
//			oldRoom->updateParents();
			oldRoomName = oldRoom->m_displayName;
		}
		else
			oldRoomName = "Plain Geometry";

		// assign new parent;
		const VICUS::Object* obj = dynamic_cast<const VICUS::Object*>(selectedRoom);
		Q_ASSERT(obj != nullptr);
		selectedRoom->m_surfaces.back().m_parent = const_cast<VICUS::Object*>(obj);
		messageBoxText.append(QString("%3: %1 --> %2\n").arg(oldRoomName).arg(newRoomName).arg(surf->m_displayName));
	}

	// we delete the old surfaces and update their parents;
	for(std::map<unsigned int, std::vector<unsigned int>>::iterator it = idTodeleteSurfPositions.begin(); it != idTodeleteSurfPositions.end(); ++it) {
		VICUS::Room *oldRoom = vp.roomByID(it->first);

		for(unsigned int i=it->second.size(); i>0;--i) {
			std::vector<VICUS::Surface>::iterator it2 = oldRoom->m_surfaces.begin();
			std::advance(it2, it->second[i-1]);
			oldRoom->m_surfaces.erase(it2);
		}

		oldRoom->updateParents();
	}

	if(hadAtLeastOneChangedSurface) {
		// update all pointers
//		vp.updatePointers();
		SVUndoModifyProject * undo = new SVUndoModifyProject("Assiging Surfaces to different Room.", vp);
		undo->push();
		// SVProjectHandler::instance().setModified(SVProjectHandler::AllModified);
		QMessageBox::information(this, QString("Assigning surfaces to room '%1'").arg(newRoomName), messageBoxText );
	}
	else {
		QMessageBox::information(this, QString("Assigning surfaces to room '%1'").arg(newRoomName), QString("All surfaces were already assigned to room '%1'").arg(newRoomName) );
	}
}

void SVPropBuildingZoneProperty::on_tableWidgetZones_itemSelectionChanged() {
	QList<QTableWidgetSelectionRange> range = m_ui->tableWidgetZones->selectedRanges();
	m_ui->pushButtonAddSurface->setEnabled(true);
	for (unsigned int i=0; i<range.size(); ++i) {
		const QTableWidgetSelectionRange &r = range[i];

		if (r.rowCount() > 1 || r.rowCount() == 0) {
			m_ui->pushButtonAddSurface->setEnabled(false);
			return;
		}
		else{
			unsigned int id = m_ui->tableWidgetZones->item(r.bottomRow(), 0)->text().toUInt();
			const VICUS::Room* room = dynamic_cast<const VICUS::Room*>(project().objectById(id));

			Q_ASSERT(room != nullptr);

			m_selectedRoomID = const_cast<VICUS::Room*>(room)->m_id;
		}
	}
}
