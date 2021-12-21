#include "SVCoSimCO2VentilationDialog.h"
#include "ui_SVCoSimCO2VentilationDialog.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVPreferencesDialog.h"

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
	m_ui->plainTextEdit->setEnabled(false);
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

	QString errorColor = SVSettings::instance().m_theme == SVSettings::TT_Dark ? "red" : "darkRed";
	QString successColor = SVSettings::instance().m_theme == SVSettings::TT_Dark ? "lime" : "darkGreen";

	bool success = true;
	// TODO : check for sufficient project parameters
	//		  in case of missing data, set appropriate error text in m_ui->labelCheckSummary and return false

	if (SVProjectHandler::instance().projectFile().isEmpty()) {
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Please save project first!</span>").arg(errorColor));
		return false;
	}
	// check for correct extension
	if (!SVProjectHandler::instance().projectFile().toLower().endsWith(".vicus")) {
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Expected project file name to end with '.vicus'. Please save project with correct file name!</span>").arg(errorColor));
		return false;
	}



	if (success)
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Project data ok for CO2 balance and ventilation control model.</span>").arg(successColor));

	return success;
}



void SVCoSimCO2VentilationDialog::on_pushButtonGenerate_clicked() {
	// Note: content checks have been made already in checkProjectData()
	m_ui->plainTextEdit->setEnabled(true);

	// generate file paths
	QString vicusProjectFile = SVProjectHandler::instance().projectFile();  //  -> "/path/to/project.vicus"

	// get overall base dir
	QString baseDir = vicusProjectFile.left(vicusProjectFile.length()-6);  //  -> "/path/to/project"

	m_co2FMUBaseDir = baseDir + ".fmus/CO2Balance";
	m_co2FMUFilePath = baseDir + ".fmus/CO2Balance.fmu";
	m_nandradFMUFilePath = baseDir + ".fmus/NANDRAD.fmu";
	m_msimProjectFilePath = baseDir + ".msim";


	// TODO : compose tsv files and CO2Balance-Project file

	// Beispielausgabe
	QString tsvFile = "blubb.tsv";
	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg("CO2Balance/" + tsvFile));


	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg(m_co2FMUFilePath));

	// NANDRAD FMU

	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg(m_nandradFMUFilePath));


	// MSIM-Project
	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg(m_msimProjectFilePath));


	// all successful, enable "Launch MasterSim" button

	m_ui->pushButtonLaunchMasterSim->setEnabled(true);
}


void SVCoSimCO2VentilationDialog::on_pushButtonLaunchMasterSim_clicked() {
	// check if we have MasterSim path configured
	QString masterSimPath = SVSettings::instance().m_masterSimExecutable;
	if (masterSimPath.isEmpty() || !QFileInfo::exists(masterSimPath))
	{
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to "
																  "MASTERSIM in the preferences dialog!"));
		SVMainWindow::instance().preferencesDialog()->edit(0);
		// TODO Andreas, find a way to show a mode-less window within a dialog's event loop
		masterSimPath = SVSettings::instance().m_masterSimExecutable;
		// still no valid path?
		if (masterSimPath.isEmpty() || !QFileInfo::exists(masterSimPath))
			return;
	}

	// launch MasterSim
	bool res = QProcess::startDetached(masterSimPath, QStringList() << m_msimProjectFilePath, QString());
	if (!res) {
		QMessageBox::critical(this, tr("Error starting external application"), tr("MASTERSIM '%1' could not be started.")
							  .arg(masterSimPath));
	}
}
