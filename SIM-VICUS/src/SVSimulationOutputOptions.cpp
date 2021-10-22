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

#include <VICUS_Outputs.h>
#include <VICUS_KeywordList.h>

#include <IBK_FileReader.h>

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

	QString nandradFilePath = SVProjectHandler::instance().nandradProjectFilePath();
	QString fileName = SVProjectHandler::instance().projectFile();

	int pos = fileName.lastIndexOf(".");
	fileName = fileName.left(pos);

	IBK::Path filePath(nandradFilePath.toStdString());
	IBK::Path file = IBK::Path(fileName.toStdString() ) / "var" / "output_reference_list.txt" ;

	qDebug() << file.c_str();

	std::vector<std::string> outputContent;
	try {
		IBK::FileReader::readAll(file, outputContent, std::vector<std::string>());
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(IBK::FormatString("Could not open file '%1' with output definitons.").arg(file.c_str()), FUNC_ID);
	}

	initOutputTable(outputContent.size()-1); // mind last column

	// now we go through all read lines
	std::vector<std::string> tokens;
	for (unsigned int i=0; i<outputContent.size(); ++i) {
		if ( i == 0 )
			continue;

		std::string &line = outputContent[i];
		IBK::explode(line, tokens, "\t", IBK::EF_NoFlags);

		QTableWidgetItem *item;
		for(unsigned int j=0; j<tokens.size(); ++j) {
			item = new QTableWidgetItem();
			QString trimmedString = QString::fromStdString(tokens[j]).trimmed();
			item->setText(trimmedString);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

			if (j == 0) {
				m_ui->tableWidgetOutputList->setItem(i-1, OT_VariableName, item);
			}
			else if (j == 1)
				m_ui->tableWidgetOutputList->setItem(i-1, OT_SourceObjectIds, item);
			else if (j == 2)
				m_ui->tableWidgetOutputList->setItem(i-1, OT_VectorIndexes, item);
			else if (j == 3)
				m_ui->tableWidgetOutputList->setItem(i-1, OT_Unit, item);
			else if (j == 4)
				m_ui->tableWidgetOutputList->setItem(i-1, OT_Description, item);
		}
	}
}

void SVSimulationOutputOptions::initOutputTable(unsigned int rowCount) {
	m_ui->tableWidgetOutputList->setColumnCount(5);
	m_ui->tableWidgetOutputList->setHorizontalHeaderLabels( QStringList() << tr("Name") << tr("Unit") << tr("Description") << tr("Source object id(s)") << tr("Vector indexes/ids") );
	m_ui->tableWidgetOutputList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputList->setColumnWidth(1, 100);
	m_ui->tableWidgetOutputList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	m_ui->tableWidgetOutputList->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputList->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputList->setRowCount(rowCount);

//	m_ui->tableWidgetOutputList->verticalHeader()->setDefaultSectionSize(19);
	m_ui->tableWidgetOutputList->verticalHeader()->setVisible(false);
	m_ui->tableWidgetOutputList->horizontalHeader()->setMinimumSectionSize(19);
	m_ui->tableWidgetOutputList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ui->tableWidgetOutputList->setSelectionMode(QAbstractItemView::SingleSelection);
	m_ui->tableWidgetOutputList->setAlternatingRowColors(true);
	m_ui->tableWidgetOutputList->setSortingEnabled(true);
//	m_ui->tableWidgetOutputList->sortByColumn(0, Qt::AscendingOrder);

	QString headerStyleSheet = QString("QHeaderView::section:horizontal {font-weight:bold;}");
	m_ui->tableWidgetOutputList->horizontalHeader()->setStyleSheet(headerStyleSheet);
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

	m_ui->tableWidgetOutputList->setEnabled(defaultToggled);
}
