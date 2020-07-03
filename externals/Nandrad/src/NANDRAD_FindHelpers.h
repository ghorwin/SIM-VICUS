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

#ifndef NANDRAD_FindHelpersH
#define NANDRAD_FindHelpersH

#include <string>

#include "NANDRAD_ConstructionInstance.h"
#include "NANDRAD_OutputDefinition.h"
#include "NANDRAD_EmbeddedObject.h"

namespace NANDRAD {

#if 0

/*! Helper class to find a NANDRAD data model definition by its ID-name.
	This functor can be used with types that have a public member variable
	named m_name.

\code
// illustrates usage with an OutputGrid definition
std::vector<OutputGrid>::iterator it = std::find(m_outputGrids.begin(),
												 m_outputGrids.end(),
												 FindByName<OutputGrid>("First floor") );
\endcode
*/
template <typename T>
class FindByName {
public:
	/*! Constructor, requires search name as argument. */
	FindByName(const std::string &name) : m_name(name)
	{
	}

	/*! Comparison operator, returns true if m_name member of object
		definition 'def' equals search name.
	*/
	bool operator()(const T & def) {
		if (def.m_name == m_name) return true;
		return false;
	}

	std::string	 m_name;
};


/*! Helper class to find a NANDRAD data model definition by its ID-number.
	This functor can be used with types that have a public member variable
	named m_id.

\code
// illustrates usage with a Zone definition
std::vector<Zone>::iterator it = std::find(m_zones.begin(),
										   m_zones.end(),
										   FindByID<Zone>(15) );
\endcode
*/
template <typename T>
class FindByID {
public:
	/*! Constructor, requires search name as argument. */
	FindByID(unsigned int id) : m_id(id)
	{
	}

	/*! Comparison operator, returns true if m_id member of object
		definition 'def' equals search id.
	*/
	bool operator()(const T & def) {
		if (def.m_id == m_id) return true;
		return false;
	}

	unsigned int m_id;
};


/*! Helper class to find construction instances that contain an interface with given ID.
\code
std::vector<ConstructionInstance>::iterator it = std::find(
	m_constructionInstances.begin(),
	m_constructionInstance.end(),
	FindConstructionInstanceByInterfaceID(222) );
\endcode
*/
class FindConstructionInstanceByInterfaceID {
public:
	FindConstructionInstanceByInterfaceID(unsigned int id) : m_id(id)
	{
	}

	bool operator()( const std::pair< const unsigned int, ConstructionInstance > & cInst) {
		// prepared for use of std::map
		std::vector<Interface>::const_iterator interfaceIt =
			cInst.second.m_interfaces.begin();

		for ( ; interfaceIt != cInst.second.m_interfaces.end(); ++interfaceIt)
			if (interfaceIt->m_id == m_id) return true;
		return false;
	}

	unsigned int m_id;
};


/*! Helper class to find construction instances that contain an embedded object with given ID.
\code
std::vector<ConstructionInstance>::iterator it = std::find(
	m_constructionInstances.begin(),
	m_constructionInstance.end(),
	FindConstructionInstanceByEmbedddedObjectID(524) );
\endcode
*/
class FindConstructionInstanceByEmbedddedObjectID {
public:
	FindConstructionInstanceByEmbedddedObjectID( unsigned int id ) : m_id(id)
	{
	}

	bool operator()( const std::pair< const unsigned int, ConstructionInstance > & cInst) const {
		// preparation for std::map
		std::vector<NANDRAD::EmbeddedObject>::const_iterator embeddedObjectIt =
			cInst.second.m_embeddedObjects.begin();

		for ( ; embeddedObjectIt != cInst.second.m_embeddedObjects.end(); ++embeddedObjectIt)
			if (embeddedObjectIt->m_id == m_id) return true;
		return false;
	}

	unsigned int m_id;
};


/*! Helper class to find construction instances that contain a thermal area with given ID.
\code
std::vector<ConstructionInstance>::iterator it = std::find(
	m_constructionInstances.begin(),
	m_constructionInstance.end(),
	FindConstructionInstanceByThermalAreaID(524) );
\endcode
*/
class FindConstructionInstanceByThermalAreaID {
public:
	FindConstructionInstanceByThermalAreaID( unsigned int id) : m_id(id)
	{
	}

	bool operator()( const std::pair< const unsigned int, ConstructionInstance > & cInst) const {
		// preparation for std::map
		std::vector<NANDRAD::EmbeddedObject>::const_iterator embeddedObjectIt =
			cInst.second.m_embeddedObjects.begin();

		for ( ; embeddedObjectIt != cInst.second.m_embeddedObjects.end(); ++embeddedObjectIt)
			if (embeddedObjectIt->m_id == m_id) return true;
		return false;
	}

	unsigned int m_id;
};


/*! Helper class to find construction instances that contain a certain contruction type ID.
\code
std::vector<ConstructionInstance>::iterator it = std::find(
	m_constructionInstances.begin(),
	m_constructionInstance.end(),
	FindConstructionInstanceByConstructionTypeID(524) );
\endcode
*/
class FindConstructionInstanceByConstructionTypeID {
public:
	FindConstructionInstanceByConstructionTypeID( unsigned int id) : m_id(id)
	{
	}

	bool operator()( const std::pair< const unsigned int, ConstructionInstance > & cInst) const {
			if (cInst.second.m_constructionTypeId == m_id) return true;
		return false;
	}

	unsigned int m_id;
};


/*! Helper class to find output definition that carries the given ID name.

\code
std::vector<OutputDefinition>::iterator it = std::find(
	m_definitions.begin(),
	m_definitions.end(),
	FindOutputDefinitionByGridName("Hourly Values") );
\endcode
*/
class FindOutputDefinitionByGridName {
public:
	FindOutputDefinitionByGridName( const std::string & idName ) : m_idName(idName)
	{
	}

	bool operator()( const OutputDefinition & cInst) const {
			return cInst.m_gridName == m_idName;
	}

	std::string m_idName;
};

#endif

} // namespace NANDRAD

#endif // NANDRAD_FindHelpersH
