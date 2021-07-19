/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef NM_PhysicsH
#define NM_PhysicsH

#include <algorithm>

namespace NANDRAD_MODEL {

/*!	\file NM_Physics.h
	\brief Physical constants and functions used in NANDRAD.
	Calculations are provided for fluid dynamics and some missing material parameters.
*/

/*! Molar mass of carbon dioxid.*/
const double MolarMassCO2 = 44.01e-03;

/*! Calculates Reynolds number [-] of a moving fluid.
	\param v mean fluid flow velocity [m/s]
	\param kinVis fluid kinematic viscosity [m2/s]
	\param l characteristic length [m]
*/
double ReynoldsNumber(const double &v, const double &kinVis, const double &l);

/*! Calculates Prandtl number [-] of a moving fluid.
	\param kinVis fluid kinematic viscosity [m2/s]
	\param cp fluid specific heat capacity [J/kgK]
	\param lambda fluid thermal conductivity [W/mK]
	\param rho fluid mass density [kg/m3]
*/
double PrandtlNumber(const double &kinVis, const double &cp, const double &lambda, const double &rho);

/*! Calculates nusselt number [-] for laminar fluid flow through a pipe.
	Nusselt number has fixed minimum to laminar
	\param reynolds Reynolds number [-]
	\param prandtl Prandtl number [-]
	\param l Characteristic length [m]
	\param d Pipe outside diameter [m]
*/
double NusseltNumberLaminar(const double &reynolds, const double &prandtl, const double &l, const double &d);

/*! Calculates nusselt number [-] for a turbulent fluid through a pipe. Reynolds should not be 0! This function is only
 * called by NusseltNumber() in case Re > RE_LAMINAR.
	Nusselt number has fixed minimum to laminar
	\param reynolds Reynolds number [-]
	\param prandtl Prandtl number [-]
	\param l Characteristic length [m]
	\param d Pipe outside diameter [m]
*/
double NusseltNumberTurbulent(const double &reynolds, const double &prandtl, const double &l, const double &d);

/*! Calculates nusselt number [-] for a turbulent fluid through a pipe.
	Nusselt number has fixed minimum to laminar
	\param reynolds Reynolds number [-]
	\param prandtl Prandtl number [-]
	\param l Characteristic length [m]
	\param d Pipe outside diameter [m]
*/
double NusseltNumber(const double &reynolds, const double &prandtl, const double &l, const double &d);

/*! Calculates the darcy friction factor [-] according to swamee-jain euqation (approximation of colebrook-white)
	\param reynolds Reynolds number [-]
	\param d Pipe outside diameter [m]
	\param roughness Pipe wall roughness [m]
*/
double FrictionFactorSwamee(const double &reynolds, const double &d, const double &roughness);

} // namespace NANDRAD_MODEL

#endif // NM_PhysicsH
