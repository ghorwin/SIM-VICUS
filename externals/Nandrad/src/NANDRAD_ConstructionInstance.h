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

#ifndef NANDRAD_ConstructionInstanceH
#define NANDRAD_ConstructionInstanceH

#include "NANDRAD_Material.h"
#include "NANDRAD_Interface.h"
#include "NANDRAD_EmbeddedObject.h"
#include "NANDRAD_CodeGenMacros.h"

namespace IBK {
	class Parameter;
}

namespace NANDRAD {

class Project;
class ConstructionType;

/*!	Defines a wall/floor/ceiling construction instance.

	A ConstructionInstance contains all information about a wall and possibly embedded objects
	like windows.

	Note that the area parameter stores the gross area of the wall, including all embedded objects.
	Naturally, their area must not exceed the wall area.
	A window area that is equal to the wall area is interpreted as a wall consisting of only windows
	without a construction behind.

	Each construction instance stores its surface information inside interface data structures
	for side A and side B. Side A is besides construction layer with index 0.
	By default, an interface has no boundary condition information (model types are set to undefined)
	and hence no fluxes are calculated.
	There must be at least one interface with valid boundary condition parametrization for a construction
	instance to be valid. Not referenced wall constructions will still be calculated, as they may
	serve as storage medium or provide sensor data. However, if not needed, they should be removed
	to improve performance.
*/
class ConstructionInstance  {
public:

	/*! Construction-specific parameters required by several models. */
	enum para_t {
		/*! Orientation of the wall [deg]. */
		P_Orientation,					// Keyword: Orientation				[Deg]	'Orientation of the wall [deg].'
		/*! Inclination of the wall [deg]. */
		P_Inclination,					// Keyword: Inclination				[Deg]	'Inclination of the wall [deg].'
		/*! Gross area of the wall [m2]. */
		P_Area,							// Keyword: Area					[m2]	'Gross area of the wall [m2].'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Is called in case the given interface is connected to a zone and moreover has a long wave emission model defined. It then checks if all construction instances connected to the same zone
	 *  also have a long wave emission model defined and stores pointers to the connected interfaces. Also checks if all required view factors are given by the according zone. */
	void checkAndPrepareLongWaveHeatExchange(const Project & prj, Interface & ourInterface);

	/*! Checks for valid parameters and stores quick-access pointer to associated construction type.
		Material and ConstructionType objects have already been checked for correctness.
		\note This function throws an exception if invalid parameters are defined, parameters are missing, or
			the construction type ID is invalid/unknown.
	*/
	void checkParameters(const Project & prj);

	/*! A special form of comparison operator: tests if the construction would yield
		the same results as the other construction when being simulated.
		The test checks for:
		- same construction type ID
		- interface objects for locations A and B result in same behavior
	*/
	bool behavesLike(const ConstructionInstance & other) const;

	/*! If a valid interface parameter block exists at side A, this function returns the references zone ID, otherwise 0.
		Use this function to check if there is an actual zone (not the outside) connected at this interface.
	*/
	unsigned int interfaceAZoneID() const;

	/*! If a valid interface parameter block exists at side B, this function returns the references zone ID, otherwise 0.
		Use this function to check if there is an actual zone (not the outside) connected at this interface.
	*/
	unsigned int interfaceBZoneID() const;

	/*! Returns true if construction is connected on either side via existing Interface definition to
		the zone with given zoneID.
	*/
	bool connectedTo(unsigned int zoneId) const {
		return ( (m_interfaceA.m_id != INVALID_ID && m_interfaceA.m_zoneId == zoneId) ||
				 (m_interfaceB.m_id != INVALID_ID && m_interfaceB.m_zoneId == zoneId) );
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int				m_id = INVALID_ID;				// XML:A:required
	/*! IBK-language encoded name of construction instance. */
	std::string					m_displayName;					// XML:A
	/*! The id number of the corresponding construction type. */
	unsigned int				m_constructionTypeId;			// XML:E:required

	/*! List of parameters. */
	IBK::Parameter				m_para[NUM_P];					// XML:E

	/*! The interface at side A. */
	Interface					m_interfaceA;					// XML:E:tag=InterfaceA
	/*! The interface at side B. */
	Interface					m_interfaceB;					// XML:E:tag=InterfaceB

	/*! All embedded objects. Embedded objects cut out an area of the current construction and substitute
		wall simulation by an explicit simulation model.
	*/
	std::vector<EmbeddedObject>	m_embeddedObjects;				// XML:E

	// *** Variables used only during simulation ***

	/*! Cached net transfer area [m2] for heat conduction to room, calculated as difference between P_Area parameter
		and sum of all embedded object areas (updated in checkParameters()).
	*/
	double						m_netHeatTransferArea = 999;

	/*! Quick-access pointer to the underlying construction type. */
	const ConstructionType		*m_constructionType = nullptr;



};


} // namespace NANDRAD

#endif // NANDRAD_ConstructionInstanceH
