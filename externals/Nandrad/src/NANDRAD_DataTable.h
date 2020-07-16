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
		Throws an IBK::Exception, if number of rows in columns mismatches.
	*/
	void setEncodedString(const std::string & str);

	/*! Returns content of data table as encoded string. */
	std::string encodedString() const;


	/*! The actual data member. */
	std::map<std::string, std::vector<double> >		m_values;
};

} // namespace NANDRAD

#endif // NANDRAD_DataTableH
