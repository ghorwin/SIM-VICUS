#include "EP_ShadingBuildingDetailed.h"

#include <IBK_StringUtils.h>
#include <IBK_math.h>

#include "EP_Version.h"

namespace EP {

void ShadingBuildingDetailed::read(const std::vector<std::string> & str, unsigned int /*version*/) {
	FUNCID(ShadingBuildingDetailed::read);

	//for version 8.3
//	if (version != EP::Version::VN_8_3)
//		throw IBK::Exception("Only version 8.3 supported.", FUNC_ID);


	m_name = str[1];

	unsigned int count = IBK::string2val<unsigned int>(str[3]);
	// polyline starts at index 4
	for (size_t i=4; i+2<str.size(); ) {
		IBKMK::Vector3D vec;
		vec.m_x = IBK::rounded<6>(IBK::string2val<double>(str[i]));	++i;
		vec.m_y = IBK::rounded<6>(IBK::string2val<double>(str[i]));	++i;
		vec.m_z = IBK::rounded<6>(IBK::string2val<double>(str[i]));	++i;
		m_polyline.push_back(vec);
	}

	// check for correct number of points
	if (count != m_polyline.size())
		throw IBK::Exception("Invalid polygon points in Shading:Building:detailed.", FUNC_ID);
}


void ShadingBuildingDetailed::write(std::string & /*outStr*/, unsigned int /*version*/) const {
	// TODO???
}

} // namespace EP
