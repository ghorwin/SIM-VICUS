#ifndef IBKMK_TriangulationH
#define IBKMK_TriangulationH

#include <IBK_point.h>
#include <IBK_assert.h>

namespace IBKMK {

/*! Performs triangulation. */
class Triangulation {
public:

	/*! Set points to triangulate.
		No duplicate points (within tolerance allowed!)
		Also, edges must mark outer and inner boundaries of surface.
	*/
	bool setPoints(const std::vector<IBK::point2D<double> > & points,
				   const std::vector<std::pair<unsigned int, unsigned int> > & edges);

	/*! Tolerance criterion - points within this distances are
		takes and "same".
	*/
	double	m_tolerance;

	struct triangle_t {
		triangle_t() {}
		triangle_t(unsigned int n1, unsigned int n2, unsigned int n3) :
			i1(n1), i2(n2), i3(n3)
		{}

		unsigned int i1, i2, i3;
	};

	/*! Contains the generated triangles after triangulation has completed. */
	std::vector<triangle_t>		m_triangles;
};

} // namespace IBKMK

#endif // IBKMK_TriangulationH
