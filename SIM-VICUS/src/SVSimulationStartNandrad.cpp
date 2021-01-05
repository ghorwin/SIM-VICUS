#include "SVSimulationStartNandrad.h"
#include "ui_SVSimulationStartNandrad.h"

#include <QHBoxLayout>
#include <QMessageBox>


#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVSimulationPerformanceOptions.h"
#include "SVSimulationLocationOptions.h"
#include "SVSimulationOutputOptions.h"
#include "SVSimulationModelOptions.h"
#include "SVSimulationRunRequestDialog.h"
#include "SVConstants.h"

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
	m_ui->labelTerminalEmulator->setVisible(false);
	m_ui->comboBoxTermEmulator->setVisible(false);
#elif defined(Q_OS_LINUX)
	m_ui->checkBoxCloseConsoleWindow->setVisible(false);
#else
	// mac has neither option
	m_ui->checkBoxCloseConsoleWindow->setVisible(false);
	m_ui->labelTerminalEmulator->setVisible(false);
	m_ui->comboBoxTermEmulator->setVisible(false);
#endif
	m_ui->comboBoxTermEmulator->blockSignals(true);
	m_ui->comboBoxTermEmulator->setCurrentIndex(SVSettings::instance().m_terminalEmulator);
	m_ui->comboBoxTermEmulator->blockSignals(false);


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
		m_simulationModelOptions = new SVSimulationModelOptions(this, m_simParams, m_location);
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


	if (m_simParams == NANDRAD::SimulationParameter()) {
		m_simParams.m_solarLoadsDistributionModel.m_distributionType = NANDRAD::SolarLoadsDistributionModel::SWR_AreaWeighted;
		NANDRAD::KeywordList::setParameter(m_simParams.m_solarLoadsDistributionModel.m_para,
										   "SolarLoadsDistributionModel::para_t",
										   NANDRAD::SolarLoadsDistributionModel::P_RadiationLoadFractionZone, 50);
	}
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
	if (m_simParams.m_para[NANDRAD::SimulationParameter::P_InitialTemperature].name.empty() ||
		m_simParams.m_para[NANDRAD::SimulationParameter::P_InitialTemperature].IO_unit.base_id() != IBK_UNIT_ID_SECONDS)
	{
		NANDRAD::KeywordList::setParameter(m_simParams.m_para,
										   "SimulationParameter::para_t",
										   NANDRAD::SimulationParameter::P_InitialTemperature, 20);
	}

	// create default output settings, if nothing has been defined, yet
	if (m_outputs == VICUS::Outputs()) {
		m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].set("CreateDefaultZoneOutputs", true);
		NANDRAD::OutputGrid og;
		og.m_name = tr("Hourly values").toStdString();
		NANDRAD::Interval iv;
		NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_Start, 0);
//		NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_End, 0);
		NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_StepSize, 1);
		og.m_intervals.push_back(iv);
		m_outputs.m_grids.push_back(og);
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
	if (!startSimulation(false))
		return; // keep dialog open

	// TODO : should we keep the dialog open if simulation crashed?
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


/*! This exception class collects information about errors that occurred during transformation of data.
	The data in this class can be used to select the problematic geometry so that errors/missing data can be fixed quickly.
*/
class ConversionError : public IBK::Exception {
public:
	/*! TODO: */
	ConversionError(const std::string & errmsg) : IBK::Exception(errmsg, "ConversionError") {}
	ConversionError(const IBK::FormatString & errmsg) : IBK::Exception(errmsg, "ConversionError") {}
};


bool SVSimulationStartNandrad::generateNandradProject(NANDRAD::Project & p) {

	// TODO : in time this will be a rather lengthy function, maybe we should move this to a separate class with
	//        different member functions

	// simulation settings
	p.m_simulationParameter = m_simParams;

	// solver parameters
	p.m_solverParameter = m_solverParams;

	// location settings
	p.m_location = m_location;
	// do we have a climate path?
	if (!m_location.m_climateFileName.isValid()) {
		m_ui->tabWidget->setCurrentWidget(m_ui->tabClimate);
		QMessageBox::critical(this, tr("Starting NANDRAD simulation"), tr("A climate data file is needed. Please select a climate data file!"));
		return false;
	}

	// geometry

	// we process all zones and buildings and create NANDRAD project data
	// we also check that all referenced database properties are available and transfer them accordingly

	// this set collects all component instances that are actually used/referenced by zone surfaces
	// for now, unassociated components are ignored
	std::set<const VICUS::ComponentInstance*> usedComponentInstances;

	for (const VICUS::Building & b : project().m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				// first create a NANDRAD zone for the room
				NANDRAD::Zone z;
				z.m_id = r.m_id;
				z.m_displayName = r.m_displayName.toStdString();
				// Note: in the code below we expect the parameter's base units to be the same as the default unit for the
				//       populated parameters

				// TODO : what if we do not have an area or a zone volume, yet?
				NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, r.m_para[VICUS::Room::P_Area].value);
				NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, r.m_para[VICUS::Room::P_Volume].value);

				// for now, zones are always active
				z.m_type = NANDRAD::Zone::ZT_Active;
				// finally append zone
				p.m_zones.push_back(z);

				// now process all surfaces
				for (const VICUS::Surface & s : r.m_surfaces) {
					// each surface can be either a construction to the outside, to a fixed zone or to a different zone
					// the latter is only recognized, if we search through all zones and check their association with a surface.

					// if we have a component associated, remember its ID
					usedComponentInstances.insert(s.m_componentInstance);
				}
			}
		}
	}

	// this set collects all construction type IDs, which will be used to create constructionInstances
	std::set<unsigned int> usedConstructionTypes;

	const SVDatabase & db = SVSettings::instance().m_db;

	// now process all components and generate construction instances
	for (const VICUS::ComponentInstance * ci : usedComponentInstances) {
		if (ci == nullptr)
			continue; // skip invalid
		// lookup component that's referenced by componantInstance
		Q_ASSERT(ci->m_componentID != VICUS::INVALID_ID);
		// Note: component ID may be invalid or component may have been deleted from DB already
		const VICUS::Component * comp = SVSettings::instance().m_db.m_components[ci->m_componentID];
		if (comp == nullptr) {
			QMessageBox::critical(this, tr("Starting NANDRAD simulation"),
				tr("Component ID %1 referenced from component instance %2, but there is no such component.")
								  .arg(ci->m_componentID).arg(ci->m_id));
			return false;
		}
		if (!comp->isValid(db.m_materials, db.m_constructions, db.m_boundaryConditions)) {
			QMessageBox::critical(this, tr("Starting NANDRAD simulation"),
				tr("Component '%1' (id=%2), referenced from component instance (id=%3) has invalid/incomplete parametrization.")
					.arg(QString::fromStdString(comp->m_displayName.string(IBK::MultiLanguageString::m_language, "en")))
					.arg(ci->m_componentID).arg(ci->m_id));
			return false;
		}

		// now generate a construction instance
		NANDRAD::ConstructionInstance cinst;
		cinst.m_id = ci->m_id;

		// store reference to construction type (i.e. to be generated from component)
		cinst.m_constructionTypeId = comp->m_idOpaqueConstruction;
		usedConstructionTypes.insert(comp->m_idOpaqueConstruction);

		// set construction instance parameters
		// we have eitherone or two surfaces associated
		if (ci->m_sideASurface != nullptr) {
			// compute area
			double area = ci->m_sideASurface->m_geometry.area();
			if (ci->m_sideBSurface != nullptr) {
				// have both
				double areaB = ci->m_sideBSurface->m_geometry.area();
				// check if both areas are approximately the same
				if (std::fabs(area - areaB) > SAME_DISTANCE_PARAMETER_ABSTOL) {
					QMessageBox::critical(this, tr("Starting NANDRAD simulation"),
						tr("Component/construction %1 references surfaces %2 and %3, with mismatching areas %3 and %4 m2.")
										  .arg(ci->m_id).arg(ci->m_sideASurfaceID).arg(ci->m_sideBSurfaceID)
										  .arg(area).arg(areaB));
					return false;
				}
				/// TODO : Dirk, we have orientation of side A and B... which one do we use?
				double orientation = 0;
				double inclination = 0;

				// set parameters
				NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Inclination, inclination);
				NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Orientation, orientation);

				cinst.m_displayName = tr("Internal wall between surfaces '%1' and '%2'")
						.arg(ci->m_sideASurface->m_displayName).arg(ci->m_sideBSurface->m_displayName).toStdString();
			}
			else {

				// we only have side A, take orientation and inclination from side A

				/// TODO : Dirk, berechnen
				double orientation = 0;
				double inclination = 0;

				// set parameters
				NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Inclination, inclination);
				NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Orientation, orientation);

				cinst.m_displayName = ci->m_sideASurface->m_displayName.toStdString();
			}
			// set area parameter (computed from side A, but if side B is given as well, the area is the same
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, area);
		}
		else {
			Q_ASSERT(ci->m_sideBSurface != nullptr);

			// we only have side B, take orientation and inclination from side B

			/// TODO : Dirk, berechnen
			double orientation = 0;
			double inclination = 0;

			// set parameters
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Inclination, inclination);
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Orientation, orientation);

			// set area parameter
			double area = ci->m_sideBSurface->m_geometry.area();
			NANDRAD::KeywordList::setParameter(cinst.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, area);

			cinst.m_displayName = ci->m_sideBSurface->m_displayName.toStdString();
		}

		// add to list of construction instances
		p.m_constructionInstances.push_back(cinst);
	}

	// database elements

	std::set<unsigned int> usedMaterials;

	for (unsigned int conTypeID : usedConstructionTypes) {
		// lookup construction type in DB - since we checked component definitions with isValid() already above,
		// we can be sure that the construction instances exist and have valid parameters
		const VICUS::Construction * con = db.m_constructions[conTypeID];
		Q_ASSERT(con != nullptr);

		// now create a construction type
		NANDRAD::ConstructionType conType;
		conType.m_id = conTypeID;
		conType.m_displayName = con->m_displayName.string(IBK::MultiLanguageString::m_language, "en");

		for (const VICUS::MaterialLayer & ml : con->m_materialLayers) {
			NANDRAD::MaterialLayer mlayer;
			mlayer.m_matId = ml.m_matId;
			usedMaterials.insert(ml.m_matId);
			mlayer.m_thickness = ml.m_thickness.value;
			conType.m_materialLayers.push_back(mlayer);
		}
	}

	// outputs



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


bool SVSimulationStartNandrad::startSimulation(bool testInit) {
	// compose NANDRAD project file and start simulation

	// generate NANDRAD project
	NANDRAD::Project p;

	p.m_location = m_location;
	p.m_solverParameter = m_solverParams;
	p.m_simulationParameter = m_simParams;

	if (!generateNandradProject(p)) {
		return false;
	}

	// save project
	p.writeXML(IBK::Path(m_nandradProjectFilePath.toStdString()));
	/// TODO : check if project file was correctly written

	QString resultPath = QFileInfo(SVProjectHandler::instance().projectFile()).completeBaseName();
	resultPath = QFileInfo(SVProjectHandler::instance().projectFile()).dir().filePath(resultPath);
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
	SVSettings::TerminalEmulators runOption = (SVSettings::TerminalEmulators)m_ui->comboBoxTermEmulator->currentIndex();
	bool success = SVSettings::startProcess(m_solverExecutable, commandLineArgs, m_nandradProjectFilePath, runOption);
	if (!success) {
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(m_solverExecutable));
		return false;
	}

	// all ok, solver is running
	return true;
}


void SVSimulationStartNandrad::on_comboBoxTermEmulator_currentIndexChanged(int index) {
	SVSettings::instance().m_terminalEmulator = (SVSettings::TerminalEmulators)(index);
}


void SVSimulationStartNandrad::on_pushButtonTestInit_clicked() {
	startSimulation(true);
}
