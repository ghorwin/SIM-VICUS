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

#include "QtExt_ValidatingLineEdit.h"

#include <limits>

#include <QPalette>
#include <QEvent>

#include "QtExt_Style.h"
#include "QtExt_Locale.h"

namespace QtExt {

ValidatingLineEdit::ValidatingLineEdit(QWidget * parent) :
	QLineEdit(parent)
{
	// customize appearance
	QPalette palEdit;
	palEdit.setColor(QPalette::Base, Style::EditFieldBackground);
	setPalette(palEdit);
	setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChanged(const QString&)));
	connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}


bool ValidatingLineEdit::isValid() const {
	if (m_allowEmpty && text().trimmed().isEmpty())
		return true;
	double val;
	if (!isValidNumber(val))
		return false;

	return isValidImpl(val);
}


void ValidatingLineEdit::setValidator(ValidatorBase* validator) {
	ValidatingInputBase::setValidator(validator);
}

bool ValidatingLineEdit::isValidNumber(double & val) const {
	bool ok;
	QString textTemp = text();
	if (m_acceptOnlyInteger) {
		val = QLocale().toInt(textTemp, &ok);
	}
	else {
		// first try current locale
		val = QtExt::Locale().toDoubleWithFallback(textTemp, &ok);
	}
	return ok;
}


void ValidatingLineEdit::setValue(double value) {
	QString textTemp;
	if (m_formatter.get() != nullptr) {
		textTemp = m_formatter->formatted(value);
	}
	else if(m_format != 'a' ) {
		textTemp = QtExt::Locale().toString(value, m_format, m_precision);
	}
	else {
		textTemp = QtExt::Locale().toString(value, 'g', 15);
	}

	// this will update the appearance/state of the line edit, also trigger
	// onTextChanged() which will use the validator to detect correctness
	// of value and color the line appropriately
	setText(textTemp);

	// Note: when line edit is read-only or disabled, validator is not called and value
	// is not set -> do that manually since setting a value in
	// a read-only line edit can only be done by code and thus is expected
	// to be valid
	if (isReadOnly() || !isEnabled())
		m_value = value;
}


void ValidatingLineEdit::setText(const QString& str) {
	blockSignals(true);
	QLineEdit::setText(str.trimmed());
	blockSignals(false);
	onTextChanged(text()); // this will update the value and state of the line edit
}


void ValidatingLineEdit::setEnabled(bool enabled) {
	QLineEdit::setEnabled(enabled);
	onTextChanged(text());
	repaint();
}


void ValidatingLineEdit::setReadOnly(bool readOnly) {
	QLineEdit::setReadOnly(readOnly);
	onTextChanged(text());
	repaint();
}


void ValidatingLineEdit::setEmptyAllowed(bool allowEmpty, const QString & placeholderText) {
	m_allowEmpty = allowEmpty;
	setPlaceholderText(placeholderText);
}


void ValidatingLineEdit::setFromParameter(const IBK::Parameter & p, const IBK::Unit & u) {
	if (p.empty())
		setText("");
	else {
		if (p.IO_unit.base_id() != u.base_id())
			throw IBK::Exception(
					IBK::FormatString("Mismatching units, cannot relate unit if parameter '%1' to requested target unit '%2'.")
					.arg(p.IO_unit.name()).arg(u.name()), "[ValidatingLineEdit::setFromParameter]");
		double val = p.get_value(u);
		setValue( val );
	}
}


void ValidatingLineEdit::setFromParameterOrClear(const IBK::Parameter & p, const IBK::Unit & u) {
	if (p.empty() || (p.IO_unit.base_id() != u.base_id()))
		setText("");
	else {
		double val = p.get_value(u);
		setValue( val );
	}
}


void ValidatingLineEdit::setFromParameterOrDefault(const IBK::Parameter & p, const IBK::Parameter & defaultPara) {
	// default parameter also defines the target unit
	IBK::Unit targetUnit = defaultPara.IO_unit;
	if (p.empty()) {
		setValue( defaultPara.get_value() );
	}
	else {
		Q_ASSERT(p.IO_unit.base_id() == targetUnit.base_id()); // if p is given, we expect a matching unit
		double val = p.get_value(targetUnit);
		setValue( val );
	}
}


void ValidatingLineEdit::onEditingFinished() {
	if (isValid())
		emit editingFinishedSuccessfully();
}


void ValidatingLineEdit::onTextChanged ( const QString& ) {
	if (!isEnabled()) {
		setStyleSheet("");
		setToolTip("");
		return;
	}
	if (isReadOnly()) {
		QPalette palEdit;
		setStyleSheet("QLineEdit:enabled { background-color: "+ Style::ReadOnlyEditFieldBackground +"; color: 'black'}");
		setToolTip("");
		return;
	}

	// Note: isValid() will update m_value when it value in text is ok
	if (!isValid()) {
		setStyleSheet("QLineEdit { background-color: "+ Style::ErrorEditFieldBackground +";}");

		if( m_validator.get() != nullptr && !m_validator->toolTip().isEmpty()) {
			setToolTip(m_validator->toolTip());
		}
		else {
			setToolTip(m_toolTip);
		}
	}
	else {
		setStyleSheet("");
		setToolTip("");
	}
}


void ValidatingLineEdit::changeEvent(QEvent *event) {
	if (event->type() == QEvent::EnabledChange) {
		onTextChanged(QString());
	}
}


} // namespace QtExt

