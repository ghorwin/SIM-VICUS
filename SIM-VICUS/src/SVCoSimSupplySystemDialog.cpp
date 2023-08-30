#include "SVCoSimSupplySystemDialog.h"
#include "ui_SVCoSimSupplySystemDialog.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVPreferencesDialog.h"

#include <QtExt_Directories.h>

#include <JlCompress.h>

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include <NANDRAD_Project.h>

#include <IBK_algorithm.h>
#include <IBK_BuildFlags.h>
#include <IBK_FormatString.h>

SVCoSimSupplySystemDialog::SVCoSimSupplySystemDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVCoSimSupplySystemDialog)
{
	m_ui->setupUi(this);
}


SVCoSimSupplySystemDialog::~SVCoSimSupplySystemDialog() {
	delete m_ui;
}


int SVCoSimSupplySystemDialog::exec() {
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


bool SVCoSimSupplySystemDialog::checkProjectData() const {
	QString errorColor = SVSettings::instance().m_theme == SVSettings::TT_Dark ? "red" : "darkRed";
	QString successColor = SVSettings::instance().m_theme == SVSettings::TT_Dark ? "lime" : "darkGreen";

	bool success = true;

	if (SVProjectHandler::instance().projectFile().isEmpty()) {
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Please save project first!</span>").arg(errorColor));
		return false;
	}
	// check for correct extension
	if (!SVProjectHandler::instance().projectFile().toLower().endsWith(".vicus")) {
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Expected project file name to end with '.vicus'. Please save project with correct file name!</span>").arg(errorColor));
		return false;
	}

	// check for sufficient project parameters
	if (success)
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Project data ok for CO2 balance and ventilation control model.</span>").arg(successColor));

	return success;
}


void SVCoSimSupplySystemDialog::generateNandradFMU(NANDRAD::Project project, const QString &modelName,
													 const QString &tmpDir, const QString &fmuTargetFile) const
{
	FUNCID(SVCoSimSupplySystemDialog::generateNandradFMU);

	bool success = true;

	// generate path to Nandrad FMU
	// create fmu directory
	QDir fmuDir(tmpDir);
	fmuDir.mkdir(fmuDir.path());

	QString resourceDir = tmpDir + "/resources";
	fmuDir.mkdir(resourceDir);

	// if we have a target path, copy the referenced climate data file to the new location and modify the path
	IBK::Path resourcePath(resourceDir.toStdString());
	IBK::Path fullClimatePath = project.m_location.m_climateFilePath.withReplacedPlaceholders(project.m_placeholders);
	if (!fullClimatePath.isFile()) {
		IBK_Message(IBK::FormatString("The referenced climate data file '%1' does not exist. Please select a climate data file!")
					.arg(fullClimatePath.str()), IBK::MSG_ERROR, FUNC_ID);
	}

	// target file path
	std::string targetFName = fullClimatePath.filename().str();
	targetFName = IBK::replace_string(targetFName, " ", "_");
	IBK::Path targetClimatePath = resourcePath / targetFName;
	IBK::IBK_Message( IBK::FormatString("Copying climate data file '%1' to '<fmu>/resources'\n").arg(fullClimatePath.filename()),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	if (!IBK::Path::copy(fullClimatePath, targetClimatePath)) {
		IBK_Message(IBK::FormatString("Cannot copy the referenced climate data file '%1' to target directory '%2'.")
					.arg(fullClimatePath.str()).arg(resourcePath.str()), IBK::MSG_ERROR, FUNC_ID);
	}
	// modify reference in project file
	project.m_location.m_climateFilePath = "${Project Directory}/" + targetFName;

	// copy used resources/tsv files

	// schedules with tsv-files
	for (std::map<std::string, std::vector<NANDRAD::LinearSplineParameter> >::iterator
		 it = project.m_schedules.m_annualSchedules.begin();
		 it != project.m_schedules.m_annualSchedules.end(); ++ it)
	{
		// TODO: skip FMI-substituted quantities

		for (NANDRAD::LinearSplineParameter & spline : it->second) {
			if (!spline.m_name.empty() && spline.m_tsvFile.isValid()) {
				// Mind: tsv file path may be relative path to project directory or elsewhere
				IBK::Path tsvFilePath = spline.m_tsvFile.withReplacedPlaceholders(project.m_placeholders);
				// TODO Anne, check if targetPath exists already and issue a warning/error message or
				//      append a new suffix to the tsv-filename to make it unique!
				//      Can happen if two LinearSplines reference tsv files like:
				//         /data/control/setpoints.tsv
				//         /data/control_alternative/setpoints.tsv
				IBK::Path targetPath = resourcePath / tsvFilePath.filename();
				IBK::Path::copy(tsvFilePath, targetPath);
				// change tsv file to point to relative path
				spline.m_tsvFile = IBK::Path("${Project Directory}/" + tsvFilePath.filename().str());
			}
		}
	}

	// network heatexchange spline data
	for (NANDRAD::HydraulicNetwork & n : project.m_hydraulicNetworks)
		for (NANDRAD::HydraulicNetworkElement & elem : n.m_elements) {
			for (int i=0; i<NANDRAD::HydraulicNetworkHeatExchange::NUM_SPL; ++i)
				if (!elem.m_heatExchange.m_splPara[i].m_name.empty() && elem.m_heatExchange.m_splPara[i].m_tsvFile.isValid()) {
					// Mind: tsv file path may be relative path to project directory or elsewhere
					IBK::Path tsvFilePath = elem.m_heatExchange.m_splPara[i].m_tsvFile.withReplacedPlaceholders(project.m_placeholders);
					IBK::Path targetPath = resourcePath / tsvFilePath.filename();
					IBK::Path::copy(tsvFilePath, targetPath);
					// change tsv file to point to relative path
					elem.m_heatExchange.m_splPara[i].m_tsvFile = IBK::Path("${Project Directory}/" + tsvFilePath.filename().str());
				}
		}


	// now all referenced files are stored alongside the project
	// remove not needed Database placeholder from placeholders list (but keep all custom placeholders!)
	auto it = project.m_placeholders.find("Database");
	if (it != project.m_placeholders.end())
		project.m_placeholders.erase(it);

	// now write the project into the export directory, it will always be called "Project.nandrad"
	project.writeXML(resourcePath / "Project.nandrad");
	IBK::IBK_Message( IBK::FormatString("Creating 'Project.nandrad' in '<fmu>/resources'\n"),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);


	// write model description

	QString modelId = "471a3b52-4923-44d8-ab4b-fcdb813c1244";
	QString modelDescription = "NANDRAD Simulation Model";
	QString version = NANDRAD::LONG_VERSION;

	generateModelDescription(tmpDir, modelName, modelId, modelDescription, version,
							 project.m_fmiDescription.m_inputVariables,
							 project.m_fmiDescription.m_outputVariables);


	// generate temporary directories for libraries

	QString installDir = SVSettings::instance().m_installDir;
	QString libraryDir = tmpDir + "/binaries";

	fmuDir.mkdir(libraryDir);

	// copy the binaries
	generateBinaries(installDir, libraryDir, "NandradSolverFMI", modelName, version);

	if (success) {

		// zip up the archive
		IBK::IBK_Message( IBK::FormatString("Compressing folder and creating FMU '%1'.\n").arg(fmuTargetFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		success = JlCompress::compressDir(fmuTargetFile, tmpDir);
		if (!success) {
			IBK::IBK_Message(IBK::FormatString("Error compressing the FMU archive (maybe invalid target path or "
												   "invalid characters used?)."), IBK::MSG_ERROR, FUNC_ID);
		}
	}

	// remove temporary directory structure
	QtExt::Directories::removeDirRecursively(tmpDir);
}


void SVCoSimSupplySystemDialog::generateMasterSimFile(	const QString &targetDir,
														const QString &modelName) const
{

	// create tsv files for all missing schedules

	const char * const CO2_SIMULATOR_TEMPLATE =
			"simulator 0 0 CO2ComfortVentilation #a0522d \"${NAME}.fmus/CO2ComfortVentilation.fmu\"\n";

	const char * const NANDRAD_SIMULATOR_TEMPLATE =
			"simulator 1 0 NANDRAD #a0522d \"${NAME}.fmus/NANDRAD.fmu\"\n";

//	const char * const INPUT_CONNECTION_TEMPLATE =
//			"graph NANDRAD.${NAME} CO2ComfortVentilation.${NAME}\n";

//	const char * const OUTPUT_CONNECTION_TEMPLATE =
//			"graph CO2ComfortVentilation.${NAME} NANDRAD.${NAME}\n";

//	const char * const MAX_CO2_CONNECTION_TEMPLATE =
//			"graph MaximumCO2Concentration_${TEMPLATE}.MaximumCO2Concentration CO2ComfortVentilation.${NAME} 0 0.000001\n";

	const VICUS::Project &prj = SVProjectHandler::instance().project();

	// start with global template
	IBK::Path fPath(":/templates/coSimulation.msim.template");
	QFile f(fPath.c_str());
	f.open(QFile::ReadOnly);
	QTextStream strm(&f);

	QString modelDesc = strm.readAll();

	// generate header

	// ${DATETIME} - 2018-08-01T12:49:19Z
	QDateTime t=QDateTime::currentDateTime();
	QString dt = t.toString(Qt::ISODate);
	modelDesc.replace("${DATETIME}", dt);

	// ${STARTTIME}
	QString startTimeStr = QString("%1 d").arg(prj.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value/3600./24.);
	modelDesc.replace("${STARTTIME}", startTimeStr);

	// ${ENDTIME}
	QString endTimeStr = QString("%1 d").arg(prj.m_simulationParameter.m_interval.endTime()/3600./24.);
	modelDesc.replace("${ENDTIME}", endTimeStr);


	// generate simulator section

	QString simulators;

	// add simulators
	QString co2simulator = CO2_SIMULATOR_TEMPLATE;
	co2simulator.replace("${NAME}", modelName);
	simulators += co2simulator;

	QString nandradSimulator = NANDRAD_SIMULATOR_TEMPLATE;
	nandradSimulator.replace("${NAME}", modelName);
	simulators += nandradSimulator;

	modelDesc.replace("${SIMULATORS}", simulators);


	// generate connection section

	QString connections;

	modelDesc.replace("${CONNECTIONS}", connections);

	QFile of(targetDir + "/" + modelName + ".msim");
	of.open(QFile::WriteOnly);
	of.write(modelDesc.toUtf8());
	of.close();
}


void SVCoSimSupplySystemDialog::generateBinaries(const QString & sourceDir,
												   const QString & targetDir,
												   const QString & sourceName,
												   const QString & targetName,
												   const QString & version) const
{
	FUNCID(SVCoSimSupplySystemDialog::generateBinaries);
	// copy the binaries

	QDir fmuDir(targetDir);

	// linux
	QString fmuLibFile;

#ifdef IBK_DEBUG
	if(!version.isEmpty()) {
		fmuLibFile = sourceDir + "/lib" + sourceName + ".so" + "." + version;
	}
	else {
		fmuLibFile = sourceDir + "/lib" + sourceName + ".so";
	}
#else
	fmuLibFile = sourceDir + "/lib" + sourceName + ".so";
#endif
	if (QFile(fmuLibFile).exists()) {
		fmuDir.mkdir(targetDir + "/linux64");
		QString targetPath = targetDir + "/linux64/" + targetName + ".so";
		IBK::IBK_Message( IBK::FormatString("Copying Linux FMU lib '%1' to '%2'\n").arg(fmuLibFile.toStdString()).arg(targetPath.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QFile::copy(fmuLibFile, fmuDir.absoluteFilePath(targetPath));
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (linux64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// macos
	fmuLibFile = sourceDir + "/lib" + sourceName + ".dylib";
	if (QFile(fmuLibFile).exists()) {
		fmuDir.mkdir(targetDir + "/darwin64");
		QString targetPath = targetDir + "/darwin64/" + targetName + ".dylib";
		IBK::IBK_Message( IBK::FormatString("Copying MacOS FMU lib '%1' to '%2'\n").arg(fmuLibFile.toStdString()).arg(targetPath.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QFile::copy(fmuLibFile, fmuDir.absoluteFilePath(targetPath));
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (darwin64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// win64
	fmuLibFile = sourceDir + "/" + sourceName + ".dll";

	if (QFile(fmuLibFile).exists()) {
		IBK::IBK_Message( IBK::FormatString("Copying Win64 FMU lib '%1'\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		fmuDir.mkdir(targetDir + "/win64");
		QString targetPath = targetDir + "/win64/" + targetName + ".dll";
		QString binTargetPath = fmuDir.absoluteFilePath(targetDir + "/win64/");
		QFile::copy(fmuLibFile, fmuDir.absoluteFilePath(targetPath));

		QStringList copyFiles;
		copyFiles << "msvcp140.dll"
				  << "vcomp140.dll"
				  << "vcruntime140.dll";
		for (int i=0; i<copyFiles.count(); ++i) {
			if (!QFile::exists(copyFiles[i])) {
				IBK::IBK_Message( IBK::FormatString("Missing file '%1' to copy into FMU archive.\n").arg(copyFiles[i].toStdString()),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			}
			else {
				QFile::copy(sourceDir + "/" + copyFiles[i], binTargetPath + "/" + QFileInfo(copyFiles[i]).fileName());
				IBK::IBK_Message( IBK::FormatString("Copying '%1' into FMU archive\n").arg(copyFiles[i].toStdString()),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			}
		}
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (Win64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

}

void SVCoSimSupplySystemDialog::generateModelDescription(const QString & targetDir,
														   const QString & modelName,
														   const QString & modelId,
														   const QString & modelDescription,
														   const QString & version,
														   const std::vector<NANDRAD::FMIVariableDefinition> & inputVars,
														   const std::vector<NANDRAD::FMIVariableDefinition> & outputVars) const
{
	FUNCID(SVCoSimSupplySystemDialog::generateModelDescription);

	// start with global template
	IBK::Path fPath(":/templates/modelDescription.xml.template");
	QFile f(fPath.c_str());
	f.open(QFile::ReadOnly);
	QTextStream strm(&f);

	QString modelDesc = strm.readAll();

	// ${MODELNAME}
	modelDesc.replace("${MODELNAME}", "\"" + modelName + "\"");

	// ${MODELNAME}
	modelDesc.replace("${DESCRIPTION}", "\"" + modelDescription + "\"");

	// ${GUID}
	modelDesc.replace("${GUID}", "\"{" + modelId + "}\"");

	// ${VERSION}
	modelDesc.replace("${VERSION}", "\"" + version + "\"");

	// ${DATETIME} - 2018-08-01T12:49:19Z
	QDateTime t=QDateTime::currentDateTime();
	QString dt = t.toString(Qt::ISODate);
	modelDesc.replace("${DATETIME}", dt);

	// generate variable and modelStructure section
	QString modelVariables;
	QString modelStructure;

	const char * const INPUT_VAR_TEMPLATE =
			"		<!-- Index of variable = \"${INDEX}\" -->\n"
			"		<ScalarVariable\n"
			"			name=\"${NAME}\"\n"
			"			valueReference=\"${VALUEREF}\"\n"
			"			variability=\"continuous\"\n"
			"			causality=\"input\">\n"
			"			<Real start=\"${STARTVALUE}\" unit=\"${REALVARUNIT}\"/>\n"
			"		</ScalarVariable>\n"
			"\n";

	const char * const OUTPUT_VAR_TEMPLATE =
			"		<!-- Index of variable = \"${INDEX}\" -->\n"
			"		<ScalarVariable\n"
			"			name=\"${NAME}\"\n"
			"			valueReference=\"${VALUEREF}\"\n"
			"			variability=\"continuous\"\n"
			"			causality=\"output\"\n"
			"			initial=\"calculated\">\n"
			"			<Real unit=\"${REALVARUNIT}\"/>\n"
			"		</ScalarVariable>\n"
			"\n";

	// process all variables
	QSet<QString> units;

	// write CO2 model decription file: inputs for NANDRAD are outputs for CO2 ventilation FMU
	int index=1;
	for (std::vector<NANDRAD::FMIVariableDefinition>::const_iterator
		 varIt = inputVars.begin(); varIt != inputVars.end();
		 ++varIt, ++index)
	{
		const NANDRAD::FMIVariableDefinition & varDef = *varIt;
		QString varDesc;
		varDesc = INPUT_VAR_TEMPLATE;
		varDesc.replace("${INDEX}", QString("%1").arg(index));
		varDesc.replace("${NAME}", varDef.m_fmiVarName.c_str());
		varDesc.replace("${VALUEREF}", QString("%1").arg(varDef.m_fmiValueRef));
		varDesc.replace("${STARTVALUE}", QString::number(varDef.m_fmiStartValue));
		varDesc.replace("${REALVARUNIT}", varDef.m_unit.c_str());
		modelVariables += varDesc;
		units.insert(varDef.m_unit.c_str());
	}

	for (std::vector<NANDRAD::FMIVariableDefinition>::const_iterator
		 varIt = outputVars.begin(); varIt != outputVars.end();
		 ++varIt, ++index)
	{
		const NANDRAD::FMIVariableDefinition & varDef = *varIt;
		QString varDesc;
		varDesc = OUTPUT_VAR_TEMPLATE;
		varDesc.replace("${INDEX}", QString("%1").arg(index));
		varDesc.replace("${NAME}", varDef.m_fmiVarName.c_str());
		varDesc.replace("${VALUEREF}", QString("%1").arg(varDef.m_fmiValueRef));
		varDesc.replace("${STARTVALUE}", QString::number(varDef.m_fmiStartValue));
		varDesc.replace("${REALVARUNIT}", varDef.m_unit.c_str());
		modelVariables += varDesc;
		modelStructure += QString(" 			<Unknown index=\"%1\"/>\n").arg(index);
		units.insert(varDef.m_unit.c_str());
	}

	// ${MODELVARIABLES}
	modelDesc.replace("${MODELVARIABLES}", modelVariables);

	// compose unit definitions section
	//		<UnitDefinitions>
	//			<Unit name="C"/>
	//			<Unit name="W/m2"/>
	//		</UnitDefinitions>

	QString unitDefs;
	if (!units.isEmpty()) {
		unitDefs += "	<UnitDefinitions>\n";
		for (QString u : units) {
			unitDefs += "		<Unit name=\"" + u + "\"/>\n";
		}
		unitDefs += "	</UnitDefinitions>\n";
	}

	modelDesc.replace("${UNIT_DEFINITIONS}", unitDefs);

	// ${MODEL_STRUCTURE_OUTPUTS} -
	// 			<Unknown index="1"/>
	//			<Unknown index="2"/>
	modelDesc.replace("${MODEL_STRUCTURE_OUTPUTS}", modelStructure);

	// finally write the file
	IBK::IBK_Message( IBK::FormatString("Creating '<fmu>/modelDescription.xml'\n"),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	QFile of(targetDir + "/modelDescription.xml");
	of.open(QFile::WriteOnly);
	of.write(modelDesc.toUtf8());
	of.close();

}


void SVCoSimSupplySystemDialog::on_pushButtonGenerate_clicked() {
	FUNCID(SVCoSimSupplySystemDialog::on_pushButtonGenerate_clicked);

	// Note: content checks have been made already in checkProjectData()
	m_ui->plainTextEdit->setEnabled(true);

	// generate file paths
	QString vicusProjectFile = SVProjectHandler::instance().projectFile();  //  -> "/path/to/project.vicus"
	QFileInfo finfo(vicusProjectFile);
	// parent path
	QString parentPath = finfo.dir().absolutePath();	//  -> "/path/to"
	QString baseName = finfo.baseName();				//  -> "project"

	// get overall base dir
	QString basePath = parentPath + "/" + baseName;		//  -> "/path/to/project"
	QDir baseDir(basePath);

	// remove generation directory if existing
	if (baseDir.exists()) {
		IBK::IBK_Message("Removing existing FMU export directory.", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QtExt::Directories::removeDirRecursively(baseDir.absolutePath());
	}

	// first create base directory
	baseDir.mkdir(baseDir.path());
	baseDir.mkdir(baseDir.path()+".fmus");

	m_nandradFMUBaseDir = basePath + ".fmus/NANDRAD";
	m_nandradFMUFilePath = basePath + ".fmus/NANDRAD.fmu";
	m_msimProjectFilePath = basePath + ".msim";


	// store access to VICUS project file
	const VICUS::Project &project = SVProjectHandler::instance().project();


	// NANDRAD FMU

	NANDRAD::Project nandradProject;

	QString resourceDir = m_nandradFMUBaseDir + "/resources";
	QString nandradProjectFile = resourceDir + "/Project.nandrad";
	QStringList errorStack;

	// generate NANDRAD FMU

	try {
		// set placeholders in NANDRAD Project (VICUS-Project does not have placeholders)
		nandradProject.m_placeholders[VICUS::DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::databasesDir()).toStdString());
		nandradProject.m_placeholders[VICUS::USER_DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::userDataDir()).toStdString());
		// "Project Directory" placeholder is needed to resolve paths to files referenced via relative paths
		nandradProject.m_placeholders["Project Directory"] = parentPath.toStdString();

		project.generateNandradProject(nandradProject, errorStack, nandradProjectFile.toStdString());
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
		return;
	}


	// generate fmu description
	QString modelName = baseName;

	generateNandradFMU(nandradProject, modelName, m_nandradFMUBaseDir, m_nandradFMUFilePath);

	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg(m_nandradFMUFilePath));


	// generate MSIM-Project in base path

	IBK::Path parentDir = IBK::Path(vicusProjectFile.toStdString()).parentPath();
	generateMasterSimFile(QString(parentDir.c_str()), modelName);
	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg(m_msimProjectFilePath));


	// all successful, enable "Launch MasterSim" button

	m_ui->pushButtonLaunchMasterSim->setEnabled(true);
}


void SVCoSimSupplySystemDialog::on_pushButtonLaunchMasterSim_clicked() {
	// check if we have MasterSim path configured
	QString masterSimPath = SVSettings::instance().m_masterSimExecutable;

	QFileInfo info(masterSimPath);

#ifdef Q_OS_LINUX
	if(!masterSimPath.isEmpty()) {
		//search file through all pathes
		if(!info.exists() && info.path()==".") {
			// retrieve file path using linux 'which' commmanf
			std::string cmd = "which " + masterSimPath.toStdString();

			char buffer[128];
			std::string result;
			// use pipe for executing command and getting feedback
			FILE* pipe = popen(cmd.c_str(), "r");

			if (!pipe) {
				QMessageBox::critical(this, tr("Error starting external application"), tr("MASTERSIM path '%1' could not be found.")
									  .arg(masterSimPath));
			}
			try {
				while (fgets(buffer, sizeof buffer, pipe) != nullptr)
					result += buffer;
			}
			catch (...) {
				pclose(pipe);
				QMessageBox::critical(this, tr("Error starting external application"), tr("MASTERSIM path '%1' could not be found.")
									  .arg(masterSimPath));
				return;
			}
			pclose(pipe);

			masterSimPath = QString::fromStdString(result).simplified();
			info.setFile(masterSimPath);
		}
	}
#endif

	if (masterSimPath.isEmpty() || !info.exists())
	{
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to "
																  "MASTERSIM in the preferences dialog!"));

		SVMainWindow::instance().preferencesDialog()->edit(0);

		masterSimPath = SVSettings::instance().m_masterSimExecutable;

		// still no valid path?
		if (masterSimPath.isEmpty() || !QFileInfo::exists(masterSimPath))
		return;
	}

	// launch MasterSim - run option is only needed for linux, and otherwise it will always be -1
#if defined(Q_OS_LINUX)
	SVSettings::TerminalEmulators runOption = SVSettings::instance().m_terminalEmulator;
#else
	SVSettings::TerminalEmulators runOption = (SVSettings::TerminalEmulators)-1;
#endif

	bool res = SVSettings::startProcess(masterSimPath, QStringList(), m_msimProjectFilePath, runOption);
	if (!res) {
		QMessageBox::critical(this, tr("Error starting external application"), tr("MASTERSIM '%1' could not be started.")
							  .arg(masterSimPath));
	}
}
