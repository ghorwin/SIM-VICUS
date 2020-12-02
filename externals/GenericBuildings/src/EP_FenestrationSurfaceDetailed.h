#ifndef EP_FENESTRATIONSURFACEDETAILED_H
#define EP_FENESTRATIONSURFACEDETAILED_H

#include <IBKMK_Vector3D.h>

namespace EP {

class FenestrationSurfaceDetailed
{
public:

	enum SurfaceType{
		ST_Door,
		ST_GlassDoor,
		ST_Window,
		NUM_ST
	};

	enum OutsideBoundaryCondition{
		OC_Surface,
		OC_Outdoors,
		OC_Ground,
		OC_Adiabatic,
		NUM_OC
	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Read "FenestrationSurface:Detailed" section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;

	/*! Calculates the view factor to ground from polyline. */
	void calcViewFactorToGround();

	// *** PUBLIC MEMBER VARIABLES ***

	std::string						m_name;
	SurfaceType						m_surfaceType;
	std::string						m_constructionName;
	std::string						m_bsdName;
	std::string						m_outsideBoundaryConditionObject;
	double							m_viewFactorToGround;
	std::string						m_shadingControlName;
	std::string						m_frameAndDividerName;

	std::vector<IBKMK::Vector3D>	m_polyline;
};
}


#endif // EP_FENESTRATIONSURFACEDETAILED_H
