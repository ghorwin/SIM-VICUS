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

#include "SVPropNetworkEditWidget.h"
#include "ui_SVPropNetworkEditWidget.h"

#include <QMessageBox>

#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordListQt.h>
#include <NANDRAD_KeywordList.h>

#include <VICUS_KeywordList.h>
#include <VICUS_utilities.h>


#include <QtExt_Conversions.h>

#include "Vic3DWireFrameObject.h"

#include "SVViewStateHandler.h"
#include "SVNavigationTreeWidget.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoDeleteNetwork.h"
#include "SVUndoModifyNetwork.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVNetworkDialogSelectPipes.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVDBNetworkFluidEditWidget.h"
#include "SVStyle.h"
#include "SVPropModeSelectionWidget.h"
#include "SVUndoTreeNodeState.h"


SVPropNetworkEditWidget::SVPropNetworkEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkEditWidget)
{
	m_ui->setupUi(this);

	// network property page
	m_ui->stackedWidget->setCurrentIndex(0);

	// setup combobox node types
	m_ui->comboBoxNodeType->clear();
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Mixer),
									VICUS::NetworkNode::NT_Mixer);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Source),
									VICUS::NetworkNode::NT_Source);
	m_ui->comboBoxNodeType->addItem(VICUS::KeywordList::Keyword("NetworkNode::NodeType", VICUS::NetworkNode::NT_Building),
									VICUS::NetworkNode::NT_Building);

	// connect browse filename widget
	connect(m_ui->widgetBrowseFileNameTSVFile, SIGNAL(editingFinished()), this, SLOT(on_heatExchangeDataFile_editingFinished()));
	// and set up
	m_ui->widgetBrowseFileNameTSVFile->setup("", true, true, tr("Data files (*.tsv)"), SVSettings::instance().m_dontUseNativeDialogs);

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
	m_ui->lineEditNodeMaxHeatingDemand->setup(0, std::numeric_limits<double>::max(), tr("Maximum Heating Demand"), false, true);
	m_ui->lineEditThresholdSmallEdge->setup(0, std::numeric_limits<double>::max(), tr("Maximum Heating Demand"), false, true);
	m_ui->lineEditNodeX->setup(0, std::numeric_limits<double>::max(), tr("x position of node"), true, true);
	m_ui->lineEditNodeY->setup(0, std::numeric_limits<double>::max(), tr("y position of node"), true, true);
	m_ui->lineEditHeatFlux->setup(0, std::numeric_limits<double>::max(), tr("value of constant heat flux"), true, true);
	m_ui->lineEditTemperature->setup(0, std::numeric_limits<double>::max(), tr("value of constant temperature"), true, true);
	m_ui->lineEditThresholdSmallEdge->setup(0, std::numeric_limits<double>::max(), tr("edges smaller than this value will be cutted"), false, true);
	m_ui->lineEditNodeMaxHeatingDemand->setup(0, std::numeric_limits<double>::max(), tr("maximum heating demand at this node"), false, true);
	m_ui->lineEditHXTransferCoefficient->setup(0, std::numeric_limits<double>::max(), tr("convective heat exchange coefficient, set =0 to neglect"), true, true);

}


SVPropNetworkEditWidget::~SVPropNetworkEditWidget() {
	delete m_ui;
}


void SVPropNetworkEditWidget::setPropertyMode(int propertyIndex) {
	qDebug() << "SVPropNetworkEditWidget::setPropertyMode: propertyIndex =" << propertyIndex;

	switch (propertyIndex) {
		case 0 : m_ui->stackedWidget->setCurrentIndex(0); break; // page Network
		case 1 : m_ui->stackedWidget->setCurrentIndex(1); break; // page Node
		case 2 : m_ui->stackedWidget->setCurrentIndex(2); break; // page Edge
		case 3 : m_ui->stackedWidget->setCurrentIndex(3); break; // page SubNetwork
		case 4 : m_ui->stackedWidget->setCurrentIndex(4); break; // page HeatExchange
	}
}


void SVPropNetworkEditWidget::selectionChanged(unsigned int networkId) {

	m_currentEdges.clear();
	m_currentNodes.clear();

	// set current network based on given id
	m_currentConstNetwork = VICUS::element(project().m_geometricNetworks, networkId);

	// can be nullptr if no network was selected
	if (m_currentConstNetwork != nullptr){

		// get all selected objects of type network, objects must be visible
		std::set<const VICUS::Object *> objs;
		project().selectObjects(objs, VICUS::Project::SG_Network, true, true);

		// We already have the correct networkId and m_currentConstNetwork is already set correctly
		// So we only cast the nodes / edges here
		for (const VICUS::Object* o : objs) {

			// if parent does not exist, this is an entire network
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
	}

	// now update UI
	setAllEnabled(false);
	if (m_currentConstNetwork != nullptr){
		updateNetworkProperties();
	}
	else{
		clearUI();
		return;
	}

	// node(s) selected
	if (!m_currentNodes.empty() && m_currentEdges.empty()){
		updateNodeProperties();
	}
	// edge(s) selected
	else if(m_currentNodes.empty() && !m_currentEdges.empty()){
		updateEdgeProperties();
	}
	// none selected
	else if (m_currentConstNetwork == nullptr){
		clearUI();
	}
}


void SVPropNetworkEditWidget::updateNodeProperties() {
	Q_ASSERT(!m_currentNodes.empty());

	// enable / disable wiudgets
	bool uniformNodeType = uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_type);
	m_ui->groupBoxNode->setEnabled(uniformNodeType);
	m_ui->groupBoxSelectedSubNetwork->setEnabled(!m_currentNodes.empty() && m_currentEdges.empty());

	// if node type is not uniform, no editing will be allowed
	m_ui->comboBoxNodeType->setCurrentIndex(m_ui->comboBoxNodeType->findData(m_currentNodes[0]->m_type));
	m_ui->lineEditNodeMaxHeatingDemand->setEnabled(m_currentNodes[0]->m_type == VICUS::NetworkNode::NT_Building);
	m_ui->lineEditNodeX->setEnabled(m_currentNodes.size() == 1);
	m_ui->lineEditNodeY->setEnabled(m_currentNodes.size() == 1);

	if (m_currentNodes.size() == 1){
		m_ui->labelNodeId->setText(QString("%1").arg(m_currentNodes[0]->m_id));
		m_ui->lineEditNodeDisplayName->setText(m_currentNodes[0]->m_displayName);
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
		m_ui->lineEditNodeMaxHeatingDemand->setValue(m_currentNodes[0]->m_maxHeatingDemand.value);
	else
		m_ui->lineEditNodeMaxHeatingDemand->clear();

	// current sub network name
	m_ui->labelSelectedSubNetwork->clear();
	const SVDatabase & db = SVSettings::instance().m_db;
	if (uniformProperty(m_currentNodes, &VICUS::NetworkNode::m_idSubNetwork)){
		const VICUS::SubNetwork *subNet = db.m_subNetworks[m_currentNodes[0]->m_idSubNetwork];
		if (subNet != nullptr)
			m_ui->labelSelectedSubNetwork->setText(QtExt::MultiLangString2QString(subNet->m_displayName));
	}

	//  *** Update hx properties
	updateHeatExchangeProperties();
}


void SVPropNetworkEditWidget::updateEdgeProperties() {

	Q_ASSERT(!m_currentEdges.empty());

	// enable / disable wiudgets
	bool uniformEdge = uniformProperty(m_currentEdges, &VICUS::NetworkEdge::m_idPipe);
	m_ui->groupBoxEdge->setEnabled(uniformEdge);
	m_ui->groupBoxSelectedPipe->setEnabled(!m_currentEdges.empty() && m_currentNodes.empty());

	// update edge length and display name
	if (m_currentEdges.size() == 1){
		m_ui->labelPipeLength->setText(QString("%1 m").arg(m_currentEdges[0]->length()));
		m_ui->lineEditEdgeDisplayName->setText(m_currentEdges[0]->m_displayName);
		m_ui->labelTempChangeIndicator->setText(QString("%1 K/K").arg(m_currentEdges[0]->m_tempChangeIndicator));
	}
	else{
		m_ui->labelPipeLength->clear();
		m_ui->lineEditEdgeDisplayName->clear();
		m_ui->labelTempChangeIndicator->clear();
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

	//  *** Update hx properties
	updateHeatExchangeProperties();
}


void SVPropNetworkEditWidget::updateNetworkProperties()
{
	Q_ASSERT(m_currentConstNetwork != nullptr);

	// enable all network group boxes
	m_ui->groupBoxProperties->setEnabled(true);
	m_ui->groupBoxSizePipes->setEnabled(true);
	m_ui->groupBoxVisualisation->setEnabled(true);
	m_ui->groupBoxEditNetwork->setEnabled(true);
	m_ui->groupBoxRemoveShortEdges->setEnabled(true);
	m_ui->groupBoxCurrentHeatExchange->setEnabled(true);
	m_ui->groupBoxCurrentPipes->setEnabled(true);
	m_ui->groupBoxCurrentSubNetworks->setEnabled(true);

	// general network infos
	m_ui->labelNetworkName->setText(m_currentConstNetwork->m_displayName);
	m_ui->labelEdgeCount->setText(QString("%1").arg(m_currentConstNetwork->m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_currentConstNetwork->m_nodes.size()));

	const SVDatabase & db = SVSettings::instance().m_db;
	const VICUS::NetworkFluid * fluid = db.m_fluids[m_currentNetwork.m_idFluid];
	if (fluid != nullptr){
		m_ui->labelFluidName->setText(QtExt::MultiLangString2QString(fluid->m_displayName));
		m_ui->labelFluidName->setStyleSheet("QLabel {color: black}");
	}
	else{
		m_ui->labelFluidName->setText("No fluid selected");
		m_ui->labelFluidName->setStyleSheet("QLabel {color: red}");
	}

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

	// scales
	m_ui->horizontalSliderScaleEdges->setValue((int)m_currentConstNetwork->m_scaleEdges);
	m_ui->horizontalSliderScaleNodes->setValue((int)m_currentConstNetwork->m_scaleNodes);

	// Pipe sizing algotithm parameters
	m_ui->doubleSpinBoxTemperatureSetpoint->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_TemperatureSetpoint].get_value(IBK::Unit("C")));
	m_ui->doubleSpinBoxTemperatureDifference->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_TemperatureDifference].value);
	m_ui->doubleSpinBoxMaximumPressureLoss->setValue(m_currentConstNetwork->m_para[VICUS::Network::P_MaxPressureLoss].value);


	//  *** Update pipes table widget ***

	std::vector<unsigned int> pipeIds;
	for (const VICUS::NetworkEdge &e: m_currentConstNetwork->m_edges){
		if (std::find(pipeIds.begin(), pipeIds.end(), e.m_idPipe) == pipeIds.end() &&
			e.m_idPipe != VICUS::INVALID_ID)
			pipeIds.push_back(e.m_idPipe);
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
	for (const VICUS::NetworkNode &n: m_currentConstNetwork->m_nodes){
		if (std::find(subNetworkIds.begin(), subNetworkIds.end(), n.m_idSubNetwork) == subNetworkIds.end() &&
				n.m_idSubNetwork != VICUS::INVALID_ID)
			subNetworkIds.push_back(n.m_idSubNetwork);
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
	for (const VICUS::NetworkEdge &e: m_currentConstNetwork->m_edges){
		if (std::find(currentHxTypes.begin(), currentHxTypes.end(), e.m_heatExchange.m_modelType) == currentHxTypes.end())
			currentHxTypes.push_back(e.m_heatExchange.m_modelType);
	}
	for (const VICUS::NetworkNode &n: m_currentConstNetwork->m_nodes){
		if (std::find(currentHxTypes.begin(), currentHxTypes.end(), n.m_heatExchange.m_modelType) == currentHxTypes.end())
			currentHxTypes.push_back(n.m_heatExchange.m_modelType);
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


void SVPropNetworkEditWidget::updateHeatExchangeProperties()
{
	// clear widgets
	m_ui->comboBoxHeatExchangeType->clear();
	m_ui->lineEditHeatFlux->clear();
	m_ui->lineEditTemperature->clear();
	m_ui->lineEditHXTransferCoefficient->clear();
	m_ui->widgetBrowseFileNameTSVFile->setFilename("");

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
	else if (!m_currentEdges.empty())
		modelType =  m_currentEdges[0]->networkComponentModelType() ;
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
	m_ui->widgetBrowseFileNameTSVFile->setEnabled(false);
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
			m_ui->widgetBrowseFileNameTSVFile->setEnabled(true);
			m_ui->labelHXTransferCoefficient->setEnabled(true);
			m_ui->lineEditHXTransferCoefficient->setEnabled(true);
			break;
		}
		case NANDRAD::HydraulicNetworkHeatExchange ::T_HeatLossSplineCondenser:
		case NANDRAD::HydraulicNetworkHeatExchange ::T_HeatLossSpline:{
			m_ui->labelDataFile->setEnabled(true);
			m_ui->widgetBrowseFileNameTSVFile->setEnabled(true);
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
		m_ui->widgetBrowseFileNameTSVFile->setFilename(QString::fromStdString(
												hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss].m_tsvFile.str()));
}


void SVPropNetworkEditWidget::clearUI(){
	m_ui->labelNetworkName->clear();
	m_ui->labelFluidName->clear();
	m_ui->labelNodeId->clear();
	m_ui->labelPipeLength->clear();
	m_ui->labelPipeLength->clear();
	m_ui->labelEdgeCount->clear();
	m_ui->labelNodeCount->clear();
	m_ui->labelNetworkConnected->clear();
	m_ui->labelLargestDiameter->clear();
	m_ui->labelSmallestDiameter->clear();
	m_ui->labelTotalLength->clear();

	m_ui->lineEditNodeMaxHeatingDemand->clear();
	m_ui->lineEditNodeX->clear();
	m_ui->lineEditNodeY->clear();
	m_ui->lineEditHeatFlux->clear();
	m_ui->lineEditTemperature->clear();
	m_ui->lineEditEdgeDisplayName->clear();
	m_ui->lineEditNodeDisplayName->clear();
	m_ui->lineEditThresholdSmallEdge->clear();
	m_ui->lineEditHXTransferCoefficient->clear();
	m_ui->widgetBrowseFileNameTSVFile->setFilename("");

	m_ui->checkBoxSupplyPipe->setChecked(false);
	m_ui->comboBoxHeatExchangeType->clear();

	m_ui->tableWidgetPipes->clearContents();
	m_ui->tableWidgetSubNetworks->clearContents();
	m_ui->tableWidgetHeatExchange->clearContents();
}

void SVPropNetworkEditWidget::setAllEnabled(bool enabled)
{
	m_ui->groupBoxProperties->setEnabled(enabled);
	m_ui->groupBoxNode->setEnabled(enabled);
	m_ui->groupBoxEdge->setEnabled(enabled);
	m_ui->groupBoxSizePipes->setEnabled(enabled);
	m_ui->groupBoxVisualisation->setEnabled(enabled);
	m_ui->groupBoxEditNetwork->setEnabled(enabled);
	m_ui->groupBoxHeatExchange->setEnabled(enabled);
	m_ui->groupBoxCurrentHeatExchange->setEnabled(enabled);
	m_ui->groupBoxCurrentPipes->setEnabled(enabled);
	m_ui->groupBoxSelectedPipe->setEnabled(enabled);
	m_ui->groupBoxRemoveShortEdges->setEnabled(enabled);
	m_ui->groupBoxSelectedSubNetwork->setEnabled(enabled);
	m_ui->groupBoxCurrentSubNetworks->setEnabled(enabled);
}


QString SVPropNetworkEditWidget::largestDiameter() const
{
	const SVDatabase & db = SVSettings::instance().m_db;
	double dMax = 0;
	for (const VICUS::NetworkEdge &edge: m_currentConstNetwork->m_edges){
		const VICUS::NetworkPipe * p = db.m_pipes[edge.m_idPipe];
		if (p == nullptr)
			return QString();
		if (p->m_para[VICUS::NetworkPipe::P_DiameterOutside].value > dMax)
			dMax = p->m_para[VICUS::NetworkPipe::P_DiameterOutside].value;
	}
	return QString("%1").arg(dMax);
}


QString SVPropNetworkEditWidget::smallestDiameter() const
{
	const SVDatabase & db = SVSettings::instance().m_db;
	double dMin = std::numeric_limits<double>::max();
	for (const VICUS::NetworkEdge &edge: m_currentConstNetwork->m_edges){
		const VICUS::NetworkPipe * p = db.m_pipes[edge.m_idPipe];
		if (p == nullptr)
			return QString();
		if (p->m_para[VICUS::NetworkPipe::P_DiameterOutside].value < dMin)
			dMin = p->m_para[VICUS::NetworkPipe::P_DiameterOutside].value;
	}
	return QString("%1").arg(dMin);
}


void SVPropNetworkEditWidget::modifyHeatExchangeProperties()
{
	if (!setNetwork())
		return;

	NANDRAD::HydraulicNetworkHeatExchange hx;

	// set model type
	NANDRAD::HydraulicNetworkHeatExchange::ModelType modelType =
			NANDRAD::HydraulicNetworkHeatExchange::ModelType(m_ui->comboBoxHeatExchangeType->currentData().toUInt());
	hx.m_modelType = modelType;

	// if adiabatic skip reading all parameters
	if (hx.m_modelType != NANDRAD::HydraulicNetworkHeatExchange::NUM_T){

		// set heat loss
		if (m_ui->lineEditHeatFlux->isValid())
			NANDRAD::KeywordList::setParameter(hx.m_para, "HydraulicNetworkHeatExchange::para_t",
											 NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss,
											 m_ui->lineEditHeatFlux->value());
		else
			hx.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss].clear();

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

		// set data file
		IBK::Path tsvFile(m_ui->widgetBrowseFileNameTSVFile->filename().toStdString());
		tsvFile = SVProjectHandler::instance().replacePathPlaceholders(tsvFile);
		if (tsvFile.isValid() && (modelType == NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline ||
								  modelType == NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSplineCondenser)){
			// get relative file path
			IBK::Path curr = IBK::Path(SVProjectHandler::instance().projectFile().toStdString()).parentPath();
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss] = NANDRAD::LinearSplineParameter("HeatLoss",
																	 NANDRAD::LinearSplineParameter::I_LINEAR,
																	IBK::Path("${Project Directory}" / tsvFile.relativePath(curr)));
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss].m_wrapMethod = NANDRAD::LinearSplineParameter::C_CYCLIC;
		}
		else
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_HeatLoss] = NANDRAD::LinearSplineParameter();

		if (tsvFile.isValid() && modelType == NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureSpline){
			// get relative file path
			IBK::Path tsvFile(m_ui->widgetBrowseFileNameTSVFile->filename().toStdString());
			IBK::Path curr = IBK::Path(SVProjectHandler::instance().projectFile().toStdString()).parentPath();
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature] = NANDRAD::LinearSplineParameter("Temperature",
																	 NANDRAD::LinearSplineParameter::I_LINEAR,
																	IBK::Path("${Project Directory}" / tsvFile.relativePath(curr)));
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature].m_wrapMethod = NANDRAD::LinearSplineParameter::C_CYCLIC;
		}
		else
			hx.m_splPara[NANDRAD::HydraulicNetworkHeatExchange::SPL_Temperature] = NANDRAD::LinearSplineParameter();

	}

	// set hx properties to nodes
	if (!m_currentNodes.empty()){
		for (const VICUS::NetworkNode * nodeConst: m_currentNodes){
			m_currentNetwork.nodeById(nodeConst->m_id)->m_heatExchange = hx;
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

	updateHeatExchangeProperties();
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
	m_currentNetwork = *VICUS::element(p.m_geometricNetworks, m_currentConstNetwork->m_id);
	return true;
}


void SVPropNetworkEditWidget::onModified(int modificationType, ModificationInfo * /*data*/) {

	// remove this ???


	//	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
//	switch (modType) {
//		case SVProjectHandler::NetworkModified:
//		case SVProjectHandler::AllModified:
//		case SVProjectHandler::NodeStateModified:
//			selectionChanged(); // updates m_currentXXXX and the UI
//		break;

//		default: ; // just to make compiler happy
//	}
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

void SVPropNetworkEditWidget::on_checkBoxSupplyPipe_clicked()
{
	modifyEdgeProperty(&VICUS::NetworkEdge::m_supply, m_ui->checkBoxSupplyPipe->isChecked());
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


void SVPropNetworkEditWidget::on_lineEditNodeMaxHeatingDemand_editingFinished()
{
	if (m_ui->lineEditNodeMaxHeatingDemand->isValid())
		modifyNodeProperty(&VICUS::NetworkNode::m_maxHeatingDemand,
						   IBK::Parameter("MaxHeatingDemand", m_ui->lineEditNodeMaxHeatingDemand->value(), "W"));
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
	modifyNodeProperty(&VICUS::NetworkNode::m_displayName, m_ui->lineEditNodeDisplayName->text());
}


void SVPropNetworkEditWidget::on_lineEditEdgeDisplayName_editingFinished()
{
	modifyEdgeProperty(&VICUS::NetworkEdge::m_displayName, m_ui->lineEditEdgeDisplayName->text());
}

void SVPropNetworkEditWidget::on_pushButtonSizePipeDimensions_clicked()
{
	FUNCID(SVPropNetworkEditWidget::on_pushButtonSizePipeDimensions_clicked);
	modifySizingParams();
	if (!setNetwork())
		return;
	const SVDatabase & db = SVSettings::instance().m_db;
	const VICUS::NetworkFluid * fluid = db.m_fluids[m_currentNetwork.m_idFluid];
	if (fluid == nullptr)
		throw IBK::Exception(IBK::FormatString("Could not find fluid with id %1 in fluid database")
							.arg(m_currentNetwork.m_idFluid), FUNC_ID);

	// filter out list of available pipes
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
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	m_currentNetwork.cleanDeadEnds();
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_pushButtonReduceRedundantNodes_clicked()
{
	if (!setNetwork())
		return;

	// set current network invisible
	m_currentNetwork.setVisible(false);
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undoMod = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undoMod->push(); // modifies project and updates views

	// make copy with reduced nodes
	VICUS::Network newNetwork = m_currentNetwork.clone();
	newNetwork.m_id = VICUS::uniqueId(project().m_geometricNetworks);  // new unique id
	newNetwork.m_edges.clear();
	newNetwork.m_nodes.clear();
	newNetwork.m_displayName = QString("%1_noRedundants").arg(m_currentNetwork.m_displayName);
	newNetwork.setVisible(true);

	// algorithm
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	m_currentNetwork.cleanRedundantEdges(newNetwork);
	newNetwork.updateNodeEdgeConnectionPointers();
	newNetwork.updateExtends();

	SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("modified network"), newNetwork);
	undo->push(); // modifies project and updates views
	m_propModeSelectionWidget->setCurrentNetwork(newNetwork.m_id);
}


void SVPropNetworkEditWidget::on_pushButtonRemoveSmallEdge_clicked()
{
	if (!setNetwork())
		return;

	double threshold = 0;
	if (m_ui->lineEditThresholdSmallEdge->isValid())
		threshold = m_ui->lineEditThresholdSmallEdge->value();
	else
		return;

	// make a copy which will keep the original network data
	VICUS::Network reducedNetwork = m_currentNetwork.clone();
	reducedNetwork.m_id = VICUS::uniqueId(project().m_geometricNetworks);  // new unique id
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	m_currentNetwork.setVisible(false);
	reducedNetwork.setVisible(false);
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undoMod = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undoMod->push(); // modifies project and updates views

	// now modify the current network (new id, new name)
	reducedNetwork.m_id = VICUS::uniqueId(project().m_geometricNetworks);
	reducedNetwork.m_displayName = QString("%1_noShortEdges").arg(m_currentNetwork.m_displayName);
	reducedNetwork.removeShortEdges(threshold);
	reducedNetwork.setVisible(true);
	reducedNetwork.updateExtends();

	SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("modified network"), reducedNetwork);
	undo->push(); // modifies project and updates views
	m_propModeSelectionWidget->setCurrentNetwork(reducedNetwork.m_id);
}


void SVPropNetworkEditWidget::on_pushButtonSelectPipes_clicked()
{
	if (!setNetwork())
		return;

	SVNetworkDialogSelectPipes *dialog = new SVNetworkDialogSelectPipes(this);
	dialog->edit(m_currentNetwork);

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_comboBoxHeatExchangeType_activated(int index)
{
	modifyHeatExchangeProperties();
}


void SVPropNetworkEditWidget::on_pushButtonSelectFluid_clicked()
{
	unsigned int currentId  = m_currentConstNetwork->m_idFluid;
	SVDatabaseEditDialog *dialog = SVMainWindow::instance().dbFluidEditDialog();
	unsigned int newId = dialog->select(currentId);
	if (newId > 0){
		if (!setNetwork())
			return;
		m_currentNetwork.m_idFluid = newId;
		m_currentNetwork.updateNodeEdgeConnectionPointers();
		unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
		undo->push(); // modifies project and updates views
	}
}


void SVPropNetworkEditWidget::on_pushButtonAssignPipe_clicked()
{
	unsigned int currentId = 0;
	if (m_currentEdges.size() > 0)
		currentId = m_currentEdges[0]->m_idPipe;
	unsigned int newId = SVMainWindow::instance().dbPipeEditDialog()->select(currentId);
	modifyEdgeProperty(&VICUS::NetworkEdge::m_idPipe, newId);
}

void SVPropNetworkEditWidget::on_pushButtonEditPipe_clicked()
{
	unsigned int currentId = 0;
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item != nullptr)
		currentId = item->data(Qt::UserRole).toUInt();
	SVMainWindow::instance().dbPipeEditDialog()->edit(currentId);
}


void SVPropNetworkEditWidget::on_pushButtonEditSubNetworks_clicked()
{
	unsigned int currentId = 0;
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item != nullptr)
		currentId = item->data(Qt::UserRole).toUInt();
	SVMainWindow::instance().dbSubNetworkEditDialog()->edit(currentId);
}


void SVPropNetworkEditWidget::on_pushButtonAssignSubNetwork_clicked()
{
	unsigned int currentId = 0;
	if (m_currentNodes.size() > 0)
		currentId = m_currentNodes[0]->m_idSubNetwork;
	unsigned int newId = SVMainWindow::instance().dbSubNetworkEditDialog()->select(currentId);
	modifyNodeProperty(&VICUS::NetworkNode::m_idSubNetwork, newId);
}


void SVPropNetworkEditWidget::on_pushButtonTempChangeIndicator_clicked()
{
	if (!setNetwork())
		return;
	m_currentNetwork.updateNodeEdgeConnectionPointers();

	const SVDatabase & db = SVSettings::instance().m_db;
	const VICUS::NetworkFluid *fluid = db.m_fluids[m_currentNetwork.m_idFluid];
	Q_ASSERT(fluid!=nullptr);
	std::map<unsigned int, std::vector<VICUS::NetworkEdge *> > shortestPaths;
	m_currentNetwork.calcTemperatureChangeIndicator(*fluid, db.m_pipes, shortestPaths);

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}


void SVPropNetworkEditWidget::on_pushButtonDeleteNetwork_clicked()
{
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoDeleteNetwork * undo = new SVUndoDeleteNetwork(tr("Network deleted"), networkIndex);
	undo->push(); // modifies project and updates views
	if (project().m_geometricNetworks.size()>0)
		m_propModeSelectionWidget->setCurrentNetwork(project().m_geometricNetworks[0].m_id);
}

void SVPropNetworkEditWidget::on_pushButtonRecalculateLength_clicked()
{
	if (!setNetwork())
		return;
	m_currentNetwork.updateNodeEdgeConnectionPointers();

	for (const VICUS::NetworkEdge * edge: m_currentEdges){
		m_currentNetwork.edge(edge->nodeId1(), edge->nodeId2())->setLengthFromCoordinates();
	}

	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
	updateNetworkProperties();
}


void SVPropNetworkEditWidget::on_pushButtonSelectEdgesWithPipe_clicked() {

	Q_ASSERT(m_currentConstNetwork != nullptr);
	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item == nullptr)
		return;
	unsigned int pipeId = item->data(Qt::UserRole).toUInt();

	// collect edges
	std::set<unsigned int> edgeIds;
	for (const VICUS::NetworkEdge &e: m_currentConstNetwork->m_edges) {
		if (e.m_idPipe == pipeId)
			edgeIds.insert(e.uniqueID());
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


void SVPropNetworkEditWidget::on_pushButtonExchangePipe_clicked() {

	if (!setNetwork())
		return;

	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetPipes->currentItem();
	if (item == nullptr)
		return;
	unsigned int oldId = item->data(Qt::UserRole).toUInt();

	// get new id
	unsigned int newId = SVMainWindow::instance().dbPipeEditDialog()->select(oldId);
	if (newId == oldId || newId==VICUS::INVALID_ID)
		return;

	// modify edges
	for (VICUS::NetworkEdge &e: m_currentNetwork.m_edges) {
		if (e.m_idPipe == oldId)
			e.m_idPipe = newId;
	}
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_tableWidgetPipes_itemSelectionChanged() {
	bool enabled =  m_ui->tableWidgetPipes->currentRow() != -1;
	m_ui->pushButtonEditPipe->setEnabled(enabled);
	m_ui->pushButtonSelectEdgesWithPipe->setEnabled(enabled);
	m_ui->pushButtonExchangePipe->setEnabled(enabled);
}


void SVPropNetworkEditWidget::on_pushButtonExchangeSubNetwork_clicked()
{
	if (!setNetwork())
		return;

	// get id from current item
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item == nullptr)
		return;
	unsigned int oldId = item->data(Qt::UserRole).toUInt();

	// get new id
	unsigned int newId = SVMainWindow::instance().dbSubNetworkEditDialog()->select(oldId);
	if (newId == oldId || newId==VICUS::INVALID_ID)
		return;

	// modify nodes
	for (VICUS::NetworkNode &n: m_currentNetwork.m_nodes) {
		if (n.m_idSubNetwork == oldId)
			n.m_idSubNetwork = newId;
	}
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}


void SVPropNetworkEditWidget::on_pushButtonSelectNodesWithSubNetwork_clicked()
{
	Q_ASSERT(m_currentConstNetwork != nullptr);
	// get pipeId from current item
	QTableWidgetItem *item = m_ui->tableWidgetSubNetworks->currentItem();
	if (item == nullptr)
		return;
	unsigned int id = item->data(Qt::UserRole).toUInt();

	// collect sub networks
	std::set<unsigned int> nodeIds;
	for (const VICUS::NetworkNode &n: m_currentConstNetwork->m_nodes) {
		if (n.m_idSubNetwork == id)
			nodeIds.insert(n.uniqueID());
	}

	const VICUS::SubNetwork * sub = SVSettings::instance().m_db.m_subNetworks[id];
	QString undoText;
	if (sub != nullptr)
		undoText = tr("Select nodes with sub network '%1'.").arg(QtExt::MultiLangString2QString(sub->m_displayName));
	else
		undoText = tr("Select nodes with invalid/missing sub network.");

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, nodeIds, true);
	undo->push();
}


void SVPropNetworkEditWidget::on_tableWidgetSubNetworks_itemSelectionChanged()
{
	bool enabled =  m_ui->tableWidgetSubNetworks->currentRow() != -1;
	m_ui->pushButtonEditSubNetworks->setEnabled(enabled);
	m_ui->pushButtonSelectNodesWithSubNetwork->setEnabled(enabled);
	m_ui->pushButtonExchangeSubNetwork->setEnabled(enabled);
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
		m_currentNetwork.nodeById(nodeConst->m_id)->*property = value;
	}
	m_currentNetwork.updateNodeEdgeConnectionPointers();
	unsigned int networkIndex = std::distance(&project().m_geometricNetworks.front(), m_currentConstNetwork);
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_currentNetwork);
	undo->push(); // modifies project and updates views
}

