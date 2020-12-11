/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

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
		virtual ~FormatterBase() {}

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
	~ValidatingInputBase();

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
