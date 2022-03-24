#include "SVPropBuildingZonePropertyTableModel.h"

#include "SVProjectHandler.h"
#include "SVUndoModifyRoom.h"

#include <VICUS_Building.h>
#include <VICUS_KeywordList.h>
#include <VICUS_Project.h>
#include <VICUS_Room.h>

#include <QColor>
#include <QMessageBox>

SVPropBuildingZonePropertyTableModel::SVPropBuildingZonePropertyTableModel(QObject * parent, bool inputVariableTable) :
	QAbstractTableModel(parent)
{
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

	const VICUS::Room &room = m_rooms[(size_t)index.row()];
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

		case Qt::EditRole :
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
	VICUS::Room &room = m_rooms[(size_t)index.row()];
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

	std::vector<VICUS::Building> buildings = project().m_buildings;

	// first find the room
	bool foundRoom = false;
	for(unsigned int i=0; i<buildings.size(); ++i){
		const VICUS::Building & b = buildings[i];
		for(unsigned j=0; j<b.m_buildingLevels.size(); ++j){
			VICUS::BuildingLevel bl = b.m_buildingLevels[j];
			for(unsigned k=0; k<bl.m_rooms.size(); ++k){
				VICUS::Room & r = bl.m_rooms[k];
				if(r.m_id == room.m_id){

					SVUndoModifyRoom * undo = new SVUndoModifyRoom(text, room, i, j, k);
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

	emit dataChanged(index, index);
	return true;
}


void SVPropBuildingZonePropertyTableModel::updateBuildingLevelIndex(int buildingIndex, int buildingLevelIndex)
{

	if(buildingIndex == -1){
		m_rooms.clear();
		for( const VICUS::Building &b : project().m_buildings){
			// put all rooms in
			for( const VICUS::BuildingLevel &bl : b.m_buildingLevels)
				for( const VICUS::Room &r : bl.m_rooms)
					m_rooms.push_back(r);
		}
	}
	else{
		m_rooms.clear();
		if(buildingLevelIndex == -1){
			// put all rooms of this building in
			for( const VICUS::BuildingLevel &bl : project().m_buildings[(unsigned int) buildingIndex].m_buildingLevels)
				for( const VICUS::Room &r : bl.m_rooms)
					m_rooms.push_back(r);
		}
		else{
			// put only rooms of this building level in
			const VICUS::BuildingLevel &bl = project().m_buildings[(unsigned int) buildingIndex].
					m_buildingLevels[(unsigned int) buildingLevelIndex];
			m_rooms = bl.m_rooms;
		}
	}
}

