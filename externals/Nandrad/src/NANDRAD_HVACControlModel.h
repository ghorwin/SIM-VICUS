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

#ifndef NANDRAD_HVACControlModelH
#define NANDRAD_HVACControlModelH

#include <IBK_Parameter.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! Contains data for a control model that generates control signals for different heating equipment based
	on thermostat values.
*/
class HVACControlModel {
public:
	/*! Different model variants. */
	enum modelType_t {
		MT_Heating,						// Keyword: Heating						'Heating control model'
		NUM_MT
	};

	/*! Operating mode. */
	enum OperatingMode {
		/*! Input signal is forwarded unmodified to all heating models. */
		OM_Parallel,					// Keyword: Parallel					'Parallel operation'
		NUM_OM
	};

	NANDRAD_READWRITE

	/*! Checks parameters for valid values. */
	void checkParameters() const;

	/*! Unique ID-number for this model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;		// XML:A:required

	/*! Model type. */
	modelType_t							m_modelType = NUM_MT;			// XML:A:required

	/*! Operating mode. */
	OperatingMode						m_operatingMode = NUM_OM;		// XML:A:required

	/*! Object list with zones that this model is to be apply to. */
	std::string							m_zoneObjectList;				// XML:E:required
};

} // namespace NANDRAD

#endif // NANDRAD_HVACControlModelH
