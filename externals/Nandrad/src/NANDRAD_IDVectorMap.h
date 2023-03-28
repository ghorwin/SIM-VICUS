#ifndef NANDRAD_IDVectorMapH
#define NANDRAD_IDVectorMapH

#include <map>
#include <vector>
#include <string>

#include <IBK_Exception.h>
#include <IBK_StringUtils.h>



namespace NANDRAD {


/*! Data structure that contains a map, where keys are ids and values are vectors of values, for example other IDs.
	For each id (key) there can be a different number of associated values in the vector.
*/
template<typename t>
class IDVectorMap {
public:

	/*! Inequility operator. */
	bool operator!=(const IDVectorMap & other) const {
		return m_values != other.m_values;
	}

	/*! Sets content of vector map from encoded string.
		Setting the following string "id1:1 5 3;id2:7 2 2" is equivalent to executing
		the following code:
		\code
		m_values[id1] = std::vector<unsigned int>{1,5,3};
		m_values[id2] = std::vector<unsigned int>{7,2,2};
		\endcode

		\note It is possible to use "," and a whitespace (space or tab) character as number separator.
	*/
	void setEncodedString(const std::string & str) {
		FUNCID(IDVectorMap::setEncodedString);
		std::vector<std::string> tokens;
		IBK::explode(str, tokens, ';', IBK::EF_TrimTokens);
		m_values.clear();
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
			// check that column name isn't duplicated
			unsigned int keyId = IBK::string2val<unsigned int>(parts[0]);
			if (m_values.find(keyId) != m_values.end())
				throw IBK::Exception(IBK::FormatString("Duplicate ID '%1' for map keys.").arg(keyId), FUNC_ID);
			// create new vector
			m_values[keyId] = std::vector<t>();
			for (const double & val: valVector)
				m_values[keyId].push_back((t)val);
		}
	}

	/*! Returns content of data table as encoded string using tab a value separator.
		\note Values are written with default number formatting.
	*/
	std::string encodedString() const {
		std::stringstream strm;
		for (const std::pair<const unsigned int, std::vector<t> > & column : m_values) {
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


	/*! The actual data member.
		\warning Do not expect vectors to have the same size!
	*/
	std::map<unsigned int, std::vector<t> >		m_values;
};

} // namespace NANDRAD

#endif // NANDRAD_IDVectorMapH
