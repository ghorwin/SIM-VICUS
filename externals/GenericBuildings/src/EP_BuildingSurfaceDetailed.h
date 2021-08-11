#ifndef EP_BuildingSurfaceDetailedH
#define EP_BuildingSurfaceDetailedH


#include <IBKMK_Vector3D.h>

namespace EP {


class BuildingSurfaceDetailed
{
public:

	enum SurfaceType{
		ST_Floor,
		ST_Wall,
		ST_Ceiling,
		ST_Roof,
		NUM_ST
	};

	enum OutsideBoundaryCondition{
		OC_Surface,
		OC_Outdoors,
		OC_Ground,
		OC_Adiabatic,
		NUM_OC
	};

	/*! Set surface conditions to outside.*/
	void setSurfaceConditions(const OutsideBoundaryCondition &obc, const std::string &otherBSDName = "");

	// *** PUBLIC MEMBER FUNCTIONS ***
	/*! Read "BuildingSurface:Detailed" section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Calculates the view factor to ground from polyline. */
	void calcViewFactorToGround();

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;

	// *** PUBLIC MEMBER VARIABLES ***
	std::string						m_name;
	SurfaceType						m_surfaceType;
	std::string						m_constructionName;
	std::string						m_zoneName;
	OutsideBoundaryCondition		m_outsideBoundaryCondition;
	std::string						m_outsideBoundaryConditionObject;
	bool							m_sunExposed;
	bool							m_windExposed;
	double							m_viewFactorToGround;
	std::vector<IBKMK::Vector3D>	m_polyline;

};
}

#endif // EP_BuildingSurfaceDetailedH
