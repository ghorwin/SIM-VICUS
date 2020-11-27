#include "SVNetworkEditDialog.h"
#include "ui_SVNetworkEditDialog.h"

#include "VICUS_Project.h"
#include "SVProjectHandler.h"
#include "SVUndoAddNetwork.h"

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
	if (!p.m_networks.empty()){
		m_existingNetworksMap.clear();
		for (auto it = p.m_networks.begin(); it!=p.m_networks.end(); ++it)
			m_existingNetworksMap.insert(QString::fromStdString(it->m_name), it->m_id);
		m_ui->comboBoxSelectNetwork->clear();
		m_ui->comboBoxSelectNetwork->addItems(QStringList(m_existingNetworksMap.keys()));
	}

	setNetwork();

	if (exec() != QDialog::Accepted)
		return;

}

void SVNetworkEditDialog::updateStatus()
{
	m_ui->labelEdgeCount->setText(QString("%1").arg(m_network.m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_network.m_nodes.size()));
	m_ui->labelNetworkConnected->setText(QString("%1").arg(m_network.checkConnectedGraph()));
	m_ui->labelTotalLength->setText(QString("%1").arg(m_network.totalLength()));
}

void SVNetworkEditDialog::setNetwork()
{
	unsigned id = m_existingNetworksMap.value(m_ui->comboBoxSelectNetwork->currentText());
	m_network = *project().element(project().m_networks, id);
	updateStatus();
}

void SVNetworkEditDialog::on_pushButtonGenerateIntersections_clicked()
{
	m_network.generateIntersections();
	updateStatus();

	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Added network"), m_network);
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
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Added network"), m_network);
	undo->push(); // modifies project and updates views
}

void SVNetworkEditDialog::on_pushButtonCalculateLength_clicked()
{
	m_network.calculateLengths();
	updateStatus();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Added network"), m_network);
	undo->push(); // modifies project and updates views
}

void SVNetworkEditDialog::on_pushButtonReduceRedundants_clicked()
{
	VICUS::Network tmp = m_network;
	tmp.clear();
	m_network.networkWithReducedEdges(tmp);
	m_network = tmp;
	updateStatus();
	SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Added network"), m_network);
	undo->push(); // modifies project and updates views
}
