#include "SVNetworkImportDialog.h"
#include "ui_SVNetworkImportDialog.h"

#include <IBK_Path.h>

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <VICUS_Network.h>
#include <VICUS_NetworkLine.h>

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

	// normalize all nodes
	double x = QLocale().toDouble(m_ui->lineEditXOrigin->text());
	double y = QLocale().toDouble(m_ui->lineEditYOrigin->text());

	for (VICUS::NetworkNode & node : n.m_nodes) {
		node.m_x -= x;
		node.m_y -= y;
	}

	n.m_name = m_ui->lineEditNetworkName->text().toStdString();

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
		n.updateNodeEdgeConnectionPointers();

		m_ui->labelEdgeCount->setText(QString("%1").arg(n.m_edges.size()));
		double minX = std::numeric_limits<double>::max();
		double maxX = -std::numeric_limits<double>::max();
		double minY = std::numeric_limits<double>::max();
		double maxY = -std::numeric_limits<double>::max();
		// now process all nodes
		for (const VICUS::NetworkNode & node : n.m_nodes) {
			minX = qMin(minX, node.m_x);
			maxX = qMax(maxX, node.m_x);
			minY = qMin(minY, node.m_y);
			maxY = qMax(maxY, node.m_y);
		}
		m_ui->labelCoordinateRange->setText( QString("[%L1,%L2]...[%L3,%L4]").arg(minX).arg(minY).arg(maxX).arg(maxY));
		m_ui->lineEditXOrigin->setText( QString("%L1").arg(0.5*(minX + maxX)));
		m_ui->lineEditYOrigin->setText( QString("%L1").arg(0.5*(minY + maxY)));

		// copy network over to data structure
		*m_network = n;
	}
	catch (IBK::Exception & ex) {
		QMessageBox::critical(this, QString(), tr("Error reading GIS data file:\n%1").arg(ex.what()));
		return;
	}

}
