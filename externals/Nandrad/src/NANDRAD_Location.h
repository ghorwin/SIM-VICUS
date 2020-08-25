/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

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
#include <IBK_Flag.h>
#include <IBK_Parameter.h>

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
		P_Latitude,					// Keyword: Latitude		[Deg]	'Latitude.'
		P_Longitude,				// Keyword: Longitude		[Deg]	'Longitude.'
		P_Albedo,					// Keyword: Albedo			[%]		'Albedo value [0..100].'
		P_Altitude,					// Keyword: Altitude		[m]		'Altitude of building as height above NN [m].'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Parameter set. */
	IBK::Parameter				m_para[NUM_P];							// XML:E

	/*! Name of the climate data file. */
	IBK::Path					m_climateFileName;						// XML:E

	/*! Optional: name of the external shading factor data file. */
	IBK::Path					m_shadingFactorFileName;				// XML:E

	/*! If enabled, Perez model for diffuse radiation is used. */
	IBK::Flag					m_perezDiffuseRadiationModel;			// XML:E

	/*! We may place one or more sensors outside. */
	std::vector<Sensor>			m_sensors;								// XML:E

};

} // namespace NANDRAD

#endif // NANDRAD_LocationH
