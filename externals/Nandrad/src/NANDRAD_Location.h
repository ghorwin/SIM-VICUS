/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#ifndef NANDRAD_LocationH
#define NANDRAD_LocationH

#include <IBK_Path.h>
#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

#include "NANDRAD_Sensor.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	\brief Declaration for class Location

	The Location stores all references needed to specify climate and climate loads of the
	whole building. Note that an orientation can be given both for construction instances
	and as a global location parameter inducing a rotation twice.

	The climate is specified by a reference string ( climate not only depends
	on latitude and longitute but also detailed location information).
*/

class Location {
public:

	enum para_t {
		LP_LATITUDE,				// Keyword: Latitude		[Deg]	'Latitude.'
		LP_LONGITUDE,				// Keyword: Longitude		[Deg]	'Longitude.'
		LP_ALBEDO,					// Keyword: Albedo			[%]		'Albedo value [0..100].'
		LP_ALTITUDE,				// Keyword: Altitude		[m]		'Altitude of building as height above NN [m].'
		NUM_LP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Parameter set. */
	IBK::Parameter				m_para[NUM_LP];							// XML:E

	/*! Name of the climate data file. */
	IBK::Path					m_climateFileName;						// XML:E

	/*! Optional: name of the external shading factor data file. */
	IBK::Path					m_shadingFactorFileName;				// XML:E

	/*! We may place one or more sensors outside. */
	std::vector<Sensor>			m_sensors;								// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_LocationH
