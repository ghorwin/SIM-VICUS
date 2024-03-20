#ifndef SVPROPNETWORKEDITWIDGETH
#define SVPROPNETWORKEDITWIDGETH

#include <QWidget>

#include <VICUS_utilities.h>
#include <VICUS_Project.h>

#include "SVUndoModifyNetwork.h"
#include "SVProjectHandler.h"

namespace VICUS {
	class NetworkEdge;
	class NetworkNode;
	class Network;
}

class SVPropNetworkNodesWidget;
class SVPropNetworkEdgesWidget;
class SVPropNetworkSubStationWidget;
class SVPropNetworkHeatExchangeWidget;
class SVPropNetworkGeometryWidget;

namespace Ui {
class SVPropNetworkEditWidget;
}

/*!
 *  This class owns the ToolBox and pointers to all network editing widgets, which are used in the ToolBox. It also provides them with the currently selected network, edges and nodes.
 *  It reacts on project changes (and selection changes), updates the network editing widgets and also updates th color view.
 *  The template function are convenience functions that are used by the editing widgets.
 */

class SVPropNetworkEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SVPropNetworkEditWidget(QWidget *parent = nullptr);
	~SVPropNetworkEditWidget();

	/*! modifies the given property of selected node(s).
	 * Encapsulates the process of retrieving the according node and conducting the undo */
	template<typename TNodeProp, typename Tval>
	void modifyNodeProperty(TNodeProp property, const Tval &value) {
		VICUS::Project p = project();
		if (p.m_activeNetworkId == VICUS::INVALID_ID || m_currentNodes.empty())
			return;
		VICUS::Network *network = VICUS::element(p.m_geometricNetworks, p.m_activeNetworkId);
		Q_ASSERT(network!=nullptr);

		for (const VICUS::NetworkNode * nodeConst: m_currentNodes)
			network->nodeById(nodeConst->m_id)->*property = value;

		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), *network);
		undo->push(); // modifies project and updates views
	}

	/*! modifies the given property of selected edge(s).
	 * Encapsulates the process of retrieving the according edge and conducting the undo */
	template <typename TEdgeProp, typename Tval>
	void modifyEdgeProperty(TEdgeProp property, const Tval & value) {
		VICUS::Project p = project();
		if (p.m_activeNetworkId == VICUS::INVALID_ID || m_currentEdges.empty())
			return;
		VICUS::Network *network = VICUS::element(p.m_geometricNetworks, p.m_activeNetworkId);
		Q_ASSERT(network!=nullptr);

		for (const VICUS::NetworkEdge * edgeConst: m_currentEdges)
			network->edgeById(edgeConst->m_id)->*property = value;

		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), *network);
		undo->push(); // modifies project and updates views
		return;
	}

	/*! determines wether the given property of the vector of objects is constant for all elements in the vector */
	template <typename Telem, typename Tprop>
	static bool uniformProperty(const std::vector<Telem *> & vec, Tprop property) {
		if (vec.empty())
			return false;
		typename std::vector<Telem *>::const_iterator itFirst = vec.begin();
		for (typename std::vector<Telem *>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
			if ((*itFirst)->*property != (*it)->*property)
				return false;
		}
		return true;
	}

	/*! Returns current property type (index of toolbox) */
	unsigned int currentPropertyType();

	/*! Switches property type to selected mode */
	void setPropertyType(int propertyType);

	/*! The currently selected edges */
	std::vector<const VICUS::NetworkEdge *> m_currentEdges;

	/*! The currently selected nodes */
	std::vector<const VICUS::NetworkNode *> m_currentNodes;

	/*! True if the current selection belongs to the active network (which shall be edited). If so editing shall be possible.*/
	bool									m_activeNetworkSelected = false;

public slots:
	void onModified(int modificationType);

	/*! Connects to the ToolBox and changes the color view */
	void onPropertyTypeChanged(int propertyType);

	void onStyleChanged();

private slots:
	void on_comboBoxCurrentNetwork_activated(int);

	void on_toolButtonAssignToCurrent_clicked();

	void on_checkBoxShowAllNetworks_clicked();

private:

	/*! Filters network objects from currently selected objects and stores their pointers. */
	void findSelectedObjects();

	/*! Populates the combobox with all available networks */
	void updateComboBoxNetworks();

	Ui::SVPropNetworkEditWidget		*m_ui;

	SVPropNetworkGeometryWidget		*m_geometryWidget = nullptr;
	SVPropNetworkNodesWidget		*m_nodesWidget = nullptr;
	SVPropNetworkEdgesWidget		*m_edgesWidget = nullptr;
	SVPropNetworkSubStationWidget	*m_subStationWidget = nullptr;
	SVPropNetworkHeatExchangeWidget *m_hxWidget = nullptr;

};



#endif // SVPROPNETWORKEDITWIDGETH
