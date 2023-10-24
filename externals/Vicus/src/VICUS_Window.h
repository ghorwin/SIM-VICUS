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

#ifndef VICUS_WindowH
#define VICUS_WindowH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_AbstractDBElement.h"
#include "VICUS_WindowFrame.h"
#include "VICUS_WindowGlazingSystem.h"
#include "VICUS_WindowDivider.h"
#include "VICUS_Database.h"
#include "VICUS_Material.h"

#include <QString>
#include <QColor>

namespace VICUS {

/*! Data for a window (embedded object), defined through glazing system, frame a dividers. */
class Window : public AbstractDBElement {
public:

	enum Method {
		M_None,						// Keyword: None								'None'
		M_Fraction,					// Keyword: Fraction							'Fraction of area'
		M_ConstantWidth,			// Keyword: ConstantWidth						'Constant width'
		NUM_M
	};

	enum para_t {
		P_FrameWidth,				// Keyword: FrameWidth					[m]		'Frame width of the window'
		P_FrameFraction,			// Keyword: FrameFraction				[---]	'Frame area fraction of the window'
		P_DividerWidth,				// Keyword: DividerWidth				[m]		'Divider width of the window'
		P_DividerFraction,			// Keyword: DividerFraction				[---]	'Divider area fraction of the window'
		NUM_P
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_OVERRIDE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	/*! Checks if references glazing system exist and if all parameters are valid. */
	bool isValid() const;

	/*! Computes the u-Value. */
	bool calculateUValue(double & UValue,
						 const VICUS::Database<Material> & materials,
						 const VICUS::Database<WindowGlazingSystem> & glazingSystems,
						 double ri, double re) const;

	// *** PUBLIC MEMBER VARIABLES ***

	//:inherited	unsigned int					m_id = INVALID_ID;		// XML:A:required
	//:inherited	IBK::MultiLanguageString		m_displayName;			// XML:A
	//:inherited	QColor							m_color;				// XML:A

	/*! ID of glazing system referenced from this window definition. */
	unsigned int					m_idGlazingSystem = INVALID_ID;			// XML:A

	/*! Notes. */
	QString							m_notes;								// XML:E

	/*! Data source. */
	QString							m_dataSource;							// XML:E

	/*! Method for frame. */
	Method							m_methodFrame = NUM_M;					// XML::A

	/*! Method for Divider. */
	Method							m_methodDivider = NUM_M;				// XML::A

	/*! Parameter. */
	IBK::Parameter					m_para[NUM_P];							// XML:E

	/*! Frame parameters (optional). */
	WindowFrame						m_frame;								// XML:E

	/*! Divider parameters (optional). */
	WindowDivider					m_divider;								// XML:E

};

} // namespace VICUS


#endif // VICUS_WindowH
