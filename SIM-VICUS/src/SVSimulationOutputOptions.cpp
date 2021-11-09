/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVSimulationOutputOptions.h"
#include "ui_SVSimulationOutputOptions.h"
#include "SVSimulationStartNandrad.h"

#include <VICUS_Outputs.h>
#include <VICUS_OutputDefinition.h>
#include <VICUS_KeywordList.h>

#include <IBK_FileReader.h>
#include <IBK_UnitList.h>

#include <NANDRAD_KeywordList.h>


#include <QModelIndex>
#include <QAbstractTableModel>
#include <QMessageBox>
#include <QTextCodec>

#include "SVProjectHandler.h"

#include "SVStyle.h"

SVSimulationOutputOptions::SVSimulationOutputOptions(QWidget *parent, VICUS::Outputs & outputs) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationOutputOptions),
	m_outputs(&outputs)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutOutputs->setMargin(0);

	// m_outputDefinitions = &m_outputs->m_definitions;

	m_ui->tableWidgetOutputGrids->setColumnCount(4);
	m_ui->tableWidgetOutputGrids->setHorizontalHeaderLabels( QStringList() << tr("Name") << tr("Intervals") << tr("Start") << tr("End") );
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetOutputGrids);
	m_ui->tableWidgetOutputGrids->setSortingEnabled(false);
	m_ui->radioButtonDefault->setChecked(true);

	m_outputTableModel = new SVSimulationOutputTableModel(this);
	m_outputTableModel->m_outputDefinitions = &m_outputDefinitions;

	m_outputTableProxyModel = new QSortFilterProxyModel(this);
	m_outputTableProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	m_outputTableProxyModel->setSourceModel(m_outputTableModel);
	m_ui->tableViewOutputList->setModel(m_outputTableProxyModel);

	SVStyle::formatDatabaseTableView(m_ui->tableViewOutputList);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetSourceObjectIds);

	m_ui->tableViewOutputList->setSelectionMode(QAbstractItemView::MultiSelection);
	m_ui->tableViewOutputList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_ui->tableViewOutputList->sortByColumn(1, Qt::AscendingOrder);

	m_ui->splitter->setStretchFactor(0,0);
	m_ui->splitter->setStretchFactor(1,1);

	m_ui->splitter_2->setStretchFactor(0,1);
	m_ui->splitter_2->setStretchFactor(1,0);

	m_ui->tableWidgetSourceObjectIds->setColumnCount(2);
	m_ui->tableWidgetSourceObjectIds->setHorizontalHeaderLabels(QStringList() << tr("ID") << tr("Display Name") );
	m_ui->tableWidgetSourceObjectIds->setColumnWidth(0, 50);
	m_ui->tableWidgetSourceObjectIds->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableWidgetSourceObjectIds->setSelectionMode(QAbstractItemView::MultiSelection);
	m_ui->tableWidgetSourceObjectIds->setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_ui->comboBoxTimeType->blockSignals(true);
	m_ui->comboBoxTimeType->addItem("None", NANDRAD::OutputDefinition::OTT_NONE);
	m_ui->comboBoxTimeType->addItem("Mean", NANDRAD::OutputDefinition::OTT_MEAN);
	m_ui->comboBoxTimeType->addItem("Integral", NANDRAD::OutputDefinition::OTT_INTEGRAL);
	m_ui->comboBoxTimeType->blockSignals(false);

	connect(m_ui->tableViewOutputList->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &SVSimulationOutputOptions::on_selectionChanged);

	m_font = QFont();
}


SVSimulationOutputOptions::~SVSimulationOutputOptions() {
	delete m_ui;
}


void SVSimulationOutputOptions::updateUi() {

	m_ui->checkBoxDefaultZoneOutputs->setChecked(
				m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].isEnabled());

	m_ui->checkBoxDefaultNetworkOutputs->setChecked(
				m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].isEnabled());

	m_ui->tableWidgetOutputGrids->clearContents();
	m_ui->comboBoxOutputGrid->clear();
	m_ui->tableWidgetOutputGrids->setRowCount(m_outputs->m_grids.size());

	// we set the correct timetype
	if(m_activeOutputDefinition != nullptr)
		for(unsigned int i=0; i<m_ui->comboBoxTimeType->count(); ++i) {
			if (m_activeOutputDefinition->m_outputdefinition.m_timeType == m_ui->comboBoxTimeType->itemData(i, Qt::UserRole)) {
				m_ui->comboBoxTimeType->setCurrentIndex(i);
				break;
			}
		}

	// we update the output grid
	for (unsigned int i=0; i<m_outputs->m_grids.size(); ++i) {
		const NANDRAD::OutputGrid & og = m_outputs->m_grids[i];
		QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(og.m_name));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetOutputGrids->setItem((int)i,0, item);


		// only populate start and end if intervals are all valid
		try {
			og.checkIntervalDefinition();

			item = new QTableWidgetItem(QString("%1").arg(og.m_intervals.size()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,1, item);

			QString start = QString("+ %L1 %2")
					.arg(og.m_intervals[0].m_para[NANDRAD::Interval::P_Start].value)
					.arg(QString::fromStdString(og.m_intervals[0].m_para[NANDRAD::Interval::P_Start].IO_unit.name()));

			item = new QTableWidgetItem(start);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,2, item);

			QString end;
			if (og.m_intervals.back().m_para[NANDRAD::Interval::P_End].value == 0.0)
				end = tr("End of simulation");
			else {
				end = QString("+ %L1 %2")
						.arg(og.m_intervals.back().m_para[NANDRAD::Interval::P_End].value)
						.arg(QString::fromStdString(og.m_intervals.back().m_para[NANDRAD::Interval::P_End].IO_unit.name()));
			}

			item = new QTableWidgetItem(end);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,3, item);
			m_ui->comboBoxOutputGrid->blockSignals(true);
			m_ui->comboBoxOutputGrid->addItem(QString::fromStdString(og.m_name), i);
			m_ui->comboBoxOutputGrid->blockSignals(false);
		}
		catch (...) {
			item = new QTableWidgetItem("---");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,1, item);
			item = new QTableWidgetItem("---");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,2, item);
			item = new QTableWidgetItem("---");
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,3, item);
		}
	}

}

// Returns empty QByteArray() on failure.
std::string fileChecksum(const QString &fileName,
						QCryptographicHash::Algorithm hashAlgorithm)
{
	QFile f(fileName);
	if (f.open(QFile::ReadOnly)) {
		QCryptographicHash hash(hashAlgorithm);
		if (hash.addData(&f)) {
			return QString(hash.result().toHex()).toStdString();
		}
	}
	return std::string();
}


void SVSimulationOutputOptions::generateOutputTable() {
	FUNCID(SVSimulationOutputOptions::generateOutputTable);
	// parse the variable list file
	// generate a data object with all output data
	//
	// fill the table in the outputs tab
	// user can select needed outputs
	// if simulation is run, all needed putputs are generated in NANDRAD file

	QString nandradFileString = SVProjectHandler::instance().nandradProjectFilePath();
	QString fileName = SVProjectHandler::instance().projectFile();

	int pos = fileName.lastIndexOf(".");
	QString fileNameWithoutEnding = fileName.left(pos);

	IBK::Path nandradFilePath(nandradFileString.toStdString());
	IBK::Path fileOutputVars = IBK::Path(fileNameWithoutEnding.toStdString() ) / "var" / "output_reference_list.txt" ;

	qDebug() << fileOutputVars.c_str();

	std::vector<std::string> outputContent;
	std::string checkSum =  fileChecksum(QString::fromStdString(fileOutputVars.str()), QCryptographicHash::Md5);

	try {
		IBK::FileReader::readAll(fileOutputVars, outputContent, std::vector<std::string>());
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Could not open file '%1' with output definitons.").arg(fileOutputVars.c_str()), FUNC_ID);
	}

	try {
		m_nandradProject.readXML(nandradFilePath);
	} catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Could not parse nandrad file '%1' to match output definitons.").arg(nandradFilePath.c_str()), FUNC_ID);
	}

	for (unsigned int i=0; i<outputContent.size(); ++i) {
		if (outputContent[i] == "")
			outputContent.erase(outputContent.begin() + i);
	}

	m_outputDefinitions.resize(outputContent.size()-1); // mind last column

	// now we go through all read lines
	std::vector<std::string> tokens;
	for (unsigned int i=0; i<outputContent.size(); ++i) {
		if ( i == 0 )
			continue;

		std::string &line = outputContent[i];
		IBK::explode(line, tokens, "\t", IBK::EF_NoFlags);

		QTableWidgetItem *item;
		std::vector<std::string> object;

		for(unsigned int j=0; j<tokens.size(); ++j) {
			item = new QTableWidgetItem();
			QString trimmedString = QString::fromStdString(tokens[j]).trimmed();
			item->setText(trimmedString);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

			OutputDefinition &od = m_outputDefinitions[i-1];

			od.m_outputdefinition.m_id = i-1;

			// set pointer to output grid
			if(m_outputs != nullptr)
				od.m_outputdefinition.m_outputGrid = &m_outputs->m_grids[0];

			// set Output time type
			od.m_outputdefinition.m_timeType = VICUS::OutputDefinition::OTT_NONE;

			if (j == ORT_VariableName) {
				object = IBK::explode(trimmedString.toStdString(), '.', 2);
				od.m_outputdefinition.m_type = object[0];
				od.m_outputdefinition.m_name = object[1];
			}
			else if (j == ORT_Unit)
				od.m_outputdefinition.m_unit = IBK::Unit(trimmedString.toStdString());
			else if (j == ORT_Description)
				od.m_outputdefinition.m_description = trimmedString.toStdString();
			else if (j == ORT_SourceObjectIds) {
				od.m_outputdefinition.m_sourceObjectIds.clear();
				std::vector<std::string> ids;
				IBK::explode(trimmedString.toStdString(), ids, ",", IBK::EF_NoFlags);
				for (std::string id : ids) {

					// we get the nandrad id
					unsigned int nandradId = QString::fromStdString(id).toUInt();
					std::string name = "No object found";

					if ( object[0] == "ConstructionInstance" ) {
						findNandradName(&m_nandradProject.m_constructionInstances, nandradId, name);
					}
					else if ( object[0] == "Zone" ) {
						findNandradName(&m_nandradProject.m_zones, nandradId, name);
					}
					else if ( object[0] == "Location" ) {
						name = "Climate Data";
					}
					else if ( object[0] == "EmbeddedObject" ) {
						for ( const NANDRAD::ConstructionInstance &ci : m_nandradProject.m_constructionInstances ) {
							findNandradName(&ci.m_embeddedObjects, nandradId, name);
						}
					}
					else if ( object[0] == "Model" ) {
						if (findNandradName(&m_nandradProject.m_models.m_internalLoadsModels,nandradId, name));
						else if (findNandradName(&m_nandradProject.m_models.m_shadingControlModels,nandradId, name));
						else if (findNandradName(&m_nandradProject.m_models.m_heatLoadSummationModels,nandradId, name));
						else if (findNandradName(&m_nandradProject.m_models.m_idealPipeRegisterModels,nandradId, name));
						else if (findNandradName(&m_nandradProject.m_models.m_naturalVentilationModels,nandradId, name));
						else if (findNandradName(&m_nandradProject.m_models.m_idealHeatingCoolingModels,nandradId, name));
						else if (findNandradName(&m_nandradProject.m_models.m_networkInterfaceAdapterModels,nandradId, name));
						else if (findNandradName(&m_nandradProject.m_models.m_idealSurfaceHeatingCoolingModels,nandradId, name));
					}

					od.m_outputdefinition.m_sourceObjectIds.push_back(nandradId);
					od.m_outputdefinition.m_idToSourceObject[nandradId] = VICUS::SourceObject(nandradId,name);
				}
			}
			else if (j == 2) {
				od.m_outputdefinition.m_vectorIds.clear();
				std::vector<std::string> ids;
				IBK::explode(trimmedString.toStdString(), ids, ",", IBK::EF_NoFlags);
				for (std::string id : ids) {
					od.m_outputdefinition.m_vectorIds.push_back(QString::fromStdString(id).toUInt() );
				}
			}
		}
	}

	// we restore our outputs if checkSum is equal
	if(m_outputs->m_checkSum.empty())
		m_outputs->m_checkSum = fileChecksum(QString::fromStdString(fileOutputVars.str()), QCryptographicHash::Md5);
	else {
		if (m_outputs->m_checkSum == checkSum){
			m_ui->radioButtonCustom->setChecked(true);
			// regenerate outputs
			for (unsigned int i=0; i<m_outputs->m_outputDefinitions.size(); ++i) {
				for (unsigned int j=0; j<m_outputDefinitions.size(); ++j) {
					if (m_outputs->m_outputDefinitions[i].m_id == m_outputDefinitions[j].m_outputdefinition.m_id) {
						m_outputDefinitions[j].m_isActive = true;
						for (unsigned int id : m_outputs->m_outputDefinitions[i].m_activeSourceObjectIds) {
							m_outputDefinitions[j].m_outputdefinition.m_idToSourceObject[id].m_isActive = true;
						}
						m_outputDefinitions[j].m_outputdefinition.m_activeSourceObjectIds = m_outputs->m_outputDefinitions[i].m_activeSourceObjectIds;
						break;
					}
				}
			}
			QMessageBox::information(this, QString(), tr("Regenerated custom outputs under 'outputs' tab."));
		}
		else
			m_outputs->m_outputDefinitions.clear();
	}


	m_outputTableModel->reset();
	//	m_ui->tableViewOutputList->resizeColumnsToContents();
	m_ui->tableViewOutputList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableViewOutputList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui->tableViewOutputList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	m_ui->tableViewOutputList->setColumnWidth(3, 50);
	m_ui->tableViewOutputList->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
}

void SVSimulationOutputOptions::initOutputTable(unsigned int rowCount) {
	//	m_ui->tableWidgetOutputList->setColumnCount(5);
	//	m_ui->tableWidgetOutputList->setHorizontalHeaderLabels( QStringList() << tr("Name") << tr("Unit") << tr("Description") << tr("Source object id(s)") << tr("Vector indexes/ids") );
	//	m_ui->tableWidgetOutputList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//	m_ui->tableWidgetOutputList->setColumnWidth(1, 100);
	//	m_ui->tableWidgetOutputList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	//	m_ui->tableWidgetOutputList->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	//	m_ui->tableWidgetOutputList->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	//	m_ui->tableWidgetOutputList->setRowCount(rowCount);

	//	m_ui->tableWidgetOutputList->verticalHeader()->setDefaultSectionSize(19);
	//	m_ui->tableWidgetOutputList->verticalHeader()->setVisible(false);
	//	m_ui->tableWidgetOutputList->horizontalHeader()->setMinimumSectionSize(19);
	//	m_ui->tableWidgetOutputList->setSelectionBehavior(QAbstractItemView::SelectRows);
	//	m_ui->tableWidgetOutputList->setSelectionMode(QAbstractItemView::SingleSelection);
	//	m_ui->tableWidgetOutputList->setAlternatingRowColors(true);
	//	m_ui->tableWidgetOutputList->setSortingEnabled(true);
	//	m_ui->tableWidgetOutputList->sortByColumn(0, Qt::AscendingOrder);

	//	QString headerStyleSheet = QString("QHeaderView::section:horizontal {font-weight:bold;}");
	//	m_ui->tableWidgetOutputList->horizontalHeader()->setStyleSheet(headerStyleSheet);
}

void SVSimulationOutputOptions::generateOutputs(const std::vector<NANDRAD::ObjectList> &objectList) {
	m_outputDefinitionsNandrad.clear();
	m_objectListsNandrad.clear();

	// we also want to use the already defined object lists and use them as well
	for (const NANDRAD::ObjectList &ol : objectList)
		m_objectListsNandrad[ol.m_name] = ol;

	// we iterate over our defined outputs
	for (unsigned int i=0; i<m_outputs->m_outputDefinitions.size(); ++i) {

		const VICUS::OutputDefinition &od = m_outputs->m_outputDefinitions[i];

		std::string refName;

		// ==============================================
		// GRID
		// ==============================================
		// till now we only support output grids with hourly values

		{
			// generate output grid, if needed
			int ogInd = -1;
			for (unsigned int i=0; i<m_outputs->m_grids.size(); ++i) {
				NANDRAD::OutputGrid & og = m_outputs->m_grids[i];
				if (og.m_intervals.size() == 1 &&
						og.m_intervals.back().m_para[NANDRAD::Interval::P_Start].value == 0.0 &&
						og.m_intervals.back().m_para[NANDRAD::Interval::P_End].name.empty() &&
						og.m_intervals.back().m_para[NANDRAD::Interval::P_StepSize].value == 3600.0)
				{
					ogInd = (int)i;
					break;
				}
			}
			// create one, if not yet existing
			if (ogInd == -1) {
				NANDRAD::OutputGrid og;
				og.m_name = refName = tr("Hourly values").toStdString();
				NANDRAD::Interval iv;
				NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_Start, 0);
				NANDRAD::KeywordList::setParameter(iv.m_para, "Interval::para_t", NANDRAD::Interval::P_StepSize, 1);
				og.m_intervals.push_back(iv);
				m_outputs->m_grids.push_back(og);
			}
			else {
				refName = m_outputs->m_grids[(unsigned int)ogInd].m_name;
			}
		}

		// ==============================================
		// OBJECT LISTS
		// ==============================================
		// we generate the object list

		NANDRAD::ObjectList ol;
		ol.m_name = od.m_type + "[";
		for (unsigned int i=0; i<od.m_sourceObjectIds.size(); ++i) {
			bool foundActiveId = false;
			for (unsigned int j=0; j<od.m_activeSourceObjectIds.size(); j++) {
				if(od.m_activeSourceObjectIds[j] == od.m_sourceObjectIds[i]) {// object is active
					foundActiveId = true;
					break;
				}
			}

			if(!foundActiveId)
				continue;
			ol.m_name += QString::number(od.m_sourceObjectIds[i]).toStdString();
			if (i<od.m_sourceObjectIds.size()-1)
				ol.m_name += ",";

			ol.m_filterID.m_ids.insert(od.m_sourceObjectIds[i]);
		}
		ol.m_name += "]";
		if (od.m_type == "Location")
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
		else if (od.m_type == "Zone")
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		else if (od.m_type == "Model")
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
		else if (od.m_type == "ConstructionInstance")
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
		else if (od.m_type == "NetworkElement")
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		else if (od.m_type == "EmbeddedObject")
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_EMBEDDED_OBJECT;
		else if (od.m_type == "Schedule")
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_SCHEDULE;


		// we have to look if we already have an object list with the same content
		// if we find one, we take its name and use it as reference to it
		if (!findEqualObjectList(ol))
			m_objectListsNandrad[ol.m_name] = ol;

		// ==============================================
		// OUTPUT DEFINITIONS & OBJECT LISTS
		// ==============================================
		// we already have a name for the output grid, start generating default outputs

		NANDRAD::OutputDefinition nod;
		nod.m_gridName = refName;
		nod.m_objectListName = ol.m_name;
		nod.m_timeType = (NANDRAD::OutputDefinition::timeType_t)od.m_timeType;

		// handling for all output that also reference vectors
		if (!od.m_vectorIds.empty()) {
			for (unsigned int i=0; i<od.m_vectorIds.size(); ++i) {
				nod.m_quantity = od.m_name + "[" + QString::number(od.m_vectorIds[i]).toStdString() + "]";
				m_outputDefinitionsNandrad[nod.m_quantity] = nod;
			}
		}
		else {
			nod.m_quantity = od.m_name;
			m_outputDefinitionsNandrad[nod.m_quantity] = nod;
		}
	}
}

bool SVSimulationOutputOptions::findEqualObjectList(NANDRAD::ObjectList & objectList) {
	for ( std::map<std::string, NANDRAD::ObjectList>::const_iterator it = m_objectListsNandrad.begin();
		  it != m_objectListsNandrad.end(); ++it) {
		if (objectList.m_referenceType == it->second.m_referenceType) {
			// we sort both vectors
			if (it->second.m_filterID == objectList.m_filterID) {
				objectList = it->second;
				return true;
			}
		}
	}
	return false;
}

std::vector<NANDRAD::ObjectList> SVSimulationOutputOptions::objectLists() {
	std::vector<NANDRAD::ObjectList> objectListsVector;
	for (std::map<std::string, NANDRAD::ObjectList>::const_iterator it = m_objectListsNandrad.begin();
		 it != m_objectListsNandrad.end(); ++it	 )
		objectListsVector.push_back(it->second);

	return objectListsVector;
}

std::vector<NANDRAD::OutputDefinition> SVSimulationOutputOptions::outputDefinitions() {
	std::vector<NANDRAD::OutputDefinition> outputDefinitionsVector;
	for (std::map<std::string, NANDRAD::OutputDefinition>::const_iterator it = m_outputDefinitionsNandrad.begin();
		 it != m_outputDefinitionsNandrad.end(); ++it	 )
		outputDefinitionsVector.push_back(it->second);

	return outputDefinitionsVector;
}


void SVSimulationOutputOptions::on_checkBoxDefaultZoneOutputs_toggled(bool checked) {
	if (checked)
		m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs]
				.set(VICUS::KeywordList::Keyword("Outputs::flag_t", VICUS::Outputs::F_CreateDefaultZoneOutputs), checked);
	else
		m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].clear();
}

void SVSimulationOutputOptions::on_checkBoxDefaultNetworkOutputs_toggled(bool checked){
	if (checked)
		m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs]
				.set(VICUS::KeywordList::Keyword("Outputs::flag_t", VICUS::Outputs::F_CreateDefaultNetworkOutputs), checked);
	else
		m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].clear();
}

void SVSimulationOutputOptions::on_radioButtonDefault_toggled(bool defaultToggled) {

	m_ui->radioButtonCustom->blockSignals(true);
	m_ui->radioButtonCustom->setChecked(!defaultToggled);
	m_ui->radioButtonCustom->blockSignals(false);

	//	m_ui->radioButtonDefault->setChecked(!defaultToggled);

	m_ui->checkBoxDefaultZoneOutputs->setEnabled(defaultToggled);
	m_ui->checkBoxDefaultNetworkOutputs->setEnabled(defaultToggled);

	m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs] = IBK::Flag("CreateDefaultZoneOutputs",
																			   m_ui->checkBoxDefaultZoneOutputs->isChecked() && defaultToggled );
	m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs] = IBK::Flag("CreateDefaultNetworkOutputs",
																				  m_ui->checkBoxDefaultNetworkOutputs->isChecked() && defaultToggled );

	m_ui->widgetCustomOutputs->setEnabled(!defaultToggled);
}

void SVSimulationOutputOptions::on_lineEditType_textEdited(const QString &filterKey) {
	m_ui->lineEditName->clear();
	m_outputTableProxyModel->setFilterWildcard(filterKey);
	m_outputTableProxyModel->setFilterKeyColumn(1);
	// m_itemIsSet = false;
}


void SVSimulationOutputOptions::on_tableViewOutputList_doubleClicked(const QModelIndex &index) {

	QModelIndex sourceIndex = m_outputTableProxyModel->mapToSource(index);
	OutputDefinition &od = m_outputDefinitions[sourceIndex.row()];

	Q_ASSERT(sourceIndex.row()<m_outputDefinitions.size());

	bool outputDefinitionState = od.m_isActive;

	m_outputTableModel->updateOutputData(sourceIndex.row());

	QTableWidget &tw = *m_ui->tableWidgetSourceObjectIds;
	tw.setSortingEnabled(false);
	QTableWidgetItem *itemID, *itemName;
	for (unsigned int i=0; i<od.m_outputdefinition.m_sourceObjectIds.size(); ++i) {
		VICUS::SourceObject &so =	od.m_outputdefinition.m_idToSourceObject[od.m_outputdefinition.m_sourceObjectIds[i]];
		so.m_isActive = !outputDefinitionState;

		if(!outputDefinitionState) { // source object has been set active
			od.m_outputdefinition.m_activeSourceObjectIds.push_back(so.m_id); // object id not in vector
		}
		else {
			for(std::vector<unsigned int>::iterator it = od.m_outputdefinition.m_activeSourceObjectIds.begin();
				it != od.m_outputdefinition.m_activeSourceObjectIds.end(); ++it ) {
				if(*it == so.m_id) {
					// delete source object id
					od.m_outputdefinition.m_activeSourceObjectIds.erase(it);
					break;
				}
			}
		}

		itemID = new QTableWidgetItem();
		itemName = new QTableWidgetItem();

		itemID->setText(QString::number(so.m_id));
		itemName->setText(QString::fromStdString(so.m_displayName));

		tw.setItem(i,0,itemID);
		tw.setItem(i,1,itemName);

		QFont f(m_font);
		f.setItalic(!so.m_isActive);
		f.setBold(!outputDefinitionState && so.m_isActive);


		itemID->setFont(f);
		itemName->setFont(f);

		itemID->setFlags(itemID->flags()& ~Qt::ItemIsEditable);
		itemName->setFlags(itemID->flags()& ~Qt::ItemIsEditable);

		itemID->setData(Qt::UserRole, i);
		itemName->setData(Qt::UserRole, i);
	}

	// we update the output definition
	updateOutputDefinition(od, !outputDefinitionState);

	tw.setSortingEnabled(true);

	m_ui->tableWidgetSourceObjectIds->setEnabled(!outputDefinitionState);

	m_ui->comboBoxOutputGrid->setCurrentText(QString::fromStdString(od.m_outputdefinition.m_name));

	// we also have to generate an output in the project
	// we need an hourly output grid, look if we have already one defined (should be!)
	m_outputTableModel->updateOutputData(sourceIndex.row());



}


void SVSimulationOutputOptions::on_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

	if(selected.empty() && deselected.empty()) {
		m_itemIsSet = false;
		return;
	}

	const QItemSelection &selection = selected.empty() ? deselected : selected;
	unsigned int shift = selected.empty() ? 1 : 0; // we need a shift if deselection happens; This also needs to be positive or negatibe

	QTableWidget &tw = *m_ui->tableWidgetSourceObjectIds;
	tw.clearContents();
	tw.setSortingEnabled(false);

	// we find our last selected object
	unsigned int row = 0;
	if(m_itemIsSet && !m_itemSelection.indexes().isEmpty()) {
		const QModelIndex &sourceIndexMax = m_outputTableProxyModel->mapToSource(selection.indexes()[selection.indexes().size()-1]);
		const QModelIndex &cachedSourceIndexMax = m_outputTableProxyModel->mapToSource(m_itemSelection.indexes()[m_itemSelection.indexes().size()-1]);

		int deltaMaxRow = sourceIndexMax.row() - cachedSourceIndexMax.row();

		qDebug() << "Actual row: " + QString::number(sourceIndexMax.row()) + " | Cached Row: " + QString::number(cachedSourceIndexMax.row());

		row = (deltaMaxRow == 0 ? m_outputTableProxyModel->mapToSource(selection.indexes()[0]).row()-shift : sourceIndexMax.row()+shift); // for deselection we need a +/-1 shift; for selection no shift
	}
	else
		row = m_outputTableProxyModel->mapToSource(selection.indexes()[0]).row();

	const OutputDefinition &od = m_outputDefinitions[row];
	m_ui->labelSourceObjects->setText("Select " + QString::fromStdString(od.m_outputdefinition.m_type) + "s");

	m_activeOutputDefinition = &od;
	m_ui->comboBoxTimeType->blockSignals(true);
	m_ui->comboBoxTimeType->setCurrentIndex(od.m_outputdefinition.m_timeType);

	tw.setEnabled(od.m_isActive);
	tw.setSortingEnabled(true);
	tw.sortByColumn(1, Qt::AscendingOrder);

	updateOutputUi(row);

	m_ui->comboBoxTimeType->blockSignals(false);
	// we cache our selection
	m_itemSelection = selection;
	m_itemIsSet = true;
}

void SVSimulationOutputOptions::on_pushButtonAllSources_clicked() {
	OutputDefinition &od = *const_cast<OutputDefinition*>(m_activeOutputDefinition);
	for (unsigned int i=0; i<od.m_outputdefinition.m_sourceObjectIds.size(); ++i) {

		std::vector<unsigned int> &os = const_cast<OutputDefinition*>(m_activeOutputDefinition)->m_outputdefinition.m_activeSourceObjectIds;

		od.m_outputdefinition.m_idToSourceObject[od.m_outputdefinition.m_sourceObjectIds[i]].m_isActive = true;

		if(std::find(os.begin(), os.end(), od.m_outputdefinition.m_sourceObjectIds[i]) != os.end())
			os.push_back(od.m_outputdefinition.m_sourceObjectIds[i]); // object id not in vector

		QTableWidgetItem *itemID = m_ui->tableWidgetSourceObjectIds->item(i, 0);
		QTableWidgetItem *itemName = m_ui->tableWidgetSourceObjectIds->item(i, 1);

		QFont f(m_font);
		f.setItalic(false);
		f.setBold(true);
		itemID->setFont(f);
		itemName->setFont(f);
	}

	updateOutputDefinition(od, true);
}

void SVSimulationOutputOptions::on_tableWidgetSourceObjectIds_itemDoubleClicked(QTableWidgetItem *item) {
	OutputDefinition &od = *const_cast<OutputDefinition*>(m_activeOutputDefinition);
	unsigned int row = item->row();

	QTableWidgetItem *itemID = m_ui->tableWidgetSourceObjectIds->item(row, 0);
	QTableWidgetItem *itemName = m_ui->tableWidgetSourceObjectIds->item(row, 1);

	VICUS::SourceObject &so = od.m_outputdefinition.m_idToSourceObject[m_activeOutputDefinition->m_outputdefinition.m_sourceObjectIds[itemID->data(Qt::UserRole).toUInt()]];

	bool objectState = so.m_isActive;
	const_cast<VICUS::SourceObject &>(so).m_isActive = !objectState;

	if(!objectState) { // source object has been set active
		od.m_outputdefinition.m_activeSourceObjectIds.push_back(so.m_id); // object id not in vector
	}
	else {
		for(std::vector<unsigned int>::iterator it = od.m_outputdefinition.m_activeSourceObjectIds.begin();
			it != od.m_outputdefinition.m_activeSourceObjectIds.end(); ++it ) {
			if(*it == so.m_id) {
				// delete source object id
				od.m_outputdefinition.m_activeSourceObjectIds.erase(it);
				break;
			}
		}
	}

	updateOutputDefinition(od,od.m_isActive && !od.m_outputdefinition.m_sourceObjectIds.empty());

	QFont f(m_font);
	f.setItalic(!so.m_isActive);
	f.setBold(so.m_isActive);
	itemID->setFont(f);
	itemName->setFont(f);
}

void SVSimulationOutputOptions::on_lineEditName_textEdited(const QString & filterKey) {
	m_ui->lineEditType->clear();
	m_outputTableProxyModel->setFilterWildcard(filterKey);
	m_outputTableProxyModel->setFilterKeyColumn(2);
	m_itemIsSet = false;
}

void SVSimulationOutputOptions::on_pushButtonAllSourcesDeselected_clicked(){
	OutputDefinition &od = *const_cast<OutputDefinition*>(m_activeOutputDefinition);
	od.m_outputdefinition.m_activeSourceObjectIds.clear();
	for (unsigned int i=0; i<od.m_outputdefinition.m_sourceObjectIds.size(); ++i) {

		QTableWidgetItem *itemID = m_ui->tableWidgetSourceObjectIds->item(i, 0);
		QTableWidgetItem *itemName = m_ui->tableWidgetSourceObjectIds->item(i, 1);

		QFont f(m_font);
		f.setItalic(true);
		f.setBold(false);
		itemID->setFont(f);
		itemName->setFont(f);

	}

	updateOutputDefinition(od, false);
}

void SVSimulationOutputOptions::on_comboBoxOutoutGrid_currentIndexChanged(int index){
	Q_ASSERT(index < m_outputs->m_grids.size());

	const_cast<OutputDefinition*>(m_activeOutputDefinition)->m_outputdefinition.m_outputGrid = &m_outputs->m_grids[index];
}

template<typename T>
bool SVSimulationOutputOptions::findNandradName(const std::vector<T> *models, const unsigned int idNandrad, std::string & name) {

	if (models == nullptr)
		return false;

	for ( const T &m : *models) {
		if (m.m_id == idNandrad) {
			name = m.m_displayName;
			return true;
		}
	}
}

void SVSimulationOutputOptions::on_toolButtonAddOutput_clicked() {
	QItemSelectionModel *selection = m_ui->tableViewOutputList->selectionModel();
	unsigned int cachedRow;

	for (const QModelIndex & proxyIndex: m_ui->tableViewOutputList->selectionModel()->selectedRows()) {
		// configure new input var - requires valid selection
		Q_ASSERT(proxyIndex.isValid());

		// map 2 source index
		QModelIndex srcIndex = m_outputTableProxyModel->mapToSource(proxyIndex);

		unsigned int row = (unsigned int)srcIndex.row();
		Q_ASSERT(row < m_outputDefinitions.size());

		// we set our
		updateOutputDefinitionState(row, true);

		cachedRow = row;
	}

	updateOutputUi(cachedRow);
	m_ui->tableViewOutputList->setFocus();
}

void SVSimulationOutputOptions::updateOutputDefinitionState(unsigned int row, bool newState) {
	Q_ASSERT(row<m_outputDefinitions.size());

	OutputDefinition &od = m_outputDefinitions[row];

	// and we also set the states to our source objects
	for (unsigned int i=0; i<od.m_outputdefinition.m_sourceObjectIds.size(); ++i) {
		VICUS::SourceObject &so = od.m_outputdefinition.m_idToSourceObject[od.m_outputdefinition.m_sourceObjectIds[i]];
		so.m_isActive = newState;

		if(newState) { // source object has been set active
			od.m_outputdefinition.m_activeSourceObjectIds.push_back(so.m_id); // object id not in vector
		}
		else {
			for(std::vector<unsigned int>::iterator it = od.m_outputdefinition.m_activeSourceObjectIds.begin();
				it != od.m_outputdefinition.m_activeSourceObjectIds.end(); ++it ) {
				if(*it == so.m_id) {
					// delete source object id
					od.m_outputdefinition.m_activeSourceObjectIds.erase(it);
					break;
				}
			}
		}
	}

	// set new state for output definition with updated source ids
	updateOutputDefinition(od, newState);

}

void SVSimulationOutputOptions::updateOutputUi(unsigned int row) {

	Q_ASSERT(row<m_outputDefinitions.size());

	OutputDefinition &od = m_outputDefinitions[row];

	QTableWidget &tw = *m_ui->tableWidgetSourceObjectIds;
	m_ui->textBrowserDescription->setText(QString::fromStdString(od.m_outputdefinition.m_description));

	if(od.m_outputdefinition.m_sourceObjectIds.size() == 1) {
		m_ui->widgetSource->setVisible(false);
		m_ui->verticalSpacer->changeSize(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding);
		m_ui->textBrowserDescription->setMaximumHeight(100);
	}
	else {
		m_ui->widgetSource->setVisible(true);
		m_ui->textBrowserDescription->setMaximumHeight(1670000);
		m_ui->verticalSpacer->changeSize(0,0, QSizePolicy::Fixed, QSizePolicy::Fixed);
		tw.clearContents(); // we also clear all previous contents
		tw.setRowCount(od.m_outputdefinition.m_sourceObjectIds.size() );
		m_ui->labelSourceObjects->setText("Select " + QString::fromStdString(od.m_outputdefinition.m_type) + "s");
		QTableWidgetItem *itemID, *itemName;
		for (unsigned int i=0; i<od.m_outputdefinition.m_sourceObjectIds.size(); ++i) {
			VICUS::SourceObject &so = od.m_outputdefinition.m_idToSourceObject[od.m_outputdefinition.m_sourceObjectIds[i]];

			// we construct new items
			itemID = new QTableWidgetItem();
			itemName = new QTableWidgetItem();

			// we set the font
			QFont f(m_font);
			f.setItalic(!so.m_isActive);
			f.setBold(od.m_isActive && so.m_isActive);

			itemID->setFont(f);
			itemName->setFont(f);

			// we set the text
			itemID->setText(QString::number(so.m_id));
			itemName->setText(QString::fromStdString(so.m_displayName));

			// we set the flags
			itemID->setFlags(itemID->flags()& ~Qt::ItemIsEditable);
			itemName->setFlags(itemID->flags()& ~Qt::ItemIsEditable);

			// we set our user role with our index
			itemID->setData(Qt::UserRole, i);
			itemName->setData(Qt::UserRole, i);

			// finally we set the items
			tw.setItem(i,0,itemID);
			tw.setItem(i,1,itemName);
		}
	}

}

void SVSimulationOutputOptions::updateOutputDefinition(OutputDefinition &od, bool active) {
	bool inList = false;

	od.m_isActive = active;

	if(active) {
		for (VICUS::OutputDefinition &vod : m_outputs->m_outputDefinitions) {
			if(vod.m_id == od.m_outputdefinition.m_id) {
				vod = od.m_outputdefinition; // update the definition
				inList = true;
				break;
			}
		}
		if(!inList)
			m_outputs->m_outputDefinitions.push_back(od.m_outputdefinition); // update the definition
	}
	else {
		for (unsigned int i =0; i<m_outputs->m_outputDefinitions.size(); ++i) {
			VICUS::OutputDefinition &vod = m_outputs->m_outputDefinitions[i];
			if(vod.m_id == od.m_outputdefinition.m_id) {
				m_outputs->m_outputDefinitions.erase(m_outputs->m_outputDefinitions.begin()+i); // erase the definition
				break;
			}
		}
		QModelIndex sourceIndex = m_outputTableProxyModel->mapToSource(m_ui->tableViewOutputList->currentIndex());
		Q_ASSERT(sourceIndex.row()<m_outputDefinitions.size());
		m_outputTableModel->updateOutputData((unsigned int)sourceIndex.row());
	}

}

void SVSimulationOutputOptions::on_toolButtonRemoveOutput_clicked() {
	QItemSelectionModel *selection = m_ui->tableViewOutputList->selectionModel();
	unsigned int cachedRow;

	m_itemIsSet = false;

	for (const QModelIndex & proxyIndex: m_ui->tableViewOutputList->selectionModel()->selectedRows()) {
		// configure new input var - requires valid selection
		Q_ASSERT(proxyIndex.isValid());

		// map 2 source index
		QModelIndex srcIndex = m_outputTableProxyModel->mapToSource(proxyIndex);

		unsigned int row = (unsigned int)srcIndex.row();
		Q_ASSERT(row < m_outputDefinitions.size());

		// we set our
		updateOutputDefinitionState(row, false);

		cachedRow = row;
	}

	updateOutputUi(cachedRow);
//	on_checkBoxShowActive_toggled(m_ui->checkBoxShowActive->isChecked());
}

void SVSimulationOutputOptions::on_checkBoxShowActive_toggled(bool checked){
	m_outputTableProxyModel->setFilterRole(Qt::UserRole);
	m_outputTableProxyModel->setFilterKeyColumn(0);
	m_itemIsSet = false;
	if(checked)
		m_outputTableProxyModel->setFilterWildcard("1");
	else
		m_outputTableProxyModel->setFilterWildcard("*");
}

void SVSimulationOutputOptions::on_comboBoxTimeType_currentIndexChanged(int index){

	try {
		IBK::UnitList::integralQuantity(m_activeOutputDefinition->m_outputdefinition.m_unit, false, true);
	}  catch (...) {
		QMessageBox::critical(this, QString(), tr("No time type 'integral' for unit '%1' allowed.\nUse 'none' or 'mean' as time type for output.").arg(QString::fromStdString(m_activeOutputDefinition->m_outputdefinition.m_unit.name())));
		m_ui->comboBoxTimeType->setCurrentIndex(NANDRAD::OutputDefinition::OTT_NONE);
		return;
	}

	const_cast<OutputDefinition*>(m_activeOutputDefinition)->m_outputdefinition.m_timeType = (VICUS::OutputDefinition::timeType_t)index;
}


void SVSimulationOutputOptions::on_toolButtonRemoveSource_clicked(){

	OutputDefinition &od = *const_cast<OutputDefinition*>(m_activeOutputDefinition);

	for (const QModelIndex & proxyIndex: m_ui->tableWidgetSourceObjectIds->selectionModel()->selectedRows()) {
		unsigned int row = (unsigned int)proxyIndex.row();

		VICUS::SourceObject &so = od.m_outputdefinition.m_idToSourceObject[m_ui->tableWidgetSourceObjectIds->item(row, 0)->text().toUInt()];
		so.m_isActive = false;

		for(std::vector<unsigned int>::iterator it = od.m_outputdefinition.m_activeSourceObjectIds.begin();
			it != od.m_outputdefinition.m_activeSourceObjectIds.end(); ++it ) {
			if(*it == so.m_id) {
				// delete source object id
				od.m_outputdefinition.m_activeSourceObjectIds.erase(it);
				break;
			}
		}

	}

	updateOutputDefinition(od, od.m_isActive && !od.m_outputdefinition.m_activeSourceObjectIds.empty());
	updateOutputUi(m_ui->tableViewOutputList->currentIndex().row());
	on_checkBoxShowActive_toggled(m_ui->checkBoxShowActive->isChecked());
}


void SVSimulationOutputOptions::on_toolButtonAddSource_clicked(){
	OutputDefinition &od = *const_cast<OutputDefinition*>(m_activeOutputDefinition);

	for (const QModelIndex & proxyIndex: m_ui->tableWidgetSourceObjectIds->selectionModel()->selectedRows()) {
		unsigned int row = (unsigned int)proxyIndex.row();

		VICUS::SourceObject &so = od.m_outputdefinition.m_idToSourceObject[m_ui->tableWidgetSourceObjectIds->item(row, 0)->text().toUInt()];
		so.m_isActive = true;

		bool inList = false;
		for(std::vector<unsigned int>::iterator it = od.m_outputdefinition.m_activeSourceObjectIds.begin();
			it != od.m_outputdefinition.m_activeSourceObjectIds.end(); ++it ) {
			if(*it == so.m_id) {
				// already in list of source object ids
				inList = true;
				break;
			}
		}

		if (!inList)
			od.m_outputdefinition.m_activeSourceObjectIds.push_back(so.m_id);
	}

	updateOutputUi(m_ui->tableViewOutputList->currentIndex().row());
	updateOutputDefinition(od, od.m_isActive && !od.m_outputdefinition.m_activeSourceObjectIds.empty());
	on_checkBoxShowActive_toggled(m_ui->checkBoxShowActive->isChecked());
}

