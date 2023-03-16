#include "EP_Frame.h"

#include <IBK_StringUtils.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include "EP_Version.h"

namespace EP {

void Frame::read(const std::vector<std::string> & str, unsigned int /*version*/) {
//	const char * const FUNC_ID = "[Frame::read]";
	std::vector<std::string> str1(str);

	// for version 8.3
//	if(version != EP::Version::VN_8_3)
//		return;

	if (str1.size() <17)
		str1.resize(17,"");

	m_name = str1[1];
	m_widthFrame = IBK::string2valDef<double>(str1[2],0);
	// idx = 3 -> outside projection
	// idx = 4 -> inside projection
	m_conductanceFrame = IBK::string2valDef<double>(str1[5],0);
	// idx = 6 -> frame-edge glass conductance
	// idx = 7 -> solar absorptance
	// idx = 8 -> visible absorptance
	// idx = 9 -> thermal Hemispherical Emissivity
	// idx = 10 -> Divider Type

	//Divider
	m_widthDivider = IBK::string2valDef<double>(str1[11],0);
	// idx = 12 -> Number of Horizontal Dividers
	// idx = 13 -> Number of Vertical Dividers
	// idx = 14 -> outside projection
	// idx = 15 -> inside projection
	m_conductanceDivider = IBK::string2valDef<double>(str1[16],0);
}


void Frame::write(std::string &outStr, unsigned int version) const {
	IBK_ASSERT(version == EP::Version::VN_8_3);

	std::stringstream ss;
	ss << "Frame," << std::endl;
	ss << m_name << "," << std::endl;

	ss << IBK::val2string(m_widthFrame) << "," << std::endl;

	for(unsigned int i=0; i<2; ++i)
		ss << "," << std::endl;

	ss << IBK::val2string(m_conductanceFrame) << "," << std::endl;
	for(unsigned int i=0; i<4; ++i)
		ss << "," << std::endl;

	ss << "DividedLite" << "," << std::endl;
	ss << IBK::val2string(m_widthDivider) << "," << std::endl;
	for(unsigned int i=0; i<4; ++i)
		ss << "," << std::endl;
	ss << IBK::val2string(m_conductanceDivider) << "," << std::endl;

	outStr += ss.str();
}

bool Frame::behavesLike(const Frame & other) const {

	if(!IBK::nearly_equal<4>(m_widthFrame, other.m_widthFrame))
		return false;
	if(!IBK::nearly_equal<4>(m_conductanceFrame, other.m_conductanceFrame))
		return false;
	if(!IBK::nearly_equal<4>(m_conductanceDivider, other.m_conductanceDivider))
		return false;
	if(!IBK::nearly_equal<4>(m_widthDivider, other.m_widthDivider))
		return false;

	return true;
}

bool Frame::operator==(const Frame & other) const {
	return behavesLike(other);
}



}
