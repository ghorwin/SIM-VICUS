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

#include "SVPropNetworkPropertiesWidget.h"
#include "ui_SVPropNetworkPropertiesWidget.h"

#include <QMessageBox>

#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordListQt.h>

#include <VICUS_KeywordList.h>
#include <VICUS_utilities.h>

#include <SV_Conversions.h>

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoModifyNetwork.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVStyle.h"
#include "SVUndoTreeNodeState.h"
#include "SVConstants.h"
#include "SVTimeSeriesPreviewDialog.h"


SVPropNetworkPropertiesWidget::SVPropNetworkPropertiesWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);

	// connect with project handler
	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified, this, &SVPropNetworkPropertiesWidget::onModified);

	m_ui->stackedWidget->setCurrentIndex(0);

	// setup combobox node types
	m_ui->comboBoxNodeType->clear();
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
									VICUS::NetworkNode::NT_Mixer);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
									VICUS::NetworkNode::NT_Source);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_SubStation),
									VICUS::NetworkNode::NT_SubStation);

	// setup table widgets
	m_ui->tableWidgetPipes->setColumnCount(2);
	m_ui->tableWidgetPipes->setHorizontalHeaderLabels(QStringList() << QString() << tr("Pipes"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetPipes);
	m_ui->tableWidgetPipes->setSortingEnabled(false);
	m_ui->tableWidgetPipes->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetPipes->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetPipes->horizontalHeader()->setStretchLastSection(true);

	m_ui->tableWidgetHeatExchange->setColumnCount(2);
	m_ui->tableWidgetHeatExchange->setHorizontalHeaderLabels(QStringList() << QString() << tr("Heat Exchange Types"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetHeatExchange);
	m_ui->tableWidgetHeatExchange->setSortingEnabled(false);
	m_ui->tableWidgetHeatExchange->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetHeatExchange->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetHeatExchange->horizontalHeader()->setStretchLastSection(true);

	m_ui->tableWidgetSubNetworks->setColumnCount(2);
	m_ui->tableWidgetSubNetworks->setHorizontalHeaderLabels(QStringList() << QString() << tr("Sub Networks"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSubNetworks);
	m_ui->tableWidgetSubNetworks->setSortingEnabled(false);
	m_ui->tableWidgetSubNetworks->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetSubNetworks->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetSubNetworks->horizontalHeader()->setStretchLastSection(true);

	// validating line edits
	m_ui->lineEditNodeMaximumHeatingDemand->setup(0, std::numeric_limits<double>::max(), tr("Maximum Heating Demand"), false, true);
	m_ui->lineEditNodeXPosition->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("x position of node"), true, true);
	m_ui->lineEditNodeYPosition->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("y position of node"), true, true);
	m_ui->lineEditHeatFlux->setup(0, std::numeric_limits<double>::max(), tr("value of constant heat flux"), true, true);
	m_ui->lineEditTemperature->setup(0, std::numeric_limits<double>::max(), tr("value of constant temperature"), true, true);
	m_ui->lineEditHXTransferCoefficient->setup(0, std::numeric_limits<double>::max(), tr("convective heat exchange coefficient, set =0 to neglect"), true, true);

	onModified(SVProjectHandler::AllModified, nullptr);

	// disable buttons related to table widgets
	on_tableWidgetPipes_itemSelectionChanged();
	on_tableWidgetSubNetworks_itemSelectionChanged();

	m_ui->frameSource->setStyleSheet(".QFrame { background-color: #e68a00; }"); //QColor(230, 138, 0), orange
	m_ui->frameMixer->setStyleSheet(".QFrame { background-color: #77b300; }"); //QColor(119, 179, 0), green
	m_ui->frameSubStation->setStyleSheet(".QFrame { background-color: #006bb3; }"); // QColor(0, 107, 179); // blue

}


SVPropNetworkPropertiesWidget::~SVPropNetworkPropertiesWidget() {
	delete m_ui;
}


void SVPropNetworkPropertiesWidget::on_comboBoxNetworkProperties_currentIndexChanged(int index) {
	setPropertyType(index);
}


void SVPropNetworkPropertiesWidget::setPropertyType(int networkPropertyType) {

	// set page
	switch (NetworkPropertyTypes(networkPropertyType)) {
		case NT_Node			: m_ui->stackedWidget->setCurrentIndex(0); break; // page Node
		case NT_Edge			: m_ui->stackedWidget->setCurrentIndex(1); break; // page Edge
		case NT_SubStation		: m_ui->stackedWidget->setCurrentIndex(2); break; // page SubNetwork
		case NT_HeatExchange	: m_ui->stackedWidget->setCurrentIndex(3); break; // page HeatExchange
	}
	// set coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (NetworkPropertyTypes(networkPropertyType)) {
		case NT_Node			: vs.m_objectColorMode = SVViewState::OCM_NetworkNode ; break;
		case NT_Edge			: vs.m_objectColorMode = SVViewState::OCM_NetworkEdge ; break;
		case NT_SubStation		: vs.m_objectColorMode = SVViewState::OCM_NetworkSubNetworks ; break;
		case NT_HeatExchange	: vs.m_objectColorMode = SVViewState::OCM_NetworkHeatExchange ; break;
	}
	SVViewStateHandler::instance().setViewState(vs);
}


int SVPropNetworkPropertiesWidget::currentPropertyType() {
	return m_ui->comboBoxNetworkProperties->currentIndex();
}


void SVPropNetworkPropertiesWidget::onModified(int modificationType, ModificationInfo * /*data*/) {

	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {

		case SVProjectHandler::NodeStateModified:
		case SVProjectHandler::AllModified:
		case SVProjectHandler::NetworkGeometryChanged: {

			onSelectionChanged();

			// now update UI
			setAllEnabled(false);
			clearUI();

			updateTableWidgets();

			// node(s) selected
			if (!m_currentNodes.empty()){
				updateNodeProperties();
			}
			// edge(s) selected
			if(!m_currentEdges.empty()){
				updateEdgeProperties();
			}
			if(!m_currentEdges.empty() || !m_currentNodes.empty()){
				updateHeatExchangeProperties();
			}

		} break;
		default: ; // just to make compiler happy
	}
}


void SVPropNetworkPropertiesWidget::onSelectionChanged() {

	m_currentEdges.clear();
	m_currentNodes.clear();
	m_currentNetwork = nullptr;

	// get all selected objects of type network, objects must be visible
	std::set<const VICUS::Object *> objs;
	project().selectObjects(objs, VICUS::Project::SG_Network, true, true);

	// We already have the correct networkId and m_currentConstNetwork is already set correctly
	// So we only cast the nodes / edges here
	for (const VICUS::Object* o : objs) {

		// if parent does not exist, this is an entire network or something else
		if (o->m_parent == nullptr){
			continue;
		}

		// else we have nodes or edges which we want to cast
		else {
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
	}

	// We check if the selected nodes, edges belong to different networks,
	// if so: we clear everything. We don't allow editing of different networks at the same time
	std::vector<unsigned int> networkIds;
	for (const VICUS::NetworkEdge *e: m_currentEdges)
		networkIds.push_back(e->m_parent->m_id);
	for (const VICUS::NetworkNode *n: m_currentNodes)
		networkIds.push_back(n->m_parent->m_id);

	if (networkIds.size()>0)
		m_currentNetwork = dynamic_cast<const VICUS::Network*>(project().objectById(networkIds[0]));

	for (unsigned int id: networkIds){
		if (id != networkIds[0]) {
			m_currentEdges.clear();
			m_currentNodes.clear();
			m_currentNetwork = nullptr;
		}
	}
}


void SVPropNetworkPropertiesWidget::updateNodeProperties() {
	Q_ASSERT(!m_currentNodes.empty());

	// enable / disable widgets
	bool uniformNodeType = uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_type);
	m_ui->groupBoxEditNode->setEnabled(uniformNodeType && m_currentEdges.empty());
	m_ui->groupBoxSelectedSubNetwork->setEnabled(!m_currentNodes.empty() && m_currentEdges.empty());

	// if node type is not uniform, no editing will be allowed
	m_ui->comboBoxNodeType->setCurrentIndex(m_ui->comboBoxNodeType->findData(m_currentNodes[0]->m_type));
	m_ui->lineEditNodeMaximumHeatingDemand->setEnabled(m_currentNodes[0]->m_type == VICUS::NetworkNode::NT_SubStation);
	m_ui->lineEditNodeXPosition->setEnabled(m_currentNodes.size() == 1);
	m_ui->lineEditNodeYPosition->setEnabled(m_currentNodes.size() == 1);

	if (m_currentNodes.size() == 1){
		m_ui->labelNodeId->setText(QString("%1").arg(m_currentNodes[0]->m_id));
		m_ui->lineEditNodeName->setText(m_currentNodes[0]->m_displayName);
		m_ui->lineEditNodeXPosition->setValue(m_currentNodes[0]->m_position.m_x);
		m_ui->lineEditNodeYPosition->setValue(m_currentNodes[0]->m_position.m_y);
	}
	else{
		m_ui->labelNodeId->clear();
		m_ui->lineEditNodeName->clear();
		m_ui->lineEditNodeXPosition->clear();
		m_ui->lineEditNodeYPosition->clear();
	}

	if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_maxHeatingDemand))
		m_ui->lineEditNodeMaximumHeatingDemand->setValue(m_currentNodes[0]->m_maxHeatingDemand.value);
	else
		m_ui->lineEditNodeMaximumHeatingDemand->clear();

	// current sub network name
	m_ui->labelSelectedSubNetwork->clear();
	const SVDatabase & db = SVSettings::instance().m_db;
	if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_idSubNetwork)){
		const VICUS::SubNetwork *subNet = db.m_subNetworks[m_currentNodes[0]->m_idSubNetwork];
		if (subNet != nullptr)
			m_ui->labelSelectedSubNetwork->setText(QtExt::MultiLangString2QString(subNet->m_displayName));
	}
}


void SVPropNetworkPropertiesWidget::updateEdgeProperties() {
	Q_ASSERT(!m_currentEdges.empty());

	// enable / disable wiudgets
	bool uniformEdge = uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_idPipe);
	m_ui->groupBoxEdge->setEnabled(uniformEdge);
	m_ui->groupBoxSelectedPipe->setEnabled(!m_currentEdges.empty() && m_currentNodes.empty());

	// update edge length and display name
	if (m_currentEdges.size() == 1){
		m_ui->labelPipeLength->setText(QString("%1 m").arg(m_currentEdges[0]->length()));
		m_ui->lineEditEdgeDisplayName->setText(m_currentEdges[0]->m_displayName);
	}
	else{
		m_ui->labelPipeLength->clear();
		m_ui->lineEditEdgeDisplayName->clear();
	}

	// update supply property
	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_supply))
		m_ui->checkBoxSupplyPipe->setChecked(m_currentEdges[0]->m_supply);
	else
		m_ui->checkBoxSupplyPipe->setCheckState(Qt::CheckState::PartiallyChecked);

	// update pipe label
	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_idPipe)){
		const SVDatabase  & db = SVSettings::instance().m_db;
		const VICUS::NetworkPipe * pipe = db.m_pipes[m_currentEdges[0]->m_idPipe];
		if(pipe == nullptr)
			m_ui->labelSelectedPipe->clear();
		else
			m_ui->labelSelectedPipe->setText(QtExt::MultiLangString2QString(pipe->m_displayName));
	}
	else{
		m_ui->labelSelectedPipe->clear();
	}
}


void SVPropNetworkPropertiesWidget::updateTableWidgets() {

	// enable all network group boxes
	m_ui->groupBoxCurrentHeatExchange->setEnabled(true);
	m_ui->groupBoxCurrentPipes->setEnabled(true);
	m_ui->groupBoxCurrentSubNetworks->setEnabled(true);

	const SVDatabase & db = SVSettings::instance().m_db;
	const VICUS::Project & p = project();

	//  *** Update pipes table widget ***

	std::vector<unsigned int> pipeIds;
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge &e: net.m_edges){
			if (std::find(pipeIds.begin(), pipeIds.end(), e.m_idPipe) == pipeIds.end() &&
				e.m_idPipe != VICUS::INVALID_ID)
				pipeIds.push_back(e.m_idPipe);
		}
	}
	// sort in ascending order of ids
	std::sort(pipeIds.begin(), pipeIds.end());

	int currentRow = m_ui->tableWidgetPipes->currentRow();
	m_ui->tableWidgetPipes->blockSignals(true);
	m_ui->tableWidgetPipes->clearContents();
	m_ui->tableWidgetPipes->setRowCount((int)pipeIds.size());
	int row = 0;
	for (unsigned int id: pipeIds){
		 const VICUS::NetworkPipe *pipe = db.m_pipes[id];
		 QTableWidgetItem * item = new QTableWidgetItem();
		 // special handling for components with "invalid" component id
		 if (pipe == nullptr)
			 item->setBackground(QColor(64,64,64));
		 else
			 item->setBackground(pipe->m_color);
		 item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		 item->setData(Qt::UserRole, id);
		 m_ui->tableWidgetPipes->setItem(row, 0, item);

		 item = new QTableWidgetItem();
		 if (pipe == nullptr)
			 item->setText(tr("<invalid pipe id>"));
		 else
			 item->setText(QtExt::MultiLangString2QString(pipe->m_displayName));
		 item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		 item->setData(Qt::UserRole, id);
		 m_ui->tableWidgetPipes->setItem(row, 1, item);

		 ++row;
	}
	// reselect row
	m_ui->tableWidgetPipes->blockSignals(false);
	m_ui->tableWidgetPipes->selectRow(std::min(currentRow, m_ui->tableWidgetPipes->rowCount()-1));


	//  *** Update sub networks table widget ***

	std::vector<unsigned int> subNetworkIds;
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkNode &n: net.m_nodes){
			if (std::find(subNetworkIds.begin(), subNetworkIds.end(), n.m_idSubNetwork) == subNetworkIds.end() &&
					n.m_idSubNetwork != VICUS::INVALID_ID)
				subNetworkIds.push_back(n.m_idSubNetwork);
		}
	}

	currentRow = m_ui->tableWidgetSubNetworks->currentRow();
	m_ui->tableWidgetSubNetworks->blockSignals(true);
	m_ui->tableWidgetSubNetworks->clearContents();
	m_ui->tableWidgetSubNetworks->setRowCount((int)subNetworkIds.size());
	row = 0;
	for (unsigned int id: subNetworkIds){
		 const VICUS::SubNetwork *subNet = db.m_subNetworks[id];
		 QTableWidgetItem * item = new QTableWidgetItem();
		 // special handling for components with "invalid" component id
		 if (subNet == nullptr)
			 item->setBackground(QColor(64,64,64));
		 else
			 item->setBackground(subNet->m_color);
		 item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		 item->setData(Qt::UserRole, id);
		 m_ui->tableWidgetSubNetworks->setItem(row, 0, item);

		 item = new QTableWidgetItem();
		 item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		 if (subNet == nullptr)
			 item->setText(tr("<invalid sub network id>"));
		 else
			 item->setText(QtExt::MultiLangString2QString(subNet->m_displayName));
		item->setData(Qt::UserRole, id);
		 m_ui->tableWidgetSubNetworks->setItem(row, 1, item);

		 ++row;
	}
	// reselect row
	m_ui->tableWidgetSubNetworks->blockSignals(false);
	m_ui->tableWidgetSubNetworks->selectRow(std::min(currentRow, m_ui->tableWidgetSubNetworks->rowCount()-1));


	// *** update heat exchange table widget

	// collect hx types used in this network
	std::vector<NANDRAD::HydraulicNetworkHeatExchange::ModelType> currentHxTypes;
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge &e: net.m_edges){
			if (std::find(currentHxTypes.begin(), currentHxTypes.end(), e.m_heatExchange.m_modelType) == currentHxTypes.end())
				currentHxTypes.push_back(e.m_heatExchange.m_modelType);
		}
	}
	for (const VICUS::Network &net: p.m_geometricNetworks) {
		for (const VICUS::NetworkNode &n: net.m_nodes){
			if (std::find(currentHxTypes.begin(), currentHxTypes.end(), n.m_heatExchange.m_modelType) == currentHxTypes.end())
				currentHxTypes.push_back(n.m_heatExchange.m_modelType);
		}
	}

	// update heat exchange table widget
	m_ui->tableWidgetHeatExchange->blockSignals(true);
	m_ui->tableWidgetHeatExchange->clearContents();
	m_ui->tableWidgetHeatExchange->setRowCount((int)currentHxTypes.size());
	row = 0;
	for (NANDRAD::HydraulicNetworkHeatExchange::ModelType type: currentHxTypes){
		 QTableWidgetItem * item = new QTableWidgetItem();
		 item->setBackground(VICUS::Network::colorHeatExchangeType(type));
		 item->setFlags(Qt::ItemIsEnabled); // cannot select color item!
		 m_ui->tableWidgetHeatExchange->setItem(row, 0, item);
		 item = new QTableWidgetItem();
		 if (type == NANDRAD::HydraulicNetworkHeatExchange::NUM_T)
			 item->setText("Adiabatic");
		 else
			item->setText(NANDRAD::KeywordListQt::Keyword("HydraulicNetworkHeatExchange::ModelType", (int)type));
		 item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		 m_ui->tableWidgetHeatExchange->setItem(row, 1, item);

		 ++row;
	}
	m_ui->tableWidgetHeatExchange->blockSignals(false);
}


void SVPropNetworkPropertiesWidget::updateHeatExchangeProperties() {

	// clear widgets
	m_ui->comboBoxHeatExchangeType->clear();
	m_ui->lineEditHeatFlux->clear();
	m_ui->lineEditTemperature->clear();
	m_ui->lineEditHXTransferCoefficient->clear();
	m_ui->labelHeatExchangeSpline->setText("");

	// in case we have a mixed selection of nodes and edges
	// or no object selected at all: don't proceed
	m_ui->groupBoxHeatExchange->setEnabled(false);
	if ((!m_currentEdges.empty() && !m_currentNodes.empty()) ||
		(m_currentEdges.empty() && m_currentNodes.empty()) ){
		return;
	}


	// *** populate combobox

	// get the current component modelType in order to know which heat exchange types are allowed
	const SVDatabase & db = SVSettings::instance().m_db;
	VICUS::NetworkComponent::ModelType modelType = VICUS::NetworkComponent::NUM_MT;
	// if we have node(s)
	if (!m_currentNodes.empty()){
		const VICUS::SubNetwork *sub = db.m_subNetworks[m_currentNodes[0]->m_idSubNetwork];
		if (sub == nullptr)
			return;
		const VICUS::NetworkComponent *comp = sub->heatExchangeComponent(db.m_networkComponents);
		if (comp == nullptr)
			return;
		modelType = comp->m_modelType;
	}
	// if we have edge(s)
	else if (!m_currentEdges.empty()){
		modelType = VICUS::NetworkComponent::MT_DynamicPipe;
	}
	else
		return;

	// now get the available heat exchange types
	std::vector<NANDRAD::HydraulicNetworkHeatExchange::ModelType> availableHxTypes =
			NANDRAD::HydraulicNetworkHeatExchange::availableHeatExchangeTypes(VICUS::NetworkComponent::nandradNetworkComponentModelType(modelType));

	// if no hx type is possible return
	if (availableHxTypes.empty())
		return;

	// ui editing is possible now
	m_ui->groupBoxHeatExchange->setEnabled(true);

	// populate the combobox
	for (unsigned int i: availableHxTypes){
		if (i == NANDRAD::HydraulicNetworkHeatExchange::NUM_T)
			m_ui->comboBoxHeatExchangeType->addItem("Adiabatic", NANDRAD::HydraulicNetworkHeatExchange::NUM_T);
		else
			m_ui->comboBoxHeatExchangeType->addItem(NANDRAD::KeywordListQt::Description
													("HydraulicNetworkHeatExchange::ModelType", (int)i), i);
	}
	m_ui->comboBoxHeatExchangeType->setCurrentIndex(-1);


	// *** update line edits

	// disable all
	m_ui->labelTemperature->setEnabled(false);
	m_ui->lineEditTemperature->setEnabled(false);
	m_ui->labelHXTransferCoefficient->setEnabled(false);
	m_ui->lineEditHXTransferCoefficient->setEnabled(false);
	m_ui->labelHeatFlux->setEnabled(false);
	m_ui->lineEditHeatFlux->setEnabled(false);
	m_ui->labelDataFile->setEnabled(false);
	m_ui->labelHeatExchangeSpline->setEnabled(false);
	m_ui->toolButtonHeatExchangeSpline->setEnabled(false);
	m_ui->labelZoneId->setEnabled(false);
	m_ui->comboBoxZoneId->setEnabled(false);

	// get current hx properties
	NANDRAD::HydraulicNetworkHeatExchange hx;
	if (uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_heatExchange))
		hx = m_currentEdges[0]->m_heatExchange;
	else if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_heatExchange))
		hx = m_currentNodes[0]->m_heatExchange;
	else
		return;

	// enable widgets based on current heat exchange type
	switch (hx.m_modelType) {
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant:{
			m_ui->labelHeatFlux->setEnabled(true);
			m_ui->lineEditHeatFlux->setEnabled(true);
			break;
		}
		case NANDRAD::HydraulicNetworkHeatExchange ::T_TemperatureConstant:{
			m_ui->labelTemperature->setEnabled(true);
			m_ui->lineEditTemperature->setEnabled(true);
			m_ui->labelHXTransferCoefficient->setEnabled(true);
			m_ui->lineEditHXTransferCoefficient->setEnabled(true);
			break;
		}
		case NANDRAD::HydraulicNetworkHeatExchange ::T_TemperatureSpline:
		case NANDRAD::HydraulicNetworkHeatExchange ::T_TemperatureSplineEvaporator:{
			m_ui->labelDataFile->setEnabled(true);
			m_ui->labelHeatExchangeSpline->setEnabled(true);
			m_ui->labelHXTransferCoefficient->setEnabled(true);
			m_ui->lineEditHXTransferCoefficient->setEnabled(true);
			m_ui->toolButtonHeatExchangeSpline->setEnabled(true);
			break;
		}
		case NANDRAD::HydraulicNetworkHeatExchange ::T_HeatLossSplineCondenser:
		case NANDRAD::HydraulicNetworkHeatExchange ::T_HeatingDemandSpaceHeating:
		case NANDRAD::HydraulicNetworkHeatExchange ::T_HeatLossSpline:{
			m_ui->labelDataFile->setEnabled(true);
			m_ui->labelHeatExchangeSpline->setEnabled(true);
			m_ui->toolButtonHeatExchangeSpline->setEnabled(true);
			break;
		}
		case NANDRAD::HydraulicNetworkHeatExchange ::T_TemperatureConstructionLayer:
		case NANDRAD::HydraulicNetworkHeatExchange ::T_TemperatureZone:
		case NANDRAD::HydraulicNetworkHeatExchange ::NUM_T:
			break;
	}

	// update combobox and line edits
	m_ui->comboBoxHeatExchangeType->setCurrentIndex(m_ui->comboBoxHeatExchangeType->findData(hx.m_modelType));

	if (!hx.m_para[NANDRAD::HydraulicNetworkHeatExchange ::P_HeatLoss].empty())
		m_ui->lineEditHeatFlux->setValue(hx.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss].value);

	if (!hx.m_para[NANDRAD::HydraulicNetworkHeatExchange ::P_Temperature].empty())
		m_ui->lineEditTemperature->setValue(hx.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_Temperature].get_value("C"));

	if (!hx.m_para[NANDRAD::HydraulicNetworkHeatExchange ::P_ExternalHeatTransferCoefficient].empty())
		m_ui->lineEditHXTransferCoefficient->setValue(hx.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].value);

	if (hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange ::SPL_HeatLoss].m_tsvFile.isValid())
		m_ui->labelHeatExchangeSpline->setText(QString::fromStdString(
												hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss].m_tsvFile.str()));

	if (hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange ::SPL_Temperature].m_tsvFile.isValid())
		m_ui->labelHeatExchangeSpline->setText(QString::fromStdString(
												hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature].m_tsvFile.str()));
}


void SVPropNetworkPropertiesWidget::clearUI(){

	m_ui->labelNodeId->clear();
	m_ui->lineEditNodeXPosition->clear();
	m_ui->lineEditNodeYPosition->clear();
	m_ui->lineEditNodeName->clear();
	m_ui->lineEditNodeMaximumHeatingDemand->clear();

	m_ui->labelPipeLength->clear();
	m_ui->lineEditEdgeDisplayName->clear();
	m_ui->checkBoxSupplyPipe->setChecked(false);
	m_ui->tableWidgetPipes->clearContents();

	m_ui->tableWidgetSubNetworks->clearContents();

	m_ui->lineEditHeatFlux->clear();
	m_ui->lineEditTemperature->clear();
	m_ui->lineEditHXTransferCoefficient->clear();
	m_ui->labelHeatExchangeSpline->clear();
	m_ui->comboBoxHeatExchangeType->clear();
	m_ui->tableWidgetHeatExchange->clearContents();
}


void SVPropNetworkPropertiesWidget::setAllEnabled(bool enabled) {
	m_ui->groupBoxEditNode->setEnabled(enabled);
	m_ui->groupBoxEdge->setEnabled(enabled);
	m_ui->groupBoxHeatExchange->setEnabled(enabled);
	m_ui->groupBoxCurrentHeatExchange->setEnabled(enabled);
	m_ui->groupBoxCurrentPipes->setEnabled(enabled);
	m_ui->groupBoxSelectedPipe->setEnabled(enabled);
	m_ui->groupBoxSelectedSubNetwork->setEnabled(enabled);
	m_ui->groupBoxCurrentSubNetworks->setEnabled(enabled);
}


void SVPropNetworkPropertiesWidget::modifyHeatExchangeProperties() {

	NANDRAD::HydraulicNetworkHeatExchange hx;
	if (!m_currentNodes.empty())
		hx = m_currentNodes[0]->m_heatExchange;
	else if (!m_currentEdges.empty())
		hx = m_currentEdges[0]->m_heatExchange;
	else
		return; // this should never happen

	// store splines
	NANDRAD::LinearSplineParameter splTemp = hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature];
	NANDRAD::LinearSplineParameter splHeatLoss = hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss];

	// now clear all
	hx = NANDRAD::HydraulicNetworkHeatExchange();

	// set model type
	NANDRAD::HydraulicNetworkHeatExchange::ModelType modelType =
			NANDRAD::HydraulicNetworkHeatExchange::ModelType(m_ui->comboBoxHeatExchangeType->currentData().toUInt());
	hx.m_modelType = modelType;

	// set parameters depending on model type
	switch (hx.m_modelType) {
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant: {
			// set heat loss
			if (m_ui->lineEditHeatFlux->isValid())
				NANDRAD::KeywordList::setParameter(hx.m_para, "HydraulicNetworkHeatExchange::para_t",
												 NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss,
												 m_ui->lineEditHeatFlux->value());
			else
				hx.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss].clear();
		} break;

		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstant: {
			// set temperature
			if (m_ui->lineEditTemperature->isValid())
				NANDRAD::KeywordList::setParameter(hx.m_para, "HydraulicNetworkHeatExchange::para_t",
												 NANDRAD::HydraulicNetworkHeatExchange::P_Temperature,
												 m_ui->lineEditTemperature->value());
			else
				hx.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_Temperature].clear();
			// set external hx coefficient
			if (m_ui->lineEditHXTransferCoefficient->isValid())
				NANDRAD::KeywordList::setParameter(hx.m_para, "HydraulicNetworkHeatExchange::para_t",
												 NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient,
												 m_ui->lineEditHXTransferCoefficient->value());
			else
				hx.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].clear();
		} break;

		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline: {
			// reset spline
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature] = splTemp;
			// set external hx coefficient
			if (m_ui->lineEditHXTransferCoefficient->isValid())
				NANDRAD::KeywordList::setParameter(hx.m_para, "HydraulicNetworkHeatExchange::para_t",
												 NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient,
												 m_ui->lineEditHXTransferCoefficient->value());
			else
				hx.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_ExternalHeatTransferCoefficient].clear();

		} break;

		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator:
			// reset spline
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature] = splTemp;
		break;
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
			// reset spline
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss] = splHeatLoss;
		break;
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureZone:
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer:
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatingDemandSpaceHeating:
		case NANDRAD::HydraulicNetworkHeatExchange::NUM_T:
			break;
	}

	// set hx properties to nodes / edges
	if (!m_currentNodes.empty())
		modifyNodeProperty(&VICUS::NetworkNode::m_heatExchange, hx);
	if (!m_currentEdges.empty())
		modifyEdgeProperty(&VICUS::NetworkEdge::m_heatExchange, hx);
}



void SVPropNetworkPropertiesWidget::on_comboBoxNodeType_activated(int index) {
	modifyNodeProperty(&VICUS::NetworkNode::m_type, VICUS::NetworkNode::NodeType(
		m_ui->comboBoxNodeType->currentData().toUInt()) );
}


void SVPropNetworkPropertiesWidget::on_lineEditNodeXPosition_editingFinishedSuccessfully() {
	if (!m_ui->lineEditNodeXPosition->isValid() || !m_ui->lineEditNodeYPosition->isValid())
		return;
	IBKMK::Vector3D	vec(m_ui->lineEditNodeXPosition->value(), m_ui->lineEditNodeYPosition->value(), 0);
	modifyNodeProperty(&VICUS::NetworkNode::m_position, vec);
}


void SVPropNetworkPropertiesWidget::on_lineEditNodeYPosition_editingFinishedSuccessfully() {
	if (!m_ui->lineEditNodeXPosition->isValid() || !m_ui->lineEditNodeYPosition->isValid())
		return;
	IBKMK::Vector3D	vec(m_ui->lineEditNodeXPosition->value(), m_ui->lineEditNodeYPosition->value(), 0);
	modifyNodeProperty(&VICUS::NetworkNode::m_position, vec);

}
void SVPropNetworkPropertiesWidget::on_checkBoxSupplyPipe_clicked()
{
	modifyEdgeProperty(&VICUS::NetworkEdge::m_supply, m_ui->checkBoxSupplyPipe->isChecked());
}


void SVPropNetworkPropertiesWidget::on_lineEditNodeMaximumHeatingDemand_editingFinishedSuccessfully() {
	if (m_ui->lineEditNodeMaximumHeatingDemand->isValid())
		modifyNodeProperty(&VICUS::NetworkNode::m_maxHeatingDemand,
						   IBK::Parameter("MaxHeatingDemand", m_ui->lineEditNodeMaximumHeatingDemand->value(), "W"));
}

void SVPropNetworkPropertiesWidget::on_lineEditHeatFlux_editingFinished()
{
	modifyHeatExchangeProperties();
}


void SVPropNetworkPropertiesWidget::on_lineEditTemperature_editingFinished()
{
	modifyHeatExchangeProperties();
}


void SVPropNetworkPropertiesWidget::on_heatExchangeDataFile_editingFinished()
{
	modifyHeatExchangeProperties();
}

void SVPropNetworkPropertiesWidget::on_lineEditHXTransferCoefficient_editingFinished()
{
	modifyHeatExchangeProperties();
}

void SVPropNetworkPropertiesWidget::on_lineEditNodeName_editingFinished()
{
	modifyNodeProperty(&VICUS::NetworkNode::m_displayName, m_ui->lineEditNodeName->text());
}


void SVPropNetworkPropertiesWidget::on_lineEditEdgeDisplayName_editingFinished()
{
	modifyEdgeProperty(&VICUS::NetworkEdge::m_displayName, m_ui->lineEditEdgeDisplayName->text());
}


void SVPropNetworkPropertiesWidget::on_comboBoxHeatExchangeType_activated(int /*index*/)
{
	modifyHeatExchangeProperties();
}


void SVPropNetworkPropertiesWidget::on_pushButtonAssignPipe_clicked()
{
	unsigned int currentId = 0;
	if (m_currentEdges.size() > 0)
		currentId = m_currentEdges[0]->m_idPipe;
	unsigned int newId = SVMainWindow::instance().dbPipeEditDialog()->select(currentId);
	modifyEdgeProperty(&VICUS::NetworkEdge::m_idPipe, newId);
}

void SVPropNetworkPropertiesWidget::on_pushButtonEditPipe_clicked()
{
	unsigned int currentId = 0;
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item != nullptr)
		currentId = item->data(Qt::UserRole).toUInt();
	SVMainWindow::instance().dbPipeEditDialog()->edit(currentId);
	updateTableWidgets(); // for color update
}


void SVPropNetworkPropertiesWidget::on_pushButtonEditSubNetworks_clicked()
{
	unsigned int currentId = 0;
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item != nullptr)
		currentId = item->data(Qt::UserRole).toUInt();
	SVMainWindow::instance().dbSubNetworkEditDialog()->edit(currentId);
	updateTableWidgets(); // for color update
}


void SVPropNetworkPropertiesWidget::on_pushButtonAssignSubNetwork_clicked()
{
	unsigned int currentId = 0;
	if (m_currentNodes.size() > 0)
		currentId = m_currentNodes[0]->m_idSubNetwork;
	unsigned int newId = SVMainWindow::instance().dbSubNetworkEditDialog()->select(currentId);
	modifyNodeProperty(&VICUS::NetworkNode::m_idSubNetwork, newId);
}



void SVPropNetworkPropertiesWidget::on_pushButtonRecalculateLength_clicked() {
	VICUS::Project p = project();
	VICUS::Network *network = VICUS::element(p.m_geometricNetworks, m_currentNetwork->m_id);
	Q_ASSERT(network!=nullptr);

	network->updateNodeEdgeConnectionPointers();
	for (const VICUS::NetworkEdge * edge: m_currentEdges){
		network->edge(edge->nodeId1(), edge->nodeId2())->setLengthFromCoordinates();
	}

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), *network);
	undo->push(); // modifies project and updates views
	updateTableWidgets();
}


void SVPropNetworkPropertiesWidget::on_pushButtonSelectEdgesWithPipe_clicked() {

	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item == nullptr)
		return;
	unsigned int pipeId = item->data(Qt::UserRole).toUInt();

	// collect edges
	std::set<unsigned int> edgeIds;
	const VICUS::Project &p = project();
	for (const VICUS::Network &network: p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge &e: network.m_edges) {
			if (e.m_idPipe == pipeId)
				edgeIds.insert(e.m_id);
		}
	}

	const VICUS::NetworkPipe * pipe = SVSettings::instance().m_db.m_pipes[pipeId];
	QString undoText;
	if (pipe != nullptr)
		undoText = tr("Select edges with pipe '%1'.").arg(QtExt::MultiLangString2QString(pipe->m_displayName));
	else
		undoText = tr("Select edges with invalid/missing pipe.");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, edgeIds, true);
	undo->push();
}


void SVPropNetworkPropertiesWidget::on_pushButtonExchangePipe_clicked() {

	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item == nullptr)
		return;
	unsigned int oldId = item->data(Qt::UserRole).toUInt();

	// get new id
	unsigned int newId = SVMainWindow::instance().dbPipeEditDialog()->select(oldId);
	if (newId == oldId || newId==VICUS::INVALID_ID)
		return;

	VICUS::Project p = project();
	for (VICUS::Network &network: p.m_geometricNetworks) {
		// modify edges
		for (VICUS::NetworkEdge &e: network.m_edges) {
			if (e.m_idPipe == oldId)
				e.m_idPipe = newId;
		}
		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), network);
		undo->push(); // modifies project and updates views
	}
}


void SVPropNetworkPropertiesWidget::on_tableWidgetPipes_itemSelectionChanged() {
	bool enabled =  m_ui->tableWidgetPipes->currentRow() != -1;
	m_ui->pushButtonEditPipe->setEnabled(enabled);
	m_ui->pushButtonSelectEdgesWithPipe->setEnabled(enabled);
	m_ui->pushButtonExchangePipe->setEnabled(enabled);
}


void SVPropNetworkPropertiesWidget::on_pushButtonExchangeSubNetwork_clicked() {

	// get id from current item
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item == nullptr)
		return;
	unsigned int oldId = item->data(Qt::UserRole).toUInt();

	// get new id
	unsigned int newId = SVMainWindow::instance().dbSubNetworkEditDialog()->select(oldId);
	if (newId == oldId || newId==VICUS::INVALID_ID)
		return;

	VICUS::Project p = project();
	for (VICUS::Network &network: p.m_geometricNetworks) {
		// modify nodes
		for (VICUS::NetworkNode &n: network.m_nodes) {
			if (n.m_idSubNetwork == oldId)
				n.m_idSubNetwork = newId;
		}
		// undo
		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), network);
		undo->push(); // modifies project and updates views
	}
	updateTableWidgets(); // for color update
}


void SVPropNetworkPropertiesWidget::on_pushButtonSelectNodesWithSubNetwork_clicked() {

	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item == nullptr)
		return;
	unsigned int currentId = item->data(Qt::UserRole).toUInt();

	// collect sub networks
	std::set<unsigned int> nodeIds;
	const VICUS::Project &p = project();
	for (const VICUS::Network &network: p.m_geometricNetworks) {
		for (const VICUS::NetworkNode &n: network.m_nodes) {
			if (n.m_idSubNetwork == currentId)
				nodeIds.insert(n.m_id);
		}
	}

	const VICUS::SubNetwork * sub = SVSettings::instance().m_db.m_subNetworks[currentId];
	QString undoText;
	if (sub != nullptr)
		undoText = tr("Select nodes with sub network '%1'.").arg(QtExt::MultiLangString2QString(sub->m_displayName));
	else
		undoText = tr("Select nodes with invalid/missing sub network.");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, nodeIds, true);
	undo->push();
}


void SVPropNetworkPropertiesWidget::on_tableWidgetSubNetworks_itemSelectionChanged() {
	bool enabled = m_ui->tableWidgetSubNetworks->currentRow() != -1;
	m_ui->pushButtonEditSubNetworks->setEnabled(enabled);
	m_ui->pushButtonSelectNodesWithSubNetwork->setEnabled(enabled);
	m_ui->pushButtonExchangeSubNetwork->setEnabled(enabled);
}



void SVPropNetworkPropertiesWidget::on_toolButtonHeatExchangeSpline_clicked() {

	Q_ASSERT(m_currentNetwork!=nullptr);

	// get type of spline (temperature or heat flux)
	NANDRAD::HydraulicNetworkHeatExchange::ModelType modelType =
			NANDRAD::HydraulicNetworkHeatExchange::ModelType(m_ui->comboBoxHeatExchangeType->currentData().toUInt());
	NANDRAD::HydraulicNetworkHeatExchange::splinePara_t splType = NANDRAD::HydraulicNetworkHeatExchange::NUM_SPL;
	switch (modelType) {
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline:
		case NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSplineEvaporator:
			splType = NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature;
		break;
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
		case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser:
			splType = NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss;
		break;
		default:
			return; // we can only set a spline for the above model types
	}

	NANDRAD::HydraulicNetworkHeatExchange hx;
	if (!m_currentNodes.empty())
		hx = m_currentNodes[0]->m_heatExchange;
	else if (!m_currentEdges.empty())
		hx = m_currentEdges[0]->m_heatExchange;
	else
		return; // this should never happen

	NANDRAD::LinearSplineParameter &spl = hx.m_splPara[splType];
	SVTimeSeriesPreviewDialog *diag = new SVTimeSeriesPreviewDialog(this);
	diag->select(spl);

	// naming needs to be always like this
	if (splType == NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature)
		spl.m_name = "Temperature";
	else if (splType == NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss)
		spl.m_name = "HeatLoss";
	else
		return;

	// set hx properties to nodes / edges
	if (!m_currentNodes.empty())
		modifyNodeProperty(&VICUS::NetworkNode::m_heatExchange, hx);
	if (!m_currentEdges.empty())
		modifyEdgeProperty(&VICUS::NetworkEdge::m_heatExchange, hx);
}



template <typename TEdgeProp, typename Tval>
void SVPropNetworkPropertiesWidget::modifyEdgeProperty(TEdgeProp property, const Tval & value) {

	Q_ASSERT(m_currentNetwork!=nullptr);
	VICUS::Project p = project();
	VICUS::Network *network = VICUS::element(p.m_geometricNetworks, m_currentNetwork->m_id);
	Q_ASSERT(network!=nullptr);

	Q_ASSERT(!m_currentEdges.empty());
	for (const VICUS::NetworkEdge * edgeConst: m_currentEdges)
		network->edgeById(edgeConst->m_id)->*property = value;

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), *network);
	undo->push(); // modifies project and updates views
	return;
}


template<typename TNodeProp, typename Tval>
void SVPropNetworkPropertiesWidget::modifyNodeProperty(TNodeProp property, const Tval &value) {

	Q_ASSERT(m_currentNetwork!=nullptr);
	VICUS::Project p = project();
	VICUS::Network *network = VICUS::element(p.m_geometricNetworks, m_currentNetwork->m_id);
	Q_ASSERT(network!=nullptr);

	Q_ASSERT(!m_currentNodes.empty());
	for (const VICUS::NetworkNode * nodeConst: m_currentNodes)
		network->nodeById(nodeConst->m_id)->*property = value;

	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), *network);
	undo->push(); // modifies project and updates views
}


