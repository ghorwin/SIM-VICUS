#include "NANDRAD_DataTable.h"

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>

namespace NANDRAD {


void DataTable::setEncodedString(const std::string & str) {
	FUNCID(DataTable::setEncodedString);
	std::vector<std::string> tokens;
	IBK::explode(str, tokens, ';', IBK::EF_TrimTokens);
	m_values.clear();
	int count = -1;
	for (const std::string & colStr : tokens) {
		std::vector<std::string> parts;
		IBK::explode_in2(colStr, parts, ':');
		if (parts.size() != 2)
			throw IBK::Exception(IBK::FormatString("Invalid data in table, expected ':' as separator in column data '%1'").arg(colStr), FUNC_ID);
		std::vector<double> val;
		std::string line_with_spaces = IBK::replace_string(parts[1], ",", " ");
		try {
			IBK::string2valueVector(line_with_spaces, val);
		} catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Invalid data in table, in column data '%1'").arg(colStr), FUNC_ID);
		}
		// check for correct column count
		if (count == -1)
			count = val.size();
		else if (count != (int)val.size())
			throw IBK::Exception(IBK::FormatString("Mismatching number of values (=%1) in column '%2' compared to previous columns.").arg(val.size()).arg(parts[0]), FUNC_ID);
		// check that column name isn't duplicated
		if (m_values.find(parts[0]) != m_values.end())
			throw IBK::Exception(IBK::FormatString("Duplicate column ID '%1' in table.").arg(parts[0]), FUNC_ID);
		m_values[parts[0]] = val;
	}
}


std::string DataTable::encodedString() const {
	// encode data table
	std::stringstream strm;
	for (const std::pair<const std::string, std::vector<double> > & column : m_values) {
		strm << IBK::trim_copy(column.first) << ":";
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
