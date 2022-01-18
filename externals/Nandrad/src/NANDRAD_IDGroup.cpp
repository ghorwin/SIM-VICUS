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

#include "NANDRAD_Constants.h"
#include "NANDRAD_IDGroup.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_assert.h>

#include <algorithm>

namespace NANDRAD {

void IDGroup::setEncodedString(const std::string & encodedString) {
	FUNCID(IDGroup::setEncodedString);
	// clear all entries
	m_allIDs = false;
	m_ids.clear();
	m_idIntervals.clear();
	// No data? return
	if (encodedString.empty())
		return;

	try {
		// a model interval is asked for output
		std::list<std::string> idTokens;
		IBK::explode(encodedString,idTokens,',');
		// for each token check for special cases and occurancy of - sign
		for (std::list<std::string>::const_iterator it = idTokens.begin(); it != idTokens.end(); ++it) {
			std::string tokenTrimmed = *it;
			IBK::trim(tokenTrimmed);

			if (tokenTrimmed.empty()) {
				throw IBK::Exception( IBK::FormatString("Invalid encoding format."), FUNC_ID);
			}
			if (tokenTrimmed == "*") {
				// check if set or vector is not empty
				if (!m_idIntervals.empty() || !m_ids.empty())
					throw IBK::Exception(IBK::FormatString("String '*' can only be used stand-alone."), FUNC_ID);
				m_allIDs = true;
			}
			else {
				// check if m_allIDs is still false
				if (m_allIDs)
					throw IBK::Exception( IBK::FormatString("String '*' can only be used stand-alone."), FUNC_ID);

				if (tokenTrimmed.find("-")!= std::string::npos) {
					std::list<std::string> intervalTokens;
					IBK::explode(tokenTrimmed,intervalTokens,'-');
					// check for correct interval format
					if (intervalTokens.size() != 2)
						throw IBK::Exception(IBK::FormatString("Wrong interval format."),FUNC_ID);

					// fix 10-1 to become 1-10 etc.
					unsigned int lowerID = IBK::string2val<unsigned int>(intervalTokens.front());
					unsigned int upperID = IBK::string2val<unsigned int>(intervalTokens.back());
					if (lowerID > upperID)
						std::swap(lowerID, upperID);

					std::pair<unsigned int, unsigned int> idInterval(lowerID, upperID);
					m_idIntervals.push_back(idInterval);
				}
				else {
					m_ids.insert(IBK::string2val<unsigned int> (tokenTrimmed) );
				}
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error initializing IDGroup from String '%1'.").arg(encodedString), FUNC_ID);		/// TODO Anne ich habe hier mal Änderungen durchgeführt bitte prüfen ob das richtig ist.
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError initializing IDGroup from String '%2'.").arg(ex2.what()).arg(encodedString)
							, FUNC_ID);
	}
}


std::string IDGroup::encodedString() const {
	std::string str;
	// first check if all desitnation are chosen
	if (m_allIDs) {
		return std::string("*");
	}
	// select all intervals
	if (!m_idIntervals.empty()) {
		for(unsigned int i=0; i < m_idIntervals.size(); ++i) {
			if (i != 0)
				str += ",";
			str += IBK::val2string<unsigned int> (m_idIntervals[i].first);
			str += "-";
			str += IBK::val2string<unsigned int> (m_idIntervals[i].second);
		}
	}
	// second all single ids
	if (!m_ids.empty()) {
		// set a delimiter between id-intervals and ids
		if (!m_idIntervals.empty() )
			str += ",";

		for (std::set<unsigned int>::const_iterator it = m_ids.begin(); it != m_ids.end(); ++it) {
			if (it != m_ids.begin())
				str += ",";
			str += IBK::val2string<unsigned int> (*it);
		}
	}

	return str;
}


bool IDGroup::empty() const {
	return (!m_allIDs && m_idIntervals.empty() && m_ids.empty());
}


bool IDGroup::contains(unsigned int id) const {
	if (m_allIDs)
		return true;
	if (m_ids.find(id) != m_ids.end() )
		return true;
	for (unsigned int i = 0; i < m_idIntervals.size(); ++i) {
		const std::pair<unsigned int, unsigned int> &interval = m_idIntervals[i];
		// id is inside interval
		if (id >= interval.first && id <= interval.second)
			return true;
	}
	// all checks failed
	return false;
}


const IDGroup IDGroup::operator+(const IDGroup &group) {
	IDGroup mergedIDGroup;
	// merge id groups
	if (group.m_allIDs || m_allIDs) {
		mergedIDGroup.m_allIDs = true;
	}
	else {
		// merge all single id numbers
		std::set<unsigned int>::const_iterator idItA = m_ids.begin();
		for (; idItA != m_ids.end(); ++idItA) {
			unsigned int idA = *idItA;
			// check if model id is part of an interval
			std::vector<std::pair<unsigned int, unsigned int> >::const_iterator intervalItB = group.m_idIntervals.begin();
			while (intervalItB != group.m_idIntervals.end() &&
				(idA < intervalItB->first || idA > intervalItB->second))
				++ intervalItB;
			// id number not part of an interval
			if (intervalItB == group.m_idIntervals.end())
				mergedIDGroup.m_ids.insert(idA);
		}

		for (std::set<unsigned int>::const_iterator idItB = group.m_ids.begin(); idItB != group.m_ids.end(); ++idItB) {
			unsigned int idB = *idItB;
			// check if model id is part of an interval
			std::vector<std::pair<unsigned int, unsigned int> >::const_iterator intervalItA = m_idIntervals.begin();
			while (intervalItA != m_idIntervals.end() &&
				(idB < intervalItA->first || idB > intervalItA->second))
				++ intervalItA;
			// id number not part of an interval
			if (intervalItA == m_idIntervals.end())
				mergedIDGroup.m_ids.insert(idB);
		}

		// cut all id interval from group B out of intervals from group A numbers
		std::vector<std::pair<unsigned int, unsigned int> > complementIntervals;
		std::vector<std::pair<unsigned int, unsigned int> >::const_iterator intervalItA = m_idIntervals.begin();
		for(; intervalItA != m_idIntervals.end(); ++intervalItA) {
			std::pair<unsigned int, unsigned int> intervalA = *intervalItA;
			std::vector<std::pair<unsigned int, unsigned int> >::const_iterator intervalItB = group.m_idIntervals.begin();
			unsigned int k = 0;
			bool complementIsEmpty = false;
			// check if model id crosses an id interval, enlarge if necessary and store interval index
			for (; intervalItB != group.m_idIntervals.end(); ++ intervalItB, ++k) {
				const unsigned int lowerId = intervalItB->first;
				const unsigned int upperId = intervalItB->second;
				// interval A crosses interval B -> reduce interval A
				if (intervalA.first  >= lowerId && intervalA.first <= upperId) {
					intervalA.first = std::max(intervalA.second + 1,upperId + 1);
				}
				if (intervalA.second  >= lowerId && intervalA.second <= upperId) {
					IBK_ASSERT(lowerId != 0 && intervalA.first != 0);
					intervalA.second = std::min(intervalA.first - 1,lowerId - 1);
				}
				// intervalA is empty after complement operation
				if (intervalA.second == 0 || intervalA.second < intervalA.first) {
					complementIsEmpty = true;
					break;
				}
			}
			// only add filled intervals
			if (!complementIsEmpty)
				complementIntervals.push_back(intervalA);
		}
		mergedIDGroup.m_idIntervals = complementIntervals;
		// now add all remaining intervals of id group B
		for (unsigned int i = 0; i < group.m_idIntervals.size(); ++i) {
			std::pair<unsigned int, unsigned int> intervalB = group.m_idIntervals[i];
			mergedIDGroup.m_idIntervals.push_back(intervalB);
		}
	}
	return mergedIDGroup;
}


bool IDGroup::operator!=(const IDGroup & other) const {
	return (m_allIDs != other.m_allIDs ||
			m_ids != other.m_ids ||
			m_idIntervals != other.m_idIntervals);
}


} // namespace NANDRAD

