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

	bool setNetwork();

	/*! modifies the given property of selected edge(s).
	 * Encapsulates the process of retrieving the according edge and conducting the undo */
	template <typename TEdgeProp, typename Tval>
	void modifyEdgeProperty(TEdgeProp property, const Tval & value);

	/*! modifies the given property of selected node(s).
	 * Encapsulates the process of retrieving the according node and conducting the undo */
	template <typename TNodeProp, typename Tval>
	void modifyNodeProperty(TNodeProp property, const Tval & value);

	/*! determines wether the given property of the vector of objects is constant for all elements in the vector */
	template <typename Telem, typename Tprop>
	static bool uniformProperty(const std::vector<Telem *> & vec, Tprop property)
	{
		if (vec.empty())
			return false;
		typename std::vector<Telem *>::const_iterator itFirst = vec.begin();
		for (typename std::vector<Telem *>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
			if ((*itFirst)->*property != (*it)->*property)
				return false;
		}
		return true;
	}

	const VICUS::Network * currentNetwork();

	std::vector<const VICUS::NetworkEdge *> currentNetworkEdges();

	std::vector<const VICUS::NetworkNode *> currentNetworkNodes();

	Ui::SVPropNetworkEditWidget		*m_ui;

	VICUS::Network					m_network;

	QMap<QString, unsigned>			m_mapComponents;

	QMap<QString, unsigned>			m_mapNodeTypes;

	QMap<QString, unsigned>			m_mapDBPipes;

	unsigned int					m_treeItemId = 0;

	std::set<const VICUS::Object*>	m_selectedObjects;

	SelectionState					m_selection = NUM_S;
};



#endif // SVPropNetworkEditWidgetH
