#include "NandradFMUGeneratorWidget.h"
#include "ui_NandradFMUGeneratorWidget.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>
#include <QTimer>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include <QtExt_Directories.h>

#include <JlCompress.h>

#include <IBK_messages.h>

#include "VariableTableModel.h"

#ifdef Q_OS_WIN
#undef UNICODE
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

const char * const ORGANIZATION = "IBK";
const char * const PROGRAM_NAME = "NANDRADFMUGenerator";


NandradFMUGeneratorWidget::NandradFMUGeneratorWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::NandradFMUGeneratorWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditTargetDirectory->setup("", false, true, QString());

	// create models
	m_availableInputVariables = new VariableTableModel(this, true);
	m_availableOutputVariables = new VariableTableModel(this, false);

	// TODO : create sort filter proxy model

	// set models
	m_ui->tableViewInputVars->setModel(m_availableInputVariables);
	m_ui->tableViewOutputVars->setModel(m_availableOutputVariables);

	m_ui->tableViewInputVars->horizontalHeader()->setStretchLastSection(true);

	QTableView * v = m_ui->tableViewInputVars;
	v->verticalHeader()->setDefaultSectionSize(19);
	v->verticalHeader()->setVisible(false);
	v->horizontalHeader()->setMinimumSectionSize(19);
	v->setSelectionBehavior(QAbstractItemView::SelectRows);
	v->setSelectionMode(QAbstractItemView::SingleSelection);
	v->setAlternatingRowColors(true);
	v->setSortingEnabled(false);
	v->sortByColumn(0, Qt::AscendingOrder);
	// smaller font for entire table
	QFont f;
#ifdef Q_OS_LINUX
	f.setPointSizeF(f.pointSizeF()*0.8);
#endif // Q_OS_WIN
	v->setFont(f);
	v->horizontalHeader()->setFont(f); // Note: on Linux/Mac this won't work until Qt 5.11.1 - this was a bug between Qt 4.8...5.11.1

	v = m_ui->tableViewOutputVars;
	v->verticalHeader()->setDefaultSectionSize(19);
	v->verticalHeader()->setVisible(false);
	v->horizontalHeader()->setMinimumSectionSize(19);
	v->setSelectionBehavior(QAbstractItemView::SelectRows);
	v->setSelectionMode(QAbstractItemView::SingleSelection);
	v->setAlternatingRowColors(true);
	v->setSortingEnabled(false);
	v->sortByColumn(0, Qt::AscendingOrder);
	v->setFont(f);
	v->horizontalHeader()->setFont(f); // Note: on Linux/Mac this won't work until Qt 5.11.1 - this was a bug between Qt 4.8...5.11.1

	m_ui->tabWidget->setCurrentIndex(0);

}


NandradFMUGeneratorWidget::~NandradFMUGeneratorWidget() {
	delete m_ui;
}


void NandradFMUGeneratorWidget::init() {
	// If we do not yet have a file path to NANDRAD project, try to load the last one used
	if (!m_nandradFilePath.isValid()) {
		// restore last correct project file
		QSettings s(ORGANIZATION, PROGRAM_NAME);
		QString projectFile = s.value("LastNANDRADProject").toString();
		if (!projectFile.isEmpty() && QFile(projectFile).exists()) {
			m_nandradFilePath = IBK::Path(projectFile.toStdString());
			// also restore FMU target path
			QString FMUExportDirectory = s.value("LastFMUExportDirectory").toString();
			if (!FMUExportDirectory.isEmpty())
				m_fmuExportDirectory = IBK::Path(FMUExportDirectory.toStdString());
		}
	}
	if (!m_nandradFilePath.isValid()) {
		// If we do not yet have a file path to NANDRAD project, ask for it on start
		QTimer::singleShot(0, this, &NandradFMUGeneratorWidget::on_pushButtonSelectNandradProject_clicked);
	}
	else {
		// setup user interface with project file data
		QTimer::singleShot(0, this, &NandradFMUGeneratorWidget::setup);

		// afterwards trigger auto-export, if requested
		if (!m_autoExportModelName.isEmpty())
			QTimer::singleShot(0, this, &NandradFMUGeneratorWidget::autoGenerate);
	}
}

void NandradFMUGeneratorWidget::setModelName(const QString & modelName) {
	m_ui->lineEditModelName->setText(modelName);
	on_lineEditModelName_editingFinished();
}


int NandradFMUGeneratorWidget::setup() {

	// read NANDRAD project
	try {
		m_project = NANDRAD::Project();
		m_project.readXML(m_nandradFilePath);

		// also perform basic sanity checks on fmi definitions
		m_project.m_fmiDescription.checkParameters();
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		QMessageBox::critical(this, tr("Error reading NANDRAD project"),
							  tr("Reading of NANDRAD project file '%1' failed.").arg(QString::fromStdString(m_nandradFilePath.str())) );
		// disable all GUI elements
		setGUIState(false);
		return 1;
	}

	// store project file for next start of generator tool
	QSettings s(ORGANIZATION, PROGRAM_NAME);
	s.setValue("LastNANDRADProject", QString::fromStdString(m_nandradFilePath.str()) );

	setGUIState(true);

	// we set default FMU model name automatically if not yet specified
	if (m_project.m_fmiDescription.m_modelName.empty())
		m_project.m_fmiDescription.m_modelName = m_nandradFilePath.filename().withoutExtension().str();

	// initialize fmu export path from project file if still empty
	if (!m_fmuExportDirectory.isValid())
		m_fmuExportDirectory = m_nandradFilePath.parentPath();

	// *** transfer general parameters

	m_ui->lineEditNandradProjectFilePath->setText( QString::fromStdString(m_nandradFilePath.str()) );
	m_ui->lineEditModelName->setText( QString::fromStdString(m_project.m_fmiDescription.m_modelName) );
	m_ui->lineEditTargetDirectory->setFilename( QString::fromStdString(m_fmuExportDirectory.str()) );
	// check correct FMU name and update target file path
	on_lineEditModelName_editingFinished();
	// now test-init the solver and update the variable tables
	updateVariableLists();

	return 0; // success
}


void NandradFMUGeneratorWidget::autoGenerate() {
	// set override model name
	setModelName(m_autoExportModelName);
	bool success = generate();
	if (!success)
		QApplication::exit(); // exit with return code 1
	else
		QApplication::quit(); // exit with return code 0 = success
}

void NandradFMUGeneratorWidget::on_tableWidgetInputVars_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{

}

void NandradFMUGeneratorWidget::on_tableWidgetOutputVars_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{

}


void NandradFMUGeneratorWidget::on_toolButtonAddInputVariable_clicked() {

}


void NandradFMUGeneratorWidget::on_toolButtonRemoveInputVariable_clicked() {

}



void NandradFMUGeneratorWidget::on_toolButtonAddOutputVariable_clicked() {

}


void NandradFMUGeneratorWidget::on_toolButtonRemoveOutputVariable_clicked() {

}


void NandradFMUGeneratorWidget::on_pushButtonGenerate_clicked() {
	// first update NANDRAD Project and save it to file
	on_pushButtonSaveNandradProject_clicked();

	// input data check
	if (!checkModelName())
		return;

	// now generate the FMU
	generate();
}


void NandradFMUGeneratorWidget::on_lineEditModelName_editingFinished() {
	m_ui->lineEditFMUPath->setText("---");
	if (!checkModelName())
		return;
	QString modelName = m_ui->lineEditModelName->text();
	// update FMU path
	m_ui->lineEditFMUPath->setText( QString::fromStdString(m_fmuExportDirectory.str()) + "/" + modelName + ".fmu");
	m_project.m_fmiDescription.m_modelName = modelName.toStdString();
}


void NandradFMUGeneratorWidget::on_lineEditTargetDirectory_editingFinished() {
	if (m_ui->lineEditTargetDirectory->filename().trimmed().isEmpty())
		return;
	m_fmuExportDirectory = IBK::Path(m_ui->lineEditTargetDirectory->filename().trimmed().toStdString());
	on_lineEditModelName_editingFinished();
}


void NandradFMUGeneratorWidget::on_lineEditTargetDirectory_returnPressed() {
	if (m_ui->lineEditTargetDirectory->filename().trimmed().isEmpty())
		return;
	m_fmuExportDirectory = IBK::Path(m_ui->lineEditTargetDirectory->filename().trimmed().toStdString());
	on_lineEditModelName_editingFinished();
}


void NandradFMUGeneratorWidget::on_pushButtonSaveNandradProject_clicked() {
	// input data check
	if (!checkModelName())
		return;

	// data in m_project is already up-to-date, so just write out the project
	m_project.writeXML(m_nandradFilePath);
	QMessageBox::information(this, tr("Project file written"),
							 tr("Saved NANDRAD Project '%1'.").arg(QString::fromStdString(m_nandradFilePath.str())));
}


void NandradFMUGeneratorWidget::on_pushButtonSelectNandradProject_clicked() {
	QSettings s(ORGANIZATION, PROGRAM_NAME);
	QString projectFile = s.value("LastNANDRADProject").toString();
	QString fname = QFileDialog::getOpenFileName(this, tr("Select NANDRAD Project"), projectFile,
												 tr("NANDRAD Project Files (*.nandrad);;All files (*)"), nullptr,
												 QFileDialog::DontUseNativeDialog);
	if (fname.isEmpty()) {
		setGUIState(false);
		return; // dialog was cancelled
	}

	m_nandradFilePath = IBK::Path(fname.toStdString());

	// setup user interface with project file data
	setup();
}


void NandradFMUGeneratorWidget::onProcessStarted() {
	qDebug() << "Started NandradSolver successfully";
}


void NandradFMUGeneratorWidget::onProcessErrorOccurred() {
	qDebug() << "NandradSolver start error";
}


// *** PRIVATE MEMBER FUNCTIONS ****


void NandradFMUGeneratorWidget::setGUIState(bool active) {
	// if active, all table widgets and push buttons are enabled, otherwise disabled
	m_ui->tabInputVariables->setEnabled(active);
	m_ui->tabOutputVariables->setEnabled(active);
	if (!active)
		m_ui->tabWidget->setCurrentIndex(0);
	m_ui->pushButtonGenerate->setEnabled(active);
	m_ui->pushButtonSaveNandradProject->setEnabled(active);
	m_ui->lineEditModelName->setEnabled(active);
	m_ui->lineEditTargetDirectory->setEnabled(active);
	m_ui->lineEditFMUPath->setEnabled(active);
}


bool NandradFMUGeneratorWidget::checkModelName() {
	QString modelName = m_ui->lineEditModelName->text().trimmed();
	if (modelName.isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Missing model name."));
		return false;
	}

	// check model name for allowed characters
	const std::string allowedChars = "-.,";
	for (int i=0; i<modelName.size(); ++i) {
		// check if character is an accepted char
		QChar ch = modelName[i];
		if (ch >= 'A' && ch <= 'Z') continue;
		if (ch >= 'a' && ch <= 'z') continue;
		if (ch >= '0' && ch <= '9') continue;
		// check any other acceptable chars
		if (allowedChars.find(ch.toLatin1()) != std::string::npos) continue;
		QMessageBox::critical(this, QString(), tr("Model name contains invalid characters."));
		return false;
	}

	// check leading 0
	if (modelName[0] >= '0' && modelName[0] <= '9') {
		QMessageBox::critical(this, QString(), tr("Model name must not start with a number character."));
		return false;
	}

	if (m_ui->lineEditTargetDirectory->filename().trimmed().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Missing target path name."));
		return false;
	}

	return true;
}


void NandradFMUGeneratorWidget::updateVariableLists() {
	// Test-Init project and then read input/output value refs

	QStringList commandLineArgs;
	commandLineArgs.append("--test-init");

#ifdef Q_OS_WIN
	commandLineArgs.append("-x"); // do not show "press key"
#endif

//	bool success = startProcess(m_nandradSolverExecutable, commandLineArgs, QString::fromStdString(m_nandradFilePath.str()));

	commandLineArgs.append(QString::fromStdString(m_nandradFilePath.str()));

	QString solverExecutable = m_nandradSolverExecutable;

	QProcess proc(this);
	proc.setProgram(solverExecutable);
	proc.setArguments(commandLineArgs);

	connect(&proc, &QProcess::started, this, &NandradFMUGeneratorWidget::onProcessStarted);
#if QT_VERSION >= 0x050600
	connect(&proc, &QProcess::errorOccurred, this, &NandradFMUGeneratorWidget::onProcessErrorOccurred);
#endif
	proc.start();
	// start process
	bool success = proc.waitForStarted();
	if (!success) {
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(solverExecutable));
		return;
	}

	proc.waitForFinished();

	if (proc.exitStatus() == QProcess::NormalExit) {
		if (proc.exitCode() != 0) {
			QMessageBox::critical(this, QString(), tr("There were errors during project test-initialization. Please ensure that the NANDRAD project runs successfully!"));
			return;
		}
	}

	// TODO : For extremely large simulation projects, the intialization itself may take more than 30 seconds, so
	//        we may add a progress indicator dialog


	QString nandradProjectFilePath = QString::fromStdString(m_nandradFilePath.str());
	// now parse the variable lists
	IBK::Path varDir(nandradProjectFilePath.toStdString());
	varDir = varDir.withoutExtension() / "var";

	QString inputVarsFile = QString::fromStdString( (varDir / "input_reference_list.txt").str() );
	QString errmsg;
	if (!m_availableInputVariables->parseVariableList(inputVarsFile, errmsg)) {
		return;
	}

	QString outputVarsFile = QString::fromStdString( (varDir / "output_reference_list.txt").str() );
	if (!m_availableOutputVariables->parseVariableList(outputVarsFile, errmsg)) {
		return;
	}

#if 0
	// now we set units and descriptions in input variables that match output variables
	for (NANDRAD::FMIVariableDefinition & var : m_availableInputVariables->variables()) {
		// lookup matching output variable by name
		for (std::vector<NANDRAD::FMIVariableDefinition>::const_iterator it = m_availableOutputVariables.begin();
			 it != m_availableOutputVariables.end(); ++it)
		{
			if (var.m_varName == it->m_varName) {
				var.m_unit = it->m_unit;
				var.m_fmiVarDescription = it->m_fmiVarDescription;
				break;
			}
		}
	}
#endif
	// now update set of used value references
	m_usedValueRefs.clear();
	m_usedValueRefs.insert(42); // reserve value ref for ResultsRootDir

	QMessageBox::information(this, tr("NANDRAD Test-init successful"),
							 tr("NANDRAD solver was started and the project was initialised, successfully. "
								"%1 FMU input-variables and %2 output variables available.")
							 .arg(m_availableInputVariables->rowCount()).arg(m_availableOutputVariables->rowCount()));

	updateFMUVariableTables();
}


void NandradFMUGeneratorWidget::updateFMUVariableTables() {
	// ok, we have:
	// - m_availableInputVariables - list of input variables to NANDRAD and default fmi variable names
	// - m_availableOutputVariables - list of output variables from NANDRAD and default fmi variable names
	// the variables in these lists have no value references yet (i.e. they are undefined)
	//
	// we also have:
	// - m_project.m_fmiDescription.m_inputVariables
	// - m_project.m_fmiDescription.m_outputVariables
	//
	// This is user data and can be wrong/invalid in all possible ways.
	// We are now optimistic and take each variable from the project and try to find a matching available
	// variable by only comparing the NANDRAD variable reference (ref type, object ID, var name).
	// If we do not find a match, this variable definition is bogus/outdated and marked as invalid.
	// If we did find a match, we must check for all other things that might go wrong. The m_availableXXX lists
	// serve as valid reference. Whenever a variable from the project has passed all checks, the m_availableXXX lists
	// are updated to the variable's contents and now have the merged state (available + valid definitions from project).
	//
	// Note: some basic checks on the project's fmi definitions have already been done when the project was read.
	//       At least we know that there are no invalid duplicate value references or FMI variable names around.

	// first process all defined input variables
	std::vector<NANDRAD::FMIVariableDefinition> invalidInputVars;
	std::vector<NANDRAD::FMIVariableDefinition> validInputVars;
	QStringList errors;
	for (NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_inputVariables) {

		// first check if there is already an *output* variable with same value reference
		// -> then we have an invalid definition
		bool invalid = false;
		for (const NANDRAD::FMIVariableDefinition & outVar : m_availableOutputVariables->variables()) {
			if (outVar.m_fmiValueRef != NANDRAD::INVALID_ID && outVar.m_fmiValueRef == var.m_fmiValueRef) {
				errors.append(tr("Duplicate FMI value ref %1 in project.").arg(outVar.m_fmiValueRef));
				invalid = true;
				break;
			}
		}
		if (invalid) {
			invalidInputVars.push_back(var);
			continue;
		}

		// lookup variable in available input variables
		std::vector<NANDRAD::FMIVariableDefinition>::const_iterator it = m_availableInputVariables->variables().begin();
		for (; it != m_availableInputVariables->variables().end(); ++it) {
			if (var.m_varName == it->m_varName &&
				var.m_objectId == it->m_objectId &&
				var.m_vectorIndex == it->m_vectorIndex)
			{
				break; // match found - stop search
			}
		}
		// no such NANDRAD variable found?
		if (it == m_availableInputVariables->variables().end()) {
			errors.append(tr("FMI input variable '%1' in project does not match any available NANDRAD input variables.")
						  .arg(QString::fromStdString(var.m_fmiVarName)) );
			invalidInputVars.push_back(var); // remember as invalid input var definition
			continue;
		}

		// found a matching NANDRAD variable, now we do all the other checks

		// check that the available input variable is not yet used (by a previous project FMI input variable)
		if (it->m_fmiValueRef != NANDRAD::INVALID_ID) {
			errors.append(tr("FMI input variable '%1' in project references a NANDRAD variable previously "
							 "referenced by FMI variable with value ref %2.").arg(QString::fromStdString(var.m_fmiVarName))
						  .arg(it->m_fmiValueRef));
			invalidInputVars.push_back(var); // remember as invalid input var definition
			continue;
		}

		// update m_availableInputVariables list with data from project

		// TODO : Fix this
//		it->m_fmiVarName = var.m_fmiVarName;
//		it->m_fmiValueRef = var.m_fmiValueRef; // setting a valid value ref marks this variable as used and valid
		m_usedValueRefs.insert(var.m_fmiValueRef); // remember value ref as used

		var.m_unit = it->m_unit;
		var.m_fmiVarDescription = it->m_fmiVarDescription;

		// remember this variable as valid
		validInputVars.push_back(var);
	}

	// now the same for outputs, but without allowing duplicate valueRefs
#if 0
	std::vector<NANDRAD::FMIVariableDefinition> invalidOutputVars;
	std::vector<NANDRAD::FMIVariableDefinition> validOutputVars;
	for (NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_outputVariables) {
		// lookup variable in available variables
		std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_availableOutputVariables.begin();
		for (; it != m_availableOutputVariables.end(); ++it) {
			if (var.m_varName == it->m_varName &&
				var.m_objectId == it->m_objectId &&
				var.m_vectorIndex == it->m_vectorIndex)
			{
				break; // match found - stop search
			}
		}
		// not found?
		if (it == m_availableOutputVariables.end()) {
			errors.append(tr("FMI input variable '%1' in project does not match any available NANDRAD input variables.")
						  .arg(QString::fromStdString(var.m_fmiVarName)) );
			invalidOutputVars.push_back(var); // remember as invalid input var definition
			continue;
		}

		// update m_availableInputVariables list with data from project
		it->m_fmiVarName = var.m_fmiVarName;
		it->m_fmiValueRef = var.m_fmiValueRef; // setting a valid value ref marks this variable as used and valid
		m_usedValueRefs.insert(var.m_fmiValueRef); // remember value ref as used

		var.m_unit = it->m_unit;
		var.m_fmiVarDescription = it->m_fmiVarDescription;
		// remember this variable as valid
		validOutputVars.push_back(var);
	}

	// now we keep only valid variables in project
	m_project.m_fmiDescription.m_inputVariables.swap(validInputVars);
	m_project.m_fmiDescription.m_outputVariables.swap(validOutputVars);
#endif

	dumpUsedValueRefs();

}




void NandradFMUGeneratorWidget::dumpUsedValueRefs() const {
#if 0
	qDebug() << "-----";
	for (unsigned int id : m_usedValueRefs)
		qDebug() << id;
#endif
}


bool  NandradFMUGeneratorWidget::generate() {
	FUNCID(NandradFMUGeneratorWidget::generate);

	QString fmuModelName = m_ui->lineEditModelName->text().trimmed();

	// generation process:
	// 1. create directory structure
	// 2. write modelDescription.xml
	// 3. copy dll/shared library files
	// 4. copy resources:
	//    - referenced climate data files/tsv-files (adjust file path to be local)
	//    - write project file
	//    - NANDRAD FMI logo image

	// zip directory structure

	// get target directory
	QString targetPath = m_ui->lineEditFMUPath->text();
	QDir baseDir = QtExt::Directories::tmpDir() + "/" + QFileInfo(targetPath).baseName();

	IBK::IBK_Message( IBK::FormatString("Composing FMU in temporary directory '%1'\n").arg(baseDir.absolutePath().toStdString()),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	// we create the following directory structure:
	// targetPath = /home/ghorwin/fmuexport/TestModel1.fmu
	// modelName = TestModel1
	//
	// zip-dir      = /<tmppath>/TestModel1
	//                /<tmppath>/TestModel1/modelDescription.xml
	//                /<tmppath>/TestModel1/Model.png
	//                /<tmppath>/TestModel1/binaries/...
	//                /<tmppath>/TestModel1/binaries/win64/TestModel1.dll
	//                /<tmppath>/TestModel1/binaries/win64/*.dll
	//                /<tmppath>/TestModel1/binaries/linux64/TestModel1.so
	//                /<tmppath>/TestModel1/binaries/darwin64/TestModel1.dylib
	//                /<tmppath>/TestModel1/resources/TestModel1.d6p
	//                /<tmppath>/TestModel1/resources/*
	// and we zip the directory such that /modelDescription.xml is in the root of the zip archive

	// remove generation directory if existing
	if (baseDir.exists()) {
		IBK::IBK_Message("Removing existing FMU export directory.", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QtExt::Directories::removeDirRecursively(baseDir.absolutePath());
	}
	// first create base directory
	baseDir.mkdir(baseDir.path());

	// now create subdirectories
	baseDir.mkdir("resources");

	NANDRAD::Project p = m_project;
	QString copyPath = baseDir.absoluteFilePath("resources");

	// if we have a target path, copy the referenced climate data file to the new location and modify the path
	IBK::Path resourcePath(copyPath.toStdString());
	IBK::Path fullClimatePath = p.m_location.m_climateFilePath.withReplacedPlaceholders(p.m_placeholders);
	if (!fullClimatePath.isFile()) {
		if (m_silent) {
			IBK_Message(IBK::FormatString("The referenced climate data file '%1' does not exist. Please select a climate data file!")
					.arg(fullClimatePath.str()), IBK::MSG_ERROR, FUNC_ID);
		}
		else {
			QMessageBox::critical(this, tr("FMU Export Error"),
				tr("The referenced climate data file '%1' does not exist. Please select a climate data file!")
					.arg(QString::fromStdString(fullClimatePath.str())) );
		}
		return false;
	}
	// target file path
	std::string targetFName = fullClimatePath.filename().str();
	targetFName = IBK::replace_string(targetFName, " ", "_");
	IBK::Path targetClimatePath = resourcePath / targetFName;
	IBK::IBK_Message( IBK::FormatString("Copying climate data file '%1' to '<fmu>/resources'\n").arg(fullClimatePath.filename()),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	if (!IBK::Path::copy(fullClimatePath, targetClimatePath))
		QMessageBox::critical(this, tr("FMU Export Error"),
			tr("Cannot copy the referenced climate data file '%1' to target directory '%2'.")
				.arg(QString::fromStdString(fullClimatePath.str()))
				.arg(QString::fromStdString(resourcePath.str())) );
	// modify reference in project file
	p.m_location.m_climateFilePath = "${Project Directory}/" + targetFName;

	// now all referenced files are stored alongside the project
	// remove not needed Database placeholder from placeholders list (but keep all custom placeholders!)
	auto it = p.m_placeholders.find("Database");
	if (it != p.m_placeholders.end())
		p.m_placeholders.erase(it);

	// now write the project into the export directory, it will always be called "project.nandrad"
	p.writeXML(resourcePath / "Project.nandrad");
	IBK::IBK_Message( IBK::FormatString("Creating 'Project.nandrad' in '<fmu>/resources'\n"),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);

	// generate the modelDescription.xml file

	// load template and replace variables
	IBK::Path fPath(":/modelDescription.xml.template");
	QFile f(fPath.c_str());
	f.open(QFile::ReadOnly);
	QTextStream strm(&f);

	QString modelDesc = strm.readAll();

	// ${MODELNAME}
	modelDesc.replace("${MODELNAME}", fmuModelName);

	// ${NANDRAD_VERSION}
	modelDesc.replace("${NANDRAD_VERSION}", NANDRAD::LONG_VERSION);

	// ${DATETIME} - 2018-08-01T12:49:19Z
	QDateTime t=QDateTime::currentDateTime();
	QString dt = t.toString(Qt::ISODate);
	modelDesc.replace("${DATETIME}", dt);

	// ${SIMDURATION} in seconds
	modelDesc.replace("${SIMDURATION}", QString("%1").arg(m_project.m_simulationParameter.m_interval.endTime(), 0, 'g', 10));

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

	// This set stores the value references of all exported input variables.
	// We do not export variables with same value ref and fmi var name, yet different NANDRAD variable twice.
	std::set<unsigned int> alreadyExportedInputVars;

	int index=1;
	for (std::vector<NANDRAD::FMIVariableDefinition>::const_iterator
		 varIt = m_project.m_fmiDescription.m_inputVariables.begin();
		 varIt != m_project.m_fmiDescription.m_inputVariables.end();
		 ++varIt, ++index)
	{
		// skip if variable with same valueref has been exported, already
		if (alreadyExportedInputVars.find(varIt->m_fmiValueRef) != alreadyExportedInputVars.end())
			continue;
		alreadyExportedInputVars.insert(varIt->m_fmiValueRef);

		const NANDRAD::FMIVariableDefinition & varDef = *varIt;
		QString varDesc;
		varDesc = INPUT_VAR_TEMPLATE;
		varDesc.replace("${INDEX}", QString("%1").arg(index));
		varDesc.replace("${NAME}", varDef.m_fmiVarName.c_str());
		varDesc.replace("${VALUEREF}", QString("%1").arg(varIt->m_fmiValueRef));
		varDesc.replace("${STARTVALUE}", QString::number(varIt->m_fmiStartValue));
		varDesc.replace("${REALVARUNIT}", varDef.m_unit.c_str());
		units.insert(varDef.m_unit.c_str());
		modelVariables += varDesc;
	}

	for (std::vector<NANDRAD::FMIVariableDefinition>::const_iterator
		 varIt = m_project.m_fmiDescription.m_outputVariables.begin();
		 varIt != m_project.m_fmiDescription.m_outputVariables.end();
		 ++varIt, ++index)
	{
		const NANDRAD::FMIVariableDefinition & varDef = *varIt;
		QString varDesc;
		varDesc = OUTPUT_VAR_TEMPLATE;
		varDesc.replace("${INDEX}", QString("%1").arg(index));
		varDesc.replace("${NAME}", varDef.m_fmiVarName.c_str());
		varDesc.replace("${VALUEREF}", QString("%1").arg(varIt->m_fmiValueRef));
		varDesc.replace("${STARTVALUE}", QString::number(varIt->m_fmiStartValue));
		varDesc.replace("${REALVARUNIT}", varDef.m_unit.c_str());
		units.insert(varDef.m_unit.c_str());
		modelVariables += varDesc;
		modelStructure += QString(" 			<Unknown index=\"%1\"/>\n").arg(index);
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
	QFile of(baseDir.absoluteFilePath("modelDescription.xml"));
	of.open(QFile::WriteOnly);
	of.write(modelDesc.toUtf8());
	of.close();

//	// create thumbnail image and copy into FMU
//	QString thumbPath = saveThumbNail();
//	QFile::copy(thumbPath, baseDir.absoluteFilePath("model.png"));


	bool success = true;

	baseDir.mkdir("binaries");
	baseDir.mkdir("binaries/linux64");
	baseDir.mkdir("binaries/win64");
	baseDir.mkdir("binaries/darwin64");

	// copy the binaries

	// linux
	QString fmuLibFile = m_installDir + "/libNandradSolverFMI.so";
	if (QFile(fmuLibFile).exists()) {
		QString targetPath = "binaries/linux64/" + fmuModelName + ".so";
		IBK::IBK_Message( IBK::FormatString("Copying Linux FMU lib '%1' to '%2'\n").arg(fmuLibFile.toStdString()).arg(targetPath.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QFile::copy(fmuLibFile, baseDir.absoluteFilePath(targetPath));
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (linux64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// macos
	fmuLibFile = m_installDir + "/libNandradSolverFMI.dylib";
	if (QFile(fmuLibFile).exists()) {
		QString targetPath = "binaries/darwin64/" + fmuModelName + ".dylib";
		IBK::IBK_Message( IBK::FormatString("Copying MacOS FMU lib '%1' to '%2'\n").arg(fmuLibFile.toStdString()).arg(targetPath.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QFile::copy(fmuLibFile, baseDir.absoluteFilePath(targetPath));
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (darwin64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// win64
	fmuLibFile = m_installDir + "/NandradSolverFMI.dll";
	if (QFile(fmuLibFile).exists()) {
		IBK::IBK_Message( IBK::FormatString("Copying Win64 FMU lib '%1'\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QString binTargetPath = baseDir.absoluteFilePath("binaries/win64/");
		QFile::copy(fmuLibFile, binTargetPath + "/" + fmuModelName + ".dll");

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
				QFile::copy(m_installDir + "/" + copyFiles[i], binTargetPath + "/" + QFileInfo(copyFiles[i]).fileName());
				IBK::IBK_Message( IBK::FormatString("Copying '%1' into FMU archive\n").arg(copyFiles[i].toStdString()),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			}
		}
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (Win64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	if (success) {

		// zip up the archive
		IBK::IBK_Message( IBK::FormatString("Compressing folder and creating FMU '%1'.\n").arg(targetPath.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		success = JlCompress::compressDir(targetPath, baseDir.absolutePath());
		if (!success) {
			if (m_silent) {
				IBK::IBK_Message(IBK::FormatString("Error compressing the FMU archive (maybe invalid target path or "
												   "invalid characters used?)."), IBK::MSG_ERROR, FUNC_ID);
			}
			else {
				QMessageBox::critical(this, tr("FMU Export Error"), tr("Error compressing the FMU archive (maybe invalid target path or invalid characters used?)."));
			}
			return false;
		}
	}

	// remove temporary directory structure
	QtExt::Directories::removeDirRecursively(baseDir.absolutePath());

	if (success) {
		if (!m_silent)
			QMessageBox::information(this, tr("FMU Export complete"), tr("FMU '%1' created.").arg(targetPath));
		return true;
	}
	else
		return false;
}





// **** static functions ****

struct VarInfo {
	VarInfo() = default;
	VarInfo(QString description, std::string unit, std::string fmuVarType) :
		m_description(description), m_unit(unit), m_fmuVarType(fmuVarType)
	{
	}

	QString			m_description;
	std::string		m_unit;
	std::string		m_fmuVarType;
};

static std::map<std::string, VarInfo> varInfo;

void NandradFMUGeneratorWidget::variableInfo(const std::string & fullVarName, QString & description, std::string & unit, std::string & fmuType) {
	// populate map on first call
	if (varInfo.empty()) {
		varInfo["Zone.AirTemperature"] = VarInfo(tr("Zone well-mixed air temperature"), "K", "Temperature");
		varInfo["Zone.WindowSolarRadiationFluxSum"] = VarInfo(tr("Sum of all short wave radiation fluxes across all windows of a zone (positive into zone)"), "W", "HeatFlux");
		varInfo["ConstructionInstance.FluxHeatConductionA"] = VarInfo(tr("Heat conduction flux across interface A (into construction)"), "W", "HeatFlux");
		varInfo["ConstructionInstance.FluxHeatConductionB"] = VarInfo(tr("Heat conduction flux across interface B (into construction)"), "W", "HeatFlux");
		varInfo["ConstructionInstance.FluxShortWaveRadiationA"] = VarInfo(tr("Short wave radiation flux across interface A (into construction)"), "W", "HeatFlux");
		varInfo["ConstructionInstance.FluxShortWaveRadiationB"] = VarInfo(tr("Short wave radiation flux across interface B (into construction)"), "W", "HeatFlux");
		varInfo["Location.Temperature"] = VarInfo(tr("Outside temperature"), "K", "Temperature");
	}

	const auto & it = varInfo.find(fullVarName);
	if (it != varInfo.end()) {
		description = it->second.m_description;
		unit = it->second.m_unit;
		fmuType = it->second.m_fmuVarType;
	}
}



void NandradFMUGeneratorWidget::on_tableViewOutputVars_doubleClicked(const QModelIndex &index) {

}
