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

SVSimulationNetworkOptions::~SVSimulationNetworkOptions()
{
	delete m_ui;
}

void SVSimulationNetworkOptions::updateUi()
{
	if (m_networks->size()==0)
		return;

	// populate networks combobox
	m_ui->comboBoxNetwork->clear();
	for (VICUS::Network n : *m_networks)
		m_ui->comboBoxNetwork->addItem(n.m_displayName, n.m_id);

	// find currently selected network (for now we select the first one with this flag)
	m_currentId = VICUS::INVALID_ID;
	for (const VICUS::Network &net: *m_networks){
		if (net.m_selectedForSimulation){
			m_currentId = net.m_id;
			break;
		}
	}
	m_ui->comboBoxNetwork->setCurrentIndex(m_ui->comboBoxNetwork->findData(m_currentId));

	// get network and update ui
	const VICUS::Network *network = VICUS::Project::element(*m_networks, m_currentId);
	if(network == nullptr)
		return;

	m_ui->comboBoxModelType->setCurrentIndex(m_ui->comboBoxModelType->findData(network->m_modelType));

	if (!network->m_para[VICUS::Network::P_ReferencePressure].empty())
		m_ui->lineEditReferencePressure->setValue(network->m_para[VICUS::Network::P_ReferencePressure].value);
	if (!network->m_para[VICUS::Network::P_DefaultFluidTemperature].empty())
		m_ui->lineEditDefaultFluidTemperature->setValue(network->m_para[VICUS::Network::P_DefaultFluidTemperature].get_value("C"));

	if (!network->m_para[VICUS::Network::P_MaxPipeDiscretization].empty())
		m_ui->lineEditMaxPipeDiscretization->setValue(network->m_para[VICUS::Network::P_MaxPipeDiscretization].value);
}


void SVSimulationNetworkOptions::modify()
{
	// get selected network
	VICUS::Network *network = VICUS::Project::element(*m_networks, m_currentId);
	if (network == nullptr)
		return;

	// set model type
	network->m_modelType = VICUS::Network::ModelType(m_ui->comboBoxModelType->currentData().toUInt());

	// set params
	if (m_ui->lineEditReferencePressure->isValid())
		VICUS::KeywordList::setParameter(network->m_para, "Network::para_t", VICUS::Network::P_ReferencePressure,
										 m_ui->lineEditReferencePressure->value());
	if (m_ui->lineEditDefaultFluidTemperature->isValid()){
		network->m_para[VICUS::Network::P_DefaultFluidTemperature] =
				IBK::Parameter("DefaultFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
		network->m_para[VICUS::Network::P_InitialFluidTemperature] =
				IBK::Parameter("InitialFluidTemperature", m_ui->lineEditDefaultFluidTemperature->value(), IBK::Unit("C"));
	}

	if (m_ui->lineEditMaxPipeDiscretization->isValid())
		VICUS::KeywordList::setParameter(network->m_para, "Network::para_t", VICUS::Network::P_MaxPipeDiscretization,
										 m_ui->lineEditMaxPipeDiscretization->value());

	unsigned int networkIndex = std::distance(&m_networks->front(), VICUS::Project::element(*m_networks, m_currentId));
	SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network properties updated"), networkIndex, *network);
	undo->push(); // modifies project and updates views
}

void SVSimulationNetworkOptions::on_comboBoxNetwork_activated(int /*index*/)
{
	// set selected flag for all networks
	m_currentId = m_ui->comboBoxNetwork->currentData().toUInt();
	unsigned int i=0;
	for (VICUS::Network &net: *m_networks){
		if (net.m_id == m_currentId)
			net.m_selectedForSimulation = true;
		else
			net.m_selectedForSimulation = false;

		SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network properties updated"), i, net);
		undo->push(); // modifies project and updates views
	}
	updateUi();
}

void SVSimulationNetworkOptions::on_comboBoxModelType_activated(int /*index*/)
{
	modify();
	updateUi();
}

void SVSimulationNetworkOptions::on_lineEditDefaultFluidTemperature_editingFinished()
{
	modify();
	updateUi();
}

void SVSimulationNetworkOptions::on_lineEditReferencePressure_editingFinished()
{
	modify();
	updateUi();
}

void SVSimulationNetworkOptions::on_lineEditMaxPipeDiscretization_editingFinished()
{
	modify();
	updateUi();
}
