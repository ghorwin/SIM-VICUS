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

	// if interpolation mode isn't explicitly given, we default to Linear
	if (m_interpolation == NANDRAD::DailyCycle::NUM_IT)
		m_interpolation = IT_Linear;

	// check if time points follow the rules
	if (m_timePoints[0] != 0.0)
		throw IBK::Exception("Invalid time points in DailyCycle (first time point must be 0).", FUNC_ID);
	if (m_timePoints.back() >= 24.0)
		throw IBK::Exception("Invalid time points in DailyCycle (last time point must be < 24 h).", FUNC_ID);

	// check for monitonically increasing time points
	double last  = m_timePoints[0];
	for (unsigned int i=1; i<m_timePoints.size(); ++i) {
		if (last >= m_timePoints[i])
			throw IBK::Exception("Invalid time points in DailyCycle : time points must follow in strictly monotonic order.", FUNC_ID);
		last = m_timePoints[i];
	}


	// first check correct number of values
	unsigned int valueCount = m_values.m_values.begin()->second.size();
	if (valueCount != m_timePoints.size())
		throw IBK::Exception(IBK::FormatString("Mismatching number of values (=%1) compared to time points (=%2) in DailyCycle.")
							 .arg(valueCount).arg(m_timePoints.size()), FUNC_ID);


	//	This map holds the units matching the values in m_values.

	// process column headers - extract units and variable names
	for (std::map<std::string, std::vector<double> >::const_iterator valIT = m_values.m_values.begin();
		 valIT != m_values.m_values.end(); ++valIT)
	{
		// extract unit from parenthesis
		std::string::size_type p1 = valIT->first.rfind("[");
		std::string::size_type p2 = valIT->first.rfind("]");
		if (p1 != std::string::npos || p2 != std::string::npos) {
			if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1) {
				std::string varName = valIT->first.substr(0, p1);
				IBK::trim(varName);
				std::string unitName = valIT->first.substr(p1+1, p2-p1-1);
				IBK::trim(unitName);
				if (!varName.empty() && !unitName.empty()) {
					// now convert unit
					try {
						IBK::Unit u(unitName);
						// register as variable, if it does not yet exist
						if (std::find(m_valueData.begin(), m_valueData.end(), varName) != m_valueData.end())
							throw IBK::Exception(IBK::FormatString("Duplicate variable name '%1' "
																   "in DataTable of daily cycle.").arg(varName), FUNC_ID);
						// store variable
						m_valueData.push_back( valueData_t(varName, u, &valIT->second) ); // persistent pointer is guaranteed here
					} catch (...) {
						throw IBK::Exception(IBK::FormatString("Invalid/unrecognized unit '%1' "
															   "in scheduled variable caption.").arg(unitName), FUNC_ID);
					}
					continue; // next column
				}
			}
			throw IBK::Exception(IBK::FormatString("Invalid format of variable caption '%1', "
												   "expected format 'varname [unit]'.").arg(valIT->first), FUNC_ID);
		}
		// no unit - assume unitless parameter
		m_valueData.push_back( valueData_t(valIT->first, IBK::Unit("-"), &valIT->second) ); // persistent pointer is guaranteed here
	}
}


bool DailyCycle::operator!=(const DailyCycle & other) const {
	// Note: we do not compare solver runtime parameters here!
	if (m_interpolation != other.m_interpolation) return true;
	if (m_timePoints != other.m_timePoints) return true;
	if (m_values != other.m_values) return true;
	return false;
}


} // namespace NANDRAD

