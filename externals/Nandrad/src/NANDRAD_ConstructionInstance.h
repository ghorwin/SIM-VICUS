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

#ifndef NANDRAD_ConstructionInstanceH
#define NANDRAD_ConstructionInstanceH

#include <string>

#include "NANDRAD_Interface.h"
//#include "NANDRAD_EmbeddedObject.h"
#include "NANDRAD_CodeGenMacros.h"

namespace IBK {
	class Parameter;
}

namespace NANDRAD {

class ConstructionType;

/*!	\brief Defines a wall/floor/ceiling construction instance.

	A ConstructionInstance selects all information about a wall that are needed for wall
	temperature simulation and loads from windows.

	Geometry of the wall is stored inside the parameter list. For calculation needed are
	wall orientation and inclination, the complete wall area including windows dores and
	openings and the window are (area of all windows of the wall including frame). The
	window area always should be smaller than the wall area! A window area that equals
	the wall area is interpreted as a wall consisting of only windows without an
	construction behind.

	Each construction instance stores its surface information inside interface data structures
	for side A and side B. Side A is besides construction layer index 0.
	By default, an interface has no boundary condition information (model types are set to undefined)
	and hence no fluxes are calculated.
	There must be at least one interface with valid boundary condition parametrization for a construction
	instance to be valid.

	Construction and window information is referenced due to a single construction type and
	window type id number. Only one window type and only one shading type are allowed. The
	shading type is automatically linked to all windows of the wall.
*/

class ConstructionInstance  {
public:

	/*! Construction-specific parameters required by several models. */
	enum para_t {
		CP_ORIENTATION,					// Keyword: Orientation				[Deg]	'Orientation of the wall [deg].'
		CP_INCLINATION,					// Keyword: Inclination				[Deg]	'Inclination of the wall [deg].'
		CP_AREA,						// Keyword: Area					[m2]	'Gross area of the wall [m2].'
		NUM_CP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	/*! Checks for valid parameters and stores quick-access pointer to associated construction type.
		\note This function throws an exception, if invalid parameters are defined, parameters are missing, or
			the construction type ID is invalid/unknown.
	*/
	void checkParameters(const std::vector<ConstructionType> & conTypes);

	/*! Returns an embedded object selected by its Id.
		Throws an exception if embedded object ID is not used in this construction instance.
	*/
//	const EmbeddedObject & embeddedObjectById( const unsigned int id) const;

	/*! A special form of comparison operator: tests if the construction would yield
		the same results as the other construction when being simulated.
		The test checks for:
		- same construction type ID
		- interface objects for locations A and B result in same behavior
	*/
	bool behavesLike(const ConstructionInstance & other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique id number. */
	unsigned int				m_id = INVALID_ID;				// XML:A:required
	/*! IBK-language encoded name of construction instance. */
	std::string					m_displayName;					// XML:A:
	/*! The id number of the corresponding construction type. */
	unsigned int				m_constructionTypeId;			// XML:E:required

	/*! List of parameters. */
	IBK::Parameter				m_para[NUM_CP];					// XML:E

	/*! The interface at side A. */
	Interface					m_interfaceA;					// XML:E:tag=InterfaceA
	/*! The interface at side B. */
	Interface					m_interfaceB;					// XML:E:tag=InterfaceB

	/*! All embedded objects. Embedded objects cut out an area of the current construction and substitute
		wall simulation by an explicit simulation model.
	*/
//	std::vector<EmbeddedObject>	m_embeddedObjects;

	// *** Variables used only during simulation ***

	/*! Quick-access pointer to the underlying construction type. */
	const NANDRAD::ConstructionType		*m_constructionType = nullptr;
};


} // namespace NANDRAD

#endif // NANDRAD_ConstructionInstanceH
