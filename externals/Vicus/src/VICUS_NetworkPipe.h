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

class NetworkPipe: public AbstractDBElement {
public:

	enum para_t{
		P_DiameterOutside,					// Keyword: DiameterOutside						[mm]	'Outer diameter'
		P_ThicknessWall,						// Keyword: ThicknessWall							[mm]	'Pipe wall thickness'
		P_RoughnessWall,						// Keyword: RoughnessWall							[mm]	'Pipe wall surface roughness'
		P_ThermalConductivityWall,			// Keyword: ThermalConductivityWall				[W/mK]	'Thermal conductivity of pipe wall'
		P_ThicknessInsulation,				// Keyword: ThicknessInsulation					[mm]	'Insulation thickness'
		P_ThermalConductivityInsulation,	// Keyword: ThermalConductivityInsulation		[W/mK]	'Thermal conductivity of insulation'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	double diameterInside() const{
		return m_para[P_DiameterOutside].value - 2 * m_para[P_ThicknessWall].value;
	}

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int						m_id = INVALID_ID;				// XML:A:required
	/*! Display name of fluid. */
	IBK::MultiLanguageString			m_displayName;					// XML:A
	/*! Identification color. */
	QColor								m_color;						// XML:A

	std::string							m_categoryName;					// XML:A

	IBK::Parameter						m_para[NUM_P];					// XML:E

};


} // namespace VICUS

#endif // VICUS_NetworkPipeH
