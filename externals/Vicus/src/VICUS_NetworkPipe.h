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

#include <QColor>

namespace VICUS {

class NetworkPipe: public AbstractDBElement {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	double diameterInside() const{
		return m_diameterOutside - 2 * m_wallThickness;
	}

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int						m_id = INVALID_ID;				// XML:A:required
	/*! Display name of fluid. */
	IBK::MultiLanguageString			m_displayName;					// XML:A
	/*! Identification color. */
	QColor								m_color;						// XML:A

	///TODO Hauke bitte die variablen in ein array m_para[...] umbauen

	/*! Outside diameter pipe in [mm]. */
	double								m_diameterOutside;				// XML:A:required
	/*! Wall thickness pipe in [mm]. */
	double								m_wallThickness;				// XML:A:required
	/*! Pipe wall thermal conductivity in [W/mK] */
	double								m_lambdaWall;					// XML:A:required
	/*! Pipe wall roughness in [mm] */
	double								m_roughness;					// XML:A:required
	/*! outisde diameter of insulation in [mm] */
	double								m_insulationThickness = 0.0;	// XML:A
	/*! thermal conductivity of insulation in [W/mK] */
	double								m_lambdaInsulation = 0.0;		// XML:A

};


} // namespace VICUS

#endif // VICUS_NetworkPipeH
