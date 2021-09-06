#ifndef EP_ProjectH
#define EP_ProjectH

#include <IBK_Path.h>

#include "EP_BuildingSurfaceDetailed.h"
#include "EP_FenestrationSurfaceDetailed.h"
#include "EP_Material.h"
#include "EP_WindowMaterial.h"
#include "EP_Construction.h"
#include "EP_Zone.h"
#include "EP_Version.h"
#include "EP_ShadingBuildingDetailed.h"
#include "EP_Frame.h"
#include "EP_ShadingSystems.h"

namespace EP {

class IDFParser;

class Project {
public:
	// *** PUBLIC MEMBER FUNCTIONS ***
	/*! Read IDF. */
	void readIDF(const IDFParser &idfData);



	/*! Write an class object in energy plus format.
		Returns a string with all elements.
	*/
	template <class T>
	std::string writeClassObj(const std::vector<T> &objects) const {
		std::stringstream ss;
		for (size_t i=0; i<objects.size(); ++i) {
			const T &obj = objects[i];
			std::string str;
			obj.write(str, m_version);
			ss << str;
		}

		return  ss.str();
	}

	/*! Write IDF*/
	void writeIDF(const IBK::Path &filename);

	/*! Create a merge of two projects. */
	Project mergeProjects(const Project &other, const IBKMK::Vector3D & shift);

	/*! Return a new unique id for the set ids. */
	static std::string findId(std::set<std::string> &ids, std::string id);

	/*! Return a new unique id for the set ids. And check if this id is also not in a set of otherIds. */
	static std::string findId(std::set<std::string> &ids, const std::set<std::string> & otherIds, std::string id);

	/*! Check if Building Surface Detailed elements have a wrong id or invalid link. */
	bool hasProjectDuplicateIds();

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Version of IDF*/
	EP::Version										m_version;

	/*! Vector of all building surface detailed objects (opace surfaces) */
	std::vector<Zone>   							m_zones;

	/*! Vector of all building surface detailed objects (opace surfaces) */
	std::vector<EP::BuildingSurfaceDetailed>		m_bsd;

	/*! Vector of all building surface detailed objects (opace surfaces) */
	std::vector<EP::FenestrationSurfaceDetailed>	m_fsd;

	/*! Vector of all opaque materials */
	std::vector<EP::Material>						m_materials;

	/*! Vector of all glazing materials */
	std::vector<EP::WindowMaterial>					m_windowMaterial;

	/*! Vector of all constructions */
	std::vector<EP::Construction>					m_constructions;

	/*! Vector of all ShadingBuildingDetaile */
	std::vector<EP::ShadingBuildingDetailed>		m_shadingBuildingDetailed;

	/*! Vector of all ShadingBuildingDetaile */
	std::vector<EP::Frame>							m_frames;

	/*! Vector of all ShadingBuildingDetaile */
	std::vector<EP::ShadingSystems>					m_shadingSystems;


};


} // namespace EP

#endif // EP_ProjectH
