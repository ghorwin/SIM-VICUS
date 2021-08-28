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

#ifndef VICUS_NetworkFluidH
#define VICUS_NetworkFluidH

#include <IBK_Parameter.h>
#include <NANDRAD_LinearSplineParameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <QColor>

namespace VICUS {

/*! Contains model parameters for fluid model. */
class NetworkFluid: public AbstractDBElement {
public:
	/*! Basic parameters. */
	enum para_t {
		/*! Density. */
		P_Density,					// Keyword: Density				[kg/m3]	'Dry density of the material.'
		/*! Specific heat capacity. */
		P_HeatCapacity,				// Keyword: HeatCapacity		[J/kgK]	'Specific heat capacity of the material.'
		/*! Thermal conductivity. */
		P_Conductivity,				// Keyword: Conductivity		[W/mK]	'Thermal conductivity of the dry material.'
		NUM_P
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	void defaultFluidWater(unsigned int id);

	bool isValid() const;

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;


	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! List of parameters. */
	IBK::Parameter						m_para[NUM_P];						// XML:E

	/*! Kinematic viscosity [m2/s]. */
	NANDRAD::LinearSplineParameter		m_kinematicViscosity;				// XML:E

};

} // namespace VICUS


#endif // VICUS_NetworkFluidH
