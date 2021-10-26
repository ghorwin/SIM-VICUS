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

#include <IBK_Unit.h>

#include <NANDRAD_ObjectList.h>
#include <NANDRAD_OutputDefinition.h>
#include <NANDRAD_Project.h>

#include "SVSimulationOutputTableModel.h"

namespace Ui {
	class SVSimulationOutputOptions;
}

namespace VICUS {
	class Outputs;
}

/*! Widget with settings related to location. */
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



	/*! Updates user interface with properties from the project data structure.
		This function is called whenever the dialog is first shown.
	*/
	void updateUi();

	/*! Generates the table with all available output data to define output definitions in NANDRAD.
		Therefore the "$project_folder/var/$project_name/output_reference_list.txt" is parsed
		and all necesairry data is stored.
	*/
	void generateOutputTable();

	/*! Initialized Output Table with all necessairy headers */
	void initOutputTable(unsigned int rowCount);

	/*! Returns generated object lists. */
	std::vector<NANDRAD::ObjectList> objectLists();

	/*! Returns generated output definitions. */
	std::vector<NANDRAD::OutputDefinition> outputDefinitions();

private slots:
	void on_checkBoxDefaultZoneOutputs_toggled(bool checked);

	void on_checkBoxDefaultNetworkOutputs_toggled(bool checked);

	void on_radioButtonDefault_toggled(bool defaultToggled);

	void on_lineEdit_textEdited(const QString &filterKey);

	void on_tableViewOutputList_doubleClicked(const QModelIndex &index);

	void on_tableViewOutputList_clicked(const QModelIndex &index);

private:
	Ui::SVSimulationOutputOptions					*m_ui;

	/*! Pointer to VICUS Outputs */
	VICUS::Outputs									*m_outputs;

	/*! Vector with all parsed output definitions */
	std::vector<OutputDefinition>					m_outputDefinitions;

	/*! NANDRAD file with all data */
	NANDRAD::Project								m_nandradProject;

	/*! Table model instance for input vars. */
	SVSimulationOutputTableModel					*m_outputTableModel = nullptr;

	/*! Filter model for output definition */
	QSortFilterProxyModel							*m_outputTableProxyModel = nullptr;

	// =======================================================================================
	// ONLY NEEDED FOR NANDRAD IS APPLY WHEN DOUBLE CLICKED TO OUTPUT DEFINITION IN TABLE VIEW
	// =======================================================================================

	/*! Map that holds all the object lists that are needed for nandrad specific outputs */
	std::map<std::string, NANDRAD::ObjectList>			m_objectListsNandrad;

	/*! Map that holds all the output definitions that are needed for nandrad specific outputs*/
	std::map<std::string, NANDRAD::OutputDefinition>	m_outputDefinitionsNandrad;
};

#endif // SVSimulationOutputOptionsH
