#ifndef VICUS_PlaneGeometryH
#define VICUS_PlaneGeometryH

#include <IBKMK_Vector3D.h>

#include "VICUS_CodeGenMacros.h"

namespace VICUS {

/*! Class PlaneGeometry encapsulates the vertex data and plane type of a single plane
	in the geometrical model.

	Also, it implements intersection tests (for picking).
*/
class PlaneGeometry {
	VICUS_READWRITE_PRIVATE
public:

	/*! Different types of the plane. */
	enum type_t {
		/*! Triangle defined through three vertices. */
		T_Triangle,			// Keyword: Triangle
		/*! Rectangle defined through three vertices (a, b and d). */
		T_Rectangle,		// Keyword: Rectangle
		/*! Polygon, generic polygon with n points. */
		T_Polygon,			// Keyword: Polygon
		NUM_T
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE_IFNOTEMPTY(PlaneGeometry)
	VICUS_COMP(PlaneGeometry)

	/*! Default constructor. */
	PlaneGeometry();

	/*! Initializing constructor.
		Vertexes a, b and c must be given in counter-clockwise order, so that (b-a) x (c-a) yields the normal vector of the plane.
	*/
	PlaneGeometry(type_t t, const IBKMK::Vector3D & a, const IBKMK::Vector3D & b, const IBKMK::Vector3D & c);

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Type of the plane.
		T_POLYGON is the most generic, yet T_TRIANGLE and T_RECTANGLE offer some specialized handling for
		intersection calcuation and data transfer to the graphics pipeline.
	*/
	type_t								m_type = NUM_T;				// XML:A

	/*! Points of polyline. */
	std::vector<IBKMK::Vector3D>		m_vertexes;					// XML:E
};

} // namespace VICUS

#endif // VICUS_PlaneGeometryH
