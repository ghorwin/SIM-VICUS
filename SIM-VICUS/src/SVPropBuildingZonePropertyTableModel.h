#ifndef SVPropBuildingZonePropertyTableModelH
#define SVPropBuildingZonePropertyTableModelH

#include <NANDRAD_FMIVariableDefinition.h>

#include <QAbstractTableModel>
#include <QObject>
#include <QFont>

#include <set>

namespace VICUS {
	class Room;
};


class SVPropBuildingZonePropertyTableModel : public QAbstractTableModel {
	Q_OBJECT
public:

	SVPropBuildingZonePropertyTableModel(QObject *parent, bool inputVariableTable);

	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;
	bool setData(const QModelIndex & index, const QVariant & value, int role) override;

	// changes the selection of rooms for a given building index and building level index
	void updateBuildingLevelIndex(int buildingIndex, int buildingLevelIndex);

	QFont	m_itemFont;

	// get list of all rooms by filter selection
	std::vector<VICUS::Room>		m_rooms;
};





#endif // SVPropBuildingZonePropertyTableModelH
