/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

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
		CT_Window,					// Keyword: Window						'A window'
		CT_Door,					// Keyword: Door						'A door'
		CT_Miscellaneous,			// Keyword: Miscellaneous				'Some other component type'
		NUM_CT
	};

	enum para_t {
		P_ReductionFactor,			// Keyword: ReductionFactor		[---]	'Reduction factor for dynamic shading'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::Window> & windows,
				 const VICUS::Database<VICUS::BoundaryCondition> & bcs, const VICUS::Database<Schedule> & scheduleDB) const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;			// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;				// XML:A
	//:inherited	QColor							m_color;					// XML:A

	/*! SubSurfaceComponent type. */
	SubSurfaceComponentType			m_type = CT_Miscellaneous;					// XML:E:required

	/*! Transparent construction ID (used for CT_Window). */
	unsigned int					m_idWindow = INVALID_ID;					// XML:E

	/*! Construction type ID in case of opaque component/door (used for all types but CT_Window). */
	unsigned int					m_idConstruction = INVALID_ID;				// XML:E

	/*! Boundary condition ID for Side A (usually outside). */
	unsigned int					m_idSideABoundaryCondition = INVALID_ID;	// XML:E

	/*! Boundary condition ID for Side B (usually inside). */
	unsigned int					m_idSideBBoundaryCondition = INVALID_ID;	// XML:E

	/*! Parameter. */
	IBK::Parameter					m_para[NUM_P];								// XML:E
};

} // namespace VICUS


#endif // VICUS_SubSurfaceComponentH
