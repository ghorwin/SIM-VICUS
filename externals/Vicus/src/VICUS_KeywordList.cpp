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

#include "VICUS_KeywordList.h"

#include <map>
#include <limits>
#include <iostream>

#include <IBK_FormatString.h>
#include <IBK_Exception.h>


namespace VICUS {
	/*! Holds the string to print as error when an invalid keyword is encountered. */
	const char * const INVALID_KEYWORD_INDEX_STRING = "KEYWORD_ERROR_STRING: Invalid type index";

	/*! Holds a list of all enum types/categories. */
	const char * const ENUM_TYPES[4] = {
		"NetworkFluid::para_t",
		"PlaneGeometry::type_t",
		"Project::ViewFlags",
		"ViewSettings::Flags"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<4; ++i) {
			if (enumtype == ENUM_TYPES[i]) return i;
		}
		//std::cerr << "Unknown enumeration type '" << enumtype<< "'." << std::endl;
		return -1;
	}
	

	/*! Returns a keyword string for a given category (typenum) and type number t. */
	const char * theKeyword(int typenum, int t) {
		switch (typenum) {
			// NetworkFluid::para_t
			case 0 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// PlaneGeometry::type_t
			case 1 :
			switch (t) {
				case 0 : return "Triangle";
				case 1 : return "Rectangle";
				case 2 : return "Polygon";
			} break;
			// Project::ViewFlags
			case 2 :
			switch (t) {
				case 0 : return "All";
			} break;
			// ViewSettings::Flags
			case 3 :
			switch (t) {
				case 0 : return "GridVisible";
			} break;
		} // switch
		return INVALID_KEYWORD_INDEX_STRING;
	}

	/*! Returns all keywords including deprecated for a given category (typenum) and type number (t). */
	const char * allKeywords(int typenum, int t) {
		switch (typenum) {
			// NetworkFluid::para_t
			case 0 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// PlaneGeometry::type_t
			case 1 :
			switch (t) {
				case 0 : return "Triangle";
				case 1 : return "Rectangle";
				case 2 : return "Polygon";
			} break;
			// Project::ViewFlags
			case 2 :
			switch (t) {
				case 0 : return "All";
			} break;
			// ViewSettings::Flags
			case 3 :
			switch (t) {
				case 0 : return "GridVisible";
			} break;
		} // switch
		return INVALID_KEYWORD_INDEX_STRING;
	}

	const char * KeywordList::Description(const char * const enumtype, int t, bool * no_description) {
		if (no_description != nullptr)
			*no_description = false; // we are optimistic
		switch (enum2index(enumtype)) {
			// NetworkFluid::para_t
			case 0 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
			} break;
			// PlaneGeometry::type_t
			case 1 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "Triangle";
				case 1 : if (no_description != nullptr) *no_description = true; return "Rectangle";
				case 2 : if (no_description != nullptr) *no_description = true; return "Polygon";
			} break;
			// Project::ViewFlags
			case 2 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "All";
			} break;
			// ViewSettings::Flags
			case 3 :
			switch (t) {
				case 0 : return "Grid is visible";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine description for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Description]");
	}

	const char * KeywordList::Unit(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// NetworkFluid::para_t
			case 0 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
			} break;
			// PlaneGeometry::type_t
			case 1 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Project::ViewFlags
			case 2 :
			switch (t) {
				case 0 : return "";
			} break;
			// ViewSettings::Flags
			case 3 :
			switch (t) {
				case 0 : return "";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default unit for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Unit]");
	}

	const char * KeywordList::Color(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// NetworkFluid::para_t
			case 0 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// PlaneGeometry::type_t
			case 1 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Project::ViewFlags
			case 2 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// ViewSettings::Flags
			case 3 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine color for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Color]");
	}

	double KeywordList::DefaultValue(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// NetworkFluid::para_t
			case 0 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// PlaneGeometry::type_t
			case 1 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Project::ViewFlags
			case 2 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ViewSettings::Flags
			case 3 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default value for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::DefaultValue]");
	}

	// number of entries in a keyword list
	unsigned int KeywordList::Count(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// NetworkFluid::para_t
			case 0 : return 3;
			// PlaneGeometry::type_t
			case 1 : return 3;
			// Project::ViewFlags
			case 2 : return 1;
			// ViewSettings::Flags
			case 3 : return 1;
		} // switch
		throw IBK::Exception(IBK::FormatString("Invalid enumeration type '%1'.")
			.arg(enumtype), "[KeywordList::Count]");
	}

	// max index for entries sharing a category in a keyword list
	int KeywordList::MaxIndex(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// NetworkFluid::para_t
			case 0 : return 2;
			// PlaneGeometry::type_t
			case 1 : return 2;
			// Project::ViewFlags
			case 2 : return 0;
			// ViewSettings::Flags
			case 3 : return 1;
		} // switch
		throw IBK::Exception(IBK::FormatString("Invalid enumeration type '%1'.")
			.arg(enumtype), "[KeywordList::MaxIndex]");
	}

	const char * KeywordList::Keyword(const char * const enumtype, int t) {
		const char * const kw = theKeyword(enum2index(enumtype), t);
		if (std::string(kw) == INVALID_KEYWORD_INDEX_STRING) {
			throw IBK::Exception(IBK::FormatString("Cannot determine keyword for enumeration type '%1' and index '%2'.")
				.arg(enumtype).arg(t), "[KeywordList::Keyword]");
		}
		return kw;
	}

	bool KeywordList::KeywordExists(const char * const enumtype, const std::string & kw) {
		int typenum = enum2index(enumtype);
		int i = 0;
		int maxIndexInCategory = MaxIndex( enumtype ); 
		for ( ; i <= maxIndexInCategory; ++i ) {
			std::string keywords = allKeywords(typenum, i);
			if (keywords == INVALID_KEYWORD_INDEX_STRING)
				continue;
			std::stringstream strm(keywords);
			int j = 0;
			std::string kws;
			while (strm >> kws) {
				if (kws == kw) {
					return true; // keyword exists
				}
				++j;
			}
		}
		return false; // nothing found keyword doesn't exist.
	}

	int KeywordList::Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated) {
		int typenum = enum2index(enumtype);
		int i = 0;
		int maxIndexInCategory = MaxIndex( enumtype ); 
		for ( ; i <= maxIndexInCategory; ++i ) {
			std::string keywords = allKeywords(typenum, i);
			if (keywords == INVALID_KEYWORD_INDEX_STRING)
				continue;
			std::stringstream strm(keywords);
			int j = 0;
			std::string kws;
			while (strm >> kws) {
				if (kws == kw) {
					if (deprecated != nullptr) {
						*deprecated = (j != 0);
					}
					return i;
				}
				++j;
			}
		} // for ( ; i < maxIndexInCategory; ++i ) {
		throw IBK::Exception(IBK::FormatString("Cannot determine enumeration value for "
			"enumeration type '%1' and keyword '%2'.")
			.arg(enumtype).arg(kw), "[KeywordList::Enumeration]");
	}

	bool KeywordList::CategoryExists(const char * const enumtype) {
		return enum2index(enumtype) != -1;
	}

} // namespace VICUS
