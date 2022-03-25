#include "SVPropBuildingZonePropertyTableModel.h"

#include "SVProjectHandler.h"
#include "SVUndoModifyBuildingTopology.h"
#include "SVUndoModifyRoom.h"

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

	// constant access to selected room
	const VICUS::Room &room = *m_rooms[(size_t)index.row()];

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
						return room.m_para[VICUS::Room::P_Area].get_value("m2");
					return QVariant();
				// column 3 - room volume
				case 3 :
					//get parameter
					if(!room.m_para[VICUS::Room::P_Volume].empty())
						return room.m_para[VICUS::Room::P_Volume].get_value("m3");
					return QVariant();
			}

		case Qt::EditRole : {
			Q_ASSERT(index.column() == 2 || index.column() == 3);
			switch (index.column()) {
				// column 2 - room floor area
				case 2 :
					//get parameter
					if(!room.m_para[VICUS::Room::P_Area].empty())
						return room.m_para[VICUS::Room::P_Area].get_value("m2");
					return QVariant();
				// column 3 - room volume
				case 3 :
					//get parameter
					if(!room.m_para[VICUS::Room::P_Volume].empty())
						return room.m_para[VICUS::Room::P_Volume].get_value("m3");
					return QVariant();
			}
		}

		case Qt::FontRole : {
			//      with valid value -> black, bold
			QFont f(m_itemFont);
			f.setBold(true);
			return f;
		}

		// UserRole returns value reference
		case Qt::UserRole :
			return room.m_id;
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

	//get value
	bool valueOk;
	double val = value.toDouble(&valueOk);

	if(!valueOk || val<=0)
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
		// column 2 - room floor area
		case 2 :
			//set parameter in base unit 'm2'
			VICUS::KeywordList::setParameter(room.m_para, "Room::para_t", VICUS::Room::P_Area, val);
			text = tr("Modified floor area");
		break;
		// column 3 - room volume
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


void SVPropBuildingZonePropertyTableModel::updateBuildingLevelIndex(int buildingIndex, int buildingLevelIndex)
{

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
}


void SVPropBuildingZonePropertyTableModel::calulateFloorArea(const QModelIndexList &indexes)
{
	// make a copy of buildings data structure
	std::vector<VICUS::Building>	buildings = project().m_buildings;

	// loop through all indexes, retrieve room and perform reacalulation on a copy of building
	for(const QModelIndex &index : indexes) {
		// retrieve room location
		const roomLocation &location = m_roomLocations[(size_t)index.row()];
		unsigned int i = location.m_buildingIndex;
		unsigned int j = location.m_buildingLevelIndex;
		unsigned int k = location.m_roomIndex;

		// change room floor area in builing copy
		VICUS::Room &room = buildings[i].m_buildingLevels[j].m_rooms[k];
		// make calculation here
		room.calculateFloorArea();
	}
	QString text = "Floor area calculation";

	SVUndoModifyBuildingTopology *undo = new SVUndoModifyBuildingTopology(text, buildings);
	undo->push();
}


void SVPropBuildingZonePropertyTableModel::calulateVolume(const QModelIndexList & indexes)
{
	std::vector<VICUS::Building>	buildings = project().m_buildings;

	// loop through all indexes, retrieve room and perform reacalulation on a copy of building
	for(const QModelIndex &index : indexes) {
		// retrieve room location
		const roomLocation &location = m_roomLocations[(size_t)index.row()];
		unsigned int i = location.m_buildingIndex;
		unsigned int j = location.m_buildingLevelIndex;
		unsigned int k = location.m_roomIndex;

		// change room volume in builing copy
		VICUS::Room &room = buildings[i].m_buildingLevels[j].m_rooms[k];
		// make calculation here
		room.calculateVolume();
	}
	QString text = "Volume calculation";

	SVUndoModifyBuildingTopology *undo = new SVUndoModifyBuildingTopology(text, buildings);
	undo->push();
}

