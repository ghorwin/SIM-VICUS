/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

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
	if (m_validator.get() != NULL) {
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

} // namespace QtExt
