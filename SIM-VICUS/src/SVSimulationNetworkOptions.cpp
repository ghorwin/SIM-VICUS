#include "SVSimulationNetworkOptions.h"
#include "ui_SVSimulationNetworkOptions.h"

#include "SVUndoModifyNetwork.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include <NANDRAD_KeywordListQt.h>

SVSimulationNetworkOptions::SVSimulationNetworkOptions(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationNetworkOptions)
{
	m_ui->setupUi(this);
	layout()->setContentsMargins(0,0,0,0);

	// populate model type combobox
	m_ui->comboBoxModelType->clear();
	for (unsigned int i=0; i<NANDRAD::HydraulicNetwork::NUM_MT; ++i){
		NANDRAD::HydraulicNetwork::ModelType model = NANDRAD::HydraulicNetwork::ModelType(i);
		m_ui->comboBoxModelType->addItem(QString("%1 [%2]")
										.arg(NANDRAD::KeywordListQt::Description("HydraulicNetwork::ModelType", model))
										.arg(NANDRAD::KeywordListQt::Keyword("HydraulicNetwork::ModelType", model)),
										model);
	}

	m_ui->comboBoxModelType->setCurrentIndex(m_ui->comboBoxModelType->findData(
												 NANDRAD::HydraulicNetwork::MT_ThermalHydraulicNetwork));

	// connect to project handler
	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVSimulationNetworkOptions::onModified);
}


SVSimulationNetworkOptions::~SVSimulationNetworkOptions() {
	delete m_ui;
}


void SVSimulationNetworkOptions::updateUi() {

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();

	// populate networks combobox
	const std::vector<VICUS::Network> &networks = project().m_geometricNetworks;
	m_ui->comboBoxNetwork->clear();
	for (const VICUS::Network &n : networks)
		m_ui->comboBoxNetwork->addItem(n.m_displayName, n.m_id);

	if (currentId>0)
		m_ui->comboBoxNetwork->setCurrentIndex(m_ui->comboBoxNetwork->findData(currentId));

	// manually trigger UI update
	updateNetworkParameters();
}


void SVSimulationNetworkOptions::onModified(int modificationType, ModificationInfo *) {
	switch ((SVProjectHandler::ModificationTypes)modificationType) {
	case SVProjectHandler::AllModified :
	case SVProjectHandler::NetworkDataChanged:
	case SVProjectHandler::NetworkGeometryChanged: {
		updateUi();
	} break;
	default:;
	}
}


void SVSimulationNetworkOptions::updateNetworkParameters() {

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	const VICUS::Network *network = VICUS::element(project().m_geometricNetworks, currentId);

	// none available?
	bool haveNetwork = network != nullptr;
	m_ui->comboBoxNetwork->setEnabled(false);
	m_ui->lineEditDefaultFluidTemperature->setEnabled(haveNetwork);
	m_ui->lineEditReferencePressure->setEnabled(haveNetwork);
	m_ui->lineEditMaxPipeDiscretization->setEnabled(haveNetwork);
	m_ui->comboBoxModelType->setEnabled(haveNetwork);
	m_ui->groupBoxHeatExchangeWithGround->setEnabled(haveNetwork);
	m_ui->lineEditMaxPipeDiscretization->clear();
	m_ui->lineEditReferencePressure->clear();
	m_ui->lineEditDefaultFluidTemperature->clear();
	m_ui->lineEditNumberOfSoilModels->clear();
	m_ui->lineEditPipeDepth->clear();
	m_ui->lineEditPipeSpacing->clear();
	m_ui->groupBoxHeatExchangeWithGround->setChecked(false);

	if (!haveNetwork)
		return;

	m_ui->comboBoxModelType->blockSignals(true);
	m_ui->comboBoxModelType->setCurrentIndex(m_ui->comboBoxModelType->findData(network->m_modelType));
	m_ui->comboBoxModelType->blockSignals(false);

	// update parameters
	if (!network->m_para[VICUS::Network::P_ReferencePressure].empty())
		m_ui->lineEditReferencePressure->setValue(network->m_para[VICUS::Network::P_ReferencePressure].get_value("Bar"));
	if (!network->m_para[VICUS::Network::P_DefaultFluidTemperature].empty())
		m_ui->lineEditDefaultFluidTemperature->setValue(network->m_para[VICUS::Network::P_DefaultFluidTemperature].get_value("C"));
	if (!network->m_para[VICUS::Network::P_MaxPipeDiscretization].empty())
		m_ui->lineEditMaxPipeDiscretization->setValue(network->m_para[VICUS::Network::P_MaxPipeDiscretization].value);

	// update ground heat exchange options
	m_ui->groupBoxHeatExchangeWithGround->setChecked(network->m_hasHeatExchangeWithGround);
	if (!network->m_buriedPipeProperties.m_para[VICUS::NetworkBuriedPipeProperties::P_PipeSpacing].empty())
		m_ui->lineEditPipeSpacing->setValue(network->m_buriedPipeProperties.m_para[VICUS::NetworkBuriedPipeProperties::P_PipeSpacing].value);
	if (!network->m_buriedPipeProperties.m_para[VICUS::NetworkBuriedPipeProperties::P_PipeDepth].empty())
		m_ui->lineEditPipeDepth->setValue(network->m_buriedPipeProperties.m_para[VICUS::NetworkBuriedPipeProperties::P_PipeDepth].value);
	m_ui->lineEditNumberOfSoilModels->setValue(network->m_buriedPipeProperties.m_numberOfSoilModels);
}


void SVSimulationNetworkOptions::on_comboBoxNetwork_activated(int /*index*/) {
	updateNetworkParameters();
}


void SVSimulationNetworkOptions::on_comboBoxModelType_activated(int /*index*/) {
	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	VICUS::Project p = project();
	VICUS::Network *net = VICUS::element(p.m_geometricNetworks, currentId);
	Q_ASSERT(net!=nullptr);

	net->m_modelType = VICUS::Network::ModelType(m_ui->comboBoxModelType->currentData().toUInt());

	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("modified network", *net);
	undo->push();
}


void SVSimulationNetworkOptions::on_lineEditDefaultFluidTemperature_editingFinished() {
	if (!m_ui->lineEditDefaultFluidTemperature->isValid())
		return;

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	VICUS::Project p = project();
	VICUS::Network *net = VICUS::element(p.m_geometricNetworks, currentId);
	Q_ASSERT(net!=nullptr);

	net->m_para[VICUS::Network::P_DefaultFluidTemperature] =
					IBK::Parameter("DefaultFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
	net->m_para[VICUS::Network::P_InitialFluidTemperature] =
				IBK::Parameter("InitialFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));

	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("modified network", *net);
	undo->push();
}


void SVSimulationNetworkOptions::on_lineEditReferencePressure_editingFinished() {
	if (!m_ui->lineEditReferencePressure->isValid())
		return;

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	VICUS::Project p = project();
	VICUS::Network *net = VICUS::element(p.m_geometricNetworks, currentId);
	Q_ASSERT(net!=nullptr);

	net->m_para[VICUS::Network::P_ReferencePressure] = IBK::Parameter(VICUS::KeywordList::Keyword("Network::para_t", VICUS::Network::P_ReferencePressure),
																			m_ui->lineEditReferencePressure->value(),
																			"Bar");

	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("modified network", *net);
	undo->push();
}


void SVSimulationNetworkOptions::on_lineEditMaxPipeDiscretization_editingFinished() {
	if (!m_ui->lineEditMaxPipeDiscretization->isValid())
		return;

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	VICUS::Project p = project();
	VICUS::Network *net = VICUS::element(p.m_geometricNetworks, currentId);
	Q_ASSERT(net!=nullptr);

	VICUS::KeywordList::setParameter(net->m_para, "Network::para_t", VICUS::Network::P_MaxPipeDiscretization,
										 m_ui->lineEditMaxPipeDiscretization->value());

	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("modified network", *net);
	undo->push();
}


void SVSimulationNetworkOptions::on_lineEditPipeSpacing_editingFinished() {
	if (!m_ui->lineEditPipeSpacing->isValid())
		return;

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	VICUS::Project p = project();
	VICUS::Network *net = VICUS::element(p.m_geometricNetworks, currentId);
	Q_ASSERT(net!=nullptr);

	VICUS::KeywordList::setParameter(net->m_buriedPipeProperties.m_para, "NetworkBuriedPipeProperties::para_t",
									 VICUS::NetworkBuriedPipeProperties::P_PipeSpacing,
									 m_ui->lineEditPipeSpacing->value());

	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("modified network", *net);
	undo->push();
}


void SVSimulationNetworkOptions::on_lineEditPipeDepth_editingFinished() {
	if (!m_ui->lineEditPipeDepth->isValid())
		return;

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	VICUS::Project p = project();
	VICUS::Network *net = VICUS::element(p.m_geometricNetworks, currentId);
	Q_ASSERT(net!=nullptr);

	VICUS::KeywordList::setParameter(net->m_buriedPipeProperties.m_para, "NetworkBuriedPipeProperties::para_t",
									 VICUS::NetworkBuriedPipeProperties::P_PipeDepth,
									 m_ui->lineEditPipeDepth->value());

	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("modified network", *net);
	undo->push();
}


void SVSimulationNetworkOptions::on_lineEditNumberOfSoilModels_editingFinished() {
	if (!m_ui->lineEditNumberOfSoilModels->isValid())
		return;

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	VICUS::Project p = project();
	VICUS::Network *net = VICUS::element(p.m_geometricNetworks, currentId);
	Q_ASSERT(net!=nullptr);

	net->m_buriedPipeProperties.m_numberOfSoilModels = (unsigned int)m_ui->lineEditNumberOfSoilModels->value();

	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("modified network", *net);
	undo->push();
}


void SVSimulationNetworkOptions::on_groupBoxHeatExchangeWithGround_clicked(bool checked) {
	if (!m_ui->lineEditNumberOfSoilModels->isValid())
		return;

	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	VICUS::Project p = project();
	VICUS::Network *net = VICUS::element(p.m_geometricNetworks, currentId);
	Q_ASSERT(net!=nullptr);

	net->m_hasHeatExchangeWithGround = checked;

	SVUndoModifyNetwork *undo = new SVUndoModifyNetwork("modified network", *net);
	undo->push();
}

