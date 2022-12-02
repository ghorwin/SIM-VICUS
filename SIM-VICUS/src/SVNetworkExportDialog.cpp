#include <QFileDialog>

#include <VICUS_Project.h>
#include <VICUS_utilities.h>
#include <QJsonDocument>
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
}

SVNetworkExportDialog::~SVNetworkExportDialog()
{
	delete m_ui;
}

bool SVNetworkExportDialog::edit() {

	// store pointer to network for dialog functions to access
	const VICUS::Project &p = project();


	// update existing networks combobox
	m_ui->comboBoxNetworkSelection->clear();
	if (!p.m_geometricNetworks.empty()){
		m_existingNetworksMap.clear();
		for (auto it = p.m_geometricNetworks.begin(); it!=p.m_geometricNetworks.end(); ++it)
			m_existingNetworksMap.insert(it->m_displayName, it->m_id);
		m_ui->comboBoxNetworkSelection->addItems(QStringList(m_existingNetworksMap.keys()));
		m_ui->pushButtonExport->setEnabled(true);
	} else {
		m_ui->pushButtonExport->setEnabled(false);
	}

	return exec();
}

void SVNetworkExportDialog::on_pushButtonExport_clicked() {
	unsigned int id = m_existingNetworksMap.value(m_ui->comboBoxNetworkSelection->currentText());
	VICUS::Network network = *VICUS::element(project().m_geometricNetworks, id);
	// how to determine if it is a building or a grid?
	IBK::Path exportFile(m_ui->lineEditExportFileName->text().toStdString());
	QJsonObject root = network.exportGridToGeoJson(SVSettings::instance().m_db.m_pipes);
	QByteArray ba = QJsonDocument(root).toJson();

	std::ofstream geoJsonFile(exportFile.str());

	if ( !geoJsonFile.is_open() )
		throw IBK::Exception(IBK::FormatString("Could not open output file '%1'\n").arg(exportFile.str() ), "SVNetworkExportDialog::on_pushButtonExport_clicked");

	geoJsonFile << ba.toStdString();

	geoJsonFile.close();

	SVSettings::instance().showDoNotShowAgainMessage(this, "export-file-created", QString(tr("Export file was created")), QString(tr("File was saved at: ") + "%1" ).arg(exportFile.c_str()));

}


void SVNetworkExportDialog::on_pushButtonSelectFileLocation_clicked() {
	QString fname = QFileDialog::getSaveFileName(this, tr("Select file name and location"),
												 "../../data/vicus/GeometryTests/Network", tr("GeoJson-Files (*.geojson)"), nullptr,
												 SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
												 );

	if(!fname.endsWith(".geojson"))
			fname += ".geojson";
	m_ui->lineEditExportFileName->setText(fname);
}

