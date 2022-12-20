#include "SVSimulationLCAResultsDialog.h"
#include "ui_SVSimulationLCAResultsDialog.h"

SVSimulationLCAResultsDialog::SVSimulationLCAResultsDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationLCAResultsDialog)
{
	m_ui->setupUi(this);

	setup();
}

SVSimulationLCAResultsDialog::~SVSimulationLCAResultsDialog() {
	delete m_ui;
}

void SVSimulationLCAResultsDialog::setLcaResults() {
	//m_ui->treeWidgetLcaResults->
}

void SVSimulationLCAResultsDialog::setup() {
	// Add data to treeWidget
	m_ui->treeWidgetLcaResults->setColumnCount(15);
	QStringList headers;
	headers << "Category" << "Component type" << "Component name" << "Area [m2]" << "GWP (CO2-Äqu.) [kg/(m2a)";
	headers << "ODP (R11-Äqu.) [kg/(m2a)]" << "POCP (C2H4-Äqu.) [kg/(m2a)]" << "AP (SO2-Äqu.) [kg/(m2a)]" << "EP (PO4-Äqu.) [kg/(m2a)]";

	m_ui->treeWidgetLcaResults->setHeaderLabels(headers);

}
