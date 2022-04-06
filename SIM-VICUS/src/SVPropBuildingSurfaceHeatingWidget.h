#ifndef SVPropBuildingSurfaceHeatingWidgetH
#define SVPropBuildingSurfaceHeatingWidgetH

#include <QWidget>

#include <set>

class QTableWidgetItem;

namespace Ui {
	class SVPropBuildingSurfaceHeatingWidget;
}

namespace VICUS {
	class ComponentInstance;
	class Network;
}

/*! A property widget to assign and edit surface heatings. */
class SVPropBuildingSurfaceHeatingWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVPropBuildingSurfaceHeatingWidget(QWidget *parent = nullptr);
	~SVPropBuildingSurfaceHeatingWidget();

	/*! Updates user interface. */
	void updateUi();

	/*! Public access to generic network map. */
	std::map<unsigned int, const VICUS::Network*> genericNetworks() const
			{ return m_genericNetworks;}

	void generateGenericNetwork();
	void editGenericNetwork(unsigned int networkId);

private slots:
	void on_comboBoxSurfaceHeatingComponentFilter_currentIndexChanged(int index);
	void on_tableWidgetSurfaceHeating_itemChanged(QTableWidgetItem *item);
	void on_pushButtonRemoveSurfaceHeating_clicked();
	void on_pushButtonAssignSurfaceHeating_clicked();
	void on_tableWidgetSurfaceHeating_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
	void on_tableWidgetSurfaceHeating_itemSelectionChanged();
	void on_pushButtonRemoveSelectedSurfaceHeating_clicked();
	void on_pushButtonAssignSurfaceHeatingControlZone_clicked();
	void on_pushButtonAssignSurfaceHeatingNetwork_clicked();

private:
	Ui::SVPropBuildingSurfaceHeatingWidget *m_ui;

	std::set<const VICUS::ComponentInstance*>	m_selectedComponentInstances;

	// additional pointer to all networks that are generated generically
	std::map<unsigned int, const VICUS::Network*> m_genericNetworks;
};

#endif // SVPropBuildingSurfaceHeatingWidgetH
