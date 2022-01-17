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

#ifndef QtExt_CoordinateIndexEditH
#define QtExt_CoordinateIndexEditH

#include <QWidget>
#include <QVector>

#include "QtExt_ValidatingInputBase.h"

namespace QtExt {

namespace Ui {
class CoordinateIndexEdit;
}

/*! \brief CoordinateIndexEdit allows to edit a coordinate vector by value and/or index.
	The ccordinate vector must be set before use. The vector must be sorted.
	The edit widget consists of two combined edit fields.
	One spin edit for the indices and one ValidatingLineEdit for the coordinate values. For properties of this edit field \sa QtExt::ValidatingLineEdit .
*/
class CoordinateIndexEdit : public QWidget
{
	Q_OBJECT

public:
	/*! Default c'tor. */
	explicit CoordinateIndexEdit(QWidget *parent = nullptr);

	/*! Standard destructor. Delete the ui.*/
	~CoordinateIndexEdit();

	/*! Initialises the internal coordinate vector.*/
	void set(const QVector<double>& coordinates);

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
	double value() const;

	/*! Return the current index of the coordinate vector.*/
	int index() const;

	/*! Set the index to the given one and updates coordinate with value from internal vector at given position.
	   \param index Index in coordinate vector.
	*/
	void setIndex(unsigned int index);

protected:
	void changeEvent(QEvent *e);

signals:
	/*! Emits the result of the editing, but only if a result was entered correctly. */
	void editingFinishedSuccessfully();

private slots:
	/*! Is called then a change in the edit field is finished in order to check the values. */
	void onEditingFinished();

	/*! Is called then index in spin box is changed.*/
	void onIndexChanged(int index);

private:
	Ui::CoordinateIndexEdit *ui;
	QVector<double>	m_coordinates;
	bool			m_ascending;
	int				m_maxCoordinateIndex;
	int				m_minCoordinateIndex;

	/*! Return if the internal coordinate vector is valid.*/
	bool			validCoordinates() const;
};


} // namespace QtExt
#endif // QtExt_CoordinateIndexEditH
