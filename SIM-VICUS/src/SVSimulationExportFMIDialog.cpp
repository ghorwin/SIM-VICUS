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

	m_ui->lineEditFilePath->setup("", true, false, tr("FMU files (*.fmu);;All files (*.*)"));

	m_ui->tableWidgetInputVars->setColumnCount(7);
	m_ui->tableWidgetInputVars->setRowCount(3);

	QTableWidgetItem * item = new QTableWidgetItem("1");
	m_ui->tableWidgetInputVars->setItem(0,0,item);

	item = new QTableWidgetItem("2");
	m_ui->tableWidgetInputVars->setItem(0,1,item);
	item = new QTableWidgetItem("3");
	m_ui->tableWidgetInputVars->setItem(1,0,item);
	item = new QTableWidgetItem("4");
	m_ui->tableWidgetInputVars->setItem(1,1,item);
	m_ui->tableWidgetInputVars->setHorizontalHeaderLabels(QStringList()
			  << tr("Model variable")
			  << tr("Object ID")
			  << tr("Vector value index/ID")
			  << tr("Unit")
			  << tr("FMI variable name")
			  << tr("FMI Type")
			  << tr("FMI value reference"));
	m_ui->tableWidgetInputVars->horizontalHeader()->setStretchLastSection(true);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetInputVars);

	m_ui->tableWidgetOutputVars->setColumnCount(7);
	m_ui->tableWidgetOutputVars->setHorizontalHeaderLabels(QStringList()
			  << tr("Model variable")
			  << tr("Object ID")
			  << tr("Vector value index/ID")
			  << tr("Unit")
			  << tr("FMI variable name")
			  << tr("FMI Type")
			  << tr("FMI value reference"));
	m_ui->tableWidgetOutputVars->horizontalHeader()->setStretchLastSection(true);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetOutputVars);
}


SVSimulationExportFMIDialog::~SVSimulationExportFMIDialog() {
	delete m_ui;
}


int SVSimulationExportFMIDialog::edit() {

	// create a copy of the current project
	m_localProject = project();
	m_localProject.updatePointers();

	if (m_localProject.m_fmiDescription.m_modelName.empty())
		m_localProject.m_fmiDescription.m_modelName = QFileInfo(SVProjectHandler::instance().projectFile()).baseName().toStdString();

	if (!m_localProject.m_fmiDescription.m_FMUPath.isValid())
		m_localProject.m_fmiDescription.m_FMUPath = IBK::Path(SVProjectHandler::instance().projectFile().toStdString()).withoutExtension() + ".fmu";

	// *** transfer general parameters

	m_ui->lineEditModelName->setText( QString::fromStdString(m_localProject.m_fmiDescription.m_modelName) );
	m_ui->lineEditFilePath->setFilename( QString::fromStdString(m_localProject.m_fmiDescription.m_FMUPath.str()) );

	updateVariableLists(true);

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

	updateVariableLists(false);
	updateFMUVariableTables();
}


void SVSimulationExportFMIDialog::updateVariableLists(bool silent) {
	QString nandradProjectFilePath = SVProjectHandler::instance().nandradProjectFilePath();
	// now parse the variable lists
	IBK::Path varDir(nandradProjectFilePath.toStdString());
	varDir = varDir.withoutExtension() / "var";

	QString inputVarsFile = QString::fromStdString( (varDir / "input_reference_list.txt").str() );
	if (!parseVariableList(inputVarsFile, m_modelInputVariables, silent))
		return;

	QString outputVarsFile = QString::fromStdString( (varDir / "output_reference_list.txt").str() );
	if (!parseVariableList(outputVarsFile, m_modelOutputVariables, silent))
		return;
	updateFMUVariableTables();
}


void SVSimulationExportFMIDialog::updateFMUVariableTables() {

	m_ui->tableWidgetInputVars->setRowCount(0);
	m_ui->tableWidgetOutputVars->setRowCount(0);
	for (unsigned int i = 0; i<m_localProject.m_fmiDescription.m_variables.size(); ++i) {
		const NANDRAD::FMIVariableDefinition & var = m_localProject.m_fmiDescription.m_variables[i];
		if (var.m_inputVariable) {
			// check if variable exists
			bool exists = (m_modelInputVariables.find(QString::fromStdString(var.m_varName)) != m_modelInputVariables.end());
			appendVariableEntry(i, m_ui->tableWidgetInputVars, exists);
		}
		else {
			// check if variable exists
			bool exists = (m_modelOutputVariables.find(QString::fromStdString(var.m_varName)) != m_modelOutputVariables.end());
			appendVariableEntry(i, m_ui->tableWidgetOutputVars, exists);
		}
	}

	m_ui->tableWidgetInputVars->resizeColumnsToContents();
	m_ui->tableWidgetOutputVars->resizeColumnsToContents();
}


bool SVSimulationExportFMIDialog::parseVariableList(const QString & varsFile,
													std::map<QString, IDInfo> & modelVariables, bool silent)
{
	QFile inputVarF(varsFile);
	if (!inputVarF.open(QFile::ReadOnly)) {
		if (!silent)
			QMessageBox::critical(this, QString(), tr("Could not read file '%1'. Re-run solver initialization!")
								  .arg(varsFile));
		return false;
	}

	QStringList vars = QString(inputVarF.readAll()).split('\n');

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

		// split object IDs and vector-value IDs
		IDInfo info;
		QString idString = tokens[1].trimmed();
		if (!idString.isEmpty()) {
			QStringList ids = idString.split(",");
			for (QString idstr : ids)
				info.m_objectIDs.push_back( idstr.toUInt());
		}
		idString = tokens[2].trimmed();
		if (!idString.isEmpty()) {
			QStringList ids = idString.split(",");
			for (QString idstr : ids)
				info.m_vectorIndexes.push_back( idstr.toUInt());
		}

		modelVariables[tokens[0].trimmed()] = info;
	}
	return true;
}


void SVSimulationExportFMIDialog::appendVariableEntry(unsigned int index, QTableWidget * tableWidget, bool exists) {
	const NANDRAD::FMIVariableDefinition & var = m_localProject.m_fmiDescription.m_variables[index];

	tableWidget->blockSignals(true);
	// Important: disable sorting of table, otherwise index access might be complicated
	tableWidget->setSortingEnabled(false);
	int row = tableWidget->rowCount();
	tableWidget->setRowCount(row+1);

	QFont disabledFont(tableWidget->font());
	disabledFont.setItalic(true);
	QColor disabledColor(Qt::gray);

	QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(var.m_varName));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	if (!exists) {
		item->setFont(disabledFont);
		item->setTextColor(disabledColor);
	}
	item->setData(Qt::UserRole, index);
	tableWidget->setItem(row, 0, item);

	item = new QTableWidgetItem(QString("%1").arg(var.m_objectID));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	if (!exists) {
		item->setFont(disabledFont);
		item->setTextColor(disabledColor);
	}
	tableWidget->setItem(row, 1, item);

	item = new QTableWidgetItem(QString("%1").arg(var.m_vectorIndex));
	if (var.m_vectorIndex == NANDRAD::INVALID_ID)
		item->setText(""); // no -1 display
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	if (!exists) {
		item->setFont(disabledFont);
		item->setTextColor(disabledColor);
	}
	tableWidget->setItem(row, 2, item);

	item = new QTableWidgetItem(QString::fromStdString(var.m_unit));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	if (!exists) {
		item->setFont(disabledFont);
		item->setTextColor(disabledColor);
	}
	tableWidget->setItem(row, 3, item);

	item = new QTableWidgetItem(QString::fromStdString(var.m_fmiVarName));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	if (!exists) {
		item->setFont(disabledFont);
		item->setTextColor(disabledColor);
	}
	tableWidget->setItem(row, 4, item);

	item = new QTableWidgetItem(QString::fromStdString(var.m_fmiTypeName));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	if (!exists) {
		item->setFont(disabledFont);
		item->setTextColor(disabledColor);
	}
	tableWidget->setItem(row, 5, item);

	item = new QTableWidgetItem(QString("%1").arg(var.m_fmiValueRef));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	if (!exists) {
		item->setFont(disabledFont);
		item->setTextColor(disabledColor);
	}
	tableWidget->setItem(row, 6, item);

	tableWidget->resizeColumnsToContents();
	tableWidget->blockSignals(false);
	tableWidget->setSortingEnabled(true);
	tableWidget->selectRow(row);
}




void SVSimulationExportFMIDialog::on_tableWidgetInputVars_currentCellChanged(int , int , int , int ) {
	m_ui->toolButtonCopyInputVariable->setEnabled(m_ui->tableWidgetInputVars->currentRow() != -1);
	m_ui->toolButtonRemoveInputVariable->setEnabled(m_ui->tableWidgetInputVars->currentRow() != -1);
}


void SVSimulationExportFMIDialog::on_tableWidgetOutputVars_currentCellChanged(int , int , int , int ) {
	m_ui->toolButtonCopyOutputVariable->setEnabled(m_ui->tableWidgetOutputVars->currentRow() != -1);
	m_ui->toolButtonRemoveOutputVariable->setEnabled(m_ui->tableWidgetOutputVars->currentRow() != -1);
}


void SVSimulationExportFMIDialog::on_toolButtonAddInputVariable_clicked() {
	// add FMU variable to input vars

	NANDRAD::FMIVariableDefinition var;
	var.m_varName = "Zone.AirTemperature";
	var.m_objectID = 1;
	var.m_unit = "K";
	var.m_vectorIndex = NANDRAD::INVALID_ID; // scalar variable

	var.m_fmiVarName = "Zone(1).AirTemperature";
	var.m_fmiTypeName = "Temperature";
	var.m_fmiValueRef = 160;

	m_localProject.m_fmiDescription.m_variables.push_back(var);

	// now also add an entry into the table
	appendVariableEntry(m_localProject.m_fmiDescription.m_variables.size()-1, m_ui->tableWidgetInputVars, true);
}


void SVSimulationExportFMIDialog::on_toolButtonRemoveInputVariable_clicked() {
	int row = m_ui->tableWidgetInputVars->currentRow();
	Q_ASSERT(row != -1);
	QTableWidgetItem * item = m_ui->tableWidgetInputVars->item(row, 0);
	unsigned int varIndex = item->data(Qt::UserRole).toUInt();
	Q_ASSERT(varIndex < m_localProject.m_fmiDescription.m_variables.size());

	m_localProject.m_fmiDescription.m_variables.erase(m_localProject.m_fmiDescription.m_variables.begin()+varIndex);
	m_ui->tableWidgetInputVars->removeRow(row);
//	updateFMUVariableTables();
	row = qMin(row, m_ui->tableWidgetInputVars->rowCount()-1);
	m_ui->tableWidgetInputVars->selectRow(row);
}


void SVSimulationExportFMIDialog::on_toolButtonRemoveOutputVariable_clicked() {
	int row = m_ui->tableWidgetOutputVars->currentRow();
	Q_ASSERT(row != -1);
	QTableWidgetItem * item = m_ui->tableWidgetOutputVars->item(row, 0);
	unsigned int varIndex = item->data(Qt::UserRole).toUInt();
	Q_ASSERT(varIndex < m_localProject.m_fmiDescription.m_variables.size());

	m_localProject.m_fmiDescription.m_variables.erase(m_localProject.m_fmiDescription.m_variables.begin()+varIndex);
	m_ui->tableWidgetOutputVars->removeRow(row);
//	updateFMUVariableTables();
	row = qMin(row, m_ui->tableWidgetOutputVars->rowCount()-1);
	m_ui->tableWidgetOutputVars->selectRow(row);
}


void SVSimulationExportFMIDialog::on_pushButtonGenerateAllVariables_clicked() {
	// process all input and output variables and generate new variables

	// naming scheme for FMI variables:

	// Zone(5).AirTemperature
	// Network(1).MassFlux(10)  - no distinguishing between ID and index

	unsigned int valRef = 150;

	std::vector<NANDRAD::FMIVariableDefinition> newVars;

	// first input variables
	for (const auto & var : m_modelInputVariables) {
		std::vector<std::string> varParts = IBK::explode(var.first.toStdString(), '.');

		Q_ASSERT(varParts.size() == 2);

		for (const unsigned int & id : var.second.m_objectIDs) {
			NANDRAD::FMIVariableDefinition fmiVar;
			fmiVar.m_inputVariable = true;
			fmiVar.m_varName = var.first.toStdString();
			fmiVar.m_objectID = id;
			fmiVar.m_fmiValueRef = ++valRef;
			QString desc;
			variableInfo(fmiVar.m_varName, desc, fmiVar.m_unit, fmiVar.m_fmiTypeName);
			if (var.second.m_vectorIndexes.empty()) {
				// scalar variable
				fmiVar.m_vectorIndex = NANDRAD::INVALID_ID;
				fmiVar.m_fmiVarName = IBK::FormatString("%1(%2).%3")
						.arg(varParts[0]).arg(id).arg(varParts[1]).str();
				if (!m_localProject.m_fmiDescription.hasVariable(fmiVar))
					newVars.push_back(fmiVar);
			}
			else {
				for (const unsigned int & vectorID : var.second.m_vectorIndexes) {
					fmiVar.m_vectorIndex = vectorID;
					fmiVar.m_fmiVarName = IBK::FormatString("%1(%2).%3(%4)")
							.arg(varParts[0]).arg(fmiVar.m_objectID)
							.arg(varParts[1]).arg(vectorID).str();
					if (!m_localProject.m_fmiDescription.hasVariable(fmiVar))
						newVars.push_back(fmiVar);
				}
			}
		}

	}

	// now output variables
	for (const auto & var : m_modelOutputVariables) {
		std::vector<std::string> varParts = IBK::explode(var.first.toStdString(), '.');

		Q_ASSERT(varParts.size() == 2);

		for (const unsigned int & id : var.second.m_objectIDs) {
			NANDRAD::FMIVariableDefinition fmiVar;
			fmiVar.m_inputVariable = false;
			fmiVar.m_varName = var.first.toStdString();
			fmiVar.m_objectID = id;
			QString desc;
			variableInfo(fmiVar.m_varName, desc, fmiVar.m_unit, fmiVar.m_fmiTypeName);
			if (var.second.m_vectorIndexes.empty()) {
				// scalar variable
				fmiVar.m_fmiValueRef = ++valRef;
				fmiVar.m_vectorIndex = NANDRAD::INVALID_ID;
				fmiVar.m_fmiVarName = IBK::FormatString("%1(%2).%3")
						.arg(varParts[0]).arg(id).arg(varParts[1]).str();
				if (!m_localProject.m_fmiDescription.hasVariable(fmiVar))
					newVars.push_back(fmiVar);
			}
			else {
				for (const unsigned int & vectorID : var.second.m_vectorIndexes) {
					fmiVar.m_vectorIndex = vectorID;
					fmiVar.m_fmiValueRef = ++valRef;
					fmiVar.m_fmiVarName = IBK::FormatString("%1(%2).%3(%4)")
							.arg(varParts[0]).arg(fmiVar.m_objectID)
							.arg(varParts[1]).arg(vectorID).str();
					// add only those new variables to project that do not exist already in variable list
					if (!m_localProject.m_fmiDescription.hasVariable(fmiVar))
						newVars.push_back(fmiVar);
				}
			}
		}
	}

	m_localProject.m_fmiDescription.m_variables.insert(m_localProject.m_fmiDescription.m_variables.end(),
													   newVars.begin(), newVars.end());
	updateFMUVariableTables();
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

void SVSimulationExportFMIDialog::variableInfo(const std::string & fullVarName, QString & description, std::string & unit, std::string & fmuType) {
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
