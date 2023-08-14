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

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <clocale>

#include <sstream>

#include "IBK_UnitList.h"
#include "IBK_UnitData.h"
#include "IBK_Unit.h"
#include "IBK_StringUtils.h"
#include "IBK_messages.h"
#include "IBK_math.h"
#include "IBK_Exception.h"
#include "IBK_FormatString.h"
#include "IBK_Path.h"
#include "IBK_FileUtils.h"

namespace IBK {


/*! The default unit list as encoded string, used by IBK::UnitList::read_default(). */
extern const char * const DEFAULT_UNITS;

/*! List of deprecated units that have been replaced by others.
	Map is populated in constructor of UnitList().
*/
static std::map<std::string, std::string> DEPRECATED_LIST;

/*****************************************************************************
 * Public methods
 *****************************************************************************/

UnitList::UnitList() {
	// read the default unit list instead
	bool success = read_default(); (void)success;
	IBK_ASSERT(success);
	// ensure correct unit IDs for seconds and meters
	for (const_iterator it = begin(); it != end(); ++it) {
		if ((*it)->name() == "s") {
			IBK_ASSERT((*it)->id() == IBK_UNIT_ID_SECONDS);
		}
		else if ((*it)->name() == "m") {
			IBK_ASSERT((*it)->id() == IBK_UNIT_ID_METERS);
		}
	}

	// populate DEPRECATED_LIST
	DEPRECATED_LIST.clear();

	//	"L/m2s        * 3600 L/m2h        * 86400 L/m2d       * 86400 mm/d        * 3600 mm/h  * 1 l/m2s   * 3600 l/m2h  * 86400 l/m2d     \n"
	//	"L/m3s        * 3600 L/m3h        * 1 l/m3s           * 3600 l/m3h     \n"

	DEPRECATED_LIST["l/m2s"] = "L/m2s";
	DEPRECATED_LIST["l/m2h"] = "L/m2h";
	DEPRECATED_LIST["l/m2d"] = "L/m2d";

	DEPRECATED_LIST["l/m3s"] = "L/m3s";
	DEPRECATED_LIST["l/m3h"] = "L/m3h";
}

UnitList & UnitList::instance() {
	static UnitList theList;
	return theList;
}

// Reads the unit list from a file
bool UnitList::read_file(const std::string& filename, bool overwrite) {
	std::ifstream in;
	IBK::open_ifstream(in, IBK::Path(filename));
	return read(in, overwrite);
}
// ---------------------------------------------------------------------------


// Reads the unitlist from default string
bool UnitList::read_default() {
	std::stringstream strm(DEFAULT_UNITS);
	bool success = read(strm, true); // we always overwrite when reading the default units
	if (success)
		m_builtinUnitCount = size();
	return success;
}


// Reads the unitlist from an input filestream
bool UnitList::read(std::istream& stream, bool overwrite) {
	FUNCID(UnitList::read);
	if (!stream)  return false;

	unsigned int base_index=0, current_index=0;
	if (overwrite) {
		clear();                        // Clear list
		m_builtinUnitCount = 0;
	}

	std::set<std::string> all_units; // temporary set of units, used in duplicate check

	while (stream.good()) {
		std::stringstream ins;           // Input-String-Stream
		std::string str;                 // Unit string
		std::getline(stream,str);
		ins << str;                 // fill string stream
		char op=' ';                // conversion operator
		unsigned int op_id;         // enum value of operator
		double factor;              // conversion factor

		// read first unit in line
		ins >> str;
		if (ins.good()) {
			// sanity check, ensure uniqueness of units
			if (all_units.find(str) != all_units.end())
				throw IBK::Exception(FormatString("Duplicate unit '%1' defined.").arg(str), FUNC_ID);
			all_units.insert(str);

			// add base unit to the list
			add(new UnitData(current_index, str, base_index, 1.0, OP_NONE));

			// Try reading the next unit
			ins >> op >> factor >> str;
			while (!ins.fail()) {
				// sanity check, ensure uniqueness of units
				if (all_units.find(str) != all_units.end())
					throw IBK::Exception(FormatString("Duplicate unit '%1' defined.").arg(str), FUNC_ID);
				all_units.insert(str);

				current_index++;
				switch (op) {
					case '+' : op_id = OP_ADD; break;
					case '-' : op_id = OP_SUB; break;
					case '*' : op_id = OP_MUL; break;
					case '/' : op_id = OP_DIV; break;
					case '%' : op_id = OP_SPECIAL; break;
					default  : op_id = OP_NONE;
				}
				add(new UnitData(current_index, str, base_index, factor, op_id));
				ins >> op >> factor >> str;
			}
			current_index++;          // next unit
			base_index=current_index; // is also the base unit
		}
	}
	if (empty()) return false;
	return true;
}
// ---------------------------------------------------------------------------

bool UnitList::read(const std::string& str, bool overwrite) {
	std::string tmp = str;
	std::replace(tmp.begin(), tmp.end(), ';', '\n');
	std::istringstream lstrm(tmp);
	return read(lstrm, overwrite);
}
// ---------------------------------------------------------------------------

bool UnitList::write(const std::string& filename) const {
	if (empty()) return false;

	std::ofstream out(filename.c_str());
	// write first base unit
	out << std::setw(10) << std::left << list_.front()->name() << "   ";
	// remember last base unit index
	unsigned int last_base=0;
	const_iterator endit=end();
	const_iterator it=begin();
	++it;
	for (; it!=endit; ++it) {
		// get next unit data
		UnitData* unit=*it;
		// do we have a new base unit?
		if (unit->base_id()!=last_base) {
			// new line and new base unit
			out << ";" << std::endl << std::setw(10) << std::left << unit->name() << "   ";
			last_base=unit->base_id();
		} else {
			std::stringstream ustream;
			switch (unit->operation()) {
				case OP_ADD : ustream << "+ "; break;
				case OP_SUB : ustream << "- "; break;
				case OP_MUL : ustream << "* "; break;
				case OP_DIV : ustream << "/ "; break;
				case OP_SPECIAL : ustream << "% "; break;
				default     : ustream << "| "; break;
			}
			ustream << unit->factor() << " " << unit->name() << " ";
			out << std::setw(20) << std::left << ustream.str();
		}
	}
	out << ";" << std::endl;    // finish last line
	return out.good();
}
// ---------------------------------------------------------------------------

bool UnitList::convertible_units(std::vector<Unit>& list, const Unit& src,
								  const bool removeSrcUnit) const
{
	unsigned int baseid( src.base_id() );
	list.clear();           // empty list
	unsigned int id=src.id();
	const_iterator endit=end();
	const_iterator it=begin();
	while (it!=endit) {
		// get pointer to current unit and increase pointer afterwards
		const UnitData* unit( *it++ );
		// 1. test whether base units are the same
		if (unit->base_id()==baseid) {
			// 2. get unit id
			unsigned int uid=unit->id();
			// 3. if units to not match and remove is == true, do NOT add unit
			if (removeSrcUnit && uid==id) continue;
			// 4. add unit to the list
			list.push_back( Unit(uid) );
		}
	}
	if (list.empty()) return false;
	return true;
}

// ---------------------------------------------------------------------------

void UnitList::all_units( std::vector<Unit>& list ) const {
	// empty list
	list.clear();
	const_iterator endit=end();
	const_iterator it=begin();
	while (it!=endit) {

		// get pointer to current unit and increase pointer afterwards
		const UnitData* unit( *it++ );

		// ignore undifined units
		if (unit->name() == "undefined")
			continue;

		// get unit id
		unsigned int uid=unit->id();
		// add unit to the list
		list.push_back( Unit(uid) );

	}
}

// ---------------------------------------------------------------------------

bool UnitList::base_units(std::vector<Unit>& list) {
	if (empty())  return false;
	list.clear();
	const_iterator endit=end();
	const_iterator it=begin();
	while (it!=endit) {
		// get pointer to current unit and increase pointer afterwards
		const UnitData* unit( *it++ );
		if (unit->id()==unit->base_id())
			list.push_back( Unit(unit->id()) );
	}
	return true;
}
// ---------------------------------------------------------------------------

const UnitData* UnitList::retrieve(unsigned int unitid) const {
	if (unitid >= size())
		throw IBK::Exception( "[UnitList::retrieve]  Unit index "+val2string(unitid)+" out of range 0.."+val2string(static_cast<int>(size())-1)+"!","[UnitList::retrieve]" );
	return operator[](unitid);
}
// ---------------------------------------------------------------------------

const UnitData* UnitList::retrieve(const std::string &ustr) const {
	// apply deprecated unit conversion
	std::string str;
	std::map<std::string, std::string>::const_iterator depit = DEPRECATED_LIST.find(ustr);
	if (depit == DEPRECATED_LIST.end())
		str = ustr;
	else
		str = depit->second;
	const_iterator endit=end();
	const_iterator it=begin();
	while (it!=endit) {
		const UnitData* unit( *it );
		std::string name = unit->name();
		if ( name==str)
			return unit;
		++it;
	}
	return nullptr;
}
// ---------------------------------------------------------------------------

// Calculates conversion factor and operation to relate the src and target units
void UnitList::relate_units(const UnitData* src, const UnitData* target,
							 double& fact, unsigned int& op) const
{
	// Are the units the same?
	if (src->id()==target->id())  {
		fact=1;
		op=OP_NONE;
	}
	// is it actually possible to convert the units?
	if (src->base_id()!=target->base_id())
		throw IBK::Exception("Can't relate unit ["+
			src->name()+"] to ["+target->name()+ "]!","[UnitList::relate_units]");
	// check if src unit or target unit require special operations
	if (src->operation()==OP_SPECIAL || target->operation()==OP_SPECIAL) {
		fact = 1;
		op = OP_SPECIAL;
		return;
	}
	// check if src unit is already a base unit (no conversion necessary)
	if (src->operation()==OP_NONE) {
		// return only + or -
		// If we convert backward, we need to get the opposite conversion operation.
		switch (target->operation()) {
			case OP_ADD :
				op=OP_ADD;
				fact=target->factor();
				break;

			case OP_SUB :
				op=OP_ADD;
				fact=-target->factor();
				return;

			case OP_MUL :
				op=OP_MUL;
				fact=target->factor();
				return;

			case OP_DIV :
				op=OP_MUL;
				if (target->factor()==0.)
					throw IBK::Exception("Div by zero! (tried to convert ["+src->name()+
						"] to ["+target->name()+ "] )!","[UnitList::relate_units]");
				fact=1.0/target->factor();
				return;

			default :
				fact=1;
				op=OP_NONE;
				return;
		}
	}

	// test + or - operation
	if (src->operation()==OP_ADD || src->operation()==OP_SUB) {
		// DO NOT modifiy the member variable, create copy of variable.
		double own_factor=src->factor();
		if (src->operation()==OP_ADD)
			own_factor=-own_factor;
		// calculate conversion factors
		switch (target->operation()) {
			case OP_ADD :
				op=OP_ADD;
				fact=own_factor+target->factor();
				return;

			case OP_SUB :
				op=OP_ADD;
				fact=own_factor-target->factor();
				return;

			case OP_MUL :    // both multiplication and division aren't allowed
			case OP_DIV :
				throw IBK::Exception("* and / operations "
					"not allowed when using + and - operations! "
					"(tried to convert ["+src->name()+"] to ["+target->name()+ "] )!",
					"[UnitList::relate_units]");

			default :
				fact=src->factor();
				op=OP_ADD;
				return;
		}
	}
	// test * or / operation
	if (src->operation()==OP_MUL || src->operation()==OP_DIV) {
		double own_factor=src->factor();
		// convert to base unit
		if (src->operation()==OP_MUL) {
			if (own_factor==0.)
				throw IBK::Exception("Div by zero! "
					"(tried to convert ["+src->name()+"] to ["+target->name()+ "] )!",
					"[UnitList::relate_units]");
			own_factor=1.0/own_factor;
		}
		// Calculate conversion factors
		switch (target->operation()) {
			case OP_ADD : // addition and subtraction are not allowed in combination with
			case OP_SUB : // * and /
				throw IBK::Exception("+ and - operations "
					"not allowed when using * and / operations! "
					"(tried to convert ["+src->name()+"] to ["+target->name()+ "] )!",
					"[UnitList::relate_units]");

			case OP_MUL :
				op=OP_MUL;
				fact=own_factor*target->factor();
				return;

			case OP_DIV :
				op=OP_MUL;
				if (target->factor()==0.)
					throw IBK::Exception("Div by zero! "
						"(tried to convert ["+src->name()+"] to ["+target->name()+ "] )!",
						"[UnitList::relate_units]");
				fact=own_factor/target->factor();
				return;

			default :
				fact=own_factor;
				op=OP_MUL;
				return;
		}
	}
}
// ---------------------------------------------------------------------------

void UnitList::convert(Unit src, Unit target, double& val) {
	 const UnitData* s( retrieve(src.id()) );
	 const UnitData* t( retrieve(target.id()) );
	 convert(s,t,val);
}
// ---------------------------------------------------------------------------

void UnitList::convert(const UnitData* src, const UnitData* target, double& val) {
	double fact;
	unsigned int op;
	relate_units(src, target, fact, op);
	if (op==OP_ADD) {
		val+=fact;
	}
	else if (op==OP_MUL) {
		val*=fact;
	}
	else if (op==OP_SPECIAL) {
		convert_special(src, target, val);
	}
}
// ---------------------------------------------------------------------------

void UnitList::convert_special(const UnitData* src, const UnitData* target, double& val) {
	// first convert value from src unit to base unit, store result in base_val
	const UnitData* base( retrieve(src->base_id()) );
	double base_val = val;
	if (src->operation()==OP_SPECIAL) {
		if (src->name() == "sqrt(s)") {
			base_val = val*val;        	          // from sqrt(s) -> s
		}
		else if (src->name() == "sqrt(h)") {
			base_val = val*val*3600.0;	          // from sqrt(h) -> s
		}
		else if (src->name() == "log(kg/m3)") {
			base_val = IBK::f_pow10(val);         // from log(kg/m3) -> kg/m3
		}
		else if (src->name() == "log(g/m3)") {
			base_val = IBK::f_pow10(val)/1e+3;    // from log(g/m3) -> kg/m3
		}
		else if (src->name() == "log(mg/m3)") {
			base_val = IBK::f_pow10(val)/1e+6;    // from log(mg/m3) -> kg/m3
		}
		else if (src->name() == "log(µg/m3)") {
			base_val = IBK::f_pow10(val)/1e+9;    // from log(µg/m3) -> kg/m3
		}
	}
	else {
		 convert(src, base, base_val);
	}

	// then convert from base unit to target unit
	if (target->operation()==OP_SPECIAL) {
		if (target->name() == "sqrt(s)") {
			if (base_val < 0)
				throw IBK::Exception("Trying to take square root of negative number!",
					"[UnitList::convert_special]");
			val = std::sqrt(base_val);            // from s -> sqrt(s)
		}
		else if (target->name() == "sqrt(h)") {
			if (base_val < 0)
				throw IBK::Exception("Trying to take square root of negative number!",
					"[UnitList::convert_special]");
			val = std::sqrt(base_val/3600.0);          // from s -> sqrt(h)
		}
		else if (target->name() == "log(kg/m3)") {
			if (base_val <= 0)
				throw IBK::Exception("Trying to take logarithm of zero or negative number!",
					"[UnitList::convert_special]");
			val = IBK::f_log10(base_val);         // from kg/m3 -> log(kg/m3)
		}
		else if (target->name() == "log(g/m3)") {
			if (base_val <= 0)
				throw IBK::Exception("Trying to take logarithm of zero or negative number!",
					"[UnitList::convert_special]");
			val = IBK::f_log10(base_val*1e+3);    // from kg/m3 -> log(g/m3)
		}
		else if (target->name() == "log(mg/m3)") {
			if (base_val <= 0)
				throw IBK::Exception("Trying to take logarithm of zero or negative number!",
					"[UnitList::convert_special]");
			val = IBK::f_log10(base_val*1e+6);    // from kg/m3 -> log(mg/m3)
		}
		else if (target->name() == "log(µg/m3)") {
			if (base_val <= 0)
				throw IBK::Exception("Trying to take logarithm of zero or negative number!",
					"[UnitList::convert_special]");
			val = IBK::f_log10(base_val*1e+9);    // from kg/m3 -> log(µg/m3)
		}
	}
	else {
		val = base_val;
		convert(base, target, val);
	}
}
// ---------------------------------------------------------------------------

IBK::Unit UnitList::integralQuantity(const IBK::Unit & srcUnit, bool spaceIntegral, bool timeIntegral) {
	FUNCID(UnitList::integralQuantity);
	// first retrieve name of base unit
	std::string base_name = srcUnit.base_unit().name();
	// convert all cases
	bool converted = true;
	if (spaceIntegral) {
		if (timeIntegral) {
			// space-time integration
			// search for strings ending in "/m3s"
			if (base_name.size() > 4 && base_name.rfind("/m3s") == base_name.size()-4) {
				base_name = base_name.substr(0, base_name.size()-4);
			}
			else if (base_name.size() > 4 && base_name.rfind("/m2s") == base_name.size()-4) {
				base_name = base_name.substr(0, base_name.size()-4);
			}
			else if (base_name.size() > 3 && base_name.rfind("/ms") == base_name.size()-3) {
				base_name = base_name.substr(0, base_name.size()-3);
			}
			else {
				converted = false;
			}
		}
		else {
			if (base_name.size() > 4 && base_name.rfind("/m3s") == base_name.size()-4) {
				base_name = base_name.substr(0, base_name.size()-4) + "/s";
			}
			else if (base_name.size() > 4 && base_name.rfind("/m2s") == base_name.size()-4) {
				base_name = base_name.substr(0, base_name.size()-4) + "/s";
			}
			else if (base_name.size() > 3 && base_name.rfind("/ms") == base_name.size()-3) {
				base_name = base_name.substr(0, base_name.size()-3) + "/s";
			}
			else if (base_name.size() > 3 && base_name.rfind("/m3") == base_name.size()-3) {
				base_name = base_name.substr(0, base_name.size()-3);
			}
			else if (base_name.size() > 3 && base_name.rfind("/m2") == base_name.size()-3) {
				base_name = base_name.substr(0, base_name.size()-3);
			}
			else {
				converted = false;
			}
		}
	} else if (timeIntegral) {
		// search for string ending in "/m2s", "/m3s"
		if (base_name.size() > 4 && base_name.rfind("/m2s") == base_name.size()-4) {
			base_name = base_name.substr(0, base_name.size()-1); // -> /m2
		}
		else if (base_name.size() > 4 && base_name.rfind("/m3s") == base_name.size()-4) {
			base_name = base_name.substr(0, base_name.size()-1); // -> /m3
		}
		else if (base_name.size() > 3 && base_name.rfind("/ms") == base_name.size()-3) {
			base_name = base_name.substr(0, base_name.size()-1); // -> /m
		}
		else if (base_name.size() > 2 && base_name.rfind("/s") == base_name.size()-2) {
			base_name = base_name.substr(0, base_name.size()-2);
		}
		else if (base_name == "---") {
			base_name = "s";
		}
		else {
			converted = false;
		}
	}
	else {
		return srcUnit;
	}
	if (!converted) {
		throw IBK::Exception( IBK::FormatString("Cannot obtain integral unit for base unit %1.").arg(base_name), FUNC_ID );
	}
	return IBK::Unit(base_name);
}
// ---------------------------------------------------------------------------

std::string UnitList::replaceUnitWithIntegralUnit(const std::string & original) {
	FUNCID(UnitList::replaceUnitWithIntegralUnit);
	// extract unit string
	std::string::size_type pos1 = original.find('[');
	std::string::size_type pos2 = original.find(']');
	if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 > pos2) {
		throw IBK::Exception( IBK::FormatString("Malformed string '%1', expected [<unit>] within string.").arg(original), FUNC_ID);
	}
	std::string unitStr = original.substr(pos1+1, pos2-pos1-1);
	// convert unit
	IBK::Unit uIntegral;
	try {
		IBK::Unit u(unitStr);
		// try to create integral unit
		uIntegral = integralQuantity(u, false, true);
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Unknown/invalid unit in string '%1', cannot determine time integral unit.").arg(original), FUNC_ID);
	}
	return original.substr(0, pos1+1) + uIntegral.name() + original.substr(pos2);
}
// ---------------------------------------------------------------------------


/// !!!! **** WARNING **** !!!!
///
/// When you modify units before "m" or "s", make sure to update the defines in IBK_Unit.h  !!!!!
///


// The units defined per default in the unitlist
const char * const DEFAULT_UNITS =
"undefined    \n"
"m            * 1e+03 mm          * 1e+02 cm          * 1e+01 dm       \n"
"m2           * 1e+06 mm2         * 1e+04 cm2         * 1e+02 dm2      \n"
"m3           * 1e+09 mm3         * 1e+06 cm3         * 1e+03 dm3      \n"
"s            / 60 min            / 3600 h            / 86400 d           / 3.1536e+07 a      % 0       sqrt(s)   % 0       sqrt(h)  * 1000 ms \n"
"s/s          / 60 min/s          / 3600 h/s          / 86400 d/s         / 3.1536e+07 a/s \n"
"m/s          * 100 cm/s          * 360000 cm/h       * 8.64e+06 cm/d     \n"
"m2/s         * 10000 cm2/s       * 3600 m2/h         * 3.6e+07 cm2/h     \n"
"m/s2         \n"
"s/m          * 1 kg/m2sPa        \n"
"s2/m2        \n"
"kg           * 1000 g            * 1e+06 mg          / 1000 t\n"
"kg/ms        \n"
"kg/s         * 3600 kg/h         * 86400 kg/d        * 1000 g/s          * 3.6e6 g/h         * 8.64e+07 g/d      * 31536e6 g/a   * 1e6 mg/s      * 1e9 µg/s    \n"
"kg/m2        / 100 kg/dm2        * 10 g/dm2          / 10 g/cm2          * 1e6 mg/m2         \n"
"kg/m2s       * 1000 g/m2s        * 3.6e+06 g/m2h     * 86.4e+06 g/m2d    * 3600 kg/m2h       * 1e6 mg/m2s        * 1e9 µg/m2s  * 3.6e9 mg/m2h    * 3.6e12 µg/m2h \n"
"kg/m3        / 1000 kg/dm3       * 1 g/dm3           / 1000 g/cm3        * 1000 g/m3         * 1e6 mg/m3         * 1000 kg/L   * 1 g/L         / 1000 mg/L     * 1e9 µg/m3       % 0      log(kg/m3)    % 0    log(g/m3)    % 0    log(mg/m3)    % 0    log(µg/m3) \n"
"kg/m3s       * 1000 g/m3s        * 3.6e+06 g/m3h     * 3600 kg/m3h       * 1e6 mg/m3s        * 1e9 µg/m3s        * 3.6e9 mg/m3h  * 3.6e12 µg/m3h \n"
"kg/m         * 1000 g/m          * 1 g/mm            / 1000 kg/mm        \n"
"kg/kg        * 1000 g/kg         * 1e6 mg/kg         \n"
"kg/m2s05     * 60 kg/m2h05       \n"
"J            / 1000 kJ           / 1e+06 MJ          / 3.6e+09 MWh       / 3.6e+06 kWh        / 3600     Wh  \n"
"J/m2         / 1000 kJ/m2        / 1e+06 MJ/m2       / 1e+09 GJ/m2       / 1e+02 J/dm2        / 1e+04 J/cm2       / 3.6e+06 kWh/m2          \n"
"J/m3         * 1    Ws/m3        / 1000 kJ/m3        / 1e+06 MJ/m3       / 1e+09 GJ/m3       / 1e+03 J/dm3        / 1e+06 J/cm3       / 3.6e+06 kWh/m3          \n"
"J/m3s        / 1000 kJ/m3s       / 1e+06 MJ/m3s      / 1000 J/dm3s       / 1e+06 J/cm3s       * 3600  J/m3h       * 1    W/m3         / 1000 kW/m3        / 1e+06 MW/m3       / 1000 W/dm3        / 1e+06 W/cm3       / 1e+09 W/mm3       \n"
"J/m3K        / 1000 kJ/m3K\n"
"J/s          * 3600 J/h          * 86400 J/d         * 86.4 kJ/d         * 1     W            / 1000 kW           / 1e+06 MW          * 1     Nm/s  \n"
"J/kg         / 1000 kJ/kg        \n"
"J/kgK        / 1000 kJ/kgK       * 1     Ws/kgK      / 1000 J/gK       / 1000 Ws/gK       \n"
"J/K          / 1000 kJ/K         \n"
"J/m2s        * 1    W/m2         / 1000 kW/m2        / 1e+06 MW/m2       / 100 W/dm2         / 10000 W/cm2       \n"
"W/mK         / 1000 kW/mK        \n"
"W/m2K        \n"
"W/m2K2       \n"
"W/mK2        \n"
"L/m2s        * 3600 L/m2h        * 86400 L/m2d       * 86400 mm/d        * 3600 mm/h\n"
"L/m3s        * 3600 L/m3h\n"
"m3/m2s       * 3600 m3/m2h       * 1000 dm3/m2s      * 3.6e+06 dm3/m2h   \n"
"m3/m2sPa     * 3600 m3/m2hPa     \n"
"m3/s         * 3600 m3/h         * 1000 dm3/s        * 3.6e+06 dm3/h   \n"
"m3/m3        * 100 Vol%          \n"
"m3/m3d       * 100 Vol%/d        \n"
"---          * 100 %             * 1 1               * 10 1/10           * 8 1/8         \n"
"---/d        * 100 %/d           \n"
"K            - 273.15 C          \n"
"1/K          \n"
"K/m          \n"
"m2s/kg       \n"
"Pa           / 100 hPa           / 1000 kPa          * 1e-05 Bar        * 0.000145038 PSI         * 0.00750062 Torr          \n"
"1/Pa         \n"
"Pa/m         / 1000 kPa/m        \n"
"Lux          / 1000 kLux         \n"
"Rad          * 57.2958 Deg       \n"
"m2/kg        \n"
"m2K/W        \n"
"1/m          / 100 1/cm          \n"
"logcm        \n"
"logm         \n"
"logPa        \n"
"K/Pa         \n"
"mol/kg       / 1000 mol/g        \n"
"kg/mol       * 1000 g/mol        \n"
"J/mol        / 1000 kJ/mol       \n"
"1/logcm      \n"
"mol/m3       / 1000 mol/L      / 1000 mol/dm3         / 1e+06 mol/cm3     / 1 mmol/L      / 1 mM \n"
"mol          * 1e+03 mmol        \n"
"-            "
"mm/mK        \n"
"mm/m         \n"
"m3m/m3m      * 1000 m3mm/m3m\n"
"kg/m3sK      * 1000 g/m3sK       * 3.6e+06 g/m3hK     * 3600 kg/m3hK      * 1e6 mg/m3sK   * 1e9 µg/m3sK * 3.6e9 mg/m3hK   * 3.6e12 µg/m3hK \n"
"1/s          * 60 1/min          * 3600 1/h \n"
"W/m2s *  3600.0000000       W/m2h  /  1000.0000000      kW/m2s /  1.000000e+06      MW/m2s /   100.0000000      W/dm2s / 10000.0000000      W/cm2s \n"
"Person/m2    \n"
"m2/Person    \n"
"W/Person     / 1000 kW/Person    \n"
"W/K          \n"
"kWh/m2a\n"
"kWh/a\n"
"m2/m3\n"
"Kh\n"
;


}  // namespace IBK


