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

#include "NANDRAD_Models.h"

#include <IBK_messages.h>

namespace NANDRAD {

/*! Test function that checks that all objects in the given vector have different m_id parameters. */
template <typename T>
void checkForUniqueModelIDs(const std::vector<T> & vec, std::set<unsigned int> & usedIDs) {
	FUNCID(NANDRAD::checkForUniqueIDs);

	for (const T & t : vec) {
		if (usedIDs.find(t.m_id) != usedIDs.end())
			throw IBK::Exception(IBK::FormatString("Duplicate model/object ID #%1.")
								 .arg(t.m_id), FUNC_ID);
		usedIDs.insert(t.m_id);
	}
}


void Models::checkForUniqueIDs() const {
	FUNCID(Models::checkForUniqueIDs);

	std::set<unsigned int> usedIDs;

	try {
		// check all natural ventilation models for unique ids and store
		// it in usedIDs container
		checkForUniqueModelIDs(m_naturalVentilationModels, usedIDs);
		// check all internal loads model ids against each other and against
		// all entries in usedIDs container
		checkForUniqueModelIDs(m_internalLoadsModels, usedIDs);
		// the same for shading control models
		checkForUniqueModelIDs(m_shadingControlModels, usedIDs);
		// the same for thermostats
		checkForUniqueModelIDs(m_thermostats, usedIDs);
		// the same for heating models
		checkForUniqueModelIDs(m_idealHeatingCoolingModels, usedIDs);
		// the same for heating surface models
		checkForUniqueModelIDs(m_idealSurfaceHeatingCoolingModels, usedIDs);
		// the same for pipe register models
		checkForUniqueModelIDs(m_idealPipeRegisterModels, usedIDs);
		// the same for heat load summation models
		checkForUniqueModelIDs(m_heatLoadSummationModels, usedIDs);
		// the same for network interface models
		checkForUniqueModelIDs(m_networkInterfaceAdapterModels, usedIDs);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Duplicate ID found in model parameter blocks.", FUNC_ID);
	}
}

} // namespace NANDRAD
