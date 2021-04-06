#include "SVSimulationExportFMIDialog.h"
#include "ui_SVSimulationExportFMIDialog.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QProcess>

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVStyle.h"

SVSimulationExportFMIDialog::SVSimulationExportFMIDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVSimulationExportFMIDialog)
{
	m_ui->setupUi(this);

	m_ui->tableWidgetInputVars->setColumnCount(7);
	m_ui->tableWidgetInputVars->setHorizontalHeaderLabels( QStringList()
				<< tr("Used") << tr("Object") << tr("Variable") << tr("Index/ID")
				<< tr("FMIVarName") << tr("FMI Type") << tr("FMI Value Reference"));
	m_ui->tableWidgetInputVars->horizontalHeader()->setStretchLastSection(true);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetInputVars);
}


SVSimulationExportFMIDialog::~SVSimulationExportFMIDialog() {
	delete m_ui;
}


int SVSimulationExportFMIDialog::edit() {

	// create a copy of the current project
	m_localProject = project();
	m_localProject.updatePointers();

	updateInputVariableLists(true);

	return exec();
}


void SVSimulationExportFMIDialog::on_pushButtonClose_clicked() {
	// store data in project and close dialog
	accept();
}


void SVSimulationExportFMIDialog::on_pushButtonUpdateVariableList_clicked() {
	// create local copy of project

	// generate NANDRAD project, start solver as background process and read variable lists
	NANDRAD::Project p;

	try {
		SVSettings::instance().m_db.updateEmbeddedDatabase(m_localProject);
		m_localProject.generateNandradProject(p);
	}
	catch (VICUS::Project::ConversionError & ex) {
		QMessageBox::critical(this, tr("NANDRAD Project Generation Error"),
							  tr("%1\nBefore exporting an FMU, please make sure that the simulation runs correctly!").arg(ex.what()) );
		return;
	}
	catch (IBK::Exception & ex) {
		// just show a generic error message
		ex.writeMsgStackToError();
		QMessageBox::critical(this, tr("NANDRAD Project Generation Error"),
							  tr("An error occurred during NANDRAD project generation.\n"
								 "Before exporting an FMU, please make sure that the simulation runs correctly!"));
		return;
	}

	// save project
	QString nandradProjectFilePath = SVProjectHandler::instance().nandradProjectFilePath();
	p.writeXML(IBK::Path(nandradProjectFilePath.toStdString()));

	QStringList commandLineArgs;
	commandLineArgs.append("--test-init");
	commandLineArgs.append(nandradProjectFilePath);

	QString solverExecutable = SVSettings::nandradSolverExecutable();

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

	updateInputVariableLists(false);
}


void SVSimulationExportFMIDialog::updateInputVariableLists(bool silent) {
	QString nandradProjectFilePath = SVProjectHandler::instance().nandradProjectFilePath();
	// now parse the variable lists
	IBK::Path varDir(nandradProjectFilePath.toStdString());
	varDir = varDir.withoutExtension() / "var";

	QString inputVarsFile = QString::fromStdString( (varDir / "input_reference_list.txt").str() );
	QFile inputVarF(inputVarsFile);
	if (!inputVarF.open(QFile::ReadOnly)) {
		if (!silent)
			QMessageBox::critical(this, QString(), tr("Could not read file '%1'. Re-run solver initialization!")
								  .arg(inputVarsFile));
		return;
	}

	QStringList inputVars = QString(inputVarF.readAll()).split('\n');
	std::vector<NANDRAD::FMIVariableDefinition>	inputVarDefs;
	// we process all but first line
	for (int j=1; j<inputVars.count(); ++j) {
		inputVars[j] = inputVars[j].trimmed();
		if (inputVars[j].isEmpty())
			continue; // skip (trailing) empty lines)
		QStringList tokens = inputVars[j].split('\t');
		if (tokens.count() != 4) {
			QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Re-run solver initialization!")
				.arg(inputVarsFile));
			return;
		}

		NANDRAD::FMIVariableDefinition varDef;
		varDef.m_objectType = tokens[0].trimmed().toStdString();
		varDef.m_objectID = tokens[1].toUInt();
		varDef.m_varName = tokens[2].trimmed().toStdString();
		int varID = tokens[3].trimmed().toInt();
		if (varID == -1)
			varDef.m_varID = NANDRAD::INVALID_ID;
		else
			varDef.m_varID = (unsigned int)varID;

		inputVarDefs.push_back(varDef);
	}

	// now populate the table widget
	m_ui->tableWidgetInputVars->setRowCount(inputVarDefs.size());

	for (unsigned int i=0; i<inputVarDefs.size(); ++i) {
		const NANDRAD::FMIVariableDefinition & ivar = inputVarDefs[i];
		// check if we have this variable configured already?
		const NANDRAD::FMIVariableDefinition * existingDef = nullptr;
		for (const NANDRAD::FMIVariableDefinition & varDef : m_localProject.m_fmiDescription.m_inputVariableDefs) {
			if (varDef.sameModelVarAs(ivar)) {
				existingDef = &varDef;
				break;
			}
		}

		// BUG: wenn man das TableWidet item in der Spalte 0 als erstes Einfügt, bleiben die restlichen Spalten leer.
		//      das scheint ein seltsamer Bug in QTableWidget zu sein... müsste man mal debuggen.
		QTableWidgetItem * item1 = new QTableWidgetItem;
		item1->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		if (existingDef != nullptr)
			item1->setCheckState(Qt::Checked);
		else
			item1->setCheckState(Qt::Unchecked);
		m_ui->tableWidgetInputVars->setItem(i, 0, item1);

		QTableWidgetItem * item = new QTableWidgetItem;
		item->setText( QString("%1.%2").arg(QString::fromStdString(ivar.m_objectType)).arg(ivar.m_objectID));
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetInputVars->setItem(i, 1, item);

		item = new QTableWidgetItem(QString::fromStdString(ivar.m_varName));
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetInputVars->setItem(i, 2, item);

		if (ivar.m_varID != NANDRAD::INVALID_ID) {
			item = new QTableWidgetItem(ivar.m_varID);
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 3, item);
		}

		if (existingDef != nullptr) {
			item = new QTableWidgetItem(QString::fromStdString(existingDef->m_fmiVarName));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			m_ui->tableWidgetInputVars->setItem(i, 4, item);

			item = new QTableWidgetItem(QString::fromStdString(existingDef->m_fmiTypeName));
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 5, item);

			item = new QTableWidgetItem(QString("%1").arg(existingDef->m_fmiValueRef));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			m_ui->tableWidgetInputVars->setItem(i, 6, item);
		}
		else {
			item = new QTableWidgetItem;
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 4, item);

			item = new QTableWidgetItem;
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 5, item);

			item = new QTableWidgetItem;
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 6, item);
		}


	}
}



void SVSimulationExportFMIDialog::on_tableWidgetInputVars_itemChanged(QTableWidgetItem *item) {
	qDebug() << item->row() << item->column();
}


void SVSimulationExportFMIDialog::updateOutputVariableLists(bool silent) {
#if 0
	QString nandradProjectFilePath = SVProjectHandler::instance().nandradProjectFilePath();
	// now parse the variable lists
	IBK::Path varDir(nandradProjectFilePath.toStdString());
	varDir = varDir.withoutExtension() / "var";

	QString varsFile = QString::fromStdString( (varDir / "output_reference_list.txt").str() );
	QFile varF(varsFile);
	if (!varF.open(QFile::ReadOnly)) {
		if (!silent)
			QMessageBox::critical(this, QString(), tr("Could not read file '%1'. Re-run solver initialization!")
								  .arg(varsFile));
		return;
	}

	QStringList vars = QString(varF.readAll()).split('\n');
	std::vector<NANDRAD::FMIVariableDefinition>	varDefs;
	// we process all but first line
	for (int j=1; j<vars.count(); ++j) {
		vars[j] = vars[j].trimmed();
		if (vars[j].isEmpty())
			continue; // skip (trailing) empty lines)
		QStringList tokens = vars[j].split('\t');
		if (tokens.count() != 4) {
			QMessageBox::critical(this, QString(), tr("Invalid data in file '%1'. Re-run solver initialization!")
				.arg(varsFile));
			return;
		}

		NANDRAD::FMIVariableDefinition varDef;
		varDef.m_objectType = tokens[0].trimmed().toStdString();
		varDef.m_objectID = tokens[1].toUInt();
		varDef.m_varName = tokens[2].trimmed().toStdString();
		int varID = tokens[3].trimmed().toInt();
		if (varID == -1)
			varDef.m_varID = NANDRAD::INVALID_ID;
		else
			varDef.m_varID = (unsigned int)varID;

		varDefs.push_back(varDef);
	}

	// now populate the table widget
	m_ui->tableWidgetInputVars->setRowCount(varDefs.size());

	for (unsigned int i=0; i<varDefs.size(); ++i) {
		const NANDRAD::FMIVariableDefinition & ivar = varDefs[i];
		// check if we have this variable configured already?
		const NANDRAD::FMIVariableDefinition * existingDef = nullptr;
		for (const NANDRAD::FMIVariableDefinition & varDef : m_localProject.m_fmiDescription.m_inputVariableDefs) {
			if (varDef.sameModelVarAs(ivar)) {
				existingDef = &varDef;
				break;
			}
		}

		// BUG: wenn man das TableWidet item in der Spalte 0 als erstes Einfügt, bleiben die restlichen Spalten leer.
		//      das scheint ein seltsamer Bug in QTableWidget zu sein... müsste man mal debuggen.
		QTableWidgetItem * item1 = new QTableWidgetItem;
		item1->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		if (existingDef != nullptr)
			item1->setCheckState(Qt::Checked);
		else
			item1->setCheckState(Qt::Unchecked);
		m_ui->tableWidgetInputVars->setItem(i, 0, item1);

		QTableWidgetItem * item = new QTableWidgetItem;
		item->setText( QString("%1.%2").arg(QString::fromStdString(ivar.m_objectType)).arg(ivar.m_objectID));
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetInputVars->setItem(i, 1, item);

		item = new QTableWidgetItem(QString::fromStdString(ivar.m_varName));
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetInputVars->setItem(i, 2, item);

		if (ivar.m_varID != NANDRAD::INVALID_ID) {
			item = new QTableWidgetItem(ivar.m_varID);
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 3, item);
		}

		if (existingDef != nullptr) {
			item = new QTableWidgetItem(QString::fromStdString(existingDef->m_fmiVarName));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			m_ui->tableWidgetInputVars->setItem(i, 4, item);

			item = new QTableWidgetItem(QString::fromStdString(existingDef->m_fmiTypeName));
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 5, item);

			item = new QTableWidgetItem(QString("%1").arg(existingDef->m_fmiValueRef));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			m_ui->tableWidgetInputVars->setItem(i, 6, item);
		}
		else {
			item = new QTableWidgetItem;
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 4, item);

			item = new QTableWidgetItem;
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 5, item);

			item = new QTableWidgetItem;
			item->setFlags(Qt::ItemIsEnabled);
			m_ui->tableWidgetInputVars->setItem(i, 6, item);
		}


	}
#endif
}
