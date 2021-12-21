#include "SVCoSimCO2VentilationDialog.h"
#include "ui_SVCoSimCO2VentilationDialog.h"

#include <SVProjectHandler.h>
#include <VICUS_Project.h>

SVCoSimCO2VentilationDialog::SVCoSimCO2VentilationDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVCoSimCO2VentilationDialog)
{
	m_ui->setupUi(this);
}


SVCoSimCO2VentilationDialog::~SVCoSimCO2VentilationDialog() {
	delete m_ui;
}


int SVCoSimCO2VentilationDialog::exec() {
	m_ui->plainTextEdit->clear();
	m_ui->pushButtonLaunchMasterSim->setEnabled(false);
	// check VICUS project for sufficient data input for CO2 ventilation and update button states
	if (!checkProjectData()) {
		// disable buttons
		m_ui->pushButtonGenerate->setEnabled(false);
		return QDialog::exec();
	}
	// enable generate button
	m_ui->pushButtonGenerate->setEnabled(true);
	return QDialog::exec();
}


bool SVCoSimCO2VentilationDialog::checkProjectData() const {
	const VICUS::Project & p = project();

	bool success = true;
	// TODO : check for sufficient project parameters
	//		  in case of missing data, set appropriate error text in m_ui->labelCheckSummary and return false


	if (success)
		m_ui->labelCheckSummary->setText(tr("Project data ok for CO2 balance and ventilation control model."));

	return success;
}
