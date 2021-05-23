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

#ifndef NANDRAD_IDGroupH
#define NANDRAD_IDGroupH

#include <string>
#include <set>
#include <vector>
#include <utility> // for std::pair

namespace NANDRAD {

/*!	An IDGroup references one or more IDs and handles the encoding/decoding
	of an IDGroup-string.
	\code
	// supported encodings
	string severalIDs = "16,20,22";
	string allIDs = "*";
	string idInterval = "5-10,50-60";
	string mixedIDs = "1,3,55,5-10";
	// decode with IDGroup
	IDGroup grp;
	grp.setEncodedString(severalIDs);
	// encode string
	grp.encodedString();
	\endcode
*/
class IDGroup {
public:

	/*! Set ID group data from an encoded string.
		This function throws an exception, if an invalid format is encountered or
		IDs are specified in addition to a wildcard character.
	*/
	void setEncodedString(const std::string & encodedString);

	/*! Encode ID group data into a string representation. */
	std::string encodedString() const;

	/*! Returns true if neither the wildcard flag m_allIDs is set, nor any IDs are
		specified in m_ids or m_idIntervals. */
	bool empty() const;

	/*! Returns true if either the wildcard flag m_allIDs is set, or the
		id is in m_ids or enclosed in m_idIntervals.
	*/
	bool contains(unsigned int id) const;

	/*! Merges two id groups. */
	const IDGroup operator+(const IDGroup &);

	/*! Comparison operator by value. */
	bool operator==(const IDGroup & other) const { return !operator!=(other); }
	/*! Not-equal comparison operator by value. */
	bool operator!=(const IDGroup & other) const;


	// *** PUBLIC MEMBER VARIABLES ***

	/*! If true, the encoded string containted a wildcard character * to indicate all IDs.
		\warning A wildcard overrides all other IDs.
	*/
	bool												m_allIDs = false;
	/*! Set of individually listed IDs. */
	std::set<unsigned int>								m_ids;
	/*! Model id intervals.*/
	std::vector<std::pair<unsigned int, unsigned int> >	m_idIntervals;


}; // IDGroup


} // namespace NANDRAD

#endif // NANDRAD_IDGroupH
