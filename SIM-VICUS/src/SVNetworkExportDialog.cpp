#include <QFileDialog>

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include <QFile>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonObject>

#include <QPushButton>

#include <IBKMK_UTM.h>
#include <IBK_FileUtils.h>

#include <fstream>

#include "SVProjectHandler.h"
#include "SVNetworkExportDialog.h"
#include "ui_SVNetworkExportDialog.h"
#include "SVSettings.h"

SVNetworkExportDialog::SVNetworkExportDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkExportDialog)
{
	m_ui->setupUi(this);

	setWindowTitle("Export Network as geoJSON");

	m_ui->radioButtonNorthern->setChecked(true);

	//setup the UTM Zone combobox
	m_ui->comboBoxUTMZone->clear();
	for(int i = 0; i < 60; i++){
		m_ui->comboBoxUTMZone->addItem(QString("%1").arg(i));
	}
	m_ui->comboBoxUTMZone->setCurrentIndex(32);
}


SVNetworkExportDialog::~SVNetworkExportDialog() {
	delete m_ui;
}


bool SVNetworkExportDialog::edit() {

	// store pointer to network for dialog functions to access
	const VICUS::Project &p = project();

	m_ui->lineEditExportFileName->setup("", true, false, tr("GeoJson-Files (*.geojson)"), SVSettings::instance().m_dontUseNativeDialogs);

	// update existing networks combobox
	m_ui->comboBoxNetworkSelection->clear();
	if (!p.m_geometricNetworks.empty()){
		for (auto it = p.m_geometricNetworks.begin(); it!=p.m_geometricNetworks.end(); ++it)
			m_ui->comboBoxNetworkSelection->addItem(it->m_displayName, it->m_id);
	}

	updateOkButtonEnableState();

	return exec();
}


void SVNetworkExportDialog::exportToGeoJson(unsigned int networkId) {
	FUNCID(SVNetworkExportDialog::exportToGeoJson);

	const VICUS::Network *network = VICUS::element(project().m_geometricNetworks, networkId);
	Q_ASSERT(network != nullptr);

	VICUS::Database<VICUS::NetworkPipe> &pipeDB = SVSettings::instance().m_db.m_pipes;

	QJsonObject jsonFile;
	// fill with meta data
	jsonFile["type"] = "FeatureCollection";
	jsonFile["name"] = network->m_displayName;
	QJsonObject crs;
	crs["type"] = "name";
	QJsonObject namedCrs;
	namedCrs["name"] = "urn:ogc:def:crs:OGC:1.3:CRS84"; //standard crs
	crs["properties"] = namedCrs;
	jsonFile["crs"] = crs;

	QJsonArray features;
	int featureId = 0;

	// export the pipeline edge by edge if selected
	if(m_ui->checkBoxExportPipeline->isChecked()){
		for(const VICUS::NetworkEdge & edge : network->m_edges){
			QJsonObject feature;
			feature["type"] = "Feature";
			QJsonObject properties;
			properties["id"] = featureId;

			double pipeThickness = pipeDB[edge.m_idPipe]->m_para[VICUS::NetworkPipe::P_DiameterOutside].get_value("mm");
			properties["da"] = pipeThickness;

			feature["properties"] = properties;
			QJsonObject geometry;
			geometry["type"] = "LineString";

			QJsonArray coordinates;
			// each feature contains coordinates of the node of one edge
			QJsonArray coordinate1;
			QJsonArray coordinate2;

			const VICUS::NetworkNode * node1 = edge.m_node1;
			const VICUS::NetworkNode * node2 = edge.m_node2;
			Q_ASSERT(node1 != nullptr);
			Q_ASSERT(node2 != nullptr);

			int utmZone = m_ui->comboBoxUTMZone->currentIndex();
			bool isSouthern = m_ui->radioButtonSouthern->isChecked();

			double x1 = node1->m_position.m_x + network->m_origin.m_x;
			double y1 = node1->m_position.m_y + network->m_origin.m_y;
			if (m_ui->groupBoxToUTM->isChecked()) {
				double lat1, lon1;
				IBKMK::UTMXYToLatLon (x1, y1, utmZone, isSouthern, lat1, lon1);
				coordinate1.push_back(IBKMK::RadToDeg(lon1));
				coordinate1.push_back(IBKMK::RadToDeg(lat1));
			}
			else {
				coordinate1.push_back(x1);
				coordinate1.push_back(y1);
			}

			double lat2, lon2;
			double x2 = node2->m_position.m_x + network->m_origin.m_x;
			double y2 = node2->m_position.m_y + network->m_origin.m_y;
			if (m_ui->groupBoxToUTM->isChecked()) {
				IBKMK::UTMXYToLatLon (x2, y2, utmZone, isSouthern, lat2, lon2);
				coordinate2.push_back(IBKMK::RadToDeg(lon2));
				coordinate2.push_back(IBKMK::RadToDeg(lat2));
			}
			else {
				coordinate2.push_back(x2);
				coordinate2.push_back(y2);
			}

			coordinates.push_back(coordinate1);
			coordinates.push_back(coordinate2);

			geometry["coordinates"] = coordinates;
			feature["geometry"] = geometry;
			features.push_back(feature);
		}
	}
	// export all nodes of the network with type substation
	if(m_ui->checkBoxExportSubStation->isChecked()){
		for(const VICUS::NetworkNode & n : network->m_nodes){
			if(n.m_type != VICUS::NetworkNode::NT_SubStation) {
				continue;
			}
			QJsonObject feature;
			feature["type"] = "Feature";

			QJsonObject properties;
			properties["id"] = featureId++;
			properties["Name"] = n.m_displayName;
			properties["HeatingDemand"] = n.m_maxHeatingDemand.value;

			feature["properties"] = properties;

			QJsonObject geometry;
			geometry["type"] = "Point";

			QJsonArray coordinates;
			//convert coordinates to lat-lon format
			double lat, lon;
			double x = n.m_position.m_x + network->m_origin.m_x;
			double y = n.m_position.m_y + network->m_origin.m_y;
			if (m_ui->groupBoxToUTM->isChecked()) {
				int utmZone = m_ui->comboBoxUTMZone->currentIndex();
				bool isSouthern = m_ui->radioButtonSouthern->isChecked();
				IBKMK::UTMXYToLatLon (x, y, utmZone, isSouthern, lat, lon);
				coordinates.push_back(IBKMK::RadToDeg(lon));
				coordinates.push_back(IBKMK::RadToDeg(lat));
			}
			else {
				coordinates.push_back(x);
				coordinates.push_back(y);
			}

			geometry["coordinates"] = coordinates;
			feature["geometry"] = geometry;
			features.push_back(feature);
		}
	}
	jsonFile["features"] = features;

	// write to a file
	QByteArray ba = QJsonDocument(jsonFile).toJson(QJsonDocument::Compact);

	IBK::Path exportFile(m_ui->lineEditExportFileName->filename().toStdString());

	std::ofstream geoJsonFile;

	if ( !IBK::open_ofstream(geoJsonFile, exportFile) )
		throw IBK::Exception(IBK::FormatString("Could not open output file '%1'\n").arg(exportFile), FUNC_ID);

	geoJsonFile << ba.toStdString();

	geoJsonFile.close();

	// show message that file was created
	QMessageBox::information(this, "File created", tr("Exported file saved to") + QString(":\n '%1'").arg(exportFile.c_str()));
}


void SVNetworkExportDialog::on_checkBoxExportPipeline_toggled(/*bool checked*/) {
	updateOkButtonEnableState();
}


void SVNetworkExportDialog::on_checkBoxExportSubStation_toggled(/*bool checked*/) {
	updateOkButtonEnableState();
}


void SVNetworkExportDialog::updateOkButtonEnableState(){
	//check if a network is selected
	if(!m_ui->comboBoxNetworkSelection->currentText().isEmpty())
		//check if a file is selected
		if(!m_ui->lineEditExportFileName->filename().isEmpty()){
			//check if at least one checkbox is checked
			if(m_ui->checkBoxExportPipeline->isChecked() || m_ui->checkBoxExportSubStation->isChecked()){
				m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
				return;
			}
		}
	m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
}


void SVNetworkExportDialog::on_lineEditExportFileName_editingFinished() {
	if(!m_ui->lineEditExportFileName->filename().endsWith(".geojson")){
		m_ui->lineEditExportFileName->setFilename(m_ui->lineEditExportFileName->filename() + ".geojson");
	}
	updateOkButtonEnableState();
}


void SVNetworkExportDialog::on_buttonBox_accepted() {
	unsigned int id = m_ui->comboBoxNetworkSelection->currentData().toUInt();
	exportToGeoJson(id);
}

