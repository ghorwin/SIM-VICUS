#include "SVSimulationLCAResultsDialog.h"
#include "ui_SVSimulationLCAResultsDialog.h"

SVSimulationLCAResultsDialog::SVSimulationLCAResultsDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationLCAResultsDialog)
{
	m_ui->setupUi(this);
}

SVSimulationLCAResultsDialog::~SVSimulationLCAResultsDialog() {
	delete m_ui;
}

void SVSimulationLCAResultsDialog::setLcaResults() {
	//	m_ui->treeWidgetLcaResults->
}
