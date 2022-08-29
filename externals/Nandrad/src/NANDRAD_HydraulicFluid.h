/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NANDRAD_HydraulicFluidH
#define NANDRAD_HydraulicFluidH

#include <IBK_Parameter.h>

#include "NANDRAD_LinearSplineParameter.h"
#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

/*! Properties of a fluid within a network. */
class HydraulicFluid {
public:

	/*! Basic parameters. */
	enum para_t {
		P_Density,					// Keyword: Density				[kg/m3]	'Dry density of the material.'
		P_HeatCapacity,				// Keyword: HeatCapacity		[J/kgK]	'Specific heat capacity of the material.'
		P_Conductivity,				// Keyword: Conductivity		[W/mK]	'Thermal conductivity of the dry material.'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	/*! Populates the HydraulicFluid object with properties of water. */
	void defaultFluidWater();

	/*! Populates the HydraulicFluid object with properties of air. */
	void defaultFluidAir();

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters(int networkModelType);

	/*! Comparies objects by physical parametrization (excluding ID and displayname and object list). */
	bool equal(const HydraulicFluid & other) const;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Display name of fluid. */
	std::string							m_displayName;					// XML:A
	/*! List of parameters. */
	IBK::Parameter						m_para[NUM_P];					// XML:E

	/*! Kinematic viscosity [m2/s]. */
	LinearSplineParameter				m_kinematicViscosity;			// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicFluidH
