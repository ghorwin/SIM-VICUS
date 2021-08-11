#ifndef EP_ShadingBuildingDetailedH
#define EP_ShadingBuildingDetailedH

#include <IBKMK_Vector3D.h>

namespace EP {

/*! A simple shading object, will become an annonymous surface. */
class ShadingBuildingDetailed {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Read "Shading:Building:Detailed" section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;

	// *** PUBLIC MEMBER VARIABLES ***
	std::string						m_name;
	std::vector<IBKMK::Vector3D>	m_polyline;
};

} // namespace EP

#endif // EP_ShadingBuildingDetailedH
