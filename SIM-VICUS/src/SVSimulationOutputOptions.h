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

#ifndef SVSimulationOutputOptionsH
#define SVSimulationOutputOptionsH

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QTableWidgetItem>
#include <QItemSelectionModel>

#include <IBK_Unit.h>

#include <NANDRAD_ObjectList.h>
#include <NANDRAD_OutputDefinition.h>
#include <NANDRAD_Project.h>

#include <VICUS_OutputDefinition.h>
#include <VICUS_Outputs.h>

#include "SVSimulationOutputTableModel.h"


namespace Ui {
	class SVSimulationOutputOptions;
}

namespace VICUS {
	class Outputs;
}

class SVOutputGridEditDialog;
class ModificationInfo;
class SVSimulationStartOptions;

/*! Widget for specifying simulation outputs.
	The widget is split into two tabs - one for configuration of output grids and one for output definitions.

	Output grids are defined just as in DELPHIN in a table.

	OutputDefinitions, however, could be configured in different ways:

	a) default output definitions - can be created for many objects independently of the actual project, hereby using
	   generic object lists
	b) manual definition: here the NANDRAD solver export needs to be done first, then the output_reference_list.txt
	   is parsed and all actually available outputs are offered to the user. This procedure, however, is highly error prone
	   and can lead to a lot of problems:
	   - NANDRAD export can fail -> available output list cannot be generated/updated (may be outdated!)
	   - NANDRAD test init can fail

	For option b)  one has to consider, that the VICUS project may have changed and previously configured outputs
	won't be available anylonger. Hence, there should be a table showing configured outputs additionally to available
	outputs and highlight those that are no longer valid. This might include missing IDs (though NANDRAD only generates
	a warning in these cases).

	Option a and b can be used together (Option a is a check-box).

	General workflow for manual configuration:

	- open widget/dialog; clear table with available outputs, show only table with configured outputs
	- if "output_reference_list.txt" for current VICUS/NANDRAD Project exists, parse and read it
	- if time-stamp of VICUS project is newer than output_reference_list.txt, highlight "update available outputs" button
	- when user presses button "update available outputs" -> run NANDRAD export, NANDRAD test init -> report error if anything
	  fails
	- if successful, parse output reference list
	- populate table with available outputs
	- update status of already configured output definitions
*/
class SVSimulationOutputOptions : public QWidget {
	Q_OBJECT

public:
	SVSimulationOutputOptions(QWidget *parent);
	~SVSimulationOutputOptions();

	/*! Updates user interface with properties from the project data structure.
	*/
	void updateUi();

	/*! Pointer to simulation start widget */
	const SVSimulationStartOptions					*m_simulationStartOptions = nullptr;

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * /*data*/ );

private slots:

	/*! Triggered when user selects a different available output. */
	void onAvailableOutputSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

	/*! Triggered when user selects an output grid. */
	void on_tableWidgetOutputGrids_itemSelectionChanged();

	/*! Triggered, when user selects another output definition. */
	void on_tableWidgetOutputDefinitions_itemSelectionChanged();

	void on_pushButtonUpdateOutputList_clicked();

	void on_listWidgetObjectIDs_itemSelectionChanged();
	void on_listWidgetVectorIndexes_itemSelectionChanged();

	void on_toolButtonAddDefinition_clicked();
	void on_toolButtonRemoveDefinition_clicked();

	void on_toolButtonAddGrid_clicked();
	void on_toolButtonEditGrid_clicked();
	void on_toolButtonRemoveGrid_clicked();

	void on_tableViewAvailableOutputs_doubleClicked(const QModelIndex &index);

	/*! Triggered, when user manually edited a cell. */
	void on_tableWidgetOutputDefinitions_itemChanged(QTableWidgetItem *item);

	void on_checkBoxDefaultNetworkOutputs_clicked(bool checked);

	void on_checkBoxDefaultNetworkSummationModels_clicked(bool checked);

	void on_checkBoxDefaultBuildingOutputs_clicked(bool checked);

	void on_checkBoxBinaryFormat_clicked(bool checked);

	void on_lineEditReferenceType_textEdited(const QString &filterText);

	void on_lineEditQuantity_textEdited(const QString &filterText);

	void on_checkBoxEnableSelectVectorIndices_clicked(bool checked);

private:
	/*! Updates the output definition table, using the model with available outputs. */
	void updateOutputDefinitionTable();

	/*! Updates the output definition table, using the model with available outputs. */
	void updateOutputGridTable();

	/*! Updates the visibility of the "outdated output list" widget. */
	void updateOutdatedLabel();

	/*! Pointer to Ui */
	Ui::SVSimulationOutputOptions					*m_ui;

	/*! Table model instance that provides list with available output variables. */
	SVSimulationOutputTableModel					*m_outputTableModel = nullptr;

	/*! Filter model for output definition */
	QSortFilterProxyModel							*m_outputTableProxyModel = nullptr;

	/*! Edit dialog for output grids. */
	SVOutputGridEditDialog							*m_outputGridEditDialog = nullptr;

};

#endif // SVSimulationOutputOptionsH
