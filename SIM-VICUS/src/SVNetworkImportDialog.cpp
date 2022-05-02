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

#include "SVNetworkImportDialog.h"
#include "ui_SVNetworkImportDialog.h"

#include "SVUndoAddNetwork.h"
#include "SVUndoModifyNetwork.h"

#include <IBK_Path.h>

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <VICUS_Network.h>
#include <VICUS_NetworkLine.h>
#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include "SVSettings.h"
#include "SVProjectHandler.h"


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
			m_existingNetworksMap.insert(it->m_displayName, it->m_id);
		m_ui->comboBoxNetworkSelectionBox->clear();
		m_ui->comboBoxNetworkSelectionBox->addItems(QStringList(m_existingNetworksMap.keys()));
	}

	return exec();
}


void SVNetworkImportDialog::on_pushButtonGISNetwork_clicked() {
	// ask user to select csv file

	// TODO Hauke, store last selected directory and restore instead of hardcoding relative path
	QString fname = QFileDialog::getOpenFileName(this, tr("Select csv-file with GIS data"),
												 "../../data/vicus/GeometryTests/Network", tr("CSV-Files (*.csv)"), nullptr,
												 SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
												 );
	if (fname.isEmpty())
		return;

	// make sure we have updated ids
	const VICUS::Project &p = project();

	// try to read network
	IBK::Path networkFile(fname.toStdString());
	try {
		// read new network
		if (m_ui->radioButtonNewNetwork->isChecked()){
			m_network = VICUS::Network ();
			m_network.m_id = p.nextUnusedID();
			readNetworkData(networkFile, m_network, ++m_network.m_id);

			double xOrigin = 0.5*(m_network.m_extends.left + m_network.m_extends.right);
			double yOrigin = 0.5*(m_network.m_extends.top + m_network.m_extends.bottom);
			m_ui->lineEditXOrigin->setText( QString("%L1").arg(xOrigin));
			m_ui->lineEditYOrigin->setText( QString("%L1").arg(yOrigin));
			m_network.setOrigin(IBKMK::Vector3D(xOrigin, yOrigin, 0));

			m_network.m_displayName = uniqueName(m_ui->lineEditNetworkName->text());

			SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("Added network"), m_network);
			undo->push(); // modifies project and updates views
		}

		// add to existing network
		else {
			unsigned int id = m_existingNetworksMap.value(m_ui->comboBoxNetworkSelectionBox->currentText());
			m_network = *VICUS::element(project().m_geometricNetworks, id);
			readNetworkData(networkFile, m_network, p.nextUnusedID());

			m_ui->lineEditXOrigin->setText( QString("%L1").arg(m_network.m_origin.m_x));
			m_ui->lineEditYOrigin->setText( QString("%L1").arg(m_network.m_origin.m_y));

			m_network.updateExtends();
			unsigned int networkIndex = (unsigned int) m_ui->comboBoxNetworkSelectionBox->currentIndex();
			SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network modified"), networkIndex, m_network);
			undo->push(); // modifies project and updates views
		}

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

void SVNetworkImportDialog::readNetworkData(const IBK::Path &fname, VICUS::Network &network, unsigned int nextId) const {

	if (m_ui->radioButtonEdges->isChecked()){
		network.readGridFromCSV(fname, nextId);
	}
	else{
		network.readBuildingsFromCSV(fname, QLocale().toDouble(m_ui->lineEditHeatingDemand->text()), nextId);
	}
	network.updateExtends();

	m_ui->labelEdgeCount->setText(QString("%1").arg(m_network.m_edges.size()));
	m_ui->labelNodeCount->setText(QString("%1").arg(m_network.m_nodes.size()));
	m_ui->labelCoordinateRange->setText( QString("[%L1,%L2]...[%L3,%L4]").arg(m_network.m_extends.left)
										 .arg(m_network.m_extends.top).arg(m_network.m_extends.right).arg(m_network.m_extends.bottom));
}


QString SVNetworkImportDialog::uniqueName(const QString &name) {
	QString uniqueName = name;
	for (auto it = project().m_geometricNetworks.begin(); it!=project().m_geometricNetworks.end(); ++it){
		if (uniqueName == it->m_displayName)
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
