/*	The NANDRAD data model library.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NANDRAD_DataTableH
#define NANDRAD_DataTableH

#include <map>
#include <vector>
#include <string>

namespace NANDRAD {

/*! A data member for a table with named columns. */
class DataTable {
public:

	/*! Inequility operator. */
	bool operator!=(const DataTable & other) const {
		return m_values != other.m_values;
	}

	/*! Sets content of data table from encoded string.
		Setting the following string "Col1:1 5 3;Col2:7 2 2" is equivalent to executing
		the following code:
		\code
		m_values["Col1"] = std::vector<double>{1,5,3};
		m_values["Col2"] = std::vector<double>{7,2,2};
		\endcode
		Throws an IBK::Exception, if number of rows in columns mismatches.

		\note It is possible to use , and a whitespace (space or tab) character as number separator.
	*/
	void setEncodedString(const std::string & str);

	/*! Returns content of data table as encoded string using tab a value separator. */
	std::string encodedString() const;

	/*! Convenience function that looks up a parameter data vector in the map and returns
		a const reference to it.
		Throws an IBK::Exception if the parameter name doesn't exist.
	*/
	const std::vector<double> & valueVector(const std::string & parameterName) const;


	/*! The actual data member. */
	std::map<std::string, std::vector<double> >		m_values;
};

} // namespace NANDRAD

#endif // NANDRAD_DataTableH
