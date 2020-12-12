/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "QtExt_ParameterEdit.h"

#include <limits>

#include <QPalette>
#include <QLocale>
#include <QComboBox>

#include <IBK_Parameter.h>
#include <IBK_Unit.h>
#include <IBK_UnitList.h>
#include <IBK_Exception.h>

namespace QtExt {

ParameterEdit::ParameterEdit(QWidget *parent) :
	ValidatingLineEdit(parent),
	m_buddyUnitCombo(NULL)
{
}


void ParameterEdit::setup(QComboBox * comboBox, const IBK::Unit & unit,
						  double minVal, double maxVal, const QString & toolTip, bool minValAllowed, bool maxValAllowed)
{
	ValidatingLineEdit::setup(minVal, maxVal, toolTip, minValAllowed, maxValAllowed);
	setBuddyUnitCombo(comboBox, unit);
}


void ParameterEdit::setFromParameter(const IBK::Parameter & param,
												   double defaultValue, const IBK::Unit & defaultUnit)
{
	m_displayUnit = defaultUnit;
	// check if parameter is set
	if (param.name.empty()) {
		setValue(defaultValue);
		setUnit(defaultUnit); // default value is always in given unit
		return;
	}
	try {
		// get value already converted into display unit
		double val = param.get_value(defaultUnit);
		m_displayUnit = param.IO_unit;
		setUnit(param.IO_unit); // set display unit, might throw an exception if units aren't convertible
		setValue( val );
		// Note: if no unit buddy combo box, simply the plain parameter value is set in lineedit without
		// unit conversion
	}
	catch (IBK::Exception &) {
		setValue( defaultValue );
	}
}


void ParameterEdit::setFromParameter(const IBK::Parameter & param, const IBK::Unit & unit) {
	m_displayUnit = unit;
	// check if parameter is set
	if (param.name.empty()) {
		setText(""); // will show the placeholder text
		return;
	}
	try {
		// get value already converted into display unit
		double val = param.get_value();
		if (m_buddyUnitCombo) {
			m_displayUnit = param.IO_unit;
			setUnit(param.IO_unit); // set display unit, might throw an exception if units aren't convertible
		}
		setValue(val);
		// Note: if no unit buddy combo box, simply the plain parameter value is set in lineedit without
		// unit conversion
	}
	catch (IBK::Exception &) {
		setText("");
	}
}


void ParameterEdit::setFromParameter(const IBK::Parameter & param) {
	m_displayUnit.clear();
	if (m_buddyUnitCombo) {
		m_displayUnit = IBK::Unit(m_buddyUnitCombo->currentText().toUtf8().data());
	}
	// check if parameter is set
	if (param.name.empty()) {
		setText(""); // will show the placeholder text
		return;
	}
	try {
		// get value already converted into display unit
		double val = param.get_value();
		if (m_buddyUnitCombo) {
			setUnit(param.IO_unit); // set display unit, might throw an exception if units aren't convertible
			m_displayUnit = param.IO_unit;
		}
		setValue( val );
		// Note: if no unit buddy combo box, simply the plain parameter value is set in lineedit without
		// unit conversion
	}
	catch (IBK::Exception &) {
		setText(""); // clear line edit
	}
}



void ParameterEdit::setEnabled(bool enabled) {
	m_buddyUnitCombo->setEnabled(enabled);
	ValidatingLineEdit::setEnabled(enabled);
}


IBK::Parameter ParameterEdit::toParameter(const char * const parameterName) const {
	Q_ASSERT(isValid());
	if (text().trimmed().isEmpty() && !placeholderText().isEmpty()) {
		return IBK::Parameter();
	}
	double val;
	if (!isValidNumber(val))
		return IBK::Parameter();

	return IBK::Parameter(parameterName, val, m_displayUnit);
}


IBK::Unit ParameterEdit::currentUnit() const {
	return m_displayUnit;
}


void ParameterEdit::unitChanged(QString unit) {
	// get current value
	double val;
	bool ok = isValidNumber(val);
	if (ok) {
		IBK::Unit oldUnit = m_displayUnit;
		IBK::Unit newUnit(unit.toUtf8().data());
		IBK::UnitList::instance().convert(oldUnit, newUnit, val);
		blockSignals(true);
		setValue( val );
		blockSignals(false);
		m_displayUnit = newUnit;
		if (isValid())
			emit editingFinishedSuccessfully();
	}
}

void ParameterEdit::setBuddyUnitCombo(QComboBox * comboBox, const IBK::Unit & unit) {
	if (m_buddyUnitCombo != NULL) {
		disconnect(m_buddyUnitCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(unitChanged(QString)));
	}
	m_buddyUnitCombo = comboBox;
	if (m_buddyUnitCombo != NULL) {
		m_buddyUnitCombo->clear();
		populateUnitCombo(m_buddyUnitCombo, unit);
		connect(m_buddyUnitCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(unitChanged(QString)));
	}
}


void ParameterEdit::setUnit(const IBK::Unit & unit) {
	const char * const FUNC_ID = "[ParameterEdit::setUnit]";
	if (m_buddyUnitCombo == NULL)
		throw IBK::Exception("UnitCombo buddy not set.", FUNC_ID);

	m_buddyUnitCombo->blockSignals(true);
	int i=0;
	for (; i<m_buddyUnitCombo->count(); ++i) {
		if (m_buddyUnitCombo->itemData(i).toUInt() == unit.id()) {
			m_buddyUnitCombo->setCurrentIndex(i);
			break;
		}
	}
	if (i == m_buddyUnitCombo->count())
		throw IBK::Exception("Invalid unit, does not match list of possible units.", FUNC_ID);
	m_buddyUnitCombo->blockSignals(false);
}


void ParameterEdit::populateUnitCombo(QComboBox * combo, const IBK::Unit & u) {
	std::vector<IBK::Unit> units;
	IBK::UnitList::instance().convertible_units(units, u);
	for (unsigned int i=0; i<units.size(); ++i)
		combo->addItem(QString::fromUtf8(units[i].name().c_str()),units[i].id());
}

} // namespace QtExt
