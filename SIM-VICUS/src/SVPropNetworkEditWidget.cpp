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

	m_ui->labelSelectionInfo->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxComponent->setVisible(false);
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


void SVPropNetworkEditWidget::setPropertyMode(int propertyIndex) {
	qDebug() << "SVPropNetworkEditWidget::setPropertyMode: propertyIndex =" << propertyIndex;

	switch (propertyIndex) {
		case 0 : showNetworkProperties(); break;
		case 1 : showNodeProperties(); break;
		case 2 : showEdgeProperties(); break;
	}

	selectionChanged();
}


void SVPropNetworkEditWidget::selectionChanged() {
	std::set<const VICUS::Object *> objs;
	// get all selected objects of type network, objects must be visible
	project().selectObjects(objs, VICUS::Project::SG_Network, true, true);

	m_currentEdges.clear();
	m_currentNodes.clear();
	std::vector<const VICUS::Network*> networks;

	// cast objects to nodes, edges and network
	for (const VICUS::Object* o : objs) {
		const VICUS::Network * network = dynamic_cast<const VICUS::Network*>(o);
		if (network != nullptr) {
			networks.push_back(network);
			continue;
		}
		network = dynamic_cast<const VICUS::Network*>(o->m_parent);
		if (network != nullptr && std::find(networks.begin(), networks.end(), network) == networks.end()) {
			networks.push_back(network);
		}
		const VICUS::NetworkEdge * edge = dynamic_cast<const VICUS::NetworkEdge*>(o);
		if (edge != nullptr) {
			m_currentEdges.push_back(edge);
			continue;
		}
		const VICUS::NetworkNode * node = dynamic_cast<const VICUS::NetworkNode*>(o);
		if (node != nullptr) {
			m_currentNodes.push_back(node);
			continue;
		}
	}

	// assign current network only if there is exactly one selected,
	// if more than one network was selected: clear all, nothing should be shown or edited
	if (networks.empty())
		m_currentNetwork = nullptr;
	else if (networks.size()>1){
		m_currentNetwork = nullptr;
		m_currentEdges.clear();
		m_currentNodes.clear();
	}
	else
		m_currentNetwork = networks[0];

	// now update UI
	updateNetworkProperties();
	updateNodeProperties();
	updateEdgeProperties();
	updateSizingParams();
}


void SVPropNetworkEditWidget::updateNodeProperties()
{
	if (m_currentNodes.size() == 0){
		// clear all
		m_ui->lineEditNodeHeatingDemand->clear();
		m_ui->labelNodeId->clear();
		m_ui->lineEditNodeX->clear();
		m_ui->lineEditNodeY->clear();
		return;
	}

	setupComboBoxComponents();

	// if node type is not uniform, no editing will be allowed
	bool uniformNodeType = uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_type);
	m_ui->groupBoxNode->setEnabled(uniformNodeType);
	m_ui->groupBoxComponent->setEnabled(uniformNodeType);
	m_ui->comboBoxNodeType->setCurrentText(m_mapNodeTypes.key(m_currentNodes[0]->m_type));
	m_ui->lineEditNodeHeatingDemand->setEnabled(m_currentNodes[0]->m_type == VICUS::NetworkNode::NT_Building);

	if (m_currentNodes.size() == 1){
		m_ui->labelNodeId->setText(QString("%1").arg(m_currentNodes[0]->m_id));
		m_ui->lineEditNodeX->setValue(m_currentNodes[0]->m_position.m_x);
		m_ui->lineEditNodeY->setValue(m_currentNodes[0]->m_position.m_y);
	}
	else{
		m_ui->labelNodeId->setText("");
		m_ui->lineEditNodeX->clear();
		m_ui->lineEditNodeY->clear();
	}

	if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_maxHeatingDemand))
		m_ui->lineEditNodeHeatingDemand->setValue(m_currentNodes[0]->m_maxHeatingDemand);
	else
		m_ui->lineEditNodeHeatingDemand->clear();

	if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_componentId)){
		QString test = m_mapComponents.key(m_currentNodes[0]->m_componentId);
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(m_currentNodes[0]->m_componentId));
		const VICUS::Network * network = m_currentNetwork;
		const NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(network->m_hydraulicComponents,
																					 m_currentNodes[0]->m_componentId);
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


void SVPropNetworkEditWidget::updateEdgeProperties() {

	if (m_currentEdges.size() == 0){
		// clear all
		m_ui->labelPipeLength->clear();
		m_ui->labelPipeLength->clear();
		m_ui->checkBoxSupplyPipe->setChecked(false);
		return;
	}

	setupComboBoxComponents();
	setupComboboxPipeDB();

	if (m_currentEdges.size() == 1)
		m_ui->labelPipeLength->setText(QString("%1 m").arg(m_currentEdges[0]->length()));
	else
		m_ui->labelPipeLength->setText("");

	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_pipeId))
		m_ui->comboBoxPipeDB->setCurrentText(m_mapDBPipes.key(m_currentEdges[0]->m_pipeId));
	else
		m_ui->comboBoxPipeDB->setCurrentText(m_mapDBPipes.key(VICUS::INVALID_ID));

	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_supply))
		m_ui->checkBoxSupplyPipe->setChecked(m_currentEdges[0]->m_supply);
	else
		m_ui->checkBoxSupplyPipe->setCheckState(Qt::CheckState::PartiallyChecked);

	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_componentId)){
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(m_currentEdges[0]->m_componentId));
		const VICUS::Network * network = m_currentNetwork;
		const NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(network->m_hydraulicComponents,
																					 m_currentEdges[0]->m_componentId);
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
	if (m_currentNetwork == nullptr){
		// clear all
		m_ui->labelEdgeCount->setText("");
		m_ui->labelNodeCount->setText("");
		m_ui->labelNetworkConnected->setText("");
		m_ui->labelLargestDiameter->setText("");
		m_ui->labelSmallestDiameter->setText("");
		m_ui->labelTotalLength->setText("");
		return;
	}

	m_ui->labelEdgeCount->setText(QString("%1").arg(m_currentNetwork->m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_currentNetwork->m_nodes.size()));
	if (m_currentNetwork->checkConnectedGraph()){
		m_ui->labelNetworkConnected->setText("Network is connected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: green}");
	}
	else{
		m_ui->labelNetworkConnected->setText("Network is unconnected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: red}");
	}
	m_ui->labelTotalLength->setText(QString("%1 m").arg(m_currentNetwork->totalLength()));
	m_ui->pushButtonConnectBuildings->setEnabled(m_currentNetwork->nextUnconnectedBuilding()>=0);
	m_ui->pushButtonReduceDeadEnds->setEnabled(m_currentNetwork->checkConnectedGraph() && m_currentNetwork->numberOfBuildings() > 0);
	m_ui->labelLargestDiameter->setText(QString("%1 mm").arg(m_currentNetwork->largestDiameter()));
	m_ui->labelSmallestDiameter->setText(QString("%1 mm").arg(m_currentNetwork->smallestDiameter()));

	m_ui->horizontalSliderScaleEdges->setValue(m_currentNetwork->m_scaleEdges);
	m_ui->horizontalSliderScaleNodes->setValue(m_currentNetwork->m_scaleNodes);
}


void SVPropNetworkEditWidget::updateSizingParams() {
	if (m_currentNetwork != nullptr){
		m_ui->doubleSpinBoxTemperatureSetpoint->setValue(m_currentNetwork->m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].get_value(IBK::Unit("C")));
		m_ui->doubleSpinBoxTemperatureDifference->setValue(m_currentNetwork->m_sizingPara[VICUS::Network::SP_TemperatureDifference].value);
		m_ui->doubleSpinBoxMaximumPressureLoss->setValue(m_currentNetwork->m_sizingPara[VICUS::Network::SP_MaxPressureLoss].value);
	}
}


void SVPropNetworkEditWidget::setupComboboxPipeDB()
{
	if (m_currentNetwork == nullptr)
		return;
	m_mapDBPipes.clear();
	m_mapDBPipes.insert("", VICUS::INVALID_ID);
	for (auto it = m_currentNetwork->m_networkPipeDB.begin(); it!= m_currentNetwork->m_networkPipeDB.end(); ++it){
		m_mapDBPipes.insert(QString::fromStdString(""+IBK::FormatString("%1 [%2 mm]").arg(it->m_displayName)
												   .arg(it->m_diameterOutside)), it->m_id);
	}
	m_ui->comboBoxPipeDB->clear();
	m_ui->comboBoxPipeDB->addItems(m_mapDBPipes.keys());
}


void SVPropNetworkEditWidget::setupComboBoxComponents()
{
	const VICUS::Network * network = m_currentNetwork;
	if (network == nullptr)
		return;
	m_mapComponents.clear();
	m_mapComponents.insert("", VICUS::INVALID_ID);
	for (const NANDRAD::HydraulicNetworkComponent &comp: network->m_hydraulicComponents)
		m_mapComponents.insert(QString::fromStdString(comp.m_displayName), comp.m_id);
	m_ui->comboBoxComponent->clear();
	m_ui->comboBoxComponent->addItems(m_mapComponents.keys());
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
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network sizing parameters modified"), networkIndex, m_network);
	undo->push(); // modifies project and updates views
}


bool SVPropNetworkEditWidget::setNetwork() {
	if (m_currentNetwork == nullptr)
		return false;
	VICUS::Project p = project();
	m_network = *p.element(p.m_geometricNetworks, m_currentNetwork->m_id);
	return true;
}


void SVPropNetworkEditWidget::showNetworkProperties() {
	m_ui->labelSelectionInfo->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(true);
	m_ui->groupBoxEditNetwork->setVisible(true);
	m_ui->groupBoxHeatExchange->setVisible(false);
}


void SVPropNetworkEditWidget::showNodeProperties() {
	m_ui->labelSelectionInfo->setVisible(false);
	m_ui->groupBoxNode->setVisible(true);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(true);
}


void SVPropNetworkEditWidget::showEdgeProperties() {
	m_ui->labelSelectionInfo->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(true);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(true);
}


void SVPropNetworkEditWidget::showMixedSelectionInfo() {
	showNetworkProperties();
	m_ui->labelSelectionInfo->setVisible(true);
}


void SVPropNetworkEditWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
		case SVProjectHandler::NetworkModified:
		case SVProjectHandler::AllModified:
		case SVProjectHandler::NodeStateModified:
			selectionChanged(); // updates m_currentXXXX and the UI
		break;

		default: ; // just to make compiler happy
	}
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

void SVPropNetworkEditWidget::on_comboBoxComponent_activated(const QString &arg1)
{
	// the functions will simply return in case it is not a Node/Edge
	modifyEdgeProperty(&VICUS::NetworkEdge::m_componentId, m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
	modifyNodeProperty(&VICUS::NetworkNode::m_componentId, m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
}

void SVPropNetworkEditWidget::on_horizontalSliderScaleNodes_valueChanged(int value)
{
	if (!setNetwork())
		return;
	m_network.m_scaleNodes = value;
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network visualization properties updated"), networkIndex, m_network);
	undo->push(); // modifies project and updates views
}

void SVPropNetworkEditWidget::on_horizontalSliderScaleEdges_valueChanged(int value)
{
	if (!setNetwork())
		return;
	m_network.m_scaleEdges = value;
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network visualization properties updated"), networkIndex, m_network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_pushButtonEditComponents_clicked() {
	const VICUS::Network * network = m_currentNetwork;
	std::vector<const VICUS::NetworkNode *> nodes = m_currentNodes;
	std::vector<const VICUS::NetworkEdge *> edges = m_currentEdges;
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
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_network);
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
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_network);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonConnectBuildings_clicked()
{
	if (!setNetwork())
		return;
	m_network.updateNodeEdgeConnectionPointers();
	m_network.connectBuildings(false);
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_network);
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
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_network);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonReduceRedundantNodes_clicked()
{
	if (!setNetwork())
		return;

	// set current network invisible
	m_network.m_visible = false;
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undoMod = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_network);
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
	const std::vector<const VICUS::NetworkEdge *> & edges = m_currentEdges;
	if (edges[0] == nullptr)
		return;
	for (const VICUS::NetworkEdge * edgeConst: edges){
		VICUS::NetworkEdge * edge = m_network.edge(edgeConst->nodeId1(), edgeConst->nodeId2());
		Q_ASSERT(edge != nullptr);
		edge->*property = value;
	}
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_network);
	undo->push(); // modifies project and updates views
	return;
}


template<typename TNodeProp, typename Tval>
void SVPropNetworkEditWidget::modifyNodeProperty(TNodeProp property, const Tval &value)
{
	if (!setNetwork())
		return;
	std::vector<const VICUS::NetworkNode *> nodes = m_currentNodes;
	if (nodes[0] == nullptr)
		return;
	for (const VICUS::NetworkNode * nodeConst: nodes){
		m_network.m_nodes[nodeConst->m_id].*property = value;
	}
	m_network.updateNodeEdgeConnectionPointers();
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_network);
	undo->push(); // modifies project and updates views
}

