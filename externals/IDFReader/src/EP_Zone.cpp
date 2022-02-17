#include "EP_Zone.h"

#include <IBK_StringUtils.h>
#include "EP_Version.h"

namespace EP {

void Zone::read(const std::vector<std::string> & str, unsigned int /*version*/) {
	//for version 8.3
//	if(version != EP::Version::VN_8_3)
//		return;

	m_name = str[1];
	/* ignore elements:
	Direction of North
	X,Y,Z Origin
	Type
	Multiplier
	*/
	if (!str[8].empty() && IBK::tolower_string(str[8]) != "autocalculate") {
		try {
			m_ceilingHeight = IBK::string2val<double>(str[8]);
		} catch (...) {
			m_ceilingHeight = -1; // optional
		}
	}
	else
		m_ceilingHeight = -1;

	if (!str[9].empty() && IBK::tolower_string(str[9]) != "autocalculate")
		m_volume = IBK::string2val<double>(str[9]);
	else
		m_volume = -1;

	if (!str[10].empty() && IBK::tolower_string(str[10]) != "autocalculate")
		m_floorArea = IBK::string2val<double>(str[10]);
	else
		m_floorArea = -1;

	//ignore other elements
}


void Zone::write(std::string &outStr, unsigned int version) const {

	if(version != EP::Version::VN_8_3)
		return;

	std::stringstream ss;
	ss << "Zone," << std::endl;
	ss << m_name << "," << std::endl;
	ss << "0" << "," << std::endl;
	ss << "0" << "," << std::endl;
	ss << "0" << "," << std::endl;
	ss << "0" << "," << std::endl;
	ss << "1" << "," << std::endl;                  //Type
	ss << "1" << "," << std::endl;                  //Multiplier

	ss << m_ceilingHeight << "," << std::endl;
	ss << m_volume << "," << std::endl;
	ss << m_floorArea<< "," << std::endl;
	ss << "" << "," << std::endl;
	ss << "" << "," << std::endl;
	ss << "Yes" << ";" << std::endl<< std::endl;

	outStr += ss.str();

}

} // namespace EP
