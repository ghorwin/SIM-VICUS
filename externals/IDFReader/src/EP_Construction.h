#ifndef EP_CONSTRUCTION_H
#define EP_CONSTRUCTION_H

#include <vector>
#include <string>

namespace EP {

class Material;
class WindowMaterial;

/*!
	IDF Construction Class
	CHECK no changes between EP version 8.3 to 9.5.
*/
class Construction
{
public:


	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Read "Construction"  section in IDF. */
	void read(const std::vector<std::string> & str, unsigned int version);

	/*! Write IDF Class. */
	void write(std::string &outStr, unsigned int version) const;

	bool behavesLike(const Construction &other, const std::vector<Material> &mats,
					 const std::vector<Material> otherMats,
					 const std::vector<WindowMaterial> & winMats,
					 const std::vector<WindowMaterial> otherWinMats) const;

	// *** PUBLIC MEMBER VARIABLES ***

	std::string					m_name;
	std::vector<std::string>	m_layers;

};
}

#endif // EP_CONSTRUCTION_H
