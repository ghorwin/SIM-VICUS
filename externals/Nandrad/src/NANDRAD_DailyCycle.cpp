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

#include <IBK_UnitList.h>

#include "NANDRAD_Constants.h"
#include "NANDRAD_DailyCycle.h"
#include "NANDRAD_KeywordList.h"

namespace NANDRAD {

void DailyCycle::prepareCalculation() {
	FUNCID(DailyCycle::prepareCalculation);

	// empty daily cycle? throw exception
	if (m_values.m_values.empty())
		throw IBK::Exception("Missing DataTable values in DailyCycle.", FUNC_ID);

	// empty time points? throw exception
	if (m_timePoints.empty())
		throw IBK::Exception("Missing time points values in DailyCycle.", FUNC_ID);

	// time unit valid?
	if (m_timeUnit.id() == 0 || m_timeUnit.base_id() != IBK_UNIT_ID_SECONDS)
		throw IBK::Exception(IBK::FormatString("Invalid/undefined time unit '%1' in DailyCycle.")
							 .arg(m_timeUnit.name()), FUNC_ID);

	if (m_interpolation == NUM_IT)
		m_interpolation = IT_LINEAR;
	else if (m_interpolation != IT_CONSTANT)
		throw IBK::Exception("Invalid/undefined interpolation type in DailyCycle.", FUNC_ID);

	// first check correct number of values
	unsigned int valueCount = m_values.m_values.begin()->second.size();
	if (valueCount != m_timePoints.size())
		throw IBK::Exception(IBK::FormatString("Mismatching number of values (=%1) compared to time points (=%2) in DailyCycle.")
							 .arg(valueCount).arg(m_timePoints.size()), FUNC_ID);


	//	This map holds the units matching the values in m_values.

	// process column headers - extract units and variable names
	for (auto v : m_values.m_values) {
		// extract unit from parenthesis
		std::string::size_type p1 = v.first.rfind("[");
		std::string::size_type p2 = v.first.rfind("]");
		if (p1 != std::string::npos || p2 != std::string::npos) {
			if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
				std::string varName = v.first.substr(0, p1);
				IBK::trim(varName);
				std::string unitName = v.first.substr(p1+1, p2-p1-1);
				IBK::trim(unitName);
				if (!varName.empty() && !unitName.empty()) {
					// now convert unit
					try {
						IBK::Unit u(unitName);
						// register as variable, if it does not yet exist
						if (std::find(m_valueNames.begin(), m_valueNames.end(), varName) != m_valueNames.end())
							throw IBK::Exception(IBK::FormatString("Duplicate variable name '%1' "
																   "in DataTable of daily cycle.").arg(varName), FUNC_ID);
						// store variable
						m_valueUnits.push_back(u);
						m_valueNames.push_back(varName);
					} catch (...) {
						throw IBK::Exception(IBK::FormatString("Invalid/unrecognized unit '%1' "
															   "in scheduled variable caption.").arg(unitName), FUNC_ID);
					}
				}
			}
			throw IBK::Exception(IBK::FormatString("Invalid format of variable caption '%1', "
												   "expected format 'varname [unit]'.").arg(v.first), FUNC_ID);
		}
		// no unit - assume unitless parameter
		m_valueUnits.push_back(IBK::Unit("-"));
		m_valueNames.push_back(v.first);
	}
	// now store the collected data in the linear vectors
}


bool DailyCycle::operator!=(const DailyCycle & other) const {
	// Note: we do not compare solver runtime parameters here!
	if (m_interpolation != other.m_interpolation) return true;
	if (m_timePoints != other.m_timePoints) return true;
	if (m_timeUnit != other.m_timeUnit) return true;
	if (m_values != other.m_values) return true;
	return false;
}


} // namespace NANDRAD

