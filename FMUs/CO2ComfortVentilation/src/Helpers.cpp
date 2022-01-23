#include <stdexcept>
#include <string>
#include <algorithm>

#ifdef _WIN32
#ifdef NOMINMAX
#include <windows.h>
#else
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#endif
#endif

#include "Helpers.h"

#ifdef _WIN32

  #ifndef _WIN64

	#define IBK_USE_STOD

  #else

	#include "fast_float/fast_float.h"

  #endif

#else

  #include "fast_float/fast_float.h"

#endif

template <>
double string2val<double>(const std::string& str) {
	double val;
	if (str=="1.#QNAN")
		return std::numeric_limits<double>::quiet_NaN();
#ifdef IBK_USE_STOD
	// for 32-bit, use std::stod()
	size_t pos;
	try {
		val = std::stod(str, &pos); // may throw std::out_of_range or std::invalid_argument
		if (str.find_first_not_of(" \t\n", pos) != std::string::npos)
			throw std::exception();
	}
	catch (...) {
		throw IBK::Exception(IBK::FormatString("Could not convert '%1' into value.").arg(str), "[IBK::string2val<double>]");
	}
#else
	auto answer = fast_float::from_chars(str.data(), str.data()+str.size(), val);
	if (answer.ec != std::errc())
		throw std::runtime_error((std::string("Could not convert " + str +
			std::string(" into value."))).c_str());
#endif
	return val;
}

template <>
bool string2val<bool>(const std::string & str) {
	// only allow 'true' and 'false'
	if (str == "true" || str == "1")
		return true;
	else if (str == "false" || str == "0")
		return false;
	throw std::runtime_error((std::string("Could not convert '") + str + std::string("' into bool.")).c_str());
}


template <>
double string2valDef<double>(const std::string& str, const double & def) {
	if (str=="1.#QNAN")
		return std::numeric_limits<double>::quiet_NaN();
	double val;
#ifdef IBK_USE_STOD
	// for 32-bit, use std::stod()
	size_t pos;
	if (std::locale().name() != "C")
		setlocale(LC_ALL, "C");
	try {
		val = std::stod(str, &pos); // may throw std::out_of_range or std::invalid_argument
		if (str.find_first_not_of(" \t\n", pos) != std::string::npos)
			throw std::exception();
	}
	catch (...) {
		throw std::runtime_error((std::string("Could not convert " + str +
			std::string(" into value."))).c_str());
	}
#else
	auto answer = fast_float::from_chars(str.data(), str.data()+str.size(), val);
	if (answer.ec != std::errc())
		return def;
#endif
	return val;
}


size_t explode(const std::string& str, std::vector<std::string>& tokens, const std::string& delims) {
	tokens.clear();
	std::string tmp;

	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		bool delim_found = false;
		for (std::string::const_iterator tit = delims.begin(); tit != delims.end(); ++tit) {
			if (*it == *tit) {
				if (!tmp.empty())
					tokens.push_back(tmp);
				tmp.clear();
				delim_found = true;
				break;
			}
		}
		if (!delim_found)
			tmp += *it;
	}
	if (!tmp.empty()) {
		tokens.push_back(tmp);
	}
	return tokens.size();
}


void trim(std::string& str, const std::string& trimchars) {
	size_t f = str.find_first_not_of(trimchars);
	size_t l = str.find_last_not_of(trimchars);
	if (f != std::string::npos && l != std::string::npos)
		str = str.substr(f, l - f + 1);
	else
		str.clear();
}


// windows specific conversion functions
#if defined(_WIN32)

std::wstring UTF8ToWstring(const std::string& utf8str) {
	if (utf8str.empty())
		return std::wstring();

	int reslength = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, 0, 0);
	if (reslength == 0)
		throw std::runtime_error("Cannot create wide string from UTF8 string.");

	std::vector<wchar_t> wide(reslength, L'\0');
	int writtenLength = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, &wide[0], reslength);
	if (writtenLength == 0)
		throw std::runtime_error("Cannot create wide string from UTF8 string.");

	return std::wstring(&wide[0]);
}

std::string WstringToUTF8(const std::wstring& wide) {
	if (wide.empty())
		return std::string();

	int reslength = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, 0, 0, 0, 0);
	if (reslength == 0)
		throw  std::runtime_error("Cannot create UTF8 string from wide string.");

	std::vector<char> str(reslength, '\0');
	int writtenLength = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &str[0], reslength, 0, 0);
	if (writtenLength == 0)
		throw std::runtime_error("Cannot create UTF8 string from wide string.");
	return std::string(&str[0]);
}


#endif

void convertToBaseUnit(double &val, std::string &baseUnit, std::string unit) {

	// time units
	if (unit == "d") {
		val *= 24.* 3600.;
		baseUnit = "s";
		return;
	}
	if (unit == "h") {
		val *= 3600.;
		baseUnit = "s";
		return;
	}
	if (unit == "min") {
		val *= 60.;
		baseUnit = "s";
		return;
	}
	if (unit == "s") {
		baseUnit = "s";
		return;
	}
	// temperature units
	if (unit == "C") {
		val += 273.15;
		baseUnit = "K";
		return;
	}
	if (unit == "K") {
		baseUnit = "K";
		return;
	}
	// volume units
	if (unit == "ppm") {
		val *= 1e-06;
		baseUnit = "mol/mol";
		return;
	}
	if (unit == "mol/mol") {
		baseUnit = "mol/mol";
		return;
	}
	// mass units
	if (unit == "g") {
		val /= 1000.;
		baseUnit = "kg";
		return;
	}
	if (unit == "kg") {
		baseUnit = "kg";
		return;
	}
	// mass density
	if (unit == "g/l") {
		val *= 1e-06;
		baseUnit = "kg/m3";
		return;
	}
	if (unit == "kg/m3") {
		baseUnit = "kg/m3";
		return;
	}
	// mass flux
	if (unit == "l/h") {
		val *= 1e-03/3600.;
		unit = "kg/s";
		return;
	}
	if (unit == "kg/s") {
		baseUnit = "kg/s";
		return;
	}
	// mass flux per area
	if (unit == "kg/m2s") {
		baseUnit = "kg/m2s";
		return;
	}
	// air change rate
	if (unit == "1/h") {
		val /= 3600.;
		baseUnit = "1/s";
		return;
	}
	if (unit == "1/s") {
		baseUnit = "1/s";
		return;
	}
	// area
	if (unit == "m2") {
		baseUnit = "m2";
		return;
	}
	// volume
	if (unit == "l") {
		val /= 1000.;
		baseUnit = "m3";
		return;
	}
	if (unit == "m3") {
		baseUnit = "m3";
		return;
	}

	throw std::runtime_error((std::string("Unknown unit '") + unit + std::string("'.")).c_str());
}

