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
		and all necesairry data is stored
	*/
	void generateOutputTable();

	/*! Initialized Output Table with all necessairy headers */
	void initOutputTable(unsigned int rowCount);

private slots:
	void on_checkBoxDefaultZoneOutputs_toggled(bool checked);

	void on_checkBoxDefaultNetworkOutputs_toggled(bool checked);

	void on_radioButtonDefault_toggled(bool defaultToggled);

private:
	Ui::SVSimulationOutputOptions		*m_ui;
	VICUS::Outputs						*m_outputs;
};

#endif // SVSimulationOutputOptionsH
