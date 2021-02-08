#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include <QMessageBox>

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVDialogSelectNetworkPipes.h"
#include "SVDBNetworkComponentEditDialog.h"
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
	m_ui->comboBoxNodeType->clear();
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
									VICUS::NetworkNode::NT_Mixer);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
									VICUS::NetworkNode::NT_Source);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Building),
									VICUS::NetworkNode::NT_Building);
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
	if (m_currentConstNetwork != nullptr){
		setAllEnabled(true);
		updateNetworkProperties();
		updateSizingParams();
	}
	else{
		clearUI();
		setAllEnabled(false);
		return;
	}
	if (!m_currentNodes.empty() && m_currentEdges.empty()){
		setAllEnabled(true);
		updateNodeProperties();
	}
	else if(m_currentNodes.empty() && !m_currentEdges.empty()){
		setAllEnabled(true);
		updateEdgeProperties();
	}
	else{
		m_ui->groupBoxNode->setEnabled(false);
		m_ui->groupBoxEdge->setEnabled(false);
		m_ui->groupBoxComponent->setEnabled(false);
		clearUI();
	}
}


void SVPropNetworkEditWidget::updateNodeProperties() {
	Q_ASSERT(!m_currentNodes.empty());

	const SVDatabase  & db = SVSettings::instance().m_db;

	setupComboBoxComponents();

	// if node type is not uniform, no editing will be allowed
	bool uniformNodeType = uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_type);
	m_ui->groupBoxNode->setEnabled(uniformNodeType);
	m_ui->groupBoxComponent->setEnabled(uniformNodeType);
	m_ui->comboBoxNodeType->setCurrentIndex(m_ui->comboBoxNodeType->findData(m_currentNodes[0]->m_type));
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
		m_ui->comboBoxComponent->setCurrentIndex(m_ui->comboBoxComponent->findData(m_currentNodes[0]->m_componentId));
		const VICUS::NetworkComponent *comp = db.m_networkComponents[m_currentNodes[0]->m_componentId];
		if (comp != nullptr)
			m_ui->groupBoxHeatExchange->setEnabled(
				NANDRAD::HydraulicNetworkComponent::hasHeatExchange((NANDRAD::HydraulicNetworkComponent::ModelType)comp->m_modelType)
							&& comp->m_heatExchangeType != (int)NANDRAD::HydraulicNetworkComponent::NUM_HT);
		else
			m_ui->groupBoxHeatExchange->setEnabled(false);
	}
	else{
		m_ui->comboBoxComponent->setCurrentIndex(-1);
	}
}


void SVPropNetworkEditWidget::updateEdgeProperties() {

	Q_ASSERT(!m_currentEdges.empty());

	const SVDatabase  & db = SVSettings::instance().m_db;

	setupComboBoxComponents();
	setupComboboxPipeDB();

	if (m_currentEdges.size() == 1)
		m_ui->labelPipeLength->setText(QString("%1 m").arg(m_currentEdges[0]->length()));
	else
		m_ui->labelPipeLength->setText("");

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

	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_componentId)){
		m_ui->comboBoxComponent->setCurrentIndex(m_ui->comboBoxComponent->findData(m_currentEdges[0]->m_componentId));
		const VICUS::NetworkComponent *comp = db.m_networkComponents[m_currentEdges[0]->m_componentId];
		if (comp != nullptr)
			m_ui->groupBoxHeatExchange->setEnabled(
				NANDRAD::HydraulicNetworkComponent::hasHeatExchange((NANDRAD::HydraulicNetworkComponent::ModelType)comp->m_modelType)
							&& comp->m_heatExchangeType != (int)NANDRAD::HydraulicNetworkComponent::NUM_HT);
		else
			m_ui->groupBoxHeatExchange->setEnabled(false);
	}
	else{
		m_ui->comboBoxComponent->setCurrentIndex(-1);
	}
//	m_ui->comboBoxHeatExchangeType->setCurrentText(m_mapHeatExchangeType.key(edge->m_heatExchangeType));
//	m_ui->comboBoxHeatExchangeType->setEnabled(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(edge->m_modelType));
//	m_ui->groupBoxHeatExchange->setEnabled(NANDRAD::HydraulicNetworkComponent::hasHeatExchange(edge->m_modelType)
//										   && edge->m_heatExchangeType != NANDRAD::HydraulicNetworkComponent::NUM_HT);
//	m_ui->lineEditHeatFlux->setValue(edge->m_heatExchangePara[NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant].value);
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


void SVPropNetworkEditWidget::updateSizingParams() {
	if (m_currentConstNetwork != nullptr){
		m_ui->doubleSpinBoxTemperatureSetpoint->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_TemperatureSetpoint].get_value(IBK::Unit("C")));
		m_ui->doubleSpinBoxTemperatureDifference->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_TemperatureDifference].value);
		m_ui->doubleSpinBoxMaximumPressureLoss->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_MaxPressureLoss].value);
	}
}


void SVPropNetworkEditWidget::clearUI(){
	m_ui->lineEditNodeHeatingDemand->clear();
	m_ui->labelNodeId->clear();
	m_ui->lineEditNodeX->clear();
	m_ui->lineEditNodeY->clear();
	m_ui->labelPipeLength->clear();
	m_ui->labelPipeLength->clear();
	m_ui->checkBoxSupplyPipe->setChecked(false);
	m_ui->comboBoxPipeDB->setCurrentIndex(-1);
	m_ui->labelEdgeCount->setText("");
	m_ui->labelNodeCount->setText("");
	m_ui->labelNetworkConnected->setText("");
	m_ui->labelLargestDiameter->setText("");
	m_ui->labelSmallestDiameter->setText("");
	m_ui->labelTotalLength->setText("");
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


void SVPropNetworkEditWidget::setupComboBoxComponents()
{
	const SVDatabase & db = SVSettings::instance().m_db;
	m_ui->comboBoxComponent->clear();
	for (auto comp = db.m_networkComponents.begin(); comp!=db.m_networkComponents.end(); ++comp)
		m_ui->comboBoxComponent->addItem(QString::fromStdString(
										comp->second.m_displayName.string(IBK::MultiLanguageString::m_language, "en")),
										comp->second.m_id);
}


void SVPropNetworkEditWidget::modifySizingParams()
{
	if (!setNetwork())
		return;
	m_currentNetwork.m_para[VICUS::Network::P_TemperatureSetpoint].set(VICUS::KeywordList::Keyword("Network::SizingParam", VICUS::Network::P_TemperatureSetpoint),
																	   m_ui->doubleSpinBoxTemperatureSetpoint->value(),
																	   IBK::Unit("C"));
	VICUS::KeywordList::setParameter(m_currentNetwork.m_para, "Network::SizingParam", VICUS::Network::P_TemperatureDifference,
									 m_ui->doubleSpinBoxTemperatureDifference->value());
	VICUS::KeywordList::setParameter(m_currentNetwork.m_para, "Network::SizingParam", VICUS::Network::P_MaxPressureLoss,
									 m_ui->doubleSpinBoxMaximumPressureLoss->value());
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network sizing parameters modified"), networkIndex, m_currentNetwork);
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
	m_ui->labelSelectionInfo->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(true);
	m_ui->groupBoxEditNetwork->setVisible(true);
	m_ui->groupBoxVisualisation->setVisible(true);
	m_ui->groupBoxHeatExchange->setVisible(false);
	m_ui->groupBoxComponent->setVisible(false);
}


void SVPropNetworkEditWidget::showNodeProperties() {
	m_ui->labelSelectionInfo->setVisible(false);
	m_ui->groupBoxNode->setVisible(true);
	m_ui->groupBoxEdge->setVisible(false);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxVisualisation->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(true);
	m_ui->groupBoxComponent->setVisible(true);
}


void SVPropNetworkEditWidget::showEdgeProperties() {
	m_ui->labelSelectionInfo->setVisible(false);
	m_ui->groupBoxNode->setVisible(false);
	m_ui->groupBoxEdge->setVisible(true);
	m_ui->groupBoxSizePipes->setVisible(false);
	m_ui->groupBoxVisualisation->setVisible(false);
	m_ui->groupBoxEditNetwork->setVisible(false);
	m_ui->groupBoxHeatExchange->setVisible(true);
	m_ui->groupBoxComponent->setVisible(true);
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

void SVPropNetworkEditWidget::on_comboBoxComponent_activated(const QString &arg1)
{
	if (!m_currentEdges.empty())
		modifyEdgeProperty(&VICUS::NetworkEdge::m_componentId, m_ui->comboBoxComponent->currentData().toUInt());
	if (!m_currentNodes.empty())
		modifyNodeProperty(&VICUS::NetworkNode::m_componentId, m_ui->comboBoxComponent->currentData().toUInt());
}

void SVPropNetworkEditWidget::on_horizontalSliderScaleNodes_valueChanged(int value)
{

	if (!setNetwork())
		return;
	m_currentNetwork.m_scaleNodes = value;
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network visualization properties updated"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}

void SVPropNetworkEditWidget::on_horizontalSliderScaleEdges_valueChanged(int value)
{
	if (!setNetwork())
		return;
	m_currentNetwork.m_scaleEdges = value;
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network visualization properties updated"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_pushButtonEditComponents_clicked() {

	unsigned int componentId = 0;
	if (m_currentNodes[0] != nullptr)
		componentId = m_currentNodes[0]->m_componentId;
	else if (m_currentEdges[0] != nullptr)
		componentId = m_currentEdges[0]->m_componentId;
	else
		return;

	SVDBNetworkComponentEditDialog *dialog = new SVDBNetworkComponentEditDialog(this);
	int idx = dialog->select(componentId);
	setupComboBoxComponents();
	m_ui->comboBoxComponent->setCurrentIndex(m_ui->comboBoxComponent->findData(idx));
#if 0
	if (dialog->edit(m_currentConstNetwork->m_id, componentId) == QDialog::Accepted){
		setupComboBoxComponents();
		m_ui->comboBoxComponent->setCurrentText(m_mapComponents.key(dialog->currentComponentId()));
	}
	else{
		setupComboBoxComponents();
	}

	modifyEdgeProperty(&VICUS::NetworkEdge::m_componentId, m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
	modifyNodeProperty(&VICUS::NetworkNode::m_componentId, m_mapComponents.value(m_ui->comboBoxComponent->currentText()));
#endif
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
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
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
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}

void SVPropNetworkEditWidget::on_pushButtonConnectBuildings_clicked()
{
	if (!setNetwork())
		return;
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	m_currentNetwork.connectBuildings(false);
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
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
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, newNetwork);
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
	SVUndoModifyExistingNetwork * undoMod = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
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
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
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
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
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
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}
