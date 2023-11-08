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

#ifndef ACOUSTICTEMPLATE_H
#define ACOUSTICTEMPLATE_H

#include <IBK_IntPara.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_AbstractDBElement.h"

#include "NANDRAD_LinearSplineParameter.h"

namespace VICUS {


class AcousticTemplate : public AbstractDBElement {
public:
	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Enum type with all possible parameters for evaluation.*/
	enum splinePara_t {
		SP_MaxValue,						// Keyword: MaxValue						[-]		'Max values for evaluation'
		SP_MinValue,						// Keyword: MinValue						[-]		'Max values for evaluation'
		NUM_SP
	};

	/*! C'tor */
	AcousticTemplate() {}


	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! Notes. */
	IBK::MultiLanguageString			m_note;									// XML:E

	/*! Data source. */
	IBK::MultiLanguageString			m_dataSource;							// XML:E

	double								m_evaluationOffset;						// XML:E
	double								m_evaluationFactor;						// XML:E


	/*! Normalized angle-dependent SHGC values. */
	NANDRAD::LinearSplineParameter		m_splinePara[NUM_SP];					// XML:E
};

}

#endif // ACOUSTICTEMPLATE_H
