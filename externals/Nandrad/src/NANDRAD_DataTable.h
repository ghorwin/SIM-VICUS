#ifndef NANDRAD_DataTableH
#define NANDRAD_DataTableH

#include <map>
#include <vector>
#include <string>

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! A data member for a table with named columns. */
class DataTable {
public:

	NANDRAD_READWRITE
	NANDRAD_COMP(DataTable)

	std::map<std::string, std::vector<double> >		m_values;
};

} // namespace NANDRAD

#endif // NANDRAD_DataTableH
