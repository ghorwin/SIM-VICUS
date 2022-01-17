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

#ifndef QtExt_ValidatingInputBaseH
#define QtExt_ValidatingInputBaseH

#include <memory>
#include <QString>
#include <QObject>

namespace QtExt {

	/*! \brief Base class for validators used in ValidatingLineEdit.
		This class must be used as base class if one want to implement a validator to ValidatingLineEdit.
		It contains only two pure virtual functions isValid and toolTip.
		Custom validators are useful, when simple min/max range checks are not sufficient, e.g. when
		inputs of two line edits depend on each other: value A > value B.

		\note Class derives from QObject so that translated texts can be used for tool tips.
	*/
	class ValidatorBase : public QObject {
		Q_OBJECT
	public:

		/*! This function controls if the given value is valid or not.
			\param value Double value for checking.
			\return Returns true if value passed validation checks.
		*/
		virtual bool isValid(double value) = 0;

		/*! Returns a toolTip for the case of invalid values.
			It can be used in derived classes in order to create different tool tips depending on the reason
			why the value is not valid.
		*/
		virtual QString toolTip() = 0;
	};


	/*! \brief Base class for value to string formaters used in ValidatingLineEdit.
		This class must be used as base class if one want to implement a formatter to ValidatingLineEdit.
		It contains only one pure virtual function formatted.
		A custom formatter is meaningful if format depends on magnitude of value and generic formatting is not desired.
	*/
	class FormatterBase {
	public:
		/*! If derived classes hold members. */
		virtual ~FormatterBase();

		/*! This function returns a string representing the value.
			\param value Double value for formatting.
			\return Function shall return the double value properly formatted.
		*/
		virtual QString formatted(double value) = 0;
	};

/*! This class holds and manages validator and formatter objects.
	It is meant to be inherited by Qt-based widget classes, that want to provide formatting/validation capabilities.
*/
class ValidatingInputBase {
public:
	ValidatingInputBase();
	virtual ~ValidatingInputBase();

	/*! Set the range for validating and the tool tip string.
		\param minVal Minimum Value.
		\param maxVal Maximum Value.
		\param toolTip Tool tip which is shown if the value isn't valid.
		\param minValueAllowed Minimum value itself is part of accepted value range.
		\param maxValueAllowed Maximum value itself is part of accepted value range.
	*/
	void setup(double minVal, double maxVal, const QString & toolTip, bool minValueAllowed = false, bool maxValueAllowed = false);

	/*! Set the range for validating and the tool tip string.
		\param maxVal Maximum Value.
		\param maxValueAllowed Maximum value itself is part of accepted value range.
	*/
	void setMaximum(double maxVal, bool maxValueAllowed = false);

	/*! Set the range for validating and the tool tip string.
		\param maxVal Maximum Value.
		\param maxValueAllowed Maximum value itself is part of accepted value range.
	*/
	void setMinimum(double minVal, bool minValueAllowed = false);

	/*! Sets a new validator. Class will take ownership of this.
		Any existing validator will be deleted.
		\param validator New validator as derivation of ValidatorBase.
	*/
	void setValidator(ValidatorBase* validator);

	/*! Returns the current validator or NULL if no validator exist.*/
	ValidatorBase* validator() const;

	/*! Set the internal format string that is used by setValue().
		\param format specifies the number format. It can be:
			\li e format as [-]9.9e[+|-]999
			\li E format as [-]9.9E[+|-]999
			\li f format as [-]9.9
			\li g use e or f format, whichever is the most concise
			\li G use E or f format, whichever is the most concise
			\li a no format (default formatting)

		\param precision specifies for 'e', 'E' and 'f' the number of digits after decimal point
				and for 'g' and 'G' the maximum number of significant digits, if format 'a' is set, precision has no meaning.
		Format will be applied in next call to setValue().
		\note Users may still enter text in their own fashion, user input is not corrected to match this format.
	*/
	void setFormat(char format, int precision);

	/*! Sets a new formatter which will be used instead of built-in formatting rules in calls to setValue().
		Line edit will take ownership of this. Previously assigned formatter will be deleted.
		\param formatter New validator as derivation of ValidatorBase.
	*/
	void setFormatter(FormatterBase* formatter);

	/*! Returns the current formater or NULL if no formater exist.*/
	FormatterBase* formatter() const;

protected:
	// *** PROTECTED MEMBER FUNCTIONS ***

	/*! Test function for the given value.
		It returns true if the given value fits for all tests given in the current instance.
		\param value Value to be set.
	*/
	bool isValidImpl(double val) const;

	// *** PROTECTED MEMBER VARIABLES ***

	/*! Lower limit of values that can be entered. */
	double							m_minVal;
	/*! Upper limit of values that can be entered. */
	double							m_maxVal;
	/*! Tool tip to be shown when invalid input is entered. */
	QString							m_toolTip;
	/*! If true, the lower limit is included in the range. */
	bool							m_minValueAllowed;
	/*! If true, the upper limit is included in the range. */
	bool							m_maxValueAllowed;
	/*! If true, line edit may be empty and still valid (default is false). */
	bool							m_allowEmpty;
	/*! If true, only integer values are accepted. */
	bool							m_acceptOnlyInteger;

#if __cplusplus <= 199711L
	/*! Holds validator object if provided. */
	std::auto_ptr<ValidatorBase>	m_validator;
	/*! Holds formatter object if provided. */
	std::auto_ptr<FormatterBase>	m_formatter;
#else
	/*! Holds validator object if provided. */
	std::unique_ptr<ValidatorBase>	m_validator;
	/*! Holds formatter object if provided. */
	std::unique_ptr<FormatterBase>	m_formatter;
#endif //__cplusplus <= 199711L

	/*! Standard number format if no formatter is provided. */
	char							m_format;
	/*! Standard precision if no formatter is provided. */
	int								m_precision;

	/*! Caches value that was last accepted, updated in isValid() (therefore mutable). */
	mutable double					m_value;

};


} // namespace QtExt

#endif // QtExt_ValidatingInputBaseH
