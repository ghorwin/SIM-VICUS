/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#ifndef NM_PhysicsH
#define NM_PhysicsH

#include <algorithm>
#include <IBK_math.h>

namespace NANDRAD_MODEL {

/*!	\file NM_Physics.h
	\brief Physical constants and functions used in NANDRAD.
	Calculations are provided for fluid dynamics and some missing material parameters.
*/

/*! Molar mass of carbon dioxid.*/
const double MolarMassCO2 = 44.01e-03;

/*! Calculates Reynolds number of a moving fluid.
	\param rho fluid mass density
	\param v mean fluid flow velocity
	\param mue fluid dynamic viscosity
	\param l characteristic length
*/
inline double ReynoldsNumber(const double rho, const double v, const double mue, const double l) {
	return rho * v * l / mue;
}

/*! Calculates Prandtl number of a moving fluid.
	\param mue fluid dynamic viscosity
	\param Cp fluid specific heat capacity
	\param lambda fluid thermal conductivity
*/
inline double PrandtlNumber(const double mue, const double Cp, const double lambda) {
	return mue * Cp / lambda;
}

/*! Calculates nusselt number for a turbulent fluid through a pipe.
	\param reynolds Reynolds number
	\param prandtl Prandtl number
	\param xi Pressure loss coefficient
	\param l Characteristic length
	\param d Pipe outside diameter
*/
inline double NusseltNumberLaminar(const double /*reynold*/, const double /*prandtl*/, const double /*xi*/, const double /*l*/, const double /*d*/) {
	return 3.66;
}

/*! Calculates nusselt number for a turbulent fluid through a pipe.
	Nusselt number has fixed minimum to laminar
	\param reynolds Reynolds number
	\param prandtl Prandtl number
	\param xi Pressure loss coefficient
	\param l Characteristic length
	\param d Pipe outside diameter
*/
inline double NusseltNumberTurbulent(const double reynold, const double prandtl, const double xi, const double l, const double d) {
	return  std::max<double>(3.66, xi / 8. * (reynold - 1000.)*prandtl /
		(1. + 12.7 * IBK::f_sqrt(xi / 8.) * (IBK::f_pow(prandtl, 0.6667) - 1.)) *
		(1. + IBK::f_pow(d / l, 0.6667)));

}

/*! Calculates pressure loss coefficient for a turbulent fluid through a pipe.
	\param reynolds Reynolds number
*/
inline double PressureLossCoeff(const double reynold) {
	return 1 / IBK::f_pow(1.82 * IBK::f_log10(reynold) - 1.64,2);
}

} // namespace NANDRAD_MODEL

#endif // NM_PhysicsH
