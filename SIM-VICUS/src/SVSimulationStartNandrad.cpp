#include "SVSimulationStartNandrad.h"
#include "ui_SVSimulationStartNandrad.h"

#include <QHBoxLayout>


#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVSimulationPerformanceOptions.h"
#include "SVSimulationLocationOptions.h"
#include "SVSimulationOutputOptions.h"
#include "SVSimulationModelOptions.h"

#include "SVLogFileDialog.h"

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
#ifdef WIN32
	m_ui->checkBoxCloseConsoleWindow->setChecked(true);
#else
	m_ui->checkBoxCloseConsoleWindow->setVisible(false);
#endif


	{
		m_simulationPerformanceOptions = new SVSimulationPerformanceOptions(this, m_solverParams);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationPerformanceOptions);
		m_ui->tabPerformanceOptions->setLayout(h);
	}
	{
		m_simulationLocationOptions = new SVSimulationLocationOptions(this, m_location);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationLocationOptions);
		m_ui->tabClimate->setLayout(h);
	}
	{
		m_simulationOutputOptions = new SVSimulationOutputOptions(this, m_outputs);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationOutputOptions);
		m_ui->tabOutputs->setLayout(h);
	}
	{
		m_simulationModelOptions = new SVSimulationModelOptions(this, m_simParams);
		QHBoxLayout * h = new QHBoxLayout;
		h->addWidget(m_simulationModelOptions);
		m_ui->tabSimOptions->setLayout(h);
	}
}


SVSimulationStartNandrad::~SVSimulationStartNandrad() {
	delete m_ui;
}


int SVSimulationStartNandrad::edit() {

	m_solverExecutable = QFileInfo(SVSettings::instance().m_installDir + "/NandradSolver").filePath();
#ifdef WIN32
	m_solverExecutable += ".exe";
#endif // WIN32

	QString nandradProjectFilePath = QFileInfo(SVProjectHandler::instance().projectFile()).completeBaseName() + ".nandrad";
	m_nandradProjectFilePath = QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(nandradProjectFilePath);

	// store current project settings
	m_solverParams = project().m_solverParameter;
	m_location = project().m_location;
	m_outputs = project().m_outputs;
	m_simParams = project().m_simulationParameter;


	// initialize simulation parameters with meaningful defaults and fix possibly wrong values
	// in project (wherever they come from)
	if (m_simParams.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].name.empty() ||
		m_simParams.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value < 0)
	{
		m_simParams.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", 2019);
	}

	if (m_simParams.m_interval.m_para[ NANDRAD::Interval::P_Start].name.empty() ||
		m_simParams.m_interval.m_para[ NANDRAD::Interval::P_Start].value < 0 ||
		m_simParams.m_interval.m_para[ NANDRAD::Interval::P_Start].value > 365*24*3600)
	{
		m_simParams.m_interval.m_para[ NANDRAD::Interval::P_Start].set("Start", 0, IBK::Unit("d"));
	}

	if (m_simParams.m_interval.m_para[ NANDRAD::Interval::P_End].name.empty() ||
		m_simParams.m_interval.m_para[ NANDRAD::Interval::P_End].value < 0)
	{
		m_simParams.m_interval.m_para[ NANDRAD::Interval::P_End].set("End", 1, IBK::Unit("a"));
	}

	m_simulationPerformanceOptions->updateUi();
	m_simulationLocationOptions->updateUi();
	m_simulationOutputOptions->updateUi();
	m_simulationModelOptions->updateUi();

	updateTimeFrameEdits();
	updateCmdLine();

	return exec();
}


void SVSimulationStartNandrad::on_pushButtonClose_clicked() {
	// store data in project and close dialog
	storeInput();
	close();
}


void SVSimulationStartNandrad::on_pushButtonRun_clicked() {
	// compose NANDRAD project file and start simulation

	// generate NANDRAD project
	NANDRAD::Project p;

	p.m_location = m_location;
	p.m_solverParameter = m_solverParams;

	generateNandradProject(p);

	// save project
	p.writeXML(IBK::Path(m_nandradProjectFilePath.toStdString()));
	/// TODO : check if project file was correctly written

	// launch solver
	bool success = SVSettings::startProcess(m_solverExecutable, m_cmdArgs, m_nandradProjectFilePath);
	if (!success) {
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(m_solverExecutable));
		return;
	}

	storeInput();
	close(); // finally close dialog
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
	m_simParams.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", startTime.year());
	m_simParams.m_interval.m_para[NANDRAD::Interval::P_Start].set("Start", startTime.secondsOfYear(), IBK::Unit("s"));
	updateTimeFrameEdits();
}


void SVSimulationStartNandrad::on_lineEditEndDate_editingFinished() {
	IBK::Time endTime = IBK::Time::fromDateTimeFormat(m_ui->lineEditEndDate->text().toStdString());

	// compose start time (startYear and offset are given and well defined, we ensure that)
	int startYear = m_simParams.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = m_simParams.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// compute difference between dates
	IBK::Time diff = endTime - startTime; // Might be negative!
	if (!diff.isValid()) {
		m_ui->lineEditDuration->setValue(0); // set zero duration to indicate that something is wrong!
		return;
	}

	// end date is the offset from start, so we first need the start date
	m_simParams.m_interval.m_para[NANDRAD::Interval::P_End].set("End", diff.secondsOfYear(), IBK::Unit("s"));
	m_simParams.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = m_ui->lineEditDuration->currentUnit();

	updateTimeFrameEdits();
}


void SVSimulationStartNandrad::on_lineEditDuration_editingFinishedSuccessfully() {
	// we always update the end time and let the end time signal do the undo action stuff
	IBK::Parameter durPara = m_ui->lineEditDuration->toParameter("Duration");
	if (durPara.name.empty())
		return; // invalid input in parameter edit

	if (durPara.value <= 0)
		return; // invalid input in parameter edit

	int startYear = m_simParams.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	double offset = m_simParams.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	IBK::Time startTime(startYear, offset);

	// add duration
	startTime += durPara.value;
	m_simParams.m_interval.m_para[NANDRAD::Interval::P_End].set("End", startTime.secondsOfYear(), IBK::Unit("s"));
	// set duration unit in parameter - this will be used to select matching unit in combo box
	m_simParams.m_interval.m_para[NANDRAD::Interval::P_End].IO_unit = durPara.IO_unit;
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


bool SVSimulationStartNandrad::generateNandradProject(NANDRAD::Project & p) const {
	// simulation settings

	return true;
}


void SVSimulationStartNandrad::storeInput() {

	/// TODO : create an undo action for modification of the project

}


void SVSimulationStartNandrad::updateTimeFrameEdits() {

	m_ui->lineEditStartDate->blockSignals(true);
	m_ui->lineEditEndDate->blockSignals(true);
	m_ui->lineEditDuration->blockSignals(true);

	// Note: we can be sure that all the parameters are set, though possibly to invalid values

	int startYear = m_simParams.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].value;
	// fall-back to zero, if not specified
	double startOffset = m_simParams.m_interval.m_para[ NANDRAD::Interval::P_Start].value;

	IBK::Time t(startYear, startOffset);
	m_ui->lineEditStartDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	double endTime = m_simParams.m_interval.m_para[ NANDRAD::Interval::P_End].value;
	if (m_simParams.m_interval.m_para[ NANDRAD::Interval::P_End].name.empty())
		endTime = startOffset + 365*24*3600; // fallback to 1 year
	double simDuration = endTime - startOffset;
	t += simDuration;

	m_ui->lineEditEndDate->setText( QString::fromStdString(t.toDateTimeFormat()) );

	IBK::Parameter durationPara;
	// use unit from end
	durationPara = IBK::Parameter("Duration", 0, m_simParams.m_interval.m_para[ NANDRAD::Interval::P_End].IO_unit);
	durationPara.value = simDuration; // set value in seconds
	m_ui->lineEditDuration->setFromParameter(durationPara);

	m_ui->lineEditStartDate->blockSignals(false);
	m_ui->lineEditEndDate->blockSignals(false);
	m_ui->lineEditDuration->blockSignals(false);
}

