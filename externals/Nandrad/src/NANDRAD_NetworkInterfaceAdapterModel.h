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

#ifndef NANDRAD_NetworkInterfaceAdapterModelH
#define NANDRAD_NetworkInterfaceAdapterModelH

#include "NANDRAD_HydraulicFluid.h"

namespace NANDRAD {

/*! A model that computes return temperature based on given supply temperature and mass flow by removing energy from the fluid.
	Energy loss is retrieved from HeatLoadSummationModel.
*/
class NetworkInterfaceAdapterModel {
public:

	NANDRAD_READWRITE

	/*! Checks parameters for valid values. */
	void checkParameters() const;

	/*! Unique ID-number for this ventilation rate model. */
	unsigned int						m_id = NANDRAD::INVALID_ID;					// XML:A:required
	/*! Some display/comment name for this model (optional). */
	std::string							m_displayName;								// XML:A

	/*! The fluid properties. */
	IBK::Parameter						m_fluidHeatCapacity;						// XML:E

	/*! Reference to HeatLoadSummationModel required as source for heat load. */
	unsigned int						m_summationModelId = NANDRAD::INVALID_ID;	// XML:A:required
};

} // namespace NANDRAD

#endif // NANDRAD_NetworkInterfaceAdapterModelH
