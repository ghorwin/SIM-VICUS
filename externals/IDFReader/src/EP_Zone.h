#ifndef EP_ZONE_H
#define EP_ZONE_H

#include <IBKMK_Vector3D.h>

namespace EP {

/*!
	IDF Zone Class
	CHECK no changes between EP version 8.3 to 9.5.
*/

class Zone
{
public:

	// *** PUBLIC MEMBER FUNCTIONS ***
	/*! Read "BuildingSurface:Detailed" section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;



	// *** PUBLIC MEMBER VARIABLES ***
	std::string						m_name;
	double							m_ceilingHeight;
	double							m_volume;
	double							m_floorArea;
};
}

#endif // EP_ZONE_H
