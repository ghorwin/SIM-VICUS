#include "SVPropBuildingZonePropertyTableModel.h"

#include "SVProjectHandler.h"
#include "SVUndoModifyBuildingTopology.h"
#include "SVUndoModifyRoom.h"

#include <IBK_StopWatch.h>

#include <VICUS_Building.h>
#include <VICUS_KeywordList.h>
#include <VICUS_Project.h>
#include <VICUS_Room.h>

#include <QColor>
#include <QMessageBox>

SVPropBuildingZonePropertyTableModel::SVPropBuildingZonePropertyTableModel(QObject * parent) :
	QAbstractTableModel(parent)
{
	// get access to all rooms
	updateBuildingLevelIndex(-1,-1);
}

int SVPropBuildingZonePropertyTableModel::rowCount(const QModelIndex & /*parent*/) const {
	return m_rooms.size();
}


int SVPropBuildingZonePropertyTableModel::columnCount(const QModelIndex & /*parent*/) const {
	return 4;
}


QVariant SVPropBuildingZonePropertyTableModel::data(const QModelIndex & index, int role) const {
	if (!index.isValid())
		return QVariant();

	Q_ASSERT((size_t)index.row() < m_rooms.size());
	Q_ASSERT(m_rooms[(size_t)index.row()] != nullptr);

	// constant access to selected room
	const VICUS::Room &room = *m_rooms[(size_t)index.row()];

	// qDebug() << room.m_displayName;

	switch (role) {
		case Qt::DisplayRole :
			switch (index.column()) {
			// column 0 - room id
			case 0 :
				return room.m_id;
				// column 1 - room name
			case 1 :
				return room.m_displayName;
				// column 2 - room floor area
			case 2 :
				//get parameter
				if(!room.m_para[VICUS::Room::P_Area].empty())
					return (int)(room.m_para[VICUS::Room::P_Area].get_value("m2")*100)/100.0;
				return QVariant();
				// column 3 - room volume
			case 3 :
				//get parameter
				if(!room.m_para[VICUS::Room::P_Volume].empty())
					return (int)(room.m_para[VICUS::Room::P_Volume].get_value("m3")*100)/100.0;
				return QVariant();
			}

		case Qt::EditRole : {
			Q_ASSERT(index.column() == 3 || index.column() == 4);
			switch (index.column()) {
			// column 3 - room floor area
			case 2 :
				//get parameter
				if(!room.m_para[VICUS::Room::P_Area].empty())
					return room.m_para[VICUS::Room::P_Area].get_value("m2");
				return QVariant();
				// column 4 - room volume
			case 3 :
				//get parameter
				if(!room.m_para[VICUS::Room::P_Volume].empty())
					return room.m_para[VICUS::Room::P_Volume].get_value("m3");
				return QVariant();
			}
		}

		case Qt::FontRole : {
			//      with valid value -> black, bold
			if(index.column() == 2 || index.column() == 3) {
				QFont f(m_itemFont);
				f.setBold(true);
				f.setItalic(true);
				return f;
			}
		}

			// UserRole returns value reference
		case Qt::UserRole : {
			return room.m_id;
		}
		case Qt::ForegroundRole : {
			// vars with INVALID values -> red text color
			if (index.column() == 2)
				if (!room.m_para[VICUS::Room::P_Area].empty() && room.m_para[VICUS::Room::P_Area].get_value("m2") < 1e-2)
					return QColor(Qt::red);
			if (index.column() == 3)
				if (!room.m_para[VICUS::Room::P_Volume].empty() && room.m_para[VICUS::Room::P_Volume].get_value("m3") < 1e-2)
					return QColor(Qt::red);
		}
	}
	return QVariant();
}


QVariant SVPropBuildingZonePropertyTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	static QStringList headers = QStringList()
			<< tr("Id")
			<< tr("Name")
			<< tr("Area [m2]")
			<< tr("Volume [m3]");

	if (orientation == Qt::Vertical)
		return QVariant();
	switch (role) {
	case Qt::DisplayRole :
		return headers[section];
	}
	return QVariant();
}


Qt::ItemFlags SVPropBuildingZonePropertyTableModel::flags(const QModelIndex & index) const {
	Q_ASSERT((size_t)index.row() < m_rooms.size());
	// column 0 - room id
	if (index.column() == 0)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	// column 1 - room name
	if (index.column() == 1)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	// column 2 - room floor area
	if (index.column() == 2)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	// column 3 - room volume
	if (index.column() == 3)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
	return QAbstractTableModel::flags(index); // call base class implementation
}


bool SVPropBuildingZonePropertyTableModel::setData(const QModelIndex & index, const QVariant & value, int /*role*/) {
	Q_ASSERT(index.isValid());
	Q_ASSERT(index.column() == 2 || index.column() == 3);
	Q_ASSERT((size_t)index.row() < m_rooms.size());
	Q_ASSERT(m_rooms[(size_t)index.row()] != nullptr);

	//get value
	bool valueOk;
	double val = value.toDouble(&valueOk);

	if(!valueOk || val<0)
		return false;

	QString text;
	// retreive room location
	const roomLocation &location = m_roomLocations[(size_t)index.row()];
	unsigned int i = location.m_buildingIndex;
	unsigned int j = location.m_buildingLevelIndex;
	unsigned int k = location.m_roomIndex;

	// retrieve a copy of room
	VICUS::Room room = *m_rooms[(size_t)index.row()];

	switch (index.column()) {
	// column 3 - room floor area
	case 2 :
		//set parameter in base unit 'm2'
		VICUS::KeywordList::setParameter(room.m_para, "Room::para_t", VICUS::Room::P_Area, val);
		text = tr("Modified floor area");
		break;
		// column 4 - room volume
	case 3 :
		//set parameter in base unit 'm3'
		VICUS::KeywordList::setParameter(room.m_para, "Room::para_t", VICUS::Room::P_Volume, val);
		text = tr("Modified volume");
		break;
	default: break;
	}

	// create undo action for room parameter change
	SVUndoModifyRoom * undo = new SVUndoModifyRoom(text, room, i, j, k);
	// copy new room parametrization into VICUS data structure
	undo->push();

	emit dataChanged(index, index);
	return true;
}

void SVPropBuildingZonePropertyTableModel::reset() {
	beginResetModel();
	endResetModel();
}

void SVPropBuildingZonePropertyTableModel::updateBuildingLevelIndex(int buildingIndex, int buildingLevelIndex) {

	const std::vector<VICUS::Building> &buildings = project().m_buildings;

	m_rooms.clear();
	m_roomLocations.clear();

	if(buildingIndex <= 0){
		// put all rooms of this building into vector
		for(unsigned int i=0; i<buildings.size(); ++i){
			const VICUS::Building & b = buildings[i];
			for(unsigned j=0; j<b.m_buildingLevels.size(); ++j){
				const VICUS::BuildingLevel &bl = b.m_buildingLevels[j];
				for(unsigned k=0; k<bl.m_rooms.size(); ++k) {
					m_rooms.push_back(&bl.m_rooms[k]);
					m_roomLocations.push_back(roomLocation(i,j,k));
				}
			}
		}
	}
	else{
		if(buildingLevelIndex <= 0){
			// filter all rooms of a given building
			unsigned int i = (unsigned int) buildingIndex - 1;
			const VICUS::Building & b = buildings[i];
			for(unsigned j=0; j<b.m_buildingLevels.size(); ++j){
				const VICUS::BuildingLevel &bl = b.m_buildingLevels[j];
				for(unsigned k=0; k<bl.m_rooms.size(); ++k) {
					m_rooms.push_back(&bl.m_rooms[k]);
					m_roomLocations.push_back(roomLocation(i,j,k));
				}
			}
		}
		else {
			// put only rooms of current building level into vector
			unsigned int i = (unsigned int) buildingIndex - 1;
			unsigned int j = (unsigned int) buildingLevelIndex - 1;
			const VICUS::BuildingLevel &bl = project().m_buildings[i].
					m_buildingLevels[j];
			for(unsigned k=0; k<bl.m_rooms.size(); ++k) {
				m_rooms.push_back(&bl.m_rooms[k]);
				m_roomLocations.push_back(roomLocation(i,j,k));
			}
		}
	}
	reset();
}


void SVPropBuildingZonePropertyTableModel::calulateFloorArea(Notification * notify, const QModelIndexList &indexes) {

	IBK::StopWatch totalTimer;
	totalTimer.start();
	// the stop watch object and progress counter are used only in a critical section
	IBK::StopWatch w;
	w.start();
	notify->notify(0);
	int roomsCompleted = 1;

	// make a copy of buildings data structure
	std::vector<VICUS::Building>	buildings = project().m_buildings;

	// loop through all indexes, retrieve room and perform reacalulation on a copy of building
	for(const QModelIndex &index : indexes) {

		if (notify->m_aborted)
			return; // skip ahead to quickly stop loop

		Q_ASSERT((size_t)index.row() < m_rooms.size());
		// retrieve room location
		const roomLocation &location = m_roomLocations[(size_t)index.row()];
		unsigned int i = location.m_buildingIndex;
		unsigned int j = location.m_buildingLevelIndex;
		unsigned int k = location.m_roomIndex;

		// change room floor area in builing copy
		VICUS::Room &room = buildings[i].m_buildingLevels[j].m_rooms[k];
		// make calculation here
		room.calculateFloorArea();

		// only notify every second or so
		if (!notify->m_aborted && w.difference() > 100) {
			notify->notify(double(roomsCompleted) / double(indexes.size()));
			w.start();
		}

		++roomsCompleted;
	}
	QString text = "Floor area calculation";

	notify->notify(1);

	SVUndoModifyBuildingTopology *undo = new SVUndoModifyBuildingTopology(text, buildings);
	undo->push();

	reset();
}


void SVPropBuildingZonePropertyTableModel::calulateVolume(Notification * notify, const QModelIndexList & indexes) {

	IBK::StopWatch totalTimer;
	totalTimer.start();
	// the stop watch object and progress counter are used only in a critical section
	IBK::StopWatch w;
	w.start();
	notify->notify(0);
	int roomsCompleted = 1;

	std::vector<VICUS::Building>	buildings = project().m_buildings;

	// loop through all indexes, retrieve room and perform reacalulation on a copy of building
	for(const QModelIndex &index : indexes) {

		if (notify->m_aborted)
			return; // skip ahead to quickly stop loop

		Q_ASSERT((size_t)index.row() < m_rooms.size());

		// retrieve room location
		const roomLocation &location = m_roomLocations[(size_t)index.row()];
		unsigned int i = location.m_buildingIndex;
		unsigned int j = location.m_buildingLevelIndex;
		unsigned int k = location.m_roomIndex;

		// change room volume in builing copy
		VICUS::Room &room = buildings[i].m_buildingLevels[j].m_rooms[k];
		// make calculation here
		room.calculateVolume();

		// only notify every second or so
		if (!notify->m_aborted && w.difference() > 100) {
			qDebug() << double(roomsCompleted) / double(indexes.size());
			notify->notify(double(roomsCompleted) / double(indexes.size()) );
			w.start();
		}

		++roomsCompleted;
	}
	QString text = "Volume calculation";

	notify->notify(1);

	SVUndoModifyBuildingTopology *undo = new SVUndoModifyBuildingTopology(text, buildings);
	undo->push();

	reset();
}


bool SVPropBuildingZonePropertyTableModel::assignSurfaces(const QModelIndex & index, QString &msg) {
	Q_ASSERT(index.isValid());

	// udate a project copy
	VICUS::Project vp = project();
	vp.updatePointers();

	Q_ASSERT((size_t)index.row() < m_rooms.size());
	Q_ASSERT(m_rooms[(size_t)index.row()] != nullptr);

	// retieve a copy of the selected room
	VICUS::Room &room = *vp.roomByID(m_rooms[(size_t)index.row()]->m_id);

	// we get the selected surfaces
	std::vector<const VICUS::Surface*> surfs;
	vp.selectedSurfaces(surfs, VICUS::Project::SG_Building);
	QString newRoomName, oldRoomName;

	newRoomName = room.m_displayName;
	bool hadAtLeastOneChangedSurface = false;

	std::map<unsigned int, std::vector<unsigned int>> idTodeleteSurfPositions;

	// we have to mind plain geometry and already assigned surfaces
	for(const VICUS::Surface *surf : surfs) {

		// if surface has already a parent we have to delete the surface from the parent
		// and update its parent
		VICUS::Room* oldRoom = dynamic_cast<VICUS::Room*>(surf->m_parent);
		if(oldRoom->m_id == room.m_id)
			continue; // skip if surface is already assigned to specified room

		room.m_surfaces.push_back(*surf);
		room.updateParents();
		hadAtLeastOneChangedSurface = true;

		if (oldRoom != nullptr) {
			for (unsigned int i=0; i<oldRoom->m_surfaces.size(); ++i) {
				VICUS::Surface &s = oldRoom->m_surfaces[i];
				if(s.m_id == surf->m_id) {
					idTodeleteSurfPositions[oldRoom->m_id].push_back(i);
					break;
				}
			}
			oldRoomName = oldRoom->m_displayName;
		}
		else
			oldRoomName = "Plain Geometry";

		// assign new parent;
		const VICUS::Object* obj = dynamic_cast<const VICUS::Object*>(&room);
		Q_ASSERT(obj != nullptr);
		room.m_surfaces.back().m_parent = const_cast<VICUS::Object*>(obj);
		msg.append(QString("%3: %1 --> %2\n").arg(oldRoomName).arg(newRoomName).arg(surf->m_displayName));
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
		SVUndoModifyBuildingTopology * undo = new SVUndoModifyBuildingTopology("Assigning surfaces to different room.", vp.m_buildings);
		undo->push();
		return true;
	}
	else {
		return false;
	}

}

