#ifndef HelpersH
#define HelpersH

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

/*! Helpers for string manipulation and unit conversion.
*/


/*! Converts the value 'val' to a string. */
template <class T>
std::string val2string(const T val) {
	if (!(val == val)) { //-V501
		return "1.#QNAN";
	}
	std::stringstream strm;
	strm << val;
	return strm.str();
}


/*! Converts the boolean 'val' to a string. */
template <>
inline std::string val2string<bool>(const bool val) {
	return (val ? "true" : "false");
}


/*! Converts the value 'val' to a string with given precision. */
template <class T>
std::string val2string(const T val, const int precision) {
	if (!(val == val)) { //-V501
		return "1.#QNAN";
	}
	std::stringstream strm;
	strm.precision(precision);
	strm << val;
	return strm.str();
}

/*! Converts the value 'val' to a string using special format qualifiers (total string width and fill chars). */
template <class T>
std::string val2string(const T val, const std::size_t width, const char fillchar) {
	if (!(val == val)) { //-V501
		return "1.#QNAN";
	}
	std::stringstream strm;
	strm.fill(fillchar);
	strm << std::setw(width) << val;
	return strm.str();
}

/*! Converts the value 'val' to a string using special format qualifiers (precision, total string width and fill chars). */
template <class T>
std::string val2string(const T val, const int precision, const std::size_t width, const char fillchar) {
	if (!(val == val)) { //-V501
		return "1.#QNAN";
	}
	std::stringstream strm;
	strm.fill(fillchar);
	strm.precision(precision);
	strm << std::setw(width) << val;
	return strm.str();
}

/*! Attempts to extract a numerical value from a string.
\code
double val = string2val<double>("2.5");
\endcode
*/
template <class T>
T string2val(const std::string& str) {
	T val;
	std::stringstream strm(str);
	if (!(strm >> val))
		throw std::runtime_error((std::string("Could not convert " + str + std::string(" into value."))).c_str());
	return val;
}


template <>
double string2val<double>(const std::string& str);

template <>
bool string2val<bool>(const std::string& str);

/*! Attempts to extract a numerical value from a string.
Returns the def value in case of non valid string.
\code
double val = string2val<double>("2.5", 0.0);
\endcode
*/
template <class T>
T string2valDef(const std::string& str, const T& def) {
	std::stringstream strm(str);
	T val;
	if (!(strm >> val))
		return def;
	return val;
}

/*! Attempts to extract a numerical value from a string.
Returns the def value in case of non valid string.
\code
double val = string2val<double>("2.5", 0.0);
\endcode
*/
template <>
double string2valDef<double>(const std::string& str, const double & def);
/*! Explodes the string 'str' into substrings (stored in the string list
'tokens'), which were separated by 'delim' characters.
\param str Original string
\param tokens Vector with tokens
\param delims Separation characters
\param explodeFlags Combination of flags to control behaviour
\sa ExplodeFlags
*/
size_t explode(const std::string& str, std::vector<std::string>& tokens, const std::string& delims);

/*! Trims the string on both sides (removes tabs and spaces).
This is the inplace-version of the trim() function.
*/
void trim(std::string& str, const std::string& trimchars = " \t\r");

// windows specific conversion functions
#if defined(_WIN32)

/*! Converts a given UTF8 encoded string into a unicode string.*/
std::wstring UTF8ToWstring(const std::string& utf8str);

/*! Converts a given unicode string into a UTF8 encoded string.*/
std::string WstringToUTF8(const std::wstring& wide);

#endif

/*! Converts a value to SI base unit unit.
\param val Parameter value that should be converted
\param unit Parameter unit for value
*/
void convertToBaseUnit(double &val, std::string &baseUnit, std::string unit);



#endif // StringUtilsH
