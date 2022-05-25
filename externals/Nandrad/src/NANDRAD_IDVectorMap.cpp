#include "NANDRAD_IDVectorMap.h"

#include <algorithm>

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

namespace NANDRAD {


void IDVectorMap::setEncodedString(const std::string & str) {
	FUNCID(IDVectorMap::setEncodedString);
	std::vector<std::string> tokens;
	IBK::explode(str, tokens, ';', IBK::EF_TrimTokens);
	m_values.clear();
	int count = -1;
	for (std::string & colStr : tokens) {
		// tokens might have leading/trailing \n, so we execute a trim again
		IBK::trim(colStr, " \t\r\n");
		std::vector<std::string> parts;
		IBK::explode_in2(colStr, parts, ':');
		if (parts.size() != 2)
			throw IBK::Exception(IBK::FormatString("Invalid data in table, expected ':' as separator in column data '%1'").arg(colStr), FUNC_ID);
		std::vector<double> valVector; // we use double here as this is easier for conversion
		std::string line_with_spaces = parts[1];
		line_with_spaces = IBK::replace_string(parts[1], ",", " ");
		try {
			IBK::string2valueVector(line_with_spaces, valVector);
		} catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Invalid data in table, in column data '%1'").arg(colStr), FUNC_ID);
		}
		// check for correct column count
		if (count == -1)
			count = valVector.size();
		else if (count != (int)valVector.size())
			throw IBK::Exception(IBK::FormatString("Mismatching number of values (=%1) in column '%2' compared to previous columns.").arg(valVector.size()).arg(parts[0]), FUNC_ID);
		// check that column name isn't duplicated
		unsigned int keyId = IBK::string2val<unsigned int>(parts[0]);
		if (m_values.find(keyId) != m_values.end())
			throw IBK::Exception(IBK::FormatString("Duplicate column ID '%1' in table.").arg(keyId), FUNC_ID);
		// create new vector
		m_values[keyId] = std::vector<unsigned int>();
		for (const double & val: valVector)
			m_values[keyId].push_back((unsigned int)val);
	}
}


std::string IDVectorMap::encodedString() const {
	std::stringstream strm;
	for (const std::pair<const unsigned int, std::vector<unsigned int> > & column : m_values) {
		strm << column.first << ":";
		for (unsigned int i=0; i<column.second.size(); ++i) {
			strm << column.second[i];
			if (i < column.second.size()-1)
				strm << ",";
		}

		strm << ";";
	}
	return strm.str();
}

} // namespace NANDRAD
