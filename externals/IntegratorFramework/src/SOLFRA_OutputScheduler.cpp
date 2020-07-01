#include "SOLFRA_OutputScheduler.h"

namespace SOLFRA {

double OutputScheduler::nextOutputTime(double t) {
	const double dt_out = 3600;
	double tout = static_cast<int>(t/dt_out) * dt_out;
	if (tout < t + dt_out*1e-8)
		tout += dt_out;
	return tout;
}

} // namespace SOLFRA

