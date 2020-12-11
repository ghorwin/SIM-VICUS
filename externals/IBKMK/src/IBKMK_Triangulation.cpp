#include "IBKMK_Triangulation.h"

namespace IBKMK {

bool Triangulation::setPoints(const std::vector<IBK::point2D<double> > & points) {
	IBK_ASSERT(sizeof(IBK::point2D<double>) == 2*sizeof(double));
	return setPoints(points.size(), (const double*)points.data());
}


bool Triangulation::setPoints(unsigned int n, const double points[]) {

	return true;
}


} // namespace IBKMK

