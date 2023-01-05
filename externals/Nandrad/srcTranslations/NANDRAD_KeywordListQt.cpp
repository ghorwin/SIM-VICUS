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
	tr("Heating control model");
	tr("Parallel operation");
	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Only Hydraulic calculation with constant temperature");
	tr("Thermo-hydraulic calculation");
	tr("Air network that may be connected with zones.");
	tr("Default temperature for HydraulicNetwork models");
	tr("Initial temperature of the fluid");
	tr("Reference pressure of network");
	tr("Pipe with a single fluid volume");
	tr("Pipe with discretized fluid volume");
	tr("Pump with defined pressure head");
	tr("Pump with defined mass flux");
	tr("Pump with controlled pressure head");
	tr("Pump with linear pressure head curve");
	tr("Simple heat exchanger");
	tr("Heat pump installed at source side, based on carnot efficiency");
	tr("Heat pump installed at supply side, based on carnot efficiency");
	tr("Heat pump installed at source side, based on polynom for COP");
	tr("Heat pump (on-off) at source side, based on polynoms for heating power and el. power");
	tr("Controlled valve");
	tr("Valve with constant pressure loss");
	tr("Ideal heater with predefined supply temperature");
	tr("Adiabatic element with pressure loss defined by zeta-value");
	tr("Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)");
	tr("Pressure loss coefficient for the component (zeta-value)");
	tr("Pump predefined pressure head");
	tr("Pump predefined mass flux");
	tr("Pump maximum efficiency");
	tr("Fraction of pump heat loss due to inefficiency that heats up the fluid");
	tr("Pump maximum pressure head at point of minimal mass flow of pump");
	tr("Pump maximum electrical power at point of optimal operation");
	tr("Design pressure head of VariablePressureHeadPump");
	tr("Design mass flux of VariablePressureHeadPump");
	tr("Factor to reduce pressure head of VariablePressureHeadPump");
	tr("Water or air volume of the component");
	tr("Maximum width/length of discretized volumes in pipe");
	tr("Carnot efficiency eta");
	tr("Maximum heating power");
	tr("Pressure loss for valve");
	tr("Minimum outlet temperature of heat exchanger, used for clipping of heat extraction");
	tr("HeatingPowerB0W35");
	tr("Heat pump supply temperature for heating buffer storage");
	tr("Heat pump return temperature for heating buffer storage");
	tr("Heat pump supply temperature for DHW buffer storage");
	tr("Heat pump return temperature for DHW buffer storage");
	tr("Heat pump heating buffer storage volume");
	tr("DHWBufferVolume");
	tr("Set points are given as constant parameters");
	tr("Scheduled set point values");
	tr("Control temperature difference of this element");
	tr("Control temperature difference of the following element");
	tr("Control zone thermostat values");
	tr("Control mass flux");
	tr("Control pump operation depending on following element");
	tr("Control pressure difference at worst point in the network");
	tr("PController");
	tr("PIController");
	tr("PIDController");
	tr("OnOffController");
	tr("Kp-parameter");
	tr("Ki-parameter");
	tr("Kd-parameter");
	tr("Target temperature difference");
	tr("Target mass flux");
	tr("Threshold value for pump operation");
	tr("Reset value for controller integral part");
	tr("Target pressure difference");
	tr("ID of zone containing thermostat");
	tr("Pipe length");
	tr("Number of parallel pipes");
	tr("Number of parallel elements");
	tr("Constant temperature");
	tr("Time-dependent temperature from spline");
	tr("Evaporator medium temperature for heat pump");
	tr("Zone air temperature");
	tr("Active construction layer (floor heating)");
	tr("Constant heat loss");
	tr("Heat loss from spline");
	tr("Heat loss of condenser in heat pump model");
	tr("Heating demand for space heating");
	tr("Temperature for heat exchange");
	tr("Constant heat flux out of the element (heat loss)");
	tr("External heat transfer coeffient for the outside boundary");
	tr("Temperature for heat exchange");
	tr("Constant heat flux out of the element (heat loss)");
	tr("ID of coupled zone for thermal exchange");
	tr("ID of coupled construction instance for thermal exchange");
	tr("Roughness of pipe material");
	tr("Inner diameter of pipe");
	tr("Outer diameter of pipe");
	tr("Length-specific U-Value of pipe wall incl. insulation");
	tr("Specific heat capaciy of pipe wall");
	tr("Density of pipe wall");
	tr("Maximum heating power per floor area");
	tr("Maximum cooling power per floor area");
	tr("Kp-parameter");
	tr("Ki-parameter");
	tr("Constant supply temperature");
	tr("Scheduled supply temperature");
	tr("Medium supply temperature");
	tr("Maximum mass flux through the pipe");
	tr("Pipe length");
	tr("Inner diameter of pipe");
	tr("Length-specific U-Value of pipe wall incl. insulation");
	tr("Number of parallel pipes");
	tr("Maximum heating power per surface area");
	tr("Maximum cooling power per surface area");
	tr("Pressure coeffient.");
	tr("Use results from external wind flow calculation.");
	tr("Constant heat exchange coefficient");
	tr("No convective heat exchange");
	tr("Convective heat transfer coefficient");
	tr("Constant model");
	tr("No long wave radiation exchange");
	tr("Long wave emissivity");
	tr("Constant model");
	tr("No short wave radiation exchange");
	tr("Solar absorption coefficient");
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
	tr("Constant internal loads");
	tr("Scheduled internal loads");
	tr("Complete person moisture load per zone floor area.");
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
	tr("Use diffuse radiation model for anisotropic radiation (Perez)");
	tr("If true, shading factors for exterior shading are stored for continuous time points (no cyclic use)");
	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Model references of climate/location models.");
	tr("Model references inside a room.");
	tr("Model references a wall.");
	tr("Model references an embedded object.");
	tr("Model references generic scheduled data that is not associated with a specific object type.");
	tr("Model references of a model object.");
	tr("Model references of a hydraulic network.");
	tr("Model references of flow elements of a hydraulic network.");
	tr("Constant ventilation rate (also can used as infiltration)");
	tr("Scheduled ventilation rate");
	tr("Scheduled basic air exchange (infiltration) with an additional increased air exchange (ventilation) if the (constant) control conditions are met.");
	tr("Scheduled basic air exchange (infiltration) with an additional increased air exchange and scheduled minimum/maximum temperature limits.");
	tr("Ventilation rate for Constant model");
	tr("Upper limit of comfort range");
	tr("Lower limit of comfort range");
	tr("Maximum wind speed to allow ventilation increase");
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
	tr("SomeStove");
	tr("SomeOven");
	tr("SomeHeater");
	tr("SomeFurnace");
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
	tr("Constant set points");
	tr("Scheduled set points");
	tr("Heating set point");
	tr("Cooling set point");
	tr("Control tolerance for temperatures");
	tr("Offset of lower and upper hysteresis band from set points");
	tr("Air temperature");
	tr("Operative temperature");
	tr("Analog");
	tr("Digital");
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
	tr("Constant reduction factor.");
	tr("Precomputed reduction factor.");
	tr("Reduction factor is computed based on control model");
	tr("Reduction factor (remaining percentage of solar gains if shading is closed).");
	tr("Zone with constant temperature");
	tr("Zone with schedule defined temperature");
	tr("Zone with energy balance equation");
	tr("Ground zone (calculates temperature based on heat loss to ground model)");
	tr("Temperature of the zone if set constant");
	tr("Relative humidity of the zone if set constant");
	tr("CO2 concentration of the zone if set constant");
	tr("Net usage area of the ground floor (for area-related outputs and loads)");
	tr("Zone air volume");
	tr("Extra heat capacity");
}


QString KeywordListQt::Description( const std::string & category, int keywordId) { 

	std::string description = KeywordList::Description(category.c_str(), keywordId);
	return tr(description.c_str());

}

} // namespace
