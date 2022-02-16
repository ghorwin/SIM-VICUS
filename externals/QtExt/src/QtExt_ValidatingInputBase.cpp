/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#include "QtExt_ValidatingInputBase.h"

#include <limits>

namespace QtExt {

ValidatingInputBase::ValidatingInputBase() :
	m_minVal(-std::numeric_limits<double>::max()),
	m_maxVal(std::numeric_limits<double>::max()),
	m_minValueAllowed(false),
	m_maxValueAllowed(false),
	m_allowEmpty(false),
	m_acceptOnlyInteger(false),
	m_format('a'),
	m_precision(0)
{

}

ValidatingInputBase::~ValidatingInputBase() {

}


void ValidatingInputBase::setup(double minVal, double maxVal, const QString &toolTip, bool minValueAllowed, bool maxValueAllowed) {
	m_minVal = minVal;
	m_maxVal = maxVal;
	m_toolTip = toolTip;
	m_minValueAllowed = minValueAllowed;
	m_maxValueAllowed = maxValueAllowed;
}

void ValidatingInputBase::setMaximum(double maxVal, bool maxValueAllowed) {
	m_maxVal = maxVal;
	m_maxValueAllowed = maxValueAllowed;
}

void ValidatingInputBase::setMinimum(double minVal, bool minValueAllowed) {
	m_minVal = minVal;
	m_minValueAllowed = minValueAllowed;
}

void ValidatingInputBase::setValidator(ValidatorBase* validator) {
	m_validator.reset(validator);
}


ValidatorBase* ValidatingInputBase::validator() const {
	return m_validator.get();
}


void ValidatingInputBase::setFormatter(FormatterBase *formatter) {
	m_formatter.reset(formatter);
}


FormatterBase* ValidatingInputBase::formatter() const {
	return m_formatter.get();
}


void ValidatingInputBase::setFormat(char format, int precision) {
	m_format = format;
	if (m_format != 'e' && m_format != 'E' && m_format != 'f' && m_format != 'g' && m_format != 'G')
		m_format = 'a';
	m_precision = precision;
}


bool ValidatingInputBase::isValidImpl(double val) const {
	if (m_validator.get() != nullptr) {
		if (!m_validator->isValid(val))
			return false;
	}
	else {
		if (m_minValueAllowed) {
			if (val < m_minVal)
				return false;
		}
		else {
			if (val <= m_minVal)
				return false;
		}
		if (m_maxValueAllowed) {
			if (val > m_maxVal)
				return false;
		}
		else {
			if (val >= m_maxVal)
				return false;
		}
	}
	// only accept the value if validation rules say it's ok
	m_value = val;
	return true;
}


FormatterBase::~FormatterBase() {}


} // namespace QtExt
