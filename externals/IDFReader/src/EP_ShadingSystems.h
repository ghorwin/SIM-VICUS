#ifndef EP_ShadingSystemsH
#define EP_ShadingSystemsH

#include <vector>
#include <string>

namespace EP {


/*!
	IDF Blind Class
	IDF Shade Class
	IDF Screen Class
	CHECK no changes between EP version 8.3 to 9.5.
*/
class ShadingSystems
{
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	ShadingSystems(){}

	/*! Read "Material" and "Material:NoMass" section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;

	bool behavesLike(const ShadingSystems &other) const;

	/*! Compares this instance with another by value and returns true if they equal. */
	bool operator==(const ShadingSystems & other) const;

	/*! Compares this instance with another by value and returns true if they differ. */
	bool operator!=(const ShadingSystems & other)  const { return ! operator==(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	enum Type{
		Blind,
		Shade,
		Screen,
		NUM_T
	};

	enum SlatOrientation{
		Horizontal,
		Vertical
	};

	Type						m_type;
	SlatOrientation				m_slatOrientation = Horizontal;
	std::string					m_name;
	double						m_shadeToGlassDistance;		//also for blind and screen
	double						m_topOpening;
	double						m_bottomOpening;
	double						m_leftOpening;
	double						m_rightOpening;
	double						m_conductivity;				// W/mK


};
}

#endif // EP_ShadingSystemsH
