#include "NM_Physics.h"

#include <cmath>
#include <IBK_assert.h>

/*! Reynolds number where flow switches from laminar to transition state. */
#define RE_LAMINAR		1700

/*! Reynolds number where flow switches from transition state to turbulent */
#define RE_TURBULENT	4000

namespace NANDRAD_MODEL {


double FrictionFactorSwamee(const double &reynolds, const double &diameter, const double &roughness){
	if (reynolds < RE_LAMINAR)
		return 64/reynolds;
	else if (reynolds < RE_TURBULENT){
		double fLam = 64/RE_LAMINAR; // f(RE_LAMINAR)
		double fTurb = std::log10((roughness / diameter) / 3.7 + 5.74 / std::pow(RE_TURBULENT, 0.9) );
		fTurb = 0.25/(fTurb*fTurb); // f(RE_TURBULENT)
		// now interpolate linearly between fLam and fTurb
		return fLam + (reynolds - RE_LAMINAR) * (fTurb - fLam) / (RE_TURBULENT - RE_LAMINAR);
	}
	else{
		double f = std::log10( (roughness / diameter) / 3.7 + 5.74 / std::pow(reynolds, 0.9) ) ;
		return	0.25 / (f*f);
	}
}

double NusseltNumber(const double &reynolds, const double &prandtl, const double &l, const double &d)
{
	if (reynolds < RE_LAMINAR){
		return NusseltNumberLaminar(reynolds, prandtl, l, d);
	}
	else if (reynolds < RE_TURBULENT){
		double nuLam = NusseltNumberLaminar(RE_LAMINAR, prandtl, l, d);
		double nuTurb = NusseltNumberTurbulent(RE_TURBULENT, prandtl, l, d);
		return nuLam + (reynolds - RE_LAMINAR) * (nuTurb - nuLam) / (RE_TURBULENT - RE_LAMINAR);
	}
	else {
		return NusseltNumberTurbulent(reynolds, prandtl, l, d);
	}
}

double NusseltNumberTurbulent(const double &reynolds, const double &prandtl, const double &l, const double &d)
{
	IBK_ASSERT(reynolds>0);
	double zeta = std::pow(1.8 * std::log10(reynolds) - 1.5, -2.0);
	return zeta / 8. * reynolds*prandtl /
		(1. + 12.7 * std::sqrt(zeta / 8.) * (std::pow(prandtl, 0.6667) - 1.)) *
							 (1. + std::pow(d / l, 0.6667));
}

double NusseltNumberLaminar(const double &reynolds, const double &prandtl, const double &l, const double &d)
{
	if (reynolds <=0)
		return 3.66; // for velocity=0
	else
		return std::pow( 49.37 + std::pow(1.615 * std::pow(reynolds * prandtl * d/l, 1/3) - 0.7, 3.0) , 1/3);
}

double PrandtlNumber(const double &kinVis, const double &cp, const double &lambda, const double &rho)
{
	return kinVis * cp * rho / lambda;
}

double ReynoldsNumber(const double &v, const double &kinVis, const double &d)
{
	return  v * d / kinVis;
}


} // namespace NANDRAD_MODEL
