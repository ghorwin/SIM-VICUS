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

#ifndef SVSimulationStartNetworkSimH
#define SVSimulationStartNetworkSimH

#include <QDialog>
#include <QMap>

namespace Ui {
	class SVSimulationStartNetworkSim;
}

namespace NANDRAD {
	class Project;
}

#define PI				3.141592653589793238

/*! Dialog for starting a simulation.
	It generates a NANDRAD project based on the current VICUS project contents.
*/
class SVSimulationStartNetworkSim : public QDialog {
	Q_OBJECT

public:
	explicit SVSimulationStartNetworkSim(QWidget *parent = nullptr);
	~SVSimulationStartNetworkSim();

	/*! Starts the dialog and populates its content based on the current project's content. */
	void edit();

private slots:
	void on_checkBoxCloseConsoleWindow_toggled(bool checked);

	void on_pushButtonRun_clicked();

	void on_lineEditReferencePressure_editingFinished();

	void on_lineEditDefaultFluidTemperature_editingFinished();

private:
	void updateCmdLine();

	bool generateNandradProject(NANDRAD::Project & p) const;

	void modifyParameters();

	void updateLineEdits();

	void toggleRunButton();

	Ui::SVSimulationStartNetworkSim		*m_ui;

	QMap<QString, unsigned int>			m_networksMap;
	QString								m_solverExecutable;
	QStringList							m_cmdLine;
	QString								m_targetProjectFile;
};

#endif // SVSimulationStartNetworkSimH
