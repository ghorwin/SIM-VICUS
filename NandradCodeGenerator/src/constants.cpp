#include "constants.h"

const char * const QT_HEADER_TEMPLATE =

		"/*	The NANDRAD data model library.\n"
		"\n"
		"	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany\n"
		"\n"
		"	Primary authors:\n"
		"	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>\n"
		"	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>\n"
		"\n"
		"	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)\n"
		"\n"
		"	This library is free software; you can redistribute it and/or\n"
		"	modify it under the terms of the GNU Lesser General Public\n"
		"	License as published by the Free Software Foundation; either\n"
		"	version 3 of the License, or (at your option) any later version.\n"
		"\n"
		"	This library is distributed in the hope that it will be useful,\n"
		"	but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
		"	Lesser General Public License for more details.\n"
		"*/\n"
		"\n"
		"#ifndef ${PREFIX}_KeywordListQtH\n"
		"#define ${PREFIX}_KeywordListQtH\n\n"
		"#include <${PREFIX}_KeywordList.h>\n"
		"#include <QString>\n"
		"#include <QColor>\n"
		"#include <QCoreApplication>\n\n"
		"namespace ${NAMESPACE} {\n\n"
		"class KeywordListQt {\n"
		"Q_DECLARE_TR_FUNCTIONS(KeywordListQt)\n"
		"public:\n\n"
		"	KeywordListQt();\n\n"
		"	/*! Returns a keyword for an enum value t of type enumtype. */\n"
		"	static const char * Keyword(const char * const enumtype, int t){ return KeywordList::Keyword( enumtype, t ); }\n\n"
		"	/*! Returns a formate keyword string as in \"Keyword [Unit]\" for an enum value t of type enumtype. */\n"
		"	static QString FormatedKeyword(const char * const enumtype, int t){ return QString(\"%1 [%2]\").arg( KeywordList::Keyword( enumtype, t ) ).arg(KeywordList::Unit( enumtype, t ) ); } \n\n"
		"	/*! Returns a translated description for an enum value t of type enumtype.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		If no descrption is given, the keyword itself is returned.\n"
		"		\\param enumtype 		The full enumeration type including the class name.\n"
		"		\\param t 				The enumeration type cast in an int.\n"
		"		\\param no_description	The optional argument is set to true, if there was no description\n"
		"		for this keyword, otherwise to false.\n"
		"	*/\n"
		"	static QString Description(const std::string & category, int keywordId);\n\n"
		"	/*! Returns a default unit for an enum value t of type enumtype.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		Returns an empty string if no default unit was specified.\n"
		"		\\param enumtype The full enumeration type including the class name.\n"
		"		\\param t 		The enumeration type cast in an int.\n"
		"	*/\n"
		"	static const char * Unit(const char * const enumtype, int t){ return KeywordList::Unit( enumtype, t ); }\n\n"
		"	/*! Returns a default unit for an enum value t of type enumtype.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		Returns an empty string if no default unit was specified.\n"
		"		\\param enumtype The full enumeration type including the class name.\n"
		"		\\param t 		The enumeration type cast in an int.\n"
		"	*/\n"
		"	static QColor Color(const char * const enumtype, int t){ return QColor( KeywordList::Color( enumtype, t ) ); }\n\n"
		"	/*! Returns a default value for an enum value t of type enumtype.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		Returns an nan if no default value was specified.\n"
		"		\\param enumtype The full enumeration type including the class name.\n"
		"		\\param t 		The enumeration type cast in an int.\n"
		"	*/\n"
		"	static double DefaultValue(const char * const enumtype, int t){ return KeywordList::DefaultValue( enumtype, t ); }\n\n"
		"	/*! Returns an enumeration value for a given keyword kw of type enumtype.\n"
		"		This function throws an exception if the keyword or the enumeration type is invalid or unknown.\n"
		"		\\param enumtype 	The full enumeration type including the class name.\n"
		"		\\param kw 			The keyword string.\n"
		"		\\param deprecated 	The optional argument is set the true if the keyword kw is deprecated.\n"
		"	*/\n"
		"	static int Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated = nullptr){ return KeywordList::Enumeration( enumtype, kw, deprecated ); }\n\n\n"
		"	/*!	Returns the maximum index for entries of a category in the keyword list.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		\\param enumtype 	The full enumeration type including the class name.\n"
		"	*/\n"
		"	static int MaxIndex(const char * const enumtype){ return KeywordList::MaxIndex( enumtype ); }\n\n\n"
		"	/*! Returns the number of keywords in this category.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		\\param enumtype 	The full enumeration type including the class name.\n"
		"	*/\n"
		"	static unsigned int Count(const char * const enumtype){ return KeywordList::Count( enumtype ); }\n\n\n"
		"	/*! Checks whether a keyword exists in the enumeration of type enumtype.\n"
		"		\\return Returns true if the keyword is valid, otherwise false.\n"
		"	*/\n"
		"	static bool KeywordExists(const char * const enumtype, const std::string & kw){ return KeywordList::KeywordExists( enumtype, kw ); }\n\n\n"
		"	/*! Checks whether a category of type enumtype exists.\n"
		"		\\return Returns true if the category/enum type exists, otherwise false.\n"
		"	*/\n"
		"	static bool CategoryExists(const char * const enumtype){ return KeywordList::CategoryExists( enumtype ); }\n\n\n"

		"}; // class\n\n"
		"} // namespace \n"
		"/*!\n"
		"	\\file ${PREFIX}_KeywordList.h\n"
		"	\\brief Contains the declaration of class KeywordListQt.\n"
		"*/\n\n"
		"#endif // ${PREFIX}_KeywordListQtH\n";



const char * const QT_CPP_HEADER_TEMPLATE =

		"/*	The NANDRAD data model library.\n"
		"\n"
		"	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany\n"
		"\n"
		"	Primary authors:\n"
		"	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>\n"
		"	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>\n"
		"\n"
		"	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)\n"
		"\n"
		"	This library is free software; you can redistribute it and/or\n"
		"	modify it under the terms of the GNU Lesser General Public\n"
		"	License as published by the Free Software Foundation; either\n"
		"	version 3 of the License, or (at your option) any later version.\n"
		"\n"
		"	This library is distributed in the hope that it will be useful,\n"
		"	but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
		"	Lesser General Public License for more details.\n"
		"*/\n"
		"\n"
		"#include \"${PREFIX}_KeywordListQt.h\"\n\n"
		"namespace ${NAMESPACE} {\n"
		"KeywordListQt::KeywordListQt() {\n\n";

const char * const QT_CPP_FOOTER_TEMPLATE =

		"}\n\n\n"
		"QString KeywordListQt::Description( const std::string & category, int keywordId) { \n\n"
		"	std::string description = KeywordList::Description(category.c_str(), keywordId);\n"
		"	return tr(description.c_str());\n\n"
		"}\n\n"
		"} // namespace\n";


const char * const HEADER_TEMPLATE =

		"/*	The NANDRAD data model library.\n"
		"\n"
		"	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany\n"
		"\n"
		"	Primary authors:\n"
		"	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>\n"
		"	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>\n"
		"\n"
		"	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)\n"
		"\n"
		"	This library is free software; you can redistribute it and/or\n"
		"	modify it under the terms of the GNU Lesser General Public\n"
		"	License as published by the Free Software Foundation; either\n"
		"	version 3 of the License, or (at your option) any later version.\n"
		"\n"
		"	This library is distributed in the hope that it will be useful,\n"
		"	but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
		"	Lesser General Public License for more details.\n"
		"*/\n"
		"\n"
		"#ifndef ${PREFIX}_KeywordListH\n"
		"#define ${PREFIX}_KeywordListH\n\n"
		"#include <string>\n"
		"#include <typeinfo>\n\n"
		"#include <IBK_Parameter.h>\n"
		"#include <IBK_IntPara.h>\n\n"
		"namespace ${NAMESPACE} {\n\n"
		"/*! The class KeywordList provides conversion functionality between keyword strings\n"
		"	and their respective enumeration values.\n"
		"	Keyword strings are used in the NANDRAD project files as identifier. In the code\n"
		"	we use the enumeration values. You can obtain a keyword for a given enumeration value\n"
		"	with the static function Keyword(). The corresponding enumeration value can be\n"
		"	obtained with Enumeration(), as shown in the example below.\n"
		"	\\code\n"
		"	// to get the keyword string use\n"
		"	string kw = KeywordList::Keyword(\"Assignment::type_t\", Assignment::ST_MATERIAL);\n"
		"	// and to get the enumeration value\n"
		"	Assignment::type_t t = KeywordList::Enumeration(\"Assignment::type_t}\", kw);\n"
		"	// the string \"Assignment::type_t\" is the full enumeration type and formed from the class\n"
		"	// name and the enumeration type name\n"
		"	\\endcode\n"
		"	Remember to pass the correct and full enumeration type as first argument to Keyword.\n"
		"	\\note 	Instead of overloading the Keyword() function for different types\n"
		"			in the current implementation we use a generic form in order to avoid\n"
		"			adding compilation dependencies for\n"
		"			all CPP files in the project that include the keyword list.\n"
		"*/\n"
		"class KeywordList {\n"
		"public:\n"
		"	/*!Returns a keyword for an enum value t of type enumtype. */\n"
		"	static const char * Keyword(const char * const enumtype, int t);\n\n"
		"	/*! Returns a description for an enum value t of type enumtype.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		If no descrption is given, the keyword itself is returned.\n"
		"		\\param enumtype 		The full enumeration type including the class name.\n"
		"		\\param t 				The enumeration type cast in an int.\n"
		"		\\param no_description	The optional argument is set to true, if there was no description\n"
		"		for this keyword, otherwise to false.\n"
		"	*/\n"
		"	static const char * Description(const char * const enumtype, int t, bool * no_description = nullptr);\n\n"
		"	/*! Returns a default unit for an enum value t of type enumtype.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		Returns an empty string if no default unit was specified.\n"
		"		\\param enumtype The full enumeration type including the class name.\n"
		"		\\param t 		The enumeration type cast in an int.\n"
		"	*/\n"
		"	static const char * Unit(const char * const enumtype, int t);\n\n"
		"	/*! Returns a color string for an enum value t of type enumtype.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		Returns the color code for 'white' if no color value was specified.\n"
		"		\\param enumtype The full enumeration type including the class name.\n"
		"		\\param t 		The enumeration type cast in an int.\n"
		"	*/\n"
		"	static const char * Color(const char * const enumtype, int t);\n\n"
		"	/*! Returns a default value for an enum value t of type enumtype.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		Returns an nan if no default value was specified.\n"
		"		\\param enumtype The full enumeration type including the class name.\n"
		"		\\param t 		The enumeration type cast in an int.\n"
		"	*/\n"
		"	static double DefaultValue(const char * const enumtype, int t);\n\n"
		"	/*! Returns an enumeration value for a given keyword kw of type enumtype.\n"
		"		This function throws an exception if the keyword or the enumeration type is invalid or unknown.\n"
		"		\\param enumtype 	The full enumeration type including the class name.\n"
		"		\\param kw 			The keyword string.\n"
		"		\\param deprecated 	The optional argument is set the true if the keyword kw is deprecated.\n"
		"	*/\n"
		"	static int Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated = nullptr);\n\n\n"
		"	/*!	Returns the maximum index for entries of a category in the keyword list.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		\\param enumtype 	The full enumeration type including the class name.\n"
		"	*/\n"
		"	static int MaxIndex(const char * const enumtype);\n\n\n"
		"	/*! Returns the number of keywords in this category.\n"
		"		This function throws an exception if the enumeration type is invalid or unknown.\n"
		"		\\param enumtype 	The full enumeration type including the class name.\n"
		"	*/\n"
		"	static unsigned int Count(const char * const enumtype);\n\n"
		"	/*! Checks whether a keyword exists in the enumeration of type enumtype.\n"
		"		\\return Returns true if the keyword is valid, otherwise false.\n"
		"	*/\n"
		"	static bool KeywordExists(const char * const enumtype, const std::string & kw);\n"
		"	/*! Checks whether a category of type enumtype exists.\n"
		"		\\return Returns true if the category/enum type exists, otherwise false.\n"
		"	*/\n"
		"	static bool CategoryExists(const char * const enumtype);\n"
		"	/*! Convenience function to set an IBK::Parameter inside a static C-array of IBK::Parameters with\n"
		"		correct name, value and unit.\n"
		"		\\code\n"
		"		// Example: in declaration of class 'MyClass'\n\n"
		"		enum MyParameters {\n"
		"			MP_Temperature,			// Keyword: Temperature   [C]   'Some temperatures'\n"
		"			MP_Mass,				// Keyword: Mass [kg]           'Some mass'\n"
		"			NUM_MP\n"
		"		};\n\n"
		"		// static array with parameter values\n"
		"		IBK::Parameter m_para[NUM_MP];\n\n\n\n"
		"		// setting the parameter in code\n\n"
		"		NANDRAD::setParameter(m_para, \"MyClass::MyParameters\", MP_Temperature, 23.5);\n"
		"		NANDRAD::setParameter(m_para, \"MyClass::MyParameters\", MP_Mass, 1500);\n\n"
		"		\\endcode\n\n"
		"		\\param para Pointer to begin of array with IBK::Parameters\n"
		"		\\param enumtype The enumeration type (class::enumtypename)\n"
		"		\\param n Enumeration value (which parameter to set)\n"
		"		\\param val The value (given in the unit specified for the parameter; requires a unit to be given)\n"
		"	*/\n"
		"	static void setParameter(IBK::Parameter para[], const char * const enumtype, int n, const double &val);\n\n"
		"	/*! As setParameter(), but for IBK::IntPara. */\n"
		"	static void setIntPara(IBK::IntPara para[], const char * const enumtype, int n, const int &val);\n\n"
		"};\n\n"
		"} // namespace ${NAMESPACE}\n\n"
		"/*!\n"
		"	\\file ${PREFIX}_KeywordList.h\n"
		"	\\brief Contains the declaration of class KeywordList.\n"
		"*/\n\n"
		"#endif // ${PREFIX}_KeywordListH\n";


const char * const CPP_HEADER_TEMPLATE =

		"/*	The NANDRAD data model library.\n"
		"\n"
		"	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany\n"
		"\n"
		"	Primary authors:\n"
		"	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>\n"
		"	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>\n"
		"\n"
		"	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)\n"
		"\n"
		"	This library is free software; you can redistribute it and/or\n"
		"	modify it under the terms of the GNU Lesser General Public\n"
		"	License as published by the Free Software Foundation; either\n"
		"	version 3 of the License, or (at your option) any later version.\n"
		"\n"
		"	This library is distributed in the hope that it will be useful,\n"
		"	but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
		"	Lesser General Public License for more details.\n"
		"*/\n"
		"\n"
		"#include \"${PREFIX}_KeywordList.h\"\n\n"
		"#include <map>\n"
		"#include <limits>\n"
		"#include <iostream>\n\n"
		"#include <IBK_FormatString.h>\n"
		"#include <IBK_Exception.h>\n\n\n"
		"namespace ${NAMESPACE} {\n";

const char * const CPP_FOOTER =

		"	const char * KeywordList::Keyword(const char * const enumtype, int t) {\n"
		"		const char * const kw = theKeyword(enum2index(enumtype), t);\n"
		"		if (std::string(kw) == INVALID_KEYWORD_INDEX_STRING) {\n"
		"			throw IBK::Exception(IBK::FormatString(\"Cannot determine keyword for enumeration type '%1' and index '%2'.\")\n"
		"				.arg(enumtype).arg(t), \"[KeywordList::Keyword]\");\n"
		"		}\n"
		"		return kw;\n"
		"	}\n"
		"\n"
		"	bool KeywordList::KeywordExists(const char * const enumtype, const std::string & kw) {\n"
		"		int typenum = enum2index(enumtype);\n"
		"		int i = 0;\n"
		"		int maxIndexInCategory = MaxIndex( enumtype ); \n"
		"		for ( ; i <= maxIndexInCategory; ++i ) {\n"
		"			std::string keywords = allKeywords(typenum, i);\n"
		"			if (keywords == INVALID_KEYWORD_INDEX_STRING)\n"
		"				continue;\n"
		"			std::stringstream strm(keywords);\n"
		"			int j = 0;\n"
		"			std::string kws;\n"
		"			while (strm >> kws) {\n"
		"				if (kws == kw) {\n"
		"					return true; // keyword exists\n"
		"				}\n"
		"				++j;\n"
		"			}\n"
		"		}\n"
		"		return false; // nothing found keyword doesn't exist.\n"
		"	}\n"
		"\n"
		"	int KeywordList::Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated) {\n"
		"		int typenum = enum2index(enumtype);\n"
		"		int i = 0;\n"
		"		int maxIndexInCategory = MaxIndex( enumtype ); \n"
		"		for ( ; i <= maxIndexInCategory; ++i ) {\n"
		"			std::string keywords = allKeywords(typenum, i);\n"
		"			if (keywords == INVALID_KEYWORD_INDEX_STRING)\n"
		"				continue;\n"
		"			std::stringstream strm(keywords);\n"
		"			int j = 0;\n"
		"			std::string kws;\n"
		"			while (strm >> kws) {\n"
		"				if (kws == kw) {\n"
		"					if (deprecated != nullptr) {\n"
		"						*deprecated = (j != 0);\n"
		"					}\n"
		"					return i;\n"
		"				}\n"
		"				++j;\n"
		"			}\n"
		"		} // for ( ; i < maxIndexInCategory; ++i ) {\n"
		"		throw IBK::Exception(IBK::FormatString(\"Cannot determine enumeration value for \"\n"
		"			\"enumeration type '%1' and keyword '%2'.\")\n"
		"			.arg(enumtype).arg(kw), \"[KeywordList::Enumeration]\");\n"
		"	}\n\n"
		"	bool KeywordList::CategoryExists(const char * const enumtype) {\n"
		"		return enum2index(enumtype) != -1;\n"
		"	}\n\n"
		"	void KeywordList::setParameter(IBK::Parameter para[], const char * const enumtype, int n, const double &val) {\n"
		"		para[n] = IBK::Parameter(Keyword(enumtype, n), val, Unit(enumtype, n));\n"
		"	}\n\n"
		"	void KeywordList::setIntPara(IBK::IntPara para[], const char * const enumtype, int n, const int &val) {\n"
		"		para[n] = IBK::IntPara(Keyword(enumtype, n), val);\n"
		"	}\n"
		"\n";


const char * const CPP_READWRITE_HEADER =
		"/*	The NANDRAD data model library.\n"
		"\n"
		"	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany\n"
		"\n"
		"	Primary authors:\n"
		"	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>\n"
		"	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>\n"
		"\n"
		"	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)\n"
		"\n"
		"	This library is free software; you can redistribute it and/or\n"
		"	modify it under the terms of the GNU Lesser General Public\n"
		"	License as published by the Free Software Foundation; either\n"
		"	version 3 of the License, or (at your option) any later version.\n"
		"\n"
		"	This library is distributed in the hope that it will be useful,\n"
		"	but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
		"	Lesser General Public License for more details.\n"
		"*/\n"
		"\n"
		"#include <${HEADER_FILE}>\n"
		"#include <${PREFIX}_KeywordList.h>\n"
		"\n"
		"#include <IBK_messages.h>\n"
		"#include <IBK_Exception.h>\n"
		"#include <IBK_StringUtils.h>\n"
		"#include <${PREFIX}_Constants.h>\n"
		"${OTHER_INCLUDES}"
		"\n"
		"#include <tinyxml.h>\n"
		"\n"
		"namespace ${PREFIX} {\n"
		"\n";

const char * const CPP_WRITEXML =
		"TiXmlElement * ${CLASSNAME}::writeXML${PRIVATE}(TiXmlElement * parent) const {\n"
		"	TiXmlElement * e = new TiXmlElement(\"${CLASSNAME}\");\n"
		"	parent->LinkEndChild(e);\n\n"
		"${COMMENTS}${ATTRIBUTES}${CHILD_ELEMENTS}"
		"	return e;\n"
		"}\n";

const char * const CPP_READXML =
		"void ${CLASSNAME}::readXML${PRIVATE}(const TiXmlElement * element) {\n"
		"	FUNCID(${CLASSNAME}::readXML${PRIVATE});\n"
		"\n"
		"	try {\n"
		"${ATTRIBUTES}${CHILD_ELEMENTS}"
		"	}\n"
		"	catch (IBK::Exception & ex) {\n"
		"		throw IBK::Exception( ex, IBK::FormatString(\"Error reading '${CLASSNAME}' element.\"), FUNC_ID);\n"
		"	}\n"
		"	catch (std::exception & ex2) {\n"
		"		throw IBK::Exception( IBK::FormatString(\"%1\\nError reading '${CLASSNAME}' element.\").arg(ex2.what()), FUNC_ID);\n"
		"	}\n"
		"}\n";


