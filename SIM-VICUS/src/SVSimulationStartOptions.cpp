#include "SVSimulationStartOptions.h"
#include "ui_SVSimulationStartOptions.h"

#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVUndoModifySolverParameter.h"
#include "SVLogFileDialog.h"
#include "SVMainWindow.h"
#include "SVSimulationRunRequestDialog.h"
#include "SVSimulationOutputOptions.h"
#include "SVView3DCalculation.h"

#include <QProcess>
#include <QProgressDialog>
#include <QTextEdit>

#include <NANDRAD_Project.h>

#include <QtExt_Directories.h>


SVSimulationStartOptions::SVSimulationStartOptions(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationStartOptions)
{
	m_ui->setupUi(this);
	layout()->setContentsMargins(0,0,0,0);

	connect(m_ui->lineEditDuration, &QtExt::ParameterEdit::editingFinishedSuccessfully,
			this, &SVSimulationStartOptions::on_lineEditDuration_editingFinishedSuccessfully);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVSimulationStartOptions::onModified);

	m_ui->comboBoxVerboseLevel->blockSignals(true);
	m_ui->comboBoxVerboseLevel->addItem( tr("Minimum"), 0 );
	m_ui->comboBoxVerboseLevel->addItem( tr("Normal"), 1 );
	m_ui->comboBoxVerboseLevel->addItem( tr("Detailed"), 2 );
	m_ui->comboBoxVerboseLevel->addItem( tr("Very Detailed"), 3 );
	m_ui->comboBoxVerboseLevel->setCurrentIndex(1);
	m_ui->comboBoxVerboseLevel->blockSignals(false);

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

}

SVSimulationStartOptions::~SVSimulationStartOptions()
{
	delete m_ui;
}


void SVSimulationStartOptions::onModified(int modificationType, ModificationInfo * /*data*/) {
	SVProjectHandler::ModificationTypes modType = (SVProjectHandler::ModificationTypes)modificationType;
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::SolverParametersModified:
			updateUi();
		break;
		default:;
	}
}

void SVSimulationStartOptions::updateUi() {
	m_ui->radioButtonSelectedGeometry->setEnabled(!project().m_buildings.empty());
	updateTimeFrameEdits();
	updateCmdLine();
}

void SVSimulationStartOptions::updateCmdLine() {
	m_commandLineArgs.clear();

	if (m_ui->checkBoxStepStats->isChecked())
		m_commandLineArgs.push_back("--step-stats");
	if (m_ui->checkBoxCloseConsoleWindow->isChecked())
		m_commandLineArgs.push_back("-x");
	double numThreads = m_ui->lineEditNumThreads->value();
	if (m_ui->lineEditNumThreads->isValidNumber(numThreads))
		m_commandLineArgs.push_back(QString("-p=%1").arg(numThreads));
	unsigned int vLevel = m_ui->comboBoxVerboseLevel->currentData().toUInt();
	if (vLevel!=1)
		m_commandLineArgs.push_back(QString("--verbosity-level=%1").arg(vLevel));

	m_ui->lineEditCmdLine->setText("\"" + SVSettings::nandradSolverExecutable() + "\" " + m_commandLineArgs.join(" ") + " \"" + SVProjectHandler::instance().nandradProjectFilePath() + "\"");
	m_ui->lineEditCmdLine->setCursorPosition( m_ui->lineEditCmdLine->text().length() );
}


void SVSimulationStartOptions::updateTimeFrameEdits() {

	const VICUS::Project &p = project();

	// Note: we can be sure that all the parameters are set, though possibly to invalid values
	int startYear = p.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	// fall-back to zero, if not specified
	double startOffset = p.m_simulationParameter.m_interval.m_para[ NANDRAD::Interval::P_Start].value;

	IBK::Time t(startYear, startOffset);
	m_ui->lineEditStartDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	double endTime = p.m_simulationParameter.m_interval.m_para[ NANDRAD::Interval::P_End].value; // always valid, see edit()
	double simDuration = endTime - startOffset;
	// Note: if user had manipulated project file, duration may be negative. But we just show it anyway - solver will complain!
	t += simDuration;

	m_ui->lineEditEndDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	IBK::Parameter durationPara;
	// use unit from end
	durationPara = IBK::Parameter("Duration", 0, p.m_simulationParameter.m_interval.m_para[ NANDRAD::Interval::P_End].IO_unit);
	durationPara.value = simDuration; // set value in seconds
	m_ui->lineEditDuration->setFromParameter(durationPara);

	m_ui->lineEditStartDate->blockSignals(false);
	m_ui->lineEditEndDate->blockSignals(false);
	m_ui->lineEditDuration->blockSignals(false);
}


void SVSimulationStartOptions::on_checkBoxCloseConsoleWindow_clicked() {
	updateCmdLine();
}


void SVSimulationStartOptions::on_checkBoxStepStats_clicked() {
	updateCmdLine();
}


void SVSimulationStartOptions::on_lineEditNumThreads_editingFinishedSuccessfully() {
	updateCmdLine();
}

void SVSimulationStartOptions::on_comboBoxVerboseLevel_currentIndexChanged(int /*index*/) {
	updateCmdLine();
}


void SVSimulationStartOptions::on_lineEditStartDate_editingFinished() {

	VICUS::Project p = project();

	IBK::Time startTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditStartDate->text().toStdString());

	// store current end time
	IBK::Time endTime(p.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value, p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].value);

	// update date time
	p.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", startTime.year());
	p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].set("Start", startTime.secondsOfYear(), IBK::Unit("s"));

	// check if end date is preceeding start date, and if so, fix it by adding duration to start
	// Mind: start year of startTime might be different from start year of end time
	if (startTime >= endTime) {
		// take duration from input
		// we always update the end time and let the end time signal do the undo action stuff
		IBK::Parameter durPara = m_ui->lineEditDuration->toParameter("Duration");
		if (!durPara.name.empty() && durPara.value > 0) {
			p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].value = p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value + durPara.value;
		}
		else
			p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].value = p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value + 365*24*3600; // one year by default
	}
	// update duration unit
	double diff = p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].value - p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Unit durUnit;
	if (diff  >= 365*24*3600)	durUnit.set("a");
	else if (diff  > 24*3600)	durUnit.set("d");
	else						durUnit.set("h");
	p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = durUnit;

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("Modified simulation time",p.m_solverParameter, p.m_simulationParameter);
	undo->push();
}


void SVSimulationStartOptions::on_lineEditEndDate_editingFinished() {

	VICUS::Project p = project();

	IBK::Time endTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditEndDate->text().toStdString());

	// compose start time (startYear and offset are given and well defined, we ensure that)
	int startYear = p.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// compute difference between dates
	IBK::Time diff = endTime - startTime; // Might be negative!
	if (!diff.isValid()) {
		m_ui->lineEditDuration->blockSignals(true);
		m_ui->lineEditDuration->setValue(0); // set zero duration to indicate that something is wrong!
		m_ui->lineEditDuration->blockSignals(false);
		return;
	}

	// diff holds offset from start, so we need to add startOffset
	p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].set("End", offset + diff.secondsOfYear(), IBK::Unit("s"));
	// select unit based on time selected
	IBK::Unit durUnit;
	if (diff.secondsOfYear() >= 365*24*3600)	durUnit.set("a");
	else if (diff.secondsOfYear() > 24*3600)	durUnit.set("d");
	else										durUnit.set("h");
	p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = durUnit;

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("Modified simulation time",p.m_solverParameter, p.m_simulationParameter);
	undo->push();
}


void SVSimulationStartOptions::on_lineEditDuration_editingFinishedSuccessfully() {

	VICUS::Project p = project();

	// we always update the end time and let the end time signal do the undo action stuff
	IBK::Parameter durPara = m_ui->lineEditDuration->toParameter("Duration");
	if (durPara.name.empty())
		return; // invalid input in parameter edit

	if (durPara.value <= 0)
		return; // invalid input in parameter edit

	int startYear = p.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// add duration
	startTime += durPara.value;
	p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].set("End", startTime.secondsOfYear(), IBK::Unit("s"));
	// set duration unit in parameter - this will be used to select matching unit in combo box
	p.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = durPara.IO_unit;

	SVUndoModifySolverParameter *undo = new SVUndoModifySolverParameter("Modified simulation time",p.m_solverParameter, p.m_simulationParameter);
	undo->push();
}

void selectChildSurfaces(const VICUS::Surface &s, std::vector<const VICUS::Surface*> &selectedSurfaces) {
	for (const VICUS::Surface &cs : s.childSurfaces()) {
		selectedSurfaces.push_back(&cs);
		selectChildSurfaces(cs, selectedSurfaces);
	}
}


bool SVSimulationStartOptions::startSimulation(bool testInit, bool forceForegroundProcess, bool waitForFinishedProcess, bool calculateViewFactors) {


	// we save the project before starting simulation, everything is up to date now
	SVMainWindow::instance().saveProject();

	// Calculate view factors if required

	// we collect surfaces with long wave radiation on the inner side
	std::vector<const VICUS::Surface*> surfacesWithLWRad;

	const SVDatabase &db = SVSettings::instance().m_db;

	for (const VICUS::ComponentInstance &ci : project().m_componentInstances) {
		const VICUS::Component *comp = db.m_components[ci.m_idComponent];
		Q_ASSERT(comp!=nullptr);
		const VICUS::BoundaryCondition *bcA = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
		if ( bcA != nullptr &&
			 bcA->m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT &&
			 ci.m_sideASurface != nullptr )
			surfacesWithLWRad.push_back(ci.m_sideASurface);

		const VICUS::BoundaryCondition *bcB = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
		if ( bcB != nullptr &&
			 bcB->m_longWaveEmission.m_modelType != NANDRAD::InterfaceLongWaveEmission::NUM_MT &&
			 ci.m_sideBSurface != nullptr )
			surfacesWithLWRad.push_back(ci.m_sideBSurface);
	}

	// ask user if view factors shall be calculated
	if (!surfacesWithLWRad.empty()) {
		QMessageBox::StandardButton res = QMessageBox::question(this, "View-factor calculation with View3D",
																"Long wave radiation boundary conditions have been defined at some surfaces."
																" For correct energy calculation view-factors need to be pre-calculated."
																" Calculate view factors now?"
																"\n\nYou may skip, if view-factors have been calculated previously with unchanged geometry. "
																"If geometry changed please recalculate now.");
		if (res == QMessageBox::Yes) {
			try {
				SVView3DCalculation::calculateViewFactors(this, surfacesWithLWRad);
			} catch (IBK::Exception &ex) {
				QMessageBox::critical(this, tr("Error in view-factor calculation"),
									  tr("View-factors could not be calculated. Please see error below\n%1").arg(ex.what()));
			}
		}
	}


	// Actual simulation start

	QString resultPath;
	if (!generateNANDRAD(resultPath))
		return false;
	IBK::Path resultDir(resultPath.toStdString());

	bool cleanDir = false;
	updateCmdLine();
	QStringList commandLineArgs = m_commandLineArgs;
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

	QString solverExecutable = SVSettings::nandradSolverExecutable();
	QString nandradProjectFilePath = SVProjectHandler::instance().nandradProjectFilePath();


	// delete working directory if requested
	// launch solver - run option is only needed for linux, and otherwise it will always be -1
#if defined(Q_OS_LINUX)
	SVSettings::TerminalEmulators runOption = (SVSettings::TerminalEmulators)m_ui->comboBoxTermEmulator->currentIndex();
#else
	SVSettings::TerminalEmulators runOption = (SVSettings::TerminalEmulators)-1;
#endif
	// if foreground process is forced, ignore terminal settings and launch test-init directly
	unsigned int exitCode = 0;
	bool success = false;
	if (forceForegroundProcess) {
		QProgressDialog dlg(tr("Running test-init on NANDRAD project"), tr("Cancel"), 0, 0, this);
		dlg.setMinimumDuration(0);
		dlg.setParent(this);
		dlg.show();
		qApp->processEvents();

		commandLineArgs << nandradProjectFilePath;

		QProcess p;
		p.start(solverExecutable, commandLineArgs);
		p.waitForFinished(-1);
		exitCode = (unsigned int)p.exitCode();
		success = p.exitStatus() == 0 && exitCode == 0;

		if(waitForFinishedProcess && success)
			return true;

	}
	else
		success = SVSettings::startProcess(solverExecutable, commandLineArgs, nandradProjectFilePath, runOption, &exitCode);

	if (!success) {
		QMessageBox::critical(this, QString(), tr("Error running NANDRAD solver executable or the selected terminal emulator."));
		return false;
	}
	else {
		SVSettings::instance().showDoNotShowAgainMessage(this, "simulation-has-started-in-background", QString(),
														 tr("The simulation has been started as background process in a terminal window. "
															"You can close the window once the simulation has finished. Longer simulations will continue to run "
															"even if the SIM-VICUS application is closed."));
	}

	if (exitCode != 0) { // simulation/init error
		QMessageBox::critical(this, QString(), tr("An error ocurred during simulation or initialization."));
		on_pushButtonShowScreenLog_clicked();
	}

	return false; // keep dialog open
}


void SVSimulationStartOptions::showScreenLog() {
	// compose path to log file
	// compose log file name
	QString nandradProjectFilePath = SVProjectHandler::instance().nandradProjectFilePath();
	QString logfile = QFileInfo(nandradProjectFilePath).completeBaseName() + "/log/screenlog.txt";
	logfile = QFileInfo(nandradProjectFilePath).dir().absoluteFilePath(logfile);
	SVLogFileDialog dlg(this);
	dlg.setLogFile(logfile, nandradProjectFilePath, false);
	dlg.exec();
}


bool SVSimulationStartOptions::generateNANDRAD(QString & resultPath) {

	// compose NANDRAD project file and start simulation

	// generate NANDRAD project
	NANDRAD::Project nandradProj;
	const VICUS::Project &p = project();
	QStringList errorStack;
	QString nandradProjectFilePath = SVProjectHandler::instance().nandradProjectFilePath();

	try {
		// set placeholders in NANDRAD Project (VICUS-Project does not have placeholders)
		nandradProj.m_placeholders[VICUS::DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::databasesDir()).toStdString());
		nandradProj.m_placeholders[VICUS::USER_DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::userDataDir()).toStdString());
		// "Project Directory" placeholder is needed to resolve paths to files referenced via relative paths
		nandradProj.m_placeholders["Project Directory"] = IBK::Path(nandradProjectFilePath.toStdString()).parentPath().str();

		p.generateNandradProject(nandradProj, errorStack, nandradProjectFilePath.toStdString());

	}
	catch (IBK::Exception & ex) {
		// just show a generic error message
		ex.writeMsgStackToError();
		QString fullText;
		fullText = errorStack.join("\n");
		fullText += QString::fromStdString(ex.msgStack());

		// create dialog
		QDialog dialog(this);
		QVBoxLayout mainLayout(&dialog);
		// horizontal layout for icon and label
		QHBoxLayout headerLayout;
		QLabel *iconLabel = new QLabel(&dialog);
		QIcon criticalIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
		QPixmap iconPixmap = criticalIcon.pixmap(24, 24);
		iconLabel->setPixmap(iconPixmap);
		headerLayout.addWidget(iconLabel);
		// message
		QLabel *messageLabel = new QLabel(tr("An error occurred during NANDRAD project generation."), &dialog);
		QFont labelFont = messageLabel->font();
		labelFont.setBold(true);
		messageLabel->setFont(labelFont);
		headerLayout.addWidget(messageLabel);
		mainLayout.addLayout(&headerLayout);
		// text edit for more details
		QTextEdit *textEdit = new QTextEdit(&dialog);
		textEdit->setPlaceholderText(fullText);
		mainLayout.addWidget(textEdit);
		// close button
		QHBoxLayout buttonLayout;
		QPushButton *button = new QPushButton("Close", &dialog);
		buttonLayout.addWidget(button);
		buttonLayout.setContentsMargins(0, 0, 0, 0);
		buttonLayout.setAlignment(Qt::AlignRight);
		mainLayout.addLayout(&buttonLayout);

		QObject::connect(button, &QPushButton::clicked, &dialog, &QDialog::accept);

		dialog.setLayout(&mainLayout);
		dialog.setWindowTitle("NANDRAD Error");
		dialog.exec();

		return false;
	}

	// save project
	nandradProj.writeXML(IBK::Path(nandradProjectFilePath.toStdString()));
	/// TODO : check if project file was correctly written

	// get basename, for example /var/test/project.bla.12.2.nandrad  -> "project.bla.12.2"
	resultPath = QFileInfo(SVProjectHandler::instance().projectFile()).completeBaseName();
	// compose result path "/var/test/project.bla.12.2"
	resultPath = QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(resultPath);

	return true;
}


void SVSimulationStartOptions::on_comboBoxTermEmulator_currentIndexChanged(int index) {
	SVSettings::instance().m_terminalEmulator = (SVSettings::TerminalEmulators)(index);
}


void SVSimulationStartOptions::on_lineEditStartDate_returnPressed(){
	m_ui->lineEditStartDate->setFocus();
}


void SVSimulationStartOptions::on_lineEditEndDate_returnPressed() {
	m_ui->lineEditEndDate->setFocus();
}


void SVSimulationStartOptions::on_pushButtonRun_clicked() {
	startSimulation(false);
}


void SVSimulationStartOptions::on_pushButtonTestInit_clicked() {
	Q_ASSERT(m_simulationOutputOptions!=nullptr);
	if (startSimulation(true))
		m_simulationOutputOptions->updateUi();
}


void SVSimulationStartOptions::on_pushButtonExportFMU_clicked() {

	// we save the project before starting simulation, everything is up to date now
	SVMainWindow::instance().saveProject();

	QString resultPath;
	if (!generateNANDRAD(resultPath)) {
		return;
	}
	// launch external FMU generator tool
	QStringList cmdArgs;

	QString generatorExecutable = SVSettings::nandradFMUGeneratorExecutable();
	bool success = SVSettings::startProcess(generatorExecutable, cmdArgs, SVProjectHandler::instance().nandradProjectFilePath(), SVSettings::TE_None);
	if (!success) {
		QMessageBox::critical(this, QString(), tr("Could not run FMU Generator '%1'").arg(generatorExecutable));
		return;
	}
}


void SVSimulationStartOptions::on_pushButtonShowScreenLog_clicked() {
	showScreenLog();
}

