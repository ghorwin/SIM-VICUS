/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others from the SIM-VICUS team ... :-)

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

#ifndef VICUS_SubSurfaceComponentH
#define VICUS_SubSurfaceComponentH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Window.h"
#include "VICUS_BoundaryCondition.h"

#include <IBK_MultiLanguageString.h>

#include <QString>
#include <QColor>
#include <vector>

namespace VICUS {

class SubSurfaceComponent : public AbstractDBElement {
public:

	/*! SubSurfaceComponent types. */
	enum SubSurfaceComponentType {
		CT_Window,					// Keyword: Window					'A window'
		CT_Door,					// Keyword: Door					'A door'
		CT_Miscellaneous,			// Keyword: Miscellaneous			'Some other component type'
		NUM_CT
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::Window> & windows,
				 const VICUS::Database<VICUS::BoundaryCondition> & bcs) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of component. */
	unsigned int					m_id = INVALID_ID;							// XML:A:required

	/*! Display name of component. */
	IBK::MultiLanguageString		m_displayName;								// XML:A

	/*! False color. */
	QColor							m_color;									// XML:A

	/*! SubSurfaceComponent type. */
	SubSurfaceComponentType			m_type = CT_Miscellaneous;					// XML:E:required

	/*! Transparent construction ID. */
	unsigned int					m_idWindow = INVALID_ID;					// XML:E

	/*! Boundary condition ID for Side A (usually outside). */
	unsigned int					m_idSideABoundaryCondition = INVALID_ID;	// XML:E

	/*! Boundary condition ID for Side B (usually inside). */
	unsigned int					m_idSideBBoundaryCondition = INVALID_ID;	// XML:E
};

} // namespace VICUS


#endif // VICUS_SubSurfaceComponentH
