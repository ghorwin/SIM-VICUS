/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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

#ifndef VICUS_NetworkPipeH
#define VICUS_NetworkPipeH

#include <string>
#include <vector>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <IBK_MultiLanguageString.h>
#include <IBK_Parameter.h>

#include <QColor>

namespace VICUS {

class NetworkPipe : public AbstractDBElement {
public:

	enum para_t {
		/*! This is the diameter of the pipe, not including any insulation around it. */
		P_DiameterOutside,					// Keyword: DiameterOutside						[mm]	'Outer diameter (not including optional insulation)'
		P_ThicknessWall,					// Keyword: ThicknessWall						[mm]	'Pipe wall thickness'
		P_RoughnessWall,					// Keyword: RoughnessWall						[mm]	'Pipe wall surface roughness'
		P_ThermalConductivityWall,			// Keyword: ThermalConductivityWall				[W/mK]	'Thermal conductivity of pipe wall'
		P_HeatCapacityWall,					// Keyword: HeatCapacityWall					[J/kgK]	'Specific heat capaciy of pipe wall'
		P_DensityWall,						// Keyword: DensityWall							[kg/m3]	'Density of pipe wall'
		P_ThicknessInsulation,				// Keyword: ThicknessInsulation					[mm]	'Thickness of insulation around pipe'
		P_ThermalConductivityInsulation,	// Keyword: ThermalConductivityInsulation		[W/mK]	'Thermal conductivity of insulation'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Calculates the effective U-value per m length of pipe in [W/mK]. */
	double UValue() const;

	/*! Returns the inner pipe diameter in [m].
		\warning Parameters are not checked for validity. If used unchecked, result may be negative or zero.
	*/
	double diameterInside() const;

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	/*! Generates a display name from category and entered pipe dimensions. */
	IBK::MultiLanguageString nameFromData() const;


	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! A custom category name. */
	IBK::MultiLanguageString			m_categoryName;						// XML:A

	/*! Pipe parameters. */
	IBK::Parameter						m_para[NUM_P];						// XML:E

};


} // namespace VICUS

#endif // VICUS_NetworkPipeH
