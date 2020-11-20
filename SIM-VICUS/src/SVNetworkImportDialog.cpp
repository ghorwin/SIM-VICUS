#include "SVNetworkImportDialog.h"
#include "ui_SVNetworkImportDialog.h"

#include <IBK_Path.h>

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <VICUS_Network.h>

SVNetworkImportDialog::SVNetworkImportDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVNetworkImportDialog)
{
	m_ui->setupUi(this);
}


SVNetworkImportDialog::~SVNetworkImportDialog() {
	delete m_ui;
}


bool SVNetworkImportDialog::edit(VICUS::Network & n) {
	// store pointer to network for dialog functions to access
	m_network = &n;

	if (exec() != QDialog::Accepted)
		return false;

	// store data from user interface elements
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
		VICUS::Network n;
		n.readGridFromCSV(networkFile);
		*m_network = n;
		m_ui->labelEdgeCount->setText(QString("%1").arg(n.m_edges.size()));
		double minX = std::numeric_limits<double>::max();
		double maxX = -std::numeric_limits<double>::max();
		// now process all edges

	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical(this, QString(), tr("Error reading GIS data file:\n%1").arg(ex.what()));
		return;
	}

}
