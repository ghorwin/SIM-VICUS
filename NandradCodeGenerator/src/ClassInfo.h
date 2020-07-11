#ifndef CLASSINFO_H
#define CLASSINFO_H

#include <vector>
#include <string>

#include <IBK_Path.h>

/*! Contains information parsed from a header file. */
class ClassInfo {
public:
	// Note: the Keyword-struct contains redundant data, for example the prefix in the category, which is
	//       the same for the entire class. This is mainly a convenience issue, so that all keywords from all
	//       files can be written together.
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
		std::string		enumNUM;
		unsigned int	count;
	};

	struct XMLInfo {
		bool			element;	// if false, an attribute is written
		bool			notEmpty;	// if true, string arguments are checked for emptiness after reading
		std::string		typeStr;	// C++ type string
	};


	/*! This function parses the given header file and extracts all data needed by the code generator. */
	bool parse(const IBK::Path & headerFilePath);

	IBK::Path				m_sourceHeaderFile;

	/*! Keywords collected from this class file. */
	std::vector<Keyword>	m_keywords;
	std::vector<EnumInfo>	m_enumInfo;
	std::vector<XMLInfo>	m_xmlInfo;

private:
	/*! Takes the string kwpara, searches for given delimiters (2) at end of string, and if presents takes
		the string between the delimiters and returns the string in as token. Also trimmes the kwpara string to exclude
		the extracted string and delimiters.
		If delimiters are not found, kwpara remains unmodified.
	*/
	void extractToken(std::string & kwpara, char delimiter, std::string & token);
};


#endif // CLASSINFO_H
