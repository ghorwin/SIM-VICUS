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
#include <VICUS_KeywordListQt.h>

#include <IBK_FileReader.h>
#include <IBK_UnitList.h>
#include <IBK_StringUtils.h>

#include <NANDRAD_KeywordList.h>


#include <QModelIndex>
#include <QAbstractTableModel>
#include <QMessageBox>
#include <QTextCodec>

#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "SVSimulationStartNandrad.h"

SVSimulationOutputOptions::SVSimulationOutputOptions(QWidget *parent, VICUS::Outputs & outputs, SVSimulationStartNandrad * simStartDialog) :
	QWidget(parent),
	m_ui(new Ui::SVSimulationOutputOptions),
	m_outputs(&outputs),
	m_simStartDialog(simStartDialog)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutOutputs->setMargin(0);

	// output grid table setup
	m_ui->tableWidgetOutputGrids->setColumnCount(4);
	m_ui->tableWidgetOutputGrids->setHorizontalHeaderLabels( QStringList() << tr("Name") << tr("Intervals") << tr("Start") << tr("End") );
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	m_ui->tableWidgetOutputGrids->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetOutputGrids);
	m_ui->tableWidgetOutputGrids->setSortingEnabled(false);

	// output definition table setup
	m_ui->tableWidgetOutputDefinitions->setColumnCount(6);
	m_ui->tableWidgetOutputDefinitions->setHorizontalHeaderLabels( QStringList() << tr("Quantity") << tr("Object type")
				<< tr("Object IDs") << tr("Vector IDs") << tr("Mean/Integral") << tr("Output grid")  );
	m_ui->tableWidgetOutputDefinitions->horizontalHeader()->setStretchLastSection(true);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetOutputDefinitions);
	m_ui->tableWidgetOutputDefinitions->setSortingEnabled(false);

	// create table model
	m_outputTableModel = new SVSimulationOutputTableModel(this);

	// create table model
	m_outputTableProxyModel = new QSortFilterProxyModel(this);
	m_outputTableProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	m_outputTableProxyModel->setSourceModel(m_outputTableModel);
	m_ui->tableViewAvailableOutputs->setModel(m_outputTableProxyModel);
	SVStyle::formatDatabaseTableView(m_ui->tableViewAvailableOutputs);

	m_ui->tableViewAvailableOutputs->setSelectionMode(QAbstractItemView::MultiSelection);
	m_ui->tableViewAvailableOutputs->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ui->tableViewAvailableOutputs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableViewAvailableOutputs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_ui->tableViewAvailableOutputs->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

	m_ui->tableViewAvailableOutputs->sortByColumn(1, Qt::AscendingOrder);

	// selection change signal
	connect(m_ui->tableViewAvailableOutputs->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &SVSimulationOutputOptions::onAvailableOutputSelectionChanged);

	m_ui->splitter->setStretchFactor(0, 1);
	m_ui->splitter->setStretchFactor(1, 2);
}


SVSimulationOutputOptions::~SVSimulationOutputOptions() {
	delete m_ui;
}


void SVSimulationOutputOptions::updateUi() {

	m_ui->checkBoxDefaultBuildingOutputs->setChecked(
				m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].isEnabled());

	m_ui->checkBoxDefaultNetworkOutputs->setChecked(
				m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].isEnabled());


	// *** output grids ***

	// disable selection-model signals
	m_ui->tableWidgetOutputGrids->selectionModel()->blockSignals(true);
	m_ui->tableWidgetOutputGrids->clearContents();
	m_ui->tableWidgetOutputGrids->setRowCount(m_outputs->m_grids.size());

	// we update the output grid
	for (unsigned int i=0; i<m_outputs->m_grids.size(); ++i) {
		const NANDRAD::OutputGrid & og = m_outputs->m_grids[i];
		QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(og.m_name));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->tableWidgetOutputGrids->setItem((int)i,0, item);

		try {
			// only populate start and end if intervals are all valid
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
	m_ui->tableWidgetOutputGrids->selectRow(0);
	m_ui->tableWidgetOutputGrids->selectionModel()->blockSignals(false);
	// update enabled status
	on_tableWidgetOutputGrids_itemSelectionChanged();


	// *** available outputs ***

	// now tell table model to update its output cache from the current output_reference_list.txt file
	// even though it might be outdated

	QFileInfo finfo(SVProjectHandler::instance().projectFile());
	QString fileName = finfo.dir().absoluteFilePath(finfo.completeBaseName()); // path to vicus project without .vicus or .nandrad
	fileName += "/var/output_reference_list.txt";

	m_outputTableModel->updateListFromFile(fileName);

	// show/hide the update info label
	updateOutdatedLabel();

	// *** existing output definitions ***

	updateOutputDefinitionTable();
}


void SVSimulationOutputOptions::onAvailableOutputSelectionChanged(const QItemSelection & selected, const QItemSelection & /*deselected*/) {
	m_ui->listWidgetObjectIDs->clear();
	m_ui->listWidgetVectorIndexes->clear();
	if (selected.isEmpty()) {
		m_ui->toolButtonAddDefinition->setEnabled(false);
		m_ui->listWidgetObjectIDs->setEnabled(false);
		m_ui->listWidgetVectorIndexes->setEnabled(false);
		return;
	}
	m_ui->listWidgetObjectIDs->setEnabled(true);

	// populate list widget with object IDs
	// populate list widget with vector IDs

	// check if we have vector indexes
	m_ui->listWidgetVectorIndexes->setEnabled(false);

	// trigger selection change of object/vector IDs list widgets to update plus buttons
}


void SVSimulationOutputOptions::on_tableWidgetOutputGrids_itemSelectionChanged() {
	// enable/disable buttons based on selection
	// Mind: you must not delete the default grid
	m_ui->toolButtonRemoveGrid->setEnabled(m_ui->tableWidgetOutputGrids->currentRow() != -1 && m_ui->tableWidgetOutputGrids->rowCount() > 1);
	m_ui->toolButtonEditGrid->setEnabled(m_ui->tableWidgetOutputGrids->currentRow() != -1);
}


void SVSimulationOutputOptions::on_tableWidgetOutputDefinitions_itemSelectionChanged() {
	// enable/disable buttons based on selection
	m_ui->toolButtonRemoveDefinition->setEnabled(m_ui->tableWidgetOutputDefinitions->currentRow() != -1);
}


void SVSimulationOutputOptions::on_pushButtonUpdateOutputList_clicked() {
	// run test-init and if successful, update output list
	if (!m_simStartDialog->startSimulation(true, true)) { // test-init and force background process
		return;
	}

	// now tell table model to update its output cache from the current output_reference_list.txt file
	// even though it might be outdated

	QFileInfo finfo(SVProjectHandler::instance().projectFile());
	QString fileName = finfo.dir().absoluteFilePath(finfo.completeBaseName()); // path to vicus project without .vicus or .nandrad
	fileName += "/var/output_reference_list.txt";

	m_outputTableModel->updateListFromFile(fileName);

	// finally update the table with available output definitions
	updateOutputDefinitionTable();

	updateOutdatedLabel();
}


void SVSimulationOutputOptions::updateOutputDefinitionTable() {

	m_ui->tableWidgetOutputDefinitions->selectionModel()->blockSignals(true);
	m_ui->tableWidgetOutputDefinitions->clearContents();
	m_ui->tableWidgetOutputDefinitions->setRowCount(m_outputs->m_definitions.size());

	QFont f(m_ui->tableWidgetOutputDefinitions->font());
	f.setStyle(QFont::StyleItalic);

	QColor badColor = Qt::gray; // the default, when there is not output_reference_list
	if (m_outputTableModel->rowCount(QModelIndex()) != 0)
		badColor = Qt::darkRed;
	// we update the output grid
	for (unsigned int i=0; i<m_outputs->m_definitions.size(); ++i) {
		const VICUS::OutputDefinition & of = m_outputs->m_definitions[i];

		// is this output being generated from the current project?
		bool correct = m_outputTableModel->haveOutput(of);

		QTableWidgetItem * item = new QTableWidgetItem(QString::fromStdString(of.m_quantity));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputGrids->setItem((int)i,0, item);


		item = new QTableWidgetItem(QString::fromStdString(of.m_sourceObjectType));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputGrids->setItem((int)i,1, item);

		// create list of comma-separated IDs
		std::string ids = IBK::join_numbers(of.m_sourceObjectIds, ',');
		item = new QTableWidgetItem(QString::fromStdString(ids));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputGrids->setItem((int)i,2, item);

		// create list of comma-separated IDs
		ids = IBK::join_numbers(of.m_vectorIds, ',');
		item = new QTableWidgetItem(QString::fromStdString(ids));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputGrids->setItem((int)i,3, item);

		VICUS::OutputDefinition::timeType_t tt = of.m_timeType;
		// we default to None
		if (tt == VICUS::OutputDefinition::NUM_OTT)
			tt = VICUS::OutputDefinition::OTT_NONE;
		QString ttypekw = VICUS::KeywordListQt::Keyword("OutputDefinition::timeType_t", of.m_timeType);
		item = new QTableWidgetItem(ttypekw);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputGrids->setItem((int)i,4, item);

		item = new QTableWidgetItem(QString::fromStdString(of.m_gridName));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputGrids->setItem((int)i,5, item);
	}
	m_ui->tableWidgetOutputDefinitions->selectRow(0);
	m_ui->tableWidgetOutputDefinitions->selectionModel()->blockSignals(false);
	// update enabled status
	on_tableWidgetOutputDefinitions_itemSelectionChanged();
}


void SVSimulationOutputOptions::updateOutdatedLabel() {
	QFileInfo finfo(SVProjectHandler::instance().projectFile());
	QString fileName = finfo.dir().absoluteFilePath(finfo.completeBaseName()); // path to vicus project without .vicus or .nandrad
	fileName += "/var/output_reference_list.txt";
	if (QFileInfo::exists(fileName) && QFileInfo::exists(SVProjectHandler::instance().projectFile()) ) {
		QDateTime outRefFile = QFileInfo(fileName).lastModified();
		QDateTime vicusFile = QFileInfo(SVProjectHandler::instance().projectFile()).lastModified();
		if (outRefFile < vicusFile || SVProjectHandler::instance().isModified())
			m_ui->labelOutputUpdateNeeded->setVisible(true);
		else
			m_ui->labelOutputUpdateNeeded->setVisible(false);
	}
	else {
		m_ui->labelOutputUpdateNeeded->setVisible(true);
	}
}
