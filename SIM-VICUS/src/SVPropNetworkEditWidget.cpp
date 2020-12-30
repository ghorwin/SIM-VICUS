#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"
#include "SVProjectHandler.h"
#include "SVDialogHydraulicComponents.h"

#include "NANDRAD_HydraulicNetworkComponent.h"

#include "VICUS_KeywordList.h"

SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);

	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(false);

	setupComboBoxComponents();

	// setup combobox pipe models
	m_mapPipeModels.clear();
	m_mapPipeModels.insert("<None>", NANDRAD::HydraulicNetworkComponent::NUM_MT);
	m_mapPipeModels.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_StaticPipe),
														NANDRAD::HydraulicNetworkComponent::MT_StaticPipe);
	m_mapPipeModels.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe),
														NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe);
	m_ui->comboBoxPipeModel->clear();
	m_ui->comboBoxPipeModel->addItems(m_mapPipeModels.keys());

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

	// setup combobox heat exchange type
	m_mapHeatExchangeType.clear();
	m_mapHeatExchangeType.insert("<None>", NANDRAD::HydraulicNetworkComponent::NUM_HT);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::heatExchangeType_t",
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant),
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::heatExchangeType_t",
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile),
								NANDRAD::HydraulicNetworkComponent::HT_HeatFluxDataFile);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::heatExchangeType_t",
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature),
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithFMUTemperature);
	m_mapHeatExchangeType.insert(NANDRAD::KeywordList::Description("HydraulicNetworkComponent::heatExchangeType_t",
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithZoneTemperature),
								NANDRAD::HydraulicNetworkComponent::HT_HeatExchangeWithZoneTemperature);
	m_ui->comboBoxHeatExchangeType->clear();
	m_ui->comboBoxHeatExchangeType->addItems(m_mapHeatExchangeType.keys());
	m_ui->comboBoxHeatExchangeType->setCurrentText(m_mapHeatExchangeType.key(NANDRAD::HydraulicNetworkComponent::NUM_HT));

}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::updateUi() {

	// update network ui
	const VICUS::Network * network = currentNetwork();
	if (network != nullptr){
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

		updateSizingParams();
	}

	// update edge ui
	const VICUS::NetworkEdge *edge = currentNetworkEdge();
	if (edge != nullptr){
		setupComboboxPipeDB();
		m_ui->labelPipeLength->setText(QString("%1 m").arg(edge->length()));
		m_ui->comboBoxPipeModel->setCurrentText(m_mapPipeModels.key(edge->m_modelType));
		m_ui->comboBoxPipeDB->setCurrentText(m_mapDBPipes.key(edge->m_pipeId));
		m_ui->checkBoxSupplyPipe->setChecked(edge->m_supply);
		m_ui->groupBoxHeatExchange->setVisible(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(edge->m_modelType));
	}

	// update node ui
	const VICUS::NetworkNode *node = currentNetworkNode();
	if (node != nullptr){
		m_ui->lineEditNodeHeatingDemand->setEnabled(node->m_type == VICUS::NetworkNode::NT_Building);
		m_ui->comboBoxNodeType->setCurrentText(m_mapNodeTypes.key(node->m_type));
		m_ui->labelNodeId->setText(QString("%1").arg(node->m_id));
		m_ui->label->setText(QString("%1").arg(node->m_id));
		m_ui->lineEditNodeHeatingDemand->setValue(node->m_maxHeatingDemand);
		m_ui->lineEditNodeX->setValue(node->m_position.m_x);
		m_ui->lineEditNodeY->setValue(node->m_position.m_y);
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(node->m_componentId));
		const NANDRAD::HydraulicNetworkComponent *comp = VICUS::Project::element(network->m_hydraulicComponents,
																				 node->m_componentId);
		if (comp != nullptr)
			m_ui->groupBoxHeatExchange->setVisible(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(comp->m_modelType));
		else
			m_ui->groupBoxHeatExchange->setVisible(false);
	}
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
	m_mapComponents.insert("<None>", NANDRAD::INVALID_ID);
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


void SVPropNetworkEditWidget::modifyEdgeProperties()
{
	if (!setNetwork())
		return;
	const VICUS::NetworkEdge *edgeConst = currentNetworkEdge();
	if (edgeConst == nullptr)
		return;
	VICUS::NetworkEdge * edge = m_network.edge(edgeConst->nodeId1(), edgeConst->nodeId2());
	edge->m_supply = m_ui->checkBoxSupplyPipe->isChecked();
	edge->m_modelType = NANDRAD::HydraulicNetworkComponent::modelType_t(
				m_mapPipeModels.value(m_ui->comboBoxPipeModel->currentText()));
	edge->m_pipeId = m_mapDBPipes.value(m_ui->comboBoxPipeDB->currentText());
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
	updateUi();
}


void SVPropNetworkEditWidget::modifyNodeProperties()
{
	if (!setNetwork())
		return;
	const VICUS::NetworkNode *nodeConst = currentNetworkNode();
	if (nodeConst == nullptr)
		return;
	VICUS::NetworkNode & node = m_network.m_nodes[nodeConst->m_id];
	node.m_type = VICUS::NetworkNode::NodeType(m_mapNodeTypes.value(m_ui->comboBoxNodeType->currentText()));
	node.m_componentId = m_mapComponents.value(m_ui->comboBoxComponent->currentText());
	if (m_ui->lineEditNodeHeatingDemand->isValid())
		node.m_maxHeatingDemand = m_ui->lineEditNodeHeatingDemand->value();
	if (m_ui->lineEditNodeX->isValid())
		node.m_position.m_x = m_ui->lineEditNodeX->value();
	if (m_ui->lineEditNodeY->isValid())
		node.m_position.m_y = m_ui->lineEditNodeY->value();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
	updateUi();
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
	unsigned int id = SVViewStateHandler::instance().m_navigationTreeWidget->selectedNodeID();
	if (id != 0)
		m_treeItemId = id;
	const VICUS::Project & p = project();
	const VICUS::Object * obj = p.objectById(m_treeItemId);
	if (dynamic_cast<const VICUS::Network *>(obj) != nullptr)
		return dynamic_cast<const VICUS::Network *>(obj);
	else if (dynamic_cast<const VICUS::Network *>(obj->m_parent) != nullptr)
		return dynamic_cast<const VICUS::Network *>(obj->m_parent);
	else
		return nullptr;
}

const VICUS::NetworkEdge *SVPropNetworkEditWidget::currentNetworkEdge()
{
	const VICUS::Project & p = project();
	const VICUS::Object * obj = p.objectById(m_treeItemId);
	return dynamic_cast<const VICUS::NetworkEdge *>(obj);
}

const VICUS::NetworkNode *SVPropNetworkEditWidget::currentNetworkNode()
{
	const VICUS::Project & p = project();
	const VICUS::Object * obj = p.objectById(m_treeItemId);
	return dynamic_cast<const VICUS::NetworkNode *>(obj);
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
	modifyNodeProperties();
}

void SVPropNetworkEditWidget::on_lineEditNodeX_editingFinished()
{
	modifyNodeProperties();
}

void SVPropNetworkEditWidget::on_lineEditNodeY_editingFinished()
{
	modifyNodeProperties();
}

void SVPropNetworkEditWidget::on_comboBoxPipeModel_activated(int index)
{
	modifyEdgeProperties();
}

void SVPropNetworkEditWidget::on_comboBoxPipeDB_activated(int index)
{
	modifyEdgeProperties();
}

void SVPropNetworkEditWidget::on_checkBoxSupplyPipe_clicked()
{
	modifyEdgeProperties();
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
	updateUi();
}

void SVPropNetworkEditWidget::on_pushButtonGenerateIntersections_clicked()
{
	if (!setNetwork())
		return;
	m_network.generateIntersections();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
	updateUi();
}

void SVPropNetworkEditWidget::on_pushButtonConnectBuildings_clicked()
{
	if (!setNetwork())
		return;
	m_network.connectBuildings(false);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
	updateUi();
}

void SVPropNetworkEditWidget::on_pushButtonReduceDeadEnds_clicked()
{
	if (!setNetwork())
		return;
	VICUS::Network tmp = m_network;
	tmp.clear();
	m_network.cleanDeadEnds(tmp);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), tmp);
	undo->push(); // modifies project and updates views
	updateUi();
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
	m_network.cleanRedundantEdges(tmp);
	tmp.m_visible = true;
	tmp.m_name += "_reduced";
	const VICUS::Project & p = project();
	tmp.m_id = p.uniqueId(p.m_geometricNetworks);
	tmp.updateNodeEdgeConnectionPointers();

	SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("modified network"), tmp);
	undo->push(); // modifies project and updates views
	updateUi();
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
	modifyNodeProperties();
}

void SVPropNetworkEditWidget::on_pushButtonEditComponents_clicked()
{
	const VICUS::Network * network = currentNetwork();
	SVDialogHydraulicComponents *dialog = new SVDialogHydraulicComponents(this);
	dialog->edit(network->m_id);
	setupComboBoxComponents();
}

