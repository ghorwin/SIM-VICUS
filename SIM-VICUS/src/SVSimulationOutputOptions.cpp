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
#include <VICUS_KeywordList.h>

#include <IBK_FileReader.h>

#include <NANDRAD_KeywordList.h>


#include <QModelIndex>
#include <QAbstractTableModel>

#include "SVProjectHandler.h"

#include "SVStyle.h"

SVSimulationOutputOptions::SVSimulationOutputOptions(QWidget *parent, VICUS::Outputs & outputs) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationOutputOptions),
	m_outputs(&outputs)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutOutputs->setMargin(0);

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


	m_ui->tableWidgetSourceObjectIds->setColumnCount(2);
	m_ui->tableWidgetSourceObjectIds->setHorizontalHeaderLabels(QStringList() << tr("ID") << tr("Display Name") );
	m_ui->tableWidgetSourceObjectIds->setColumnWidth(0, 50);
	m_ui->tableWidgetSourceObjectIds->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

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
	m_ui->tableWidgetOutputGrids->setRowCount(m_outputs->m_grids.size());
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
			m_ui->comboBoxOutoutGrid->blockSignals(true);
			m_ui->comboBoxOutoutGrid->addItem(QString::fromStdString(og.m_name), i);
			m_ui->comboBoxOutoutGrid->blockSignals(false);
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

			if(m_outputs != nullptr)
				od.m_outputGrid = &m_outputs->m_grids[0];

			if (j == 0) {
				object = IBK::explode(trimmedString.toStdString(), '.', 2);
				od.m_type = QString::fromStdString(object[0]);
				od.m_name = QString::fromStdString(object[1]);
			}
			else if (j == 3)
				od.m_unit = IBK::Unit(trimmedString.toStdString());
			else if (j == 4)
				od.m_description = trimmedString;
			else if (j == 1) {
				od.m_sourceObjectIds.clear();
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

					od.m_sourceObjectIds.push_back(SourceObject(nandradId,name));
				}
			}
			else if (j == 2) {
				od.m_vectorIds.clear();
				std::vector<std::string> ids;
				IBK::explode(trimmedString.toStdString(), ids, ",", IBK::EF_NoFlags);
				for (std::string id : ids) {
					od.m_vectorIds.push_back(QString::fromStdString(id).toUInt() );
				}
			}
		}
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

	////	m_ui->tableWidgetOutputList->verticalHeader()->setDefaultSectionSize(19);
	//	m_ui->tableWidgetOutputList->verticalHeader()->setVisible(false);
	//	m_ui->tableWidgetOutputList->horizontalHeader()->setMinimumSectionSize(19);
	//	m_ui->tableWidgetOutputList->setSelectionBehavior(QAbstractItemView::SelectRows);
	//	m_ui->tableWidgetOutputList->setSelectionMode(QAbstractItemView::SingleSelection);
	//	m_ui->tableWidgetOutputList->setAlternatingRowColors(true);
	//	m_ui->tableWidgetOutputList->setSortingEnabled(true);
	////	m_ui->tableWidgetOutputList->sortByColumn(0, Qt::AscendingOrder);

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
	for (unsigned int i=0; i<m_outputDefinitions.size(); ++i) {

		const OutputDefinition &od = m_outputDefinitions[i];

		if(!od.m_isActive)
			continue;


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
		ol.m_name = od.m_type.toStdString() + "[";
		for (unsigned int i=0; i<od.m_sourceObjectIds.size(); ++i) {
			if(!od.m_sourceObjectIds[i].m_isActive)
				continue;
			ol.m_name += QString::number(od.m_sourceObjectIds[i].m_id).toStdString();
			if (i<od.m_sourceObjectIds.size()-1)
				ol.m_name += ",";
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
		for (const SourceObject &so : od.m_sourceObjectIds) {
			if (!so.m_isActive)
				continue;
			ol.m_filterID.m_ids.insert(so.m_id);
		}

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

		// handling for all output that also reference vectors
		if (!od.m_vectorIds.empty()) {
			for (unsigned int i=0; i<od.m_vectorIds.size(); ++i) {
					nod.m_quantity = od.m_name.toStdString() + "[" + QString::number(od.m_vectorIds[i]).toStdString() + "]";
					m_outputDefinitionsNandrad[nod.m_quantity] = nod;
			}
		}
		else {
			nod.m_quantity = od.m_name.toStdString();
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
}

void SVSimulationOutputOptions::on_tableViewOutputList_doubleClicked(const QModelIndex &index) {

	QModelIndex sourceIndex = m_outputTableProxyModel->mapToSource(index);
	OutputDefinition &od = m_outputDefinitions[sourceIndex.row()];

	Q_ASSERT(sourceIndex.row()<m_outputDefinitions.size());

	bool outputDefinitionState = od.m_isActive;

	od.m_isActive = !outputDefinitionState;
	m_outputTableModel->updateOutputData(sourceIndex.row());


	QTableWidget &tw = *m_ui->tableWidgetSourceObjectIds;
	tw.setSortingEnabled(false);
	QTableWidgetItem *itemID, *itemName;
	for (unsigned int i=0; i<od.m_sourceObjectIds.size(); ++i) {
		SourceObject &so = od.m_sourceObjectIds[i];
		so.m_isActive = !outputDefinitionState;

		itemID = new QTableWidgetItem();
		itemName = new QTableWidgetItem();

		itemID->setText(QString::number(od.m_sourceObjectIds[i].m_id));
		itemName->setText(QString::fromStdString(od.m_sourceObjectIds[i].m_displayName));

		tw.setItem(i,0,itemID);
		tw.setItem(i,1,itemName);

		QFont f(m_font);
		f.setItalic(!so.m_isActive);
		f.setBold(od.m_isActive && so.m_isActive);


		itemID->setFont(f);
		itemName->setFont(f);

		itemID->setFlags(itemID->flags()& ~Qt::ItemIsEditable);
		itemName->setFlags(itemID->flags()& ~Qt::ItemIsEditable);

		itemID->setData(Qt::UserRole, i);
		itemName->setData(Qt::UserRole, i);
	}
	tw.setSortingEnabled(true);

	m_ui->tableWidgetSourceObjectIds->setEnabled(!outputDefinitionState);

	m_ui->comboBoxOutoutGrid->setCurrentText(od.m_name);

	// we also have to generate an output in the project
	// we need an hourly output grid, look if we have already one defined (should be!)
	m_outputTableModel->updateOutputData(sourceIndex.row());



}


void SVSimulationOutputOptions::on_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

	if(selected.empty())
		return;

	QTableWidget &tw = *m_ui->tableWidgetSourceObjectIds;
	tw.clearContents();
	tw.setSortingEnabled(false);
	QModelIndex sourceIndex = m_outputTableProxyModel->mapToSource(selected.indexes()[0]);

	const OutputDefinition &od = m_outputDefinitions[sourceIndex.row()];
	m_ui->labelSourceObjects->setText("Select " + od.m_type + "s");

	updateOutputUi(sourceIndex.row());

	tw.setEnabled(od.m_isActive);
	m_activeOutputDefinition = &od;
	tw.setSortingEnabled(true);
	tw.sortByColumn(1, Qt::AscendingOrder);
}

void SVSimulationOutputOptions::on_pushButtonAllSources_clicked() {
	OutputDefinition *od = const_cast<OutputDefinition*>(m_activeOutputDefinition);
	for (unsigned int i=0; i<od->m_sourceObjectIds.size(); ++i) {

		od->m_sourceObjectIds[i].m_isActive = true;

		QTableWidgetItem *itemID = m_ui->tableWidgetSourceObjectIds->item(i, 0);
		QTableWidgetItem *itemName = m_ui->tableWidgetSourceObjectIds->item(i, 1);

		QFont f(m_font);
		f.setItalic(!od->m_sourceObjectIds[i].m_isActive);
		f.setBold(od->m_sourceObjectIds[i].m_isActive);
		itemID->setFont(f);
		itemName->setFont(f);

	}
}

void SVSimulationOutputOptions::on_tableWidgetSourceObjectIds_itemDoubleClicked(QTableWidgetItem *item) {

	unsigned int row = item->row();

	QTableWidgetItem *itemID = m_ui->tableWidgetSourceObjectIds->item(row, 0);
	QTableWidgetItem *itemName = m_ui->tableWidgetSourceObjectIds->item(row, 1);

	const SourceObject &so = m_activeOutputDefinition->m_sourceObjectIds[itemID->data(Qt::UserRole).toUInt()];

	const_cast<SourceObject &>(so).m_isActive = !so.m_isActive;

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
}

void SVSimulationOutputOptions::on_pushButtonAllSourcesDeselected_clicked(){
	OutputDefinition *od = const_cast<OutputDefinition*>(m_activeOutputDefinition);
	for (unsigned int i=0; i<od->m_sourceObjectIds.size(); ++i) {

		od->m_sourceObjectIds[i].m_isActive = false;

		QTableWidgetItem *itemID = m_ui->tableWidgetSourceObjectIds->item(i, 0);
		QTableWidgetItem *itemName = m_ui->tableWidgetSourceObjectIds->item(i, 1);

		QFont f(m_font);
		f.setItalic(!od->m_sourceObjectIds[i].m_isActive);
		f.setBold(od->m_sourceObjectIds[i].m_isActive);
		itemID->setFont(f);
		itemName->setFont(f);

	}
}

void SVSimulationOutputOptions::on_comboBoxOutoutGrid_currentIndexChanged(int index){
	Q_ASSERT(index < m_outputs->m_grids.size());

	const_cast<OutputDefinition*>(m_activeOutputDefinition)->m_outputGrid = &m_outputs->m_grids[index];
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
}

void SVSimulationOutputOptions::updateOutputDefinitionState(unsigned int row, bool newState) {
	Q_ASSERT(row<m_outputDefinitions.size());

	OutputDefinition &od = m_outputDefinitions[row];

	// set new state for output definition
	od.m_isActive = newState;

	// and we also set the states to our source objects
	for (unsigned int i=0; i<od.m_sourceObjectIds.size(); ++i) {
		SourceObject &so = od.m_sourceObjectIds[i];
		so.m_isActive = newState;
	}
}

void SVSimulationOutputOptions::updateOutputUi(unsigned int row) {

	Q_ASSERT(row<m_outputDefinitions.size());

	OutputDefinition &od = m_outputDefinitions[row];

	QTableWidget &tw = *m_ui->tableWidgetSourceObjectIds;
	m_ui->textBrowserDescription->setText(od.m_description);

	if(od.m_sourceObjectIds.size() == 1) {
		m_ui->widgetSource->setVisible(false);
	}
	else {
		m_ui->widgetSource->setVisible(true);
		tw.clearContents(); // we also clear all previous contents
		tw.setRowCount(od.m_sourceObjectIds.size() );
		m_ui->labelSourceObjects->setText("Select " + od.m_type + "s");
		QTableWidgetItem *itemID, *itemName;
		for (unsigned int i=0; i<od.m_sourceObjectIds.size(); ++i) {
			SourceObject &so = od.m_sourceObjectIds[i];

			// we construct new items
			itemID = new QTableWidgetItem();
			itemName = new QTableWidgetItem();

			// we set the font
			QFont f(m_font);
			f.setItalic(!od.m_sourceObjectIds[i].m_isActive);
			f.setBold(od.m_isActive && od.m_sourceObjectIds[i].m_isActive);

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

void SVSimulationOutputOptions::on_toolButtonRemoveOutput_clicked() {
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
		updateOutputDefinitionState(row, false);

		cachedRow = row;
	}

	updateOutputUi(cachedRow);
	on_checkBoxShowActive_toggled(m_ui->checkBoxShowActive->isChecked());
}

void SVSimulationOutputOptions::on_checkBoxShowActive_toggled(bool checked){
	m_outputTableProxyModel->setFilterRole(Qt::UserRole);
	m_outputTableProxyModel->setFilterKeyColumn(0);
	if(checked)
		m_outputTableProxyModel->setFilterWildcard("1");
	else
		m_outputTableProxyModel->setFilterWildcard("*");
}
