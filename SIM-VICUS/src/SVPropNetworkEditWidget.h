/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef SVPropNetworkEditWidgetH
#define SVPropNetworkEditWidgetH

#include <QWidget>

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
class SVPropModeSelectionWidget;

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

	/*! This function is called whenever the current selection of edges/nodes/objects has changed.
		This can be due to user interaction with the scene, or because objects were added/deleted or
		because a project was newly loaded.
		In any case, the currently shown input widgets must be updated according to the current
		selection in the project.
	*/
	void selectionChanged(unsigned int networkId);

	SVPropModeSelectionWidget				* m_propModeSelectionWidget;

public slots:

	/*! Connected to SVProjectHandler::modified(), we listen to changes in selections. */
	void onModified( int modificationType, ModificationInfo * data );


private slots:
	void on_comboBoxNodeType_activated(int index);

	void on_lineEditNodeX_editingFinished();

	void on_lineEditNodeY_editingFinished();

	void on_checkBoxSupplyPipe_clicked();

	void on_pushButtonSizePipeDimensions_clicked();

	void on_pushButtonGenerateIntersections_clicked();

	void on_pushButtonConnectBuildings_clicked();

	void on_pushButtonReduceDeadEnds_clicked();

	void on_pushButtonReduceRedundantNodes_clicked();

	void on_lineEditHeatFlux_editingFinished();

	void on_lineEditNodeMaxHeatingDemand_editingFinished();

	void on_horizontalSliderScaleNodes_valueChanged(int value);

	void on_horizontalSliderScaleEdges_valueChanged(int value);

	void on_pushButtonSelectPipes_clicked();

	void on_lineEditNodeDisplayName_editingFinished();

	void on_lineEditEdgeDisplayName_editingFinished();

	void on_lineEditTemperature_editingFinished();

	void on_heatExchangeDataFile_editingFinished();

	void on_comboBoxHeatExchangeType_activated(int index);

	void on_lineEditHXTransferCoefficient_editingFinished();

	void on_pushButtonSelectFluid_clicked();

	void on_pushButtonAssignPipe_clicked();

	void on_pushButtonEditPipe_clicked();

	void on_pushButtonEditSubNetworks_clicked();

	void on_pushButtonAssignSubNetwork_clicked();

	void on_pushButtonTempChangeIndicator_clicked();

	void on_pushButtonRemoveSmallEdge_clicked();

	void on_pushButtonRecalculateLength_clicked();

	void on_pushButtonDeleteNetwork_clicked();

private:

	/*! Update information related to one or multiple nodes
	 */
	void updateNodeProperties();

	/*! Update information related to one or multiple edges
	 */
	void updateEdgeProperties();

	/*! Update all information that can be updated when knowing only the network,
	 * no edge/node need to be selected here
	 */
	void updateNetworkProperties();

	void updateHeatExchangeProperties();

	void modifySizingParams();

	bool setNetwork();

	void clearUI();

	void setAllEnabled(bool enabled);

	QString largestDiameter() const;

	QString smallestDiameter() const;

	void modifyHeatExchangeProperties();

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
