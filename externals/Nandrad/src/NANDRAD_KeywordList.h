/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NANDRAD_KeywordListH
#define NANDRAD_KeywordListH

#include <string>
#include <typeinfo>

#include <IBK_Parameter.h>
#include <IBK_IntPara.h>

namespace NANDRAD {

/*! The class KeywordList provides conversion functionality between keyword strings
	and their respective enumeration values.
	Keyword strings are used in the NANDRAD project files as identifier. In the code
	we use the enumeration values. You can obtain a keyword for a given enumeration value
	with the static function Keyword(). The corresponding enumeration value can be
	obtained with Enumeration(), as shown in the example below.
	\code
	// to get the keyword string use
	string kw = KeywordList::Keyword("Assignment::type_t", Assignment::ST_MATERIAL);
	// and to get the enumeration value
	Assignment::type_t t = KeywordList::Enumeration("Assignment::type_t}", kw);
	// the string "Assignment::type_t" is the full enumeration type and formed from the class
	// name and the enumeration type name
	\endcode
	Remember to pass the correct and full enumeration type as first argument to Keyword.
	\note 	Instead of overloading the Keyword() function for different types
			in the current implementation we use a generic form in order to avoid
			adding compilation dependencies for
			all CPP files in the project that include the keyword list.
*/
class KeywordList {
public:
	/*!Returns a keyword for an enum value t of type enumtype. */
	static const char * Keyword(const char * const enumtype, int t);

	/*! Returns a description for an enum value t of type enumtype.
		This function throws an exception if the enumeration type is invalid or unknown.
		If no descrption is given, the keyword itself is returned.
		\param enumtype 		The full enumeration type including the class name.
		\param t 				The enumeration type cast in an int.
		\param no_description	The optional argument is set to true, if there was no description
		for this keyword, otherwise to false.
	*/
	static const char * Description(const char * const enumtype, int t, bool * no_description = nullptr);

	/*! Returns a default unit for an enum value t of type enumtype.
		This function throws an exception if the enumeration type is invalid or unknown.
		Returns an empty string if no default unit was specified.
		\param enumtype The full enumeration type including the class name.
		\param t 		The enumeration type cast in an int.
	*/
	static const char * Unit(const char * const enumtype, int t);

	/*! Returns a color string for an enum value t of type enumtype.
		This function throws an exception if the enumeration type is invalid or unknown.
		Returns the color code for 'white' if no color value was specified.
		\param enumtype The full enumeration type including the class name.
		\param t 		The enumeration type cast in an int.
	*/
	static const char * Color(const char * const enumtype, int t);

	/*! Returns a default value for an enum value t of type enumtype.
		This function throws an exception if the enumeration type is invalid or unknown.
		Returns an nan if no default value was specified.
		\param enumtype The full enumeration type including the class name.
		\param t 		The enumeration type cast in an int.
	*/
	static double DefaultValue(const char * const enumtype, int t);

	/*! Returns an enumeration value for a given keyword kw of type enumtype.
		This function throws an exception if the keyword or the enumeration type is invalid or unknown.
		\param enumtype 	The full enumeration type including the class name.
		\param kw 			The keyword string.
		\param deprecated 	The optional argument is set the true if the keyword kw is deprecated.
	*/
	static int Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated = nullptr);


	/*!	Returns the maximum index for entries of a category in the keyword list.
		This function throws an exception if the enumeration type is invalid or unknown.
		\param enumtype 	The full enumeration type including the class name.
	*/
	static int MaxIndex(const char * const enumtype);


	/*! Returns the number of keywords in this category.
		This function throws an exception if the enumeration type is invalid or unknown.
		\param enumtype 	The full enumeration type including the class name.
	*/
	static unsigned int Count(const char * const enumtype);

	/*! Checks whether a keyword exists in the enumeration of type enumtype.
		\return Returns true if the keyword is valid, otherwise false.
	*/
	static bool KeywordExists(const char * const enumtype, const std::string & kw);
	/*! Checks whether a category of type enumtype exists.
		\return Returns true if the category/enum type exists, otherwise false.
	*/
	static bool CategoryExists(const char * const enumtype);
	/*! Convenience function to set an IBK::Parameter inside a static C-array of IBK::Parameters with
		correct name, value and unit.
		\code
		// Example: in declaration of class 'MyClass'

		enum MyParameters {
			MP_Temperature,			// Keyword: Temperature   [C]   'Some temperatures'
			MP_Mass,				// Keyword: Mass [kg]           'Some mass'
			NUM_MP
		};

		// static array with parameter values
		IBK::Parameter m_para[NUM_MP];



		// setting the parameter in code

		NANDRAD::setParameter(m_para, "MyClass::MyParameters", MP_Temperature, 23.5);
		NANDRAD::setParameter(m_para, "MyClass::MyParameters", MP_Mass, 1500);

		\endcode

		\param para Pointer to begin of array with IBK::Parameters
		\param enumtype The enumeration type (class::enumtypename)
		\param n Enumeration value (which parameter to set)
		\param val The value (given in the unit specified for the parameter; requires a unit to be given)
	*/
	static void setParameter(IBK::Parameter para[], const char * const enumtype, int n, const double &val);

	/*! As setParameter(), but for IBK::IntPara. */
	static void setIntPara(IBK::IntPara para[], const char * const enumtype, int n, const int &val);

};

} // namespace NANDRAD

/*!
	\file NANDRAD_KeywordList.h
	\brief Contains the declaration of class KeywordList.
*/

#endif // NANDRAD_KeywordListH
