#include "EP_ShadingSystems.h"

#include <IBK_StringUtils.h>
#include <IBK_math.h>
#include <IBK_messages.h>

#include "EP_Version.h"

namespace EP {

void ShadingSystems::read(const std::vector<std::string> & str, unsigned int /*version*/) {
//	const char * const FUNC_ID = "[ShadingSystems::read]";
	std::vector<std::string> str1(str);

	// for version 8.3
//	if(version != EP::Version::VN_8_3)
//		return;

	str1 = str;
	if (str1.size() < 3)
		str1.resize(30,"");

	if(IBK::tolower_string(str[0]) == "windowmaterial:shade")
		m_type = Shade;
	else if(IBK::tolower_string(str[0]) == "windowmaterial:screen")
		m_type = Screen;
	else if(IBK::tolower_string(str[0]) == "windowmaterial:blind")
		m_type = Blind;
	else
		m_type = NUM_T;

	m_name = str1[1];

	switch (m_type) {
		case EP::ShadingSystems::Blind:{
			if(IBK::tolower_string(str[2]) == "vertical")
				m_slatOrientation = Vertical;

			//slat width
			//slat seperation
			//slat thickness
			//slat angle
			if(str.size()>8)
				m_conductivity = IBK::string2valDef<double>(str[7],0);	//slat conductivity

			//slat beam solar transmittance
			//slat front side beam solar reflectance
			//slat back side beam solar reflectance
			//slat diffuse solar transmittance
			//slat front side diffuse solar reflectance
			//slat back side diffuse solar reflectance

			//slat beam visible transmittance
			//slat front side beam visible reflectance
			//slat back side beam visible reflectance
			//slat diffuse visible transmittance
			//slat front side diffuse visible reflectance
			//slat back side diffuse visible reflectance

			//slat infrared hemisphere transmittance
			//slat front side slat infrared hemisphere emissivity
			//slat back side slat infrared hemisphere emissivity

			if(str.size()>22)
				m_shadeToGlassDistance = IBK::string2valDef<double>(str[21],0);//blind to glass distance


			//slat beam visible front solar reflectance
			//slat beam visible back solar reflectance
			//slat diffuse transmittance
			//slat diffuse front solar reflectance
			//slat diffuse back solar reflectance
		}
		break;
		case EP::ShadingSystems::Shade:{
			//Solar Transmittance
			//Solar Reflectance
			//Visible Transmittance
			//Visible Reflectance
			//Infrared Hemispherical Emissivity
			//Infrared Transmittance
			//Thickness
			if(str.size()>10)
				m_conductivity = IBK::string2valDef<double>(str[9],0);	//conductivity

			//Screen Material Spacing
			//Screen Material Diameter
			if(str.size()>11)
				m_shadeToGlassDistance = IBK::string2valDef<double>(str[10],0);//Shade to Glass Distance

		}
		break;
		case EP::ShadingSystems::Screen:{
			//Reflected Beam Transmittance Accounting Method
			//Diffuse Solar Reflectance
			//Diffuse Visible Reflectance
			//Thermal Hemispherical Emissivity
			if(str.size()>7)
				m_conductivity = IBK::string2valDef<double>(str[6],0);	//slat conductivity

			//Screen Material Spacing
			//Screen Material Diameter
			if(str.size()>10)
				m_shadeToGlassDistance = IBK::string2valDef<double>(str[9],0);//Screen to Glass Distance
			}
		break;
		case EP::ShadingSystems::NUM_T:
		break;

	}
}


void ShadingSystems::write(std::string &outStr, unsigned int /*version*/) const {

	std::stringstream ss;

	switch (m_type) {
		case Blind:{
			ss << "WindowMaterial:Blind," << std::endl;
			ss << m_name << "," << std::endl;
			ss << (m_slatOrientation == Horizontal ? "Horizontal" : "Vertical") << "," << std::endl;
			ss << "0.01," << std::endl;
			ss << "0.01," << std::endl;
			ss << "0.00025," << std::endl;
			ss << "45," << std::endl;
			ss << IBK::val2string(m_conductivity) << "," << std::endl;
			ss << "0," << std::endl;
			ss << "0.2," << std::endl;
			ss << "0.2," << std::endl;
			ss << "0," << std::endl;
			ss << "0.2," << std::endl;
			ss << "0.2," << std::endl;
			ss << "0," << std::endl;
			ss << "0," << std::endl;
			ss << "0," << std::endl;
			ss << "0," << std::endl;
			ss << "0," << std::endl;
			ss << "0," << std::endl;
			ss << "0," << std::endl;
			ss << "0.9," << std::endl;
			ss << "0.9," << std::endl;

		}
		break;
		case Screen:{
			ss << "WindowMaterial:Screen," << std::endl;
			ss << m_name << "," << std::endl;
			ss << "DoNotModel," << std::endl;
			ss << "0.1," << std::endl;
			ss << "0," << std::endl;
			ss << "0.9," << std::endl;
			ss << IBK::val2string(m_conductivity) << "," << std::endl;
			ss << "0.001," << std::endl;
			ss << "0.00001," << std::endl;
			//ss << IBK::val2string(m_shadeToGlassDistance) << "," << std::endl;

		}
		break;
		case Shade: {
			ss << "WindowMaterial:Shade," << std::endl;
			ss << m_name << "," << std::endl;
			ss << "0.6," << std::endl;
			ss << "0.4," << std::endl;
			ss << "0.8," << std::endl;
			ss << "0," << std::endl;
			ss << "0.9," << std::endl;
			ss << "0.01," << std::endl;
			ss << IBK::val2string(m_conductivity) << "," << std::endl;
		}
		break;
		case NUM_T:
			return;
	}
	ss << IBK::val2string(m_shadeToGlassDistance) << "," << std::endl;
	ss << IBK::val2string(m_topOpening) << "," << std::endl;
	ss << IBK::val2string(m_bottomOpening) << "," << std::endl;
	ss << IBK::val2string(m_leftOpening) << "," << std::endl;
	ss << IBK::val2string(m_rightOpening) << ";" << std::endl;

	outStr += ss.str();
}

bool ShadingSystems::behavesLike(const ShadingSystems & other) const
{
	if(m_type != other.m_type)
		return false;

	return true;
}

bool ShadingSystems::operator==(const ShadingSystems & other) const
{
	if(m_type != other.m_type)
		return false;
	if(!IBK::nearly_equal<4>(m_conductivity, other.m_conductivity))
		return false;
	if(!IBK::nearly_equal<1>(m_topOpening, other.m_topOpening))
		return false;
	if(!IBK::nearly_equal<0>(m_bottomOpening, other.m_bottomOpening))
		return false;
	if(!IBK::nearly_equal<4>(m_leftOpening, other.m_leftOpening))
		return  false;
	if(!IBK::nearly_equal<1>(m_rightOpening, other.m_rightOpening))
		return  false;
	if(!IBK::nearly_equal<1>(m_shadeToGlassDistance, other.m_shadeToGlassDistance))
		return  false;
	if(m_type == Blind && m_slatOrientation != other.m_slatOrientation)
		return  false;

	return true;
}



}
