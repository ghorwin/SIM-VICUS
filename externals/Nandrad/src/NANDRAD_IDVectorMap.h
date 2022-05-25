#ifndef NANDRAD_IDVECTORMAPH
#define NANDRAD_IDVECTORMAPH

#include <map>
#include <vector>
#include <string>

namespace NANDRAD {

/*! Data structure that contains a map, where keys are ids and values are vectors of ids */
class IDVectorMap
{
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
		Throws an IBK::Exception, if number of rows in columns mismatches.

		\note It is possible to use , and a whitespace (space or tab) character as number separator.
	*/
	void setEncodedString(const std::string & str);

	/*! Returns content of data table as encoded string using tab a value separator. */
	std::string encodedString() const;


	/*! The actual data member. */
	std::map<unsigned int, std::vector<unsigned int> >		m_values;
};

} // namespace NANDRAD

#endif // NANDRAD_IDVECTORMAPH
