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

#ifndef NANDRAD_ModelInputReferenceH
#define NANDRAD_ModelInputReferenceH

#include <string>
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

class ObjectList;

/*!	\brief Declaration for class ModelInputReference

	References an input quantity for some model. Such a quantity is uniquely identified through:
	* type of input source (for example a zone, construction instance, ...)
	* source name, which is the keyword of the quantity defined as computed output quantity in the source model
	* target name, which is the keyword of the input variable in the target model.
	* dependent on type, the unique ID of the source model instance

	When zone quantites (for example air temperatures) are referenced, the ID must be the zone id, except for
	outside zones (MRT_OUTSIDE), where the ID is implicitely 0.

	When referencing surface properties, there is a distinction between inside and outside surfaces. The
	outside surface will have additional quantities, for example net solar radiation flux (e.g. usable by
	a shading model).

	The construction instance can be referenced to retrieve information about active components
	(wall layer heating ...) or to retrieve temperature profiles etc.

	Interfaces (MRT_INTERFACE) can be queried for wall surface temperatures.

	\note Do not use as storage member, this is purely an internal class with keyword list support.

	\todo Review this class and check whether this class is needed as such or whether the reference type
		enum should rather be moved to NandradSolver.
*/
class ModelInputReference {
public:

	/*! Types used to identify either models generated implicitly from parametrization blocks,
		or other generic models (MRT_MODEL).
		For each parametrization type that results in implicit models, for example Zone,
		ConstructionInstance, Interface etc exactly one type identifier is defined.
	*/
	enum referenceType_t {
		MRT_LOCATION,				// Keyword: Location				'Model references of climate/location models.'
		MRT_ZONE,					// Keyword: Zone					'Model references inside a room.'
		MRT_CONSTRUCTIONINSTANCE,	// Keyword: ConstructionInstance	'Model references a wall.'
		MRT_INTERFACE,				// Keyword: Interface				'Model references a wall surface.'
		MRT_EMBEDDED_OBJECT,		// Keyword: EmbeddedObject			'Model references an embedded object.'
		MRT_ACTIVE_OBJECT,			// Keyword: ActiveObject			'Model references an active object.'
		MRT_SENSOR,					// Keyword: Sensor					'Model references a sensor object.'
		MRT_SCHEDULE,				// Keyword: Schedule				'Model references scheduled data.'
		MRT_OBJECTLIST,				// Keyword: ObjectList				'Model references an object list-specific value.'
		MRT_MODEL,					// Keyword: Model					'Model references another generic model.'
		MRT_GLOBAL,					// Keyword: Global					'Model references to global physical quantities.'
		NUM_MRT
	};

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Name of the property inside the dependent (this) model. */
	std::string							m_targetName;
	/*! Name of the object list.*/
	std::string							m_objectList;
	/*! Name of the input reference */
	std::string							m_quantity;

	/*! True, if referenced quantity is to be treated constant during
		update cycles (essentially prevents adding a evaluation dependency).
	*/
	bool								m_constant = false;


	// *** Variables used only during simulation ***

	/*! Pointer to the referenced object list.
		Is set to zero after reading. Connect to the
		corresponding object list inside the model section!
		\todo deprecated?
	*/
	const NANDRAD::ObjectList			*m_objectListRef = nullptr;
};

} // namespace NANDRAD

#endif // NANDRAD_ModelInputReferenceH
