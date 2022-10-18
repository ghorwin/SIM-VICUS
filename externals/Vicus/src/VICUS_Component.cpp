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

#include "VICUS_Component.h"

namespace VICUS {

bool Component::isValid(const VICUS::Database<Material> & materials, const VICUS::Database<Construction> & constructions,
						const VICUS::Database<BoundaryCondition> & bcs, const VICUS::Database<Schedule> & scheduleDB) const
{
	try {

		// referenced construction instance exists and is valid?
		const Construction * con = constructions[m_idConstruction];
		if (con == nullptr) {
			m_errorMsg = "Construction is not set.";
			return false;
		}

		if (!con->isValid(materials)) {
			m_errorMsg = "Construction '" + con->m_displayName.string("de", true) + "' is not valid.";
			return false;
		}

		const BoundaryCondition *bcA = bcs[m_idSideABoundaryCondition];
		const BoundaryCondition *bcB = bcs[m_idSideBBoundaryCondition];

		if (bcA == nullptr && bcB == nullptr) {
			m_errorMsg = "Boundary Condition A and B is not set.";
			return false;
		}

		if (bcA != nullptr && !bcA->isValid(scheduleDB)) {
			m_errorMsg = "Boundary Condition A is not valid.";
			return false;
		}

		if (bcB != nullptr && !bcB->isValid(scheduleDB)) {
			m_errorMsg = "Boundary Condition B is not valid.";
			return false;
		}

		if( ( bcA != nullptr && bcB == nullptr && bcA->hasSetpointTemperatureForZone() ) ||
				( bcB != nullptr && bcA == nullptr && bcB->hasSetpointTemperatureForZone() ) ||
				( bcA != nullptr && bcB != nullptr &&
				bcA->hasSetpointTemperatureForZone() && bcB->hasSetpointTemperatureForZone() ) ) {
			m_errorMsg = "";
			return false;
		}

		if (m_activeLayerIndex != VICUS::INVALID_ID) {
			if (m_activeLayerIndex >= con->m_materialLayers.size()) {
				m_errorMsg = "Actice layer index '" + std::to_string(m_activeLayerIndex) +
						"' is bigger then number of layers '" + std::to_string(con->m_materialLayers.size()) + "'";
				return false;
			}
		}
		return true;

	} catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		return false;
	}
}


AbstractDBElement::ComparisonResult Component::equal(const AbstractDBElement *other) const {
	const Component * otherComp = dynamic_cast<const Component*>(other);
	if (otherComp == nullptr)
		return Different;

	if (	m_idConstruction != otherComp->m_idConstruction ||
			m_idSideABoundaryCondition != otherComp->m_idSideABoundaryCondition||
			m_idSideBBoundaryCondition != otherComp->m_idSideBBoundaryCondition ||
			m_idSurfaceProperty != otherComp->m_idSurfaceProperty ||
			m_activeLayerIndex != otherComp->m_activeLayerIndex)
		return Different;

	if (	m_displayName != otherComp->m_displayName ||
			m_dataSource != otherComp->m_dataSource ||
			m_manufacturer != otherComp->m_manufacturer ||
			m_type != otherComp->m_type ||
			m_notes != otherComp->m_notes)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
