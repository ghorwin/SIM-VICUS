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

#ifndef SVShadingCalculationDialogH
#define SVShadingCalculationDialogH

#include <QDialog>
#include <QProgressDialog>
#include <QElapsedTimer>

#include <NANDRAD_SimulationParameter.h>

#include <SVSimulationStartNandrad.h>

#include <SH_StructuralShading.h>


namespace Ui {
	class SVShadingCalculationDialog;
}

/*! The widget holds all data needed for pre calculated shading factors.
*/
class SVShadingCalculationDialog : public QDialog {
	Q_OBJECT

public:
	explicit SVShadingCalculationDialog(QWidget *parent = nullptr);
	~SVShadingCalculationDialog();

	/*! Defines the specific output type */
	enum OutputType {
		TsvFile,		///< write output to tsv file
		D6oFile,		///< write output to d6o file
		D6bFile			///< write output to d6b file
	};

	/*! Defines the detail type */
	enum DetailType {
		Fast,			///< fast calculation parameters
		Detailed,		///< detailed calculation paramtesers
		Manual,			///< manually defined parameters
		NUM_DT
	};

	int edit();

private slots:
	void on_pushButtonCalculate_clicked();

	void on_lineEditStartDate_editingFinished();

	void on_lineEditEndDate_editingFinished();

	void on_lineEditDuration_editingFinishedSuccessfully();

	void on_pushButtonChangeTime_clicked();

	void on_radioButtonFast_toggled(bool checked);

	void on_radioButtonManual_toggled(bool checked);

	void on_radioButtonDetailed_toggled(bool checked);

private:
	/*! Function that calculates the shading factors for selected outside surfaces
		Returns false if some error occurred during creation of the NANDRAD project.
	*/
	void calculateShadingFactors();


	void updateTimeFrameEdits();

	/*! Sets the simulation parameters grid size and cone deg */
	void setSimulationParameters(const DetailType &dt);

	/*! Checks if all boundary conditions are set. */
	bool checkLocationAndPeriodParameters();


	Ui::SVShadingCalculationDialog		*m_ui;								///< pointer to UI

	NANDRAD::SimulationParameter		*m_simulationParameter = nullptr;	///< stores pointer to NANDRAD simulation parameters
	SVSimulationStartNandrad			*m_simStartWidget = nullptr;		///< stores pointer to SV Simulation parameters

	std::vector<const VICUS::Surface*>	m_selSurfaces;						///< vector with selected surfaces
	std::vector<const VICUS::Surface*>	m_selObstacles;						///< vector with selected dump geometry (obstacles)

	/*! Local copy of our project data, modified in dialog and synced with global
		project when dialog is confirmed.
	*/
	VICUS::Project						m_localProject;

	SH::StructuralShading				m_shading;							///< shading object, that does the calculation
};

#endif // SVShadingCalculationDialogH
