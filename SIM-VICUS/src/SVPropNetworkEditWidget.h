#ifndef SVPropNetworkEditWidgetH
#define SVPropNetworkEditWidgetH

#include <QWidget>
#include <QMap>

#include <VICUS_Network.h>

namespace VICUS {
	class NetworkEdge;
	class NetworkNode;
}

namespace Ui {
	class SVPropNetworkEditWidget;
}

class SVHydraulicComponentParameterModel;

/*! A property widget for editing network properties. */
class SVPropNetworkEditWidget : public QWidget {
	Q_OBJECT
public:
	explicit SVPropNetworkEditWidget(QWidget *parent = nullptr);
	~SVPropNetworkEditWidget();

	/*! Called when widget is just shown, updates content to current project's data
		and selected node. */
	void updateUi();

	void showNetworkProperties();

	void showNodeProperties();

	void showEdgeProperties();

private slots:
	void on_comboBoxNodeType_activated(int index);

	void on_lineEditNodeX_editingFinished();

	void on_lineEditNodeY_editingFinished();

	void on_comboBoxPipeModel_activated(int index);

	void on_comboBoxPipeDB_activated(int index);

	void on_checkBoxSupplyPipe_clicked();

	void on_pushButtonSizePipeDimensions_clicked();

	void on_pushButtonGenerateIntersections_clicked();

	void on_pushButtonConnectBuildings_clicked();

	void on_pushButtonReduceDeadEnds_clicked();

	void on_pushButtonReduceRedundantNodes_clicked();

	void on_horizontalSliderScaleNodes_actionTriggered(int action);

	void on_horizontalSliderScaleEdges_actionTriggered(int action);

	void on_comboBoxComponent_activated(const QString &arg1);

	void on_pushButtonEditComponents_clicked();

	void on_lineEditHeatFlux_editingFinished();

	void on_comboBoxHeatExchangeType_activated(const QString &arg1);

private:

	void setupComboBoxComponents();

	void setupComboboxPipeDB();

	void updateSizingParams();

	void updateNodeProperties();

	void updateEdgeProperties();

	void updateNetworkProperties();

	void updateHeatExchangeParams();

	void modifyStatus();

	void modifySizingParams();

	void modifyNodeProperties();

	void modifyEdgeProperties();

	bool setNetwork();

	const VICUS::Network * currentNetwork();

	const VICUS::NetworkEdge * currentNetworkEdge();

	const VICUS::NetworkNode * currentNetworkNode();

	Ui::SVPropNetworkEditWidget *m_ui;

	VICUS::Network m_network;

	QMap<QString, unsigned> m_mapPipeModels;

	QMap<QString, unsigned> m_mapComponents;

	QMap<QString, unsigned> m_mapNodeTypes;

	QMap<QString, unsigned> m_mapDBPipes;

	QMap<QString, unsigned> m_mapHeatExchangeType;

	unsigned int m_treeItemId = 0;

	std::set<const VICUS::Object*> m_selectedObjects;

	SVHydraulicComponentParameterModel * m_componentParModel = nullptr;
};



#endif // SVPropNetworkEditWidgetH
