/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

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

#ifndef NANDRAD_ShadingControlModelH
#define NANDRAD_ShadingControlModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! Contains all data for natural ventilation models. */
class ShadingControlModel {
	NANDRAD_READWRITE_PRIVATE
public:
	/*! Different model variants. */
	enum modelType_t {
		/*! Ventilation rate is given as constant parameter. */
		MT_SingleIntensityControlled,				// Keyword: SingleIntensityControlled	'Simple hysteretic shading control based on global radiation sensor'
		NUM_MT
	};

	/*! Model parameters. */
	enum para_t {
		P_MaxIntensity,				// Keyword: MaxIntensity		[W/m2]		'Maximum intensity allowed before shading is closed.'
		P_MinIntensity,				// Keyword: MinIntensity		[W/m2]		'Intensity level below which shading is opened.'
		NUM_P
	};

	NANDRAD_READWRITE_IFNOT_INVALID_ID
	NANDRAD_COMPARE_WITH_ID

	/*! Unique ID-number for this ventilation rate model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;			// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string							m_displayName;						// XML:A

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;				// XML:A:required

	/*! Sensor ID of global radiation sensor, needed for SingleIntensityControlled model. */
	unsigned int						m_sensorID = NANDRAD::INVALID_ID;	// XML:A

	/*! Model parameters. */
	IBK::Parameter						m_para[NUM_P];						// XML:E
};

} // namespace NANDRAD

#endif // NANDRAD_ShadingControlModelH
