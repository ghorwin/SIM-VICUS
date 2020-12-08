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

#ifndef IBK_UnitDataH
#define IBK_UnitDataH

#include <string>
#include <iosfwd>

namespace IBK {

/*! The class UnitData holds the information for a single unit.
	This class is internally used by the UnitList and shouldn't be used directly. Normally, using IBK::Unit
	is sufficient as light-weight identification data member.
	\todo Make this class a private internal class of UnitList.
*/
class UnitData {
  public:
	/*! Default constructor, creates an undefined unit. */
	UnitData() : id_(0), name_("undefined"), base_id_(0), factor_(0), operation_(0)  {}

	/*! Initialisation constructor. */
	UnitData(const unsigned int unitid, const std::string& name,
			  const unsigned int baseunit, double fact, const unsigned int op)
	  :  id_(unitid), name_(name), base_id_(baseunit), factor_(fact), operation_(op) {}

	/*! Returns the ID number of the unit. */
	unsigned int  id()          const     { return id_; }
	/*! Returns the unit string (or name). */
	const std::string&  name()        const     { return name_; }
	/*! Returns the ID of the appropriate base unit. */
	unsigned int  base_id()     const     { return base_id_; }
	/*! Returns the factor or summand for conversion into the base unit. */
	double        factor()      const     { return factor_; }
	/*! Returns the operation for converting the unit into the base unit. */
	unsigned int  operation()   const     { return operation_; }

  private:
	unsigned int id_;           ///< ID number.
	std::string  name_;         ///< String constant, unit name.
	unsigned int base_id_;      ///< Assigned base unit it.
	double       factor_;       ///< Conversion factor/summand.
	unsigned int operation_;    ///< Conversion operation.

	/*! Returns true, if two units are equal (have the same ID number). */
	friend bool operator==(const UnitData& lhs, const UnitData& rhs);
	/*! Returns true, if two units are not equal (do not have the same ID number). */
	friend bool operator!=(const UnitData& lhs, const UnitData& rhs);
}; // class UnitData

inline bool operator==(const UnitData& lhs, const UnitData& rhs) {
						return (lhs.id_==rhs.id_); }
inline bool operator!=(const UnitData& lhs, const UnitData& rhs) {
						return (lhs.id_!=rhs.id_); }

}  // namespace IBK

/*! \file IBK_UnitData.h
	\brief Contains declaration of class UnitData.
*/

#endif // IBK_UnitDataH
