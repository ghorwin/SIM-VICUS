/*	The NANDRAD data model library.

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

#include "NANDRAD_KeywordListQt.h"

namespace NANDRAD {
KeywordListQt::KeywordListQt() {

	tr("Orientation of the wall [deg].");
	tr("Inclination of the wall [deg].");
	tr("Gross area of the wall [m2].");
	tr("Constant values in defined intervals.");
	tr("Linear interpolation between values.");
	tr("Area of the embedded object [m2].");
	tr("Parametrization of a window model.");
	tr("Parametrization of a door model.");
	tr("Parametrization of an opening model.");
	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Simple pipe at stationary flow conditions without heat exchange");
	tr("Simple pipe at stationary flow conditions with heat exchange");
	tr("Pipe with a discretized fluid volume, without heat exchange");
	tr("Pipe with a discretized fluid volume and heat exchange");
	tr("Pump with constant pressure");
	tr("Simple heat exchanger with given heat flux");
	tr("Heat pump");
	tr("Gas boiler");
	tr("Control valve");
	tr("Water storage");
	tr("Component conditioning system is a system for heating or cooling of components");
	tr("Radiator");
	tr("Mixer component");
	tr("Flow characteristics provided by FMU");
	tr("Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes).");
	tr("Pressure loss coefficient for the component (zeta-value).");
	tr("External heat transfer coeffient for the outside boundary.");
	tr("Pressure head form a pump.");
	tr("Pump efficiency.");
	tr("Motor efficiency for a pump.");
	tr("Water or air volume of the component.");
	tr("Coefficient of performance of the component.");
	tr("UA-Value of heat exchanger");
	tr("Constant heat flux");
	tr("Heat flux from data file");
	tr("Heat exchange with zone");
	tr("Heat exchange with FMU which requires temperature and provides heat flux");
	tr("Pipe length");
	tr("Temperature for heat exchange");
	tr("Heat flux for heat exchange");
	tr("ID of coupled zone for thermal exchange");
	tr("Roughness of pipe material.");
	tr("Inner diameter of pipe.");
	tr("Outer diameter of pipe.");
	tr("Length specific U-Value of pipe wall incl. insulation");
	tr("Pressure coeffient.");
	tr("Use results from external wind flow calculation.");
	tr("Constant heat exchange coefficient");
	tr("No convective heat exchange");
	tr("Constant heat transfer coefficient");
	tr("Constant model.");
	tr("No long wave radiation exchange");
	tr("Constant Long wave emissivity.");
	tr("Constant model.");
	tr("No short wave radiation exchange");
	tr("Constant Absorption coefficient [0,...,1].");
	tr("Vapor Transfer Coefficient.");
	tr("Constant model.");
	tr("Constant internal loads");
	tr("Scheduled internal loads");
	tr("Percentage of equipment load that is radiant emitted.");
	tr("Percentage of person load that is radiant emitted.");
	tr("Percentage of lighting load that is radiant emitted.");
	tr("Complete equipment load per zone floor area.");
	tr("Complete person load per zone floor area.");
	tr("Complete lighting load per zone floor area.");
	tr("Start time point.");
	tr("End time point.");
	tr("StepSize.");
	tr("Some temperatures");
	tr("Some mass");
	tr("constant");
	tr("linear");
	tr("Continuous data");
	tr("Annual cycle");
	tr("Latitude.");
	tr("Longitude.");
	tr("Albedo value [0..100 %].");
	tr("Altitude of building as height above NN [m].");
	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Model references of climate/location models.");
	tr("Model references inside a room.");
	tr("Model references a wall.");
	tr("Model references an embedded object.");
	tr("Model references an active object.");
	tr("Model references scheduled data.");
	tr("Model references an object list-specific value.");
	tr("Model references of a generic model.");
	tr("Model references to global physical quantities.");
	tr("Model references of a hydraulic network.");
	tr("Model references of flow elements of a hydraulic network.");
	tr("Constant ventilation rate");
	tr("Scheduled ventilation rate");
	tr("Ventilation rate");
	tr("Write values as calculated at output times.");
	tr("Average values in time (mean value in output step).");
	tr("Integrate values in time.");
	tr("All days (Weekend days and Weekdays).");
	tr("Weekday schedule.");
	tr("Weekend schedule.");
	tr("Special Weekday schedule: Monday.");
	tr("Special Weekday schedule: Tuesday.");
	tr("Special Weekday schedule: Wednesday.");
	tr("Special Weekday schedule: Thursday.");
	tr("Special Weekday schedule: Friday.");
	tr("Special Weekday schedule: Saturday.");
	tr("Special Weekday schedule: Sunday.");
	tr("Holiday schedule.");
	tr("Monday.");
	tr("Tuesday.");
	tr("Wednesday.");
	tr("Thursday.");
	tr("Friday.");
	tr("Saturday.");
	tr("Sunday.");
	tr("If enabled, schedules are treated as annually repeating schedules.");
	tr("X1");
	tr("X2");
	tr("I1");
	tr("I2");
	tr("ParameterSet1");
	tr("ParameterSet2");
	tr("Simple hysteretic shading control based on global radiation sensor");
	tr("Maximum intensity allowed before shading is closed.");
	tr("Intensity level below which shading is opened.");
	tr("Global initial temperature [C].");
	tr("Global initial relative humidity [%].");
	tr("Percentage of sensitive heat from domestic water istributed towrads the room.");
	tr("Air exchange rate resulting from a pressure difference of 50 Pa between inside and outside.");
	tr("Shielding coefficient for a given location and envelope type.");
	tr("Ambient temperature for a design day. Parameter that is needed for FMU export.");
	tr("Start year of the simulation.");
	tr("Flag activating moisture balance calculation if enabled.");
	tr("Flag activating CO2 balance calculation if enabled.");
	tr("Flag activating ventilation through joints and openings.");
	tr("Flag activating FMU export of climate data.");
	tr("Distribution based on surface area");
	tr("Distribution based on surface type");
	tr("Distribution based on zone-specific view factors");
	tr("Percentage of solar radiation gains attributed direcly to room [%]");
	tr("Percentage of surface solar radiation attributed to floor [%]");
	tr("Percentage of surface solar radiation attributed to roof/ceiling[%]");
	tr("Percentage of surface solar radiation attributed to walls [%]");
	tr("Relative tolerance for solver error check.");
	tr("Absolute tolerance for solver error check.");
	tr("Maximum permitted time step for integration.");
	tr("Minimum accepted time step, before solver aborts with error.");
	tr("Initial time step size (or constant step size for ExplicitEuler integrator).");
	tr("Coefficient reducing nonlinear equation solver convergence limit.");
	tr("Coefficient reducing iterative equation solver convergence limit.");
	tr("Minimum element width for wall discretization.");
	tr("Stretch factor for variable wall discretizations (0-no disc, 1-equidistance, larger than 1 - variable).");
	tr("Maximum dimension of a tile for calculation of view factors.");
	tr("Number of surface discretization elements of a wall in each direction.");
	tr("Temperature tolerance for ideal heating or cooling.");
	tr("Relative tolerance for Kinsol solver.");
	tr("Absolute tolerance for Kinsol solver.");
	tr("Maximum level of fill-in to be used for ILU preconditioner.");
	tr("Maximum dimension of Krylov subspace.");
	tr("Maximum number of nonlinear iterations.");
	tr("Maximum order allowed for multi-step solver.");
	tr("Maximum nonlinear iterations for Kinsol solver.");
	tr("Maximum number of elements per layer.");
	tr("Check schedules to determine minimum distances between steps and adjust MaxTimeStep.");
	tr("Disable line search for steady state cycles.");
	tr("Enable strict Newton for steady state cycles.");
	tr("CVODE based solver");
	tr("Explicit Euler solver");
	tr("Implicit Euler solver");
	tr("Automatic selection of integrator");
	tr("Dense solver");
	tr("KLU sparse solver");
	tr("GMRES iterative solver");
	tr("BICGSTAB iterative solver");
	tr("Automatic selection of linear equation system solver");
	tr("Incomplete LU preconditioner");
	tr("Automatic selection of preconditioner");
	tr("Gas layer");
	tr("Glass layer");
	tr("Thickness of the window layer.");
	tr("Thermal conductivity of the window layer.");
	tr("Mass density of the fill-in gas.");
	tr("Height of the detailed window.");
	tr("Width of the detailed window.");
	tr("Emissivity of surface facing outside.");
	tr("Emissivity of surface facing inside.");
	tr("Short wave transmittance at outside directed surface.");
	tr("Short wave reflectance of surface facing outside.");
	tr("Short wave reflectance of surface facing inside.");
	tr("Thermal conductivity of the gas layer.");
	tr("Dynamic viscosity of the gas layer.");
	tr("Specific heat capacity of the gas layer.");
	tr("Standard globbed-layers model.");
	tr("Detailed window model with layers.");
	tr("Thermal transmittance");
	tr("Short wave transmittance at outside directed surface.");
	tr("Standard reduction factor.");
	tr("Reduction factor (remaining percentage of solar gains if shading is closed).");
	tr("Zone with constant/predefined temperatures. (schedule)");
	tr("Zone described by a temperature node in space.");
	tr("Ground zone (calculates temperature based on standard).");
	tr("Temperature of the zone if set constant [C].");
	tr("Relative humidity of the zone if set constant [%].");
	tr("CO2 concentration of the zone if set constant [g/m3].");
	tr("Net usage area of the ground floor [m2] (for area-related outputs and loads).");
	tr("Zone air volume [m3].");
	tr("Extra heat capacity [J/K].");
}


QString KeywordListQt::Description( const std::string & category, int keywordId) { 

	std::string description = KeywordList::Description(category.c_str(), keywordId);
	return tr(description.c_str());

}

} // namespace
