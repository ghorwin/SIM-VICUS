#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include <QMessageBox>

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"
#include "SVProjectHandler.h"
#include "SVDialogHydraulicComponents.h"
#include "Vic3DWireFrameObject.h"

#include <NANDRAD_HydraulicNetworkComponent.h>

#include <VICUS_KeywordList.h>

SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);

	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxHeatExchange->setEnabled(false);

	// setup combobox node types
	m_mapNodeTypes.clear();
	m_mapNodeTypes.insert(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
						  VICUS::NetworkNode::NT_Mixer);
	m_mapNodeTypes.insert(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
						  VICUS::NetworkNode::NT_Source);
	m_mapNodeTypes.insert(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Building),
						  VICUS::NetworkNode::NT_Building);
	m_ui->comboBoxNodeType->clear();
	m_ui->comboBoxNodeType->addItems(m_mapNodeTypes.keys());
}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::updateUi(const SelectionState selectionState) {
	m_selection = selectionState;
	switch (m_selection) {
		case S_SingleObject: {// get single object from currently selected node in tree widget
			m_treeItemId = SVViewStateHandler::instance().m_navigationTreeWidget->selectedNodeID();
			break;
		}
		case S_MultipleObjects: {// get multiple selected objects (checked nodes in tree widget or selected in 3d scene)
			m_selectedObjects = SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects;
			break;
		}
		default: return;
	}
	// TODO Hauke: check if it is a mixed selection, if so show notification and no updates

	updateNetworkProperties();
	updateNodeProperties();
	updateEdgeProperties();
	updateSizingParams();
}


void SVPropNetworkEditWidget::updateNodeProperties()
{
	std::vector<const VICUS::NetworkNode *> nodes = currentNetworkNodes();
	if (nodes[0] == nullptr)
		return;

	setupComboBoxComponents();

	// if node type is not uniform, no editing will be allowed
	bool uniformNodeType = uniformProperty(nodes, &VICUS::NetworkNode::m_type);
	m_ui->groupBoxNode->setEnabled(uniformNodeType);
	m_ui->groupBoxComponent->setEnabled(uniformNodeType);
	m_ui->comboBoxNodeType->setCurrentText(m_mapNodeTypes.key(nodes[0]->m_type));
	m_ui->lineEditNodeHeatingDemand->setEnabled(nodes[0]->m_type == VICUS::NetworkNode::NT_Building);

	if (nodes.size() == 1){
		m_ui->labelNodeId->setText(QString("%1").arg(nodes[0]->m_id));
		m_ui->lineEditNodeX->setValue(nodes[0]->m_position.m_x);
		m_ui->lineEditNodeY->setValue(nodes[0]->m_position.m_y);
	}
	else{
		m_ui->labelNodeId->setText("");
		m_ui->lineEditNodeX->clear();
		m_ui->lineEditNodeY->clear();
	}

	if (uniformProperty(nodes, &VICUS::NetworkNode::m_maxHeatingDemand))
		m_ui->lineEditNodeHeatingDemand->setValue(nodes[0]->m_maxHeatingDemand);
	else
		m_ui->lineEditNodeHeatingDemand->clear();

	if (uniformProperty(nodes, &VICUS::NetworkNode::m_componentId)){
		QString test = m_mapComponents.key(nodes[0]->m_componentId);
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(nodes[0]->m_componentId));
		const VICUS::Network * network = currentNetwork();
		const NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(network->m_hydraulicComponents,
																					 nodes[0]->m_componentId);
		if (comp != nullptr)
			m_ui->groupBoxHeatExchange->setEnabled(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(comp->m_modelType)
							&& comp->m_heatExchangeType != NANDRAD::HydraulicNetworkComponent::NUM_HT);
		else
			m_ui->groupBoxHeatExchange->setEnabled(false);
	}
	else{
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(VICUS::INVALID_ID));
	}
}


void SVPropNetworkEditWidget::updateEdgeProperties()
{
	std::vector<const VICUS::NetworkEdge *> edges = currentNetworkEdges();
	if (edges[0] == nullptr)
		return;

	setupComboBoxComponents();
	setupComboboxPipeDB();

	if (edges.size() == 1)
		m_ui->labelPipeLength->setText(QString("%1 m").arg(edges[0]->length()));
	else
		m_ui->labelPipeLength->setText("");

	if (uniformProperty(edges, &VICUS::NetworkEdge::m_pipeId))
		m_ui->comboBoxPipeDB->setCurrentText(m_mapDBPipes.key(edges[0]->m_pipeId));
	else
		m_ui->comboBoxPipeDB->setCurrentText(m_mapDBPipes.key(VICUS::INVALID_ID));

	if (uniformProperty(edges, &VICUS::NetworkEdge::m_supply))
		m_ui->checkBoxSupplyPipe->setChecked(edges[0]->m_supply);
	else
		m_ui->checkBoxSupplyPipe->setCheckState(Qt::CheckState::PartiallyChecked);

	if (uniformProperty(edges, &VICUS::NetworkEdge::m_componentId)){
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(edges[0]->m_componentId));
		const VICUS::Network * network = currentNetwork();
		const NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(network->m_hydraulicComponents,
																					 edges[0]->m_componentId);
		if (comp != nullptr)
			m_ui->groupBoxHeatExchange->setEnabled(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(comp->m_modelType)
							&& comp->m_heatExchangeType != NANDRAD::HydraulicNetworkComponent::NUM_HT);
		else
			m_ui->groupBoxHeatExchange->setEnabled(false);
	}
	else{
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(VICUS::INVALID_ID));
	}

//	m_ui->comboBoxHeatExchangeType->setCurrentText(m_mapHeatExchangeType.key(edge->m_heatExchangeType));
//	m_ui->comboBoxHeatExchangeType->setEnabled(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(edge->m_modelType));
//	m_ui->groupBoxHeatExchange->setEnabled(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(edge->m_modelType)
//										   && edge->m_heatExchangeType != NANDRAD::HydraulicNetworkComponent::NUM_HT);
//	m_ui->lineEditHeatFlux->setValue(edge->m_heatExchangePara[NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant].value);
}


void SVPropNetworkEditWidget::updateNetworkProperties()
{
	const VICUS::Network * network = currentNetwork();
	if (network == nullptr)
		return;
	m_ui->labelEdgeCount->setText(QString("%1").arg(network->m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(network->m_nodes.size()));
	if (network->checkConnectedGraph()){
		m_ui->labelNetworkConnected->setText("Network is connected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: green}");
	}
	else{
		m_ui->labelNetworkConnected->setText("Network is unconnected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: red}");
	}
	m_ui->labelTotalLength->setText(QString("%1 m").arg(network->totalLength()));
	m_ui->pushButtonConnectBuildings->setEnabled(network->nextUnconnectedBuilding()>=0);
	m_ui->pushButtonReduceDeadEnds->setEnabled(network->checkConnectedGraph() && network->numberOfBuildings() > 0);
	m_ui->labelLargestDiameter->setText(QString("%1 mm").arg(network->largestDiameter()));
	m_ui->labelSmallestDiameter->setText(QString("%1 mm").arg(network->smallestDiameter()));

	m_ui->horizontalSliderScaleEdges->setValue(network->m_scaleEdges);
	m_ui->horizontalSliderScaleNodes->setValue(network->m_scaleNodes);
}


void SVPropNetworkEditWidget::updateSizingParams() {
	const VICUS::Network * network = currentNetwork();
	if (network != nullptr){
		m_ui->doubleSpinBoxTemperatureSetpoint->setValue(network->m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].get_value(IBK::Unit("C")));
		m_ui->doubleSpinBoxTemperatureDifference->setValue(network->m_sizingPara[VICUS::Network::SP_TemperatureDifference].value);
		m_ui->doubleSpinBoxMaximumPressureLoss->setValue(network->m_sizingPara[VICUS::Network::SP_MaxPressureLoss].value);
	}
}


void SVPropNetworkEditWidget::setupComboboxPipeDB()
{
	const VICUS::Network * network = currentNetwork();
	if (network == nullptr)
		return;
	m_mapDBPipes.clear();
	m_mapDBPipes.insert("", VICUS::INVALID_ID);
	for (auto it = network->m_networkPipeDB.begin(); it!= network->m_networkPipeDB.end(); ++it){
		m_mapDBPipes.insert(QString::fromStdString(""+IBK::FormatString("%1 [%2 mm]").arg(it->m_displayName)
												   .arg(it->m_diameterOutside)), it->m_id);
	}
	m_ui->comboBoxPipeDB->clear();
	m_ui->comboBoxPipeDB->addItems(m_mapDBPipes.keys());
}


void SVPropNetworkEditWidget::setupComboBoxComponents()
{
	const VICUS::Network * network = currentNetwork();
	if (network == nullptr)
		return;
	m_mapComponents.clear();
	m_mapComponents.insert("", VICUS::INVALID_ID);
	for (const NANDRAD::HydraulicNetworkComponent &comp: network->m_hydraulicComponents)
		m_mapComponents.insert(QString::fromStdString(comp.m_displayName), comp.m_id);
	m_ui->comboBoxComponent->clear();
	m_ui->comboBoxComponent->addItems(m_mapComponents.keys());
}


void SVPropNetworkEditWidget::modifyStatus() {
	if (!setNetwork())
		return;
	m_network.m_scaleEdges = m_ui->horizontalSliderScaleEdges->value();
	m_network.m_scaleNodes = m_ui->horizontalSliderScaleNodes->value();
	m_network.updateNodeEdgeConnectionPointers(); // update pointers, since next function depends on it
	m_network.updateVisualizationData(); // update visualization-related properties in network
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::modifySizingParams()
{
	if (!setNetwork())
		return;
	m_network.m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].set(VICUS::KeywordList::Keyword("Network::SizingParam", VICUS::Network::SP_TemperatureSetpoint),
																	   m_ui->doubleSpinBoxTemperatureSetpoint->value(),
																	   IBK::Unit("C"));
	VICUS::KeywordList::setParameter(m_network.m_sizingPara, "Network::SizingParam", VICUS::Network::SP_TemperatureDifference,
									 m_ui->doubleSpinBoxTemperatureDifference->value());
	VICUS::KeywordList::setParameter(m_network.m_sizingPara, "Network::SizingParam", VICUS::Network::SP_MaxPressureLoss,
									 m_ui->doubleSpinBoxMaximumPressureLoss->value());
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
}


bool SVPropNetworkEditWidget::setNetwork()
{
	const VICUS::Network * networkC = currentNetwork();
	if (networkC == nullptr)
		return false;
	VICUS::Project p = project();
	m_network = *p.element(p.m_geometricNetworks, networkC->m_id);
	return true;
}


const VICUS::Network * SVPropNetworkEditWidget::currentNetwork()
{
	if (m_selection == S_SingleObject){
		const VICUS::Project & p = project();
		const VICUS::Object * obj = p.objectById(m_treeItemId);
		if (dynamic_cast<const VICUS::Network *>(obj) != nullptr)
			return dynamic_cast<const VICUS::Network *>(obj);
		else if (dynamic_cast<const VICUS::Network *>(obj->m_parent) != nullptr)
			return dynamic_cast<const VICUS::Network *>(obj->m_parent);
	}
	else if(m_selection == S_MultipleObjects){
		Q_ASSERT(!m_selectedObjects.empty());
		for (const VICUS::Object * obj : m_selectedObjects) {
			if (dynamic_cast<const VICUS::Network *>(obj) != nullptr)
				return dynamic_cast<const VICUS::Network *>(obj);
			else if (dynamic_cast<const VICUS::Network *>(obj->m_parent) != nullptr)
				return dynamic_cast<const VICUS::Network *>(obj->m_parent);
		}
	}

	return nullptr;
}


std::vector<const VICUS::NetworkEdge *> SVPropNetworkEditWidget::currentNetworkEdges()
{
	if (m_selection == S_SingleObject){
		const VICUS::Project & p = project();
		const VICUS::Object * obj = p.objectById(m_treeItemId);
		return {dynamic_cast<const VICUS::NetworkEdge *>(obj)};
	}
	else if(m_selection == S_MultipleObjects){
		std::vector <const VICUS::NetworkEdge *> currentEdges;
		for (const VICUS::Object * obj : m_selectedObjects) {
			const VICUS::NetworkEdge * edge = dynamic_cast<const VICUS::NetworkEdge *>(obj);
			if (edge != nullptr)
				currentEdges.push_back(edge);
		}
		if (!currentEdges.empty())
			return currentEdges;
	}
	return {nullptr};
}


std::vector <const VICUS::NetworkNode *> SVPropNetworkEditWidget::currentNetworkNodes()
{
	if (m_selection == S_SingleObject){
		const VICUS::Project & p = project();
		const VICUS::Object * obj = p.objectById(m_treeItemId);
		return {dynamic_cast<const VICUS::NetworkNode *>(obj)};
	}
	else if(m_selection == S_MultipleObjects){
		std::vector <const VICUS::NetworkNode *> currentNodes;
		for (const VICUS::Object * obj : m_selectedObjects) {
			const VICUS::NetworkNode * node = dynamic_cast<const VICUS::NetworkNode *>(obj);
			if (node != nullptr)
				currentNodes.push_back(node);
		}
		if (!currentNodes.empty())
			return currentNodes;
	}
	return {nullptr};
}


void SVPropNetworkEditWidget::showNetworkProperties()
{
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(true);
	m_ui->groupBoxEditNetwork->setVisible(true);
}

void SVPropNetworkEditWidget::showNodeProperties()
{
	m_ui->groupBoxNode->setVisible(true);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
}

void SVPropNetworkEditWidget::showEdgeProperties()
{
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(true);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
}

void SVPropNetworkEditWidget::on_comboBoxNodeType_activated(int index)
{
	modifyNodeProperty(&VICUS::NetworkNode::m_type, VICUS::NetworkNode::NodeType(
						   m_mapNodeTypes.value(m_ui->comboBoxNodeType->currentText())));
}

void SVPropNetworkEditWidget::on_lineEditNodeX_editingFinished()
{
	if (!m_ui->lineEditNodeX->isValid() || !m_ui->lineEditNodeY->isValid())
		return;
	IBKMK::Vector3D	vec(m_ui->lineEditNodeX->value(), m_ui->lineEditNodeY->value(), 0);
	modifyNodeProperty(&VICUS::NetworkNode::m_position, vec);
}

void SVPropNetworkEditWidget::on_lineEditNodeY_editingFinished()
{
	if (!m_ui->lineEditNodeX->isValid() || !m_ui->lineEditNodeY->isValid())
		return;
	IBKMK::Vector3D	vec(m_ui->lineEditNodeX->value(), m_ui->lineEditNodeY->value(), 0);
	modifyNodeProperty(&VICUS::NetworkNode::m_position, vec);
}

void SVPropNetworkEditWidget::on_comboBoxPipeDB_activated(int index)
{
	modifyEdgeProperty(&VICUS::NetworkEdge::m_pipeId, m_mapDBPipes.value(m_ui->comboBoxPipeDB->currentText()));
}

void SVPropNetworkEditWidget::on_checkBoxSupplyPipe_clicked()
{
	modifyEdgeProperty(&VICUS::NetworkEdge::m_supply, m_ui->checkBoxSupplyPipe->isChecked());
}

void SVPropNetworkEditWidget::on_horizontalSliderScaleNodes_actionTriggered(int action)
{
	modifyStatus();
}

void SVPropNetworkEditWidget::on_horizontalSliderScaleEdges_actionTriggered(int action)
{
	modifyStatus();
}

void SVPropNetworkEditWidget::on_comboBoxComponent_activated(const QString &arg1)
{
	// the functions will simply return in case it is not a Node/Edge
	modifyEdgeProperty(&VICUS::NetworkEdge::m_componentId, m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
	modifyNodeProperty(&VICUS::NetworkNode::m_componentId, m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
}

void SVPropNetworkEditWidget::on_pushButtonEditComponents_clicked()
{
	const VICUS::Network * network = currentNetwork();
	std::vector<const VICUS::NetworkNode *> nodes = currentNetworkNodes();
	std::vector<const VICUS::NetworkEdge *> edges = currentNetworkEdges();
	unsigned int componentId = 0;
	if (nodes[0] != nullptr)
		componentId = nodes[0]->m_componentId;
	else if (edges[0] != nullptr)
		componentId = edges[0]->m_componentId;
	else
		return;
	SVDialogHydraulicComponents *dialog = new SVDialogHydraulicComponents(this);
	if (dialog->edit(network->m_id, componentId) == QDialog::Accepted){
		setupComboBoxComponents();
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(dialog->currentComponentId()));
	}
	else{
		setupComboBoxComponents();
	}

	modifyEdgeProperty(&VICUS::NetworkEdge::m_componentId, m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
	modifyNodeProperty(&VICUS::NetworkNode::m_componentId, m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
}

void SVPropNetworkEditWidget::on_lineEditNodeHeatingDemand_editingFinished()
{
	if (m_ui->lineEditNodeHeatingDemand->isValid())
		modifyNodeProperty(&VICUS::NetworkNode::m_maxHeatingDemand, m_ui->lineEditNodeHeatingDemand->value());
}


void SVPropNetworkEditWidget::on_lineEditHeatFlux_editingFinished()
{
}


void SVPropNetworkEditWidget::on_pushButtonSizePipeDimensions_clicked()
{
	FUNCID(SVPropNetworkEditWidget::on_pushButtonSizePipeDimensions_clicked);
	modifySizingParams();
	if (!setNetwork())
		return;
	const VICUS::Project &p = project();
	const VICUS::NetworkFluid * fluid = p.element(p.m_networkFluids, m_network.m_fluidID);
	if (fluid == nullptr)
		throw IBK::Exception(IBK::FormatString("Could not find fluid with id %1 in fluid database")
							.arg(m_network.m_fluidID), FUNC_ID);
	m_network.sizePipeDimensions(fluid);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
	updateSizingParams();
}

void SVPropNetworkEditWidget::on_pushButtonGenerateIntersections_clicked()
{
	if (!setNetwork())
		return;
	m_network.updateNodeEdgeConnectionPointers();
	m_network.generateIntersections();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonConnectBuildings_clicked()
{
	if (!setNetwork())
		return;
	m_network.updateNodeEdgeConnectionPointers();
	m_network.connectBuildings(false);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonReduceDeadEnds_clicked()
{
	if (!setNetwork())
		return;
	VICUS::Network tmp = m_network;
	tmp.clear();
	m_network.updateNodeEdgeConnectionPointers();
	m_network.cleanDeadEnds(tmp);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), tmp);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonReduceRedundantNodes_clicked()
{
	if (!setNetwork())
		return;

	// set current network invisible
	m_network.m_visible = false;
	SVUndoModifyExistingNetwork * undoMod = new SVUndoModifyExistingNetwork(tr("mod network"), m_network);
	undoMod->push(); // modifies project and updates views

	// make copy with reduced edges
	VICUS::Network tmp = m_network;
	tmp.clear();
	m_network.updateNodeEdgeConnectionPointers();
	m_network.cleanRedundantEdges(tmp);
	tmp.m_visible = true;
	tmp.m_name += "_reduced";
	const VICUS::Project & p = project();
	tmp.m_id = p.uniqueId(p.m_geometricNetworks);
	tmp.updateNodeEdgeConnectionPointers();

	SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("modified network"), tmp);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}


template <typename TEdgeProp, typename Tval>
void SVPropNetworkEditWidget::modifyEdgeProperty(TEdgeProp property, const Tval & value)
{
	if (!setNetwork())
		return;
	std::vector<const VICUS::NetworkEdge *> edges = currentNetworkEdges();
	if (edges[0] == nullptr)
		return;
	for (const VICUS::NetworkEdge * edgeConst: edges){
		VICUS::NetworkEdge * edge = m_network.edge(edgeConst->nodeId1(), edgeConst->nodeId2());
		Q_ASSERT(edge != nullptr);
		edge->*property = value;
	}
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified edge"), m_network);
	undo->push(); // modifies project and updates views
	return;
}


template<typename TNodeProp, typename Tval>
void SVPropNetworkEditWidget::modifyNodeProperty(TNodeProp property, const Tval &value)
{
	if (!setNetwork())
		return;
	std::vector<const VICUS::NetworkNode *> nodes = currentNetworkNodes();
	if (nodes[0] == nullptr)
		return;
	for (const VICUS::NetworkNode * nodeConst: nodes){
		m_network.m_nodes[nodeConst->m_id].*property = value;
	}
	m_network.updateNodeEdgeConnectionPointers();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified node"), m_network);
	undo->push(); // modifies project and updates views
}
