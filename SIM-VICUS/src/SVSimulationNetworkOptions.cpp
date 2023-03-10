#include "SVSimulationNetworkOptions.h"
#include "ui_SVSimulationNetworkOptions.h"

#include "SVUndoModifyNetwork.h"

#include <VICUS_Project.h>

#include <NANDRAD_KeywordListQt.h>

SVSimulationNetworkOptions::SVSimulationNetworkOptions(QWidget *parent, std::vector<VICUS::Network> &networks) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationNetworkOptions),
	m_networks(&networks)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);

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
}


SVSimulationNetworkOptions::~SVSimulationNetworkOptions() {
	delete m_ui;
}


void SVSimulationNetworkOptions::updateUi() {

	// populate networks combobox
	m_ui->comboBoxNetwork->blockSignals(true);
	m_ui->comboBoxNetwork->clear();
	m_ui->comboBoxNetwork->addItem(tr("None"), VICUS::INVALID_ID);
	for (const VICUS::Network &n : *m_networks)
		m_ui->comboBoxNetwork->addItem(n.m_displayName, n.m_id);
	m_current = nullptr;

	// find currently selected network (for now we select the first one with this flag)
	unsigned int currentId = VICUS::INVALID_ID;
	for (const VICUS::Network &net: *m_networks) {
		if (net.m_selectedForSimulation){
			currentId = net.m_id;
			break;
		}
	}
	// enable/disable input fields and populate UI widgets
	int idx = m_ui->comboBoxNetwork->findData(currentId);
	if (idx == -1) // invalid ID or no network selected for calculation, yet
		idx = 0; // fallback to "None"
	m_ui->comboBoxNetwork->setCurrentIndex(idx);
	m_ui->comboBoxNetwork->blockSignals(false);

	// manually trigger UI update
	on_comboBoxNetwork_activated(idx);
}


void SVSimulationNetworkOptions::on_comboBoxNetwork_activated(int /*index*/) {
	// none selected?

	bool haveNetwork = (m_ui->comboBoxNetwork->currentIndex() != 0);
	m_ui->lineEditDefaultFluidTemperature->setEnabled(haveNetwork);
	m_ui->lineEditReferencePressure->setEnabled(haveNetwork);
	m_ui->lineEditMaxPipeDiscretization->setEnabled(haveNetwork);
	m_ui->comboBoxModelType->setEnabled(haveNetwork);
	m_ui->groupBoxHeatExchangeWithGround->setEnabled(haveNetwork);

	m_current = nullptr;
	if (!haveNetwork)
		return;

	// set selected flag for all networks
	unsigned int currentId = m_ui->comboBoxNetwork->currentData().toUInt();

	// process all networks and enable only the currently selected network for calculation
	for (VICUS::Network &net: *m_networks) {
		if (net.m_id == currentId) {
			net.m_selectedForSimulation = true;
			m_current = &net;
		}
		else
			net.m_selectedForSimulation = false;
	}

	m_ui->comboBoxModelType->blockSignals(true);
	m_ui->comboBoxModelType->setCurrentIndex(m_ui->comboBoxModelType->findData(m_current->m_modelType));
	m_ui->comboBoxModelType->blockSignals(false);

	// update parameters
	if (!m_current->m_para[VICUS::Network::P_ReferencePressure].empty())
		m_ui->lineEditReferencePressure->setValue(m_current->m_para[VICUS::Network::P_ReferencePressure].get_value("Bar"));
	if (!m_current->m_para[VICUS::Network::P_DefaultFluidTemperature].empty())
		m_ui->lineEditDefaultFluidTemperature->setValue(m_current->m_para[VICUS::Network::P_DefaultFluidTemperature].get_value("C"));
	if (!m_current->m_para[VICUS::Network::P_MaxPipeDiscretization].empty())
		m_ui->lineEditMaxPipeDiscretization->setValue(m_current->m_para[VICUS::Network::P_MaxPipeDiscretization].value);

	// update ground heat exchange options
	m_ui->groupBoxHeatExchangeWithGround->setChecked(m_current->m_hasHeatExchangeWithGround);
	if (!m_current->m_buriedPipeProperties.m_para[VICUS::NetworkBuriedPipeProperties::P_PipeSpacing].empty())
		m_ui->lineEditPipeSpacing->setValue(m_current->m_buriedPipeProperties.m_para[VICUS::NetworkBuriedPipeProperties::P_PipeSpacing].value);
	if (!m_current->m_buriedPipeProperties.m_para[VICUS::NetworkBuriedPipeProperties::P_PipeDepth].empty())
		m_ui->lineEditPipeDepth->setValue(m_current->m_buriedPipeProperties.m_para[VICUS::NetworkBuriedPipeProperties::P_PipeDepth].value);
	m_ui->lineEditNumberOfSoilModels->setValue(m_current->m_buriedPipeProperties.m_numberOfSoilModels);
}


void SVSimulationNetworkOptions::on_comboBoxModelType_activated(int /*index*/) {
	Q_ASSERT(m_current!=nullptr);
	m_current->m_modelType = VICUS::Network::ModelType(m_ui->comboBoxModelType->currentData().toUInt());
}


void SVSimulationNetworkOptions::on_lineEditDefaultFluidTemperature_editingFinished() {
	Q_ASSERT(m_current!=nullptr);
	if (m_ui->lineEditDefaultFluidTemperature->isValid()){
		m_current->m_para[VICUS::Network::P_DefaultFluidTemperature] =
						IBK::Parameter("DefaultFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
		m_current->m_para[VICUS::Network::P_InitialFluidTemperature] =
					IBK::Parameter("InitialFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
	}
}


void SVSimulationNetworkOptions::on_lineEditReferencePressure_editingFinished() {
	Q_ASSERT(m_current!=nullptr);
	if (m_ui->lineEditReferencePressure->isValid())
		m_current->m_para[VICUS::Network::P_ReferencePressure] = IBK::Parameter(VICUS::KeywordList::Keyword("Network::para_t", VICUS::Network::P_ReferencePressure),
																				m_ui->lineEditReferencePressure->value(),
																				"Bar");
}


void SVSimulationNetworkOptions::on_lineEditMaxPipeDiscretization_editingFinished() {
	Q_ASSERT(m_current!=nullptr);
	if (m_ui->lineEditMaxPipeDiscretization->isValid())
		VICUS::KeywordList::setParameter(m_current->m_para, "Network::para_t", VICUS::Network::P_MaxPipeDiscretization,
											 m_ui->lineEditMaxPipeDiscretization->value());
}


void SVSimulationNetworkOptions::on_lineEditPipeSpacing_editingFinished()
{
	Q_ASSERT(m_current!=nullptr);
	if (m_ui->lineEditPipeSpacing->isValid())
		VICUS::KeywordList::setParameter(m_current->m_buriedPipeProperties.m_para, "NetworkBuriedPipeProperties::para_t",
										 VICUS::NetworkBuriedPipeProperties::P_PipeSpacing,
										 m_ui->lineEditPipeSpacing->value());
}

void SVSimulationNetworkOptions::on_lineEditPipeDepth_editingFinished()
{
	Q_ASSERT(m_current!=nullptr);
	if (m_ui->lineEditPipeDepth->isValid())
		VICUS::KeywordList::setParameter(m_current->m_buriedPipeProperties.m_para, "NetworkBuriedPipeProperties::para_t",
										 VICUS::NetworkBuriedPipeProperties::P_PipeDepth,
										 m_ui->lineEditPipeDepth->value());
}


void SVSimulationNetworkOptions::on_lineEditNumberOfSoilModels_editingFinished()
{
	Q_ASSERT(m_current!=nullptr);
	if (m_ui->lineEditNumberOfSoilModels->isValid())
		m_current->m_buriedPipeProperties.m_numberOfSoilModels = m_ui->lineEditNumberOfSoilModels->value();
}

void SVSimulationNetworkOptions::on_groupBoxHeatExchangeWithGround_clicked(bool checked) {
	m_current->m_hasHeatExchangeWithGround = checked;
}

