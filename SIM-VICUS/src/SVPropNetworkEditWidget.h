#ifndef SVPropNetworkEditWidgetH
#define SVPropNetworkEditWidgetH

#include <QWidget>
#include <QMap>

#include <VICUS_Network.h>

namespace VICUS {
	class NetworkEdge;
	class NetworkNode;
	class NetworkComponent;
}

namespace Ui {
	class SVPropNetworkEditWidget;
}

class ModificationInfo;

/*! A property widget for editing network properties. */
class SVPropNetworkEditWidget : public QWidget {
	Q_OBJECT
public:
	explicit SVPropNetworkEditWidget(QWidget *parent = nullptr);
	~SVPropNetworkEditWidget();

	/*! Selects the respective edit mode for the widget.
		This function is called whenever a selection change or edit mode change has occurred
		that needs an update of the scene mode or switch of active network.
	*/
	void setPropertyMode(int propertyIndex);

	void showNetworkProperties();

	void showNodeProperties();

	void showEdgeProperties();

	void showComponentProperties();

public slots:

	/*! Connected to SVProjectHandler::modified(), we listen to changes in selections. */
	void onModified( int modificationType, ModificationInfo * data );


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

	void on_pushButtonEditComponents_clicked();

	void on_lineEditHeatFlux_editingFinished();

	void on_lineEditNodeHeatingDemand_editingFinished();

	void on_horizontalSliderScaleNodes_valueChanged(int value);

	void on_horizontalSliderScaleEdges_valueChanged(int value);

	void on_pushButtonSelectPipes_clicked();

	void on_comboBoxComponent_currentIndexChanged(int index);

	void on_lineEditNodeDisplayName_editingFinished();

	void on_lineEditEdgeDisplayName_editingFinished();

	void on_lineEditTemperature_editingFinished();

	void on_heatExchangeDataFile_editingFinished();

	void on_comboBoxHeatExchangeType_currentIndexChanged(int index);

private:
	/*! This function is called whenever the current selection of edges/nodes/objects has changed.
		This can be due to user interaction with the scene, or because objects were added/deleted or
		because a project was newly loaded.
		In any case, the currently shown input widgets must be updated according to the current
		selection in the project.
	*/
	void selectionChanged();

	void setupComboBoxComponents();

	void setupComboboxPipeDB();

	void setupComboboxHeatExchangeType();

	void updateSizingParams();

	void updateNodeProperties();

	void updateEdgeProperties();

	void updateNetworkProperties();

	void updateHeatExchangeProperties();

	void modifyStatus();

	void modifySizingParams();

	bool setNetwork();

	void clearUI();

	void setAllEnabled(bool enabled);

	void setAllHeatExchangeWidgetsVisible(bool visible);

	const VICUS::NetworkComponent *currentComponent();

	QString largestDiameter() const;

	QString smallestDiameter() const;

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

	Ui::SVPropNetworkEditWidget		*m_ui;

	/*! Contains the currently selected network, or the network, of the currently selected nodes/edges.	*/
	const VICUS::Network *			m_currentConstNetwork = nullptr;

	/*! Contains the currently selected network, or the network, of the currently selected nodes/edges.	*/
	VICUS::Network					m_currentNetwork;

	std::vector<const VICUS::NetworkEdge *> m_currentEdges;

	std::vector<const VICUS::NetworkNode *> m_currentNodes;
};



#endif // SVPropNetworkEditWidgetH
