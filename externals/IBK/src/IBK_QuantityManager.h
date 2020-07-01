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

#ifndef IBK_QuantityManagerH
#define IBK_QuantityManagerH

#include <string>
#include <vector>
#include <iosfwd>
#include <map>
#include <set>

#include "IBK_Quantity.h"
#include "IBK_Path.h"

namespace IBK {

/*!	The class QuantityManager manages definitions of physical quantities.

	A list of known quantities can be read from file/string similarly to the unit list
	and then the properties of the different quantities can be retrieved.
	The Quantity manager provides conversion functionality between Quantities ID strings,
	and enumeration values of a defined subset of quantities.

	A quantity is uniquely identified through its type and ID-name.

	The QuantityManager combines the functionality of a manager for all quantities and
	for a subset of quantities actually used in any application.
	\code
	// define quantities (can be done in a text file, descriptions are optional)
	std::string quantities =
		"STATE Moisture mass density [kg/m3] 'Total mass of liquid water and water vapor'\n"
		"STATE Temperature           [C]     'Some temperature'\n"
		"STATE Pressure              [Pa]\n"
		"FLUX HeatFluxDensity        [W/m2]\n";
	// populate quantity manager with quantities
	QuantityManager quantMan;
	quantMan.read(quantities);
	\endcode
*/
class QuantityManager {
public:
	/*! Constructor. */
	QuantityManager();

	/*! Reads list of quantities from string (newline separated entries).
		Previously existing quantities are replaced.
	*/
	void read(const std::string & data);

	/*! Writes list of quantities to output stream. */
	void write(std::ostream & out);

	/*! Convenience function, opens file and reads quantities from file. */
	void readFromFile(const IBK::Path & fname);

	/*! Convenience function, opens file for writing and writes quantities to file. */
	void writeToFile(const IBK::Path & fname);

	/*! Resets the member variables. */
	void clear();

	/*! Returns the index of the quantity in the global list of quantities.
		Indicies are invalidated with each call to read() or clear().
		\return Returns an index if quantity was found, otherwise -1.
	*/
	int index(const Quantity & quantity) const;

	/*! Returns the quantity definition data for the global index.
		Throws an exception when quantity index is invalid.
	*/
	const Quantity & quantity(unsigned int globalIndex) const;

	/*! Looks up the quantity definition by type and name.
		Throws an exception when quantity is invalid/unknown.
	*/
	const Quantity & quantity(IBK::Quantity::type_t t, const std::string & quantityName) const;

	/*! Returns the first quantity definition for a given name (if any).
		Throws an exception when quantity name is invalid/unknown.
	*/
	const Quantity & firstQuantityWithName(const std::string & quantityName) const;

	/*! Generates set with subset of quantities that all belong to requested type. */
	std::set<Quantity> quantitiesOfType(IBK::Quantity::type_t t) const;

	/*! Generates set with subset of quantities that all belong to requested name. */
	std::set<Quantity> quantitiesWithName(const std::string & name) const;

	/*! Gives read-only access to list of quantities. */
	const std::vector<Quantity> & quantities() const { return m_quantities; }

	/*! Adds a new quantity to the quantity manager.
		Will throw an IBK::Exception, if quantity exists already.
	*/
	void addQuantity(const IBK::Quantity & quantity);

private:
	/*! Holds all quantities (optimized for index access using global index). */
	std::vector<Quantity>						m_quantities;

	typedef std::pair<Quantity::type_t, std::string>	QuantityIdentifier;

	/*! Map for connecting unique quantity names to global ids (for reverse lookup). */
	std::map<QuantityIdentifier, unsigned int>			m_globalIndexMap;
};

} // namespace IBK

/*! \file IBK_QuantityManager.h
	\brief Contains the declaration of class QuantityManager.
*/

#endif // IBK_QuantityManagerH
