#ifndef SVPropBuildingZonePropertyH
#define SVPropBuildingZonePropertyH

#include <QWidget>

#include <set>

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

	void on_tableWidgetZones_itemDoubleClicked(QTableWidgetItem *item);

	void on_tableWidgetZones_cellChanged(int row, int column);

	void on_pushButtonFloorArea_clicked();

	void on_pushButtonVolume_clicked();


	//void on_tableWidgetZones_itemSelectionChanged();

	//void on_tableWidgetZones_cellPressed(int row, int column);

	void on_pushButtonAssignSurface_clicked();

	void on_tableWidgetZones_itemSelectionChanged();

private:

	void calculatedParameters(bool floorAreaCalc = true, bool onlySelected = true);

	Ui::SVPropBuildingZoneProperty	*m_ui;

	/*! Holds selected room for adding surfaces to it.
		Updated in itemSelectionChanged.
	*/
	unsigned int					m_selectedRoomID;
};

#endif // SVPropBuildingZonePropertyH
