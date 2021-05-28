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

#ifndef NANDRAD_ModelInputReferenceH
#define NANDRAD_ModelInputReferenceH

#include <string>
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*!	Defines a reference type enumeration, used to identify groups of objects with same global access type.
	\todo is the class scope still needed?
*/
class ModelInputReference {
public:

	/*! Types used to identify either models generated implicitly from parametrization blocks,
		or other generic models (MRT_MODEL).
		For each parametrization type that results in implicit models, for example Zone,
		ConstructionInstance, Interface etc. exactly one type identifier is defined.
	*/
	enum referenceType_t {
		MRT_LOCATION,				// Keyword: Location				'Model references of climate/location models.'
		MRT_ZONE,					// Keyword: Zone					'Model references inside a room.'
		MRT_CONSTRUCTIONINSTANCE,	// Keyword: ConstructionInstance	'Model references a wall.'
		MRT_EMBEDDED_OBJECT,		// Keyword: EmbeddedObject			'Model references an embedded object.'
		MRT_SCHEDULE,				// Keyword: Schedule				'Model references generic scheduled data that is not associated with a specific object type.'
		MRT_MODEL,					// Keyword: Model					'Model references of a model object.'
		MRT_NETWORK,				// Keyword: Network					'Model references of a hydraulic network.'
		MRT_NETWORKELEMENT,			// Keyword: NetworkElement			'Model references of flow elements of a hydraulic network.'
		NUM_MRT
	};

};

} // namespace NANDRAD

#endif // NANDRAD_ModelInputReferenceH
