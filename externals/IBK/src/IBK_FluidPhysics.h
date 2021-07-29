#ifndef FLUIDPHYSICS_H
#define FLUIDPHYSICS_H


namespace IBK {

/*!	\file IBK_FluidPhysics.h
	\brief Physical constants and functions needed to calculate heat transport and wall friction of fluidy in pipes.
*/

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

/*! Calculates the darcy friction factor [-] according to swamee-jain equation (approximation of colebrook-white)
	\param reynolds Reynolds number [-]
	\param d Pipe outside diameter [m]
	\param roughness Pipe wall roughness [m]
*/
double FrictionFactorSwamee(const double &reynolds, const double &d, const double &roughness);


} // namespace IBK

#endif // FLUIDPHYSICS_H
