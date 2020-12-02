#include "EP_Material.h"

#include <IBK_StringUtils.h>
#include <IBK_math.h>

#include "EP_Version.h"

namespace EP {

void Material::read(const std::vector<std::string> & str, unsigned int version)
{
	const char * const FUNC_ID = "[Material::read]";
	std::vector<std::string> str1;

	//for version 8.3
	if(version != EP::Version::VN_8_3)
		return;

	str1 = str;
	if(str1.size()!= 10)
		str1.resize(10,"");


	m_name = str1[1];

	if(IBK::tolower_string(str1[2]) == "veryrough")
		m_roughness = Roughness::R_VeryRough;
	else if(IBK::tolower_string(str1[2]) == "rough")
		m_roughness = Roughness::R_Rough;
	else if(IBK::tolower_string(str1[2]) == "mediumrough")
		m_roughness = Roughness::R_MediumRough;
	else if(IBK::tolower_string(str1[2]) == "verysmooth")
		m_roughness = Roughness::R_VerySmooth;
	else if(IBK::tolower_string(str1[2]) == "smooth")
		m_roughness = Roughness::R_Smooth;
	else if(IBK::tolower_string(str1[2]) == "mediumsmooth")
		m_roughness = Roughness::R_MediumSmooth;
	else {
		m_roughness = Roughness::R_MediumRough;
		//warnung rausgeben
	}

	if(IBK::tolower_string(str1[0]) == "material"){
		m_thickness = IBK::string2val<double>(str1[3]);
		m_conductivity = IBK::string2val<double>(str1[4]);
		m_density = IBK::string2val<double>(str1[5]);
		m_specHeatCapa = IBK::string2val<double>(str1[6]);

		m_thermalAbsorptance = IBK::string2valDef(str1[7],0.9);
		m_solarAbsorptance = IBK::string2valDef(str1[8],0.6);
		m_visibleAbsorptance = IBK::string2valDef(str1[9],0.6);
	}
	else if(IBK::tolower_string(str1[0]) == "material:nomass"){
		double resistance = IBK::string2val<double>(str[3]);
		if(resistance<=0)
			IBK::Exception(IBK::FormatString("Resistance is not valid. Material name '%1'").arg(m_name), FUNC_ID);
		m_thickness = 0.1;
		m_specHeatCapa = 1000;
		m_density = 1.2;
		m_conductivity = m_thickness/resistance;

		m_thermalAbsorptance = IBK::string2valDef(str1[4],0.9);
		m_solarAbsorptance = IBK::string2valDef(str1[5],0.6);
		m_visibleAbsorptance = IBK::string2valDef(str1[6],0.6);
	}
}

void Material::write(std::string &outStr, unsigned int version) const
{
	if(version != EP::Version::VN_8_3)
		return;

	std::stringstream ss;
	ss << "Material," << std::endl;
	ss << m_name << "," << std::endl;

	switch (m_roughness) {
		case R_Rough: ss << "Rough" << "," << std::endl;				break;
		case R_Smooth: ss << "Smooth" << "," << std::endl;				break;
		case R_VeryRough: ss << "VeryRough" << "," << std::endl;		break;
		case R_VerySmooth: ss << "VerySmooth" << "," << std::endl;		break;
		case R_MediumRough: ss << "MediumRough" << "," << std::endl;	break;
		case R_MediumSmooth: ss << "MediumSmooth" << "," << std::endl;	break;
		case NUM_R: return;
	}

	ss << m_thickness << "," << std::endl;
	ss << m_conductivity << "," << std::endl;
	ss << m_density << "," << std::endl;
	ss << m_specHeatCapa << "," << std::endl;
	ss << m_thermalAbsorptance << "," << std::endl;
	ss << m_solarAbsorptance << "," << std::endl;
	ss << m_visibleAbsorptance << ";" << std::endl;

	outStr += ss.str();
}

bool Material::behavesLike(const Material & other) const
{

	if(!IBK::nearly_equal<4>(m_conductivity, other.m_conductivity))
		return false;
	if(!IBK::nearly_equal<1>(m_density, other.m_density))
		return false;
	if(!IBK::nearly_equal<0>(m_specHeatCapa, other.m_specHeatCapa))
		return false;
	if(!IBK::nearly_equal<4>(m_thickness, other.m_thickness))
		return  false;
	if(!IBK::nearly_equal<1>(m_solarAbsorptance, other.m_solarAbsorptance))
		return  false;
	if(!IBK::nearly_equal<1>(m_thermalAbsorptance, other.m_thermalAbsorptance))
		return  false;
	if(!IBK::nearly_equal<1>(m_visibleAbsorptance, other.m_visibleAbsorptance))
		return  false;

	return true;
}

bool Material::operator==(const Material & other) const
{
	if(!IBK::nearly_equal<4>(m_conductivity, other.m_conductivity))
		return false;
	if(!IBK::nearly_equal<1>(m_density, other.m_density))
		return false;
	if(!IBK::nearly_equal<0>(m_specHeatCapa, other.m_specHeatCapa))
		return false;
	if(!IBK::nearly_equal<4>(m_thickness, other.m_thickness))
		return  false;

	return true;
}



}
