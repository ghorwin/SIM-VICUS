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

namespace NANDRAD_MODEL {

/*!	\file NM_Physics.h
	\brief Physical constants and functions used in NANDRAD.
	Calculations are provided for fluid dynamics and some missing material parameters.
*/

/*! Molar mass of carbon dioxid.*/
const double MolarMassCO2 = 44.01e-03;

/*! Calculates Reynolds number of a moving fluid.
	\param v mean fluid flow velocity
	\param kinVis fluid kinematic viscosity
	\param l characteristic length
*/
double ReynoldsNumber(const double &v, const double &kinVis, const double &l);

/*! Calculates Prandtl number of a moving fluid.
	\param kinVis fluid kinematic viscosity
	\param cp fluid specific heat capacity
	\param lambda fluid thermal conductivity
	\param rho fluid mass density
*/
double PrandtlNumber(const double &kinVis, const double &cp, const double &lambda, const double &rho);

/*! Simple relation for friction factor for a turbulent fluid through a pipe. Only used for Nusselt calculation.
	\param reynolds Reynolds number
*/
double FrictionFactorSimple(const double &reynolds);

/*! Calculates nusselt number for laminar fluid flow through a pipe.
	Nusselt number has fixed minimum to laminar
	\param reynolds Reynolds number
	\param prandtl Prandtl number
	\param f friction factor
	\param l Characteristic length
	\param d Pipe outside diameter
*/
double NusseltNumberLaminar(const double &reynolds, const double &prandtl, const double &l, const double &d);

/*! Calculates nusselt number for a turbulent fluid through a pipe.
	Nusselt number has fixed minimum to laminar
	\param reynolds Reynolds number
	\param prandtl Prandtl number
	\param f friction factor
	\param l Characteristic length
	\param d Pipe outside diameter
*/
double NusseltNumberTurbulent(const double &reynolds, const double &prandtl, const double &l, const double &d);

/*! Calculates nusselt number for a turbulent fluid through a pipe.
	Nusselt number has fixed minimum to laminar
	\param reynolds Reynolds number
	\param prandtl Prandtl number
	\param f friction factor
	\param l Characteristic length
	\param d Pipe outside diameter
*/
double NusseltNumber(const double &reynolds, const double &prandtl, const double &l, const double &d);

double FrictionFactorSwamee(const double &reynolds, const double &diameter, const double &roughness);

} // namespace NANDRAD_MODEL

#endif // NM_PhysicsH
