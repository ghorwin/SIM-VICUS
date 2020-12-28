#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"
#include "SVProjectHandler.h"

#include "NANDRAD_HydraulicNetworkComponent.h"

#include "VICUS_KeywordList.h"

#include "QtExt_Locale.h"

SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);

	m_ui->tableViewComponentParams->horizontalHeader()->setVisible(true);

	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(false);

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
	m_mapComponents.insert("<None>", NANDRAD::HydraulicNetworkComponent::NUM_MT);
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
	m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(NANDRAD::HydraulicNetworkComponent::NUM_MT));

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
	m_ui->comboBoxHeatExchangeType->setCurrentText(m_mapHeatExchangeType.key(NANDRAD::HydraulicNetworkComponent::NUM_HT));

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


void SVPropNetworkEditWidget::updateHydraulicComponent()
{
	const VICUS::NetworkNode *node = dynamic_cast<const VICUS::NetworkNode *>(m_obj);
	if (node == nullptr )
		return;
	NANDRAD::HydraulicNetworkComponent component;
	const NANDRAD::HydraulicNetworkComponent *tmpComp = VICUS::Project::element(m_network->m_hydraulicComponents, node->m_componentId);
	if (tmpComp == nullptr)
		return;
	component = *tmpComp;

	m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(component.m_modelType));
	m_ui->groupBoxHeatExchange->setVisible(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(component.m_modelType));
	m_ui->comboBoxHeatExchangeType->setCurrentText(m_mapHeatExchangeType.key(component.m_heatExchangeType));
	// ... hx parameter

	// Create component parameter model and connect model to table view:
	m_componentParModel = new ComponentParameterModel(this);
	connect(m_componentParModel, SIGNAL(editCompleted(void)), this, SLOT(on_componentParModel_editCompleted(void)));
	m_componentParModel->setComponent(component);
	m_ui->tableViewComponentParams->setModel(m_componentParModel);
	m_ui->tableViewComponentParams->show();
}


void SVPropNetworkEditWidget::modifyHydraulicComponent()
{
	FUNCID(SVPropNetworkEditWidget::modifyHydraulicComponent);

	// get network
	networkFromId();
	if (m_network == nullptr)
		return;
	VICUS::Network network = *m_network;
	network.updateNodeEdgeConnectionPointers();

	// get node
	const VICUS::NetworkNode *nodeConst = dynamic_cast<const VICUS::NetworkNode *>(m_obj);
	if (nodeConst == nullptr )
		return;
	VICUS::NetworkNode *node = &network.m_nodes[nodeConst->m_id];

	// check if this node has a component id already, and get the according component
	NANDRAD::HydraulicNetworkComponent component;
	if (nodeConst->m_componentId != VICUS::INVALID_ID){
		const NANDRAD::HydraulicNetworkComponent *tmpComp = VICUS::Project::element(m_network->m_hydraulicComponents, nodeConst->m_componentId);
		if (tmpComp == nullptr)
			throw IBK::Exception(IBK::FormatString("Component with id %1 not found in hyraulic components")
								 .arg(nodeConst->m_componentId), FUNC_ID);
		component = *tmpComp;
	}

	// this makes sure, we have no components in the catalog that dont belong to any node
	network.cleanHydraulicComponentCatalog();

	// read model type and parameter
	component.m_modelType = NANDRAD::HydraulicNetworkComponent::modelType_t(
				m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
	if (component.m_modelType == NANDRAD::HydraulicNetworkComponent::NUM_MT)
		return;

	m_componentParModel->getComponentParameter(component.m_para);

	component.m_heatExchangeType = NANDRAD::HydraulicNetworkComponent::heatExchangeType_t(
				m_mapHeatExchangeType.value(m_ui->comboBoxHeatExchangeType->currentText()));
	// hx params ...

	// if the same component is already in the catalog, set node componentId accordingly
	for (const NANDRAD::HydraulicNetworkComponent &exComponent: m_network->m_hydraulicComponents){
		if (component == exComponent){
			node->m_componentId = exComponent.m_id;
			return;
		}
	}

	// if we have obtained a component with new properties, we get a new id for it and add it to the catalog
	component.m_id = VICUS::Project::uniqueId(m_network->m_hydraulicComponents);
	network.m_hydraulicComponents.push_back(component);
	node->m_componentId = component.m_id;
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), network);
	undo->push();



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
	const VICUS::NetworkEdge *edgeConst = dynamic_cast<const VICUS::NetworkEdge *>(m_obj);
	if (edgeConst != nullptr){
		VICUS::NetworkEdge *edge = network.edge(edgeConst->nodeId1(), edgeConst->nodeId2());
		edge->m_supply = m_ui->checkBoxSupplyPipe->isChecked();
		edge->m_modelType = NANDRAD::HydraulicNetworkComponent::modelType_t(m_mapPipeModels.value(m_ui->comboBoxPipeModel->currentText()));
		edge->m_pipeId = m_mapDBPipes.value(m_ui->comboBoxPipeDB->currentText());
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
	const VICUS::NetworkNode *nodeConst = dynamic_cast<const VICUS::NetworkNode *>(m_obj);
	if (nodeConst != nullptr){
		VICUS::NetworkNode *node = &network.m_nodes[nodeConst->m_id];
		node->m_type = VICUS::NetworkNode::NodeType(m_mapNodeTypes.value(m_ui->comboBoxNodeType->currentText()));
//		n->m_heatExchangePara = NANDRAD::HydraulicNetworkComponent::heatExchangeType_t(
//					m_mapHeatExchangeType.value(m_ui->comboBoxHeatExchangeType->currentText()));
		if (m_ui->lineEditNodeHeatingDemand->isValid())
			node->m_maxHeatingDemand = m_ui->lineEditNodeHeatingDemand->value();
		if (m_ui->lineEditNodeX->isValid())
			node->m_position.m_x = m_ui->lineEditNodeX->value();
		if (m_ui->lineEditNodeY->isValid())
			node->m_position.m_y = m_ui->lineEditNodeY->value();
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


void SVPropNetworkEditWidget::networkFromId()
{
	unsigned int id = SVViewStateHandler::instance().m_navigationTreeWidget->selectedNodeID();
	if (id != 0)
		m_treeItemId = id;
	const VICUS::Project &p = project();
	m_obj = p.objectById(m_treeItemId);
	if (dynamic_cast<const VICUS::Network *>(m_obj) != nullptr)
		m_network = dynamic_cast<const VICUS::Network *>(m_obj);
	else if (dynamic_cast<const VICUS::Network *>(m_obj->m_parent) != nullptr)
		m_network = dynamic_cast<const VICUS::Network *>(m_obj->m_parent);
	else
		m_network = nullptr;
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
	modifyNodeProperties();
	modifyHydraulicComponent();
}

void SVPropNetworkEditWidget::on_componentParModel_editCompleted()
{
	modifyHydraulicComponent();
}



// *** ComponentParameterModel ***

ComponentParameterModel::ComponentParameterModel(QObject *parent) : QAbstractTableModel(parent)
{
}

void ComponentParameterModel::setComponent(const NANDRAD::HydraulicNetworkComponent & component)
{
	m_component = component;
	m_parameterList = NANDRAD::HydraulicNetworkComponent::requiredParameter(m_component.m_modelType);
}

void ComponentParameterModel::getComponentParameter(IBK::Parameter m_para[])
{
	m_para = m_component.m_para;
}

int ComponentParameterModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_parameterList.size();
}

int ComponentParameterModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 3;
}

QVariant ComponentParameterModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	if (index.row()==0){
		if (index.column()==0)
			return "componentId";
		else if (index.column()==1)
			return m_component.m_id;
		else
			return QVariant();
	}

	if (index.column()==0)
		return NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::para_t", m_parameterList[index.row()-1]);
	else if (index.column()==1)
		return m_component.m_para[m_parameterList[index.row()-1]].value;
	else if (index.column()==2)
		return NANDRAD::KeywordList::Unit("HydraulicNetworkComponent::para_t", m_parameterList[index.row()-1]);
}

QVariant ComponentParameterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		if (section == 0)
			return QString("Parameter");
		else if (section == 1)
			return QString("Value");
		else if (section == 2)
			return QString("Unit");
	}
	return QVariant();
}

bool ComponentParameterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

	if (index.column()==1 && index.row()!=0){
		bool ok = false;
		double number = QtExt::Locale().toDouble(value.toString(), &ok);
		if (!ok)
			return false;
		m_component.m_para[m_parameterList[index.row()-1]].value = number;
		emit editCompleted();
	}
	return true;
}

Qt::ItemFlags ComponentParameterModel::flags(const QModelIndex &index) const
{
	if (index.row()==0)
		return !Qt::ItemIsEnabled | !Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if (index.column()==1)
		return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
