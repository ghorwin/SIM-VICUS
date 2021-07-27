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

#ifndef VICUS_WindowGlazingSystemH
#define VICUS_WindowGlazingSystemH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_WindowGlazingLayer.h"
#include "VICUS_AbstractDBElement.h"

#include <QString>
#include <QColor>
#include <vector>

#include <IBK_Parameter.h>

#include <NANDRAD_LinearSplineParameter.h>

namespace VICUS {

class WindowGlazingSystem : public AbstractDBElement {
public:

	/*! Model types supported by the window model. */
	enum modelType_t {
		MT_Simple,						// Keyword: Simple								'Standard globbed-layers model'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_ThermalTransmittance,			// Keyword: ThermalTransmittance		[W/m2K]		'Thermal transmittance'
		NUM_P
	};

	/*! Enum type with all possible glazing system spline parameters.*/
	enum splinePara_t {
		SP_SHGC,						// Keyword: SHGC						[---]		'Incidence-angle dependent short wave transmittance across glazing system'
		NUM_SP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Comparison operator */
	ComparisonResult equal(const AbstractDBElement *other) const override;

	/*! Returns the calculate or given uValue depending on model type. [W/m2K].
		Only call this function for valid data, otherwise return value is undefined (-1).
	*/
	double uValue() const;

	/*! Returns the calculated or given SHGC for perpendicular incidence (angle 0) depending on model type. [---].
		Only call this function for valid data, otherwise return value is undefined (-1).
	*/
	double SHGC() const;

	/*! Tests if parameters are valid. */
	bool isValid() const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID-number for this glazing system (INVALID_ID = disabled/undefined). */
	unsigned int						m_id = INVALID_ID;							// XML:A:required

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;						// XML:A:required

	/*! Some display/comment name for this model (optional). */
	IBK::MultiLanguageString			m_displayName;								// XML:A

	/*! False color. */
	QColor								m_color;									// XML:A

	/*! Notes. */
	QString								m_notes;									// XML:E

	/*! Manufacturer. */
	QString								m_manufacturer;								// XML:E

	/*! Data source. */
	QString								m_dataSource;								// XML:E

	/*! List of parameters. */
	IBK::Parameter						m_para[NUM_P];								// XML:E

	/*! Normalized angle-dependent SHGC values. */
	NANDRAD::LinearSplineParameter		m_splinePara[NUM_SP];						// XML:E
};

} // namespace VICUS


#endif // VICUS_WindowGlazingSystemH
