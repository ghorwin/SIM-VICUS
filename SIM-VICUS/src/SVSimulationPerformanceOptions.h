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

#ifndef SVSimulationPerformanceOptionsH
#define SVSimulationPerformanceOptionsH

#include <QWidget>

class ModificationInfo;

namespace QtExt {
	class ValidatingLineEdit;
}

namespace NANDRAD {
	class SolverParameter;
}

namespace Ui {
	class SVSimulationPerformanceOptions;
}

/*! Edit widget for performance-related solver options.  */
class SVSimulationPerformanceOptions : public QWidget {
	Q_OBJECT
public:
	/*! Constructor, takes solver settings object reference (object is stored in main start dialog). */
	explicit SVSimulationPerformanceOptions(QWidget *parent);
	~SVSimulationPerformanceOptions();

	/*! Updates user interface with properties from the project data structure.
		This function is called, whenever the dialog is first shown.
	*/
	void updateUi();

private slots:
	/*! Triggered whenever one of the combo boxes on this widget has changed index. */
	void currentIndexChanged(int index);

	void onModified(int modificationType, ModificationInfo */*data*/);

	void on_lineEditMaxOrder_editingFinishedSuccessfully();
	void on_lineEditNonLin_editingFinishedSuccessfully();
	void on_lineEditMaxKry_editingFinishedSuccessfully();
	void on_lineEditIterative_editingFinishedSuccessfully();
	void on_lineEditPreILU_editingFinishedSuccessfully();
	void on_lineEditInitialDT_editingFinishedSuccessfully();
	void on_lineEditMinDT_editingFinishedSuccessfully();

	void on_lineEditRelTol_editingFinished();

private:
	void parameterEditingFinished(int paraEnum, const QtExt::ValidatingLineEdit *edit);
	void intParameterEditingFinished(int paraEnum, const QtExt::ValidatingLineEdit *edit);

	/*! UI pointer. */
	Ui::SVSimulationPerformanceOptions	*m_ui;
};

#endif // SVSimulationPerformanceOptionsH
