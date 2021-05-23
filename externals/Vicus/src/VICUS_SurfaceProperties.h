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

#ifndef VICUS_SurfacePropertiesH
#define VICUS_SurfacePropertiesH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

#include <QString>
#include <QColor>
#include <vector>

#include <IBK_Parameter.h>

namespace VICUS {

class SurfaceProperties : public AbstractDBElement{
public:

	/*! Basic parameters. */
	enum para_t {
		P_Specularity,				// Keyword: Specularity				[---]	'Specularity of the material.'
		P_Roughness,				// Keyword: Roughness				[---]	'Roughness of the material.'
		NUM_P
	};

	enum Type {
		T_Plastic,					// Keyword: Plastic
		T_Metal,					// Keyword: Metal
		T_Glass,					// Keyword: Glass
		NUM_T
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-number for this glazing system (INVALID_ID = disabled/undefined). */
	unsigned int						m_id = INVALID_ID;							// XML:A:required

	/*! Some display/comment name for this model (optional). */
	QString								m_displayName;								// XML:A

	/*! False color. */
	QColor								m_color;									// XML:A

	/*! Manufacturer. */
	QString								m_manufacturer;								// XML:E

	/*! Data source. */
	QString								m_dataSource;								// XML:E

	/*! List of parameters. */
	IBK::Parameter						m_para[NUM_P];								// XML:E

	Type								m_type = T_Plastic;							// XML:E

};

} // namespace VICUS


#endif // VICUS_SurfacePropertiesH
