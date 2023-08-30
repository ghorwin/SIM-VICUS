#ifndef VICUS_BTFREADERH
#define VICUS_BTFREADERH


#include <map>
#include <vector>
#include <utility>

#include <IBK_UnitVector.h>
#include <IBK_Time.h>

#include <QStringList>



namespace VICUS {

/*! Data file reader for binary (*.btf) files */

class BTFReader
{
public:

	BTFReader();

	/*! Attempts to reads a file header for a given file.
		\param fname The filename of a file to read the header data from.
		\param captions A vector with all column names (starting from column 1) without units, e.g. if column name "Temperature [C]" it would contain "Temperature"
		\param valueUnits A vector with all according units, also starting from column 1
	*/
	void parseHeaderData(const QString & fname, std::vector<std::string> & captions, std::vector<std::string> & valueUnits);

	/*! Reads all data from file
		\param fname The filename of a file to read the header data from.
		\param timePoints A vector with all time points
		\param values A vector with all other values of column 1..n, the vector is column major.
		\param captions A vector with all column names (starting from column 1) without units, e.g. if column name "Temperature [C]" it would contain "Temperature"
		\param valueUnits A vector with all according units, also starting from column 1
	*/
	void readData(const QString & fname, IBK::UnitVector &timePoints, std::vector<std::vector<double> > &values,
				  std::vector<std::string> & captions, std::vector<std::string> & valueUnits);

private:

	/*! Parses the header captions and extracts column headers and units.	*/
	void extractUnits(const std::vector<std::string> & columnHeaders,
					 IBK::Unit & timeUnit, std::vector<std::string> & captions,
					 std::vector<std::string> & valueUnits);


	/*! This time stamp is the time stamp of Midnight January 1st of the year of the first data sample. It is used to
		determine the time offset and also used as start year in DataIO.
		It is set in parseHeader()
	*/
	IBK::Time m_startYear;


	/*! Error string, shall be set when any of the functions returns false.
		Shall be a translated string since it will be shown in the user interface.
	*/
	QString	m_lastError;
};


} // namespace VICUS

#endif // VICUS_BTFREADERH
