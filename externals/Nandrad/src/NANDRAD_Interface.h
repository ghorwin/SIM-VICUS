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

#ifndef NANDRAD_InterfaceH
#define NANDRAD_InterfaceH

#include <IBK_Parameter.h>
#include <IBK_Flag.h>

#include "NANDRAD_InterfaceAirFlow.h"
#include "NANDRAD_InterfaceHeatConduction.h"
#include "NANDRAD_InterfaceSolarAbsorption.h"
#include "NANDRAD_InterfaceLongWaveEmission.h"
#include "NANDRAD_InterfaceVaporDiffusion.h"
#include "NANDRAD_CodeGenMacros.h"

class TiXmlElement;

namespace NANDRAD {

class Zone;

/*!	\brief
	An Interface identifies a surface of a wall and stores all data that are needed for boundary
	condition calculation. Its position at the wall is identified by the type_t-attribute, either
	left or right side of the construction. The interface position corresponds to the layer assembly
	defined inside the construction type of the superior construction instance
	(the construction type itself defines the layer geometry from left to right).

	The zone at the other side of the wall surface is identified by its zone ID.
	The zone ID = 0 indicates ambient climate. If the construction is adiabatic,
	there must not be an interface defined for this side.

	HeatTransferResistance and Absorption coefficient are optional parameters. If no parameters are
	given they are copied from simulation parameter settings.

*/
class Interface {
public:

	enum location_t {
		IT_A = 0,		// Keyword: A				'Interface is situated at left side labeled A.'
		IT_B,			// Keyword: B				'Interface is situated at right side labeled B.'
		NUM_IT
	};

	/*! Boundary condition parameter blocks. */
	enum condition_t {
		IP_HEATCONDUCTION,		// Keyword: HeatConduction			'Heat conduction boundary condition.'
		IP_SOLARABSORPTION,		// Keyword: SolarAbsorption			'Short-wave solar absorption boundary condition.'
		IP_LONGWAVEEMISSION,	// Keyword: LongWaveEmission		'Long wave emission (and counter radiation) boundary condition.'
		IP_VAPORDIFFUSION,		// Keyword: VaporDiffusion			'Vapor diffusion boundary condition.'
		NUM_IP
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	Interface();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	//void readXML(const TiXmlElement * element);

	NANDRAD_READWRITE

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	//TiXmlElement * writeXML(TiXmlElement * parent) const;

	/*! Special form of comparison operator, tests if parameters that have an
		impact on result calculation are the same (zone, location, physical parameters).
	*/
	bool behavesLike(const Interface & other) const {
		return (m_location == other.m_location &&
				m_zoneId == other.m_zoneId &&
				m_heatConduction == other.m_heatConduction &&
				m_solarAbsorption == other.m_solarAbsorption &&
				m_longWaveEmission == other.m_longWaveEmission &&
				m_vaporDiffusion == other.m_vaporDiffusion);
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! ID of the referenced surface/interface. */
	unsigned int								m_id;					// XML:A:required
	/*! IBK-language encoded name of interface. */
	std::string									m_displayName;			// XML:A
	/*! The position of the interface, left ore right of the construction. */
	location_t									m_location;				//
	/*! The id number of the neighboring zone. */
	unsigned int								m_zoneId;				// XML:E
	/*! The name of the neighboring zone. */
	std::string									m_zoneDisplayName;		// XML:E

	/*! Enables the interface models. */
	IBK::Flag									m_condition[NUM_IP];	// XML:E

	// Boundary condition parameters
	/*! Model for heat transfer coefficient. */
	InterfaceHeatConduction						m_heatConduction;		// XML:E
	/*! Model for solar absorption coefficient. */
	InterfaceSolarAbsorption					m_solarAbsorption;		// XML:E
	/*! Model for long wave emissivity. */
	InterfaceLongWaveEmission					m_longWaveEmission;		// XML:E
	/*! Model for vapor diffusion. */
	InterfaceVaporDiffusion						m_vaporDiffusion;		// XML:E
	/*! Model for air flow calculation. */
	InterfaceAirFlow							m_airFlow;				// XML:E


	/*! Reference to neighbor zone.*/
	const Zone*									m_zoneRef;
};

} // namespace NANDRAD

#endif // InterfaceH
