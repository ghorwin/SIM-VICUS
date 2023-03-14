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
#include "NANDRAD_Constants.h"

namespace NANDRAD {

class Zone;

/*!	An Interface identifies a surface of a wall and stores all data that are needed for boundary
	condition calculation. Each ConstructionInstance contains two member variables of type
	Interface, which uniquely define the boundary condition parameters at side
	A or B of the construction (A - layer index 0, B - other side).

	The zone at the other side of the wall surface is identified by its zone ID.
	The zone ID = 0 indicates ambient climate. If the construction is adiabatic,
	there must not be an interface defined for this side.
*/
class Interface {
	NANDRAD_READWRITE_PRIVATE
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	void readXML(const TiXmlElement * element);
	TiXmlElement * writeXML(TiXmlElement * parent) const;

	NANDRAD_COMPARE_WITH_ID

	/*! Checks for valid parameters.
		\note This function throws an exception if invalid parameters are defined or parameters are missing.
	*/
	void checkParameters() const;

	/*! Special form of comparison operator, tests if parameters that have an
		impact on result calculation are the same (zone, location, physical parameters).
	*/
	bool behavesLike(const Interface & other) const {
		return (m_zoneId == other.m_zoneId &&
				m_heatConduction == other.m_heatConduction &&
				m_solarAbsorption == other.m_solarAbsorption &&
				m_longWaveEmission == other.m_longWaveEmission &&
				m_vaporDiffusion == other.m_vaporDiffusion);
	}

	/*! Updates the comment regarding the zone this interface is connected to. */
	void updateComment(const std::vector<Zone> & zones);

	/*! Returns true, if any of the parameters blocks have boundary conditions defined.
		If the model ID m_id is INVALID_ID, the function returns false (the default).
		A boundary condition parameter block counts as defined, if the modelType is not NUM_MT.
	*/
	bool haveBCParameters() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! ID of the referenced surface/interface. */
	unsigned int								m_id = INVALID_ID;		// XML:A:required
	/*! The id number of the neighboring zone. */
	unsigned int								m_zoneId = 0;			// XML:A

	// Boundary condition parameters
	/*! Model for convective heat transfer. */
	InterfaceHeatConduction						m_heatConduction;		// XML:E
	/*! Model for solar absorption. */
	InterfaceSolarAbsorption					m_solarAbsorption;		// XML:E
	/*! Model for long wave emissivity. */
	InterfaceLongWaveEmission					m_longWaveEmission;		// XML:E
	/*! Model for vapor diffusion (requires enabled moisture balance). */
	InterfaceVaporDiffusion						m_vaporDiffusion;		// XML:E
	/*! Model for air flow calculation. */
	InterfaceAirFlow							m_airFlow;				// XML:E

	/*! Comment, indicating the zone this interface links to.
		\warning This comment is set automatically when the project is written
				 with Project::writeXML(). Any existing text will be overwritten.
	*/
	std::string									m_comment;				// XML:C



	// *** Runtime variables ***

	/*! Stores pointer to connected zone */
	const Zone									*m_zone = nullptr;

	/*! Set to true if this is a side A interface of a construction instance.
		Initialized in ConstructionInstance::checkParameters().
	*/
	bool										m_isSideA;

	/*! Stores view factors for construction instances visible from this one, whereby the key is the id of the visible construction instance. */
	std::map<unsigned int, double>				m_viewFactors;

	/*! Stores pointer to interfaces visible from this one, whereby the key is the id of the according construction instance.
	 *  This is required to have access to the emission coefficient of the visible interface. */
	std::map<unsigned int, const NANDRAD::Interface*>	m_connectedInterfaces;
};

} // namespace NANDRAD

#endif // NANDRAD_InterfaceH
