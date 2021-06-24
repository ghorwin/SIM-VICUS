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

#ifdef Q_OS_WIN
#undef UNICODE
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

const char * const ORGANIZATION = "IBK";
const char * const PROGRAM_NAME = "NANDRADFMUGenerator";


bool startProcess(const QString & executable, QStringList commandLineArgs, const QString & projectFile) {
	// spawn process
#ifdef Q_OS_WIN

	/// \todo use wide-string version of API and/or encapsulate spawn process into a function

	// Use WinAPI to create a solver process
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	std::string utf8String = projectFile.toStdString().data();
	si.lpTitle = (char*)utf8String.c_str();
//	si.dwFlags = STARTF_USESHOWWINDOW;
//	si.wShowWindow = SW_SHOW;
	ZeroMemory( &pi, sizeof(pi) );
	const unsigned int lower_priority = 0x00004000;
	QString cmdLine = QString("\"%1\" %2 \"%3\"")
		.arg(executable)
		.arg(commandLineArgs.join(" "))
		.arg(projectFile);

	std::string cmd = cmdLine.toLatin1().data();
	// Start the child process.
	if( !CreateProcessA( NULL,   // No module name (use command line).
		&cmd[0], 				// Command line.
		NULL,             		// Process handle not inheritable.
		NULL,             		// Thread handle not inheritable.
		FALSE,            		// Set handle inheritance to FALSE.
		lower_priority,   		// Create with priority lower then normal.
		NULL,             		// Use parent's environment block.
		NULL,             		// Use parent's starting directory.
		&si,              		// Pointer to STARTUPINFO structure.
		&pi )             		// Pointer to PROCESS_INFORMATION structure.
	)
	{
		return false;
	}
	return true;

#else // Q_OS_WIN

	// append project file to arguments, no quotes needed, since Qt takes care of that
	commandLineArgs << projectFile;
//	qint64 pid;
	commandLineArgs = QStringList() << "-hold"
									<< "-fa" << "'Monospace'"
									<< "-fs" << "9"
									<< "-geometry" << "120x40" << "-e" << executable << commandLineArgs;
	QString terminalProgram = "xterm";

	QProcess proc;
	proc.setProgram(terminalProgram);
	proc.setArguments(commandLineArgs);

	proc.start();
	bool success = proc.waitForFinished();

//	success = QProcess::startDetached(terminalProgram, commandLineArgs, QString(), &pid);

	// TODO : do something with the process identifier... mayby check after a few seconds, if the process is still running?
	return success;

#endif // Q_OS_WIN
}


NandradFMUGeneratorWidget::NandradFMUGeneratorWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::NandradFMUGeneratorWidget)
{
	m_ui->setupUi(this);

	m_ui->lineEditTargetDirectory->setup("", false, true, QString());

	m_ui->tableWidgetInputVars->setColumnCount(8);
	m_ui->tableWidgetInputVars->setRowCount(0);

	m_ui->tableWidgetInputVars->setHorizontalHeaderLabels(QStringList()
			  << tr("NANDRAD Variable Name")
			  << tr("Object ID")
			  << tr("Vector value index/ID")
			  << tr("Unit")
			  << tr("FMI Variable Name")
			  << tr("FMI value reference")
			  << tr("FMI Type")
			  << tr("Description")
	);

	m_ui->tableWidgetInputVars->horizontalHeader()->setStretchLastSection(true);

	QTableView * v = m_ui->tableWidgetInputVars;
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

	m_ui->tableWidgetOutputVars->setColumnCount(8);
	m_ui->tableWidgetOutputVars->setHorizontalHeaderLabels(QStringList()
			   << tr("NANDRAD Variable Name")
			   << tr("Object ID")
			   << tr("Vector value index/ID")
			   << tr("Unit")
			   << tr("FMI Variable Name")
			   << tr("FMI value reference")
			   << tr("FMI Type")
			   << tr("Description")
	 );
	m_ui->tableWidgetOutputVars->horizontalHeader()->setStretchLastSection(true);
	v = m_ui->tableWidgetOutputVars;
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
		QApplication::exit(1); // exit with return code 1
	else
		QApplication::quit(); // exit with return code 0 = success
}


void NandradFMUGeneratorWidget::on_tableWidgetInputVars_currentCellChanged(int currentRow, int , int , int ) {
	m_ui->toolButtonAddInputVariable->setEnabled(false);
	m_ui->toolButtonRemoveInputVariable->setEnabled(false);
	if (currentRow == -1)
		return;
	unsigned int valueRef = m_availableInputVariables[(unsigned int)currentRow].m_fmiValueRef;
	// already configured?
	if (valueRef == NANDRAD::INVALID_ID)
		m_ui->toolButtonAddInputVariable->setEnabled(true); // not yet configured -> add button on
	else
		m_ui->toolButtonRemoveInputVariable->setEnabled(true); // already configured -> remove button on
}


void NandradFMUGeneratorWidget::on_tableWidgetOutputVars_currentCellChanged(int currentRow, int , int , int ) {
	m_ui->toolButtonAddOutputVariable->setEnabled(false);
	m_ui->toolButtonRemoveOutputVariable->setEnabled(false);
	if (currentRow == -1)
		return;
	unsigned int valueRef = m_availableOutputVariables[(unsigned int)currentRow].m_fmiValueRef;
	// already configured?
	if (valueRef == NANDRAD::INVALID_ID)
		m_ui->toolButtonAddOutputVariable->setEnabled(true); // not yet configured -> add button on
	else
		m_ui->toolButtonRemoveOutputVariable->setEnabled(true); // already configured -> remove button on
}


void NandradFMUGeneratorWidget::on_toolButtonAddInputVariable_clicked() {
	int row = m_ui->tableWidgetInputVars->currentRow();
	Q_ASSERT(row != -1);
	NANDRAD::FMIVariableDefinition & var = m_availableInputVariables[(unsigned int)row];
	unsigned int valRef = var.m_fmiValueRef;
	Q_ASSERT(valRef == NANDRAD::INVALID_ID); // must be a valid, unused reference

	// got it, now create a copy of the variable description, copy it to the project and assign a valid value reference
	unsigned int newValueRef = *m_usedValueRefs.rbegin() + 1;

	// check if there is already another configured FMI variable with same name
	unsigned int otherValueRef = 0;
	for (unsigned int j=0; j<m_availableInputVariables.size(); ++j) {
		if (row == (int)j) continue; // skip ourself
		if (m_availableInputVariables[j].m_fmiValueRef != NANDRAD::INVALID_ID &&
				m_availableInputVariables[j].m_fmiVarName == var.m_fmiVarName)
		{
			otherValueRef = m_availableInputVariables[j].m_fmiValueRef;
			break;
		}
	}
	if (otherValueRef != 0)
		newValueRef = otherValueRef;

	// assign and remember new fmiValueRef
	m_usedValueRefs.insert(newValueRef);
	var.m_fmiValueRef = newValueRef;

	// add variable definition to project
	m_project.m_fmiDescription.m_inputVariables.push_back(var);

	// set new value reference in table
	m_ui->tableWidgetInputVars->blockSignals(true);
	m_ui->tableWidgetInputVars->item(row, 5)->setText(QString("%1").arg(newValueRef));
	// now update appearance of table row
	QFont f(m_ui->tableWidgetInputVars->font());
	f.setBold(true);
	for (int i=0; i<8; ++i) {
		m_ui->tableWidgetInputVars->item(row, i)->setFont(f);
		m_ui->tableWidgetInputVars->item(row, i)->setTextColor(Qt::black);
	}
	m_ui->tableWidgetInputVars->blockSignals(false);
	on_tableWidgetInputVars_currentCellChanged(row,0,0,0);

	dumpUsedValueRefs();
}


void NandradFMUGeneratorWidget::on_toolButtonRemoveInputVariable_clicked() {
	int row = m_ui->tableWidgetInputVars->currentRow();
	Q_ASSERT(row != -1);

	NANDRAD::FMIVariableDefinition & var = m_availableInputVariables[(unsigned int)row];
	unsigned int valRef = var.m_fmiValueRef;
	bool used = false;
	for (unsigned int i=0; i<m_availableInputVariables.size(); ++i) {
		// skip our row
		if (row == (int)i)
			continue;
		if (m_availableInputVariables[i].m_fmiValueRef == valRef) {
			used = true;
			break;
		}
	}
	// remove value reference from set of used value references, unless it is used still by another input variable
	// (with same fmiVarName)
	if (!used)
		m_usedValueRefs.erase(valRef);

	// lookup existing definition in m_project and remove it there
	for (std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_project.m_fmiDescription.m_inputVariables.begin();
		 it != m_project.m_fmiDescription.m_inputVariables.end(); ++it)
	{
		if (var.m_objectId == it->m_objectId && var.m_vectorIndex == it->m_vectorIndex && var.m_varName == it->m_varName) {
			m_project.m_fmiDescription.m_inputVariables.erase(it);
			break;
		}
	}
	// lookup existing definition in m_availableInputVariables and clear the value reference there
	var.m_fmiValueRef = NANDRAD::INVALID_ID;
	// if valid, just clear item flags
	m_ui->tableWidgetInputVars->blockSignals(true);
	var.m_fmiValueRef = NANDRAD::INVALID_ID;
	m_ui->tableWidgetInputVars->item(row, 5)->setText("---");
	// now reset table row to uninitialized state
	QFont f(m_ui->tableWidgetInputVars->font());
	f.setItalic(true);
	for (int i=0; i<8; ++i) {
		m_ui->tableWidgetInputVars->item(row, i)->setFont(f);
		m_ui->tableWidgetInputVars->item(row, i)->setTextColor(Qt::gray);
	}
	m_ui->tableWidgetInputVars->blockSignals(false);
	on_tableWidgetInputVars_currentCellChanged(row,0,0,0);
	dumpUsedValueRefs();
}


void NandradFMUGeneratorWidget::on_tableWidgetInputVars_itemDoubleClicked(QTableWidgetItem * item) {
	// depending on the state of the buttons, call either add or remove
	if (m_ui->toolButtonAddInputVariable->isEnabled())
		m_ui->toolButtonAddInputVariable->click();
	else if (m_ui->toolButtonRemoveInputVariable->isEnabled()) {
		// special case, if in column 4, do not disable variable, since double-click is needed for "edit" mode
		if (item->column() != 4)
			m_ui->toolButtonRemoveInputVariable->click();
	}
}


void NandradFMUGeneratorWidget::on_toolButtonAddOutputVariable_clicked() {
	int row = m_ui->tableWidgetOutputVars->currentRow();
	Q_ASSERT(row != -1);
	NANDRAD::FMIVariableDefinition & var = m_availableOutputVariables[(unsigned int)row];
	unsigned int valRef = var.m_fmiValueRef;
	Q_ASSERT(valRef == NANDRAD::INVALID_ID); // must be a valid, unused reference

	// got it, now create a copy of the variable description, copy it to the project and assign a valid value reference
	unsigned int newValueRef = *m_usedValueRefs.rbegin() + 1;

	// assign and remember new fmiValueRef
	m_usedValueRefs.insert(newValueRef);
	var.m_fmiValueRef = newValueRef;

	// add variable definition to project
	m_project.m_fmiDescription.m_outputVariables.push_back(var);

	// set new value reference in table
	m_ui->tableWidgetOutputVars->blockSignals(true);
	m_ui->tableWidgetOutputVars->item(row, 5)->setText(QString("%1").arg(newValueRef));
	// now update appearance of table row
	QFont f(m_ui->tableWidgetOutputVars->font());
	f.setBold(true);
	for (int i=0; i<8; ++i) {
		m_ui->tableWidgetOutputVars->item(row, i)->setFont(f);
		m_ui->tableWidgetOutputVars->item(row, i)->setTextColor(Qt::black);
	}
	m_ui->tableWidgetOutputVars->blockSignals(false);
	on_tableWidgetOutputVars_currentCellChanged(row,0,0,0);

	dumpUsedValueRefs();
}


void NandradFMUGeneratorWidget::on_toolButtonRemoveOutputVariable_clicked() {
	int row = m_ui->tableWidgetOutputVars->currentRow();
	Q_ASSERT(row != -1);

	NANDRAD::FMIVariableDefinition & var = m_availableOutputVariables[(unsigned int)row];
	unsigned int valRef = var.m_fmiValueRef;
	// remove value reference from set of used value references
	m_usedValueRefs.erase(valRef);

	// lookup existing definition in m_project and remove it there
	for (std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_project.m_fmiDescription.m_outputVariables.begin();
		 it != m_project.m_fmiDescription.m_outputVariables.end(); ++it)
	{
		if (it->m_fmiVarName == var.m_fmiVarName) {
			m_project.m_fmiDescription.m_outputVariables.erase(it);
			break;
		}
	}
	// lookup existing definition in m_availableInputVariables and clear the value reference there
	var.m_fmiValueRef = NANDRAD::INVALID_ID;
	// if valid, just clear item flags
	m_ui->tableWidgetOutputVars->blockSignals(true);
	var.m_fmiValueRef = NANDRAD::INVALID_ID;
	m_ui->tableWidgetOutputVars->item(row, 5)->setText("---");
	// now reset table row to uninitialized state
	QFont f(m_ui->tableWidgetOutputVars->font());
	f.setItalic(true);
	for (int i=0; i<8; ++i) {
		m_ui->tableWidgetOutputVars->item(row, i)->setFont(f);
		m_ui->tableWidgetOutputVars->item(row, i)->setTextColor(Qt::gray);
	}
	m_ui->tableWidgetOutputVars->blockSignals(false);
	on_tableWidgetOutputVars_currentCellChanged(row,0,0,0);
	dumpUsedValueRefs();
}


void NandradFMUGeneratorWidget::on_tableWidgetOutputVars_itemDoubleClicked(QTableWidgetItem * item) {
	// depending on the state of the buttons, call either add or remove
	if (m_ui->toolButtonAddOutputVariable->isEnabled())
		m_ui->toolButtonAddOutputVariable->click();
	else if (m_ui->toolButtonRemoveOutputVariable->isEnabled()) {
		// special case, if in column 4, do not disable variable, since double-click is needed for "edit" mode
		if (item->column() != 4)
			m_ui->toolButtonRemoveOutputVariable->click();
	}
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

	m_availableInputVariables.clear();
	QString inputVarsFile = QString::fromStdString( (varDir / "input_reference_list.txt").str() );
	if (!parseVariableList(inputVarsFile, m_availableInputVariables))
		return;

	m_availableOutputVariables.clear();
	QString outputVarsFile = QString::fromStdString( (varDir / "output_reference_list.txt").str() );
	if (!parseVariableList(outputVarsFile, m_availableOutputVariables))
		return;

	// now we set units and descriptions in input variables that match output variables
	for (NANDRAD::FMIVariableDefinition & var : m_availableInputVariables) {
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

	// now update set of used value references
	m_usedValueRefs.clear();
	m_usedValueRefs.insert(42); // reserve value ref for ResultsRootDir

	QMessageBox::information(this, tr("NANDRAD Test-init successful"),
							 tr("NANDRAD solver was started and the project was initialised, successfully. "
								"%1 FMU input-variables and %2 output variables available.")
							 .arg(m_availableInputVariables.size()).arg(m_availableOutputVariables.size()));

	updateFMUVariableTables();
}


bool NandradFMUGeneratorWidget::parseVariableList(const QString & varsFile,
												  std::vector<NANDRAD::FMIVariableDefinition> & modelVariables)
{
	QFile inputVarF(varsFile);
	if (!inputVarF.open(QFile::ReadOnly)) {
			QMessageBox::critical(this, QString(), tr("Could not read file '%1'. Re-run solver initialization!")
								  .arg(varsFile));
		return false;
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
			QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Re-run solver initialization!")
				.arg(varsFile));
			return false;
		}

		// extract all the data we need from the strings
		QStringList varNameTokens = tokens[0].trimmed().split(".");
		if (varNameTokens.count() != 2) {
			QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Malformed variable name '%2'. Re-run solver initialization!")
				.arg(varsFile).arg(tokens[0]));
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
				QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Unrecognized unit '%2'. Re-run solver initialization!")
					.arg(varsFile).arg(unit));
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
			QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Object ID required for variable '%2'. Re-run solver initialization!")
				.arg(varsFile).arg(tokens[0]));
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

	dumpUsedValueRefs();

	// now populate the tables
	populateTable(m_ui->tableWidgetInputVars, m_availableInputVariables);
	populateTable(m_ui->tableWidgetOutputVars, m_availableOutputVariables);

	if (m_ui->tableWidgetInputVars->rowCount() != 0) {
		m_ui->tableWidgetInputVars->selectRow(0);
		m_ui->tableWidgetInputVars->scrollToItem(m_ui->tableWidgetInputVars->currentItem());
	}
	else
		m_ui->tableWidgetInputVars->selectRow(-1);

	if (m_ui->tableWidgetOutputVars->rowCount() != 0) {
		m_ui->tableWidgetOutputVars->selectRow(0);
		m_ui->tableWidgetOutputVars->scrollToItem(m_ui->tableWidgetOutputVars->currentItem());
	}
	else
		m_ui->tableWidgetOutputVars->selectRow(-1);
}


void NandradFMUGeneratorWidget::populateTable(QTableWidget * table,
											  const std::vector<NANDRAD::FMIVariableDefinition> & availableVars)
{
	table->setRowCount(0);
	table->setSortingEnabled(false); // disable sorting while we add rows
	// then add the valid variables
	for (const NANDRAD::FMIVariableDefinition & var : availableVars)
		appendVariableEntry(table, var);

	table->setSortingEnabled(false); // no sorting!
	table->resizeColumnsToContents();
}


void NandradFMUGeneratorWidget::appendVariableEntry(QTableWidget * tableWidget, const NANDRAD::FMIVariableDefinition & var) {
	tableWidget->blockSignals(true);

	// add new row
	int row = tableWidget->rowCount();
	tableWidget->setRowCount(row+1);

	QFont itemFont(tableWidget->font());
	QColor itemColor(Qt::black);
	if (var.m_fmiValueRef == NANDRAD::INVALID_ID) {
		itemFont.setItalic(true);
		itemColor = QColor(Qt::gray);
	}
	else {
		itemFont.setBold(true);
	}

	QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(var.m_varName));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	tableWidget->setItem(row, 0, item);


	item = new QTableWidgetItem(QString("%1").arg(var.m_objectId));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	tableWidget->setItem(row, 1, item);

	item = new QTableWidgetItem(QString("%1").arg(var.m_vectorIndex));
	if (var.m_vectorIndex == NANDRAD::INVALID_ID)
		item->setText(""); // no -1 display
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	tableWidget->setItem(row, 2, item);

	item = new QTableWidgetItem(QString::fromStdString(var.m_unit));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	tableWidget->setItem(row, 3, item);

	item = new QTableWidgetItem(QString::fromStdString(var.m_fmiVarName));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	tableWidget->setItem(row, 4, item);

	item = new QTableWidgetItem();
	if (var.m_fmiValueRef == NANDRAD::INVALID_ID)
		item->setText("---");
	else
		item->setText(QString("%1").arg(var.m_fmiValueRef));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	tableWidget->setItem(row, 5, item);

	item = new QTableWidgetItem(QString::fromStdString(var.m_fmiTypeName));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	tableWidget->setItem(row, 6, item);

	item = new QTableWidgetItem(QString::fromStdString(var.m_fmiVarDescription));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	tableWidget->setItem(row, 7, item);

	tableWidget->blockSignals(false);
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




void NandradFMUGeneratorWidget::on_tableWidgetInputVars_itemChanged(QTableWidgetItem *item) {
	// assert that we are in the correct column
	Q_ASSERT(item->column() == 4);
	// read newly entered name
	unsigned int row = (unsigned int)item->row();
	std::string newFMIName = item->text().trimmed().toStdString();
	NANDRAD::FMIVariableDefinition & var = m_availableInputVariables[row];
	std::string oldFMIName = var.m_fmiVarName;

	// check if this FMI variable name has been used somewhere already
	// if yes, unit must match and the variable must be selected
	// all variables with same FMI name must get the same valueReference

	// loop over all defined input variables
	unsigned int otherVarsValueRef = 0;
	for (unsigned int i=0; i<m_availableInputVariables.size(); ++i) {
		// skip ourselves
		if (item->row() == (int)i)
			continue;
		if (m_availableInputVariables[i].m_fmiVarName == newFMIName) {
			// found a duplicate, check unit
			if (m_availableInputVariables[i].m_unit != var.m_unit) {
				QMessageBox::critical(this, tr("Naming error"), tr("There is already another FMI variable with name '%1', "
																   "yet unit '%2', which differs from unit '%3' of currently selected variable.")
									  .arg(QString::fromStdString(newFMIName),
										   QString::fromStdString(m_availableInputVariables[i].m_unit),
										   QString::fromStdString(var.m_unit) ) );
				m_ui->tableWidgetInputVars->blockSignals(true);
				item->setText( QString::fromStdString(var.m_fmiVarName) );
				m_ui->tableWidgetInputVars->blockSignals(false);
				return;
			}
			// remember value ref of other variable (in case of several variables of same type should all have the same valueRef)
			otherVarsValueRef = m_availableInputVariables[i].m_fmiValueRef;
		}
	}
	// all ok, keep the variable name
	var.m_fmiVarName = newFMIName;

	// if we did find another variable, take its value reference
	if (otherVarsValueRef != 0) {
		// remove current variable's value ref from set of currently used value references
		m_usedValueRefs.erase(var.m_fmiValueRef);

		// store new value ref
		var.m_fmiValueRef = otherVarsValueRef;

		// update table cell with value reference
		m_ui->tableWidgetInputVars->blockSignals(true);
		QTableWidgetItem * valueRefItem = m_ui->tableWidgetInputVars->item(item->row(), 5); // column 5 = valueRef
		valueRefItem->setText( QString("%1").arg(otherVarsValueRef) );
		m_ui->tableWidgetInputVars->blockSignals(false);
	}

	// also look up variable definition in project file and sync definition
	for (unsigned int i=0; i<m_project.m_fmiDescription.m_inputVariables.size(); ++i) {
		// Mind: compare by NANDRAD variable definition, since we may have several vars with the same fmiVarName
		if (var.m_varName == m_project.m_fmiDescription.m_inputVariables[i].m_varName &&
			var.m_objectId == m_project.m_fmiDescription.m_inputVariables[i].m_objectId &&
			var.m_vectorIndex == m_project.m_fmiDescription.m_inputVariables[i].m_vectorIndex)
		{
			m_project.m_fmiDescription.m_inputVariables[i] = var;
			break;
		}
	}
	dumpUsedValueRefs();
}


void NandradFMUGeneratorWidget::on_tableWidgetOutputVars_itemChanged(QTableWidgetItem *item) {
	// assert that we are in the correct column
	Q_ASSERT(item->column() == 4);
	// read newly entered name
	unsigned int row = (unsigned int)item->row();
	std::string newFMIName = item->text().trimmed().toStdString();
	NANDRAD::FMIVariableDefinition & var = m_availableOutputVariables[row];
	std::string oldFMIName = var.m_fmiVarName;

	// loop over all defined input variables and check for duplicate FMI variable name (only in output variables for now)
	for (unsigned int i=0; i<m_availableOutputVariables.size(); ++i) {
		// skip ourselves
		if (item->row() == (int)i)
			continue;
		if (m_availableOutputVariables[i].m_fmiVarName == newFMIName) {
			// found a duplicate, check unit
			QMessageBox::critical(this, tr("Naming error"), tr("There is already another FMI output variable with name '%1'.")
								  .arg(QString::fromStdString(newFMIName)) );
			m_ui->tableWidgetOutputVars->blockSignals(true);
			item->setText( QString::fromStdString(var.m_fmiVarName) );
			m_ui->tableWidgetOutputVars->blockSignals(false);
			return;
		}
	}
	// all ok, keep the variable name
	var.m_fmiVarName = newFMIName;

	// also look up variable definition in project file and sync definition
	for (unsigned int i=0; i<m_project.m_fmiDescription.m_outputVariables.size(); ++i) {
		// Mind: output variables have unique fmi names, so we can just compare by FMI name
		if (m_project.m_fmiDescription.m_outputVariables[i].m_fmiVarName == oldFMIName) {
			m_project.m_fmiDescription.m_outputVariables[i] = var;
			break;
		}
	}
}
