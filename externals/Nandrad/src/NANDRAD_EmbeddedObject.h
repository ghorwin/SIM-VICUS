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

#ifndef NANDRAD_EmbeddedObjectH
#define NANDRAD_EmbeddedObjectH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"
#include "NANDRAD_EmbeddedObjectWindow.h"

//#include "NANDRAD_EmbeddedObjectDoor.h"
//#include "NANDRAD_EmbeddedObjectHole.h"

namespace NANDRAD {

class Project;

/*!	An embedded object generally defines a wall opening (a window or a door).
	That means, the calculation radiant heat fluxes and heat fluxes by heat transmission are performed
	by a window model or door model. The embedded oject	stores an exchangable parameter model
	that defines the name for the calculation model and constant model parameters.
*/
class EmbeddedObject {
public:

	/*! Parameters to be defined for the various object types and models. */
	enum para_t {
		P_Area,				// Keyword: Area						[m2]				'Area of the embedded object [m2].'
		NUM_P
	};
	/*! Type of embedded object, determines set of physical effects described by this object. */
	enum objectType_t {
		OT_Window,			// Keyword: Window			'Parametrization of a window model.'
		OT_Door,			// Keyword: Door			'Parametrization of a door model.'
		OT_Hole,			// Keyword: Hole			'Parametrization of an opening model.'
		NUM_OT
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! returns an object type if a model for an object was defined. */
	objectType_t objectType() const;

	/*! Checks for valid parameters (value ranges).
		Also creates quick-access pointer connections to referenced data objects.
	*/
	void checkParameters(const Project & prj);


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of the EmbeddedObject. */
	unsigned int						m_id			= INVALID_ID;		// XML:A:required

	/*! IBK-language encoded name of the embedded object. */
	std::string							m_displayName;						// XML:A

	/*! List of constant parameters.*/
	IBK::Parameter						m_para[NUM_P];						// XML:E

	/*! Data container for window model. */
	EmbeddedObjectWindow				m_window;							// XML:E:tag=Window

//	/*! Data container for door model. */
//	EmbeddedObjectDoor					m_door;

//	/*! Data container for door model. */
//	EmbeddedObjectHole					m_hole;


}; // EmbeddedObject

} // namespace NANDRAD

#endif // NANDRAD_EmbeddedObjectH
