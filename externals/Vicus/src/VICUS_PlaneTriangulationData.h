#ifndef VICUS_PlaneTriangulationDataH
#define VICUS_PlaneTriangulationDataH

#include <IBKMK_Vector3D.h>
#include <IBKMK_Triangulation.h>

namespace VICUS {

/*! Class PlaneTriangulationData holds data needed to draw a triangulated plane.
	This is basically a vector of vertexes and a vector of triangles.
*/
class PlaneTriangulationData {
public:
	// *** PUBLIC MEMBER FUNCTIONS ***

	void clear() {
		m_triangles.clear();
		m_vertexes.clear();
	}

	/*! Contains the vertex indexes for each triangle that the plane is composed of. */
	std::vector<IBKMK::Triangulation::triangle_t>		m_triangles;

	/*! The vertexes used by the triangles. */
	std::vector<IBKMK::Vector3D>						m_vertexes;

	/*! The normal vector. */
	IBKMK::Vector3D										m_normal;
};

} // namespace VICUS

#endif // VICUS_PlaneTriangulationDataH
