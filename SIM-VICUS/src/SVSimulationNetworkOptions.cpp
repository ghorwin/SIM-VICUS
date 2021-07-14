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
	for (VICUS::Network n : *m_networks)
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


//void SVSimulationNetworkOptions::modify() {
//	return; // TODO Hauke, rewrite

//	// get selected network
//	VICUS::Network *network = VICUS::Project::element(*m_networks, m_currentId);
//	if (network == nullptr)
//		return;

//	// set model type
//	network->m_modelType = VICUS::Network::ModelType(m_ui->comboBoxModelType->currentData().toUInt());

//	// set params
//	if (m_ui->lineEditReferencePressure->isValid())
//		VICUS::KeywordList::setParameter(network->m_para, "Network::para_t", VICUS::Network::P_ReferencePressure,
//										 m_ui->lineEditReferencePressure->value());
//	if (m_ui->lineEditDefaultFluidTemperature->isValid()){
//		network->m_para[VICUS::Network::P_DefaultFluidTemperature] =
//				IBK::Parameter("DefaultFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
//		network->m_para[VICUS::Network::P_InitialFluidTemperature] =
//				IBK::Parameter("InitialFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
//	}

//	if (m_ui->lineEditMaxPipeDiscretization->isValid())
//		VICUS::KeywordList::setParameter(network->m_para, "Network::para_t", VICUS::Network::P_MaxPipeDiscretization,
//										 m_ui->lineEditMaxPipeDiscretization->value());

//	unsigned int networkIndex = std::distance(&m_networks->front(), VICUS::Project::element(*m_networks, m_currentId));
//	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network properties updated"), networkIndex, *network);
//	undo->push(); // modifies project and updates views
//}


void SVSimulationNetworkOptions::on_comboBoxNetwork_activated(int /*index*/) {
	// none selected?

	bool haveNetwork = (m_ui->comboBoxNetwork->currentIndex() != 0);
	m_ui->lineEditDefaultFluidTemperature->setEnabled(haveNetwork);
	m_ui->lineEditReferencePressure->setEnabled(haveNetwork);
	m_ui->lineEditMaxPipeDiscretization->setEnabled(haveNetwork);
	m_ui->comboBoxModelType->setEnabled(haveNetwork);

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

	if (!m_current->m_para[VICUS::Network::P_ReferencePressure].empty())
		m_ui->lineEditReferencePressure->setValue(m_current->m_para[VICUS::Network::P_ReferencePressure].value);
	else
		m_ui->lineEditReferencePressure->setValue(0);
	if (!m_current->m_para[VICUS::Network::P_DefaultFluidTemperature].empty())
		m_ui->lineEditDefaultFluidTemperature->setValue(m_current->m_para[VICUS::Network::P_DefaultFluidTemperature].get_value("C"));
	else
		m_ui->lineEditDefaultFluidTemperature->setValue(35);

	if (!m_current->m_para[VICUS::Network::P_MaxPipeDiscretization].empty())
		m_ui->lineEditMaxPipeDiscretization->setValue(m_current->m_para[VICUS::Network::P_MaxPipeDiscretization].value);
	else
		m_ui->lineEditMaxPipeDiscretization->setValue(10);
}


void SVSimulationNetworkOptions::on_comboBoxModelType_activated(int /*index*/) {
	// TODO : Update current network's data
}


void SVSimulationNetworkOptions::on_lineEditDefaultFluidTemperature_editingFinished() {
	// TODO : Update current network's data
}


void SVSimulationNetworkOptions::on_lineEditReferencePressure_editingFinished() {
	// TODO : Update current network's data
}


void SVSimulationNetworkOptions::on_lineEditMaxPipeDiscretization_editingFinished() {
	// TODO : Update current network's data
}
