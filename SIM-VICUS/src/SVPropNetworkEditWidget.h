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

enum SelectionState {
	S_SingleObject,
	S_MultipleObjects,
	NUM_S
};

/*! A property widget for editing network properties. */
class SVPropNetworkEditWidget : public QWidget {
	Q_OBJECT
public:
	explicit SVPropNetworkEditWidget(QWidget *parent = nullptr);
	~SVPropNetworkEditWidget();

	/*! Called when widget is just shown, updates content to current project's data
		and selected node. */
	void updateUi(const SelectionState selectionState);

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

	void on_lineEditNodeHeatingDemand_editingFinished();

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

	template <class Telem, class Tprop>
	bool uniformProperty(std::vector<Telem*> vec, Tprop prop);

	const VICUS::Network * currentNetwork();

	const VICUS::NetworkEdge * currentNetworkEdges();

	std::vector<const VICUS::NetworkNode *> currentNetworkNodes();

	Ui::SVPropNetworkEditWidget		*m_ui;

	VICUS::Network					m_network;

	QMap<QString, unsigned>			m_mapPipeModels;

	QMap<QString, unsigned>			m_mapComponents;

	QMap<QString, unsigned>			m_mapNodeTypes;

	QMap<QString, unsigned>			m_mapDBPipes;

	QMap<QString, unsigned>			m_mapHeatExchangeType;

	unsigned int					m_treeItemId = 0;

	std::set<const VICUS::Object*>	m_selectedObjects;

	SelectionState					m_selection = NUM_S;
};



#endif // SVPropNetworkEditWidgetH
