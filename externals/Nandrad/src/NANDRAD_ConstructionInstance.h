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

#ifndef NANDRAD_ConstructionInstanceH
#define NANDRAD_ConstructionInstanceH

#include <string>
#include <map>

#include "NANDRAD_Interface.h"
//#include "NANDRAD_EmbeddedObject.h"

namespace IBK {
	class Parameter;
}

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Defines a wall/floor/ceiling instance.

	A ConstructionInstance selects all information about a wall that are needed for wall
	temperature simulation and loads from windows.

	Geometry of the wall is stored inside the parameter list. For calculation needed are
	wall orientation and inclination, the complete wall area including windows dores and
	openings and the window are (area of all windows of the wall including frame). The
	window area always should be smaller than the wall area! A window area that equals
	the wall area is interpreted as a wall consisting of only windows without an
	construction behind.

	Each constructionInstance stores its surface information inside the interface vector.
	Maximum two interfaces are allowed, each of them characterized with the attribute
	either Interface::type_t::IT_LEFT or Interface::type_t::IT_RIGHT. A non-existent interface
	is equivalent to an adiabatic wall surface.

	Construction and window information is referenced due to a single construction type and
	window type id number. Only one window type and only one shading type are allowed. The
	shading type is automatically linked to all windows of the wall.
*/

class ConstructionInstance  {
public:
	// ***KEYWORDLIST-START***
	enum para_t {
		CP_ORIENTATION,					// Keyword: Orientation				[Deg]	'Orientation of the wall [deg].'
		CP_INCLINATION,					// Keyword: Inclination				[Deg]	'Inclination of the wall [deg].'
		CP_AREA,						// Keyword: Area					[m2]	'Gross area of the wall [m2].'
		NUM_CP
	};
	// ***KEYWORDLIST-END***


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	ConstructionInstance();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);
	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent, bool detailedOutput) const;
	/*! Writes comments at the beginning of ConstrucionInstance tag.
	*/
	void writeCommentsXML(TiXmlElement * parent) const;

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
	unsigned int				m_id;							// XML:A
	/*! IBK-language encoded name of construction instance. */
	std::string					m_displayName;					// XML:E
	/*! The id number of the corresponding construction type. */
	unsigned int				m_constructionTypeId;			// XML:E

	/*! List of parameters. */
	IBK::Parameter				m_para[NUM_CP];					// XML:E

	/*! The interfaces (connections of construction sides to zones).
		There can be maximum of 2 interfaces, one with the ID "Left" and one with the ID "Right". If either
		one is missing, this side of the construction is considered to be adiabatic (has same conditions as
		zone connected to with other interface).
		There must be at least one interface defined for a construction instance to be valid.
		\sa Interface
	*/
	std::vector<Interface>		m_interfaces;					// XML:E

	/*! All embedded objects. Embedded objects cut out an area of the current construction and substitute
		wall simulation by an explicit simulation model.
	*/
//	std::vector<EmbeddedObject>	m_embeddedObjects;

	/*! Names of the enclosing zones. */
	std::vector<std::string>	m_zoneNames;
};


} // namespace NANDRAD

#endif // ConstructionInstanceH
