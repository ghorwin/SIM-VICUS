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

#include "SVSimulationStartNandrad.h"
#include "ui_SVSimulationStartNandrad.h"

#include <QHBoxLayout>
#include <QMessageBox>


#include <VICUS_Project.h>
#include <NANDRAD_Project.h>

#include <QtExt_Directories.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVSimulationPerformanceOptions.h"
#include "SVSimulationLocationOptions.h"
#include "SVSimulationOutputOptions.h"
#include "SVSimulationModelOptions.h"
#include "SVSimulationShadingOptions.h"
#include "SVSimulationRunRequestDialog.h"
#include "SVSimulationNetworkOptions.h"
#include "SVConstants.h"
#include "SVLogFileDialog.h"
#include "SVUndoModifyProject.h"

SVSimulationStartNandrad::SVSimulationStartNandrad(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationStartNandrad)
{
	m_ui->setupUi(this);

	m_ui->comboBoxVerboseLevel->addItem( tr("Minimum"), 0 );
	m_ui->comboBoxVerboseLevel->addItem( tr("Normal"), 1 );
	m_ui->comboBoxVerboseLevel->addItem( tr("Detailed"), 2 );
	m_ui->comboBoxVerboseLevel->addItem( tr("Very Detailed"), 3 );

	m_ui->lineEditDuration->setup(m_ui->comboBoxUnitDuration, IBK::Unit("s"),
								  0, std::numeric_limits<double>::max(), tr("Duration of the simulation.") );

	m_ui->lineEditNumThreads->setup(1,64,tr("Number of parallel threads, should be less or equal to the number of physical CPU cores."), true, true);
	m_ui->lineEditNumThreads->setAcceptOnlyInteger(true);
	m_ui->lineEditNumThreads->setEmptyAllowed(true, tr("auto (using OMP_NUM_THREADS if set)","as in automatic"));
	bool ok;
	int numThreads = SVSettings::instance().m_propertyMap[SVSettings::PT_NumParallelThreads].toInt(&ok);
	if (ok) {
		if (numThreads == 0)
			m_ui->lineEditNumThreads->setText(""); // 0 = auto (empty input field)
		else
			m_ui->lineEditNumThreads->setValue(numThreads);
	}


	// for now set the defaults states hard-coded, later this should be read from stored settings
	m_ui->comboBoxVerboseLevel->setCurrentIndex(1);
#ifdef _WIN32
	m_ui->checkBoxCloseConsoleWindow->setChecked(true);
	m_ui->labelTerminalEmulator->setVisible(false);
	m_ui->comboBoxTermEmulator->setVisible(false);
#elif defined(Q_OS_LINUX)
	m_ui->checkBoxCloseConsoleWindow->setVisible(false);
	m_ui->comboBoxTermEmulator->blockSignals(true);
	m_ui->comboBoxTermEmulator->setCurrentIndex(SVSettings::instance().m_terminalEmulator);
	m_ui->comboBoxTermEmulator->blockSignals(false);
#else
	// mac has neither option
	m_ui->checkBoxCloseConsoleWindow->setVisible(false);
	m_ui->labelTerminalEmulator->setVisible(false);
	m_ui->comboBoxTermEmulator->setVisible(false);
#endif

	{
		m_simulationPerformanceOptions = new SVSimulationPerformanceOptions(this, m_localProject.m_solverParameter);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationPerformanceOptions);
		m_ui->tabPerformanceOptions->setLayout(h);
	}
	{
		m_simulationLocationOptions = new SVSimulationLocationOptions(this, m_localProject.m_location);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationLocationOptions);
		m_ui->tabClimate->setLayout(h);
	}
	{
		m_simulationOutputOptions = new SVSimulationOutputOptions(this, m_localProject.m_outputs, this);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationOutputOptions);
		m_ui->tabOutputs->setLayout(h);
	}
	{
		m_simulationModelOptions = new SVSimulationModelOptions(this, m_localProject.m_simulationParameter, m_localProject.m_location);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationModelOptions);
		m_ui->tabSimOptions->setLayout(h);
	}
	{
		m_simulationShadingOptions = new SVSimulationShadingOptions(this, m_localProject.m_simulationParameter, m_localProject.m_location);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationShadingOptions);
		m_ui->tabShadingCalculation->setLayout(h);
	}
	{
		m_simulationNetworkOptions = new SVSimulationNetworkOptions(this, m_localProject.m_geometricNetworks);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationNetworkOptions);
		m_ui->tabNetworkSettings->setLayout(h);
	}

	// start with initial page, always
	m_ui->tabWidget->setCurrentIndex(0);
}


SVSimulationStartNandrad::~SVSimulationStartNandrad() {
	delete m_ui;
}


int SVSimulationStartNandrad::edit(bool fmiExport) {

	m_solverExecutable = SVSettings::nandradSolverExecutable();

	// cache NANDRAD project file path
	m_nandradProjectFilePath = SVProjectHandler::instance().nandradProjectFilePath();

	// create a copy of the current project
	m_localProject = project(); // Mind: addresses to member variables m_solverParameters, m_location etc. do not change here!
	m_localProject.updatePointers();

	// initialize defaults
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvement
	if (simParas == NANDRAD::SimulationParameter()) {
		simParas.m_solarLoadsDistributionModel.m_distributionType = NANDRAD::SolarLoadsDistributionModel::SWR_AreaWeighted;
		NANDRAD::KeywordList::setParameter(m_localProject.m_simulationParameter.m_solarLoadsDistributionModel.m_para,
										   "SolarLoadsDistributionModel::para_t",
										   NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone, 50);
	}
	// initialize simulation parameters with meaningful defaults and fix possibly wrong values
	// in project (wherever they come from)
	if (simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].name.empty() ||
		simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value < 0)
	{
		simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", 2019);
	}

	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].name.empty() ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value < 0 ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value > 365*24*3600)
	{
		simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].set("Start", 0, IBK::Unit("d"));
	}

	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_End].name.empty() ||
		simParas.m_interval.m_para[ NANDRAD::Interval::P_End].value < 0)
	{
		simParas.m_interval.m_para[ NANDRAD::Interval::P_End].set("End", 1, IBK::Unit("a"));
	}
	if (simParas.m_para[NANDRAD::SimulationParameter::P_InitialTemperature].name.empty() ||
		simParas.m_para[NANDRAD::SimulationParameter::P_InitialTemperature].IO_unit.base_id() != IBK_UNIT_ID_SECONDS)
	{
		NANDRAD::KeywordList::setParameter(simParas.m_para,
										   "SimulationParameter::para_t",
										   NANDRAD::SimulationParameter::P_InitialTemperature, 20);
	}

	// create default output settings, if nothing has been defined, yet
	VICUS::Outputs & outputs = m_localProject.m_outputs; // readability improvement
	if (outputs == VICUS::Outputs()) {
		outputs.m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].set("CreateDefaultZoneOutputs", true);
		NANDRAD::OutputGrid og;
		og.m_name = tr("Hourly values").toStdString();
		NANDRAD::Interval iv;
		NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_Start, 0);
//		NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_End, 0);
		NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_StepSize, 1);
		og.m_intervals.push_back(iv);
		outputs.m_grids.push_back(og);
	}

	m_simulationPerformanceOptions->updateUi();
	m_simulationLocationOptions->updateUi();
	m_simulationOutputOptions->updateUi();
	m_simulationModelOptions->updateUi();
	m_simulationShadingOptions->updateUi();
	m_simulationNetworkOptions->updateUi();

	updateTimeFrameEdits();
	updateCmdLine();

	// do we have any selected surfaces?
	std::vector<const VICUS::Surface*> sel;
	project().selectedSurfaces(sel, VICUS::Project::SG_Building);
	if (sel.empty()) {
		m_ui->radioButtonEntireProject->setEnabled(false);
		m_ui->radioButtonSelectedGeometry->setEnabled(false);
		m_ui->radioButtonEntireProject->setChecked(true);
	}
	else {
		m_ui->radioButtonEntireProject->setEnabled(true);
		m_ui->radioButtonSelectedGeometry->setEnabled(true);
		m_ui->radioButtonSelectedGeometry->setChecked(true);
	}


	if (fmiExport) {
		SVSettings::instance().showDoNotShowAgainMessage(this, "FMUExportInfoMessage", tr("FMU Export"),
			tr("Adjust the simulation parameters in this dialog as needed, test-initialize the simulation and "
			   "then start the FMU generation process by pressing 'Export FMU...'")
			);
	}

	return exec();
	// if dialog was confirmed, data is transfered into project
}


void SVSimulationStartNandrad::on_pushButtonClose_clicked() {
	accept();
}


void SVSimulationStartNandrad::on_pushButtonRun_clicked() {
	if (!startSimulation(false))
		return; // keep dialog open

	accept(); // close dialog and store data
}


void SVSimulationStartNandrad::on_checkBoxCloseConsoleWindow_toggled(bool /*checked*/) {
	updateCmdLine();
}


void SVSimulationStartNandrad::on_checkBoxStepStats_toggled(bool /*checked*/) {
	updateCmdLine();
}


void SVSimulationStartNandrad::on_pushButtonShowScreenLog_clicked() {
	// compose path to log file
	// compose log file name
	QString logfile = QFileInfo(m_nandradProjectFilePath).completeBaseName() + "/log/screenlog.txt";
	logfile = QFileInfo(m_nandradProjectFilePath).dir().absoluteFilePath(logfile);
	SVLogFileDialog dlg(this);
	dlg.setLogFile(logfile, m_nandradProjectFilePath, false);
	dlg.exec();
}


void SVSimulationStartNandrad::on_lineEditStartDate_editingFinished() {
	IBK::Time startTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditStartDate->text().toStdString());

	// update date time
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", startTime.year());
	simParas.m_interval.m_para[NANDRAD::Interval::P_Start].set("Start", startTime.secondsOfYear(), IBK::Unit("s"));
	updateTimeFrameEdits();
}


void SVSimulationStartNandrad::on_lineEditEndDate_editingFinished() {
	IBK::Time endTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditEndDate->text().toStdString());

	// compose start time (startYear and offset are given and well defined, we ensure that)
	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = simParas.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// compute difference between dates
	IBK::Time diff = endTime - startTime; // Might be negative!
	if (!diff.isValid()) {
		m_ui->lineEditDuration->setValue(0); // set zero duration to indicate that something is wrong!
		return;
	}

	// end date is the offset from start, so we first need the start date
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].set("End", diff.secondsOfYear(), IBK::Unit("s"));
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = m_ui->lineEditDuration->currentUnit();

	updateTimeFrameEdits();
}


void SVSimulationStartNandrad::on_lineEditDuration_editingFinishedSuccessfully() {
	// we always update the end time and let the end time signal do the undo action stuff
	IBK::Parameter durPara = m_ui->lineEditDuration->toParameter("Duration");
	if (durPara.name.empty())
		return; // invalid input in parameter edit

	if (durPara.value <= 0)
		return; // invalid input in parameter edit

	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = simParas.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// add duration
	startTime += durPara.value;
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].set("End", startTime.secondsOfYear(), IBK::Unit("s"));
	// set duration unit in parameter - this will be used to select matching unit in combo box
	simParas.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = durPara.IO_unit;
	updateTimeFrameEdits();
}


void SVSimulationStartNandrad::updateCmdLine() {
	m_cmdArgs.clear();

	if (m_ui->checkBoxStepStats->isChecked())
		m_cmdArgs.push_back("--step-stats");
	if (m_ui->checkBoxCloseConsoleWindow->isChecked())
		m_cmdArgs.push_back("-x");

	m_ui->lineEditCmdLine->setText("\"" + m_solverExecutable + "\" " + m_cmdArgs.join(" ") + "\"" + m_nandradProjectFilePath + "\"");
	m_ui->lineEditCmdLine->setCursorPosition( m_ui->lineEditCmdLine->text().length() );
}




void SVSimulationStartNandrad::updateTimeFrameEdits() {

	m_ui->lineEditStartDate->blockSignals(true);
	m_ui->lineEditEndDate->blockSignals(true);
	m_ui->lineEditDuration->blockSignals(true);

	// Note: we can be sure that all the parameters are set, though possibly to invalid values

	NANDRAD::SimulationParameter & simParas = m_localProject.m_simulationParameter; // readability improvements
	int startYear = simParas.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	// fall-back to zero, if not specified
	double startOffset = simParas.m_interval.m_para[ NANDRAD::Interval::P_Start].value;

	IBK::Time t(startYear, startOffset);
	m_ui->lineEditStartDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	double endTime = simParas.m_interval.m_para[ NANDRAD::Interval::P_End].value;
	if (simParas.m_interval.m_para[ NANDRAD::Interval::P_End].name.empty())
		endTime = startOffset + 365*24*3600; // fallback to 1 year
	double simDuration = endTime - startOffset;
	t += simDuration;

	m_ui->lineEditEndDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	IBK::Parameter durationPara;
	// use unit from end
	durationPara = IBK::Parameter("Duration", 0, simParas.m_interval.m_para[ NANDRAD::Interval::P_End].IO_unit);
	durationPara.value = simDuration; // set value in seconds
	m_ui->lineEditDuration->setFromParameter(durationPara);

	m_ui->lineEditStartDate->blockSignals(false);
	m_ui->lineEditEndDate->blockSignals(false);
	m_ui->lineEditDuration->blockSignals(false);
}


bool SVSimulationStartNandrad::startSimulation(bool testInit, bool forceBackgroundProcess) {
	updateCmdLine();
	QString resultPath;
	if (!generateNANDRAD(resultPath, !testInit))
		return false;
	IBK::Path resultDir(resultPath.toStdString());

	bool cleanDir = false;
	QStringList commandLineArgs = m_cmdArgs;
	if (testInit) {
		commandLineArgs.append("--test-init");
	}
	else {
		SVSimulationRunRequestDialog::SimulationStartType startType = SVSimulationRunRequestDialog::Normal;
		// check if result directory exists and if yes, ask user about overwriting
		if (resultDir.exists()) {
			if (!resultDir.isDirectory()) {
				QMessageBox::critical(this, tr("Solver error"),
									  tr("There is already a file with the name of the output "
										 "directory to be created '%1'. Please remove this file "
										 "or save the project with a new name!").arg(resultPath));
				return false;
			}
			// ask user for confirmation
			if (m_simulationRunRequestDialog == nullptr)
				m_simulationRunRequestDialog = new SVSimulationRunRequestDialog(this);
			startType = m_simulationRunRequestDialog->askForOption();
			// if user aborted dialog, do nothing
			if (startType == SVSimulationRunRequestDialog::DoNotRun)
				return false;
			// only clean directory when user selected normal
			if (startType == SVSimulationRunRequestDialog::Normal)
				cleanDir = true;
		}

		// add command line option if needed
		if (startType == SVSimulationRunRequestDialog::Continue)
			commandLineArgs.append("--restart");
	}
	// clean result directory if requested
	if (cleanDir) {
		// We only delete a subdirectory with correct subdirectory structure. This
		// generally prevents accidental deleting of directories.
		IBK::Path resFolder = resultDir / "results";
		IBK::Path logFolder = resultDir / "log";
		if (resFolder.exists() && logFolder.exists()) {
			if (!IBK::Path::remove(resultDir)) {
				QMessageBox::critical(this, tr("Solver error"),
									  tr("Cannot remove result directory '%1', maybe files are still being used?").arg(resultPath) );
				return false;
			}
		}
	}

	// delete working directory if requested
	// launch solver - run option is only needed for linux, and otherwise it will always be -1
#if defined(Q_OS_LINUX)
	SVSettings::TerminalEmulators runOption = (SVSettings::TerminalEmulators)m_ui->comboBoxTermEmulator->currentIndex();
#else
	SVSettings::TerminalEmulators runOption = (SVSettings::TerminalEmulators)-1;
#endif
	// if background process is forced, ignore terminal settings
	if (forceBackgroundProcess)
		runOption = SVSettings::TE_None;
	bool success = SVSettings::startProcess(m_solverExecutable, commandLineArgs, m_nandradProjectFilePath, runOption);
	if (!success) {
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(m_solverExecutable));
		return false;
	}

	// all ok, solver is running
	return true;
}


bool SVSimulationStartNandrad::generateNANDRAD(QString & resultPath, bool generateOutputs) {
	// compose NANDRAD project file and start simulation

	// generate NANDRAD project
	NANDRAD::Project p;

	QStringList errorStack;

	SVSettings::instance().m_db.updateEmbeddedDatabase(m_localProject);
	try {
		// set placeholders in NANDRAD Project (VICUS-Project does not have placeholders)
		p.m_placeholders[VICUS::DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::databasesDir()).toStdString());
		p.m_placeholders[VICUS::USER_DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::userDataDir()).toStdString());
		// "Project Directory" placeholder is needed to resolve paths to files referenced via relative paths
		p.m_placeholders["Project Directory"] = IBK::Path(m_nandradProjectFilePath.toStdString()).parentPath().str();

		m_localProject.generateNandradProject(p, errorStack, m_nandradProjectFilePath.toStdString());

	}
	catch (IBK::Exception & ex) {
		// just show a generic error message
		ex.writeMsgStackToError();
		QMessageBox box(this);
		QString fullText = errorStack.join("\n");
		box.setDetailedText(fullText);
		box.setIcon(QMessageBox::Critical);
		box.setText(tr("An error occurred during NANDRAD project generation (see details below)."));
		box.setWindowTitle(tr("NANDRAD Project Generation Error"));
		box.exec();
		return false;
	}

	// save project
	p.writeXML(IBK::Path(m_nandradProjectFilePath.toStdString()));
	/// TODO : check if project file was correctly written

	resultPath = QFileInfo(SVProjectHandler::instance().projectFile()).completeBaseName();
	resultPath = QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(resultPath);
	return true;
}

void SVSimulationStartNandrad::on_comboBoxTermEmulator_currentIndexChanged(int index) {
	SVSettings::instance().m_terminalEmulator = (SVSettings::TerminalEmulators)(index);
}


void SVSimulationStartNandrad::on_pushButtonTestInit_clicked() {
	if (startSimulation(true) )
		m_simulationOutputOptions->updateUi();
}


void SVSimulationStartNandrad::on_pushButtonExportFMU_clicked() {
	QString resultPath;
	if (!generateNANDRAD(resultPath)) {
		return;
	}
	// launch external FMU generator tool
	QStringList cmdArgs;

	QString generatorExecutable = SVSettings::nandradFMUGeneratorExecutable();
	bool success = SVSettings::startProcess(generatorExecutable, cmdArgs, m_nandradProjectFilePath, SVSettings::TE_None);
	if (!success) {
		QMessageBox::critical(this, QString(), tr("Could not run FMU Generator '%1'").arg(generatorExecutable));
		return;
	}
}
