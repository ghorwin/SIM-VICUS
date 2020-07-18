/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#include "NANDRAD_LinearSplineParameter.h"

#include "NANDRAD_Constants.h"
#include "NANDRAD_KeywordList.h"

#include <tinyxml.h>

#include <IBK_UnitList.h>
#include <IBK_UnitVector.h>

namespace NANDRAD {


void LinearSplineParameter::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[LinearSplineParameter::readXML]";
	IBK::UnitVector xData, yData;
	try {
		std::string xUnit, yUnit;
		std::string interpolationMethod;
		TiXmlElement::readIBKLinearSplineElement(element, m_name, interpolationMethod, xUnit, xData.m_data, yUnit, yData.m_data);
		if (m_name.empty())
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
			"Missing or empty linear spline parameter name."
			), FUNC_ID);

		// convert interpolation method
		if (interpolationMethod.empty())
			m_interpolationMethod = I_Linear; // the default
		else {

			if(!KeywordList::KeywordExists("LinearSplineParameter::interpolationMethod_t",
				interpolationMethod) )
			{
				throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
							IBK::FormatString("Invalid/unknown interpolation method '%1'.").arg(interpolationMethod)
							), FUNC_ID);
			}

			m_interpolationMethod = (interpolationMethod_t)KeywordList::Enumeration(
							"LinearSplineParameter::interpolationMethod_t", interpolationMethod);
		}
		// check for value units
		try {
			m_xUnit.set(xUnit);
		}
		catch (...) {
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Invalid/unknown xUnit.")
				), FUNC_ID);
		}
		try {
			m_yUnit.set(yUnit);
		}
		catch (...) {
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Invalid/unknown yUnit.")
				), FUNC_ID);
		}
		// perform unit conversions
		xData.m_unit = m_xUnit;
		xData.convert(m_xUnit.base_unit());
		yData.m_unit = m_yUnit;
		yData.convert(m_yUnit.base_unit());
		m_values.setValues(xData.m_data, yData.m_data);
		if (!m_values.valid())
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Invalid spline data (cannot create linear spline).")
				), FUNC_ID);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading IBK:LinearSpline data."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading IBK:LinearSpline data.").arg(ex2.what()), FUNC_ID);
	}
}


void LinearSplineParameter::writeXML(TiXmlElement * parent) const {
	if (m_name.empty())
		return; // no name = no spline data specified
	IBK::UnitVector xData, yData;
	// setup unit vectors for conversion
	xData.m_data = m_values.x();
	yData.m_data = m_values.y();
	xData.m_unit = m_xUnit.base_unit();
	yData.m_unit = m_yUnit.base_unit();
	xData.convert(m_xUnit);
	yData.convert(m_yUnit);

	std::string interpolationMethod;
	if (m_interpolationMethod != NUM_I && m_interpolationMethod != I_Linear) {
		interpolationMethod = KeywordList::Keyword("LinearSplineParameter::interpolationMethod_t", m_interpolationMethod);
	}
	TiXmlElement::appendIBKLinearSplineElement(parent, m_name, interpolationMethod, m_xUnit.name(),
											   xData.m_data, m_yUnit.name(), yData.m_data);
}


bool LinearSplineParameter::operator!=(const LinearSplineParameter & other) const {
	if (m_name != other.m_name) return true;
	if (m_interpolationMethod != other.m_interpolationMethod) return true;
	if (m_values != other.m_values) return true;
	if (m_xUnit != other.m_xUnit) return true;
	if (m_yUnit != other.m_yUnit) return true;
	return false;
}


} // namespace NANDRAD

