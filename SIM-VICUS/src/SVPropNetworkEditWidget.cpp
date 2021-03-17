#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include <QMessageBox>

#include <NANDRAD_HydraulicNetworkComponent.h>

#include <VICUS_KeywordList.h>

#include "Vic3DWireFrameObject.h"

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyNetwork.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVDialogSelectNetworkPipes.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVDBNetworkFluidEditWidget.h"


SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);

	showNetworkProperties();
	setAllHeatExchangeWidgetsVisible(false);

	// setup combobox node types
	m_ui->comboBoxNodeType->clear();
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
									VICUS::NetworkNode::NT_Mixer);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
									VICUS::NetworkNode::NT_Source);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Building),
									VICUS::NetworkNode::NT_Building);

	// connect browse filename widgets
	connect(m_ui->widgetBrowseFileNameTSVFile, SIGNAL(editingFinished()), this, SLOT(on_heatExchangeDataFile_editingFinished()));

	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::T_TemperatureConstant == (int)VICUS::NetworkHeatExchange::ModelType::T_TemperatureConstant);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::T_TemperatureSpline == (int)VICUS::NetworkHeatExchange::ModelType::T_TemperatureSpline);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::T_HeatLossConstant == (int)VICUS::NetworkHeatExchange::ModelType::T_HeatLossConstant);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::T_HeatLossSpline == (int)VICUS::NetworkHeatExchange::ModelType::T_HeatLossSpline);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::T_HeatLossIdealHeatPump == (int)VICUS::NetworkHeatExchange::ModelType::T_HeatLossIdealHeatPump);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::T_TemperatureZone == (int)VICUS::NetworkHeatExchange::ModelType::T_TemperatureZone);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::T_TemperatureConstructionLayer == (int)VICUS::NetworkHeatExchange::ModelType::T_TemperatureConstructionLayer);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::T_TemperatureFMUInterface == (int)VICUS::NetworkHeatExchange::ModelType::T_TemperatureFMUInterface);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::ModelType::NUM_T == (int)VICUS::NetworkHeatExchange::ModelType::NUM_T);

	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::para_t::P_Temperature == (int)VICUS::NetworkHeatExchange::para_t::P_Temperature);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::para_t::P_HeatLoss == (int)VICUS::NetworkHeatExchange::para_t::P_HeatLoss);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::para_t::P_ExternalHeatTransferCoefficient == (int)VICUS::NetworkHeatExchange::para_t::P_ExternalHeatTransferCoefficient);
	Q_ASSERT(NANDRAD::HydraulicNetworkHeatExchange::para_t::NUM_P == (int)VICUS::NetworkHeatExchange::para_t::NUM_P);
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
		case 3 : showComponentProperties(); break;
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
		if (network != nullptr && std::find(networks.begin(), networks.end(), network) == networks.end()) {
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
		m_currentConstNetwork = nullptr;
	else if (networks.size()>1){
		m_currentConstNetwork = nullptr;
		m_currentEdges.clear();
		m_currentNodes.clear();
	}
	else
		m_currentConstNetwork = networks[0];

	// now update UI
	setAllEnabled(false);
	if (m_currentConstNetwork != nullptr){
		m_ui->groupBoxProperties->setEnabled(true);
		m_ui->groupBoxEditNetwork->setEnabled(true);
		m_ui->groupBoxVisualisation->setEnabled(true);
		m_ui->groupBoxSizePipes->setEnabled(true);
		updateNetworkProperties();
		updateSizingParams();
	}
	else{
		clearUI();
		return;
	}

	// node(s) selected
	if (!m_currentNodes.empty() && m_currentEdges.empty()){
		m_ui->groupBoxNode->setEnabled(true);
		m_ui->groupBoxComponent->setEnabled(true);
		m_ui->groupBoxHeatExchange->setEnabled(true);
		updateNodeProperties();
	}
	// edge(s) selected
	else if(m_currentNodes.empty() && !m_currentEdges.empty()){
		m_ui->groupBoxEdge->setEnabled(true);
		m_ui->groupBoxComponent->setEnabled(true);
		m_ui->groupBoxHeatExchange->setEnabled(true);
		updateEdgeProperties();
	}
	// none selected
	else{
		m_ui->groupBoxNode->setEnabled(false);
		m_ui->groupBoxEdge->setEnabled(false);
		m_ui->groupBoxComponent->setEnabled(false);
		clearUI();
	}
}


void SVPropNetworkEditWidget::updateNodeProperties() {
	Q_ASSERT(!m_currentNodes.empty());

	setupComboBoxComponents();

	// if node type is not uniform, no editing will be allowed
	bool uniformNodeType = uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_type);
	m_ui->groupBoxNode->setEnabled(uniformNodeType);
	m_ui->groupBoxComponent->setEnabled(uniformNodeType);
	for (const VICUS::NetworkNode *node: m_currentNodes){
		if (node->m_type== VICUS::NetworkNode::NT_Mixer)
			m_ui->groupBoxComponent->setEnabled(false);
	}
	m_ui->comboBoxNodeType->setCurrentIndex(m_ui->comboBoxNodeType->findData(m_currentNodes[0]->m_type));
	m_ui->lineEditNodeMaxHeatingDemand->setEnabled(m_currentNodes[0]->m_type == VICUS::NetworkNode::NT_Building);

	m_ui->lineEditNodeX->setEnabled(m_currentNodes.size() == 1);
	m_ui->lineEditNodeY->setEnabled(m_currentNodes.size() == 1);

	if (m_currentNodes.size() == 1){
		m_ui->labelNodeId->setText(QString("%1").arg(m_currentNodes[0]->m_id));
		m_ui->lineEditNodeDisplayName->setText(QString::fromStdString(m_currentNodes[0]->m_displayName));
		m_ui->lineEditNodeX->setValue(m_currentNodes[0]->m_position.m_x);
		m_ui->lineEditNodeY->setValue(m_currentNodes[0]->m_position.m_y);
	}
	else{
		m_ui->labelNodeId->clear();
		m_ui->lineEditNodeDisplayName->clear();
		m_ui->lineEditNodeX->clear();
		m_ui->lineEditNodeY->clear();
	}

	if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_maxHeatingDemand))
		m_ui->lineEditNodeMaxHeatingDemand->setValue(m_currentNodes[0]->m_maxHeatingDemand);
	else
		m_ui->lineEditNodeMaxHeatingDemand->clear();

	m_ui->comboBoxComponent->blockSignals(true);
	if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_componentId))
		m_ui->comboBoxComponent->setCurrentIndex(m_ui->comboBoxComponent->findData(m_currentNodes[0]->m_componentId));
	else
		m_ui->comboBoxComponent->setCurrentIndex(-1);
	m_ui->comboBoxComponent->blockSignals(false);

	// update combobox
	VICUS::NetworkHeatExchange hx;
	if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_heatExchange)){
		hx = m_currentNodes[0]->m_heatExchange;
		m_ui->comboBoxHeatExchangeType->setCurrentIndex(m_ui->comboBoxHeatExchangeType->findData(hx.m_modelType));
	}
	// update hx widgets
	updateHeatExchangeWidgets();
}


void SVPropNetworkEditWidget::updateEdgeProperties() {

	Q_ASSERT(!m_currentEdges.empty());

	setupComboBoxComponents();
	setupComboboxPipeDB();

	if (m_currentEdges.size() == 1){
		m_ui->labelPipeLength->setText(QString("%1 m").arg(m_currentEdges[0]->length()));
		m_ui->lineEditEdgeDisplayName->setText(QString::fromStdString(m_currentEdges[0]->m_displayName));
	}
	else{
		m_ui->labelPipeLength->clear();
		m_ui->lineEditEdgeDisplayName->clear();
	}

	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_pipeId)) {
		int idx = m_ui->comboBoxPipeDB->findData(m_currentEdges[0]->m_pipeId);
		m_ui->comboBoxPipeDB->setCurrentIndex(idx);
	}
	else
		m_ui->comboBoxPipeDB->setCurrentIndex(-1);


	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_supply))
		m_ui->checkBoxSupplyPipe->setChecked(m_currentEdges[0]->m_supply);
	else
		m_ui->checkBoxSupplyPipe->setCheckState(Qt::CheckState::PartiallyChecked);

	m_ui->comboBoxComponent->blockSignals(true);
	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_componentId))
		m_ui->comboBoxComponent->setCurrentIndex(m_ui->comboBoxComponent->findData(m_currentEdges[0]->m_componentId));
	else
		m_ui->comboBoxComponent->setCurrentIndex(-1);
	m_ui->comboBoxComponent->blockSignals(false);

	// update combobox
	VICUS::NetworkHeatExchange hx;
	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_heatExchange)){
		hx = m_currentEdges[0]->m_heatExchange;
		m_ui->comboBoxHeatExchangeType->setCurrentIndex(m_ui->comboBoxHeatExchangeType->findData(hx.m_modelType));
	}
	// update hx widgets
	updateHeatExchangeWidgets();
}


void SVPropNetworkEditWidget::updateNetworkProperties()
{
	Q_ASSERT(m_currentConstNetwork != nullptr);

	m_ui->labelNetworkName->setText(QString::fromStdString(m_currentConstNetwork->m_name));
	m_ui->labelEdgeCount->setText(QString("%1").arg(m_currentConstNetwork->m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_currentConstNetwork->m_nodes.size()));
	if (m_currentConstNetwork->checkConnectedGraph()){
		m_ui->labelNetworkConnected->setText("Network is connected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: green}");
	}
	else{
		m_ui->labelNetworkConnected->setText("Network is unconnected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: red}");
	}
	m_ui->labelTotalLength->setText(QString("%1 m").arg(m_currentConstNetwork->totalLength()));
	m_ui->pushButtonConnectBuildings->setEnabled(m_currentConstNetwork->nextUnconnectedBuilding()>=0);
	m_ui->pushButtonReduceDeadEnds->setEnabled(m_currentConstNetwork->checkConnectedGraph() && m_currentConstNetwork->numberOfBuildings() > 0);
	m_ui->labelLargestDiameter->setText(largestDiameter());
	m_ui->labelSmallestDiameter->setText(smallestDiameter());

	m_ui->horizontalSliderScaleEdges->setValue((int)m_currentConstNetwork->m_scaleEdges);
	m_ui->horizontalSliderScaleNodes->setValue((int)m_currentConstNetwork->m_scaleNodes);
}


void SVPropNetworkEditWidget::updateHeatExchangeWidgets()
{
	VICUS::NetworkHeatExchange hx;
	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_heatExchange))
		hx = m_currentEdges[0]->m_heatExchange;
	else if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_heatExchange))
		hx = m_currentNodes[0]->m_heatExchange;
	else
		return;


	// toggle visibility of widgets
	setAllHeatExchangeWidgetsVisible(false);
	switch (hx.m_modelType) {
		case VICUS::NetworkHeatExchange::T_HeatLossConstant:{
			m_ui->labelHeatFlux->setVisible(true);
			m_ui->lineEditHeatFlux->setVisible(true);
			break;
		}
		case VICUS::NetworkHeatExchange::T_TemperatureConstant:{
			m_ui->labelTemperature->setVisible(true);
			m_ui->lineEditTemperature->setVisible(true);
			m_ui->labelHXTransferCoefficient->setVisible(true);
			m_ui->lineEditHXTransferCoefficient->setVisible(true);
			break;
		}
		case VICUS::NetworkHeatExchange::T_TemperatureSpline:
		case VICUS::NetworkHeatExchange::T_HeatLossSpline:{
			m_ui->labelDataFile->setVisible(true);
			m_ui->widgetBrowseFileNameTSVFile->setVisible(true);
			m_ui->labelHXTransferCoefficient->setVisible(true);
			m_ui->lineEditHXTransferCoefficient->setVisible(true);
			break;
		}
		default:;
	}

	// clear widgets
	m_ui->lineEditHeatFlux->clear();
	m_ui->lineEditTemperature->clear();
	m_ui->lineEditHXTransferCoefficient->clear();
	m_ui->widgetBrowseFileNameTSVFile->setFilename("");

	// set values
	if (!hx.m_para[VICUS::NetworkHeatExchange::P_HeatLoss].empty())
		m_ui->lineEditHeatFlux->setValue(hx.m_para[VICUS::NetworkHeatExchange::P_HeatLoss].value);

	if (!hx.m_para[VICUS::NetworkHeatExchange::P_Temperature].empty())
		m_ui->lineEditTemperature->setValue(hx.m_para[VICUS::NetworkHeatExchange::P_Temperature].get_value("C"));

	if (!hx.m_para[VICUS::NetworkHeatExchange::P_ExternalHeatTransferCoefficient].empty())
		m_ui->lineEditHXTransferCoefficient->setValue(hx.m_para[VICUS::NetworkHeatExchange::P_ExternalHeatTransferCoefficient].value);

	if (hx.m_splPara[VICUS::NetworkHeatExchange::SPL_HeatLoss].m_tsvFile.isValid())
		m_ui->widgetBrowseFileNameTSVFile->setFilename(QString::fromStdString(
												hx.m_splPara[VICUS::NetworkHeatExchange::SPL_HeatLoss].m_tsvFile.str()));
}


void SVPropNetworkEditWidget::updateSizingParams() {
	if (m_currentConstNetwork != nullptr){
		m_ui->doubleSpinBoxTemperatureSetpoint->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_TemperatureSetpoint].get_value(IBK::Unit("C")));
		m_ui->doubleSpinBoxTemperatureDifference->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_TemperatureDifference].value);
		m_ui->doubleSpinBoxMaximumPressureLoss->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_MaxPressureLoss].value);
	}
}


void SVPropNetworkEditWidget::clearUI(){
	m_ui->lineEditNodeMaxHeatingDemand->clear();
	m_ui->labelNodeId->clear();
	m_ui->lineEditNodeX->clear();
	m_ui->lineEditNodeY->clear();
	m_ui->labelPipeLength->clear();
	m_ui->labelPipeLength->clear();
	m_ui->checkBoxSupplyPipe->setChecked(false);
	m_ui->comboBoxPipeDB->setCurrentIndex(-1);
	m_ui->labelEdgeCount->clear();
	m_ui->labelNodeCount->clear();
	m_ui->labelNetworkConnected->clear();
	m_ui->labelLargestDiameter->clear();
	m_ui->labelSmallestDiameter->clear();
	m_ui->labelTotalLength->clear();
	m_ui->lineEditHeatFlux->clear();
	m_ui->lineEditTemperature->clear();
	// TODO Hauke, clear other elements
}

void SVPropNetworkEditWidget::setAllEnabled(bool enabled)
{
	m_ui->groupBoxProperties->setEnabled(enabled);
	m_ui->groupBoxNode->setEnabled(enabled);
	m_ui->groupBoxEdge->setEnabled(enabled);
	m_ui->groupBoxSizePipes->setEnabled(enabled);
	m_ui->groupBoxVisualisation->setEnabled(enabled);
	m_ui->groupBoxEditNetwork->setEnabled(enabled);
	m_ui->groupBoxComponent->setEnabled(enabled);
	m_ui->groupBoxHeatExchange->setEnabled(enabled);
}

void SVPropNetworkEditWidget::setAllHeatExchangeWidgetsVisible(bool visible)
{
	m_ui->labelTemperature->setVisible(visible);
	m_ui->lineEditTemperature->setVisible(visible);
	m_ui->labelHXTransferCoefficient->setVisible(visible);
	m_ui->lineEditHXTransferCoefficient->setVisible(visible);
	m_ui->labelHeatFlux->setVisible(visible);
	m_ui->lineEditHeatFlux->setVisible(visible);
	m_ui->labelDataFile->setVisible(visible);
	m_ui->widgetBrowseFileNameTSVFile->setVisible(visible);
	m_ui->labelFMUFile->setVisible(visible);
	m_ui->widgetBrowseFileNameFMU->setVisible(visible);
	m_ui->labelZoneId->setVisible(visible);
	m_ui->comboBoxZoneId->setVisible(visible);
}


const VICUS::NetworkComponent *SVPropNetworkEditWidget::currentComponent()
{
	const SVDatabase & db = SVSettings::instance().m_db;

	if (!m_currentNodes.empty())
		return db.m_networkComponents[m_currentNodes[0]->m_componentId];
	else if (!m_currentEdges.empty())
		return db.m_networkComponents[m_currentEdges[0]->m_componentId];
	else
		return nullptr;
}


QString SVPropNetworkEditWidget::largestDiameter() const
{
	const SVDatabase & db = SVSettings::instance().m_db;
	double dMax = 0;
	for (const VICUS::NetworkEdge &edge: m_currentConstNetwork->m_edges){
		const VICUS::NetworkPipe * p = db.m_pipes[edge.m_pipeId];
		if (p == nullptr)
			return QString();
		if (p->m_diameterOutside > dMax)
			dMax = p->m_diameterOutside;
	}
	return QString("%1").arg(dMax);
}


QString SVPropNetworkEditWidget::smallestDiameter() const
{
	const SVDatabase & db = SVSettings::instance().m_db;
	double dMin = std::numeric_limits<double>::max();
	for (const VICUS::NetworkEdge &edge: m_currentConstNetwork->m_edges){
		const VICUS::NetworkPipe * p = db.m_pipes[edge.m_pipeId];
		if (p == nullptr)
			return QString();
		if (p->m_diameterOutside < dMin)
			dMin = p->m_diameterOutside;
	}
	return QString("%1").arg(dMin);
}


void SVPropNetworkEditWidget::modifyHeatExchangeProperties()
{
	if (!setNetwork())
		return;

	// set model type
	VICUS::NetworkHeatExchange::ModelType modelType =
			VICUS::NetworkHeatExchange::ModelType(m_ui->comboBoxHeatExchangeType->currentData().toUInt());
	VICUS::NetworkHeatExchange hx(modelType);

	// set heat loss
	if (m_ui->lineEditHeatFlux->isValid())
		VICUS::KeywordList::setParameter(hx.m_para, "NetworkHeatExchange::para_t",
										 VICUS::NetworkHeatExchange::P_HeatLoss,
										 m_ui->lineEditHeatFlux->value());
	else
		hx.m_para[VICUS::NetworkHeatExchange::P_HeatLoss].clear();

	// set temperature
	if (m_ui->lineEditTemperature->isValid())
		VICUS::KeywordList::setParameter(hx.m_para, "NetworkHeatExchange::para_t",
										 VICUS::NetworkHeatExchange::P_Temperature,
										 m_ui->lineEditTemperature->value());
	else
		hx.m_para[VICUS::NetworkHeatExchange::P_Temperature].clear();

	// set external hx coefficient
	if (m_ui->lineEditHXTransferCoefficient->isValid())
		VICUS::KeywordList::setParameter(hx.m_para, "NetworkHeatExchange::para_t",
										 VICUS::NetworkHeatExchange::P_ExternalHeatTransferCoefficient,
										 m_ui->lineEditHXTransferCoefficient->value());
	else
		hx.m_para[VICUS::NetworkHeatExchange::P_ExternalHeatTransferCoefficient].clear();

	// set data file
	IBK::Path tsvFile(m_ui->widgetBrowseFileNameTSVFile->filename().toStdString());
	if (tsvFile.isValid() && modelType == VICUS::NetworkHeatExchange::T_HeatLossSpline)
		hx.m_splPara[VICUS::NetworkHeatExchange::SPL_HeatLoss] = NANDRAD::LinearSplineParameter("HeatLoss",
																 NANDRAD::LinearSplineParameter::I_LINEAR,
																 IBK::Path(m_ui->widgetBrowseFileNameTSVFile->filename().toStdString()));
	else
		hx.m_splPara[VICUS::NetworkHeatExchange::SPL_HeatLoss] = NANDRAD::LinearSplineParameter();

	if (tsvFile.isValid() && modelType == VICUS::NetworkHeatExchange::T_TemperatureSpline)
		hx.m_splPara[VICUS::NetworkHeatExchange::SPL_Temperature] = NANDRAD::LinearSplineParameter("Temperature",
																 NANDRAD::LinearSplineParameter::I_LINEAR,
																 IBK::Path(m_ui->widgetBrowseFileNameTSVFile->filename().toStdString()));
	else
		hx.m_splPara[VICUS::NetworkHeatExchange::SPL_Temperature] = NANDRAD::LinearSplineParameter();


	// set hx properties to nodes
	if (!m_currentNodes.empty()){
		for (const VICUS::NetworkNode * nodeConst: m_currentNodes){
			m_currentNetwork.m_nodes[nodeConst->m_id].m_heatExchange = hx;
		}
	}

	// set hx properties to edges
	if (!m_currentEdges.empty()){
		for (const VICUS::NetworkEdge * edge: m_currentEdges){
			m_currentNetwork.edge(edge->nodeId1(), edge->nodeId2())->m_heatExchange = hx;
		}
	}

	m_currentNetwork.updateNodeEdgeConnectionPointers();
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::setupComboboxPipeDB()
{
	const SVDatabase  & db = SVSettings::instance().m_db;

	m_ui->comboBoxPipeDB->blockSignals(true);
	m_ui->comboBoxPipeDB->clear();
	for (unsigned int pipeID : m_currentConstNetwork->m_availablePipes) {
		// lookup pipe in DB
		const VICUS::NetworkPipe * pipe = db.m_pipes[pipeID];
		// skip missing pipes
		if (pipe == nullptr)
			continue;
		m_ui->comboBoxPipeDB->addItem(QString::fromStdString(IBK::FormatString("%1 [%2 mm]")
			.arg(pipe->m_displayName.string(IBK::MultiLanguageString::m_language, "en"))
			.arg(pipe->m_diameterOutside).str()), pipeID);
	}
	m_ui->comboBoxPipeDB->blockSignals(false);
}


void SVPropNetworkEditWidget::setupComboboxHeatExchangeType()
{
	m_ui->comboBoxHeatExchangeType->clear();

	const VICUS::NetworkComponent *comp = currentComponent();
	if (comp==nullptr)
		return;

	std::vector<unsigned int> hxTypes = NANDRAD::HydraulicNetworkHeatExchange::availableHeatExchangeTypes(
													(NANDRAD::HydraulicNetworkComponent::ModelType)comp->m_modelType);

	m_ui->comboBoxHeatExchangeType->addItem(tr("Adiabatic"), VICUS::NetworkHeatExchange::NUM_T);
	for (unsigned int i: hxTypes){
		m_ui->comboBoxHeatExchangeType->addItem(QString::fromStdString(
											VICUS::KeywordList::Description("NetworkHeatExchange::ModelType", (int)i)), i);
	}
	m_ui->comboBoxHeatExchangeType->setCurrentIndex(-1);
}


void SVPropNetworkEditWidget::setupComboBoxComponents()
{
	m_ui->comboBoxComponent->blockSignals(true);
	const SVDatabase & db = SVSettings::instance().m_db;
	m_ui->comboBoxComponent->clear();
	m_ui->comboBoxComponent->addItem(tr("<None>"), VICUS::INVALID_ID);
	for (auto comp = db.m_networkComponents.begin(); comp!=db.m_networkComponents.end(); ++comp)
		m_ui->comboBoxComponent->addItem(QString::fromStdString(
										comp->second.m_displayName.string(IBK::MultiLanguageString::m_language, "en")),
										comp->second.m_id);
	m_ui->comboBoxComponent->blockSignals(false);

	setupComboboxHeatExchangeType();
}


void SVPropNetworkEditWidget::modifySizingParams()
{
	if (!setNetwork())
		return;
	m_currentNetwork.m_para[VICUS::Network::P_TemperatureSetpoint].set(VICUS::KeywordList::Keyword("Network::para_t", VICUS::Network::P_TemperatureSetpoint),
																	   m_ui->doubleSpinBoxTemperatureSetpoint->value(),
																	   IBK::Unit("C"));
	VICUS::KeywordList::setParameter(m_currentNetwork.m_para, "Network::para_t", VICUS::Network::P_TemperatureDifference,
									 m_ui->doubleSpinBoxTemperatureDifference->value());
	VICUS::KeywordList::setParameter(m_currentNetwork.m_para, "Network::para_t", VICUS::Network::P_MaxPressureLoss,
									 m_ui->doubleSpinBoxMaximumPressureLoss->value());
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network sizing parameters modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


bool SVPropNetworkEditWidget::setNetwork() {
	if (m_currentConstNetwork == nullptr)
		return false;
	VICUS::Project p = project();
	m_currentNetwork = *p.element(p.m_geometricNetworks, m_currentConstNetwork->m_id);
	return true;
}


void SVPropNetworkEditWidget::showNetworkProperties() {
	m_ui->groupBoxProperties->setVisible(true);
	m_ui->groupBoxSizePipes->setVisible(true);
	m_ui->groupBoxVisualisation->setVisible(true);
	m_ui->groupBoxEditNetwork->setVisible(true);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxComponent->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(false);
}


void SVPropNetworkEditWidget::showNodeProperties() {
	m_ui->groupBoxProperties->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxVisualisation->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxNode->setVisible(true);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxComponent->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(false);
}


void SVPropNetworkEditWidget::showEdgeProperties() {
	m_ui->groupBoxProperties->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxVisualisation->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(true);
	m_ui->groupBoxComponent->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(false);
}

void SVPropNetworkEditWidget::showComponentProperties(){
	m_ui->groupBoxProperties->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxVisualisation->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxComponent->setVisible(true);
	m_ui->groupBoxHeatExchange->setVisible(true);
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
		m_ui->comboBoxNodeType->currentData().toUInt()) );
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
	modifyEdgeProperty(&VICUS::NetworkEdge::m_pipeId, m_ui->comboBoxPipeDB->currentData().toUInt());
}

void SVPropNetworkEditWidget::on_checkBoxSupplyPipe_clicked()
{
	modifyEdgeProperty(&VICUS::NetworkEdge::m_supply, m_ui->checkBoxSupplyPipe->isChecked());
}

void SVPropNetworkEditWidget::on_comboBoxComponent_currentIndexChanged(int index)
{
	if (!m_currentEdges.empty())
		modifyEdgeProperty(&VICUS::NetworkEdge::m_componentId, m_ui->comboBoxComponent->currentData().toUInt());
	if (!m_currentNodes.empty())
		modifyNodeProperty(&VICUS::NetworkNode::m_componentId, m_ui->comboBoxComponent->currentData().toUInt());

	setupComboboxHeatExchangeType();
	updateHeatExchangeWidgets();
}

void SVPropNetworkEditWidget::on_horizontalSliderScaleNodes_valueChanged(int value)
{

	if (!setNetwork())
		return;
	m_currentNetwork.m_scaleNodes = value;
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}

void SVPropNetworkEditWidget::on_horizontalSliderScaleEdges_valueChanged(int value)
{
	if (!setNetwork())
		return;
	m_currentNetwork.m_scaleEdges = value;
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_pushButtonEditComponents_clicked() {
	unsigned int currentId  = m_ui->comboBoxComponent->currentData().toUInt();
	SVDatabaseEditDialog *dialog = SVMainWindow::instance().dbNetworkComponentEditDialog();
	int newId = dialog->select(currentId);
	if (newId > 0){
		setupComboBoxComponents();
		m_ui->comboBoxComponent->setCurrentIndex(m_ui->comboBoxComponent->findData(newId));
	}
}

void SVPropNetworkEditWidget::on_lineEditNodeHeatingDemand_editingFinished()
{
	if (m_ui->lineEditNodeMaxHeatingDemand->isValid())
		modifyNodeProperty(&VICUS::NetworkNode::m_maxHeatingDemand, m_ui->lineEditNodeMaxHeatingDemand->value());
}


void SVPropNetworkEditWidget::on_lineEditHeatFlux_editingFinished()
{
	modifyHeatExchangeProperties();
}


void SVPropNetworkEditWidget::on_lineEditTemperature_editingFinished()
{
	modifyHeatExchangeProperties();
}


void SVPropNetworkEditWidget::on_heatExchangeDataFile_editingFinished()
{
	modifyHeatExchangeProperties();
}

void SVPropNetworkEditWidget::on_lineEditHXTransferCoefficient_editingFinished()
{
	modifyHeatExchangeProperties();
}

void SVPropNetworkEditWidget::on_lineEditNodeDisplayName_editingFinished()
{
	modifyNodeProperty(&VICUS::NetworkNode::m_displayName, m_ui->lineEditNodeDisplayName->text().toStdString());
}


void SVPropNetworkEditWidget::on_lineEditEdgeDisplayName_editingFinished()
{
	modifyEdgeProperty(&VICUS::NetworkEdge::m_displayName, m_ui->lineEditEdgeDisplayName->text().toStdString());
}

void SVPropNetworkEditWidget::on_pushButtonSizePipeDimensions_clicked()
{
	FUNCID(SVPropNetworkEditWidget::on_pushButtonSizePipeDimensions_clicked);
	modifySizingParams();
	if (!setNetwork())
		return;
	const VICUS::Project &p = project();
	const VICUS::NetworkFluid * fluid = p.element(p.m_networkFluids, m_currentNetwork.m_fluidID);
	if (fluid == nullptr)
		throw IBK::Exception(IBK::FormatString("Could not find fluid with id %1 in fluid database")
							.arg(m_currentNetwork.m_fluidID), FUNC_ID);

	// filter out list of available pipes
	const SVDatabase & db = SVSettings::instance().m_db;
	std::vector<const VICUS::NetworkPipe*> availablePipes;
	for (unsigned int pipeID : m_currentNetwork.m_availablePipes) {
		const VICUS::NetworkPipe * pipe = db.m_pipes[pipeID];
		if (pipe == nullptr) // skip unavailable/undefined pipes
			continue;
		availablePipes.push_back(pipe);
	}

	m_currentNetwork.sizePipeDimensions(fluid, availablePipes);

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
	updateSizingParams();
}

void SVPropNetworkEditWidget::on_pushButtonGenerateIntersections_clicked()
{
	if (!setNetwork())
		return;
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	m_currentNetwork.generateIntersections();

	// set all selected
	for (VICUS::NetworkNode &node: m_currentNetwork.m_nodes)
		node.m_selected = true;
	for (VICUS::NetworkEdge &edge: m_currentNetwork.m_edges)
		edge.m_selected = true;

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonConnectBuildings_clicked()
{
	if (!setNetwork())
		return;
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	m_currentNetwork.connectBuildings(false);

	// set all selected
	for (VICUS::NetworkNode &node: m_currentNetwork.m_nodes)
		node.m_selected = true;
	for (VICUS::NetworkEdge &edge: m_currentNetwork.m_edges)
		edge.m_selected = true;

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonReduceDeadEnds_clicked()
{
	if (!setNetwork())
		return;
	VICUS::Network newNetwork = m_currentNetwork.copyWithBaseParameters(); // makes a copy without edges, nodes
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	m_currentNetwork.cleanDeadEnds(newNetwork);

	// set all selected
	for (VICUS::NetworkNode &node: m_currentNetwork.m_nodes)
		node.m_selected = true;
	for (VICUS::NetworkEdge &edge: m_currentNetwork.m_edges)
		edge.m_selected = true;

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, newNetwork);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonReduceRedundantNodes_clicked()
{
	if (!setNetwork())
		return;

	// set current network invisible
	m_currentNetwork.m_visible = false;
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undoMod = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undoMod->push(); // modifies project and updates views

	// make copy with reduced edges
	VICUS::Network newNetwork = m_currentNetwork.copyWithBaseParameters(); // makes a copy without edges, nodes
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	m_currentNetwork.cleanRedundantEdges(newNetwork);
	newNetwork.m_visible = true;
	newNetwork.m_name += "_reduced";
	const VICUS::Project & p = project();
	newNetwork.m_id = p.uniqueId(p.m_geometricNetworks);
	newNetwork.updateNodeEdgeConnectionPointers();

	// set all selected
	for (VICUS::NetworkNode &node: newNetwork.m_nodes)
		node.m_selected = true;
	for (VICUS::NetworkEdge &edge: newNetwork.m_edges)
		edge.m_selected = true;

	SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("modified network"), newNetwork);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}


void SVPropNetworkEditWidget::on_pushButtonSelectPipes_clicked()
{
	if (!setNetwork())
		return;

	SVDialogSelectNetworkPipes *dialog = new SVDialogSelectNetworkPipes(this);
	dialog->edit(m_currentNetwork);

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_comboBoxHeatExchangeType_activated(int index)
{
	VICUS::NetworkHeatExchange currentHX;
	bool isUniform = true;
	if (m_currentEdges.size()>0){
		currentHX = m_currentEdges[0]->m_heatExchange;
		isUniform = uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_heatExchange);
	}
	else if (m_currentNodes.size()>0){
		currentHX = m_currentNodes[0]->m_heatExchange;
		isUniform = uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_heatExchange);
	}

	VICUS::NetworkHeatExchange::ModelType newModelType =
			VICUS::NetworkHeatExchange::ModelType(m_ui->comboBoxHeatExchangeType->currentData().toUInt());

	// if model type has not changed and it is unifrom fro all selected objects: do nothing
	if (newModelType == currentHX.m_modelType && isUniform)
		return;

	// if model type has changed, clear current hx properties
	if (newModelType != currentHX.m_modelType)
		currentHX = VICUS::NetworkHeatExchange(newModelType);

	// Note: if model type is the same like the first selected object and objects are not uniform,
	// we still apply the currentHX to all objects

	if (!setNetwork())
		return;

	// set hx properties to nodes
	if (!m_currentNodes.empty()){
		for (const VICUS::NetworkNode * nodeConst: m_currentNodes){
			m_currentNetwork.m_nodes[nodeConst->m_id].m_heatExchange = currentHX;
		}
	}

	// set hx properties to edges
	if (!m_currentEdges.empty()){
		for (const VICUS::NetworkEdge * edge: m_currentEdges){
			m_currentNetwork.edge(edge->nodeId1(), edge->nodeId2())->m_heatExchange = currentHX;
		}
	}

	m_currentNetwork.updateNodeEdgeConnectionPointers();
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views

	updateHeatExchangeWidgets();
}


template <typename TEdgeProp, typename Tval>
void SVPropNetworkEditWidget::modifyEdgeProperty(TEdgeProp property, const Tval & value)
{
	if (!setNetwork())
		return;
	Q_ASSERT(!m_currentEdges.empty());
	for (const VICUS::NetworkEdge * edgeConst: m_currentEdges){
		VICUS::NetworkEdge * edge = m_currentNetwork.edge(edgeConst->nodeId1(), edgeConst->nodeId2());
		Q_ASSERT(edge != nullptr);
		edge->*property = value;
	}
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
	return;
}


template<typename TNodeProp, typename Tval>
void SVPropNetworkEditWidget::modifyNodeProperty(TNodeProp property, const Tval &value)
{
	if (!setNetwork())
		return;
	Q_ASSERT(!m_currentNodes.empty());
	for (const VICUS::NetworkNode * nodeConst: m_currentNodes){
		m_currentNetwork.m_nodes[nodeConst->m_id].*property = value;
	}
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_pushButtonSelectFluid_clicked()
{
	unsigned int currentId  = m_currentConstNetwork->m_fluidID;
	SVDatabaseEditDialog *dialog = SVMainWindow::instance().dbFluidEditDialog();
	unsigned int newId = dialog->select(currentId);
	if (newId > 0){
		if (!setNetwork())
			return;
		m_currentNetwork.m_fluidID = newId;
		m_currentNetwork.updateNodeEdgeConnectionPointers();
		unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
		undo->push(); // modifies project and updates views
	}
}
