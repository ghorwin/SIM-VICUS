#include "EP_Construction.h"


#include <IBK_StringUtils.h>

#include "EP_Material.h"
#include "EP_WindowMaterial.h"
#include "EP_Version.h"

namespace EP {

void Construction::read(const std::vector<std::string> & str, unsigned int version)
{
//	const char * const FUNC_ID = "[Construction::read]";


	//for version 8.3
//	if(version != EP::Version::VN_8_3)
//		return;

	m_name = str[1];
	m_layers.clear();
	for (size_t i=2; i<str.size(); ++i) {
		m_layers.push_back(str[i]);
	}
}

void Construction::write(std::string &outStr, unsigned int version) const
{
	if(version != EP::Version::VN_8_3)
		return;

	if(m_layers.empty())
		return;

	std::stringstream ss;
	ss << "Construction," << std::endl;
	ss << m_name << "," << std::endl;                                                                //first layer=outside layer
	for (size_t i=0; i<m_layers.size(); ++i) {
		ss << m_layers[i];
		if(i== m_layers.size()-1)
			ss << ";" << std::endl<< std::endl;
		else
			ss << ","<< std::endl ;
	}


	outStr += ss.str();

}

bool Construction::behavesLike(const Construction & other, const std::vector<Material> & mats, const std::vector<Material> otherMats,
							   const std::vector<WindowMaterial> & winMats, const std::vector<WindowMaterial> otherWinMats) const
{
	if(m_layers.size() != other.m_layers.size())
		return false;

	std::map<std::string, Material> matsMap, otherMatsMap;
	std::map<std::string, WindowMaterial> winMatsMap, otherWinMatsMap;

	//build maps for better access
	for(const Material &mat : mats)
		matsMap[mat.m_name] = mat;
	for(const Material &mat : otherMats)
		otherMatsMap[mat.m_name] = mat;
	for(const WindowMaterial &mat : winMats)
		winMatsMap[mat.m_name] = mat;
	for(const WindowMaterial &mat : otherWinMats)
		otherWinMatsMap[mat.m_name] = mat;


	for (size_t i=0; i<other.m_layers.size(); ++i) {

		bool isWindowMaterial = !(matsMap.find(m_layers[i]) != matsMap.end());
		bool otherIsWindowMaterial = !(otherMatsMap.find(other.m_layers[i]) != otherMatsMap.end());

		//material have different categories
		if(isWindowMaterial != otherIsWindowMaterial)
			return false;

		if(!isWindowMaterial){
			if(!matsMap[m_layers[i]].behavesLike(otherMatsMap[other.m_layers[i]]))
				return false;
		}
		else {
			if(!winMatsMap[m_layers[i]].behavesLike(otherWinMatsMap[other.m_layers[i]]))
				return false;
		}
	}

	return true;

}


}
