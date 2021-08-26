#ifndef EP_WINDOWMATERIAL_H
#define EP_WINDOWMATERIAL_H

#include <vector>
#include <string>

namespace EP {


/*!
	IDF Window Material Simple Galzing System Class
	CHECK no changes between EP version 8.3 to 9.5.
*/
class WindowMaterial
{
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	WindowMaterial(){}

	WindowMaterial(double SHGC, double uValue):
		m_name("WindowMat"),
		m_uValue(uValue),
		m_SHGC(SHGC),
		m_visibleTransmittance(0.8)
	{}


	/*! Read "WindowMaterial:SimpleGlazingSystem"  section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;

	bool behavesLike(const WindowMaterial &other) const;

	// *** PUBLIC MEMBER VARIABLES ***
	std::string					m_name;
	double						m_uValue;					// W/m2K
	double						m_SHGC;						// unitless
	double						m_visibleTransmittance;		// unitless
};
}

#endif // EP_WINDOWMATERIAL_H
