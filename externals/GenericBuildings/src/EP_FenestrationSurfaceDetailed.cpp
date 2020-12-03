#include "EP_FenestrationSurfaceDetailed.h"

#include <IBK_StringUtils.h>

#include "EP_Version.h"

namespace EP {

void FenestrationSurfaceDetailed::read(const std::vector<std::string> & str, unsigned int version)
{

	//for version 8.3
//	if(version != EP::Version::VN_8_3)
//		return;


	m_name = str[1];

	if(IBK::tolower_string(str[2]) == "window")
		m_surfaceType = SurfaceType::ST_Window;
	else if(IBK::tolower_string(str[2]) == "door")
		m_surfaceType = SurfaceType::ST_Door;
	else if(IBK::tolower_string(str[2]) == "glassdoor")
		m_surfaceType = SurfaceType::ST_GlassDoor;
	else {
		m_surfaceType = SurfaceType::ST_Window;
		//warnung rausgeben
	}

	m_constructionName = str[3];
	m_bsdName = str[4];

	m_outsideBoundaryConditionObject = str[5];

	if(IBK::tolower_string(str[6]) == "autocalculate")
		m_viewFactorToGround = -1;
	else
		m_viewFactorToGround = IBK::string2val<double>(str[6]);

	m_shadingControlName = str[7];
	m_frameAndDividerName = str[8];

	//multiplier ignored
	//number of vertex ignored

	for (size_t i=11; i<str.size(); ) {
		IBKMK::Vector3D vec;
		vec.m_x = IBK::string2val<double>(str[i]);	++i;
		vec.m_y = IBK::string2val<double>(str[i]);	++i;
		vec.m_z = IBK::string2val<double>(str[i]);	++i;
		m_polyline.push_back(vec);
	}


}

void FenestrationSurfaceDetailed::write(std::string &outStr, unsigned int version) const
{
	if(version != EP::Version::VN_8_3)
		return;

	std::stringstream ss;
	ss << "FenestrationSurface:Detailed," << std::endl;
	ss << m_name << "," << std::endl;

	switch (m_surfaceType) {
		case ST_Window:			ss << "Window" << "," << std::endl;			break;
		case ST_Door:			ss << "Door" << "," << std::endl;			break;
		case ST_GlassDoor:		ss << "GlassDoor" << "," << std::endl;		break;
		default:				ss<< "," << std::endl;						break;
	}

	ss << m_constructionName << "," << std::endl;
	ss << m_bsdName << "," << std::endl;
	ss << m_outsideBoundaryConditionObject << "," << std::endl;
	ss << m_viewFactorToGround << "," << std::endl;
	ss << m_shadingControlName << "," << std::endl;
	ss << m_frameAndDividerName << "," << std::endl;
	ss << "1" << "," << std::endl;                                                                  //Multiplier
	ss << m_polyline.size() << "," << std::endl;
	for (size_t i=0; i<m_polyline.size(); ++i) {
		ss << m_polyline[i].m_x << "," << m_polyline[i].m_y << "," << m_polyline[i].m_z;
		if(i== m_polyline.size()-1)
			ss << ";" << std::endl<< std::endl;
		else
			ss << ","<< std::endl ;
	}

	ss << std::endl;
	outStr += ss.str();

}

void FenestrationSurfaceDetailed::calcViewFactorToGround()
{
//	NSG::Polygon poly(m_polyline);
//	IBKMK::Vector3D normal = poly.calcNormal();
//	m_viewFactorToGround = (1 - normal.m_z)*0.5;
}

}


