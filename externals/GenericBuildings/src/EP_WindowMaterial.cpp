#include "EP_WindowMaterial.h"

#include <IBK_StringUtils.h>
#include <IBK_math.h>

#include "EP_Version.h"

namespace EP {

void WindowMaterial::read(const std::vector<std::string> & str, unsigned int version)
{
	const char * const FUNC_ID = "[WindowMaterial::read]";
	std::vector<std::string> str1;

//	//for version 8.3
//	if(version != EP::Version::VN_8_3)
//		return;

	str1 = str;
	if(str1.size()!= 5)
		str1.resize(5,"");


	m_name = str1[1];

	m_uValue = IBK::string2val<double>(str1[2]);
	m_SHGC = IBK::string2val<double>(str1[3]);
	m_visibleTransmittance = IBK::string2valDef<double>(str1[4],0.8);

}

void WindowMaterial::write(std::string &outStr, unsigned int version) const
{

	if(version != EP::Version::VN_8_3)
		return;

	std::stringstream ss;
	ss << "WindowMaterial:SimpleGlazingSystem," << std::endl;
	ss << m_name << "," << std::endl;
	ss << m_uValue << "," << std::endl;
	ss << m_SHGC << "," << std::endl;
	ss << m_visibleTransmittance << ";" << std::endl;


	outStr += ss.str();

}

bool WindowMaterial::behavesLike(const WindowMaterial & other) const
{
	if(!IBK::near_equal(m_uValue, other.m_uValue))
		return false;

	if(!IBK::near_equal(m_SHGC, other.m_SHGC))
		return false;

	if(!IBK::near_equal(m_visibleTransmittance, other.m_visibleTransmittance))
		return false;

	return true;
}


}
