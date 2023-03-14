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
#include <IBK_FileReader.h>

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonObject>

#include <VICUS_Network.h>
#include <VICUS_NetworkLine.h>
#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include <IBKMK_UTM.h>

#include <QtExt_BrowseFilenameWidget.h>

#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"
#include "SVNetworkDialogSelectPipes.h"
#include "SVConversions.h"



SVNetworkImportDialog::SVNetworkImportDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkImportDialog)
{
	m_ui->setupUi(this);
	m_ui->lineEditMaxHeatingDemand->setup(0, 1e9, tr("HeatingDemand must be > 0 m!"), false, true);
	m_ui->lineEditOriginX->setup(0, std::numeric_limits<double>::max(), tr("Origin x-position"), true, true);
	m_ui->lineEditOriginY->setup(0, std::numeric_limits<double>::max(), tr("Origin y-position"), true, true);
	m_ui->lineEditOriginZ->setup(0, std::numeric_limits<double>::max(), tr("Origin z-position"), true, true);

	//setup the UTM Zone combobox
	QStringList texts;
	for(int i = 0; i < 60; i++){
		texts.append(QString("%1").arg(i));
	}
	m_ui->comboBoxUTMZone->clear();
	m_ui->comboBoxUTMZone->addItems(texts);
	m_ui->comboBoxUTMZone->setCurrentIndex(32);

	m_ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Import"));

	m_ui->groupBoxUTM->setChecked(false);
}


SVNetworkImportDialog::~SVNetworkImportDialog() {
	delete m_ui;
}


bool SVNetworkImportDialog::edit() {

	m_ui->lineEditPipelineFileName->setup(m_lastPipelineFilePath, true, true, tr("GeoJson-Files (*.geojson);;CSV-Files (*.csv)"),
										  SVSettings::instance().m_dontUseNativeDialogs);

	m_ui->lineEditSubStationFileName->setup(m_lastPipelineFilePath, true, true, tr("GeoJson-Files (*.geojson);;CSV-Files (*.csv)"),
											SVSettings::instance().m_dontUseNativeDialogs);

	updateUi();

	int res = exec();
	if (res) {
		if (m_ui->tabWidget->currentIndex()==0)
			importPipeline();
		else
			importSubStations();
	}
	return res;
}


void SVNetworkImportDialog::updateUi() {
	const VICUS::Project &p = project();
	m_ui->radioButtonAddToExistingNetwork->setEnabled(!p.m_geometricNetworks.empty());
	m_ui->radioButtonAddToExistingNetwork->setChecked(!p.m_geometricNetworks.empty());
	m_ui->radioButtonNewNetwork->setChecked(p.m_geometricNetworks.empty());
	toggleExistingOrNewNetwork(m_ui->radioButtonAddToExistingNetwork->isChecked());

	if (m_ui->tabImportPipes->isActiveWindow())
		checkIfPipelineImportIsEnabled();
	else
		checkIfSubStationImportIsEnabled();


	// update existing networks combobox
	m_ui->comboBoxNetworkSelectionBox->clear();
	m_ui->comboBoxNetworkSelectionBox->blockSignals(true);
	if (!p.m_geometricNetworks.empty()){
		for (auto it = p.m_geometricNetworks.begin(); it!=p.m_geometricNetworks.end(); ++it)
			m_ui->comboBoxNetworkSelectionBox->addItem(it->m_displayName, it->m_id);
	}
	m_ui->comboBoxNetworkSelectionBox->blockSignals(false);

	on_comboBoxNetworkSelectionBox_currentIndexChanged(m_ui->comboBoxNetworkSelectionBox->count()-1);
}



void SVNetworkImportDialog::importPipeline() {
	// get the file name from the line edit
	QString fname = m_ui->lineEditPipelineFileName->filename();

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

			m_network.m_availablePipes = m_availablePipes;

			readNetworkData(networkFile, m_network, p.nextUnusedID(), IT_Pipeline);

			double xOrigin = 0.5*(m_network.m_extends.left + m_network.m_extends.right);
			double yOrigin = 0.5*(m_network.m_extends.top + m_network.m_extends.bottom);
			qInfo() << QString(tr("Local origin at: ") + "%L1, %L2").arg(xOrigin).arg(yOrigin);
			m_network.setOrigin(IBKMK::Vector3D(xOrigin, yOrigin, 0));

			//convert to a std set
			std::set<QString> networkNames;
			for(int i=0; i<m_ui->comboBoxNetworkSelectionBox->count(); ++i)
				networkNames.insert(m_ui->comboBoxNetworkSelectionBox->itemText(i));
			m_network.m_displayName = VICUS::uniqueName(m_ui->lineEditNetworkName->text(), networkNames);

			m_network.updateExtends();
			SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("Added network"), m_network, true);
			undo->push(); // modifies project and updates views
		}

		// add to existing network
		else {
			unsigned int id = m_ui->comboBoxNetworkSelectionBox->currentData().toUInt();
			Q_ASSERT(VICUS::element(project().m_geometricNetworks, id) != nullptr);
			m_network = *VICUS::element(project().m_geometricNetworks, id);
			readNetworkData(networkFile, m_network, p.nextUnusedID(), IT_Pipeline);
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

	updateUi();
}


void SVNetworkImportDialog::importSubStations() {
	// get the file name from the line edit
	QString fname = m_ui->lineEditSubStationFileName->filename();

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

			readNetworkData(networkFile, m_network, ++m_network.m_id, IT_SubStation);

			double xOrigin = 0.5*(m_network.m_extends.left + m_network.m_extends.right);
			double yOrigin = 0.5*(m_network.m_extends.top + m_network.m_extends.bottom);
			qInfo() << QString(tr("Local origin at: ") + "%L1, %L2").arg(xOrigin).arg(yOrigin);
			m_network.setOrigin(IBKMK::Vector3D(xOrigin, yOrigin, 0));

			//convert to a std set
			std::set<QString> networkNames;
			for(int i=0; i<m_ui->comboBoxNetworkSelectionBox->count(); ++i)
				networkNames.insert(m_ui->comboBoxNetworkSelectionBox->itemText(i));
			m_network.m_displayName = VICUS::uniqueName(m_ui->lineEditNetworkName->text(), networkNames);

			m_network.updateExtends();
			SVUndoAddNetwork * undo = new SVUndoAddNetwork(tr("Added network"), m_network, true);
			undo->push(); // modifies project and updates views
		}

		// add to existing network
		else {
			unsigned int id = m_ui->comboBoxNetworkSelectionBox->currentData().toUInt();
			Q_ASSERT(VICUS::element(project().m_geometricNetworks, id) != nullptr);
			m_network = *VICUS::element(project().m_geometricNetworks, id);
			readNetworkData(networkFile, m_network, p.nextUnusedID(), IT_SubStation);
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

	updateUi();
}


void SVNetworkImportDialog::toggleExistingOrNewNetwork(bool readExisting){
	m_ui->comboBoxNetworkSelectionBox->setEnabled(readExisting);
	m_ui->lineEditNetworkName->setEnabled(!readExisting);
}


void SVNetworkImportDialog::convert2QJsonObject(const IBK::Path &filePath, QJsonObject & obj) const {
	FUNCID(SVNetworkImportDialog::convert2QJsonObject);
	QFile file;
	file.setFileName(filePath.absolutePath().c_str());
	if(!file.open(QIODevice::ReadOnly)){
		throw IBK::Exception("Json file couldn't be opened/found!", FUNC_ID);
	}

	QByteArray byteArray;
	byteArray = file.readAll();
	file.close();

	QJsonParseError parseError;
	QJsonDocument jsonDoc;
	jsonDoc = QJsonDocument::fromJson(byteArray, &parseError);
	if(parseError.error != QJsonParseError::NoError){
		throw IBK::Exception(QString("Parse error at %1:%1").arg(parseError.offset).arg(parseError.errorString()).toStdString(), FUNC_ID);
	}

	obj = jsonDoc.object();
}


void SVNetworkImportDialog::readNetworkData(const IBK::Path &fname, VICUS::Network &network, unsigned int nextId, ImportType importType) const {
	//stores the JsonObj if the file is a json file
	QJsonObject jsonObj;

	// can either be a network or a substation
	switch (importType) {

	case IT_Pipeline: {
		if(fname.extension() == "geojson"){
			convert2QJsonObject(fname, jsonObj);
			readGridFromGeoJson(network, jsonObj, nextId);
		}
		else {
			// if not geoJson then its a CSV File
			readGridFromCSV(network, fname, nextId);
		}
	}break;

	case IT_SubStation:{
		if(fname.extension() == "geojson"){
			convert2QJsonObject(fname, jsonObj);
			readBuildingsFromGeoJson(network, jsonObj, nextId);
			}
		else {
			// if not geoJson then its a CSV File
			readBuildingsFromCSV(network, fname, nextId);
		}
	}break;

	case NUM_IT:
		break;
	}

	network.updateExtends();

	// print out the network attributes
	qInfo() << QString(tr("Number of edges: ")+"%1").arg(m_network.m_edges.size());
	qInfo() << QString(tr("Number of nodes: ")+"%1").arg(m_network.m_nodes.size());
	qInfo() << QString(tr("Coordinate range: ")+"[%L1,%L2]...[%L3,%L4]").arg(m_network.m_extends.left)
										 .arg(m_network.m_extends.top).arg(m_network.m_extends.right).arg(m_network.m_extends.bottom);

}


void SVNetworkImportDialog::readGridFromCSV(VICUS::Network & network,const IBK::Path &filePath, unsigned int nextId) const{
	std::vector<std::string> cont;
	IBK::FileReader::readAll(filePath, cont, std::vector<std::string>());

	// extract vector of string-xy-pairs
	std::vector<std::string> tokens;
	for (std::string &line: cont){
		if (line.find("MULTILINESTRING ((") == std::string::npos)
			continue;
		IBK::trim(line, ",");
		IBK::trim(line, "\"");
		IBK::trim(line, "MULTILINESTRING ((");
		IBK::trim(line, "))");
		IBK::explode(line, tokens, ",", IBK::EF_NoFlags);

		// convert this vector to double and add it as a graph
		std::vector<std::vector<double> > polyLine;
		for (std::string str: tokens){
			std::vector<std::string> xyStr;
			IBK::explode(str, xyStr, " ", IBK::EF_NoFlags);
			double x,y;
			double z = 0;
			if (xyStr.size()>=2) {
				x = IBK::string2val<double>(xyStr[0]);
				y = IBK::string2val<double>(xyStr[1]);
			}
			else
				continue; // not a valid line
			if (xyStr.size()==3)
				z = IBK::string2val<double>(xyStr[2]);

			polyLine.push_back({x, y, z});
		}
		for (unsigned i=0; i<polyLine.size()-1; ++i){
			unsigned n1 = network.addNode(++nextId, IBKMK::Vector3D(polyLine[i][0], polyLine[i][1], polyLine[i][2]) - network.m_origin, VICUS::NetworkNode::NT_Mixer);
			unsigned n2 = network.addNode(++nextId, IBKMK::Vector3D(polyLine[i+1][0], polyLine[i+1][1], polyLine[i+1][2]) - network.m_origin, VICUS::NetworkNode::NT_Mixer);
			network.addEdge(++nextId, n1, n2, true, m_defaultPipeId);
		}
	}
}


void SVNetworkImportDialog::readGridFromGeoJson(VICUS::Network & network, const QJsonObject & jsonObj, unsigned int nextId) const {

	const QJsonArray features = jsonObj["features"].toArray();

	const VICUS::Database<VICUS::NetworkPipe> &pipeDB = SVSettings::instance().m_db.m_pipes;

	for(const QJsonValue & feature :  features) {

		QJsonObject geometry = feature.toObject()["geometry"].toObject();
		bool validObject = geometry["type"].toString() ==  "LineString" || geometry["type"].toString() ==  "MultiLineString";
		if(!validObject)
			continue;

		unsigned int pipeId = m_defaultPipeId;

		QJsonObject properties = feature.toObject()["properties"].toObject();
		double givenThickness = properties["da"].toDouble(); // should be in mm

		//find a fitting pipe from the available ones or keep default pipe
		double minDiff = std::numeric_limits<double>::max();
		for(unsigned int id : network.m_availablePipes){
			double pipeThickness = pipeDB[id]->m_para[VICUS::NetworkPipe::P_DiameterOutside].get_value("mm");
			double diff = std::abs(givenThickness - pipeThickness);

			if ( (diff<1) && (diff < minDiff) ) {
				minDiff = diff;
				pipeId = id;
			}
		}

		// MultiLineString
		if (geometry["type"].toString() ==  "MultiLineString") {
			for (const QJsonValue &val: geometry["coordinates"].toArray()) {
				importLineString(network, val.toArray(), pipeId, nextId);
			}
		}
		// LineString
		else {
			const QJsonArray &lineString = geometry["coordinates"].toArray();
			importLineString(network, lineString, pipeId, nextId);
		}
	}

}


void SVNetworkImportDialog::importLineString(VICUS::Network &network, const QJsonArray &lineString, unsigned int pipeId, unsigned int &nextId) const {

	std::vector<std::vector<double> > polyLine;

	for(const QJsonValue coordinates : lineString){
		double x,y;
		double z = 0;
		if (m_ui->groupBoxUTM->isChecked()) {
			double lon = coordinates.toArray()[0].toDouble();
			double lat = coordinates.toArray()[1].toDouble();
			//convert the LatLon Coordiantes to metric ones
			int utmZone = m_ui->comboBoxUTMZone->currentIndex();
			IBKMK::LatLonToUTMXY(lat, lon, utmZone, x, y);
		}
		else {
			x = coordinates.toArray()[0].toDouble();
			y = coordinates.toArray()[1].toDouble();
		}
		if (coordinates.toArray().size()==3)
			z = coordinates.toArray()[2].toDouble();

		polyLine.push_back({x, y, z});
	}

	for (unsigned i=0; i<polyLine.size()-1; ++i){
		unsigned n1 = network.addNode(++nextId, IBKMK::Vector3D(polyLine[i][0], polyLine[i][1], polyLine[i][2]) - network.m_origin, VICUS::NetworkNode::NT_Mixer);
		unsigned n2 = network.addNode(++nextId, IBKMK::Vector3D(polyLine[i+1][0], polyLine[i+1][1], polyLine[i+1][2]) - network.m_origin, VICUS::NetworkNode::NT_Mixer);
		network.addEdge(++nextId, n1, n2, true, pipeId);
	}
}


void SVNetworkImportDialog::readBuildingsFromCSV(VICUS::Network & network, const IBK::Path &filePath, unsigned int nextId) const {
	std::vector<std::string> cont;
	IBK::FileReader::readAll(filePath, cont, std::vector<std::string>());

	// extract vector of string-xy
	std::vector<std::string> lineSepStr;
	std::vector<std::string> xyStr;
	unsigned int counter = 0;
	for (std::string &line: cont){
		if (line.find("POINT") == std::string::npos)
			continue;
		IBK::explode(line, lineSepStr, ",", IBK::EF_NoFlags);
		IBK::trim(lineSepStr[0], "\"");
		IBK::trim(lineSepStr[0], "POINT ((");
		IBK::trim(lineSepStr[0], "))");
		IBK::explode(lineSepStr[0], xyStr, " ", IBK::EF_NoFlags);
		double x,y;
		double z = 0;
		if (xyStr.size()>=2) {
			x = IBK::string2val<double>(xyStr[0]);
			y = IBK::string2val<double>(xyStr[1]);
		}
		else
			continue; // not a valid point
		if (xyStr.size()==3)
			z = IBK::string2val<double>(xyStr[2]);


		// add node
		unsigned id = network.addNode(++nextId, IBKMK::Vector3D(x, y, z) - network.m_origin,
				VICUS::NetworkNode::NT_SubStation);

		double heatingDemand = 5000; // default value
		if (m_ui->lineEditMaxHeatingDemand->text().toDouble() > 0)
			heatingDemand = m_ui->lineEditMaxHeatingDemand->text().toDouble();
		QString name = m_ui->lineEditSubStationName->text();

		network.nodeById(id)->m_maxHeatingDemand = IBK::Parameter("MaxHeatingDemand", heatingDemand, "W");
		network.nodeById(id)->m_displayName = QString("%1_%2").arg(name).arg(++counter);
	}
}


void SVNetworkImportDialog::readBuildingsFromGeoJson(VICUS::Network & network, const QJsonObject jsonObj, unsigned int nextId) const {

	double defaultHeatingDemand = 5000; // default value
	if (m_ui->lineEditMaxHeatingDemand->text().toDouble() > 0)
		defaultHeatingDemand = m_ui->lineEditMaxHeatingDemand->text().toDouble();
	QString defaultName = m_ui->lineEditSubStationName->text();

	const QJsonArray features = jsonObj["features"].toArray();

	unsigned int counter = 0;

	for(const QJsonValue & feature :  features){

		QJsonObject geometry = feature.toObject()["geometry"].toObject();
		const QJsonObject &props = feature.toObject()["properties"].toObject();

		if(geometry["type"].toString() ==  "Point") {
			QJsonArray coordinates = geometry["coordinates"].toArray();
			importPoints(network, coordinates, props, defaultHeatingDemand, QString("%1_%2").arg(defaultName).arg(++counter), nextId);
		}
		else if(geometry["type"].toString() == "MultiPoint") {
			for (QJsonValue val: geometry["coordinates"].toArray()) {
				QJsonArray coordinates = val.toArray();
				importPoints(network, coordinates, props, defaultHeatingDemand, QString("%1_%2").arg(defaultName).arg(++counter), nextId);
			}
		}
	}
}


void SVNetworkImportDialog::importPoints(VICUS::Network & network, const QJsonArray & coordinates, const QJsonObject &properties, const double &defaultHeatingDemand,
										 const QString &defaultName, unsigned int & nextId) const {

	double x,y;
	double z = 0;
	if (m_ui->groupBoxUTM->isChecked()) {
		double lon = coordinates[0].toDouble();
		double lat = coordinates[1].toDouble();
		//covert the LatLon Coordiantes to metric ones
		int utmZone = m_ui->comboBoxUTMZone->currentIndex();
		IBKMK::LatLonToUTMXY(lat, lon, utmZone, x, y);
	}
	else {
		x = coordinates[0].toDouble();
		y = coordinates[1].toDouble();
	}

	if (coordinates.size()==3)
		z = coordinates[2].toDouble();

	double heatingDemand = defaultHeatingDemand;
	double val = properties["MaxHeatingDemand"].toDouble();
	if (val>0)
		heatingDemand = val * 1000; // expected in kW
	QString name = properties["Name"].toString();
	if (name.isEmpty())
		name = defaultName;

	// add node
	unsigned id = network.addNode(++nextId, IBKMK::Vector3D(x, y, z) - network.m_origin,
			VICUS::NetworkNode::NT_SubStation);
	network.nodeById(id)->m_maxHeatingDemand = IBK::Parameter("MaxHeatingDemand", heatingDemand, "W");
	network.nodeById(id)->m_displayName = name;
}


void SVNetworkImportDialog::on_radioButtonNewNetwork_clicked(bool checked) {
	toggleExistingOrNewNetwork(!checked);
}


void SVNetworkImportDialog::on_radioButtonAddToExistingNetwork_clicked(bool checked) {
	toggleExistingOrNewNetwork(checked);
}


void SVNetworkImportDialog::on_pushButtonSelectAvailablePipes_clicked() {

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

	// get the id of the selected default pipe
	m_defaultPipeId = SVMainWindow::instance().dbPipeEditDialog()->select(m_defaultPipeId);
	const VICUS::Database<VICUS::NetworkPipe> &pipesDB = SVSettings::instance().m_db.m_pipes;
	// set name in the ui
	m_ui->labelDefaulPipeName->setText(QtExt::MultiLangString2QString(pipesDB[m_defaultPipeId]->m_displayName));

	//check if the import is ready
	checkIfPipelineImportIsEnabled();
}


void SVNetworkImportDialog::checkIfPipelineImportIsEnabled(){
	bool buttonEnabled = false;
	// check if a filename is specified
	if(!m_ui->lineEditPipelineFileName->filename().isEmpty()){
		// check if a valid default pipe is selected
		if(m_defaultPipeId != VICUS::INVALID_ID){
			buttonEnabled = true;
		}
	}
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(buttonEnabled);
}


void SVNetworkImportDialog::checkIfSubStationImportIsEnabled(){
	bool buttonEnabled = false;
	// check if a filename is specified
	if(!m_ui->lineEditSubStationFileName->filename().isEmpty()){
		// check if the max Heating Demand is greater than 0
		if(m_ui->lineEditMaxHeatingDemand->text().toInt() > 0){
			buttonEnabled = true;
		}
	}
	m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(buttonEnabled);
}


void SVNetworkImportDialog::on_lineEditPipelineFileName_editingFinished() {
	checkIfPipelineImportIsEnabled();
}


void SVNetworkImportDialog::on_lineEditMaxHeatingDemand_editingFinished() {
	checkIfSubStationImportIsEnabled();
}


void SVNetworkImportDialog::on_lineEditSubStationFileName_editingFinished() {
	checkIfSubStationImportIsEnabled();
}


void SVNetworkImportDialog::on_comboBoxNetworkSelectionBox_currentIndexChanged(int /*index*/) {
	int currentId = m_ui->comboBoxNetworkSelectionBox->currentData().toInt();
	if (currentId>0) {
		const VICUS::Network *net = VICUS::element(project().m_geometricNetworks, (unsigned int)currentId);
		Q_ASSERT(net != nullptr);
		m_ui->lineEditOriginX->setValue(net->m_origin.m_x);
		m_ui->lineEditOriginY->setValue(net->m_origin.m_y);
		m_ui->lineEditOriginZ->setValue(net->m_origin.m_z);
	}
	else {
		m_ui->lineEditOriginX->clear();
		m_ui->lineEditOriginY->clear();
		m_ui->lineEditOriginZ->clear();
	}
}
