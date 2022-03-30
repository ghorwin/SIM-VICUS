#ifndef SVPropBuildingZonePropertyH
#define SVPropBuildingZonePropertyH


#include "SVPropBuildingZonePropertyTableModel.h"

#include <QWidget>

#include <set>

class QSortFilterProxyModel;
class QTableWidgetItem;

namespace Ui {
	class SVPropBuildingZoneProperty;
}

namespace VICUS {
	class Room;
}

/*! A property widget to edit zone properties. */
class SVPropBuildingZoneProperty : public QWidget {
	Q_OBJECT

public:
	explicit SVPropBuildingZoneProperty(QWidget *parent = nullptr);
	~SVPropBuildingZoneProperty();

	/*! Updates user interface. */
	void updateUi();

private slots:

	void on_comboBoxBuildingFilter_currentIndexChanged(int index);

	void on_comboBoxBuildingLevelFilter_currentIndexChanged(int index);

	void on_tableViewZones_doubleClicked(const QModelIndex &index);

	void on_pushButtonFloorAreaSelectedRooms_clicked();

	void on_pushButtonVolumeSelectedRooms_clicked();

	void on_pushButtonFloorAreaAllRooms_clicked();

	void on_pushButtonVolumeAllRooms_clicked();

	//void on_tableViewZones_itemSelectionChanged();

	//void on_tableViewZones_cellPressed(int row, int column);

	void on_pushButtonAssignSurface_clicked();

	void on_tableViewZones_selectionChanged();

private:

	Ui::SVPropBuildingZoneProperty			*m_ui;

	/*! Table model ifor zone properties. */
	SVPropBuildingZonePropertyTableModel	*m_zonePropertiesTableModel = nullptr;

	QSortFilterProxyModel					*m_zonePropertiesProxyModel = nullptr;

	/*! Holds selected room for adding surfaces to it.
		Updated in itemSelectionChanged.
	*/
	QModelIndex								m_selectedProxyIndex;
};

#endif // SVPropBuildingZonePropertyH
