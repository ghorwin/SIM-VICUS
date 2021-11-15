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
#include <QSortFilterProxyModel>

#include <QtExt_Directories.h>

#include <JlCompress.h>

#include <IBK_messages.h>

#include <NANDRAD_Constants.h>
#include <NANDRAD_Schedules.h>

#ifdef Q_OS_WIN
#undef UNICODE
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#include "FMUVariableTableModel.h"

const char * const ORGANIZATION = "IBK";
const char * const PROGRAM_NAME = "NANDRADFMUGenerator";

NandradFMUGeneratorWidget::NandradFMUGeneratorWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::NandradFMUGeneratorWidget),
	m_inputVariablesTableModel(new FMUVariableTableModel(this, true)),
	m_outputVariablesTableModel(new FMUVariableTableModel(this, false))
{
	m_ui->setupUi(this);

	qApp->setApplicationName(QString("NANDRAD FMU Generator %1").arg(NANDRAD::LONG_VERSION));
	qApp->setWindowIcon(QIcon(":/NandradIcon_64.png"));
	setWindowTitle(QString("NANDRAD FMU Generator %1").arg(NANDRAD::LONG_VERSION));

	m_ui->lineEditTargetDirectory->setup("", false, true, QString(), true);

	// configure models

	m_inputVariablesTableModel->m_availableVariables = &m_availableInputVariables;
	m_inputVariablesTableModel->m_displayNames = &m_displayNameTable;
	m_inputVariablesTableModel->setUseDisplayNames(m_ui->checkBoxUseDisplayNames->isChecked());
	m_outputVariablesTableModel->m_availableVariables = &m_availableOutputVariables;
	m_outputVariablesTableModel->m_displayNames = &m_displayNameTable;
	m_outputVariablesTableModel->setUseDisplayNames(m_ui->checkBoxUseDisplayNames->isChecked());

	// proxy models

	m_inputVariablesProxyModel = new QSortFilterProxyModel(this);
	m_inputVariablesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	m_outputVariablesProxyModel = new QSortFilterProxyModel(this);
	m_outputVariablesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	m_inputVariablesProxyModel->setSourceModel(m_inputVariablesTableModel);
	m_outputVariablesProxyModel->setSourceModel(m_outputVariablesTableModel);

	m_ui->tableViewInputVars->setModel(m_inputVariablesProxyModel);
	m_ui->tableViewOutputVars->setModel(m_outputVariablesProxyModel);

	connect(m_ui->tableViewInputVars->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &NandradFMUGeneratorWidget::onInputVarsSelectionChanged);

	connect(m_ui->tableViewOutputVars->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &NandradFMUGeneratorWidget::onOutputVarsSelectionChanged);

	QTableView * v = m_ui->tableViewInputVars;
	v->verticalHeader()->setDefaultSectionSize(19);
	v->verticalHeader()->setVisible(false);
	v->horizontalHeader()->setMinimumSectionSize(19);
	v->setSelectionBehavior(QAbstractItemView::SelectRows);
	v->setSelectionMode(QAbstractItemView::ExtendedSelection);
	v->setAlternatingRowColors(true);
	v->setSortingEnabled(true);
	v->sortByColumn(0, Qt::AscendingOrder);
	v->horizontalHeader()->setStretchLastSection(true);
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
	v->setSelectionMode(QAbstractItemView::ExtendedSelection);
	v->setAlternatingRowColors(true);
	v->setSortingEnabled(true);
	v->sortByColumn(0, Qt::AscendingOrder);
	v->setFont(f);
	v->horizontalHeader()->setFont(f); // Note: on Linux/Mac this won't work until Qt 5.11.1 - this was a bug between Qt 4.8...5.11.1
	v->horizontalHeader()->setStretchLastSection(true);

	m_inputVariablesTableModel->m_itemFont = f;
	m_outputVariablesTableModel->m_itemFont = f;

	m_ui->tabWidget->setCurrentIndex(0);

	// variable units/descriptions that are used as input vars, yet no matching output vars
	m_variableInfoMap["Zone.WindowSolarRadiationFluxSum"] = std::make_pair("W", "Sum of solar radiation loads through all windows");
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
		// when running in scripted mode, just run through all steps without user interaction
		if (!m_autoExportModelName.isEmpty()) {
			setup(); // in case of error, program is aborted here
			bool success = generate(); // in case of errors, error messages have been written here already
			if (!success)
				QApplication::exit(1); // exit with return code 1
			else
				QApplication::quit(); // exit with return code 0 = success
		}
		else {
			// setup user interface with project file data
			QTimer::singleShot(0, this, &NandradFMUGeneratorWidget::setup);
		}
	}
}


void NandradFMUGeneratorWidget::setup() {
	FUNCID(NandradFMUGeneratorWidget::setup);

	// clear entire data storage
	m_availableInputVariables.clear();
	m_availableOutputVariables.clear();
	m_inputVariablesTableModel->reset();
	m_outputVariablesTableModel->reset();

	// read NANDRAD project
	try {
		m_project = NANDRAD::Project();
		m_project.readXML(m_nandradFilePath);

		// also perform basic sanity checks on fmi definitions
		m_project.m_fmiDescription.checkParameters();
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		if (m_silent) {
			IBK::IBK_Message("Error reading NANDRAD project.", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			qApp->exit(1);
		}
		QMessageBox::critical(this, tr("Error reading NANDRAD project"),
							  tr("Reading of NANDRAD project file '%1' failed.").arg(QString::fromStdString(m_nandradFilePath.str())) );
		// disable all GUI elements
		setGUIState(false);
	}

	// store project file for next start of generator tool
	QSettings s(ORGANIZATION, PROGRAM_NAME);
	s.setValue("LastNANDRADProject", QString::fromStdString(m_nandradFilePath.str()) );

	setGUIState(true);

	// we set default FMU model name automatically if not yet specified
	if (m_project.m_fmiDescription.m_modelName.empty())
		m_project.m_fmiDescription.m_modelName = m_nandradFilePath.filename().withoutExtension().str();

	// initialize fmu export path from project file if still empty, or if in scripted mode
	if (!m_fmuExportDirectory.isValid() || !m_autoExportModelName.isEmpty())
		m_fmuExportDirectory = m_nandradFilePath.parentPath();

	// *** transfer general parameters

	m_ui->lineEditNandradProjectFilePath->setText( QString::fromStdString(m_nandradFilePath.str()) );
	if (!m_autoExportModelName.isEmpty())
		m_ui->lineEditModelName->setText( m_autoExportModelName );
	else
		m_ui->lineEditModelName->setText( QString::fromStdString(m_project.m_fmiDescription.m_modelName) );
	m_ui->lineEditTargetDirectory->setFilename( QString::fromStdString(m_fmuExportDirectory.str()) );
	// check correct FMU name and update target file path
	on_lineEditModelName_editingFinished();
	// now test-init the solver and update the variable tables

	// in case of error, function handles that appropriately: in scripted mode (m_silent == true),
	// the application is terminated with error message, otherwise a message box is shown and
	// ui state is disabled
	updateVariableLists();

	m_ui->tableViewInputVars->resizeColumnsToContents();
	m_ui->tableViewOutputVars->resizeColumnsToContents();
}


bool NandradFMUGeneratorWidget::renameInputVariable(unsigned int index, QString newVarName, bool autoAdjustName) {

	// Note: NANDRAD FMUs use structured variable naming, to "." is ok as variable name, like in "Office.AirTemperature"
	//
	// See FMI Standard 2.2.7: "name: The full, unique name of the variable. Every variable is uniquely identified within an
	//                          FMU instance by this name"
	//
	// Every FMU variable shall have a unique name. So we must test if the newly entered FMU name is anywhere used already.
	// However, we have different handling for output variables and input variables.
	//
	// Input variables:
	// (1) user must not select a variable name that is already used for another *output* variable; in such cases
	//     the function shall return show an error message and return false and the data shall not be modified.
	//
	// (2) if user has edited a variable with unique name and has now selected a name that is already used by another
	//     *input* variable, we expect:
	//   - the value reference assigned to the other, existing variable shall be set also to the freshly renamed variable,
	//     this both NANDRAD model variables share the same (single and unique) FMU input variable
	//   - if the existing other variable with same name has a different unit, the renaming is invalid, an error message
	//     shall be shown and the function returns with false (no data is modified)
	//
	//     Rational: in more complex models, an externally computed control parameter may be needed as input for many
	//               NANDRAD model objects. In such situations the FMU may only have one external FMI input variable
	//               that is, however, configured to be linked to several NANDRAD model object inputs. Hence, it must
	//               be possible to assign the same variable name and value reference to different NANDRAD variables in
	//               the table.
	//
	// (3) if user has edited a variable that shares the same name and value reference as another input variable,
	//     first the condition for (3) shall be checked and if matching, the steps for option (3) shall be followed.
	//     If the new variable name is unique, the FMI variable shall receive a new unique value reference.

	Q_ASSERT(index < m_availableInputVariables.size());

	NANDRAD::FMIVariableDefinition &inputVar = m_availableInputVariables[index];
	unsigned int newVarRef = inputVar.m_fmiValueRef;

	// variable must have a valid FMI value reference already (may be adjusted below)
	Q_ASSERT(newVarRef != NANDRAD::INVALID_ID);

	// check name against existing output variable with same name (forbidden!)
	for (const NANDRAD::FMIVariableDefinition &otherVar : m_availableOutputVariables) {
		// skip not-yet-configured variables
		if (otherVar.m_fmiValueRef == NANDRAD::INVALID_ID)
			continue;
		// error: name already exists as configured output variable
		if (newVarName.toStdString() == otherVar.m_fmiVarName) {
			// if we can automatically adjust the name (when adding new variables)
			// generate a new unique name and continue with it
			if (autoAdjustName) {
				newVarName = generateUniqueVariableName(newVarName);
				break;
			}
			else {
				QMessageBox::critical(this, QString(),
					  tr("FMI variable name '%1' is already used by an output variable!")
						.arg(newVarName));
				return false;
			}
		}
	}

	// in case the variable was renamed above, the loop below is useless, but we keep it for simplicity.

	// check name against existing name of references
	for (unsigned int j = 0; j < m_availableInputVariables.size(); ++j) {
		// skip equal variable in list
		if (j == index)
			continue;

		const NANDRAD::FMIVariableDefinition &otherVar = m_availableInputVariables[j];
		// skip non-selected variables
		if (otherVar.m_fmiValueRef == NANDRAD::INVALID_ID)
			continue;
		// existing name: copy reference
		if (newVarName.toStdString() == otherVar.m_fmiVarName) {
			// check unit
			if (inputVar.m_unit != otherVar.m_unit) {
				if (autoAdjustName) {
					newVarName = generateUniqueVariableName(newVarName); // Mind: we keep our own/new value ref
					break;
				}

				// if units are not equal than convert into each other
				QMessageBox::critical(this, QString(),
					  tr("There is already a configured FMI variable with name '%1' and unit '%2', but the currently selected variable has unit '%3'.")
						.arg(newVarName, QString::fromStdString(otherVar.m_unit), QString::fromStdString(inputVar.m_unit)) );
				return false;
			}
			newVarRef = otherVar.m_fmiValueRef;
			break;
		}

		// We have a variable that used to share its name and value reference with another variable.
		// Now the name is different, and hence we need to assign it a new value reference.
		// Its current value reference still matches that of another variable -> this we check and if found
		// we generate a new value reference.
		if (inputVar.m_fmiValueRef == otherVar.m_fmiValueRef) {
			// create a new reference (use the highest value and count one)
			newVarRef = *(m_usedValueRefs.rbegin()) + 1;
		}
	}

	// rename variable
	inputVar.m_fmiVarName = newVarName.toStdString();

	// We handle two cases:
	// a) variable has been renamed to another variable with same name and the value refs are new equal
	//    -> remove old value ref from m_usedValueRefs container
	// b) variable has been renamed from a same-named variable and received a new unqiue ID
	//    -> add new ID to used value refs
	if (newVarRef != inputVar.m_fmiValueRef) {
		unsigned int oldVarRef = inputVar.m_fmiValueRef;
		// change id
		inputVar.m_fmiValueRef = newVarRef;
		// decide whether to remove an unused value reference from
		// usedValueRefs container
		removeUsedInputValueRef(index, oldVarRef);
		// add new value ref to container
		m_usedValueRefs.insert(newVarRef);
	}
	dumpUsedValueRefs();

	return true;
}



bool NandradFMUGeneratorWidget::renameOutputVariable(unsigned int index, const QString & newVarName, bool autoAdjustName) {
	// Note: NANDRAD FMUs use structured variable naming, to "." is ok as variable name, like in "Office.AirTemperature"
	//
	// See FMI Standard 2.2.7: "name: The full, unique name of the variable. Every variable is uniquely identified within an
	//                          FMU instance by this name"
	//
	// Every FMU variable shall have a unique name. So we must test if the newly entered FMU name is anywhere used already.
	// However, we have different handling for output variables and input variables.
	//
	// Output variables:
	// (1) user must not select a variable name that is already used for another output/input variable; in such cases
	//     the function shall return false and the data shall not be modified. If autoAdjustName is true, the variable
	//     will be automatically renamed to have a unique name.

	Q_ASSERT(index < m_availableOutputVariables.size());

	NANDRAD::FMIVariableDefinition &outputVar = m_availableOutputVariables[index];

	// variable must have a valid FMI value reference already
	Q_ASSERT(outputVar.m_fmiValueRef != NANDRAD::INVALID_ID);

	// create a vector with all currently available variables (outputs and inputs)
	// since we must check uniqueness against all existing variables
	std::vector<NANDRAD::FMIVariableDefinition> allVariables = m_availableOutputVariables;
	allVariables.insert(allVariables.end(), m_availableInputVariables.begin(), m_availableInputVariables.end());

	// process all variables and if an FMI variable name matches our "newVarName" break the loop.
	unsigned int j=0;
	for (; j<allVariables.size(); ++j) {
		// skip equal variable (our output variables are first in the joined vector, and hence the row index matches
		// our variables row index)
		if (j == index)
			continue;

		const NANDRAD::FMIVariableDefinition &otherVar = allVariables[j];

		// if the variable has been configured already and the name matches, break
		if (otherVar.m_fmiValueRef != NANDRAD::INVALID_ID &&
			otherVar.m_fmiVarName == newVarName.toStdString())
		{
			break;
		}
	}

	// found another variable with the same name
	if (j != allVariables.size()) {
		// in case of "autoAdjustName", we simply generate a new unique variable name
		if (autoAdjustName) {
			QString adjustedNewVarName = generateUniqueVariableName(newVarName);
			// modify model accordingly
			outputVar.m_fmiVarName = adjustedNewVarName.toStdString();
		}
		else {
			// check if we have an input or output variable
			if (j < m_availableOutputVariables.size())
				QMessageBox::critical(this, QString(), tr("Output variable name '%1' already used as output variable. "
														  "Please rename other variable first!")
														  .arg(newVarName) );
			else
				QMessageBox::critical(this, QString(), tr("Output variable name '%1' already used as input variable. "
														  "Please rename other variable first!")
														  .arg(newVarName) );
			return false;
		}
	}
	else {
		// rename variable
		outputVar.m_fmiVarName = newVarName.toStdString();
	}

	dumpUsedValueRefs();

	return true;
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
	FUNCID(NandradFMUGeneratorWidget::on_lineEditModelName_editingFinished);
	m_ui->lineEditFMUPath->setText("---");
	if (!checkModelName()) {
		// in scripted mode, bail out
		if (m_silent) {
			IBK::IBK_Message("Invalid FMU model name.", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			qApp->exit(1);
		}
		return;
	}
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

	// store FMI variables inside NANDRAD project
	storeFMIVariables(m_project);
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
	if (!active) {
		// clear entire data storage for invalid state
		m_availableInputVariables.clear();  // TODO Stephan, add clear() function to model and wrap m_availableInputVariables.clear() in beginResetModel() and endResetModel()
		m_availableOutputVariables.clear();
		m_inputVariablesTableModel->reset();
		m_outputVariablesTableModel->reset();
	}
	// if active, all table widgets and push buttons are enabled, otherwise disabled
	m_ui->tabInputVariables->setEnabled(active);
	m_ui->tabOutputVariables->setEnabled(active);
	if (!active)
		m_ui->tabWidget->setCurrentIndex(0);
	m_ui->pushButtonGenerate->setEnabled(active);
	m_ui->pushButtonRefresh->setEnabled(active);
	m_ui->pushButtonSaveNandradProject->setEnabled(active);
	m_ui->lineEditModelName->setEnabled(active);
	m_ui->lineEditTargetDirectory->setEnabled(active);
	m_ui->lineEditFMUPath->setEnabled(active);
}


bool NandradFMUGeneratorWidget::checkModelName() {
	FUNCID(NandradFMUGeneratorWidget::checkModelName);
	QString modelName = m_ui->lineEditModelName->text().trimmed();
	if (modelName.isEmpty()) {
		if (m_silent) {
			IBK::IBK_Message("Empty model name is not allowed.", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			qApp->exit(1);
		}
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
		if (m_silent) {
			IBK::IBK_Message("Model name contains invalid characters.", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			qApp->exit(1);
		}
		QMessageBox::critical(this, QString(), tr("Model name contains invalid characters."));
		return false;
	}

	// check leading 0
	if (modelName[0] >= '0' && modelName[0] <= '9') {
		if (m_silent) {
			IBK::IBK_Message("Model name must not start with a number character.", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			qApp->exit(1);
		}
		QMessageBox::critical(this, QString(), tr("Model name must not start with a number character."));
		return false;
	}

	if (m_ui->lineEditTargetDirectory->filename().trimmed().isEmpty()) {
		if (m_silent) {
			IBK::IBK_Message("NANDRAD project file should be specified will full path.", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			qApp->exit(1);
		}
		QMessageBox::critical(this, QString(), tr("Missing target path name."));
		return false;
	}

	return true;
}


void NandradFMUGeneratorWidget::updateVariableLists() {
	FUNCID(NandradFMUGeneratorWidget::updateVariableLists);
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

	// TODO : change to non-event-loop call for "scripted" execution

	connect(&proc, &QProcess::started, this, &NandradFMUGeneratorWidget::onProcessStarted);
#if QT_VERSION >= 0x050600
	connect(&proc, &QProcess::errorOccurred, this, &NandradFMUGeneratorWidget::onProcessErrorOccurred);
#endif
	proc.start();
	// start process
	bool success = proc.waitForStarted();
	if (!success) {
		if (m_silent) {
			IBK::IBK_Message(IBK::FormatString("Could not run solver '%1'").arg(solverExecutable.toStdString()), IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			qApp->exit(1);
		}
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(solverExecutable));
		setGUIState(false);
		return;
	}

	proc.waitForFinished();

	if (proc.exitStatus() == QProcess::NormalExit) {
		if (proc.exitCode() != 0) {
			if (m_silent) {
				IBK::IBK_Message("There were errors during project test-initialization. Please ensure that the NANDRAD project runs successfully!", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
				qApp->exit(1);
			}
			QMessageBox::critical(this, QString(), tr("There were errors during project test-initialization. Please ensure that the NANDRAD project runs successfully!"));
			setGUIState(false);
			return;
		}
	}

	// TODO : For extremely large simulation projects, the intialization itself may take more than 30 seconds, so
	//        we may add a progress indicator dialog


	QString nandradProjectFilePath = QString::fromStdString(m_nandradFilePath.str());
	// now parse the variable lists
	IBK::Path varDir(nandradProjectFilePath.toStdString());
	varDir = varDir.withoutExtension() / "var";

	m_availableInputVariables.clear();
	QString inputVarsFile = QString::fromStdString( (varDir / "input_reference_list.txt").str() );
	if (!parseVariableList(inputVarsFile, m_availableInputVariables)) {
		m_inputVariablesTableModel->reset();
		return;
	}

	m_availableOutputVariables.clear();
	QString outputVarsFile = QString::fromStdString( (varDir / "output_reference_list.txt").str() );
	if (!parseVariableList(outputVarsFile, m_availableOutputVariables)) {
		m_outputVariablesTableModel->reset();
		return;
	}


	// *** read substitution map file
	QFile substitutionFile(QString::fromStdString( (varDir / "objectref_substitutions.txt").str() ));
	if (!substitutionFile.open(QFile::ReadOnly)) {
		throw IBK::Exception(IBK::FormatString("Could not read file '%1'. Re-run solver initialization!")
							 .arg(substitutionFile.fileName().toStdString()), FUNC_ID);
	}

	// create maps for fast translation
	m_displayNameTable.clear();
	QStringList vars = QString(substitutionFile.readAll()).split('\n');
	for (QString &str: vars){
		if (str.isEmpty())
			continue;
		QStringList tokens = str.split('\t');
		// Mind: if file is broken, we may only have one token, but we need at least 2, so skip lines with fewer tokens
		if (tokens.count() < 2)
			continue;
		// remove "id=" from var name
		std::string name = tokens[0].toStdString();
		size_t pos = name.find("(id=");
		if (pos == std::string::npos)
			continue; // definition line is broken

		size_t pos2 = name.find(")", pos+1);

		if (pos2 == std::string::npos)
			continue; // definition line is broken
		std::string id = name.substr(pos+4, pos2-pos-4);
		unsigned int objectId;
		try {
			objectId = IBK::string2val<unsigned int>(id);
		} catch (...) {
			continue; // definition line is broken
		}

		// remember substitution
		FMUVariableTableModel::DisplayNameSubstitution subst;
		subst.m_objectId = objectId;
		subst.m_modelType = name.substr(0, pos);
		subst.m_displayName = tokens[1].toStdString();
		m_displayNameTable.push_back(subst);
	}


	// now we set units and descriptions in input variables that match output variables
	for (NANDRAD::FMIVariableDefinition & var : m_availableInputVariables) {
		// lookup matching output variable by name
		std::vector<NANDRAD::FMIVariableDefinition>::const_iterator it = m_availableOutputVariables.begin();
		for (; it != m_availableOutputVariables.end(); ++it) {
			if (var.m_varName == it->m_varName) {
				var.m_unit = it->m_unit;
				var.m_fmiVarDescription = it->m_fmiVarDescription;
				break;
			}
		}
		if (it == m_availableOutputVariables.end()) {
			std::map<std::string, std::pair<std::string, std::string> >::const_iterator varInfoIt = m_variableInfoMap.find(var.m_varName);
			if (varInfoIt != m_variableInfoMap.end()) {
				var.m_unit = varInfoIt->second.first;
				var.m_fmiVarDescription = varInfoIt->second.second;
			}
		}
		// if we haven't set a unit yet, we set it to "-" as we need one when saving the variable
		// (otherwise we get an error when reading the same project because unit is required)
		if (var.m_unit.empty())
			var.m_unit = "-";
	}

	if (!m_silent)
		QMessageBox::information(this, tr("NANDRAD Test-init successful"),
							 tr("NANDRAD solver was started and the project was initialised, successfully. "
								"%1 FMU input-variables and %2 output variables available.")
							 .arg(m_availableInputVariables.size()).arg(m_availableOutputVariables.size()));

	updateFMUVariableTables();

	m_inputVariablesTableModel->reset();
	m_outputVariablesTableModel->reset();

	// initially, there is no selection in neither table views, hence we deactivate tool buttons
	m_ui->toolButtonAddInputVariable->setEnabled(false);
	m_ui->toolButtonRemoveInputVariable->setEnabled(false);
	m_ui->toolButtonAddOutputVariable->setEnabled(false);
	m_ui->toolButtonRemoveOutputVariable->setEnabled(false);
}


bool NandradFMUGeneratorWidget::parseVariableList(const QString & varsFile,
												  std::vector<NANDRAD::FMIVariableDefinition> & modelVariables)
{
	FUNCID(NandradFMUGeneratorWidget::parseVariableList);
	QFile inputVarF(varsFile);
	if (!inputVarF.open(QFile::ReadOnly)) {
		throw IBK::Exception(IBK::FormatString("Could not read file '%1'. Re-run solver initialization!")
							 .arg(varsFile.toStdString()), FUNC_ID);
	}

	QStringList vars = QString(inputVarF.readAll()).split('\n');

	//	Parse variable definitions as in the following lines:
	//
	//	Model.VentilationHeatFlux                         	1                   	1,2,3
	//	Zone.AirTemperature                               	1,2,3
	//
	// or in the output ref list file
	//
	//	Model.VentilationHeatFlux                         	1                   	2,3                 	W         	Natural ventilation/infiltration heat flux
	//	Zone.AirTemperature                               	1,2,3               	                    	C         	Room air temperature.

	// we process all but first line
	for (int j=1; j<vars.count(); ++j) {
		if (vars[j].trimmed().isEmpty())
			continue; // skip (trailing) empty lines)
		// Note: vars[j] must not be trimmed before calling split, since we may have several trailing \t which are important!
		QStringList tokens = vars[j].split('\t');
		if (tokens.count() < 3) {
			if (m_silent) {
				IBK::IBK_Message(IBK::FormatString("Invalid data in file '%1'. Re-run solver initialization!")
												   .arg(varsFile.toStdString()),
								 IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
				qApp->exit(1);
			}
			QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Re-run solver initialization!")
								  .arg(varsFile));
			setGUIState(false);
			return false;
		}

		// extract all the data we need from the strings
		QStringList varNameTokens = tokens[0].trimmed().split(".");
		if (varNameTokens.count() != 2) {
			if (m_silent) {
				IBK::IBK_Message(IBK::FormatString("Invalid data in file '%1'. Malformed variable name '%2'. Re-run solver initialization!")
												   .arg(varsFile.toStdString()).arg(tokens[0].toStdString()),
								 IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
				qApp->exit(1);
			}
			QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Malformed variable name '%2'. Re-run solver initialization!")
								  .arg(varsFile).arg(tokens[0]));
			setGUIState(false);
			return false;
		}
		QString objTypeName = varNameTokens[0];			// "Zone"
		QString nandradVarName = varNameTokens[1];		// "AirTemperature"
		QString unit;
		if (tokens.count() > 3) {
			unit = tokens[3].trimmed();
			try {
				// convert to base SI unit, except for some commonly used units without conversion factor to base unit
				if (unit != "W" &&
						unit != "W/m2" &&
						unit != "W/m3")
				{
					IBK::Unit u(unit.toStdString());
					unit = QString::fromStdString(u.base_unit().name());
				}
			}
			catch (...) {
				if (m_silent) {
					IBK::IBK_Message(IBK::FormatString("Invalid data in file '%1'. Unrecognized unit '%2'. Re-run solver initialization!")
													   .arg(varsFile.toStdString()).arg(unit.toStdString()),
									 IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
					qApp->exit(1);
				}
				QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Unrecognized unit '%2'. Re-run solver initialization!")
									  .arg(varsFile).arg(unit));
				setGUIState(false);
				return false;
			}
		}
		QString description;
		if (tokens.count() > 4)
			description = tokens[4].trimmed();


		// split object IDs and vector-value IDs
		std::vector<unsigned int>	m_objectIDs;
		std::vector<unsigned int>	m_vectorIndexes;
		QString idString = tokens[1].trimmed();
		if (idString.isEmpty()) {
			if (m_silent) {
				IBK::IBK_Message(IBK::FormatString("Invalid data in file '%1'. Object ID required for variable '%2'. Re-run solver initialization!")
												   .arg(varsFile.toStdString()).arg(tokens[0].toStdString()),
								 IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
				qApp->exit(1);
			}
			QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Object ID required for variable '%2'. Re-run solver initialization!")
								  .arg(varsFile).arg(tokens[0]));
			setGUIState(false);
			return false;
		}
		QStringList ids = idString.split(",");
		for (QString idstr : ids)
			m_objectIDs.push_back( idstr.toUInt());


		idString = tokens[2].trimmed();
		if (!idString.isEmpty()) { // empty column with vector indexes is ok for scalar results
			QStringList ids = idString.split(",");
			for (QString idstr : ids)
				m_vectorIndexes.push_back( idstr.toUInt());
		}

		// generate a variable for each combination of object ID and vector reference
		for (unsigned int objID : m_objectIDs) {

			if (m_vectorIndexes.empty()) {
				NANDRAD::FMIVariableDefinition varDef;
				varDef.m_varName = tokens[0].trimmed().toStdString(); // "Zone.AirTemperature"
				// compose a variable name
				varDef.m_fmiVarName = QString("%1(%2).%3")
						.arg(objTypeName).arg(objID).arg(nandradVarName)
						.toStdString();
				varDef.m_objectId = objID;
				varDef.m_vectorIndex = NANDRAD::INVALID_ID;
				varDef.m_fmiTypeName = ""; // TODO : how to determine the correct type?
				varDef.m_unit = unit.toStdString();
				varDef.m_fmiVarDescription = description.toStdString();
				varDef.m_fmiValueRef = NANDRAD::INVALID_ID; // will be set from either existing var in project or when configured
				// set default start value
				varDef.m_fmiStartValue = 0;
				if (varDef.m_unit == "K")				varDef.m_fmiStartValue = 293.15;
				else if (varDef.m_unit == "Pa")			varDef.m_fmiStartValue = 101325;

				modelVariables.push_back(varDef);
			}
			else {
				for (unsigned int vecIdx : m_vectorIndexes) {
					NANDRAD::FMIVariableDefinition varDef;
					varDef.m_varName = tokens[0].trimmed().toStdString(); // "Zone.AirTemperature"
					varDef.m_fmiVarName = QString("%1(%2).%3(%4)")
							.arg(objTypeName).arg(objID).arg(nandradVarName).arg(vecIdx)
							.toStdString();
					varDef.m_objectId = objID;
					varDef.m_vectorIndex = vecIdx;
					varDef.m_fmiTypeName = ""; // TODO : how to determine the correct type?
					varDef.m_unit = unit.toStdString();
					varDef.m_fmiVarDescription = description.toStdString();
					varDef.m_fmiValueRef = NANDRAD::INVALID_ID; // will be set from either existing var in project or when configured
					// set default start value
					varDef.m_fmiStartValue = 0;
					if (varDef.m_unit == "K")				varDef.m_fmiStartValue = 293.15;
					else if (varDef.m_unit == "Pa")			varDef.m_fmiStartValue = 101325;

					modelVariables.push_back(varDef);
				}

			}
		}
	}
	return true;
}


void NandradFMUGeneratorWidget::updateFMUVariableTables() {
	FUNCID(NandradFMUGeneratorWidget::updateFMUVariableTables);

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

	// now update set of used value references
	m_usedValueRefs.clear();
	m_usedValueRefs.insert(42); // reserve value ref for ResultsRootDir

	QStringList errors;
	for (NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_inputVariables) {

		// first check if there is already an *output* variable with same value reference
		// -> then we have an invalid definition
		bool invalid = false;
		for (const NANDRAD::FMIVariableDefinition & outVar : m_availableOutputVariables) {
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
		std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_availableInputVariables.begin();
		for (; it != m_availableInputVariables.end(); ++it) {
			if (var.m_varName == it->m_varName &&
					var.m_objectId == it->m_objectId &&
					var.m_vectorIndex == it->m_vectorIndex)
			{
				break; // match found - stop search
			}
		}
		// no such NANDRAD variable found?
		if (it == m_availableInputVariables.end()) {
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
		it->m_fmiVarName = var.m_fmiVarName;
		it->m_fmiValueRef = var.m_fmiValueRef; // setting a valid value ref marks this variable as used and valid
		m_usedValueRefs.insert(var.m_fmiValueRef); // remember value ref as used

		var.m_unit = it->m_unit;
		var.m_fmiVarDescription = it->m_fmiVarDescription;

		// remember this variable as valid
		validInputVars.push_back(var);
	}

	// now the same for outputs, but without allowing duplicate valueRefs

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

	if (!invalidInputVars.empty() || !invalidOutputVars.empty()) {
		IBK::IBK_Message("There are invalid FMI variable definitions in the project:\n", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
		if (!invalidInputVars.empty()) {
			IBK::IBK_Message("Invalid input variables:\n", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			for (const NANDRAD::FMIVariableDefinition & var : invalidInputVars)
				IBK::IBK_Message(IBK::FormatString("  %1 [valueRef=%2]").arg(var.m_varName).arg(var.m_fmiValueRef), IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
		}
		if (!invalidOutputVars.empty()) {
			IBK::IBK_Message("Invalid output variables:\n", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			for (const NANDRAD::FMIVariableDefinition & var : invalidOutputVars)
				IBK::IBK_Message(IBK::FormatString("  %1 [valueRef=%2]").arg(var.m_varName).arg(var.m_fmiValueRef), IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
		}
		if (!errors.empty()){
			IBK::IBK_Message("\nProblems:\n", IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			for (const QString &err: errors)
				IBK::IBK_Message(err.toStdString(), IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
		}
		if (m_silent)
			qApp->exit(1);
	}

	dumpUsedValueRefs();
}


void NandradFMUGeneratorWidget::dumpUsedValueRefs() const {
#if 0
	qDebug() << "-----";
	for (unsigned int id : m_usedValueRefs)
		qDebug() << id;
#endif
}


void NandradFMUGeneratorWidget::addVariable(bool inputVar) {
	// initialize variables
	QTableView * variableTableView								= inputVar ? m_ui->tableViewInputVars : m_ui->tableViewOutputVars;
	FMUVariableTableModel * varModel							= inputVar ? m_inputVariablesTableModel : m_outputVariablesTableModel;
	std::vector<NANDRAD::FMIVariableDefinition> & availableVars	= inputVar ? m_availableInputVariables : m_availableOutputVariables;
	QSortFilterProxyModel * proxyModel							= inputVar ? m_inputVariablesProxyModel: m_outputVariablesProxyModel;

	for (const QModelIndex & proxyIndex: variableTableView->selectionModel()->selectedRows()) {

		// configure new input var - requires valid selection
		Q_ASSERT(proxyIndex.isValid());

		// map 2 source index
		QModelIndex srcIndex = proxyModel->mapToSource(proxyIndex);

		unsigned int row = (unsigned int)srcIndex.row();
		Q_ASSERT(row < availableVars.size());
		NANDRAD::FMIVariableDefinition & var = availableVars[row];
		unsigned int valRef = var.m_fmiValueRef;

		// if the value reference has already been used then we have a mixed selection
		// where one selected item has valRef already, so we skip this one
		if (std::find(m_usedValueRefs.begin(), m_usedValueRefs.end(), valRef) != m_usedValueRefs.end())
			continue;

		Q_ASSERT(valRef == NANDRAD::INVALID_ID);

		// transfer default generated fmi variable name
		// Mind: we need to request the default-fmi-variable from the model, *before* we set
		//       a valid value reference. The model recognizes already configured variables
		//       by a value reference and does no longer to variable substitutions there.
		QModelIndex fmiVarIndex = varModel->index((int)row, 4);
		var.m_fmiVarName = fmiVarIndex.data().toString().toStdString();

		// generate a new unique value reference for the newly configured variable
		// Note: may be adjusted again in renameXXXVariable() call below, if in case of input variable
		//       an FMI variable with same name exists already
		var.m_fmiValueRef = (*m_usedValueRefs.rbegin()) + 1;
		// also insert the new value reference to list of used value refs
		m_usedValueRefs.insert(var.m_fmiValueRef);


		// When we configure the variable, we must check against other variables with the same name.
		// Unfortunately, this may give rise to a usability problem. Suppose I want to configure
		// a variable with pre-defined FMI input name "Zone(1).Temperature", but a user had already
		// renamed another input variable to this name. Then, we cannot configure this variable without
		// violating the "unique naming" convention.
		// Instead, we automatically adjust the pre-defined name and thus generate a new unique name.

		// try to use a central naming management
		if (inputVar) {
			renameInputVariable(row, QString::fromStdString(var.m_fmiVarName), true);
			// Note: if due to same naming the value reference was modified, the set m_usedValueRefs is modified
			//       inside renameInputVariable() already
		}
		else {
			renameOutputVariable(row, QString::fromStdString(var.m_fmiVarName), true);
		}

		dumpUsedValueRefs();

		// now inform model that it can tell the table view of its modifications
		varModel->variableModified(row); // we pass the source row
	}

	// change of model does not invalidate selection -> hence, currentChanged() signal is not emitted and
	// button state needs to be updated, manually
	if (inputVar) {
		m_ui->toolButtonAddInputVariable->setEnabled(false);
		m_ui->toolButtonRemoveInputVariable->setEnabled(true);
	}
	else {
		m_ui->toolButtonAddOutputVariable->setEnabled(false);
		m_ui->toolButtonRemoveOutputVariable->setEnabled(true);
	}
}


void NandradFMUGeneratorWidget::removeVariable(bool inputVar) {
	// initialize variables
	QTableView * variableTableView								= inputVar ? m_ui->tableViewInputVars : m_ui->tableViewOutputVars;
	FMUVariableTableModel * varModel							= inputVar ? m_inputVariablesTableModel : m_outputVariablesTableModel;
	std::vector<NANDRAD::FMIVariableDefinition> & availableVars	= inputVar ? m_availableInputVariables : m_availableOutputVariables;
	QSortFilterProxyModel * proxyModel							= inputVar ? m_inputVariablesProxyModel: m_outputVariablesProxyModel;

	for (const QModelIndex & proxyIndex: variableTableView->selectionModel()->selectedRows()){

		// configure new input var - requires valid selection
		Q_ASSERT(proxyIndex.isValid());

		// map 2 source index
		QModelIndex srcIndex = proxyModel->mapToSource(proxyIndex);

		unsigned int row = (unsigned int)srcIndex.row();
		Q_ASSERT(row < availableVars.size());
		NANDRAD::FMIVariableDefinition & var = availableVars[row];
		unsigned int valRef = var.m_fmiValueRef;

		// we might have a mixed selection where one item has no valRef yet, so we skip this one
		if (valRef == NANDRAD::INVALID_ID)
			continue;

		// remove value reference from set of used value references
		if (inputVar)
			// decide whether to remove an unused value reference from
			// usedValueRefs container
			removeUsedInputValueRef(row, valRef);
		else
			m_usedValueRefs.erase(valRef);

		// clear the value reference there
		var.m_fmiValueRef = NANDRAD::INVALID_ID;

		dumpUsedValueRefs();

		// now inform model that it can tell the table view of its modifications
		varModel->variableModified(row); // we pass the source row
	}

	if (inputVar) {
		m_ui->toolButtonAddInputVariable->setEnabled(true);
		m_ui->toolButtonRemoveInputVariable->setEnabled(false);
	}
	else {
		m_ui->toolButtonAddOutputVariable->setEnabled(true);
		m_ui->toolButtonRemoveOutputVariable->setEnabled(false);
	}

}


void NandradFMUGeneratorWidget::storeFMIVariables(NANDRAD::Project & prj) {

	// clear fmi description inside NANDRAD::Project
	prj.m_fmiDescription.m_inputVariables.clear();
	prj.m_fmiDescription.m_outputVariables.clear();

	// rewrite model input variables
	// (all available input variables with valid id)
	for( const NANDRAD::FMIVariableDefinition &inputVar : m_availableInputVariables) {
		// skip inactive variables
		if (inputVar.m_fmiValueRef == NANDRAD::INVALID_ID)
			continue;
		// add to fmi description
		prj.m_fmiDescription.m_inputVariables.push_back(inputVar);
	}
	// rewrite model output variables
	// (all available output variables with valid id)
	for( const NANDRAD::FMIVariableDefinition &outputVar : m_availableOutputVariables) {
		// skip inactive variables
		if (outputVar.m_fmiValueRef == NANDRAD::INVALID_ID)
			continue;
		// add to fmi description
		prj.m_fmiDescription.m_outputVariables.push_back(outputVar);
	}
}


void NandradFMUGeneratorWidget::removeUsedInputValueRef(unsigned int index, unsigned int fmiVarRef) {
	// for invalid references we need to do nothing
	Q_ASSERT(fmiVarRef != NANDRAD::INVALID_ID);

	Q_ASSERT(index < m_availableInputVariables.size());

	unsigned int j = 0;
	for (; j < m_availableInputVariables.size(); ++j) {
		// skip current variable
		if(j == index)
			continue;
		const NANDRAD::FMIVariableDefinition &otherVar = m_availableInputVariables[j];
		// skip invalid references
		if (otherVar.m_fmiValueRef == NANDRAD::INVALID_ID)
			continue;
		// variable reference is used by another quantity
		if (otherVar.m_fmiValueRef == fmiVarRef)
			break;
	}

	// remove variable reference
	if (j == m_availableInputVariables.size())
		m_usedValueRefs.erase(fmiVarRef);
}


QString NandradFMUGeneratorWidget::generateUniqueVariableName(const QString & suggestedName) const {
	std::set<QString> names;
	for (const NANDRAD::FMIVariableDefinition & var : m_availableInputVariables)
		names.insert(QString::fromStdString(var.m_fmiVarName));
	for (const NANDRAD::FMIVariableDefinition & var : m_availableOutputVariables)
		names.insert(QString::fromStdString(var.m_fmiVarName));

	QString name;
	unsigned int counter = 1;
	do {
		// compose new name based on pattern "basename_<nr>"
		name = suggestedName + QString("_%1").arg(++counter);
	}
	while (names.find(name) != names.end());
	return name;
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

	// copy FMI variables to resource project file
	storeFMIVariables(p);
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
	if (!IBK::Path::copy(fullClimatePath, targetClimatePath)) {
		if (m_silent) {
			IBK_Message(IBK::FormatString("Cannot copy the referenced climate data file '%1' to target directory '%2'.")
						.arg(fullClimatePath.str()).arg(resourcePath.str()), IBK::MSG_ERROR, FUNC_ID);
		}
		else {
			QMessageBox::critical(this, tr("FMU Export Error"),
								  tr("Cannot copy the referenced climate data file '%1' to target directory '%2'.")
								  .arg(QString::fromStdString(fullClimatePath.str()))
								  .arg(QString::fromStdString(resourcePath.str())) );
		}
		return false;
	}
	// modify reference in project file
	p.m_location.m_climateFilePath = "${Project Directory}/" + targetFName;

	// copy used resources/tsv files

	// schedules with tsv-files
	for (std::map<std::string, std::vector<NANDRAD::LinearSplineParameter> >::iterator
		 it = p.m_schedules.m_annualSchedules.begin();
		 it != p.m_schedules.m_annualSchedules.end(); ++ it) {
		// TODO: skip FMI-substituted quantities

		for (NANDRAD::LinearSplineParameter & spline : it->second) {
			if (!spline.m_name.empty() && spline.m_tsvFile.isValid()) {
				// Mind: tsv file path may be relative path to project directory or elsewhere
				IBK::Path tsvFilePath = spline.m_tsvFile.withReplacedPlaceholders(p.m_placeholders);
				IBK::Path targetPath = resourcePath / tsvFilePath.filename();
				IBK::Path::copy(tsvFilePath, targetPath);
				// change tsv file to point to relative path
				spline.m_tsvFile = IBK::Path("${Project Directory}/" + tsvFilePath.filename().str());
			}
		}
	}

	// network heatexchange spline data
	for (NANDRAD::HydraulicNetwork & n : p.m_hydraulicNetworks)
		for (NANDRAD::HydraulicNetworkElement & elem : n.m_elements) {
			for (int i=0; i<NANDRAD::HydraulicNetworkHeatExchange::NUM_SPL; ++i)
				if (!elem.m_heatExchange.m_splPara[i].m_name.empty() && elem.m_heatExchange.m_splPara[i].m_tsvFile.isValid()) {
					// Mind: tsv file path may be relative path to project directory or elsewhere
					IBK::Path tsvFilePath = elem.m_heatExchange.m_splPara[i].m_tsvFile.withReplacedPlaceholders(p.m_placeholders);
					IBK::Path targetPath = resourcePath / tsvFilePath.filename();
					IBK::Path::copy(tsvFilePath, targetPath);
					// change tsv file to point to relative path
					elem.m_heatExchange.m_splPara[i].m_tsvFile = IBK::Path("${Project Directory}/" + tsvFilePath.filename().str());
				}
		}


	// now all referenced files are stored alongside the project
	// remove not needed Database placeholder from placeholders list (but keep all custom placeholders!)
	auto it = p.m_placeholders.find("Database");
	if (it != p.m_placeholders.end())
		p.m_placeholders.erase(it);

	// now write the project into the export directory, it will always be called "Project.nandrad"
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
		if (m_silent)
			IBK::IBK_Message(IBK::FormatString("FMU created successfully.\n").arg(targetPath.toStdString()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		else
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


void NandradFMUGeneratorWidget::on_pushButtonRefresh_clicked() {

	QString fname = m_ui->lineEditNandradProjectFilePath->text();

	IBK::Path dir (fname.toStdString());
	// read NANDRAD project
	if (!dir.isValid()) {
		QMessageBox::critical(this, "Error in project file path", "Project file path is empty. Please enter a valid project file path.");
		m_ui->lineEditNandradProjectFilePath->setFocus();
		setGUIState(false);
		return; // dialog was cancelled
	}

	if (!dir.isFile()) {
		QMessageBox::critical(this, "Error in project file path", "Project file path does not specify an existing file. Please enter a valid project file path.");
		m_ui->lineEditNandradProjectFilePath->setFocus();
		setGUIState(false);
		return; // dialog was cancelled
	}

	m_nandradFilePath = dir;

	init();
}


void NandradFMUGeneratorWidget::on_tableViewInputVars_doubleClicked(const QModelIndex &index) {
	// double-click allows editing of variable name and value ref

	// depending on the state of the buttons, call either add or edit
	if (m_ui->toolButtonAddInputVariable->isEnabled())
		on_toolButtonAddInputVariable_clicked(); // add variable
	else if (m_ui->toolButtonRemoveInputVariable->isEnabled()) {
		// special case, if in column 4, do not disable variable, since double-click is needed for "edit" mode
		// Mind: the sort-filter-proxy model does not filter-out columns - so index is the same for source and proxy
		if (index.column() != 4)
			on_toolButtonRemoveInputVariable_clicked();
		// fall through - double-click is picked up by model which allows editing
	}
}


void NandradFMUGeneratorWidget::on_toolButtonAddInputVariable_clicked() {
	addVariable(true);
}


void NandradFMUGeneratorWidget::on_toolButtonRemoveInputVariable_clicked() {
	removeVariable(true);
}


void NandradFMUGeneratorWidget::onInputVarsSelectionChanged(const QItemSelection &, const QItemSelection &) {
	m_ui->toolButtonAddInputVariable->setEnabled(false);
	m_ui->toolButtonRemoveInputVariable->setEnabled(false);

	// process all selected items and check if they are all configured or all unconfigured
	bool configured = false;
	bool unconfigured = false;
	for (const QModelIndex & proxyIndex: m_ui->tableViewInputVars->selectionModel()->selectedRows()) {

		// configure new input var - requires valid selection
		Q_ASSERT(proxyIndex.isValid());
		unsigned int valueRef = proxyIndex.data(Qt::UserRole).toUInt();
		// already configured?
		if (valueRef == NANDRAD::INVALID_ID)
			unconfigured = true;
		else
			configured = true;
	}

	// any unconfigured variables?
	m_ui->toolButtonAddInputVariable->setEnabled(unconfigured); // not yet configured -> add button on
	// any already configured?
	m_ui->toolButtonRemoveInputVariable->setEnabled(configured); // already configured -> remove button on
}


void NandradFMUGeneratorWidget::on_tableViewOutputVars_doubleClicked(const QModelIndex & index) {
	// double-click allows editing of variable name and value ref

	// depending on the state of the buttons, call either add or edit
	if (m_ui->toolButtonAddOutputVariable->isEnabled())
		on_toolButtonAddOutputVariable_clicked(); // add variable
	else if (m_ui->toolButtonRemoveOutputVariable->isEnabled()) {
		// special case, if in column 4, do not disable variable, since double-click is needed for "edit" mode
		// Mind: the sort-filter-proxy model does not filter-out columns - so index is the same for source and proxy
		if (index.column() != 4)
			on_toolButtonRemoveOutputVariable_clicked();
		// fall through - double-click is picked up by model which allows editing
	}
}


void NandradFMUGeneratorWidget::on_toolButtonAddOutputVariable_clicked() {
	addVariable(false);
}


void NandradFMUGeneratorWidget::on_toolButtonRemoveOutputVariable_clicked() {
	removeVariable(false);
}


void NandradFMUGeneratorWidget::onOutputVarsSelectionChanged(const QItemSelection &, const QItemSelection &) {
	m_ui->toolButtonAddOutputVariable->setEnabled(false);
	m_ui->toolButtonRemoveOutputVariable->setEnabled(false);

	// process all selected items and check if they are all configured or all unconfigured
	bool configured = false;
	bool unconfigured = false;
	for (const QModelIndex & proxyIndex: m_ui->tableViewOutputVars->selectionModel()->selectedRows()){

		// configure new input var - requires valid selection
		Q_ASSERT(proxyIndex.isValid());
		unsigned int valueRef = proxyIndex.data(Qt::UserRole).toUInt();
		// already configured?
		if (valueRef == NANDRAD::INVALID_ID)
			unconfigured = true;
		else
			configured = true;
	}

	// any unconfigured variables?
	m_ui->toolButtonAddOutputVariable->setEnabled(unconfigured); // not yet configured -> add button on
	// any already configured?
	m_ui->toolButtonRemoveOutputVariable->setEnabled(configured); // already configured -> remove button on
}


void NandradFMUGeneratorWidget::on_lineEditInputVarNameFilter_textEdited(const QString &arg1) {
	m_ui->lineEditInputVarDescFilter->clear();
	m_inputVariablesProxyModel->setFilterWildcard(arg1);
	m_inputVariablesProxyModel->setFilterKeyColumn(0);
}


void NandradFMUGeneratorWidget::on_lineEditInputVarDescFilter_textEdited(const QString &arg1) {
	m_ui->lineEditInputVarNameFilter->clear();
	m_inputVariablesProxyModel->setFilterWildcard(arg1);
	m_inputVariablesProxyModel->setFilterKeyColumn(7);
}


void NandradFMUGeneratorWidget::on_lineEditOutputVarNameFilter_textEdited(const QString &arg1) {
	m_ui->lineEditOutputVarDescFilter->clear();
	m_outputVariablesProxyModel->setFilterWildcard(arg1);
	m_outputVariablesProxyModel->setFilterKeyColumn(0);
}


void NandradFMUGeneratorWidget::on_lineEditOutputVarDescFilter_textEdited(const QString &arg1) {
	m_ui->lineEditOutputVarNameFilter->clear();
	m_outputVariablesProxyModel->setFilterWildcard(arg1);
	m_outputVariablesProxyModel->setFilterKeyColumn(7);
}


void NandradFMUGeneratorWidget::on_checkBoxUseDisplayNames_clicked(bool checked) {
	m_inputVariablesTableModel->setUseDisplayNames(checked);
	m_outputVariablesTableModel->setUseDisplayNames(checked);
}
