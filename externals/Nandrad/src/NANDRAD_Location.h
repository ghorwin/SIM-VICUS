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

class TiXmlElement;

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
	// ***KEYWORDLIST-START***
	enum para_t {
		LP_LATITUDE,				// Keyword: Latitude		[Deg]	'Latitude.'
		LP_LONGITUDE,				// Keyword: Longitude		[Deg]	'Longitude.'
		LP_ALBEDO,					// Keyword: Albedo			[%]		'Albedo value [0..100].'
		LP_ALTITUDE,				// Keyword: Altitude		[m]		'Altitude of building as height above NN [m].'
		NUM_LP
	};
	// ***KEYWORDLIST-END***

	/*! Default constructor*/
	Location();

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);
	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent, bool detailedOutput) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Parameter set.
	*/
	IBK::Parameter				m_para[NUM_LP];

	/*! Name of the climate data file. */
	IBK::Path					m_climateFileName;

	/*! Optional: name of the eyternal shading factor data file. */
	IBK::Path					m_shadingFactorFileName;

	/*! We may place one or more sensors in the building outsude. */
	std::vector<Sensor>			m_sensors;

};

} // namespace NANDRAD

#endif // LocationH
