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
	*/
	bool setPoints(const std::vector<IBK::point2D<double> > & points);

	/*! Set points to triangulate.
		Overloaded function, expects vector with point coordinates with size 2*n and
		coordinate order x1, y1, x2, y2, ....
	*/
	bool setPoints(const std::vector<double> & coords);

	/*! Set points to triangulate.
		No duplicate points (within tolerance allowed!)
		Overloaded function, expects continuous memory array passed to 'points' with size 2*n (x1, y1, x2, y2, ...).
	*/
	bool setPoints(unsigned int n, const double points[]);

	/*! Tolerance criterion - points within this distances are
		takes and "same".
	*/
	double	m_tolerance;

	struct triangle_t {
		unsigned int i1, i2, i3;
	};

	/*! Contains the generated triangles after triangulation has completed. */
	std::vector<triangle_t>		m_triangles;
};

} // namespace IBKMK

#endif // IBKMK_TriangulationH
