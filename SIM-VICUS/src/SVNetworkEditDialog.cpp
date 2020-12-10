#include "SVNetworkEditDialog.h"
#include "ui_SVNetworkEditDialog.h"

#include "SVProjectHandler.h"
#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"

#include <VICUS_Project.h>
#include <VICUS_KeywordList.h>


#include <QMessageBox>


SVNetworkEditDialog::SVNetworkEditDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkEditDialog)
{
	m_ui->setupUi(this);
}

SVNetworkEditDialog::~SVNetworkEditDialog()
{
	delete m_ui;
}

void SVNetworkEditDialog::edit()
{
	const VICUS::Project &p = project();

	// setup combobox
	if (!p.m_geometricNetworks.empty()){
		m_existingNetworksMap.clear();
		for (auto it = p.m_geometricNetworks.begin(); it!=p.m_geometricNetworks.end(); ++it)
			m_existingNetworksMap.insert(QString::fromStdString(it->m_name), it->m_id);
		m_ui->comboBoxSelectNetwork->clear();
		m_ui->comboBoxSelectNetwork->addItems(QStringList(m_existingNetworksMap.keys()));
	}

	setNetwork();
	updateSizingParams();

	if (exec() != QDialog::Accepted)
		return;

}

void SVNetworkEditDialog::updateStatus() const{
	m_ui->labelEdgeCount->setText(QString("%1").arg(m_network.m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_network.m_nodes.size()));
	if (m_network.checkConnectedGraph()){
		m_ui->labelNetworkConnected->setText("Network is connected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: green}");
	}
	else{
		m_ui->labelNetworkConnected->setText("Network is unconnected");
		m_ui->labelNetworkConnected->setStyleSheet("QLabel {color: red}");
	}

	m_ui->labelTotalLength->setText(QString("%1").arg(m_network.totalLength()));
	m_ui->pushButtonConnectBuildings->setEnabled(m_network.nextUnconnectedBuilding()>=0);
	m_ui->pushButtonReduceDeadEnds->setEnabled(m_network.checkConnectedGraph() && m_network.numberOfBuildings() > 0);
	m_ui->labelLargestDiameter->setText(QString("%1").arg(m_network.largestDiameter()));
	m_ui->labelSmallestDiameter->setText(QString("%1").arg(m_network.smallestDiameter()));
}

void SVNetworkEditDialog::updateSizingParams()
{
	if (m_network.m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].empty())
		m_network.setDefaultSizingParams();
	m_ui->doubleSpinBoxTemperatureSetpoint->setValue(m_network.m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].get_value("C"));
	m_ui->doubleSpinBoxTemperatureDifference->setValue(m_network.m_sizingPara[VICUS::Network::SP_TemperatureDifference].get_value("K"));
	m_ui->doubleSpinBoxMaximumPressureLoss->setValue(m_network.m_sizingPara[VICUS::Network::SP_MaxPressureLoss].get_value("Pa/m"));
}

void SVNetworkEditDialog::modifySizingParams()
{
	m_network.m_sizingPara[VICUS::Network::SP_TemperatureSetpoint].set(VICUS::KeywordList::Keyword("Network::SizingParam", VICUS::Network::SP_TemperatureSetpoint),
																	   m_ui->doubleSpinBoxTemperatureSetpoint->value(),
																	   IBK::Unit("C"));
	VICUS::KeywordList::setParameter(m_network.m_sizingPara, "Network::SizingParam", VICUS::Network::SP_TemperatureDifference,
									 m_ui->doubleSpinBoxTemperatureDifference->value());
	VICUS::KeywordList::setParameter(m_network.m_sizingPara, "Network::SizingParam", VICUS::Network::SP_MaxPressureLoss,
									 m_ui->doubleSpinBoxMaximumPressureLoss->value());
}

void SVNetworkEditDialog::setNetwork()
{
	unsigned id = m_existingNetworksMap.value(m_ui->comboBoxSelectNetwork->currentText());
	m_network = *project().element(project().m_geometricNetworks, id);
	m_network.updateNodeEdgeConnectionPointers();
	updateStatus();
}

void SVNetworkEditDialog::on_pushButtonGenerateIntersections_clicked()
{
	m_network.generateIntersections();
	updateStatus();

	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
}

void SVNetworkEditDialog::on_comboBoxSelectNetwork_activated(const QString &arg1)
{
	setNetwork();
}


void SVNetworkEditDialog::on_pushButtonConnectBuildings_clicked()
{
	QMessageBox::StandardButton reply = QMessageBox::question(this, "Connect Buildings", "Extend supply pipes?",
								   QMessageBox::Yes|QMessageBox::No);
	m_network.connectBuildings(reply == QMessageBox::Yes);
	updateStatus();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
}


// reduce redundants: do this only before export to NANDRAD, maybe show it but dont save this state

//	VICUS::Network tmp;
//	tmp.m_id = m_network.m_id;
//	tmp.m_fluidID = m_network.m_fluidID;
//	tmp.m_name = m_network.m_name;
//	tmp.m_origin = m_network.m_origin;
//	m_network.networkWithReducedEdges(tmp);
//	m_network = tmp;
//	m_network.updateNodeEdgeConnectionPointers();
//	updateStatus();
//	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Added network"), m_network);
//	undo->push(); // modifies project and updates views


void SVNetworkEditDialog::on_pushButtonReduceDeadEnds_clicked()
{
	// not quite efficient, but safest method to transfer all parameters to new network
	VICUS::Network tmp = m_network;
	tmp.clear();
	m_network.networkWithoutDeadEnds(tmp);
	std::swap(m_network, tmp);
	m_network.updateNodeEdgeConnectionPointers();
	updateStatus();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
}


void SVNetworkEditDialog::on_pushButtonSizePipeDimensions_clicked()
{
	modifySizingParams();
	m_network.sizePipeDimensions(project().m_networkFluids[m_network.m_fluidID]);
	updateStatus();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("modified network"), m_network);
	undo->push(); // modifies project and updates views
}



