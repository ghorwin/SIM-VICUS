#include "SVNetworkImportDialog.h"
#include "ui_SVNetworkImportDialog.h"

#include "SVUndoAddNetwork.h"

#include <IBK_Path.h>

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <VICUS_Network.h>
#include <VICUS_NetworkLine.h>
#include <VICUS_Project.h>


SVNetworkImportDialog::SVNetworkImportDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkImportDialog)
{
	m_ui->setupUi(this);
}


SVNetworkImportDialog::~SVNetworkImportDialog() {
	delete m_ui;
}


bool SVNetworkImportDialog::edit() {

	// store pointer to network for dialog functions to access
	const VICUS::Project &p = project();

	m_ui->groupBoxSelectNetwork->setEnabled(true);
	toggleReadEdges(m_ui->radioButtonEdges->isChecked());
	toggleReadExistingNetwork(m_ui->radioButtonAddToExistingNetwork->isChecked());
	m_ui->radioButtonAddToExistingNetwork->setEnabled(!p.m_networks.empty());

	// update existing networks combobox
	if (!p.m_networks.empty()){
		m_existingNetworksMap.clear();
		for (auto it = p.m_networks.begin(); it!=p.m_networks.end(); ++it)
			m_existingNetworksMap.insert(QString::fromStdString(it->m_name), it->m_id);
		m_ui->comboBoxNetworkSelectionBox->clear();
		m_ui->comboBoxNetworkSelectionBox->addItems(QStringList(m_existingNetworksMap.keys()));
	}


	if (exec() != QDialog::Accepted)
		return false;

	// if it is a new network, allow to specify the origin
	if (m_ui->radioButtonNewNetwork->isChecked()){
		double x = QLocale().toDouble(m_ui->lineEditXOrigin->text());
		double y = QLocale().toDouble(m_ui->lineEditYOrigin->text());
		// set origin and normalize existing positions
		m_network.setOrigin(IBKMK::Vector3D(x, y, 0));
		m_network.m_name = m_ui->lineEditNetworkName->text().toStdString();

		m_network.updateExtends();

		SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("Added network"), m_network);
		undo->push(); // modifies project and updates views
	}
	else{
		SVUndoAddToExistingNetwork * undo = new SVUndoAddToExistingNetwork(tr("Added network"), m_network);
		undo->push(); // modifies project and updates views
	}

	return true;
}


void SVNetworkImportDialog::on_pushButtonGISNetwork_clicked() {
	// ask user to select csv file
	QString fname = QFileDialog::getOpenFileName(this, tr("Select csv-file with GIS data"),
												 "../../data/vicus/GeometryTests/Network", tr("CSV-Files (*.csv)"));
	if (fname.isEmpty())
		return;

	// try to read network
	IBK::Path networkFile(fname.toStdString());
	try {
		// read new network
		if (m_ui->radioButtonNewNetwork->isChecked()){
			m_network = VICUS::Network ();
			readNetworkData(networkFile, m_network);

			m_ui->lineEditXOrigin->setText( QString("%L1").arg(0.5*(m_network.m_extends.left + m_network.m_extends.right)));
			m_ui->lineEditYOrigin->setText( QString("%L1").arg(0.5*(m_network.m_extends.top + m_network.m_extends.bottom)));
		}
		// add to existing network
		else {
			unsigned id = m_existingNetworksMap.value(m_ui->comboBoxNetworkSelectionBox->currentText());
			m_network = *project().element(project().m_networks, id);
			readNetworkData(networkFile, m_network);

			m_ui->lineEditXOrigin->setText( QString("%L1").arg(m_network.m_origin.m_x));
			m_ui->lineEditYOrigin->setText( QString("%L1").arg(m_network.m_origin.m_y));
		}

		// dont allow changing network anymore
		m_ui->groupBoxSelectNetwork->setEnabled(false);

	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical(this, QString(), tr("Error reading GIS data file:\n%1").arg(ex.what()));
		return;
	}

}

void SVNetworkImportDialog::toggleReadEdges(bool readEdges)
{
	m_ui->lineEditHeatingDemand->setEnabled(!readEdges);
}

void SVNetworkImportDialog::toggleReadExistingNetwork(bool readExisting)
{
	m_ui->comboBoxNetworkSelectionBox->setEnabled(readExisting);
	m_ui->lineEditXOrigin->setEnabled(!readExisting);
	m_ui->lineEditYOrigin->setEnabled(!readExisting);
	m_ui->lineEditNetworkName->setEnabled(!readExisting);
}

void SVNetworkImportDialog::readNetworkData(const IBK::Path &fname, VICUS::Network &network) const
{
	if (m_ui->radioButtonEdges->isChecked()){
		network.readGridFromCSV(fname);
	}
	else{
		network.readBuildingsFromCSV(fname, QLocale().toDouble(m_ui->lineEditHeatingDemand->text()));
	}
	network.updateExtends();

	m_ui->labelEdgeCount->setText(QString("%1").arg(m_network.m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_network.m_nodes.size()));
	m_ui->labelCoordinateRange->setText( QString("[%L1,%L2]...[%L3,%L4]").arg(m_network.m_extends.left)
										 .arg(m_network.m_extends.top).arg(m_network.m_extends.right).arg(m_network.m_extends.bottom));
}


void SVNetworkImportDialog::on_radioButtonNewNetwork_clicked(bool checked)
{
	toggleReadExistingNetwork(!checked);
}

void SVNetworkImportDialog::on_radioButtonAddToExistingNetwork_clicked(bool checked)
{
	toggleReadExistingNetwork(checked);
}

void SVNetworkImportDialog::on_radioButtonEdges_clicked(bool checked)
{
	toggleReadEdges(checked);
}

void SVNetworkImportDialog::on_radioButtonNodes_clicked(bool checked)
{
	toggleReadEdges(!checked);
}
