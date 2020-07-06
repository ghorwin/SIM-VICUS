#ifndef CLASSINFO_H
#define CLASSINFO_H

#include <vector>
#include <string>
#include <IBK_Path.h>

/*! Contains information parsed from a header file. */
class ClassInfo {
public:
	struct Keyword {
		std::string		category;			// The enumeration type (full C++ type including class prefix)
		std::string		keyword;			// The keyword as string
		std::string		description;		// The english description text (optional)
		std::string		unit;				// The default unit (if any, otherwise an empty string)
		double			defaultValue;		// The default value in default unit if any
		int				index;				// The index/enumeration number
		int				maxIndexInCategory; // Category related maximum index
		std::string		color;				// Color assigned to this keyword; default color is black #FFFFFF
	};

	struct EnumInfo {
		std::string		categoryName;
		int				count;
	};

	struct XMLInfo {
		bool			element;	// if false, an attribute is written
		bool			notEmpty;	// if true, string arguments are checked for emptiness after reading
		std::string		typeStr;	// C++ type string
	};

	ClassInfo();


	void parse(const IBK::Path & filepath);

};

#endif // CLASSINFO_H
