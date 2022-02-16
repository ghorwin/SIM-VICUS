/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
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
	m_buddyUnitCombo(nullptr)
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
		m_displayUnit = IBK::Unit(m_buddyUnitCombo->currentText().toStdString());
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
		IBK::Unit newUnit(unit.toStdString());
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
	if (m_buddyUnitCombo != nullptr) {
		disconnect(m_buddyUnitCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(unitChanged(QString)));
	}
	m_buddyUnitCombo = comboBox;
	if (m_buddyUnitCombo != nullptr) {
		m_buddyUnitCombo->clear();
		populateUnitCombo(m_buddyUnitCombo, unit);
		connect(m_buddyUnitCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(unitChanged(QString)));
	}
}


void ParameterEdit::setUnit(const IBK::Unit & unit) {
	const char * const FUNC_ID = "[ParameterEdit::setUnit]";
	if (m_buddyUnitCombo == nullptr)
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
		combo->addItem(QString::fromStdString(units[i].name()), units[i].id());
}

} // namespace QtExt
