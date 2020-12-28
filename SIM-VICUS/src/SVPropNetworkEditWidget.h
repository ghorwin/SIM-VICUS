#ifndef SVPropNetworkEditWidgetH
#define SVPropNetworkEditWidgetH

#include <QWidget>
#include <QMap>
#include <QAbstractTableModel>

#include "NANDRAD_HydraulicNetworkComponent.h"

namespace VICUS {
	class Network;
	class Object;
}

namespace Ui {
	class SVPropNetworkEditWidget;
}

class ComponentParameterModel;

/*! A property widget for editing network properties. */
class SVPropNetworkEditWidget : public QWidget {
	Q_OBJECT
public:
	explicit SVPropNetworkEditWidget(QWidget *parent = nullptr);
	~SVPropNetworkEditWidget();

	/*! Called when widget is just shown, updates content to current project's data
		and selected node. */
	void updateUi();

	void updateSizingParams();

	void modifyStatus();

	void modifySizingParams();

	void modifyNodeProperties();

	void modifyEdgeProperties();

	void showNetworkProperties();

	void showNodeProperties();

	void showEdgeProperties();

	void setupComboboxPipeDB();

private slots:
	void on_comboBoxNodeType_activated(int index);

	void on_doubleSpinBoxNodeNeatingDemand_editingFinished();

	void on_lineEditNodeX_editingFinished();

	void on_comboBoxPipeModel_activated(int index);

	void on_comboBoxPipeDB_activated(int index);

	void on_checkBoxSupplyPipe_clicked();

	void on_pushButtonSizePipeDimensions_clicked();

	void on_pushButtonGenerateIntersections_clicked();

	void on_pushButtonConnectBuildings_clicked();

	void on_pushButtonReduceDeadEnds_clicked();

	void on_pushButtonReduceRedundantNodes_clicked();

	void on_lineEditNodeY_editingFinished();

	void on_horizontalSliderScaleNodes_actionTriggered(int action);

	void on_horizontalSliderScaleEdges_actionTriggered(int action);

	void on_comboBoxComponent_activated(const QString &arg1);

	void on_componentParModel_editCompleted();

private:

	void networkFromId();

	void updateHydraulicComponent();

	void modifyHydraulicComponent();

	Ui::SVPropNetworkEditWidget *m_ui;

	const VICUS::Network *m_network = nullptr;

	const VICUS::Object * m_obj = nullptr;

	QMap<QString, unsigned> m_mapPipeModels;

	QMap<QString, unsigned> m_mapComponents;

	QMap<QString, unsigned> m_mapNodeTypes;

	QMap<QString, unsigned> m_mapDBPipes;

	QMap<QString, unsigned> m_mapHeatExchangeType;

	unsigned int m_treeItemId = 0;

	ComponentParameterModel * m_componentParModel = nullptr;
};




class ComponentParameterModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	ComponentParameterModel(QObject *parent = 0);

	void setComponent(const NANDRAD::HydraulicNetworkComponent & component);
	void getComponentParameter(IBK::Parameter m_para[]);
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex & index) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
private:
	NANDRAD::HydraulicNetworkComponent m_component;
	std::vector<unsigned int> m_parameterList;
signals:
	void editCompleted();

};


#endif // SVPropNetworkEditWidgetH
