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

#ifndef NANDRAD_ZoneH
#define NANDRAD_ZoneH

#include <string>

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>
#include <IBK_Path.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

/*!	A zone defines a thermal zone/room with a single air temperature.

	The class Zone stores all properties needed to compute zone temperature from energy density (the conserved quantity).

	Zones can be either Constant or Active. For constant zones, the temperature is assumed to
	be fixed/predefined (given via parameter P_Temperature) whereas in Active zones the temperature
	is computed (i.e. included in the model's unknowns).
*/
class Zone {
public:

	/*! Type of zone. Defines whether zone is balanced and included in equation system. */
	enum type_t {
		/*! Constant model requires P_Temperature parameter. Optionally, a schedule with name 'TemperatureSchedule' can be used to
			identify temperature of the zone (or soil, if this is a ground zone).
		*/
		ZT_Constant,				// Keyword: Constant		'Zone with constant temperature'
		ZT_Scheduled,				// Keyword: Scheduled		'Zone with schedule defined temperature'
		ZT_Active,					// Keyword: Active			'Zone with energy balance equation'
		ZT_Ground,					// Keyword: Ground			'Ground zone (calculates temperature based on heat loss to ground model)'
		NUM_ZT
	};

	/*! Parameters of a zone. */
	enum para_t {
		P_Temperature,				// Keyword: Temperature				[C]		'Temperature of the zone if set constant'
		P_RelativeHumidity,			// Keyword: RelativeHumidity		[%]		'Relative humidity of the zone if set constant'
		P_CO2Concentration,			// Keyword: CO2Concentration		[g/m3]	'CO2 concentration of the zone if set constant'
		P_Area,						// Keyword: Area					[m2]	'Net usage area of the ground floor (for area-related outputs and loads)'
		P_Volume,					// Keyword: Volume					[m3]	'Zone air volume'
		P_HeatCapacity,				// Keyword: HeatCapacity			[J/K]	'Extra heat capacity'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE_PRIVATE
	NANDRAD_COMPARE_WITH_ID

	/*! Checks for valid and required parameters (value ranges). */
	void checkParameters() const;

	/*! Calls the generated readXMLPrivate and additionally reads the view factors */
	void readXML(const TiXmlElement * element);

	/*! Calls the generated writeXMLPrivate and additionally reads the view factors */
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of the zone. */
	unsigned int				m_id = INVALID_ID;					// XML:A:required

	/*! Descriptive name of zone. */
	std::string					m_displayName;						// XML:A

	/*! Zone type (Constant, Active).
		\sa type_t
	*/
	type_t						m_type = NUM_ZT;					// XML:A:required

	/*! Physical parameters describing the zone. */
	IBK::Parameter				m_para[NUM_P];						// XML:E

	/*! Optional: Used for view factor definition: pairs of contruction instance ids meaning the view factor from id to id. */
	typedef std::pair<unsigned int, unsigned int>  viewFactorPair;

	/*! Optional: View factors for all inside interfaces of the current zone. Contains the view factor for a pair of construction instance ids.
		The pair contains the IDs of the interconnected construction instances, the value is the view factor (0..1).
	*/
	std::vector<std::pair<viewFactorPair, double> >	m_viewFactors;

}; // Zone


} // namespace NANDRAD

#endif // NANDRAD_ZoneH
