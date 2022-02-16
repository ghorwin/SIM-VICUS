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

#ifndef QtExt_ValidatingLineEditH
#define QtExt_ValidatingLineEditH

#include <memory>

#include <QLineEdit>
#include <QString>
#include <QLocale>

#include "QtExt_ValidatingInputBase.h"

#include <IBK_Parameter.h>

namespace QtExt {

/*!	\brief Class ValidatingLineEdit provides validation functionality for entered double numbers and
			changes color of line edit when invalid content is entered.

	This line edit checks the numeric input for valid values and colors the line if an error occurs. In case
	of invalid content a tooltip is set that shows why the input is invalid.

	Currently set QLocale() will be used for converting numbers to/from the line edit.

	\code
	// ** Setup **
	// 1. First set limits of the acceptable value range, the last two arguments are optional and
	//	  can be used to specify whether the min and max values themselves are part of the acceptable range.
	lineEdit->setup(minVal, maxVal, toolTip, false, true);

	// 2. Connect to the editingFinishedSuccessfully() signal, which is only emitted when
	// the user has changed the line edit and/or the combo box and the result is a valid entry
	// with respect to the min/max values.
	connect(lineEdit, SIGNAL(editingFinishedSuccessfully()), this SLOT(onEditingFinishedSuccessfully()));
	\endcode

	The following code illustrates the usual work flow with the line edit.
	\code
	// Set any value either by using
	lineEdit->setValue(15);

	// or by setting the value directly via setText()
	lineEdit->setText(QString("%L1").arg(15, 0, 'f', 2));

	// when user has finished, the value can be retrieved
	if (lineEdit->isValid()) {
		double value = lineEdit->value();
		// ...
	}
	\endcode
*/
class ValidatingLineEdit : public QLineEdit, public ValidatingInputBase {
	Q_OBJECT
public:
	/*! Default c'tor. */
	explicit ValidatingLineEdit(QWidget * parent = 0);

	/*! Returns true if the input is a valid number and the number matches the validation rule.*/
	bool isValid() const;

	/*! Sets a new validator. Class will take ownership of this.
		Any existing validator will be deleted.
		Calls same function from base class ValidatingInputBase. Necessary in order to avoid abiguity with QLineEdit.
		\param validator New validator as derivation of ValidatorBase.
	*/
	void setValidator(ValidatorBase* validator);

	/*! Checks if the line edit contains a valid double number (regardless of range test).
		\param val The value is stored herein if the line edit's text can be parsed into a number.
		\return Returns true, if the line edit's text can be parsed into a number.
	*/
	bool isValidNumber(double & val) const;

	/*! Set the enabled state (overloaded to change background color).
		\param enabled Enabled state of widget.
	*/
	void setEnabled(bool enabled);

	/*! Set whether line edit is read only (overloaded to change background color).
		\param readOnly Read-only state of widget.
	*/
	void setReadOnly(bool readOnly);

	/*! Configure the line edit to only accept integers (if argument is true).
		The test of integer is done prior to checking ranges or calling the validator.
	*/
	void setAcceptOnlyInteger(bool acceptOnlyInteger) { m_acceptOnlyInteger = acceptOnlyInteger; }

	/*! Configures line edit to allow an empty field.
		\param allowEmpty If this is set to true, the line edit will be valid even if the line edit is empty.
		\note When retrieving values from such an input, first check for empty line before
		calling value().
		\code
		if (!lineEdit->text().trimmed().isEmpty())
			val = lineEdit->value();
		\endcode
		The placeholder text can alternatively be set/changed with a call to setPlaceholderText();
	*/
	void setEmptyAllowed(bool allowEmpty, const QString & placeholderText);

	/*! Convenience function for setting the data of a parameter in the line edit.
		If parameter is defined (name not empty), converts the value of the parameter into the given
		unit and sets this value in the line edit.
		If units mismatch, an exception is thrown. If parameter is empty, line edit is cleared.
	*/
	void setFromParameter(const IBK::Parameter & p, const IBK::Unit & u);

	/*! Convenience function for setting the data of a parameter in the line edit.
		If parameter is defined (name not empty), converts the value of the parameter into the given
		unit and sets this value in the line edit.
		If parameter is empty or units mismatch, the line edit is cleared.
	*/
	void setFromParameterOrClear(const IBK::Parameter & p, const IBK::Unit & u);

	/*! Convenience function for setting the data of a parameter in the line edit.
		If parameter is defined (name not empty), converts the value of the parameter into the unit
		of the default parameter defaultPara, otherwise the default parameter value is stored in the line edit.
		\warning Units of p and defaultPara are expected to be convertible, otherwise an assertion is raised.
	*/
	void setFromParameterOrDefault(const IBK::Parameter & p, const IBK::Parameter & defaultPara);

	/*! Sets a double value as text for the edit field using the current format or formatter.
		\param value Value to be set.
		\sa setFormat(), setFormatter(), FormatterBase
	*/
	void setValue(double value);

	/*! Returns the current value of the line edit.
		Returns the last valid number that was entered in the line edit. If the line edit currently contains
		an invalid number, the last number that was accepted is returned.
		\note You should only call this function after isValid() returned true.
	*/
	double value() const { return m_value; }

	/*! Overloaded to ensure validation is applied when setting text. */
	void setText(const QString& text);

	/*! Performs a new validating check without changing the entry.
		Useful for special validators with dependent edits.
	*/
	void check() {
		QString t = text();
		setText(t);
	}


protected:

	/*! Overloaded to react on enabled change events. */
	virtual void changeEvent(QEvent *event) override;

signals:
	/*! Emits the result of the editing, but only if a result was entered correctly. */
	void editingFinishedSuccessfully();

private slots:
	/*! Evaluates input and colors line edit accordingly. */
	void onEditingFinished();

	/*! Evaluates input and colors line edit accordingly. */
	void onTextChanged ( const QString & text );

};

} // namespace QtExt

#endif // QtExt_ValidatingLineEditH
