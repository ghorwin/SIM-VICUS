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

#ifndef NANDRAD_EmbeddedObjectH
#define NANDRAD_EmbeddedObjectH

#include <string>

#include <IBK_Parameter.h>
//#include "NANDRAD_EmbeddedObjectWindow.h"
//#include "NANDRAD_EmbeddedObjectDoor.h"
//#include "NANDRAD_EmbeddedObjectHole.h"
//#include "NANDRAD_FMUExportReference.h"
//#include "NANDRAD_FMUImportReference.h"
#include "NANDRAD_CodeGenMacros.h"

class TiXmlElement;

namespace NANDRAD {

/*!	\brief Declaration for class EmbeddedObject

	An embedded object generally defines a wall opening (a window or a door).
	That means, the calculation radiant heat fluxes and heat fluxes by heat transmission are performed
	by a window model or door model. The embedded oject	stores an exchangable parameter model
	that defines the name for the calculation model and constant model parameters.
*/
class EmbeddedObject {
public:

	/*! Parameters to be defined for the various object types and models. */
	enum para_t {
		P_Area,						// Keyword: Area						[m2]				'Area of the embedded object [m2].'
		NUM_P
	};
	/*! Type of embedded object, determines set of physical effects described by this object. */
	enum objectType_t {
		OT_WINDOW,			// Keyword: Window			'Parametrization of a window model.'
		OT_DOOR,			// Keyword: Door			'Parametrization of a door model.'
		OT_HOLE,			// Keyword: Hole			'Parametrization of an opening model.'
		NUM_OT
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default constructor. */
	EmbeddedObject();

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	//void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	//void writeXML(TiXmlElement * parent) const;

	NANDRAD_READWRITE

	/*! returns an object type if a model for an object was defined. */
	objectType_t objectType() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of the EmbeddedObject. */
	unsigned int						m_id;								// XML:A

	/*! IBK-language encoded name of the embedded object. */
	std::string							m_displayName;						// XML:A

//	/*! Data container for window model. */
//	EmbeddedObjectWindow				m_window;

//	/*! Data container for door model. */
//	EmbeddedObjectDoor					m_door;

//	/*! Data container for door model. */
//	EmbeddedObjectHole					m_hole;

//	/*! List with FMU export quantity definitions. */
//	std::vector<FMUExportReference>
//										m_FMUExportReferences;

//	/*! List with FMU import quantity definitions. */
//	std::vector<FMUImportReference>
//										m_FMUImportReferences;
//	/*! Names of the enclosing zones. */
//	std::vector<std::string>			m_zoneNames;

	/*! List of constant parameters.*/
	IBK::Parameter						m_para[NUM_P];						// XML:E

}; // EmbeddedObject

} // namespace NANDRAD

#endif // NANDRAD_EmbeddedObjectH
