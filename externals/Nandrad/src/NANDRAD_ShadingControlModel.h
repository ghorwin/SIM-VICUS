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

#ifndef NANDRAD_ShadingControlModelH
#define NANDRAD_ShadingControlModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

class ConstructionInstance;
class EmbeddedObject;
class Sensor;

/*! Parameters for a intensity controlled shading model with hysteresis. */
class ShadingControlModel {
	NANDRAD_READWRITE_PRIVATE
public:
	/*! Model parameters. */
	enum para_t {
		P_MaxIntensity,				// Keyword: MaxIntensity		[W/m2]		'Maximum intensity allowed before shading is closed.'
		P_MinIntensity,				// Keyword: MinIntensity		[W/m2]		'Intensity level below which shading is opened.'
		NUM_P
	};

	NANDRAD_READWRITE_IFNOT_INVALID_ID
	NANDRAD_COMPARE_WITH_ID

	/*! Checks for valid and required parameters (value ranges).
		Also check if referenced sensors/constructions/embedded objects exists.
	*/
	void checkParameters(const std::vector<Sensor> &sensors,
						 const std::vector<ConstructionInstance> &conInstances);

	/*! Unique ID-number for this shading controller model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;			// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string							m_displayName;						// XML:A

	/*! Sensor ID of global radiation sensor, or window/construction surface.
		This sensor corresponds to any surface in the model (sensor, embedded object, construction).
	*/
	unsigned int						m_sensorId = NANDRAD::INVALID_ID;	// XML:A:required

	/*! Model parameters. */
	IBK::Parameter						m_para[NUM_P];						// XML:E

	/*! Quick-access pointer to the requested sensor. */
	const NANDRAD::Sensor				*m_sensor = nullptr;
	/*! Quick-access pointer to the referenced construction. */
	const NANDRAD::ConstructionInstance	*m_constructionInstance = nullptr;
	/*! Quick-access pointer to the referenced embedded object. */
	const NANDRAD::EmbeddedObject		*m_embeddedObject = nullptr;
};

} // namespace NANDRAD

#endif // NANDRAD_ShadingControlModelH
