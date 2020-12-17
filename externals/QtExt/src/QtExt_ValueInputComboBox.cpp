/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "QtExt_ValueInputComboBox.h"

#include <QMessageBox>
#include <QLineEdit>

#include "QtExt_Style.h"
#include "QtExt_Locale.h"


namespace QtExt {

ValueInputComboBox::ValueInputComboBox(QWidget *parent) :
	QComboBox(parent)
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

