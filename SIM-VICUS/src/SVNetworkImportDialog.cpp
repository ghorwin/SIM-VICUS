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

#include <SVConversions.h>

#include <VICUS_Network.h>
#include <VICUS_NetworkLine.h>
#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVNetworkDialogSelectPipes.h"


SVNetworkImportDialog::SVNetworkImportDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkImportDialog)
{
	m_ui->setupUi(this);
	m_ui->lineEditMaxHeatingDemand->setup(0, 1e9, tr("HeatingDemand must be > 0 m!"), false, true);
}


SVNetworkImportDialog::~SVNetworkImportDialog() {
	delete m_ui;
}


bool SVNetworkImportDialog::edit() {

	// store pointer to network for dialog functions to access
	const VICUS::Project &p = project();

	m_ui->radioButtonAddToExistingNetwork->setEnabled(!p.m_geometricNetworks.empty());
	m_ui->radioButtonAddToExistingNetwork->setChecked(!p.m_geometricNetworks.empty());
	m_ui->radioButtonNewNetwork->setChecked(p.m_geometricNetworks.empty());
	toggleExistingOrNewNetwork(m_ui->radioButtonAddToExistingNetwork->isChecked());

	// update existing networks combobox
	if (!p.m_geometricNetworks.empty()){
		m_existingNetworksMap.clear();
		for (auto it = p.m_geometricNetworks.begin(); it!=p.m_geometricNetworks.end(); ++it)
			m_existingNetworksMap.insert(it->m_displayName, it->m_id);
		m_ui->comboBoxNetworkSelectionBox->clear();
		m_ui->comboBoxNetworkSelectionBox->addItems(QStringList(m_existingNetworksMap.keys()));
	}

	// TODO Anton:
	// - QtExt_fileSelectionWidget verwenden für die Auswahl der Dateien
	// - pipeline import button nur enablen wenn eine entsprechende Datei ausgewählt wurde und min. default pipe diameter gesetzt ist
	// - substation import button nur enablen wenn max. heating demand valid ist
	// - Buttons die einen Dialog aufrufen immer mit "..." am Ende beschriften
	// - Doku aller Funktionen und Membervariablen in Headerdatei (vergesse ich selbst oft :) mit Dokustring: /*! hier die doku */

	return exec();
}

void SVNetworkImportDialog::on_pushButtonSelectPipelineFile_clicked() {
	// ask user to select geojson or csv file
	QString fname = QFileDialog::getOpenFileName(this, tr("Select geoJson-file or CSV-file with GIS data"),
												 m_lastPipelineFilePath, tr("GeoJson-Files (*.geojson);;CSV-Files (*.csv)"), nullptr,
												 SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
	if (fname.isEmpty())
		return;
	m_lastPipelineFilePath = fname;

	// TODO Anton: das entfällt dann ...
	m_ui->lineEditPipelineFileName->setText(fname);

}


void SVNetworkImportDialog::on_pushButtonImportPipeline_clicked() {
	// get the file name from the line edit
	QString fname = m_ui->lineEditPipelineFileName->text();

	// make sure we have updated ids
	const VICUS::Project &p = project();

	// try to read network
	IBK::Path networkFile(fname.toStdString());
	try {
		// read new network
		if (m_ui->radioButtonNewNetwork->isChecked()){
			// create new network
			m_network = VICUS::Network ();
			m_network.m_id = p.nextUnusedID();

			readNetworkData(networkFile, m_network, ++m_network.m_id, Pipeline);

			double xOrigin = 0.5*(m_network.m_extends.left + m_network.m_extends.right);
			double yOrigin = 0.5*(m_network.m_extends.top + m_network.m_extends.bottom);
			qInfo() << QString(tr("Local origin at: ") + "%L1, %L2").arg(xOrigin).arg(yOrigin);
			m_network.setOrigin(IBKMK::Vector3D(xOrigin, yOrigin, 0));


			// TODO Anton: use VICUS::uniqueName(), remove this function
			m_network.m_displayName = uniqueName(m_ui->lineEditNetworkName->text());


			SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("Added network"), m_network);
			undo->push(); // modifies project and updates views
		}

		// add to existing network
		else {
			unsigned int id = m_existingNetworksMap.value(m_ui->comboBoxNetworkSelectionBox->currentText());
			m_network = *VICUS::element(project().m_geometricNetworks, id);
			readNetworkData(networkFile, m_network, p.nextUnusedID(), Pipeline);
			//transfer the availabe pipes (if given) to the network
			if(!m_availablePipes.empty()){
				m_network.m_availablePipes = m_availablePipes;
			}

			qInfo() << QString(tr("Local origin at: ") + "%L1, %L2").arg(m_network.m_origin.m_x).arg(m_network.m_origin.m_y);

			m_network.updateExtends();
			SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), m_network);
			undo->push(); // modifies project and updates views
		}

	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical(this, QString(), tr("Error reading GIS data file:\n%1").arg(ex.what()));
		return;
	}

}


void SVNetworkImportDialog::on_pushButtonSelectSubStationFile_clicked() {
	// ask user to select geo json file
	QString fname = QFileDialog::getOpenFileName(this, tr("Select geoJson-file or CSV-file with GIS data"),
												 m_lastSubstationFilePath, tr("GeoJson-Files (*.geojson);;CSV-Files (*.csv)"), nullptr,
												 SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options());
	if (fname.isEmpty())
		return;
	m_lastSubstationFilePath = fname;

	m_ui->lineEditSubStationFileName->setText(fname);
}



void SVNetworkImportDialog::on_pushButtonImportSubStation_clicked() {
// get the file name from the line edit
	QString fname = m_ui->lineEditSubStationFileName->text();

	// make sure we have updated ids
	const VICUS::Project &p = project();

	// try to read network
	IBK::Path networkFile(fname.toStdString());
	try {
		// read new network
		if (m_ui->radioButtonNewNetwork->isChecked()){
			// create new network
			m_network = VICUS::Network ();
			m_network.m_id = p.nextUnusedID();
			//transfer the availabe pipes to the new network
			m_network.m_availablePipes = m_availablePipes;

			readNetworkData(networkFile, m_network, ++m_network.m_id, SubStation);

			double xOrigin = 0.5*(m_network.m_extends.left + m_network.m_extends.right);
			double yOrigin = 0.5*(m_network.m_extends.top + m_network.m_extends.bottom);
			qInfo() << QString(tr("Local origin at: ") + "%L1, %L2").arg(xOrigin).arg(yOrigin);
			m_network.setOrigin(IBKMK::Vector3D(xOrigin, yOrigin, 0));

			// TODO Anton: use VICUS::uniqueName(), remove this function
			m_network.m_displayName = uniqueName(m_ui->lineEditNetworkName->text());

			SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("Added network"), m_network);
			undo->push(); // modifies project and updates views
		}

		// add to existing network
		else {
			unsigned int id = m_existingNetworksMap.value(m_ui->comboBoxNetworkSelectionBox->currentText());
			m_network = *VICUS::element(project().m_geometricNetworks, id);
			readNetworkData(networkFile, m_network, p.nextUnusedID(), SubStation);
			//transfer the availabe pipes (if given) to the network
			if(!m_availablePipes.empty()){
				m_network.m_availablePipes = m_availablePipes;
			}

			qInfo() << QString(tr("Local origin at: ") + "%L1, %L2").arg(m_network.m_origin.m_x).arg(m_network.m_origin.m_y);

			m_network.updateExtends();
			SVUndoModifyNetwork * undo = new SVUndoModifyNetwork(tr("Network visualization properties updated"), m_network);
			undo->push(); // modifies project and updates views
		}

	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical(this, QString(), tr("Error reading GIS data file:\n%1").arg(ex.what()));
		return;
	}
}


void SVNetworkImportDialog::toggleExistingOrNewNetwork(bool readExisting) {
	m_ui->comboBoxNetworkSelectionBox->setEnabled(readExisting);
	m_ui->lineEditNetworkName->setEnabled(!readExisting);
}


void SVNetworkImportDialog::readNetworkData(const IBK::Path &fname, VICUS::Network &network, unsigned int nextId, ImportType importType) const {

	// can either be a network or a substation
	switch (importType) {

		case Pipeline:
		if(fname.extension() == "geojson"){
			int defaultPipeId = (m_defaultPipe == nullptr)? VICUS::INVALID_ID : m_defaultPipe->m_id;
			network.readGridFromGeoJson(fname, nextId, defaultPipeId, SVSettings::instance().m_db.m_pipes);
		}
		else {
			// if not geoJson then its a CSV File
			network.readGridFromCSV(fname, nextId);
		}
		break;

		case SubStation:
		double heatingDemand = m_ui->lineEditMaxHeatingDemand->text().toDouble();
		if(fname.extension() == "geojson"){
			network.readBuildingsFromGeoJson(fname, heatingDemand, nextId);
			}
		else {
			// if not geoJson then its a CSV File
			network.readBuildingsFromCSV(fname, heatingDemand, nextId);
		}
		break;
	}

	network.updateExtends();

	qInfo() << QString(tr("Number of edges: ")+"%1").arg(m_network.m_edges.size());
	qInfo() << QString(tr("Number of nodes: ")+"%1").arg(m_network.m_nodes.size());
	qInfo() << QString(tr("Coordinate range: ")+"[%L1,%L2]...[%L3,%L4]").arg(m_network.m_extends.left)
										 .arg(m_network.m_extends.top).arg(m_network.m_extends.right).arg(m_network.m_extends.bottom);
}



// TODO Anton: use VICUS::uniqueName(), remove this function
QString SVNetworkImportDialog::uniqueName(const QString &name) {
	QString uniqueName = name;
	for (auto it = project().m_geometricNetworks.begin(); it!=project().m_geometricNetworks.end(); ++it){
		if (uniqueName == it->m_displayName)
			uniqueName += "_2";
	}
	return uniqueName;
}


void SVNetworkImportDialog::on_radioButtonNewNetwork_clicked(bool checked) {
	toggleExistingOrNewNetwork(!checked);
}


void SVNetworkImportDialog::on_radioButtonAddToExistingNetwork_clicked(bool checked) {
	toggleExistingOrNewNetwork(checked);
}


void SVNetworkImportDialog::on_pushButtonSelectAvailablePipes_clicked() {

	// TODO Anton: ?
	if (m_ui->radioButtonNewNetwork->isChecked()){

	}

	// create temporary network to store the pipes
	VICUS::Network temp = VICUS::Network ();
	temp.m_displayName = "Imported Network";
	// fill with already selected pipes
	temp.m_availablePipes = m_availablePipes;
	SVNetworkDialogSelectPipes *dialog = new SVNetworkDialogSelectPipes(this);
	dialog->edit(temp);
	// store the selected pipes
	m_availablePipes = temp.m_availablePipes;
}


void SVNetworkImportDialog::on_pushButtonSelectDefaultPipe_clicked() {

	// TODO Anton: pass current default pipeId to dialog select function
	// vielleicht besser die pipeId speichern (und mit INVALID_ID initialisieren) als den pointer ...
	// get the id of the selected default pipe
	unsigned int defaultPipeId = SVMainWindow::instance().dbPipeEditDialog()->select(-1);
	if(defaultPipeId == 4294967295){
		//invalid ID
		return;
	}
	// safe the reference from the db
	const VICUS::Database<VICUS::NetworkPipe> &pipesDB = SVSettings::instance().m_db.m_pipes;
	m_defaultPipe = pipesDB[defaultPipeId];

	// set name in the ui
	m_ui->labelDefaulPipeName->setText(QtExt::MultiLangString2QString(m_defaultPipe->m_displayName));
	// enable import button
	m_ui->pushButtonImportPipeline->setEnabled(true);
}

