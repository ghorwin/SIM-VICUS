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

#ifndef VICUS_InfiltrationH
#define VICUS_InfiltrationH

#include <QColor>

#include <IBK_Parameter.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"

namespace VICUS {

/*! Describes the course of all infiltration.

*/

class Infiltration : public AbstractDBElement {
public:

	/*! Basic parameters. */
	enum para_t {
		/*! Air change rate. */
		P_AirChangeRate,				// Keyword: AirChangeRate			[1/h]		'Air change rate.'
		/*! Shielding coefficient. */
		P_ShieldingCoefficient,			// Keyword: ShiedlindCoefficient	[-]			'Shielding coefficient for n50 value.'

		NUM_P
	};

	/*! Air Change Type.*/
	enum AirChangeType {
		AC_normal,				// Keyword: normal							[1/h]		'normal'
		AC_n50,					// Keyword: n50								[1/h]		'n50'
		NUM_AC
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all parameters are valid. */
	bool isValid() const;

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of infiltration. */
	unsigned int					m_id = INVALID_ID;						// XML:A:required

	/*! Display name of infiltration. */
	IBK::MultiLanguageString		m_displayName;							// XML:A

	/*! False color. */
	QColor							m_color;								// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;								// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;							// XML:E

	/*! Air change type. */
	AirChangeType					m_airChangeType = NUM_AC;				// XML:E

	/*! List of parameters. */
	IBK::Parameter					m_para[NUM_P];							// XML:E
};

}
#endif // VICUS_InfiltrationH
