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

#include "VICUS_SubSurfaceComponent.h"

namespace VICUS {

bool SubSurfaceComponent::isValid(const VICUS::Database<VICUS::Window> & windows,
								  const VICUS::Database<VICUS::BoundaryCondition> & bcs,
								  const VICUS::Database<VICUS::Schedule> & scheduleDB) const
{
	try {

		// referenced construction instance exists and is valid?
		const Window * win = windows[m_idWindow];
		if (win == nullptr) {
			m_errorMsg = "Window with id '" + std::to_string(m_idWindow) + "' does not exist.";
			return false;
		}

		if (!win->isValid()) {
			m_errorMsg = "Window '" + win->m_displayName.string("de", true) + "' does not exist.";
			return false;
		}

		const BoundaryCondition *bcA = bcs[m_idSideABoundaryCondition];
		const BoundaryCondition *bcB = bcs[m_idSideBBoundaryCondition];

		if (bcA == nullptr && bcB == nullptr) {
			m_errorMsg = "Boundary conditions are not set.";
			return false;
		}

		if (bcA != nullptr && !bcA->isValid(scheduleDB)) {
			m_errorMsg = "Boundary condition A '" + bcA->m_displayName.string("de", true) + "' is not valid.";
			return false;
		}

		if (bcB != nullptr && !bcB->isValid(scheduleDB)) {
			m_errorMsg = "Boundary condition B '" + bcB->m_displayName.string("de", true) + "' is not valid.";
			return false;
		}


		// TODO : Dirk, add other checks for valid/complete parametrization
		return true;

	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		m_errorMsg = ex.what();
		return false;
	}
}

AbstractDBElement::ComparisonResult SubSurfaceComponent::equal(const AbstractDBElement *other) const {
	const SubSurfaceComponent * otherComp = dynamic_cast<const SubSurfaceComponent*>(other);
	if (otherComp == nullptr)
		return Different;

	// first check critical data
	if (m_idWindow != otherComp->m_idWindow ||
			m_idSideABoundaryCondition != otherComp->m_idSideABoundaryCondition ||
			m_idSideBBoundaryCondition != otherComp->m_idSideBBoundaryCondition ||
			m_type != otherComp->m_type ||
			m_para[P_ReductionFactor].value != otherComp->m_para[P_ReductionFactor].value)
		return Different;

	//check meta data
	if (m_displayName != otherComp->m_displayName || m_color != otherComp->m_color)
		return OnlyMetaDataDiffers;

	return Equal;
}


} // namespace VICUS
