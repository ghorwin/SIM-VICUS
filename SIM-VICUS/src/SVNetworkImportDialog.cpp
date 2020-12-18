#include "SVNetworkImportDialog.h"
#include "ui_SVNetworkImportDialog.h"

#include "SVUndoAddNetwork.h"
#include "SVUndoModifyExistingNetwork.h"

#include <IBK_Path.h>

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <VICUS_Network.h>
#include <VICUS_NetworkLine.h>
#include <VICUS_Project.h>


// TODO Hauke: remove this later
#include "SVSettings.h"
#include "SVUndoAddFluid.h"


SVNetworkImportDialog::SVNetworkImportDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkImportDialog)
{
	m_ui->setupUi(this);
	m_ui->lineEditHeatingDemand->setText(QString("%L1").arg(0.5*(0)));
}


SVNetworkImportDialog::~SVNetworkImportDialog() {
	delete m_ui;
}


bool SVNetworkImportDialog::edit() {

	// store pointer to network for dialog functions to access
	const VICUS::Project &p = project();

	m_ui->groupBoxSelectNetwork->setEnabled(true);
	toggleReadEdges(m_ui->radioButtonEdges->isChecked());
	m_ui->radioButtonAddToExistingNetwork->setEnabled(!p.m_geometricNetworks.empty());
	m_ui->radioButtonAddToExistingNetwork->setChecked(!p.m_geometricNetworks.empty());
	toggleReadExistingNetwork(m_ui->radioButtonAddToExistingNetwork->isChecked());


	// update existing networks combobox
	if (!p.m_geometricNetworks.empty()){
		m_existingNetworksMap.clear();
		for (auto it = p.m_geometricNetworks.begin(); it!=p.m_geometricNetworks.end(); ++it)
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

		// generate id, set name
		m_network.m_id = VICUS::Project::uniqueId(p.m_geometricNetworks);
		m_network.m_name = uniqueName(m_ui->lineEditNetworkName->text().toStdString());


		// TODO Hauke: remove this later

		// fluids
		VICUS::NetworkFluid fluid;
		fluid.defaultFluidWater(0);

		// write database
		SVSettings &settings = SVSettings::instance();
		settings.m_dbFluids[fluid.m_id] = fluid;
		settings.writeDatabase();

		// read database, add pipes to network
		settings.readDatabase();
		for (auto it = settings.m_dbPipes.begin(); it != settings.m_dbPipes.end(); ++it)
			m_network.m_networkPipeDB.push_back(it->second);

		// add fluid to project
		fluid = settings.m_dbFluids[0];
		SVUndoAddFluid * undoF = new SVUndoAddFluid(tr("Added fluid"), fluid);
		undoF->push();

		// set network fluid id
		m_network.m_fluidID = project().m_networkFluids[0].m_id;


		// TODO Hauke: rmeove this later
//		m_network.assignSourceNode(IBKMK::Vector3D(481691, 5577509.9, 0));

		m_network.updateExtends();

		SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("Added network"), m_network);
		undo->push(); // modifies project and updates views



	}
	else{
		m_network.updateExtends();
		SVUndoModifyExistingNetwork * undo = new SVUndoModifyExistingNetwork(tr("Added network"), m_network);
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
			m_network = *project().element(project().m_geometricNetworks, id);
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


unsigned SVNetworkImportDialog::generateId()
{
	unsigned id=0;
	for (auto it = project().m_geometricNetworks.begin(); it!=project().m_geometricNetworks.end(); ++it){
		if (id == it->m_id)
			++id;
		else
			return id;
	}
	return id;
}


std::string SVNetworkImportDialog::uniqueName(const std::string &name)
{
	std::string uniqueName = name;
	for (auto it = project().m_geometricNetworks.begin(); it!=project().m_geometricNetworks.end(); ++it){
		if (uniqueName == it->m_name)
			uniqueName += "_2";
	}
	return uniqueName;
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
