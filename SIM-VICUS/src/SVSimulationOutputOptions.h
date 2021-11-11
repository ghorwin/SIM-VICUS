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
	SVSimulationOutputOptions(QWidget *parent, VICUS::Outputs & outputs);
	~SVSimulationOutputOptions();

	enum OutputType {
		OT_VariableName,
		OT_Unit,
		OT_Description,
		OT_SourceObjectIds,
		OT_VectorIndexes,
		NUM_OT
	};

	// EXAMPLE: ===============================================================================================================================================
	//
	//	(0)	Variable name                                     	Source object id(s) 	Vector indexes/ids  	Unit      	Description
	//	(1)	ConstructionInstance.ElementTemperature           	1                   	0                   	C         	Finite-volume mean element temperature
	//
	//=========================================================================================================================================================
	enum OutputReferenceListType {
		ORT_VariableName,
		ORT_SourceObjectIds,
		ORT_VectorIndexes,
		ORT_Unit,
		ORT_Description
	};

	/*! Updates user interface with properties from the project data structure.
		This function is called whenever the dialog is first shown.
	*/
	void updateUi();

#if 0
	/*! Generates the table with all available output data to define output definitions in NANDRAD.
		Therefore the "$project_folder/var/$project_name/output_reference_list.txt" is parsed
		and all necesairry data is stored.
	*/
	void generateOutputTable();

	/*! Initializes Output Table with all necessairy headers */
	void initOutputTable(unsigned int rowCount);

	/*! Populates m_objectListsNandrad and m_outputDefinitionsNandrad with all
		defined output definitions and object lists. */
	void generateOutputs(const std::vector<NANDRAD::ObjectList> & objectList);

	/*! Searches in m_objectListsNandrad for a corresponding object list
		if an corresponding object list is found 'objectList' is set to it
		\returns true if found, else false
	*/
	bool findEqualObjectList(NANDRAD::ObjectList &objectList);

	/*! Returns generated object lists in std::vector. */
	std::vector<NANDRAD::ObjectList> objectLists();

	/*! Returns generated output definitions in std::vector. */
	std::vector<NANDRAD::OutputDefinition> outputDefinitions();
#endif

private slots:

#if 0
	void on_checkBoxDefaultZoneOutputs_toggled(bool checked);

	void on_checkBoxDefaultNetworkOutputs_toggled(bool checked);

	void on_radioButtonDefault_toggled(bool defaultToggled);

	void on_lineEditType_textEdited(const QString &filterKey);

	void on_tableViewOutputList_doubleClicked(const QModelIndex &index);

	void on_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

	void on_pushButtonAllSources_clicked();

	void on_tableWidgetSourceObjectIds_itemDoubleClicked(QTableWidgetItem *item);

	void on_lineEditName_textEdited(const QString &arg1);

	void on_pushButtonAllSourcesDeselected_clicked();

	void on_comboBoxOutoutGrid_currentIndexChanged(int index);

	void on_toolButtonAddOutput_clicked();

	void on_toolButtonRemoveOutput_clicked();

	void on_checkBoxShowActive_toggled(bool checked);

	void on_comboBoxTimeType_currentIndexChanged(int index);

	void on_toolButtonRemoveSource_clicked();

	void on_toolButtonAddSource_clicked();
#endif
private:
#if 0
	/*! Finds the corresponding nandrad model by id and returns its name
		\param model Nandrad Model
		\param name Name of Nandrad Model, modified by function
		\returns true if corresponding model was found
	*/
	template<typename T>
	bool findNandradName(const std::vector<T> *model, const unsigned int idNandrad, std::string & name);

	/*! Activates output definition and all source objects */
	void updateOutputDefinitionState(unsigned int row, bool newState);

	/*! Update Output UI with Source Table. */
	void updateOutputUi(unsigned int row);

	/*! Sets Activation state of 'm_outputDefinitions' and updated 'm_outputs' with 'm_outputDefinitions'
		when it is activated
	*/
	void updateOutputDefinition(OutputDefinition &od, bool active = true);
#endif

	/*! Pointer to Ui */
	Ui::SVSimulationOutputOptions					*m_ui;

	/*! Pointer to VICUS::Outputs object in current project. */
	VICUS::Outputs									*m_outputs = nullptr;

#if 0
	/* Cached Selection model - needed to determine wether selection goes up or down
	   and to set the correct active output definition */
	QItemSelection									m_itemSelection;
	bool											m_itemIsSet = false;

	/*! Table model instance for input vars. */
	SVSimulationOutputTableModel					*m_outputTableModel = nullptr;

	/*! Filter model for output definition */
	QSortFilterProxyModel							*m_outputTableProxyModel = nullptr;

	/*! Pointer to active object in List */
	const OutputDefinition							*m_activeOutputDefinition = nullptr;

	// ============================================================================================
	//  ONLY NEEDED FOR NANDRAD; IS APPLIED WHEN OUTPUT DEFINITION IN TABLE VIEW IS DOUBLE CLICKED
	// ============================================================================================

	/*! Map that holds all the object lists that are needed for nandrad specific outputs */
	std::map<std::string, NANDRAD::ObjectList>			m_objectListsNandrad;

	/*! Map that holds all the output definitions that are needed for nandrad specific outputs*/
	std::map<std::string, NANDRAD::OutputDefinition>	m_outputDefinitionsNandrad;
#endif
};

#endif // SVSimulationOutputOptionsH
