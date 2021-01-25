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

#ifndef VICUS_KeywordListQtH
#define VICUS_KeywordListQtH

#include <VICUS_KeywordList.h>
#include <QString>
#include <QColor>
#include <QCoreApplication>

namespace VICUS {

class KeywordListQt {
Q_DECLARE_TR_FUNCTIONS(KeywordListQt)
public:

	KeywordListQt();

	/*! Returns a keyword for an enum value t of type enumtype. */
	static const char * Keyword(const char * const enumtype, int t){ return KeywordList::Keyword( enumtype, t ); }

	/*! Returns a formate keyword string as in "Keyword [Unit]" for an enum value t of type enumtype. */
	static QString FormatedKeyword(const char * const enumtype, int t){ return QString("%1 [%2]").arg( KeywordList::Keyword( enumtype, t ) ).arg(KeywordList::Unit( enumtype, t ) ); }

	/*! Returns a translated description for an enum value t of type enumtype.
		This function throws an exception if the enumeration type is invalid or unknown.
		If no descrption is given, the keyword itself is returned.
		\param enumtype 		The full enumeration type including the class name.
		\param t 				The enumeration type cast in an int.
		\param no_description	The optional argument is set to true, if there was no description
		for this keyword, otherwise to false.
	*/
	static QString Description(const std::string & category, int keywordId);

	/*! Returns a default unit for an enum value t of type enumtype.
		This function throws an exception if the enumeration type is invalid or unknown.
		Returns an empty string if no default unit was specified.
		\param enumtype The full enumeration type including the class name.
		\param t 		The enumeration type cast in an int.
	*/
	static const char * Unit(const char * const enumtype, int t){ return KeywordList::Unit( enumtype, t ); }

	/*! Returns a default unit for an enum value t of type enumtype.
		This function throws an exception if the enumeration type is invalid or unknown.
		Returns an empty string if no default unit was specified.
		\param enumtype The full enumeration type including the class name.
		\param t 		The enumeration type cast in an int.
	*/
	static QColor Color(const char * const enumtype, int t){ return QColor( KeywordList::Color( enumtype, t ) ); }

	/*! Returns a default value for an enum value t of type enumtype.
		This function throws an exception if the enumeration type is invalid or unknown.
		Returns an nan if no default value was specified.
		\param enumtype The full enumeration type including the class name.
		\param t 		The enumeration type cast in an int.
	*/
	static double DefaultValue(const char * const enumtype, int t){ return KeywordList::DefaultValue( enumtype, t ); }

	/*! Returns an enumeration value for a given keyword kw of type enumtype.
		This function throws an exception if the keyword or the enumeration type is invalid or unknown.
		\param enumtype 	The full enumeration type including the class name.
		\param kw 			The keyword string.
		\param deprecated 	The optional argument is set the true if the keyword kw is deprecated.
	*/
	static int Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated = nullptr){ return KeywordList::Enumeration( enumtype, kw, deprecated ); }


	/*!	Returns the maximum index for entries of a category in the keyword list.
		This function throws an exception if the enumeration type is invalid or unknown.
		\param enumtype 	The full enumeration type including the class name.
	*/
	static int MaxIndex(const char * const enumtype){ return KeywordList::MaxIndex( enumtype ); }


	/*! Returns the number of keywords in this category.
		This function throws an exception if the enumeration type is invalid or unknown.
		\param enumtype 	The full enumeration type including the class name.
	*/
	static unsigned int Count(const char * const enumtype){ return KeywordList::Count( enumtype ); }


	/*! Checks whether a keyword exists in the enumeration of type enumtype.
		\return Returns true if the keyword is valid, otherwise false.
	*/
	static bool KeywordExists(const char * const enumtype, const std::string & kw){ return KeywordList::KeywordExists( enumtype, kw ); }


	/*! Checks whether a category of type enumtype exists.
		\return Returns true if the category/enum type exists, otherwise false.
	*/
	static bool CategoryExists(const char * const enumtype){ return KeywordList::CategoryExists( enumtype ); }


}; // class

} // namespace
/*!
	\file VICUS_KeywordList.h
	\brief Contains the declaration of class KeywordListQt.
*/

#endif // VICUS_KeywordListQtH
