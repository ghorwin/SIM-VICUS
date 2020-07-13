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

#ifndef NANDRAD_ZoneH
#define NANDRAD_ZoneH

#include <string>
#include <set>

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>
#include <IBK_Path.h>

class TiXmlElement;

namespace NANDRAD {

class SpaceType;

/*!	\brief Declaration for class Zone

	A zone defines a thermal zone/room with a single air temperature. The class Zone stores
	all properties needed to compute zone temperature from zone energy.

	Needed for the calculation is either floor area and height, or zone volume. If all parameters
	are given, the volume property is computed from floor area and height.

	Zones can be either Constant or Active. For constant zones, the temperature is assumed to
	be fixed/predefined whereas in Active zones the temperature is computed (i.e. included in
	the model's unknowns).

	A constant zone only needs the temperature parameter. If an Active zone has a temperature parameter,
	this is used as initial condition.
*/
class Zone {
public:

	typedef std::pair<unsigned int, unsigned int>  viewFactorPair;

	// ***KEYWORDLIST-START***
	/*! Type of zone. */
	enum zoneType_t {
		ZT_CONSTANT,				// Keyword: Constant		'Zone with constant temperatures.'
		ZT_ACTIVE,					// Keyword: Active			'Zone described by a temperature node in space.'
		ZT_DETAILED,				// Keyword: Detailed		'Zone with detailed temperature in time and space.'
		ZT_GROUND,					// Keyword: Ground			'Ground zone with temperatures from a CCD climate data file.'
		NUM_ZT
	};

	/*! Location of a constant zone. */
	enum location_t {
		ZL_INSIDE,					// Keyword: Inside			'Zone is inside a building.'
		ZL_GROUND,					// Keyword: Ground			'Zone represents ground.'
		ZL_OUTSIDE,					// Keyword: Outside			'Zone respresents ambient climate.'
		NUM_ZL
	};

	/*! Parameters of a zone. */
	enum para_t {
		ZP_TEMPERATURE,				// Keyword: Temperature				[C]		'Temperature of the zone if set constant, or initial temperature for active zones [C].'
		ZP_RELATIVE_HUMIDITY,		// Keyword: RelativeHumidity		[%]		'Relative humidity of the zone if set constant, or initial humidity for active zones [%].'
		ZP_CO2_CONCENTRATION,		// Keyword: CO2Concentration		[g/m3]	'CO2 concentration of the zone if set constant, or initial concentration for active zones [g/m3].'
		ZP_AREA,					// Keyword: Area					[m2]	'Area of the ground floor [m2].'
		ZP_HEIGHT,					// Keyword: Height					[m]		'Zone height [m].'
		ZP_VOLUME,					// Keyword: Volume					[m3]	'Zone volume [m3] (computed, if area and height are given).'
		ZP_HEATCAPACITY,			// Keyword: HeatCapacity			[J/K]	'Extra heat capacity [J/K].'
		NUM_ZP
	};

	/*! Optional integer parameters. */
	enum intpara_t {
		ZI_AIRMATERIALREFERENCE,	// Keyword: AirMaterialReference	[---]	'ID reference to a mdoel with air parameter calculations.'
		NUM_ZI
	};
	// ***KEYWORDLIST-END***


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	Zone();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);
	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent, bool detailedOutput) const;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of the zone. */
	unsigned int				m_id;

	/*! IBK-language encoded name of zone. */
	std::string					m_displayName;

	/*! SpaceType ID name of current zone. */
	std::string					m_spaceType;

	/*! Zone type (Constant, Active).
		\sa zoneType_t
	*/
	zoneType_t					m_zoneType;

	/*! Zone location (Inside, Ground, Outside).
	*/
	location_t					m_location;

	/*! Physical parameters describing the zone. */
	IBK::Parameter				m_para[NUM_ZP];

	/*! Physical integer parameters describing the zone. */
	IBK::IntPara				m_intpara[NUM_ZI];

	/*! Optional name of a CCD climate data file: only for zones of type 'Ground'. */
	IBK::Path					m_climateFileName;

	/*! Optional name of a CCD climate data file display name: only for zones of type 'Ground'. */
	std::string					m_climateFileDisplayName;

	/*! Optional: view factors for all inisde interfaces of the current zone. */
	std::vector<std::pair<viewFactorPair, double> >	m_viewFactors;

	/*! Pointer to the referenced space type property.
		Is set to NULL after reading. Points to the
		corresponding NANDRAD::SpaceType object inside the model section!
	*/
	const SpaceType				*m_spaceTypeRef;
}; // Zone


} // namespace NANDRAD

#endif // ZoneH
