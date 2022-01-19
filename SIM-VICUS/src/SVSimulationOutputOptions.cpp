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

#include <SV_Conversions.h>

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
#include "SVSimulationOutputTableDelegate.h"
#include "SVOutputGridEditDialog.h"

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

	SVSimulationOutputTableDelegate * delegate = new SVSimulationOutputTableDelegate;
	delegate->m_outputs = m_outputs;
	m_ui->tableWidgetOutputDefinitions->setItemDelegate(delegate);

	// create table model
	m_outputTableModel = new SVSimulationOutputTableModel(this);

	// create table model
	m_outputTableProxyModel = new QSortFilterProxyModel(this);
	m_outputTableProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	m_outputTableProxyModel->setSourceModel(m_outputTableModel);
	m_ui->tableViewAvailableOutputs->setModel(m_outputTableProxyModel);
	SVStyle::formatDatabaseTableView(m_ui->tableViewAvailableOutputs);

	m_ui->tableViewAvailableOutputs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_ui->tableViewAvailableOutputs->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui->tableViewAvailableOutputs->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	m_ui->tableViewAvailableOutputs->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	m_ui->tableViewAvailableOutputs->sortByColumn(1, Qt::AscendingOrder);

	SVStyle::formatListView(m_ui->listWidgetObjectIDs);
	SVStyle::formatListView(m_ui->listWidgetVectorIndexes);

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

	m_ui->checkBoxDefaultNetworkSummationModels->setChecked(
				m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkSummationModels].isEnabled());


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

			QString start = QtExt::parameter2String(og.m_intervals[0].m_para[NANDRAD::Interval::P_Start]);

			item = new QTableWidgetItem(start);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			m_ui->tableWidgetOutputGrids->setItem((int)i,2, item);

			QString end;
			if (og.m_intervals.back().m_para[NANDRAD::Interval::P_End].name.empty() ||
				og.m_intervals.back().m_para[NANDRAD::Interval::P_End].value == 0.0)
			{
				end = tr("End of simulation");
			}
			else {
				end = QtExt::parameter2String(og.m_intervals.back().m_para[NANDRAD::Interval::P_End]);
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

	// set initial state for "no output ref list file available"
	m_ui->listWidgetObjectIDs->setEnabled(false);
	m_ui->listWidgetObjectIDs->clear();
	m_ui->listWidgetVectorIndexes->setEnabled(false);
	m_ui->listWidgetVectorIndexes->clear();
	m_ui->toolButtonAddDefinition->setEnabled(false);

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


// *** slots ***

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

	m_ui->listWidgetObjectIDs->blockSignals(true);

	// populate list widget with object IDs
	// populate list widget with vector IDs
	m_ui->listWidgetObjectIDs->clear();

	const QItemSelectionRange & first = selected.first();
	Q_ASSERT(!first.indexes().isEmpty());
	// get list of available object IDs
	QModelIndex proxyIdx = first.indexes().first();
	QModelIndex srcIdx = m_outputTableProxyModel->mapToSource(proxyIdx);
	// retrieve list of available objects
	std::set<unsigned int> ids = srcIdx.data(Qt::UserRole).value<std::set<unsigned int> >();
	const QString & objectType = srcIdx.data(Qt::UserRole + 2).toString();

	int objectTypeID = -1;
	if (objectType == "Zone")
		objectTypeID = 0;
	else if (objectType == "ConstructionInstance")
		objectTypeID = 1;

	for (unsigned int i : ids) {
		// lookup type-specific object and show respective display name
		QString displayName;
		switch (objectTypeID) {
			case 0 : {
				const VICUS::Room * ob = project().roomByID(i); // Mind: room ID, not unique ID
				if (ob != nullptr)
					displayName = ob->m_displayName;
			} break;

			case 1 : {
				// lookup component name
			} break;
		}

		QString label;
		if (displayName.isEmpty())
			label = QString("%1").arg(i);
		else
			label = QString("%1 '%2'").arg(i).arg(displayName);
		QListWidgetItem * item = new QListWidgetItem(label);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, i);
		m_ui->listWidgetObjectIDs->addItem(item);
	}

	m_ui->listWidgetObjectIDs->selectAll();

	// trigger selection change of object/vector IDs list widgets to update plus buttons
	// now populate the object IDs list
	m_ui->listWidgetObjectIDs->blockSignals(false);
	on_listWidgetObjectIDs_itemSelectionChanged();
}


void SVSimulationOutputOptions::on_tableWidgetOutputGrids_itemSelectionChanged() {
	// enable/disable buttons based on selection
	// Mind: you must not delete the default first grid
	m_ui->toolButtonRemoveGrid->setEnabled(m_ui->tableWidgetOutputGrids->currentRow() > 0 && m_ui->tableWidgetOutputGrids->rowCount() > 1);
	m_ui->toolButtonEditGrid->setEnabled(m_ui->tableWidgetOutputGrids->currentRow() != -1);
}


void SVSimulationOutputOptions::on_tableWidgetOutputDefinitions_itemSelectionChanged() {
	// enable/disable buttons based on selection
	m_ui->toolButtonRemoveDefinition->setEnabled(m_ui->tableWidgetOutputDefinitions->currentRow() != -1);
}


void SVSimulationOutputOptions::on_pushButtonUpdateOutputList_clicked() {
	// run test-init and if successful, update output list
	if (!m_simStartDialog->startSimulation(true, true)) { // test-init and force background process
		return; // failure, no change in dialog's state
	}

	// set initial state for "no output ref list file available or list is empty"
	m_ui->listWidgetObjectIDs->setEnabled(false);
	m_ui->listWidgetObjectIDs->clear();
	m_ui->listWidgetVectorIndexes->setEnabled(false);
	m_ui->listWidgetVectorIndexes->clear();
	m_ui->toolButtonAddDefinition->setEnabled(false);

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


void SVSimulationOutputOptions::on_listWidgetObjectIDs_itemSelectionChanged() {
	// user has selected one or more object indexes

	QList<QListWidgetItem*> sel = m_ui->listWidgetObjectIDs->selectedItems();

	if (sel.count() > 1) {
		// do not show any vector IDs, since we have multi-selection in objects
		m_ui->listWidgetVectorIndexes->setEnabled(false);
		m_ui->listWidgetVectorIndexes->clear();
		// also enable the button for configuring outputs
		m_ui->toolButtonAddDefinition->setEnabled(true);
		return;
	}

	// check if we have vector indexes
	const QModelIndexList & modList = m_ui->tableViewAvailableOutputs->selectionModel()->selectedRows();
	Q_ASSERT(!modList.isEmpty());
	QModelIndex proxyIndex = modList.first();
	QModelIndex srcIndex = m_outputTableProxyModel->mapToSource(proxyIndex);

	std::set<unsigned int> vectorIDs = srcIndex.data(Qt::UserRole + 1).value<std::set<unsigned int> >();

	if (vectorIDs.empty()) {
		// do not show any vector IDs, since we have multi-selection in objects
		m_ui->listWidgetVectorIndexes->setEnabled(false);
		m_ui->listWidgetVectorIndexes->clear();
		// also enable the button for configuring outputs
		m_ui->toolButtonAddDefinition->setEnabled(true);
		return;
	}

	// populate list widget
	m_ui->listWidgetVectorIndexes->clear();
	m_ui->listWidgetVectorIndexes->blockSignals(true);
	for (unsigned int i : vectorIDs) {
		QListWidgetItem * item = new QListWidgetItem(QString("%1").arg(i));
		item->setData(Qt::UserRole, i);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		m_ui->listWidgetVectorIndexes->addItem(item);
	}

	m_ui->listWidgetVectorIndexes->setEnabled(true);
	m_ui->listWidgetVectorIndexes->blockSignals(false);

	// now select the first element
	m_ui->listWidgetVectorIndexes->item(0)->setSelected(true);
}


void SVSimulationOutputOptions::on_toolButtonAddDefinition_clicked() {
	// create and append a new output definition with currently selected grid and default time type, which
	// is based on the quantity type

	VICUS::OutputDefinition def;

	// get selected output definition from table
	QModelIndex proxyIndex = m_ui->tableViewAvailableOutputs->currentIndex();

	def.m_sourceObjectType = proxyIndex.data(Qt::UserRole + 2).toString().toStdString();
	def.m_quantity = proxyIndex.data(Qt::UserRole + 3).toString().toStdString(); // we append the index later, if needed
	int currentGrid = m_ui->tableWidgetOutputGrids->currentRow();
	if (currentGrid != -1)
		def.m_gridName = m_ui->tableWidgetOutputGrids->item(currentGrid, 0)->text().toStdString();
	else
		def.m_gridName = "Default"; // how do we handle missing default grids?
	def.m_timeType = VICUS::OutputDefinition::OTT_NONE;
	// quantities with flux units get time-average as default
	QString unit = proxyIndex.data(Qt::UserRole+4).toString();
	if (unit == "W" || unit == "W/m2")
		def.m_timeType = VICUS::OutputDefinition::OTT_MEAN;

	// get selected objects
	for (const QListWidgetItem * i : m_ui->listWidgetObjectIDs->selectedItems())
		def.m_sourceObjectIds.push_back(i->data(Qt::UserRole).toUInt());

	// any vector IDs selected?
	if (m_ui->listWidgetVectorIndexes->count() != 0) {
		NANDRAD::IDGroup idGroup;

		for (const QListWidgetItem * i : m_ui->listWidgetVectorIndexes->selectedItems())
			idGroup.m_ids.insert(i->data(Qt::UserRole).toUInt());

		def.m_quantity = def.m_quantity + "[" + idGroup.encodedString() + "]";
	}

	m_outputs->m_definitions.push_back(def);
	updateUi();
}


void SVSimulationOutputOptions::on_toolButtonRemoveDefinition_clicked() {
	// get index of currently selected definition
	int currentDef = m_ui->tableWidgetOutputDefinitions->currentRow();
	Q_ASSERT(currentDef != -1);
	m_outputs->m_definitions.erase(m_outputs->m_definitions.begin()+currentDef);
	updateUi();
	currentDef = std::min(currentDef, m_ui->tableWidgetOutputDefinitions->rowCount()-1);
	if (currentDef != -1)
		m_ui->tableWidgetOutputDefinitions->selectRow(currentDef);
}


void SVSimulationOutputOptions::on_toolButtonAddGrid_clicked() {
	// spawn edit dialog if it does not exist yet
	if (m_outputGridEditDialog == nullptr)
		m_outputGridEditDialog = new SVOutputGridEditDialog(this);
	// create new condition
	NANDRAD::OutputGrid def;
	// compose unique identification name
	def.m_name = tr("New output grid").toStdString();
	def.m_name = IBK::pick_name(def.m_name, m_outputs->m_grids.begin(), m_outputs->m_grids.end());
	// call edit dialog
	bool success = m_outputGridEditDialog->edit(def, *m_outputs, -1);
	// if user confirmed dialog, create undo command
	if (success) {
		m_outputs->m_grids.push_back(def);
		updateUi();
		m_ui->tableWidgetOutputGrids->selectRow(m_ui->tableWidgetOutputGrids->rowCount()-1);
	}
}


void SVSimulationOutputOptions::on_toolButtonEditGrid_clicked() {
	if (m_outputGridEditDialog == nullptr)
		m_outputGridEditDialog = new SVOutputGridEditDialog(this);
	int defIdx = m_ui->tableWidgetOutputGrids->currentRow();
	Q_ASSERT((unsigned int)defIdx < m_outputs->m_grids.size());
	bool success = m_outputGridEditDialog->edit(m_outputs->m_grids[(unsigned int)defIdx], *m_outputs, defIdx);
	if (success) {
		updateUi();
		m_ui->tableWidgetOutputGrids->selectRow(defIdx);
	}
}


void SVSimulationOutputOptions::on_toolButtonRemoveGrid_clicked() {
	// get index of currently selected definition
	int currentDef = m_ui->tableWidgetOutputGrids->currentRow();
	Q_ASSERT(currentDef != -1);
	m_outputs->m_grids.erase(m_outputs->m_grids.begin()+currentDef);
	updateUi();
	currentDef = std::min(currentDef, m_ui->tableWidgetOutputGrids->rowCount()-1);
	if (currentDef != -1)
		m_ui->tableWidgetOutputGrids->selectRow(currentDef);
}


void SVSimulationOutputOptions::on_tableViewAvailableOutputs_doubleClicked(const QModelIndex &/*index*/) {
	// create output for currently selected row and all objects/vector indexes
	on_toolButtonAddDefinition_clicked();
}


void SVSimulationOutputOptions::on_tableWidgetOutputDefinitions_itemChanged(QTableWidgetItem *item) {
	unsigned int defIdx = (unsigned int)item->row();
	Q_ASSERT(m_outputs->m_definitions.size() > defIdx);
	switch (item->column()) {
		case 4 : {
			VICUS::OutputDefinition::timeType_t t;
			// determine index
			try {
				int i = VICUS::KeywordList::Enumeration("OutputDefinition::timeType_t", item->text().toStdString());
				t = (VICUS::OutputDefinition::timeType_t)i;
			} catch (...) {
				qDebug() << "Invalid option for time type";
				return; // something strange happened here?
			}
			m_outputs->m_definitions[defIdx].m_timeType = t;
		} break;

		case 5 :
			m_outputs->m_definitions[defIdx].m_gridName = item->text().toStdString();
	}
}


// *** private functions ***

void SVSimulationOutputOptions::updateOutputDefinitionTable() {

	m_ui->tableWidgetOutputDefinitions->selectionModel()->blockSignals(true);
	m_ui->tableWidgetOutputDefinitions->blockSignals(true);
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
		m_ui->tableWidgetOutputDefinitions->setItem((int)i,0, item);


		item = new QTableWidgetItem(QString::fromStdString(of.m_sourceObjectType));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputDefinitions->setItem((int)i,1, item);

		// create list of comma-separated IDs
		std::string ids = IBK::join_numbers(of.m_sourceObjectIds, ',');
		item = new QTableWidgetItem(QString::fromStdString(ids));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputDefinitions->setItem((int)i,2, item);

		// create list of comma-separated IDs
		ids = IBK::join_numbers(of.m_vectorIds, ',');
		item = new QTableWidgetItem(QString::fromStdString(ids));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputDefinitions->setItem((int)i,3, item);

		VICUS::OutputDefinition::timeType_t tt = of.m_timeType;
		// we default to None
		if (tt == VICUS::OutputDefinition::NUM_OTT)
			tt = VICUS::OutputDefinition::OTT_NONE;
		QString ttypekw = VICUS::KeywordListQt::Keyword("OutputDefinition::timeType_t", of.m_timeType);
		item = new QTableWidgetItem(ttypekw);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputDefinitions->setItem((int)i,4, item);

		item = new QTableWidgetItem(QString::fromStdString(of.m_gridName));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		// check if referenced output grid name exists
		if (std::find(m_outputs->m_grids.begin(), m_outputs->m_grids.end(), of.m_gridName) == m_outputs->m_grids.end()) {
			item->setTextColor(Qt::darkRed);
			item->setFont(f);
		}
		else if (!correct) {
			item->setTextColor(badColor);
			item->setFont(f);
		}
		m_ui->tableWidgetOutputDefinitions->setItem((int)i,5, item);
	}
	m_ui->tableWidgetOutputDefinitions->selectionModel()->blockSignals(false);
	m_ui->tableWidgetOutputDefinitions->blockSignals(false);
	m_ui->tableWidgetOutputDefinitions->resizeColumnToContents(0);
	m_ui->tableWidgetOutputDefinitions->resizeColumnToContents(1);
	m_ui->tableWidgetOutputDefinitions->resizeColumnToContents(5);

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



void SVSimulationOutputOptions::on_checkBoxDefaultNetworkOutputs_clicked(bool checked) {
	m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkOutputs].set("CreateDefaultNetworkOutputs", checked);
}

void SVSimulationOutputOptions::on_checkBoxDefaultNetworkSummationModels_clicked(bool checked) {
	m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultNetworkSummationModels].set("CreateDefaultNetworkSummationModels", checked);
}

void SVSimulationOutputOptions::on_checkBoxDefaultBuildingOutputs_clicked(bool checked) {
	m_outputs->m_flags[VICUS::Outputs::F_CreateDefaultZoneOutputs].set("CreateDefaultZoneOutputs", checked);
}

