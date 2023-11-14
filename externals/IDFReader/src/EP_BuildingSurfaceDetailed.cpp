#include "EP_BuildingSurfaceDetailed.h"

#include <IBK_StringUtils.h>
#include <IBK_math.h>

#include "EP_Version.h"

namespace EP {

void BuildingSurfaceDetailed::setSurfaceConditions(const OutsideBoundaryCondition &obc, const std::string &otherBSDName){
	m_outsideBoundaryCondition = obc;

	switch (obc) {
		case OC_Outdoors:	m_windExposed=true;		m_sunExposed=true;		break;
		case OC_Ground:
		case OC_Adiabatic:	m_windExposed=false;	m_sunExposed=false;		break;
		case OC_Surface:	m_windExposed=false;	m_sunExposed=false;		m_outsideBoundaryConditionObject= otherBSDName; break;

		case NUM_OC:	m_windExposed=true;		m_sunExposed=true;
			std::cout<< "Error in setSurfaceCondition. Invalid object: " << m_name << std::endl;
			break;
	}
}


void BuildingSurfaceDetailed::read(const std::vector<std::string> & str, unsigned int /*version*/) {
//	FUNCID(BuildingSurfaceDetailed::read);

	//for version 8.3
//	if (version != EP::Version::VN_8_3)
//		throw IBK::Exception("Only version 8.3 supported.", FUNC_ID);

	//  Example for two bsd that are interlinkd (i.e. inside wall)
	//  Both reference the same construction (which is symmetric here),
	//  and denote each other via "Outside Boundary Condition Object" ID name.
	//
	//	BuildingSurface:Detailed,
	//	  Zn001:Flr001W,           !- Name
	//	  Floor,                   !- Surface Type
	//	  Plenum Floor,            !- Construction Name
	//	  West Zone,               !- Zone Name
	//	  Surface,                 !- Outside Boundary Condition
	//	  PZn001:CeilingW,         !- Outside Boundary Condition Object
	//	  NoSun,                   !- Sun Exposure
	//	  NoWind,                  !- Wind Exposure
	//	  1.000000,                !- View Factor to Ground
	//	  4,                       !- Number of Vertices
	//	  15.240,0.000000,0.50,  !- X,Y,Z ==> Vertex 1 {m}
	//	  0.0000,0.000000,0.50,  !- X,Y,Z ==> Vertex 2 {m}
	//	  0.0000,15.24000,0.50,  !- X,Y,Z ==> Vertex 3 {m}
	//	  15.240,15.24000,0.50;  !- X,Y,Z ==> Vertex 4 {m}

	//	BuildingSurface:Detailed,
	//	  PZn001:CeilingW,         !- Name
	//	  Roof,                    !- Surface Type
	//	  Plenum Floor,            !- Construction Name
	//	  West Plenum,             !- Zone Name
	//	  Surface,                 !- Outside Boundary Condition
	//	  Zn001:Flr001W,           !- Outside Boundary Condition Object
	//	  NoSun,                   !- Sun Exposure
	//	  NoWind,                  !- Wind Exposure
	//	  1.000000,                !- View Factor to Ground
	//	  4,                       !- Number of Vertices
	//	  15.240,0.000000,0.50,  !- X,Y,Z ==> Vertex 1 {m}
	//	  15.240,15.24000,0.50,  !- X,Y,Z ==> Vertex 2 {m}
	//	  0.0000,15.24000,0.50,  !- X,Y,Z ==> Vertex 3 {m}
	//	  0.0000,0.000000,0.50;  !- X,Y,Z ==> Vertex 4 {m}

	m_name = str[1];

	if(IBK::tolower_string(str[2]) == "roof")
		m_surfaceType = SurfaceType::ST_Roof;
	else if(IBK::tolower_string(str[2]) == "floor")
		m_surfaceType = SurfaceType::ST_Floor;
	else if(IBK::tolower_string(str[2]) == "ceiling")
		m_surfaceType = SurfaceType::ST_Ceiling;
	else if(IBK::tolower_string(str[2]) == "wall")
		m_surfaceType = SurfaceType::ST_Wall;
	else {
		m_surfaceType = SurfaceType::ST_Wall;
		//warnung rausgeben
	}

	m_constructionName = str[3];
	m_zoneName = str[4];

	if(IBK::tolower_string(str[5]) == "adiabatic")
		m_outsideBoundaryCondition = OutsideBoundaryCondition::OC_Adiabatic;
	else if(IBK::tolower_string(str[5]) == "ground")
		m_outsideBoundaryCondition = OutsideBoundaryCondition::OC_Ground;
	else if(IBK::tolower_string(str[5]) == "surface")
		m_outsideBoundaryCondition = OutsideBoundaryCondition::OC_Surface;
	else if(IBK::tolower_string(str[5]) == "outdoors")
		m_outsideBoundaryCondition = OutsideBoundaryCondition::OC_Outdoors;
	else {
		m_outsideBoundaryCondition = OutsideBoundaryCondition::OC_Outdoors;
		//warnung
	}

	m_outsideBoundaryConditionObject = str[6];

	if(IBK::tolower_string(str[7]) == "sunexposed")
		m_sunExposed = true;
	else
		m_sunExposed = false;

	if(IBK::tolower_string(str[8]) == "windexposed")
		m_windExposed = true;
	else
		m_windExposed = false;

	if(IBK::tolower_string(str[9]) == "autocalculate")
		m_viewFactorToGround = -1;
	else
		m_viewFactorToGround = IBK::string2val<double>(str[9]);

	//ignored number of points str[10]

	//rounding
	for (size_t i=11; i+2<str.size(); ) {
		IBKMK::Vector3D vec;
		vec.m_x = IBK::rounded<6>(IBK::string2val<double>(str[i]));	++i;
		vec.m_y = IBK::rounded<6>(IBK::string2val<double>(str[i]));	++i;
		vec.m_z = IBK::rounded<6>(IBK::string2val<double>(str[i]));	++i;
		m_polyline.push_back(vec);
	}
}


void BuildingSurfaceDetailed::calcViewFactorToGround() {
	//NSG::Polygon poly(m_polyline);
	std::vector<IBKMK::Vector3D> polygon;
	if (polygon.size() < 3){
		m_viewFactorToGround = -1;
		return;
	}

	IBKMK::Vector3D n(0,0,0);
	IBKMK::Vector3D e(0,0,0);

	for(unsigned int i=0; i<polygon.size(); ++i)
		e += polygon[i];

	e /= polygon.size();


	for(unsigned int i=0; i<polygon.size(); ++i) {
		unsigned int s = polygon.size();
		unsigned int j = (i + s - 1)%s;

		IBKMK::Vector3D v1 = polygon[j] - e;
		IBKMK::Vector3D v2 = polygon[i] - e;

		n += v1.crossProduct(v2).normalized();
	}

	if (n.magnitudeSquared() < 0.01) {

		n = IBKMK::Vector3D(0,0,0);

		for(unsigned int i=0; i<polygon.size()-1; ++i) {
			unsigned int s = polygon.size();
			unsigned int j = (i + s - 1)%s;

			IBKMK::Vector3D v1 = polygon[j] - e;
			IBKMK::Vector3D v2 = polygon[i] - e;

			n += v1.crossProduct(v2).normalized();
		}

		if (n.magnitudeSquared() > 0.01)
			n.normalized();
			// return;

//		IBK::IBK_Message(IBK::FormatString("Start point:\t%1\t%2\t%3")
//						 .arg(e.m_x)
//						 .arg(e.m_y)
//						 .arg(e.m_z), IBK::MSG_ERROR);

		// falls wir hier rein kommen ist was kaputt
		if(false){
			for (unsigned int i=0; i<polygon.size(); ++i) {

				unsigned int s = polygon.size();
				unsigned int j = (i + s -1)%s;

				IBKMK::Vector3D v1 = polygon[j] - e;
				IBKMK::Vector3D v2 = polygon[i] - e;

				IBKMK::Vector3D ntest = v1.crossProduct(v2).normalized();

	//			IBK::IBK_Message(IBK::FormatString("Poly point %4:\t%1\t%2\t%3\t\tNormal:\t%5\t%6\t%7")
	//							 .arg(polygon[i].m_x)
	//							 .arg(polygon[i].m_y)
	//							 .arg(polygon[i].m_z)
	//							 .arg(i)
	//							 .arg(ntest.m_x)
	//							 .arg(ntest.m_y)
	//							 .arg(ntest.m_z),IBK::MSG_ERROR);
			}

		}

		//throw IBK::Exception(IBK::FormatString("Could not determine normal of polygon 3D."), FUNC_ID);
	}

	n.normalized();
	m_viewFactorToGround = (1 - n.m_z)*0.5;
}

void BuildingSurfaceDetailed::write(std::string & outStr, unsigned int version) const {

	if(m_polyline.size()<3)
		return;

	if(version != EP::Version::VN_8_3)
		return;

	std::stringstream ss;
	ss << "BuildingSurface:Detailed," << std::endl;
	ss << m_name << "," << std::endl;
	switch (m_surfaceType) {
		case ST_Roof: ss << "Roof" << "," << std::endl; break;
		case ST_Wall: ss << "Wall" << "," << std::endl; break;
		case ST_Ceiling: ss << "Ceiling" << "," << std::endl; break;
		case ST_Floor: ss << "Floor" << "," << std::endl; break;
		default: ss << ","; break;
	}
	ss << m_constructionName << "," << std::endl;
	ss << m_zoneName << "," << std::endl;
	switch (m_outsideBoundaryCondition) {
		case OC_Ground: ss << "Ground" << ",," << std::endl; break;
		case OC_Adiabatic: ss << "Adiabatic" << ",," << std::endl; break;
		case OC_Outdoors: ss << "Outdoors" << ",," << std::endl; break;
		case OC_Surface: ss << "Surface" << "," << m_outsideBoundaryConditionObject << "," << std::endl; break;
		case NUM_OC: ss << "Outdoors" << ",," << std::endl; break; //return;	Warnung ausgeben
		default: ss << "Outdoors" << ",," << std::endl; break;
	}

	if(m_sunExposed)
		ss << "SunExposed" << "," << std::endl;
	else
		ss << "NoSun" << "," << std::endl;

	if(m_windExposed)
		ss << "WindExposed" << "," << std::endl;
	else
		ss << "NoWind" << "," << std::endl;

	ss << m_viewFactorToGround << "," << std::endl;
	ss << m_polyline.size() << "," << std::endl;
	for (size_t i=0; i<m_polyline.size(); ++i) {

		ss << IBK::val2string(m_polyline[i].m_x, 10, 10, ' ') << ","
		   << IBK::val2string(m_polyline[i].m_y, 10, 10, ' ') << ","
		   << IBK::val2string(m_polyline[i].m_z, 10, 10, ' ');
		if(i== m_polyline.size()-1)
			ss << ";" << std::endl<< std::endl;
		else
			ss << ","<< std::endl ;
	}
	outStr += ss.str();
}


}
