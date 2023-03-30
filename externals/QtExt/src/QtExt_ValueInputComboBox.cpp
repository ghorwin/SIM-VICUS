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

#include "QtExt_ValueInputComboBox.h"

#include <QMessageBox>
#include <QLineEdit>
#include <QKeyEvent>

#include "QtExt_Style.h"
#include "QtExt_Locale.h"


namespace QtExt {

bool ValueInputComboBox::eventFilter(QObject* obj, QEvent* event) {
	if (event->type()==QEvent::KeyPress) {
		QKeyEvent* key = static_cast<QKeyEvent*>(event);
		if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) ) {
			m_enterPressed = true;
			return false;
		}
	}
	return false;
}

ValueInputComboBox::ValueInputComboBox(QWidget *parent) :
	QComboBox(parent),
	m_enterPressed(false)
{
	QPalette palEdit;
	palEdit.setColor(QPalette::Base, Style::EditFieldBackground);
	setPalette(palEdit);

	setEditable(true);

	QLineEdit * le = lineEdit();
	le->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	connect(le, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

	connect(this, SIGNAL(activated(int)),
			this, SLOT(onActivated(int)));
	connect(this, SIGNAL(currentTextChanged(QString)),
			this, SLOT(onCurrentTextChanged(const QString&)));

	installEventFilter(this);
}


bool ValueInputComboBox::isValid() const {
	if (m_allowEmpty && currentText().trimmed().isEmpty())
		return true;
	double val;
	if (!isValidNumber(val))
		return false;

	return isValidImpl(val);
}


bool ValueInputComboBox::isValidNumber(double & val) const {
	bool ok;
	QString textTemp = currentText();
	if (m_acceptOnlyInteger) {
		val = QLocale().toInt(textTemp, &ok);
	}
	else {
		// first try current locale
		val = QtExt::Locale().toDouble(textTemp, &ok);
		// but also allow fall-back on C-locale
		if (!ok)
			val = textTemp.toDouble(&ok);
	}
	return ok;
}


void ValueInputComboBox::setValue(double value) {
	QString textTemp;
	if (m_formatter.get() != NULL) {
		textTemp = m_formatter->formatted(value);
	}
	else if(m_format != 'a' ) {
		textTemp = QLocale().toString(value, m_format, m_precision);
	}
	else {
		textTemp = QLocale().toString(value);
	}

	// this will update the appearance/state of the line edit, also trigger
	// onTextChanged() which will use the validator to detect correctness
	// of value and color the line appropriately
	setCurrentText(textTemp);

	// Note: when line edit is read-only or disabled, validator is not called and value
	// is not set -> do that manually since setting a value in
	// a read-only line edit can only be done by code and thus is expected
	// to be valid
	if (lineEdit()->isReadOnly() || !isEnabled())
		m_value = value;
}


void ValueInputComboBox::setCurrentText(const QString& str) {
	blockSignals(true);
	QComboBox::setCurrentText(str);
	blockSignals(false);
	onCurrentTextChanged(currentText()); // this will update the value and state of the line edit
}


void ValueInputComboBox::setEnabled(bool enabled) {
	QComboBox::setEnabled(enabled);
	onCurrentTextChanged(currentText());
}


void ValueInputComboBox::setFromParameter(const IBK::Parameter & p, const std::string & unit, double defaultValue) {
	if (!p.name.empty()) {
		try {
			double value = p.get_value(unit); // might throw, when user-defined parameter has incompatible unit
			setEditText( QString("%L1").arg(value) );
		}
		catch (...) {
			setEditText( QString("%L1").arg(p.value) );
		}
	}
	else {
		setEditText( QString("%L1").arg(defaultValue) );
	}
}


bool ValueInputComboBox::check(double minVal, double maxVal, bool minValAllowed, bool maxValAllowed,
							   const QString & outOfRangeMessage)
{
	if (!isEnabled())
		return true;
	// try to get a value
	bool ok;
	// first try current locale
	double value = QtExt::Locale().toDouble(currentText(), &ok);
	// but also allow fall-back on C-locale
	if (!ok)
		value = currentText().toDouble(&ok);
	if (!ok) {
		QMessageBox::critical(this, tr("Input error"), tr("A valid numeric number is needed."));
		setFocus();
		return false;
	}
	if (value < minVal || (!minValAllowed && value == minVal)) {
		QMessageBox::critical(this, tr("Input error"), outOfRangeMessage);
		setFocus();
		return false;
	}
	if (value > maxVal || (!maxValAllowed && value == maxVal)) {
		QMessageBox::critical(this, tr("Input error"), outOfRangeMessage);
		setFocus();
		return false;
	}
	return true;
}


double ValueInputComboBox::value() const {
	// try to get a value
	bool ok;
	// first try current locale
	QString ct = currentText();
	double value = QtExt::Locale().toDouble(ct, &ok);
	// but also allow fall-back on C-locale
	if (!ok)
		value = ct.toDouble(&ok);
	if (!ok)
		return 0;
	else
		return value;
}

void ValueInputComboBox::onActivated(int index) {
	if(m_enterPressed) {
		m_enterPressed = false;
		return;
	}
	int itemCount = count();
	if(index < itemCount)
		setEditText( QString("%L1").arg( itemData(index).toDouble()) );
}


void ValueInputComboBox::onCurrentTextChanged ( const QString& ) {
	if (!isEnabled()) {
		QPalette palEdit;
		setPalette(palEdit);
		setToolTip("");
		return;
	}

	// Note: isValid() will update m_value when its value in text is ok
	if (!isValid()) {
		QPalette palEdit;
		palEdit.setColor(QPalette::Base, Style::ErrorEditFieldBackground);
		setPalette(palEdit);
		if( m_validator.get() != NULL && !m_validator->toolTip().isEmpty()) {
			setToolTip(m_validator->toolTip());
		}
		else {
			setToolTip(m_toolTip);
		}
	}
	else {
		QPalette palEdit;
		palEdit.setColor(QPalette::Base, Style::EditFieldBackground);
		setPalette(palEdit);
		setToolTip("");
	}
}


void ValueInputComboBox::onEditingFinished() {
	if (isValid())
		emit editingFinishedSuccessfully();
}

} // namespace QtExt

