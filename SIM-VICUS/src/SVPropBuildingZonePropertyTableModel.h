#ifndef SVPropBuildingZonePropertyTableModelH
#define SVPropBuildingZonePropertyTableModelH

#include "SVZoneInformationDialog.h"
#include <NANDRAD_FMIVariableDefinition.h>

#include <IBK_NotificationHandler.h>

#include <QAbstractTableModel>
#include <QObject>
#include <QFont>

#include <set>

namespace VICUS {
	class Room;
};

class Notification : public IBK::NotificationHandler {
public:
	bool	m_aborted = false;
};


class SVPropBuildingZonePropertyTableModel : public QAbstractTableModel {
	Q_OBJECT
public:

	// struct specifiing position of each room  in building list, buzilding level list and
	// local room list
	struct roomLocation {
		roomLocation(unsigned int i, unsigned int j, unsigned int k) :
			m_buildingIndex(i),
			m_buildingLevelIndex(j),
			m_roomIndex(k)
		{ }

		unsigned int m_buildingIndex;
		unsigned int m_buildingLevelIndex;
		unsigned int m_roomIndex;
	};

	SVPropBuildingZonePropertyTableModel(QObject *parent);

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;
	bool setData(const QModelIndex & index, const QVariant & value, int role) override;

	/*! Performs a model reset. */
	void reset();

	// changes the selection of rooms for a given building index and building level index
	void updateBuildingLevelIndex(int buildingIndex, int buildingLevelIndex);
	// automatically calculates zone floor area for an item selection
	void calulateFloorArea(Notification * notify, const QModelIndexList &indexes);
	// automatically calculates zone volume for an item selection
	void calulateVolume(Notification * notify, const QModelIndexList &indexes);
	// assignes surfaces to a chosen room and returns true if succeeded (false otherwise)
	bool assignSurfaces(const QModelIndex &index, QString &msg);

	// Show additional zone information
	void showZoneInformation(const QModelIndex &index);

	/*! Returns pointer to zone information dialog. */
	SVZoneInformationDialog* zoneInformationDialog();

private:
	// QFont
	QFont	m_itemFont;

	// list of all rooms by filter selection (for constant fast access)
	std::vector<const VICUS::Room*>		m_rooms;
	// corresponding room locations for each room in list
	std::vector<roomLocation>			m_roomLocations;
	// Zone information dialgo
	SVZoneInformationDialog				*m_zoneInformationDialog = nullptr;
};





#endif // SVPropBuildingZonePropertyTableModelH
