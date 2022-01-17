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

#ifndef QtExt_ParameterEditH
#define QtExt_ParameterEditH

#include "QtExt_ValidatingLineEdit.h"

#include <IBK_Unit.h>

class QComboBox;

namespace IBK {
	class Parameter;
}


namespace QtExt {

/*! A special variant of a validating line edit with optional unit combo buddy and validation functionality.
	\code
	// ** Setup with unit combo **
	// 1. First set limits, minVal and maxVal are given in the SI base unit of the units shown
	//    in the unit combo.
	lineEdit->setup(comboBoxUnits, IBK::Unit("h"), minVal, maxVal, toolTip);
	//    Note: minVal and maxVal are in the base unit of the given SI base unit of
	//    the unit passed
	//
	// 3. After initial setup, you can set the parameter in the line edit with two options:
	//    a) with default value, if parameter is currently unset
	lineEdit->setFromParameter(para, unit, defaultValue); // defaultValue in unit 'unit'
	//    b) with empty line edit when parameter is currently unset
	lineEdit->setFromParameter(para, unit);
	// Note: if paramete is set, its base unit _must_ match the unit in the unit combo, otherwise an exception is raised

	// 4. Finally, connect to the editingFinishedSuccessfully() signal, which is only emitted when
	// the user has changed the line edit and/or the combo box and the result is a valid entry
	// with respect to the min/max values.
	connect(lineEdit, SIGNAL(editingFinishedSuccessfully()), this SLOT(onEditingFinishedSuccessfully()));
	\endcode
*/
class ParameterEdit : public ValidatingLineEdit {
	Q_OBJECT
public:
	/*! Default c'tor. */
	explicit ParameterEdit(QWidget *parent = 0);

	/*! Sets up the line edit and the associated combo box.
		The values minVal and maxVal are in the given display unit.
		\code
		// set up the line edit to hold temperatures between 0..100 degC.
		lineEdit->setup(unitCombo, IBK::Unit("C"), 0, 100, tr("Must be between 0 .. 100 degree C!"));
		\endcode
	*/
	void setup(QComboBox * comboBox, const IBK::Unit & unit,
			   double minVal, double maxVal, const QString & toolTip, bool minValAllowed = false, bool maxValAllowed = false);

	/*! Transfers the value of the parameter to the line edit if the parameter is set.
		Otherwise (if parameter name is empty) the line edit is cleared and the unit remains unchanged.
		\warning If the parameter holds a non-matching unit, an exception is thrown.
		\param param Holds the physical value including parameter for the line edit.
	*/
	void setFromParameter(const IBK::Parameter & param);

	/*! Transfers the value of the parameter to the line edit if the parameter is set.
		Otherwise (if parameter name is empty) the line edit is cleared and the unit (function argument)
		is set in the buddy combo box.
		The unit is used to convert the parameter value into a selected unit.
		\param param Holds the physical value including parameter for the line edit.
		\param unit The unit to select in the combo box, if parameter is empty (otherwise the display
			unit of the parameter is selected).
		\warning Throws an exception if the unit in the parameter cannot be converted to the unit
				of the buddy combobox.
	*/
	void setFromParameter(const IBK::Parameter & param, const IBK::Unit & unit);

	/*! Transfers the value of the parameter to the line edit if the parameter is set.
		If the parameter is set (name not empty) the IO unit of the parameter is selected in the combo box
		and the value stored in the line edit. The IO unit must exist in the combo box, otherwise an exception is thrown.
		If the parameter is empty, the the default value is shown with the given default unit.
	*/
	void setFromParameter(const IBK::Parameter & param, double defaultValue, const IBK::Unit & defaultUnit);

	/*! Enables/disables line edit. */
	void setEnabled(bool enabled);

	/*! Returns content of this line edit as IBK::Parameter.
		When a unit combo buddy is set, it uses the unit in the combo box as display unit.
		Otherwise, it uses the display unit cached from setFromParameter().
	*/
	IBK::Parameter toParameter(const char * const parameterName) const;

	/*! Returns the unit currently shown in the unit combo. */
	IBK::Unit currentUnit() const;

	/*! Fills combo box with units (combo box must be empty, and should be disconnected or signal-blocked). */
	static void populateUnitCombo(QComboBox * combo, const IBK::Unit & u);

public slots:
	/*! Connected to a combo box change whenever a unit combo buddy is set. */
	void unitChanged(QString unit);

private:
	/*! Setup function of parent class is private and not implemented to disallow usage.*/
	void setup(double minVal, double maxVal, const QString & toolTip, bool minValueAllowed, bool maxValueAllowed);

	/*! Sets up the buddy unit combo.
		If previously a combo box was associated, its signal-slot connections to this line edit are removed.
		The new combo box is filled with all units convertible to the given unit, afterwards the unit is
		selected in the combo box and the change signal is connected.
		Set a NULL pointer as combo box to remove the association and signal-slot connection with the combo box.
	*/
	void setBuddyUnitCombo(QComboBox * comboBox, const IBK::Unit & unit);

	/*! Sets unit in buddy unit combo box.
		\warning You must set a buddy unit combo with setBuddyUnitCombo() before calling this function.
		\note If the buddy unit combo was not set, or the units are not convertible, and exception is raised.

		This function does not emit the editingFinishedSuccessfully() signal.
	*/
	void setUnit(const IBK::Unit & unit);

	QComboBox			*m_buddyUnitCombo;
	/*! Display unit cached from call to setFromParameter().
		If a unit combo buddy widget is set, the display unit will be synchronized
		with the unit selected in the combo box.
	*/
	IBK::Unit			m_displayUnit;
};

} // namespace QtExt

#endif // QtExt_ParameterEditH
