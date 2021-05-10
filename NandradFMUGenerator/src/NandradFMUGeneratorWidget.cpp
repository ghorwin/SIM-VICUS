#include "NandradFMUGeneratorWidget.h"
#include "ui_NandradFMUGeneratorWidget.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>
#include <QTimer>
#include <QFileDialog>
#include <QSettings>

#include <QtExt_Directories.h>

#include <JlCompress.h>

#include <IBK_messages.h>

const char * const ORGANIZATION = "IBK";
const char * const PROGRAM_NAME = "NANDRADFMUGenerator";


bool startProcess(const QString & executable, QStringList commandLineArgs, const QString & projectFile) {
	bool success;
	// spawn process
#ifdef Q_OS_WIN

	/// \todo use wide-string version of API and/or encapsulate spawn process into a function

	// Use WinAPI to create a solver process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	std::string utf8String = projectFile.toStdString().data();
	si.lpTitle = (LPSTR)utf8String.c_str();
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
	if( !CreateProcess( NULL,   // No module name (use command line).
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
	success = proc.waitForFinished();

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
	v->setSortingEnabled(true);
	v->sortByColumn(0, Qt::AscendingOrder);
	// smaller font for entire table
	QFont f;
	f.setPointSizeF(f.pointSizeF()*0.8);
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
		v->setSortingEnabled(true);
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


void NandradFMUGeneratorWidget::on_tableWidgetInputVars_currentCellChanged(int currentRow, int , int , int ) {
	m_ui->toolButtonAddInputVariable->setEnabled(false);
	m_ui->toolButtonRemoveInputVariable->setEnabled(false);
	if (currentRow == -1) {
		return;
	}
	QTableWidgetItem * item = m_ui->tableWidgetInputVars->item(currentRow, 0);
	// valid entry?
	if (item->data(Qt::UserRole).toBool()) {
		// already configured?
		if (item->data(Qt::UserRole+1).toUInt() == NANDRAD::INVALID_ID)
			m_ui->toolButtonAddInputVariable->setEnabled(true); // not yet configured -> add button on
		else
			m_ui->toolButtonRemoveInputVariable->setEnabled(true); // already configured -> remove button on
	}
	else {
		m_ui->toolButtonRemoveInputVariable->setEnabled(true); // invalid -> remove button on
	}
}


void NandradFMUGeneratorWidget::on_tableWidgetOutputVars_currentCellChanged(int currentRow, int , int , int ) {
	m_ui->toolButtonAddOutputVariable->setEnabled(false);
	m_ui->toolButtonRemoveOutputVariable->setEnabled(false);
	if (currentRow == -1) {
		return;
	}
	QTableWidgetItem * item = m_ui->tableWidgetOutputVars->item(currentRow, 0);
	// valid entry?
	if (item->data(Qt::UserRole).toBool()) {
		// already configured?
		if (item->data(Qt::UserRole+1).toUInt() == NANDRAD::INVALID_ID)
			m_ui->toolButtonAddOutputVariable->setEnabled(true); // not yet configured -> add button on
		else
			m_ui->toolButtonRemoveOutputVariable->setEnabled(true); // already configured -> remove button on
	}
	else {
		m_ui->toolButtonRemoveOutputVariable->setEnabled(true); // invalid -> remove button on
	}
}


void NandradFMUGeneratorWidget::on_toolButtonAddInputVariable_clicked() {
	// add FMU variable to input vars

	int row = m_ui->tableWidgetInputVars->currentRow();
	Q_ASSERT(row != -1);
	QTableWidgetItem * item = m_ui->tableWidgetInputVars->item(row,0);
	unsigned int valRef = item->data(Qt::UserRole+1).toUInt();
	Q_ASSERT(valRef == NANDRAD::INVALID_ID); // must be a valid, unused reference

	// find corresponding FMI variable description
	std::string fmiVarName = m_ui->tableWidgetInputVars->item(row, 4)->text().toStdString();
	for (NANDRAD::FMIVariableDefinition & var : m_availableInputVariables)  {
		if (var.m_fmiVarName == fmiVarName) {
			// got it, now create a copy of the variable description, copy it to the project and assign a valid value reference
			unsigned int newValueRef = *m_usedValueRefs.rbegin() + 1;
			m_usedValueRefs.insert(newValueRef);
			var.m_fmiValueRef = newValueRef;
			m_project.m_fmiDescription.m_inputVariables.push_back(var);
			// set new value reference in table
			m_ui->tableWidgetInputVars->item(row, 5)->setText(QString("%1").arg(newValueRef));
			item->setData(Qt::UserRole+1, newValueRef);
			// now update appearance of table row
			QFont f(m_ui->tableWidgetInputVars->font());
			f.setBold(true);
			for (int i=0; i<8; ++i) {
				m_ui->tableWidgetInputVars->item(row, i)->setFont(f);
				m_ui->tableWidgetInputVars->item(row, i)->setTextColor(Qt::black);
			}
			on_tableWidgetInputVars_currentCellChanged(row,0,0,0);
			break;
		}
	}
}


void NandradFMUGeneratorWidget::on_toolButtonRemoveInputVariable_clicked() {
	int row = m_ui->tableWidgetInputVars->currentRow();
	Q_ASSERT(row != -1);

	QTableWidgetItem * item = m_ui->tableWidgetInputVars->item(row,0);
	unsigned int valRef = item->data(Qt::UserRole+1).toUInt(); // Note: may be INVALID_ID in case of invalid definition
	// remove value reference from set of used value references
	m_usedValueRefs.erase(valRef);

	// get selected FMI variable name
	std::string fmiVarName = m_ui->tableWidgetInputVars->item(row, 4)->text().toStdString();
	// lookup existing definition in m_project and remove it there
	bool valid = item->data(Qt::UserRole).toBool();
	for (std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_project.m_fmiDescription.m_inputVariables.begin();
		 it != m_project.m_fmiDescription.m_inputVariables.end(); ++it)
	{
		if (it->m_fmiVarName == fmiVarName) {
			m_project.m_fmiDescription.m_inputVariables.erase(it);
			break;
		}
	}
	// lookup existing definition in m_availableInputVariables and clear the value reference there
	for (std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_availableInputVariables.begin();
		 it != m_availableInputVariables.end(); ++it)
	{
		if (it->m_fmiVarName == fmiVarName) {
			it->m_fmiValueRef = NANDRAD::INVALID_ID;
			break;
		}
	}
	// if valid, just clear item flags
	if (valid) {
		item->setData(Qt::UserRole+1, NANDRAD::INVALID_ID);
		m_ui->tableWidgetInputVars->item(row, 5)->setText("---");
		// now reset table row to uninitialized state
		QFont f(m_ui->tableWidgetInputVars->font());
		f.setItalic(true);
		for (int i=0; i<8; ++i) {
			m_ui->tableWidgetInputVars->item(row, i)->setFont(f);
			m_ui->tableWidgetInputVars->item(row, i)->setTextColor(Qt::gray);
		}
		on_tableWidgetInputVars_currentCellChanged(row,0,0,0);
	}
	else {
		// erase row in table
		m_ui->tableWidgetInputVars->removeRow(row);
		m_ui->tableWidgetInputVars->selectRow(std::min(row, m_ui->tableWidgetInputVars->rowCount()-1));
		on_tableWidgetInputVars_currentCellChanged(row,0,0,0);
	}
}


void NandradFMUGeneratorWidget::on_tableWidgetInputVars_itemDoubleClicked(QTableWidgetItem */*item*/) {
	// depending on the state of the buttons, call either add or remove
	if (m_ui->toolButtonAddInputVariable->isEnabled())
		m_ui->toolButtonAddInputVariable->click();
	else if (m_ui->toolButtonRemoveInputVariable->isEnabled())
		m_ui->toolButtonRemoveInputVariable->click();
}


void NandradFMUGeneratorWidget::on_toolButtonAddOutputVariable_clicked() {
	// add FMU variable to input vars

	int row = m_ui->tableWidgetOutputVars->currentRow();
	Q_ASSERT(row != -1);
	QTableWidgetItem * item = m_ui->tableWidgetOutputVars->item(row,0);
	unsigned int valRef = item->data(Qt::UserRole+1).toUInt();
	Q_ASSERT(valRef == NANDRAD::INVALID_ID); // must be a valid, unused reference

	// find corresponding FMI variable description
	std::string fmiVarName = m_ui->tableWidgetOutputVars->item(row, 4)->text().toStdString();
	for (NANDRAD::FMIVariableDefinition & var : m_availableOutputVariables)  {
		if (var.m_fmiVarName == fmiVarName) {
			// got it, now create a copy of the variable description, copy it to the project and assign a valid value reference
			unsigned int newValueRef = *m_usedValueRefs.rbegin() + 1;
			m_usedValueRefs.insert(newValueRef);
			var.m_fmiValueRef = newValueRef;
			m_project.m_fmiDescription.m_outputVariables.push_back(var);
			// set new value reference in table
			m_ui->tableWidgetOutputVars->item(row, 5)->setText(QString("%1").arg(newValueRef));
			item->setData(Qt::UserRole+1, newValueRef);
			// now update appearance of table row
			QFont f(m_ui->tableWidgetOutputVars->font());
			f.setBold(true);
			for (int i=0; i<8; ++i) {
				m_ui->tableWidgetOutputVars->item(row, i)->setFont(f);
				m_ui->tableWidgetOutputVars->item(row, i)->setTextColor(Qt::black);
			}
			on_tableWidgetOutputVars_currentCellChanged(row,0,0,0);
			break;
		}
	}
}


void NandradFMUGeneratorWidget::on_toolButtonRemoveOutputVariable_clicked() {
	int row = m_ui->tableWidgetOutputVars->currentRow();
	Q_ASSERT(row != -1);

	QTableWidgetItem * item = m_ui->tableWidgetOutputVars->item(row,0);
	unsigned int valRef = item->data(Qt::UserRole+1).toUInt(); // Note: may be INVALID_ID in case of invalid definition
	// remove value reference from set of used value references
	m_usedValueRefs.erase(valRef);

	// get selected FMI variable name
	std::string fmiVarName = m_ui->tableWidgetOutputVars->item(row, 4)->text().toStdString();
	// lookup existing definition in m_project and remove it there
	bool valid = item->data(Qt::UserRole).toBool();
	for (std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_project.m_fmiDescription.m_outputVariables.begin();
		 it != m_project.m_fmiDescription.m_outputVariables.end(); ++it)
	{
		if (it->m_fmiVarName == fmiVarName) {
			m_project.m_fmiDescription.m_outputVariables.erase(it);
			break;
		}
	}
	// lookup existing definition in m_availableInputVariables and clear the value reference there
	for (std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_availableOutputVariables.begin();
		 it != m_availableOutputVariables.end(); ++it)
	{
		if (it->m_fmiVarName == fmiVarName) {
			it->m_fmiValueRef = NANDRAD::INVALID_ID;
			break;
		}
	}
	// if valid, just clear item flags
	if (valid) {
		item->setData(Qt::UserRole+1, NANDRAD::INVALID_ID);
		m_ui->tableWidgetOutputVars->item(row, 5)->setText("---");
		// now reset table row to uninitialized state
		QFont f(m_ui->tableWidgetOutputVars->font());
		f.setItalic(true);
		for (int i=0; i<8; ++i) {
			m_ui->tableWidgetOutputVars->item(row, i)->setFont(f);
			m_ui->tableWidgetOutputVars->item(row, i)->setTextColor(Qt::gray);
		}
		on_tableWidgetOutputVars_currentCellChanged(row,0,0,0);
	}
	else {
		// erase row in table
		m_ui->tableWidgetOutputVars->removeRow(row);
		m_ui->tableWidgetOutputVars->selectRow(std::min(row, m_ui->tableWidgetOutputVars->rowCount()-1));
		on_tableWidgetOutputVars_currentCellChanged(row,0,0,0);
	}
}


void NandradFMUGeneratorWidget::on_tableWidgetOutputVars_itemDoubleClicked(QTableWidgetItem */*item*/) {
	// depending on the state of the buttons, call either add or remove
	if (m_ui->toolButtonAddOutputVariable->isEnabled())
		m_ui->toolButtonAddOutputVariable->click();
	else if (m_ui->toolButtonRemoveOutputVariable->isEnabled())
		m_ui->toolButtonRemoveOutputVariable->click();
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

//	bool success = startProcess(m_nandradSolverExecutable, commandLineArgs, QString::fromStdString(m_nandradFilePath.str()));

	commandLineArgs.append(QString::fromStdString(m_nandradFilePath.str()));

	QString solverExecutable = m_nandradSolverExecutable;

	QProcess proc(this);
	proc.setProgram(solverExecutable);
	proc.setArguments(commandLineArgs);

	proc.start();
	bool success = proc.waitForFinished();

	// TODO : For extremely large simulation projects, the intialization itself may take more than 30 seconds, so
	//        we may add a progress indicator dialog

	if (!success) {
		QMessageBox::critical(this, QString(), tr("Could not run solver '%1'").arg(solverExecutable));
		return;
	}

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

	// process all defined variables
	for (NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_inputVariables) {
		// check for duplicate value references
		if (m_usedValueRefs.find(var.m_fmiValueRef) != m_usedValueRefs.end()) {
			// remove invalid value reference, we will assign new IDs in the next step
			var.m_fmiValueRef = NANDRAD::INVALID_ID;
		}
		else
			m_usedValueRefs.insert(var.m_fmiValueRef);
	}
	for (NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_outputVariables) {
		// check for duplicate value references
		if (m_usedValueRefs.find(var.m_fmiValueRef) != m_usedValueRefs.end()) {
			// remove invalid value reference, we will assign new IDs in the next step
			var.m_fmiValueRef = NANDRAD::INVALID_ID;
		}
		else
			m_usedValueRefs.insert(var.m_fmiValueRef);
	}
	// determine first unused value reference
	unsigned int firstFreeValueRef = *m_usedValueRefs.rbegin()+1;
	// set unique value references to all existing variables (that had duplicates before)
	for (NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_inputVariables) {
		if (var.m_fmiValueRef == NANDRAD::INVALID_ID) {
			var.m_fmiValueRef = firstFreeValueRef;
			m_usedValueRefs.insert(firstFreeValueRef);
			++firstFreeValueRef;
		}
	}
	for (NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_outputVariables) {
		if (var.m_fmiValueRef == NANDRAD::INVALID_ID) {
			var.m_fmiValueRef = firstFreeValueRef;
			m_usedValueRefs.insert(firstFreeValueRef);
			++firstFreeValueRef;
		}
	}

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
		if (tokens.count() > 3)
			unit = tokens[3].trimmed();
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
				varDef.m_objectID = objID;
				varDef.m_vectorIndex = NANDRAD::INVALID_ID;
				varDef.m_fmiTypeName = ""; // TODO : how to determine the correct type?
				varDef.m_unit = unit.toStdString();
				varDef.m_fmiVarDescription = description.toStdString();
				varDef.m_fmiValueRef = NANDRAD::INVALID_ID; // will be set from either existing var in project or when configured

				modelVariables.push_back(varDef);
			}
			else {
				for (unsigned int vecIdx : m_vectorIndexes) {
					NANDRAD::FMIVariableDefinition varDef;
					varDef.m_varName = tokens[0].trimmed().toStdString(); // "Zone.AirTemperature"
					varDef.m_fmiVarName = QString("%1(%2).%3(%4)")
							.arg(objTypeName).arg(objID).arg(nandradVarName).arg(vecIdx)
							.toStdString();
					varDef.m_objectID = objID;
					varDef.m_vectorIndex = vecIdx;
					varDef.m_fmiTypeName = ""; // TODO : how to determine the correct type?
					varDef.m_unit = unit.toStdString();
					varDef.m_fmiVarDescription = description.toStdString();
					varDef.m_fmiValueRef = NANDRAD::INVALID_ID; // will be set from either existing var in project or when configured

					modelVariables.push_back(varDef);
				}

			}
		}
	}
	return true;
}


void NandradFMUGeneratorWidget::updateFMUVariableTables() {
	// we first process all variables already defined in the project, separately for inputs and ouputs
	std::vector<NANDRAD::FMIVariableDefinition> invalidInputVars;
	for (const NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_inputVariables) {
		// lookup variable in available variables
		std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_availableInputVariables.begin();
		for (; it != m_availableInputVariables.end(); ++it) {
			if (var.m_varName == it->m_varName)
				break;
		}
		// not found?
		if (it == m_availableInputVariables.end())
			invalidInputVars.push_back(var); // remember as invalid input var definition
		else
			it->m_fmiValueRef = var.m_fmiValueRef; // setting a valid value ref marks this variable as used and valid
	}

	std::vector<NANDRAD::FMIVariableDefinition> invalidOutputVars;
	for (const NANDRAD::FMIVariableDefinition & var : m_project.m_fmiDescription.m_outputVariables) {
		// lookup variable in available variables
		std::vector<NANDRAD::FMIVariableDefinition>::iterator it = m_availableOutputVariables.begin();
		for (; it != m_availableOutputVariables.end(); ++it) {
			if (var.m_varName == it->m_varName)
				break;
		}
		// not found?
		if (it == m_availableOutputVariables.end())
			invalidOutputVars.push_back(var); // remember as invalid input var definition
		else
			it->m_fmiValueRef = var.m_fmiValueRef; // setting a valid value ref marks this variable as used and valid
	}

	// now populate the tables
	populateTable(m_ui->tableWidgetInputVars, m_availableInputVariables, invalidInputVars);
	populateTable(m_ui->tableWidgetOutputVars, m_availableOutputVariables, invalidOutputVars);

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
											  const std::vector<NANDRAD::FMIVariableDefinition> & availableVars,
											  const std::vector<NANDRAD::FMIVariableDefinition> & invalidVars)
{
	table->setRowCount(0);
	table->setSortingEnabled(false); // disable sorting while we add rows
	// first add the invalid variables
	for (const NANDRAD::FMIVariableDefinition & var : invalidVars)
		appendVariableEntry(table, var, false);
	// then add the valid variables
	for (const NANDRAD::FMIVariableDefinition & var : availableVars)
		appendVariableEntry(table, var, true);

	table->setSortingEnabled(true); // re-enable sorting
	table->resizeColumnsToContents();
}



void NandradFMUGeneratorWidget::appendVariableEntry(QTableWidget * tableWidget,
													const NANDRAD::FMIVariableDefinition & var, bool valid)
{
	tableWidget->blockSignals(true);

	// add new row
	int row = tableWidget->rowCount();
	tableWidget->setRowCount(row+1);

	QFont itemFont(tableWidget->font());
	QColor itemColor(Qt::black);
	if (!valid)
		itemColor = QColor("#808080");
	else {
		if (var.m_fmiValueRef == NANDRAD::INVALID_ID) {
			itemFont.setItalic(true);
			itemColor = QColor(Qt::gray);
		}
		else {
			itemFont.setBold(true);
		}
	}

	QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(var.m_varName));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setFont(itemFont);
	item->setTextColor(itemColor);
	item->setData(Qt::UserRole, valid);
	item->setData(Qt::UserRole+1, var.m_fmiValueRef);
	tableWidget->setItem(row, 0, item);


	item = new QTableWidgetItem(QString("%1").arg(var.m_objectID));
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
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
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

	IBK::IBK_Message( IBK::FormatString("Generating FMU in directory '%1'\n").arg(baseDir.absolutePath().toStdString()),
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
	IBK::IBK_Message( IBK::FormatString("Copying climate data file '%1'\n").arg(fullClimatePath.filename()),
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

	int index=1;
	for (std::vector<NANDRAD::FMIVariableDefinition>::const_iterator
		 varIt = m_project.m_fmiDescription.m_inputVariables.begin();
		 varIt != m_project.m_fmiDescription.m_inputVariables.end();
		 ++varIt, ++index)
	{
		const NANDRAD::FMIVariableDefinition & varDef = *varIt;
		QString varDesc;
		varDesc = INPUT_VAR_TEMPLATE;
		varDesc.replace("${INDEX}", QString("%1").arg(index));
		varDesc.replace("${NAME}", varDef.m_fmiVarName.c_str());
		varDesc.replace("${VALUEREF}", QString("%1").arg(varIt->m_fmiValueRef));
		// special handling for differen variable types
		double startValue = 0;
		if (varDef.m_unit == "K")		startValue = 293.15;
		if (varDef.m_unit == "C")		startValue = 23;
		else if (varDef.m_unit == "%")	startValue = 50;
		else if (varDef.m_unit == "Pa")	startValue = 101325;
		varDesc.replace("${STARTVALUE}", QString::number(startValue));
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
		// special handling for differen variable types
		double startValue = 0;
		if (varDef.m_unit == "K")		startValue = 293.15;
		if (varDef.m_unit == "C")		startValue = 23;
		else if (varDef.m_unit == "%")	startValue = 50;
		else if (varDef.m_unit == "Pa")	startValue = 101325;
		varDesc.replace("${STARTVALUE}", QString::number(startValue));
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
		IBK::IBK_Message( IBK::FormatString("Copying Linux FMU lib '%1'\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QFile::copy(fmuLibFile,
					baseDir.absoluteFilePath("binaries/linux64/" + fmuModelName + ".so"));
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (linux64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// macos
	fmuLibFile = m_installDir + "/libNandradSolverFMI.dylib";
	if (QFile(fmuLibFile).exists()) {
		IBK::IBK_Message( IBK::FormatString("Copying MacOS FMU lib '%1'\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QFile::copy(fmuLibFile,
					baseDir.absoluteFilePath("binaries/darwin64/" + fmuModelName + ".dylib"));
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
		copyFiles << m_installDir + "/msvcp140.dll"
				  << m_installDir + "/vcomp140.dll"
				  << m_installDir + "/vcruntime140.dll";
		for (int i=0; i<copyFiles.count(); ++i) {
			if (!QFile::exists(copyFiles[i])) {
				IBK::IBK_Message( IBK::FormatString("Missing file '%1' to copy into FMU archive.\n").arg(copyFiles[i].toStdString()),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			}
			else {
				QFile::copy(copyFiles[i], binTargetPath + "/" + QFileInfo(copyFiles[i]).fileName());
			}
		}
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (Win64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	if (success) {

		// zip up the archive
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



