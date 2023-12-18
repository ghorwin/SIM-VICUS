#include "SVPropNetworkHeatExchangeWidget.h"
#include "ui_SVPropNetworkHeatExchangeWidget.h"

#include <NANDRAD_KeywordListQt.h>

#include <QtExt_ValidatingLineEdit.h>

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "SVPropNetworkEditWidget.h"
#include "SVTimeSeriesPreviewDialog.h"


SVPropNetworkHeatExchangeWidget::SVPropNetworkHeatExchangeWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropNetworkHeatExchangeWidget),
	m_pa(qobject_cast<SVPropNetworkEditWidget*>(parent))
{
	m_ui->setupUi(this);

	m_ui->tableWidgetHeatExchange->setColumnCount(2);
	m_ui->tableWidgetHeatExchange->setHorizontalHeaderLabels(QStringList() << QString() << tr("Heat Exchange Types"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetHeatExchange);
	m_ui->tableWidgetHeatExchange->setSortingEnabled(false);
	m_ui->tableWidgetHeatExchange->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	m_ui->tableWidgetHeatExchange->horizontalHeader()->resizeSection(0,20);
	m_ui->tableWidgetHeatExchange->horizontalHeader()->setStretchLastSection(true);

	m_ui->lineEditHeatFlux->setup(0, std::numeric_limits<double>::max(), tr("value of constant heat flux"), true, true);
	m_ui->lineEditTemperature->setup(0, std::numeric_limits<double>::max(), tr("value of constant temperature"), true, true);
	m_ui->lineEditHXTransferCoefficient->setup(0, std::numeric_limits<double>::max(), tr("convective heat exchange coefficient, set =0 to neglect"), true, true);

}


SVPropNetworkHeatExchangeWidget::~SVPropNetworkHeatExchangeWidget() {
	delete m_ui;
}


void SVPropNetworkHeatExchangeWidget::updateUi() {

	// clear widgets
	m_ui->comboBoxHeatExchangeType->clear();
	m_ui->lineEditHeatFlux->clear();
	m_ui->lineEditTemperature->clear();
	m_ui->lineEditHXTransferCoefficient->clear();
	m_ui->labelHeatExchangeSpline->setText("");

	// in case we have a mixed selection of nodes and edges
	// or no object selected at all: don't proceed
	m_ui->groupBoxHeatExchange->setEnabled(false);
	if ((!m_pa->m_currentEdges.empty() && !m_pa->m_currentNodes.empty()) ||
		(m_pa->m_currentEdges.empty() && m_pa->m_currentNodes.empty()) ){
		return;
	}


	// *** populate combobox

	// get the current component modelType in order to know which heat exchange types are allowed
	const SVDatabase & db = SVSettings::instance().m_db;
	VICUS::NetworkComponent::ModelType modelType = VICUS::NetworkComponent::NUM_MT;
	// if we have node(s)
	if (!m_pa->m_currentNodes.empty()){
		const VICUS::SubNetwork *sub = db.m_subNetworks[m_pa->m_currentNodes[0]->m_idSubNetwork];
		if (sub == nullptr)
			return;
		const VICUS::NetworkComponent *comp = sub->heatExchangeComponent(db.m_networkComponents);
		if (comp == nullptr)
			return;
		modelType = comp->m_modelType;
	}
	// if we have edge(s)
	else if (!m_pa->m_currentEdges.empty()){
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
	if (m_pa->uniformProperty(m_pa->m_currentEdges, &VICUS::NetworkEdge::m_heatExchange))
		hx = m_pa->m_currentEdges[0]->m_heatExchange;
	else if (m_pa->uniformProperty(m_pa->m_currentNodes, &VICUS::NetworkNode::m_heatExchange))
		hx = m_pa->m_currentNodes[0]->m_heatExchange;
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


void SVPropNetworkHeatExchangeWidget::updateTableWidget() {
	// enable all network group boxes
	m_ui->groupBoxCurrentHeatExchange->setEnabled(true);

	const VICUS::Project &p = project();

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
	int row = 0;
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


void SVPropNetworkHeatExchangeWidget::clearUi() {
	m_ui->lineEditHeatFlux->clear();
	m_ui->lineEditTemperature->clear();
	m_ui->lineEditHXTransferCoefficient->clear();
	m_ui->labelDataFile->clear();
	m_ui->comboBoxHeatExchangeType->clear();
	m_ui->tableWidgetHeatExchange->clearContents();
}


void SVPropNetworkHeatExchangeWidget::setWidgetsEnabled(bool enabled) {
	m_ui->groupBoxHeatExchange->setEnabled(enabled);
	m_ui->groupBoxCurrentHeatExchange->setEnabled(enabled);
}


void SVPropNetworkHeatExchangeWidget::modifyHeatExchangeParameters() {

	NANDRAD::HydraulicNetworkHeatExchange hx;
	if (!m_pa->m_currentNodes.empty())
		hx = m_pa->m_currentNodes[0]->m_heatExchange;
	else if (!m_pa->m_currentEdges.empty())
		hx = m_pa->m_currentEdges[0]->m_heatExchange;
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
	if (!m_pa->m_currentNodes.empty())
		m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_heatExchange, hx);
	if (!m_pa->m_currentEdges.empty())
		m_pa->modifyEdgeProperty(&VICUS::NetworkEdge::m_heatExchange, hx);
}


void SVPropNetworkHeatExchangeWidget::on_comboBoxHeatExchangeType_activated(int /*index*/) {
	modifyHeatExchangeParameters();
}


void SVPropNetworkHeatExchangeWidget::on_lineEditHeatFlux_editingFinishedSuccessfully() {
	modifyHeatExchangeParameters();
}


void SVPropNetworkHeatExchangeWidget::on_lineEditTemperature_editingFinishedSuccessfully() {
	modifyHeatExchangeParameters();
}


void SVPropNetworkHeatExchangeWidget::on_lineEditHXTransferCoefficient_editingFinishedSuccessfully() {
	modifyHeatExchangeParameters();
}


void SVPropNetworkHeatExchangeWidget::on_widgetHeatExchangeSplineFile_editingFinished() {
	modifyHeatExchangeParameters();
}


void SVPropNetworkHeatExchangeWidget::on_toolButtonHeatExchangeSpline_clicked() {
	Q_ASSERT(m_pa->m_currentNetwork!=nullptr);

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
	if (!m_pa->m_currentNodes.empty())
		hx = m_pa->m_currentNodes[0]->m_heatExchange;
	else if (!m_pa->m_currentEdges.empty())
		hx = m_pa->m_currentEdges[0]->m_heatExchange;
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
	if (!m_pa->m_currentNodes.empty())
		m_pa->modifyNodeProperty(&VICUS::NetworkNode::m_heatExchange, hx);
	if (!m_pa->m_currentEdges.empty())
		m_pa->modifyEdgeProperty(&VICUS::NetworkEdge::m_heatExchange, hx);
}

