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

#ifndef SVSimulationModelOptionsH
#define SVSimulationModelOptionsH

#include <QWidget>

class QSpinBox;
class ModificationInfo;

namespace QtExt {
	class ValidatingLineEdit;
}

namespace NANDRAD {
	class SimulationParameter;
	class Location;
}

namespace Ui {
	class SVSimulationModelOptions;
}

/*! Edit widget with general simulation/model options.  */
class SVSimulationModelOptions : public QWidget {
	Q_OBJECT
public:
	/*! Constructor, takes solver settings object reference (object is stored in main start dialog). */
	explicit SVSimulationModelOptions(QWidget *parent);
	~SVSimulationModelOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called, whenever the dialog is first shown.
	*/
	void updateUi();

private slots:

	void onModified(int modificationType, ModificationInfo */*data*/);

	void on_comboBoxSolarDistributionModeltype_currentIndexChanged(int index);

	void on_lineEditInitialTemperature_editingFinished();

	void on_checkBoxUsePerez_toggled(bool checked);

	void on_checkBoxEnableMoistureBalance_toggled(bool checked);

	void onStyleChanged();

	void on_spinBoxSolarRadiationToFloor_editingFinished();

	void on_spinBoxSolarRadiationToRoofCeiling_editingFinished();

	void on_spinBoxSolarRadiationToWalls_editingFinished();

	void on_spinBoxSolarRadiationGainsDirectlyToRoomNode_editingFinished();

	void on_pushButtonPrecalculateViewFactors_clicked();

private:

	void modifySolarLoadParameters(int paraEnum, const QSpinBox * spnBox);

	/*! UI pointer. */
	Ui::SVSimulationModelOptions	*m_ui;

};

#endif // SVSimulationModelOptionsH
