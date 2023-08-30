/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_configuration.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <ctime>

#include "IBK_Time.h"
#include "IBK_StringUtils.h"
#include "IBK_messages.h"
#include "IBK_Exception.h"
#include "IBK_Constants.h"
#include "IBK_UnitList.h"
#include "IBK_Unit.h"
#include "IBK_assert.h"

namespace IBK {



Time::Time() :
	m_year(INVALID_YEAR),
	m_sec(0)
{
}
// ---------------------------------------------------------------------------


Time::Time(int year, double sec) :
	m_year(year), m_sec(sec)
{
	while (m_sec < 0) {
		m_sec += SECONDS_PER_YEAR;
		--m_year;
	}
}
// ---------------------------------------------------------------------------


Time::Time(unsigned int hour, unsigned int minute, unsigned int sec)  {
	try {
		set(hour, minute, sec);
	}
	catch (std::exception & ) {
		m_sec = 0;
		m_year = INVALID_YEAR;
	}
}
// ---------------------------------------------------------------------------


Time::Time(int year, unsigned int month, unsigned int day, double seconds) {
	try {
		set(year, month, day, seconds);
	}
	catch (std::exception & ) {
		m_sec = 0;
		m_year = INVALID_YEAR;
	}
}
// ---------------------------------------------------------------------------


void Time::set(double seconds) {
	if (seconds < 0)
		throw IBK::Exception("Seconds cannot be negative.", "[Time::set(double seconds)]");
	m_year = 0;
	m_sec = seconds;
}
// ---------------------------------------------------------------------------


void Time::set(int year, double sec) {
	if (sec < 0)
		throw IBK::Exception("Seconds cannot be negative.", "[Time::set(int year, double sec)]");
	m_year = year;
	m_sec = sec;
}
// ---------------------------------------------------------------------------


void Time::set(unsigned int hour, unsigned int minute, unsigned int sec) {
	if (hour>23)    {
		m_year = INVALID_YEAR;
		return;
	}
	if (minute>59)    {
		m_year = INVALID_YEAR;
		return;
	}
	if (sec>59)    {
		m_year = INVALID_YEAR;
		return;
	}
	m_year = 0;
	m_sec = hour*3600 + minute*60 + sec;
}
// ---------------------------------------------------------------------------


void Time::set(int year, unsigned int month, unsigned int day, double sec) {
	FUNCID(Time::set);
	m_year = year;
	if (month > 11) {
		throw IBK::Exception("Invalid month", FUNC_ID);
	}
	if (day > 31) {
		throw IBK::Exception("Invalid day", FUNC_ID);
	}
	if (sec >= SECONDS_PER_DAY) {
		throw IBK::Exception("Invalid seconds", FUNC_ID);
	}
	m_sec = sec;
	for (unsigned int i=0; i<month; ++i) {
		m_sec += SECONDS_PER_MONTH[i];
	}
	m_sec += day*SECONDS_PER_DAY;
}
// ---------------------------------------------------------------------------


bool Time::isValid() const {
	return m_year != INVALID_YEAR;
}
// ---------------------------------------------------------------------------


Time& Time::operator+=(double secs) {
	m_sec += secs;
	while (m_sec < 0) {
		m_sec += SECONDS_PER_YEAR;
		--m_year;
	}
	return *this;
}
// ---------------------------------------------------------------------------


void Time::decomposeDate(int& year, unsigned int& month,
						unsigned int& day, double& seconds) const
{
	year = m_year;
	double s = m_sec;
	while (s >= static_cast<double>(SECONDS_PER_YEAR) ) {
		s -= SECONDS_PER_YEAR;
		++year;
	}
	month=0;
	while (s>=SECONDS_PER_MONTH[month])
		s -= SECONDS_PER_MONTH[month++];
	day = static_cast<unsigned int>(s)/SECONDS_PER_DAY;
	seconds = s - day*SECONDS_PER_DAY;
}
// ---------------------------------------------------------------------------


void Time::decomposeTOY(unsigned int& day, unsigned int & hour, unsigned int & minutes, unsigned int& seconds) const {
	seconds = static_cast<unsigned int>(m_sec);
	day = seconds/SECONDS_PER_DAY;
	seconds -= day*SECONDS_PER_DAY;

	hour = seconds / 3600;
	seconds -= hour*3600;
	minutes = seconds / 60;
	seconds = seconds % 60;
}
// ---------------------------------------------------------------------------


double Time::secondsUntil(const Time& other) const {
	return other.m_sec - m_sec + (other.m_year - m_year)*double(SECONDS_PER_YEAR);
}
// ---------------------------------------------------------------------------


Time& Time::operator-=(const Time& rhs) {
	// convert this->m_year to rhs.m_year
	while (m_year < rhs.m_year) {
		m_sec -= SECONDS_PER_YEAR;
		++m_year;
	}
	while (m_year > rhs.m_year) {
		m_sec += SECONDS_PER_YEAR;
		--m_year;
	}
	// calculate difference in seconds
	double diff_sec = m_sec - rhs.m_sec;
	if (diff_sec < 0) {
		m_sec = 0;
		m_year = INVALID_YEAR;
	}
	else {
		m_year = 0;
		m_sec = diff_sec;
		while (m_sec > SECONDS_PER_YEAR) {
			++m_year;
			m_sec -= SECONDS_PER_YEAR;
		}
	}
	return *this;
}
// ---------------------------------------------------------------------------
unsigned int Time::hour() const {

	unsigned int sec = static_cast<unsigned int>(m_sec);
	sec = sec % SECONDS_PER_DAY;
	unsigned int h = sec / 3600;

	return h;
}

unsigned int Time::minute() const {

	unsigned int sec = static_cast<unsigned int>(m_sec);
	sec = sec % SECONDS_PER_DAY;
	unsigned int h = sec / 3600;
	unsigned int m = sec / 60 - h*60;

	return m;
}

unsigned int Time::seconds() const {

	unsigned int sec = static_cast<unsigned int>(m_sec);
	sec = sec % SECONDS_PER_DAY;
	unsigned int s = sec % 60;

	return s;
}


std::string Time::toHourFormat() const {
	unsigned int sec = static_cast<unsigned int>(m_sec);
	sec = sec % SECONDS_PER_DAY;
	unsigned int h = sec / 3600;
	unsigned int m = sec / 60 - h*60;
	unsigned int s = sec % 60;
	char str[9];
	str[0] = (h >= 10) ? static_cast<char>(h / 10 + '0') : ' ';
	str[1] = static_cast<char>(h % 10 + '0');
	str[2] = ':';
	str[3] = static_cast<char>(m / 10 + '0');
	str[4] = static_cast<char>(m % 10 + '0');
	str[5] = ':';
	str[6] = static_cast<char>(s / 10 + '0');
	str[7] = static_cast<char>(s % 10 + '0');
	str[8] = 0;
	return std::string(str);
}
// ---------------------------------------------------------------------------


std::string  Time::toFullDateFormat() const {
	int y;
	unsigned int m, d;
	double s;
	decomposeDate(y, m, d, s);
	IBK::Time t(0, s);
	std::stringstream strm;
	strm << std::setw(2) << d+1 << " " << MONTH_NAMES[m] << " " << y << "  " << t.toHourFormat();
	return strm.str();
}
// ---------------------------------------------------------------------------


std::string Time::toShortDateFormat() const {
	int y;
	unsigned int m, d;
	double s;
	decomposeDate(y, m, d, s);
	IBK::Time t(0, s);
	std::stringstream strm;
	y %= 100;
	strm.fill('0');
	strm << std::setw(2) << std::right << d+1 << "."
		 << std::setw(2) << std::right << m+1 << "."
		 << std::setw(2) << std::right << y << " " << t.toHourFormat();
	return strm.str();
}
// ---------------------------------------------------------------------------


std::string Time::toShortDateFormatUS() const {
	int y;
	unsigned int m, d;
	double s;
	decomposeDate(y, m, d, s);
	IBK::Time t(0, s);
	std::stringstream strm;
	y %= 100;
	strm.fill('0');
	strm << std::setw(2) << std::right << m+1 << "/"
		 << std::setw(2) << std::right << d+1 << "/"
		 << std::setw(2) << std::right << y << " " << t.toHourFormat();
	return strm.str();
}
// ---------------------------------------------------------------------------


std::string Time::toDateTimeFormat() const {
	int y;
	unsigned int m, d;
	double s;
	decomposeDate(y, m, d, s);
	IBK::Time t(0, s);
	std::stringstream strm;
	strm.fill('0');
	strm << std::setw(2) << std::right << d+1 << "."
		 << std::setw(2) << std::right << m+1 << "."
		 << std::setw(4) << std::right << y << " "
		 << t.toHourFormat();
	return strm.str();
}
// ---------------------------------------------------------------------------


std::string Time::toDayMonthFormat() const {
	int y;
	unsigned int m, d;
	double s;
	decomposeDate(y, m, d, s);
	IBK::Time t(0, s);
	std::stringstream strm;
	strm.fill('0');
	strm << std::setw(2) << std::right << d+1 << "."
		 << std::setw(2) << std::right << m+1 << ".";
	return strm.str();
}
// ---------------------------------------------------------------------------


std::string Time::toTOYFormat(TOYFormat format) const {
	std::stringstream strm;
	unsigned int tmp;
	unsigned int t = static_cast<unsigned int>(m_sec);
	switch (format) {
		case TOY_LONG :
		{
			tmp = t / SECONDS_PER_DAY;
			if (tmp > 0) {
				strm << std::setw(4) << std::right << tmp << " day" << (tmp==1 ? " " : "s");
				t %= SECONDS_PER_DAY;
			}
			tmp = t / 3600;
			if (tmp > 0) {
				if (!strm.str().empty())  strm << ", ";
				strm << std::setw(2) << std::right << tmp << " hour" << (tmp==1 ? " " : "s");
				t %= 3600;
			}
			tmp = t / 60;
			if (tmp > 0) {
				if (!strm.str().empty())  strm << ", ";
				strm << std::setw(2) << std::right << tmp << " minute" << (tmp==1 ? " " : "s");
				t %= 60;
			}
			if (t > 0) {
				if (!strm.str().empty())  strm << " and ";
				strm << std::setw(2) << std::right << t << " second" << (t==1 ? " " : "s");
			}
			if (static_cast<unsigned int>(m_sec) == 0 && m_sec > 0) {
				unsigned int t = static_cast<unsigned int>(m_sec*1000);
				strm << std::setw(3) << std::right << t << " milli-second" << (t==1 ? " " : "s");
			}
		} break;

		case TOY_LONG_WITH_YEAR :
		{
			tmp = t / SECONDS_PER_YEAR;
			if (tmp > 0) {
				strm << std::setw(4) << std::right << tmp << " year" << (tmp==1 ? " " : "s");
				t %= SECONDS_PER_YEAR;
			}
			tmp = t / SECONDS_PER_DAY;
			if (tmp > 0) {
				strm << std::setw(4) << std::right << tmp << " day" << (tmp==1 ? " " : "s");
				t %= SECONDS_PER_DAY;
			}
			tmp = t / 3600;
			if (tmp > 0) {
				if (!strm.str().empty())  strm << ", ";
				strm << std::setw(2) << std::right << tmp << " hour" << (tmp==1 ? " " : "s");
				t %= 3600;
			}
			tmp = t / 60;
			if (tmp > 0) {
				if (!strm.str().empty())  strm << ", ";
				strm << std::setw(2) << std::right << tmp << " minute" << (tmp==1 ? " " : "s");
				t %= 60;
			}
			if (t > 0) {
				if (!strm.str().empty())  strm << " and ";
				strm << std::setw(2) << std::right << t << " second" << (t==1 ? " " : "s");
			}
		} break;

		case TOY_SHORT :
		{
			tmp = t / SECONDS_PER_DAY;
			if (tmp > 0) {
				strm << std::setw(4) << std::right << tmp << " d";
				t %= SECONDS_PER_DAY;
			}
			tmp = t / 3600;
			if (tmp > 0) {
				if (!strm.str().empty())  strm << "  ";
				strm << std::setw(2) << std::right << tmp << " h";
				t %= 3600;
			}
			tmp = t / 60;
			if (tmp > 0) {
				if (!strm.str().empty())  strm << "  ";
				strm << std::setw(2) << std::right << tmp << " min";
				t %= 60;
			}
			if (t > 0) {
				if (!strm.str().empty())  strm << "  ";
				strm << std::setw(2) << std::right << t << " s";
			}
		} break;

		case TOY_SHORT_WITH_YEAR : {
			tmp = t / SECONDS_PER_YEAR;
			if (tmp > 0) {
				strm << std::setw(4) << std::right << tmp << " a";
				t %= SECONDS_PER_YEAR;
			}
			tmp = t / SECONDS_PER_DAY;
			if (tmp > 0) {
				strm << std::setw(4) << std::right << tmp << " d";
				t %= SECONDS_PER_DAY;
			}
			tmp = t / 3600;
			if (tmp > 0) {
				if (!strm.str().empty())  strm << "  ";
				strm << std::setw(2) << std::right << tmp << " h";
				t %= 3600;
			}
			tmp = t / 60;
			if (tmp > 0) {
				if (!strm.str().empty())  strm << "  ";
				strm << std::setw(2) << std::right << tmp << " min";
				t %= 60;
			}
			if (t > 0) {
				if (!strm.str().empty())  strm << "  ";
				strm << std::setw(2) << std::right << t << " s";
			}
		} break;

		case TOY_VERY_SHORT :
		{
			tmp = t / SECONDS_PER_DAY;
			strm << std::setw(3) << std::right << tmp << " d ";
			t %= SECONDS_PER_DAY;

			tmp = t / 3600;
			strm.fill('0');
			strm << std::setw(2) << std::right << tmp << ":";
			t %= 3600;

			tmp = t / 60;

			strm << std::setw(2) << std::right << tmp << ":";
			t %= 60;

			strm << std::setw(2) << std::right << t;
		} break;

		default :
			throw IBK::Exception("Unsupported format flag.", "[Time::toTOYFormat]");
	}
	return strm.str();
}
// ---------------------------------------------------------------------------


IBK::Time Time::fromFullDateFormat(const std::string & formatted_time) {
	// parse full date format
	std::string str = formatted_time;
	std::replace(str.begin(), str.end(), ':', ' ');
	std::stringstream strm(str);

	int day;
	std::string month_name;
	int year;
	int hour, min, sec;
	if (strm >> day >> month_name >> year >> hour >> min >> sec) {
		--day;
		int month = 0;
		for (; month<12; ++month) {
			if (IBK::string_nocase_compare(IBK::MONTH_NAMES[month], month_name))
				break;
		}
		if (month == 12)
			return IBK::Time();
		return IBK::Time(year, month, day, hour*3600 + min*60 + sec);
	}
	return IBK::Time();
}
// ---------------------------------------------------------------------------


IBK::Time Time::fromDateTimeFormat(const std::string & formatted_time) {
	// parse full date format
	std::string str = formatted_time;
	std::replace(str.begin(), str.end(), ':', ' ');
	std::replace(str.begin(), str.end(), '.', ' ');
	std::stringstream strm(str);

	int day;
	int month;
	int year;
	int hour, min, sec;
	if (strm >> day >> month >> year >> hour >> min >> sec) {
		--day;
		-- month;
		return IBK::Time(year, month, day, hour*3600 + min*60 + sec);
	}
	return IBK::Time();
}
// ---------------------------------------------------------------------------


IBK::Time Time::fromDateTimeFormat2(const std::string & formatted_time) {
	// parse full date format
	std::string str = formatted_time;
	std::replace(str.begin(), str.end(), ':', ' ');
	std::replace(str.begin(), str.end(), '-', ' ');
	std::stringstream strm(str);

	int day;
	int month;
	int year;
	int hour, min, sec;
	if (strm >> year >> month >> day >> hour >> min >> sec) {
		--day;
		--month;
		return IBK::Time(year, month, day, hour*3600 + min*60 + sec);
	}
	return IBK::Time();
}
// ---------------------------------------------------------------------------


IBK::Time Time::fromTOY(const std::string & formatted_time, TOYFormat format) {
	std::string str = formatted_time;
	std::replace(str.begin(), str.end(), ':', ' ');
	// now put spaces between numbers and units
	std::string separated_string;
	for (unsigned int i = 0; i<str.size(); ++i) {
		const char * const substring = str.c_str() + i;
		if (substring[0] == 'a' ||
			substring[0] == 'd' ||
			substring[0] == 'h' ||
			(str.size() - i > 2 && substring[0] == 'm' && substring[1] == 'i' && substring[2] == 'n') ||
			substring[0] == 's')
		{
			// previous character was not a whitespace?
			if (!separated_string.empty() && separated_string[separated_string.size()-1] != ' ') {
				separated_string.push_back(' '); // insert space
			}
		}
		if (substring[0] == '\t')
			str[i] = ' ';
		separated_string.push_back(substring[0]);
	}
	std::stringstream strm(separated_string);
	switch (format) {

		// 'xxxx d  xx h  xx min  xx s'
		case TOY_SHORT :
		case TOY_SHORT_WITH_YEAR :
		{
			try {
				double value;
				std::string ustr;
				double sec = 0;
				while (strm >> value >> ustr) {
					IBK::UnitList::instance().convert(IBK::Unit(ustr), IBK::Unit("s"), value);
					sec += value;
				}
				return IBK::Time(0, sec);
			}
			catch (IBK::Exception & ) {
				return IBK::Time();
			}
		}
//		break;

		// ' 12 d 14:32:40'
		case TOY_VERY_SHORT :
		{
			unsigned int d;
			unsigned int h;
			unsigned int min;
			unsigned int s;
			if (strm >> d >> h >> min >> s) {
				return IBK::Time(0, d*SECONDS_PER_DAY + h*3600 + min*60 + s);
			}
			else {
				return IBK::Time();
			}
		}
//		break;

		default:
			throw IBK::Exception("TOY format not implemented.", "[Time::fromTOY]");
	};
//	return IBK::Time();
}
// ---------------------------------------------------------------------------

Time::TimeFormatInfo Time::formatInfo(const std::string& format) {
	Time::TimeFormatInfo res;
	const char* strOrg = format.c_str();
	const char* str = format.c_str();
	while(*str != '\0') {
		if(*str == 'y' && res.m_year4Start == -1 && res.m_year2Start == -1) {
			int start = (int)(str - strOrg);
			int i=1;
			while(*(++str) == 'y') ++i;
			if(i == 4)
				res.m_year4Start = start;
			if(i == 2)
				res.m_year2Start = start;
		}
		if(*str == 'M' && res.m_monthStart == -1) {
			int start = (int)(str - strOrg);
			int i=1;
			while(*(++str) == 'M') ++i;
			if(i == 2)
				res.m_monthStart = start;
		}
		if(*str == 'd' && res.m_dayStart == -1) {
			int start = (int)(str - strOrg);
			int i=1;
			while(*(++str) == 'd') ++i;
			if(i == 2)
				res.m_dayStart = start;
		}
		if(*str == 'h' && res.m_hourStart == -1) {
			int start = (int)(str - strOrg);
			int i=1;
			while(*(++str) == 'h') ++i;
			if(i == 2)
				res.m_hourStart = start;
		}
		if(*str == 'm' && res.m_minuteStart == -1) {
			int start = (int)(str - strOrg);
			int i=1;
			while(*(++str) == 'm') ++i;
			if(i == 2)
				res.m_minuteStart = start;
		}
		if(*str == 's' && res.m_secondStart == -1) {
			int start = (int)(str - strOrg);
			int i=1;
			while(*(++str) == 's') ++i;
			if(i == 2)
				res.m_secondStart = start;
		}

		++str;
	}
	if(res.m_year4Start > -1 && res.m_year2Start > -1)
		return Time::TimeFormatInfo();
	return res;
}

Time Time::fromString(const std::string & formatted_time, const TimeFormatInfo& formatInfo, bool useLeapYear) {
	if(!formatInfo.valid())
		return Time();

	bool fillZero = false;
	if(formatInfo.textLength() > formatted_time.size()) {
		// handling format mismatch
		if(formatInfo.isSingle()) {
			if(formatInfo.m_year4Start > -1) {
				return Time();
			}
			else {
				fillZero = true;
			}
		}
		else {
			return Time();
		}
	}


	Time result;

	const char* str = formatted_time.c_str();
//	while(isspace(*str)) ++str;
	const char* strStart = str;
	if(formatInfo.m_year4Start > -1) {
		str += formatInfo.m_year4Start;
		char ch1 = *str;
		char ch2 = *(str+1);
		char ch3 = *(str+2);
		char ch4 = *(str+3);
		if(!isdigit(ch1) || !isdigit(ch2) || !isdigit(ch3) || !isdigit(ch4))
			return Time();
		result.m_year = (ch1 - 48) * 1000 + (ch2 - 48) * 100 + (ch3 - 48) * 10 + (ch4 - 48);
	}
	str = strStart;
	if(formatInfo.m_year2Start > -1) {
		str += formatInfo.m_year2Start;
		char ch1 = fillZero ? '0' : *(str);
		char ch2 = fillZero ? *(str) : *(str+1);
		if(!isdigit(ch1) || !isdigit(ch2))
			return Time();

		result.m_year = 2000 + (ch1 - 48) * 10 + (ch2 - 48);
	}
	str = strStart;

	if(formatInfo.m_monthStart > -1) {
		str += formatInfo.m_monthStart;
		char ch1 = fillZero ? '0' : *(str);
		char ch2 = fillZero ? *(str) : *(str+1);
		if(!isdigit(ch1) || !isdigit(ch2))
			return Time();

		int month = (ch1 - 48) * 10 + (ch2 - 48);
		if(month > 1) {
			result.m_sec += SECONDS_UNTIL_MONTH[month-1];
			if(month > 2 && useLeapYear)
				result.m_sec += 86400;
		}
	}
	str = strStart;

	if(formatInfo.m_dayStart > -1) {
		str += formatInfo.m_dayStart;
		char ch1 = fillZero ? '0' : *(str);
		char ch2 = fillZero ? *(str) : *(str+1);
		if(!isdigit(ch1) || !isdigit(ch2))
			return Time();

		int day = (ch1 - 48) * 10 + (ch2 - 48) - 1;
		result.m_sec += day * 86400;
	}
	str = strStart;

	if(formatInfo.m_hourStart > -1) {
		str += formatInfo.m_hourStart;
		char ch1 = fillZero ? '0' : *(str);
		char ch2 = fillZero ? *(str) : *(str+1);
		if(!isdigit(ch1) || !isdigit(ch2))
			return Time();

		int hour = (ch1 - 48) * 10 + (ch2 - 48);
		result.m_sec += hour * 3600;
	}
	str = strStart;

	if(formatInfo.m_minuteStart > -1) {
		str += formatInfo.m_minuteStart;
		char ch1 = fillZero ? '0' : *(str);
		char ch2 = fillZero ? *(str) : *(str+1);
		if(!isdigit(ch1) || !isdigit(ch2))
			return Time();

		int minute = (ch1 - 48) * 10 + (ch2 - 48);
		result.m_sec += minute * 60;
	}
	str = strStart;

	if(formatInfo.m_secondStart > -1) {
		str += formatInfo.m_secondStart;
		char ch1 = fillZero ? '0' : *(str);
		char ch2 = fillZero ? *(str) : *(str+1);
		if(!isdigit(ch1) || !isdigit(ch2))
			return Time();

		int seconds = (ch1 - 48) * 10 + (ch2 - 48);
		result.m_sec += seconds;
	}
	str = strStart;

	return result;
}


// *** STATIC FUNCTIONS ***


std::string Time::format_time_difference(double delta_t) {
	std::stringstream strm;
	strm << std::fixed << std::setprecision(3) << std::setw(6) << std::right;
	if (delta_t<1)
		strm << std::setprecision(0) << delta_t*1000 << " ms";
	else if (delta_t<60)
		strm << delta_t << " s";
	else if (delta_t<3600)
		strm << delta_t/60.0 << " min";
	else if (delta_t<86400)
		strm << delta_t/3600.0 << " h";
	else if (delta_t<86400*365.0)
		strm << delta_t/86400.0 << " d";
	else
		strm << delta_t/(86400.0*365) << " a";
	return strm.str();
}
// ---------------------------------------------------------------------------


std::string Time::format_time_difference(double delta_t, const std::string & ustr, bool uniformUnitLength) {

	/// \todo implement usage for uniformUnitLength
	(void) uniformUnitLength;
	std::stringstream strm;
	strm << std::fixed << std::setprecision(3) << std::setw(7) << std::right; // 150 s -> 150.000 = 7 digits
	UnitList::instance().convert(Unit("s"), Unit(ustr), delta_t);
	if (ustr == "ms")
		strm << std::setprecision(0);
	strm << delta_t << " " << ustr;
	if (ustr.size() < 3)
		strm << std::string(4-ustr.size(), ' ');
	return strm.str();
}
// ---------------------------------------------------------------------------


const char * Time::suitableTimeUnit(double sec) {
	if (sec > 86400*180)
		return "a";
	if (sec > 0.5*86400)
		return "d";
	if (sec > 1800)
		return "h";
	if (sec > 180)
		return "min";
	if (sec > 1)
		return "s";
	return "ms";
}
// ---------------------------------------------------------------------------


const char * Time::suitableSpeedUnit(double sec) {
	if (sec > 86400*180)
		return "a/s";
	if (sec > 0.5*86400)
		return "d/s";
	if (sec > 1800)
		return "h/s";
	if (sec > 180)
		return "min/s";
	return "s/s";
}
// ---------------------------------------------------------------------------


std::string Time::dateRFC2822(const std::string& timeFormat ) {
	FUNCID(Time::dateRFC2822);

	// A format suitable for Internet RFC 2822 as standard or other formats.
	const char * const rfc_2822_format = "%a, %d %b %Y %H:%M:%S %z";
	char buffer [80];

	std::time_t rawtime;
	struct std::tm * timeinfo;

	std::time ( &rawtime );
	timeinfo = std::localtime ( &rawtime );

	if ( !timeinfo )
		throw IBK::Exception( "Time is out of range, error calling std::localtime().", FUNC_ID);

	#define FPRINTFTIME 1
	size_t length;
	if( timeFormat.empty()) {
		length = std::strftime ( buffer, 80, rfc_2822_format, timeinfo );
	}
	else {
		length = std::strftime ( buffer, 80, timeFormat.c_str(), timeinfo );
	}
	// if an error in strftime is occured return an empty string
	if( length == 0)
		return "";
	else
		return std::string(buffer);
}

/*

Time& Time::operator-=(unsigned int secs) {
	while (sec_ < secs)
		sec_ += SECS_PER_DAY;
	sec_ -= secs;
	return *this;
}
// ---------------------------------------------------------------------------

void Time::set(const std::string& timestr) {
	std::string str = timestr;
	for (std::string::iterator it=str.begin(); it!=str.end(); ++it)
		if (*it==':')    *it=' ';
	std::stringstream strm(str);
	unsigned int hour, min, sec;
	if (!(strm >> hour >> min))
		throw IBK::Exception("Invalid time string '" + timestr + "'!","[Time::set]");
	if (!(strm >> sec))
		sec = 0;
	set(hour, min, sec);
}
// ---------------------------------------------------------------------------





std::ostream& operator<<(std::ostream& out, const Time& t) {
	return out << t.toString();
}
// ---------------------------------------------------------------------------

std::istream& operator>>(std::istream& in, Time& t) {
	std::string tmp;
	if (in >> tmp) {
		try { t.set(tmp); }
		catch (...) { in.setstate(std::ios_base::badbit); }
	}
	return in;
}
// ---------------------------------------------------------------------------

void Time::set(int year, unsigned int month, unsigned int day, unsigned int seconds) {
	if (month>11)
		throw IBK::Exception("Month out of range!","[Time::set]");
	for (unsigned int i=0; i<month; ++i)
		seconds += SECONDS_PER_MONTH[i];
	// TODO : check that 'day' is valid
	seconds += day*86400;
	m_sec = seconds;
	m_year = year;
}


const Time& Time::operator+=(double s) {
	m_sec+=s;
	return *this;
}

Time Time::operator+(double s) const {
	Time newTime(*this);
	return newTime+=(s);
}


*/

TimeFormat::TimeFormat(const std::string& format) :
	m_formatString(format),
	m_dayStartZero(false),
	m_hourStartZero(true),
	m_minuteStartZero(true),
	m_secondStartZero(true),
	m_valid(true)
{
	if(!m_formatString.empty()) {
		const char* str = m_formatString.c_str();
		bool has4DigitsYear = false;
		bool has2DigitsYear = false;
		bool has2DigitsMonth = false;
		bool has2DigitsDay = false;
		bool has3DigitsDay = false;
		bool has2DigitsHour = false;
		bool has2DigitsMinute = false;
		bool has2DigitsSecond = false;
		while(*str != '\0') {
			if(*str == 'y') {
				if(has4DigitsYear || has2DigitsYear) {
					m_valid = false;
					m_error = "More than one year string";
					break;
				}
				int i=1;
				while(*(++str) == 'y') ++i;
				if(i == 4) {
					m_formatVect.push_back({FE_4DigitsYear,std::string()});
					has4DigitsYear = true;
				}
				else if(i == 2) {
					m_formatVect.push_back({FE_2DigitsYear,std::string()});
					has2DigitsYear = true;
				}
				else {
					m_valid = false;
					m_error = "Year string has neither 2 nor 4 characters.";
					break;
				}
			}
			else if(*str == 'M') {
				if(has2DigitsMonth) {
					m_valid = false;
					m_error = "More than one month string";
					break;
				}
				int i=1;
				while(*(++str) == 'M') ++i;
				if(i == 2) {
					m_formatVect.push_back({FE_2DigitsMonth,std::string()});
				}
				else {
					m_valid = false;
					m_error = "Month string has not two characters.";
					break;
				}
			}
			else if(*str == 'd') {
				if(has2DigitsDay || has3DigitsDay) {
					m_valid = false;
					m_error = "More than one day string";
					break;
				}
				int i=1;
				while(*(++str) == 'd') ++i;
				if(i == 3) {
					m_formatVect.push_back({FE_3DigitsDay,std::string()});
				}
				else if(i == 2) {
					m_formatVect.push_back({FE_2DigitsDay,std::string()});
				}
				else {
					m_valid = false;
					m_error = "Day string has neither 2 nor 3 characters.";
					break;
				}
			}
			else if(*str == 'h') {
				if(has2DigitsHour) {
					m_valid = false;
					m_error = "More than one hour string";
					break;
				}
				int i=1;
				while(*(++str) == 'h') ++i;
				if(i == 2) {
					m_formatVect.push_back({FE_2DigitsHour,std::string()});
				}
				else {
					m_valid = false;
					m_error = "Hour string has not two characters.";
					break;
				}
			}
			else if(*str == 'm') {
				if(has2DigitsMinute) {
					m_valid = false;
					m_error = "More than one minute string";
					break;
				}
				int i=1;
				while(*(++str) == 'm') ++i;
				if(i == 2) {
					m_formatVect.push_back({FE_2DigitsMinute,std::string()});
				}
				else {
					m_valid = false;
					m_error = "Minute string has not two characters.";
					break;
				}
			}
			else if(*str == 's') {
				if(has2DigitsSecond) {
					m_valid = false;
					m_error = "More than one second string";
					break;
				}
				int i=1;
				while(*(++str) == 's') ++i;
				if(i == 2) {
					m_formatVect.push_back({FE_2DigitsSecond,std::string()});
				}
				else {
					m_valid = false;
					m_error = "Second string has not two characters.";
					break;
				}
			}
			else {
				if(*str != '\0') {
					std::string div;
					while(*str != '\0' && *str != 'y' && *str != 'M' && *str != 'd' && *str != 'h' && *str != 'm' && *str != 's') {
						div.push_back(*str);
						++str;
					}
					if(!div.empty())
						m_formatVect.push_back({FE_Divider,div});
				}
			}
		}
		if(m_formatVect.empty()) {
			m_valid = false;
		}
		else if(m_formatVect.size() == 1 && m_formatVect.back().first == FE_Divider) {
				m_valid = false;
		}
		else {
			bool lastWasDivider = m_formatVect[0].first == FE_Divider;
			for(size_t i=1; i<m_formatVect.size(); ++i) {
				if(lastWasDivider && m_formatVect[i].first == FE_Divider) {
					m_valid = false;
					break;
				}
				if(!lastWasDivider && m_formatVect[i].first != FE_Divider) {
					m_valid = false;
					break;
				}
				lastWasDivider = m_formatVect[i].first == FE_Divider;
			}
		}
		if(!m_valid)
			m_formatVect.clear();

		if(m_formatVect.size() == 2) {
			if(m_formatVect.front().first == FE_Divider)
				m_formatVect.erase(m_formatVect.begin());
			else
				m_formatVect.pop_back();
		}
	}
	else {
		m_error = "Format string is empty";
		m_valid = false;
	}
}

static double secondsTilMonth(int month, bool useLeapYear) {
	if(month > 1) {
		double sec = SECONDS_UNTIL_MONTH[month-1];
		if(month > 2 && useLeapYear)
			sec += 86400;
		return sec;
	}
	else {
		return 0;
	}
}

static int valueFromSingleEntityString(const char* str, int length, TimeFormat::FormatEntity type, std::string& errstr) {
	unsigned int numDigits = 0;
	if(length <= 0) {
		errstr = "Time string length wrong.";
		return 0;
	}
	// check for length of string and number of digits in it
	for(unsigned int i=0; i<(unsigned int)length; ++i) {
		if(isdigit(str[i]))
			++numDigits;
		// a single entity should have no characters between digits
		if(numDigits > 0 && !isdigit(str[i])) {
			errstr = "digits are not continuously.";
			return 0;
		}
		if(str[i] == '\0') {
			errstr = "End of line inside time string";
			return 0;
		}
	}

	// no digits and more than 4 digits cannot be a valid entity
	if(numDigits == 0 || numDigits > 4)
		return 0;

	if(type == TimeFormat::FE_4DigitsYear) {
		if( length != 4 || numDigits != 4)
			return 0;

		int year = (str[0] - 48) * 1000 + (str[1] - 48) * 100 + (str[2] - 48) * 10 + (str[3] - 48);
		return year;
	}
	else if(type == TimeFormat::FE_3DigitsDay) {
		if( numDigits > 3)
			return 0;

		// skip leading characters
		while(!isdigit(*str) && *str != '\0') ++str;

		// 3 digit string
		if(numDigits == 3) {
			int val = (str[0] - 48) * 100 + (str[1] - 48) * 10 + str[2] - 48;
			return val;
		}
		// two digit string
		if(numDigits == 2) {
			int val = (str[0] - 48) * 10 + str[1] - 48;
			return val;
		}
		// one digit string
		int val = *str - 48;
		return val;
	}
	else {
		// here only one or two digits are allowed
		if(numDigits > 2)
			return 0;

		// skip leading characters
		while(!isdigit(*str) && *str != '\0') ++str;

		int val;
		// two digit string
		if(isdigit(str[1])) {
			val = (str[0] - 48) * 10 + str[1] - 48;
		}
		// one digit string
		else {
			val = *str - 48;
		}
		return val;
	}
}

double TimeFormat::valueToSeconds(int value, TimeFormat::FormatEntity type, bool useLeapYear) {
	switch(type) {
		case TimeFormat::FE_4DigitsYear:	return value  * 365.0 * 24.0 * 3600.0;
		case TimeFormat::FE_2DigitsYear:	return (value + 2000)  * 365.0 * 24.0 * 3600.0;
		case TimeFormat::FE_2DigitsMonth:	return secondsTilMonth(value, useLeapYear);
		case TimeFormat::FE_3DigitsDay:
		case TimeFormat::FE_2DigitsDay:		{
			if(value == 0 && !m_dayStartZero)
				m_dayStartZero = true;
			if(!m_dayStartZero)
				value--;
			return value  * 24.0 * 3600.0;
		}
		case TimeFormat::FE_2DigitsHour:	{
			if(!m_hourStartZero)
				value--;
			return value  * 3600.0;
		}
		case TimeFormat::FE_2DigitsMinute:	{
			if(!m_minuteStartZero)
				value--;
			return value * 60.0;
		}
		case TimeFormat::FE_2DigitsSecond:	{
			if(!m_secondStartZero)
				value--;
			return value;
		}
		default: return 0;
	}
}

double TimeFormat::secondsFromSingleEntityString(const char* str, int length, TimeFormat::FormatEntity type, std::string& errstr, bool useLeapYear) {
	int val = valueFromSingleEntityString(str, length, type, errstr);
	return valueToSeconds(val, type, useLeapYear);
}

double TimeFormat::seconds(const std::string& timeString, std::string& errstr, bool useLeapYear) {
	errstr.clear();
	if(!valid()) {
		errstr = "Time format string not valid";
		return 0;
	}

	const char* str = timeString.c_str();
	int length = (int)timeString.size();
	while(isspace(*str++)) --length;
	--str;

	IBK_ASSERT(m_formatVect.size() != 2);	// checked in constructor

	if(m_formatVect.size() == 1) {
		double val = secondsFromSingleEntityString(str, length, m_formatVect.front().first, errstr, useLeapYear);
		if(!errstr.empty())
			return 0;

		return val;
	}
	else {
		size_t iStart = 0;
		if(m_formatVect.front().first == FE_Divider) {
			while(!isdigit(*str++));
			++iStart;
		}
		const char* strStart = str;
		double val = 0;
		for(size_t i=iStart; i<m_formatVect.size(); i+=2) {
			// first must be an entity
			if( i < m_formatVect.size() - 1) {
				const std::string& div = m_formatVect[i+1].second;
				IBK_ASSERT(!div.empty());
				while(*str != div[0] && *str != '\0') ++str;
				if(*str == '\0') {
					errstr = "Divider not found";
					return 0;
				}
				val += secondsFromSingleEntityString(strStart, int(str-strStart), m_formatVect[i].first, errstr, useLeapYear);
				if(!errstr.empty())
					return 0;

				strStart = str + div.size();
				str = strStart;
			}
			else {
				while(*str != '\0') ++str;
				val += secondsFromSingleEntityString(strStart, int(str-strStart), m_formatVect[i].first, errstr, useLeapYear);
				if(!errstr.empty())
					return 0;
			}
		}
		return val;
	}
}

IBK::Time TimeFormat::time(const std::string& timeString, std::string& errstr, bool useLeapYear) {
	errstr.clear();
	if(!valid()) {
		errstr = "Time format string not valid";
		return IBK::Time();
	}


	const char* str = timeString.c_str();
	int length = (int)timeString.size();
	while(isspace(*str++)) --length;
	--str;

	IBK_ASSERT(m_formatVect.size() != 2);	// checked in constructor

	if(m_formatVect.size() == 1) {
		int val = valueFromSingleEntityString(str, length, m_formatVect.front().first, errstr);
		IBK::Time res;
		if(hasYear()) {
			if(m_formatVect.front().first == FE_2DigitsYear)
				val += 2000;
			res.set(val,0);
		}
		else {
			double sec = valueToSeconds(val, m_formatVect.front().first, useLeapYear);
			res.set(INVALID_YEAR, sec);
		}
		return res;
	}
	else {
		IBK::Time res;
		size_t i = 0;
		if(m_formatVect.front().first == FE_Divider) {
			while(!isdigit(*str++));
			++i;
		}
		const char* strStart = str;
		double val = 0;
		int year = INVALID_YEAR;
		for(; i<m_formatVect.size(); i+=2) {
			// first must be an entity
			if( i < m_formatVect.size() - 1) {
				const std::string& div = m_formatVect[i+1].second;
				IBK_ASSERT(!div.empty());
				while(*str != div[0] && *str != '\0') ++str;
				if(*str == '\0') {
					errstr = "Divider not found";
					return res;
				}
				if(m_formatVect[i].first == FE_2DigitsYear || m_formatVect[i].first == FE_4DigitsYear) {
					year = valueFromSingleEntityString(strStart, int(str-strStart), m_formatVect[i].first, errstr);
					if(!errstr.empty())
						return IBK::Time();

					if(m_formatVect[i].first == FE_2DigitsYear)
						year += 2000;
				}
				else {
					val += secondsFromSingleEntityString(strStart, int(str-strStart), m_formatVect[i].first, errstr, useLeapYear);
					if(!errstr.empty())
						return IBK::Time();
				}
				strStart = str + div.size();
				str = strStart;
			}
			else {
				while(*str != '\0') ++str;
				if(int(str-strStart) > length) {
					errstr = "End of string not found";
					return res;
				}
				if(m_formatVect[i].first == FE_2DigitsYear || m_formatVect[i].first == FE_4DigitsYear) {
					year = valueFromSingleEntityString(strStart, int(str-strStart), m_formatVect[i].first, errstr);
					if(!errstr.empty())
						return IBK::Time();
					if(m_formatVect[i].first == FE_2DigitsYear)
						year += 2000;
				}
				else {
					val += secondsFromSingleEntityString(strStart, int(str-strStart), m_formatVect[i].first, errstr, useLeapYear);
					if(!errstr.empty())
						return IBK::Time();
				}
				res.set(year, val);
				return res;
			}
		}
	}
	return IBK::Time();
}

bool TimeFormat::hasYear() const {
	for(size_t i=0; i<m_formatVect.size(); ++i) {
		if(m_formatVect[i].first == FE_2DigitsYear || m_formatVect[i].first == FE_4DigitsYear)
			return true;
	}
	return false;
}

bool TimeFormat::hasMonth() const {
	for(size_t i=0; i<m_formatVect.size(); ++i) {
		if(m_formatVect[i].first == FE_2DigitsMonth)
			return true;
	}
	return false;
}

bool TimeFormat::hasDay() const {
	for(size_t i=0; i<m_formatVect.size(); ++i) {
		if(m_formatVect[i].first == FE_2DigitsDay)
			return true;
	}
	return false;
}

} // namespace IBK
