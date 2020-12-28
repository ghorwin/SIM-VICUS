#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"
#include "SVProjectHandler.h"

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

	// setup combobox pipe models
	m_mapPipeModels.clear();
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

	// setup comobox components
	m_mapComponents.clear();
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_Radiator),
														NANDRAD::HydraulicNetworkComponent::MT_Radiator);
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger),
														NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger);
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_HeatPump),
														NANDRAD::HydraulicNetworkComponent::MT_HeatPump);
	m_mapComponents.insert(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::modelType_t",
														NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel),
														NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel);
	m_ui->comboBoxComponent->clear();
	m_ui->comboBoxComponent->addItems(m_mapComponents.keys());

	// setup combobox heat exchange type
	m_mapHeatExchangeType.clear();
	m_mapHeatExchangeType.insert("None", NANDRAD::HydraulicNetworkComponent::NUM_HT);
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
	m_ui->comboBoxHeatExchangeType->setCurrentIndex(NANDRAD::HydraulicNetworkComponent::NUM_HT);
}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::updateUi() {

	// update network ui
	networkFromId();
	if (m_network != nullptr){
		m_ui->labelEdgeCount->setText(QString("%1").arg(m_network->m_edges.size()));
		m_ui->labelNodeCount->setText(QString("%1").arg(m_network->m_nodes.size()));
		if (m_network->checkConnectedGraph()){
			m_ui->labelNetworkConnected->setText("Network is connected");
			m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: green}");
		}
		else{
			m_ui->labelNetworkConnected->setText("Network is unconnected");
			m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: red}");
		}
		m_ui->labelTotalLength->setText(QString("%1 m").arg(m_network->totalLength()));
		m_ui->pushButtonConnectBuildings->setEnabled(m_network->nextUnconnectedBuilding()>=0);
		m_ui->pushButtonReduceDeadEnds->setEnabled(m_network->checkConnectedGraph() && m_network->numberOfBuildings() > 0);
		m_ui->labelLargestDiameter->setText(QString("%1 mm").arg(m_network->largestDiameter()));
		m_ui->labelSmallestDiameter->setText(QString("%1 mm").arg(m_network->smallestDiameter()));

		m_ui->horizontalSliderScaleEdges->setValue(m_network->m_scaleEdges);
		m_ui->horizontalSliderScaleNodes->setValue(m_network->m_scaleNodes);

		updateSizingParams();
	}

	// update edge ui
	const VICUS::NetworkEdge *edge = dynamic_cast<const VICUS::NetworkEdge *>(m_obj);
	if (edge != nullptr){
		m_ui->labelPipeLength->setText(QString("%1 m").arg(edge->length()));
		m_ui->comboBoxPipeModel->setCurrentText(m_mapPipeModels.key(edge->m_modelType));
		setupComboboxPipeDB();
		m_ui->comboBoxPipeDB->setCurrentText(m_mapDBPipes.key(edge->m_pipeId));
		m_ui->checkBoxSupplyPipe->setChecked(edge->m_supply);
	}

	// update node ui
	const VICUS::NetworkNode *node = dynamic_cast<const VICUS::NetworkNode *>(m_obj);
	if (node != nullptr){
		m_ui->lineEditNodeHeatingDemand->setEnabled(node->m_type == VICUS::NetworkNode::NT_Building);
		setupComboboxComponents();
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(node->m_componentId));
		m_ui->comboBoxNodeType->setCurrentText(m_mapNodeTypes.key(node->m_type));
		m_ui->labelNodeId->setText(QString("%1").arg(node->m_id));
		m_ui->label->setText(QString("%1").arg(node->m_id));
		m_ui->lineEditNodeHeatingDemand->setValue(node->m_maxHeatingDemand);
		m_ui->lineEditNodeX->setValue(node->m_position.m_x);
		m_ui->lineEditNodeY->setValue(node->m_position.m_y);
	}
}


void SVPropNetworkEditWidget::updateSizingParams() {
	m_ui->doubleSpinBoxTemperatureSetpoint->setValue(m_network->m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].get_value(IBK::Unit("C")));
	m_ui->doubleSpinBoxTemperatureDifference->setValue(m_network->m_sizingPara[VICUS::Network::SP_TemperatureDifference].value);
	m_ui->doubleSpinBoxMaximumPressureLoss->setValue(m_network->m_sizingPara[VICUS::Network::SP_MaxPressureLoss].value);
}


void SVPropNetworkEditWidget::modifyStatus() {
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network; // Note: keeps unique IDs, but pointers are invalidated!
	network.m_scaleEdges = m_ui->horizontalSliderScaleEdges->value();
	network.m_scaleNodes = m_ui->horizontalSliderScaleNodes->value();
	network.updateNodeEdgeConnectionPointers(); // update pointers, since next function depends on it
	network.updateVisualizationData(); // update visualization-related properties in network
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push(); // modifies project and updates views
}

void SVPropNetworkEditWidget::modifySizingParams()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network;
	network.m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].set(VICUS::KeywordList::Keyword("Network::SizingParam", VICUS::Network::SP_TemperatureSetpoint),
																	   m_ui->doubleSpinBoxTemperatureSetpoint->value(),
																	   IBK::Unit("C"));
	VICUS::KeywordList::setParameter(network.m_sizingPara, "Network::SizingParam", VICUS::Network::SP_TemperatureDifference,
									 m_ui->doubleSpinBoxTemperatureDifference->value());
	VICUS::KeywordList::setParameter(network.m_sizingPara, "Network::SizingParam", VICUS::Network::SP_MaxPressureLoss,
									 m_ui->doubleSpinBoxMaximumPressureLoss->value());
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::modifyEdgeProperties()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network;
	const VICUS::NetworkEdge *edge = dynamic_cast<const VICUS::NetworkEdge *>(m_obj);
	if (edge != nullptr){
		VICUS::NetworkEdge *e = network.edge(edge->nodeId1(), edge->nodeId2());
		e->m_supply = m_ui->checkBoxSupplyPipe->isChecked();
		e->m_modelType = NANDRAD::HydraulicNetworkComponent::modelType_t(m_mapPipeModels.value(m_ui->comboBoxPipeModel->currentText()));
		e->m_pipeId = m_mapDBPipes.value(m_ui->comboBoxPipeDB->currentText());
	}
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::modifyNodeProperties()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network;
	const VICUS::NetworkNode *node = dynamic_cast<const VICUS::NetworkNode *>(m_obj);
	if (node != nullptr){
		VICUS::NetworkNode *n = &network.m_nodes[node->m_id];
		n->m_type = VICUS::NetworkNode::NodeType(m_mapNodeTypes.value(m_ui->comboBoxNodeType->currentText()));
		n->m_componentId = NANDRAD::HydraulicNetworkComponent::modelType_t(m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
//		n->m_heatExchangePara = NANDRAD::HydraulicNetworkComponent::heatExchangeType_t(
//					m_mapHeatExchangeType.value(m_ui->comboBoxHeatExchangeType->currentText()));
		if (m_ui->lineEditNodeHeatingDemand->isValid())
			n->m_maxHeatingDemand = m_ui->lineEditNodeHeatingDemand->value();
		if (m_ui->lineEditNodeX->isValid())
			n->m_position.m_x = m_ui->lineEditNodeX->value();
		if (m_ui->lineEditNodeY->isValid())
			n->m_position.m_y = m_ui->lineEditNodeY->value();
//		if (m_ui->lineEditNodeZ->isValid())
//			n->m_position.m_z = m_ui->lineEditNodeZ->value();
	}
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::setupComboboxPipeDB()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	m_mapDBPipes.clear();
	for (auto it = m_network ->m_networkPipeDB.begin(); it!= m_network ->m_networkPipeDB.end(); ++it){
		m_mapDBPipes.insert(QString::fromStdString(""+IBK::FormatString("%1 [%2 mm]").arg(it->m_displayName)
												   .arg(it->m_diameterOutside)), it->m_id);
	}
	m_ui->comboBoxPipeDB->clear();
	m_ui->comboBoxPipeDB->addItems(m_mapDBPipes.keys());
}


void SVPropNetworkEditWidget::setupComboboxComponents()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	m_mapComponents.clear();
	for (const NANDRAD::HydraulicNetworkComponent &comp: m_network->m_hydraulicComponents)
		m_mapComponents.insert(QString::fromStdString(comp.m_displayName), comp.m_id);
	m_ui->comboBoxComponent->clear();
	m_ui->comboBoxComponent->addItems(m_mapComponents.keys());
}


void SVPropNetworkEditWidget::networkFromId()
{
	unsigned int id = SVViewStateHandler::instance().m_navigationTreeWidget->selectedNodeID();
//	SVViewStateHandler::instance().m_navigationTreeWidget->ch
	if (id != 0)
		m_treeItemId = id;
	const VICUS::Project &p = project();
	m_obj = p.objectById(m_treeItemId);
	if (dynamic_cast<const VICUS::Network *>(m_obj) != nullptr)
		m_network = dynamic_cast<const VICUS::Network *>(m_obj);
	else if (dynamic_cast<const VICUS::Network *>(m_obj->m_parent) != nullptr)
		m_network = dynamic_cast<const VICUS::Network *>(m_obj->m_parent);
	else
		return; /// TODO: undefined state - m_network remains old value... this is unsafe. Probably set to nullptr or
				/// handle as assert
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

void SVPropNetworkEditWidget::on_doubleSpinBoxNodeNeatingDemand_editingFinished()
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
	networkFromId();
	if (m_network == nullptr)
		return;
	const VICUS::Project &p = project();
	const VICUS::NetworkFluid * fluid = p.element(p.m_networkFluids, m_network->m_fluidID);
	if (fluid == nullptr)
		throw IBK::Exception(IBK::FormatString("Could not find fluid with id %1 in fluid database")
							.arg(m_network->m_fluidID), FUNC_ID);
	VICUS::Network network = *m_network;
	network.updateNodeEdgeConnectionPointers();
	network.sizePipeDimensions(fluid);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push(); // modifies project and updates views
	updateUi();
}

void SVPropNetworkEditWidget::on_pushButtonGenerateIntersections_clicked()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network;
	network.updateNodeEdgeConnectionPointers();
	network.generateIntersections();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push(); // modifies project and updates views
	updateUi();
}

void SVPropNetworkEditWidget::on_pushButtonConnectBuildings_clicked()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network;
	network.updateNodeEdgeConnectionPointers();
	network.connectBuildings(false);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push(); // modifies project and updates views
	updateUi();
}

void SVPropNetworkEditWidget::on_pushButtonReduceDeadEnds_clicked()
{
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network;
	network.updateNodeEdgeConnectionPointers();
	VICUS::Network tmp = network;
	tmp.clear();
	network.cleanDeadEnds(tmp);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), tmp);
	undo->push(); // modifies project and updates views
	updateUi();
}

void SVPropNetworkEditWidget::on_pushButtonReduceRedundantNodes_clicked()
{
	networkFromId();
	if (m_network == nullptr)
		return;

	// set current network invisible
	VICUS::Network network = *m_network;
	network.updateNodeEdgeConnectionPointers();
	network.m_visible = false;
	SVUndoModifyExistingNetwork * undoMod = new SVUndoModifyExistingNetwork(tr("mod network"), network);
	undoMod->push(); // modifies project and updates views

	// make copy with reduced edges
	VICUS::Network copy = network;
	copy.clear();
	network.cleanRedundantEdges(copy);
	copy.m_visible = true;
	copy.m_name += "_reduced";
	const VICUS::Project & p = project();
	copy.m_id = p.uniqueId(p.m_geometricNetworks);
	copy.updateNodeEdgeConnectionPointers();

	SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("modified network"), copy);
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
	bool hasHX = NANDRAD::HydraulicNetworkComponent::hasHeatExchange(
				NANDRAD::HydraulicNetworkComponent::modelType_t(m_mapHeatExchangeType.value(arg1)));
	m_ui->groupBoxHeatExchange->setVisible(hasHX);
	modifyNodeProperties();
}
