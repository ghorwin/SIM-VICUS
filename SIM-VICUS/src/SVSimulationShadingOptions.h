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

#ifndef SVSimulationShadingOptionsH
#define SVSimulationShadingOptionsH

#include <QDialog>
#include <QProgressDialog>
#include <QElapsedTimer>

#include <NANDRAD_SimulationParameter.h>
#include <VICUS_Project.h>

namespace Ui {
	class SVSimulationShadingOptions;
}

namespace SH {
	class StructuralShading;
}

class ModificationInfo;

/*! The widget holds all data needed for pre calculated shading factors.
	The pre-computed shading data depends on:
	- (selection of/all) surfaces and their geometry (and subsurface geometry)
	- (selection of/all) obstacles and their geometry
	- simulation time frame
	- generation options (grid width, sun cone radius)

	All these parameters are merged into a single hash code which is used as part of the resulting
	shading file name.
*/
class SVSimulationShadingOptions : public QWidget {
	Q_OBJECT

public:
	explicit SVSimulationShadingOptions(QWidget *parent);
	~SVSimulationShadingOptions();

	/*! Defines the Calculation Method. */
	enum CalculationMethod {
		RayTracing,
		SurfaceClipping
	};

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
		Manual			///< manually defined parameters
	};

	/*! Defines the Extrusion type for the Geometry. */
	enum GeometryType {
		Flat,
		Extruded
	};

	/*! Here, we update our file name based on current selection. */
	void updateUi();

	/*! This function takes the current input and computes a hash value based on the selected data.
		Then, it composes a file name and checks if a file with this hash code in the name exists already.
		If yes, the function returns the filename of this file.
		If different files with same hash-code, yet different extensions are found, it takes preference in
		order d6b, d6o, tsv.
	*/
	bool checkForCurrentShadingFile() const;

	/*! Defines whether to use "all geometry" or "only selected geometry". */
	bool m_useOnlySelectedSurfaces = false;

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified(int modificationType, ModificationInfo *);

private slots:
	void on_pushButtonCalculate_clicked();

	void on_comboBoxCalculationMethod_activated(int);

	void on_comboBoxPresets_activated(int);

	void on_comboBoxGeometryMode_activated(int);

	void on_comboBoxFileType_activated(int);

	void on_toolButtonDeletePreviousShadingFile_clicked();

private:
	/*! Updates m_shadingFactorBaseName and the line edit in the user interface base on the current
		combo box selection.
	*/
	void updateShadingFileName();

	/*! Function that calculates the shading factors for selected outside surfaces
		Returns false if some error occurred during creation of the NANDRAD project.
	*/
	void calculateShadingFactors();

	/*! Sets the simulation parameters grid size and cone deg */
	void updateSimulationParameters(const DetailType &dt);

	void updatePreviousSimulationFileValues();


	Ui::SVSimulationShadingOptions			*m_ui;								///< pointer to UI

	/*! Start time in seconds, updated in updateUi(). */
	IBK::Time 								m_startTime;
	/*! End time in seconds, updated in updateUi(). */
	IBK::Time 								m_endTime;
	/*! Duration in seconds, updated in updateUi(). */
	double									m_durationInSec;
	/*! Longitude in Deg, updated in updateUi(). */
	double									m_longitudeInDeg;
	/*! Latitude in Deg, updated in updateUi(). */
	double									m_latitudeInDeg;

	/*! Defines the choosen Geometry Type. */
	GeometryType							m_geometryType;

	std::vector<const VICUS::Surface*>		m_selSurfaces;						///< vector with selected surfaces
	std::vector<const VICUS::SubSurface*>	m_selSubSurfaces;						///< vector with selected surfaces
	std::vector<const VICUS::Surface*>		m_selObstacles;						///< vector with selected dump geometry (obstacles)

	QString									m_shadingFactorBaseName;			///< Holds the basename for the shading factor file.

	SH::StructuralShading					*m_shading = nullptr;				///< Owned, managed as pointer-to-object to hide SH namespace
};

#endif // SVShadingCalculationDialogH

