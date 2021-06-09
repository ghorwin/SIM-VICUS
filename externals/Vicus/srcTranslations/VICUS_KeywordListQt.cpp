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

#include "VICUS_KeywordListQt.h"

namespace VICUS {
KeywordListQt::KeywordListQt() {

	tr("Outside wall construction");
	tr("Outside wall construction in contact with ground");
	tr("Interior construction");
	tr("Floor to basement");
	tr("Floor in contact with air");
	tr("Floor in contact with ground");
	tr("Ceiling construction");
	tr("Sloped roof construction");
	tr("Flat roof construction");
	tr("Flat roof construction (to heated/insulated space)");
	tr("Flat roof construction (to cold/ventilated space)");
	tr("Some other component type");
	tr("Outside wall construction");
	tr("Outside wall construction in contact with ground");
	tr("Interior construction");
	tr("Floor to basement");
	tr("Floor in contact with ground");
	tr("Ceiling construction");
	tr("Sloped roof construction");
	tr("Flat roof construction");
	tr("Miscellaneous");
	tr("Not insulated");
	tr("Inside insulated");
	tr("Core insulation");
	tr("Outside insulated");
	tr("Not selected");
	tr("Brick masonry");
	tr("Natural stones");
	tr("Concrete");
	tr("Wood");
	tr("Frame construction");
	tr("Loam");
	tr("Not selected");
	tr("Dry density of the material.");
	tr("Global Warming Potential.");
	tr("Depletion potential of the stratospheric ozone layer.");
	tr("Photochemical Ozone Creation Potential.");
	tr("Acidification potential.");
	tr("Eutrophication potential.");
	tr("Total use of non-renewable primary energy resources.");
	tr("Total use of renewable primary energy resources .");
	tr("Air change rate.");
	tr("Shielding coefficient for n50 value.");
	tr("normal");
	tr("n50");
	tr("Person Count.");
	tr("Person per area.");
	tr("Area per person.");
	tr("Power.");
	tr("Power per area.");
	tr("Convective Heat Factor.");
	tr("Latent Heat Factor.");
	tr("Loss Heat Factor.");
	tr("Person");
	tr("ElectricEquiment");
	tr("Lighting");
	tr("Other");
	tr("Person per m2");
	tr("m2 per Person");
	tr("Person count");
	tr("Power per area");
	tr("Power");
	tr("Some temperatures");
	tr("Some mass");
	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Vapor diffusion resistance factor.");
	tr("Water content in relation to 80% humidity.");
	tr("Water content at saturation.");
	tr("Coating");
	tr("Plaster");
	tr("Bricks");
	tr("NaturalStones");
	tr("Cementitious");
	tr("Insulations");
	tr("BuildingBoards");
	tr("Woodbased");
	tr("NaturalMaterials");
	tr("Soils");
	tr("CladdingSystems");
	tr("Foils");
	tr("Miscellaneous");
	tr("SinglePipe");
	tr("DoublePipe");
	tr("Temperature for pipe dimensioning algorithm");
	tr("Temperature difference for pipe dimensioning algorithm");
	tr("Maximum pressure loss for pipe dimensioning algorithm");
	tr("Reference pressure applied to reference element");
	tr("Fluid temperature for hydraulic calculation, else initial temperature");
	tr("Initial Fluid temperature for thermo-hydraulic calculation");
	tr("Pipe with a single fluid volume and with heat exchange");
	tr("Pipe with a discretized fluid volume and heat exchange");
	tr("Pump with constant pressure");
	tr("Simple heat exchanger with given heat flux");
	tr("Heat pump with variable heating power based on carnot efficiency");
	tr("On-off-type heat pump with based on manufacturer data sheet");
	tr("Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)");
	tr("Pressure loss coefficient for the component (zeta-value)");
	tr("Pressure head form a pump");
	tr("Pump efficiency");
	tr("Water or air volume of the component");
	tr("Maximum width of discretized volumes in pipe");
	tr("Carnot efficiency eta");
	tr("Maximum heating power");
	tr("Control the temperature difference at the flow element");
	tr("Control the mass flow of the flow element");
	tr("PController");
	tr("PIController");
	tr("Proportional value of controller");
	tr("Integral value of controller");
	tr("Differential value of controller");
	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Difference to constant temperature");
	tr("Difference to time-dependent temperature from spline");
	tr("Evaporator medium temperature for heat pump");
	tr("Difference to zone air temperature");
	tr("Difference to active construction layer (floor heating)");
	tr("Constant heat loss");
	tr("Heat loss from spline");
	tr("Heat loss of condenser in heat pump model");
	tr("Temperature for heat exchange");
	tr("Constant heat flux out of the element (heat loss)");
	tr("External heat transfer coeffient for the outside boundary");
	tr("Temperature for heat exchange");
	tr("Constant heat flux out of the element (heat loss)");
	tr("ID of coupled zone for thermal exchange");
	tr("ID of coupled construction instance for thermal exchange");
	tr("Building");
	tr("Mixer");
	tr("Source");
	tr("If true, output files are written in binary format (the default, if flag is missing).");
	tr("If true, default output definitions for zones are created.");
	tr("Floor area of the zone.");
	tr("Volume of the zone.");
	tr("A window");
	tr("A door");
	tr("Some other component type");
	tr("Specularity of the material.");
	tr("Roughness of the material.");
	tr("Plastic");
	tr("Metal");
	tr("Glass");
	tr("Air change rate.");
	tr("Grid is visible");
	tr("None");
	tr("Fraction");
	tr("ConstantWidth");
	tr("Frame width of the window.");
	tr("Frame area fraction of the window.");
	tr("Divider width of the window.");
	tr("Divider area fraction of the window.");
	tr("Divider material thickness.");
	tr("Frame material thickness.");
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
	tr("Upper limit for room air temperature.");
	tr("Lower limit for room air temperature.");
	tr("Upper limit for outside air temperature.");
	tr("Lower limit for outside air temperature.");
	tr("Temperature difference limit (inside - outside).");
	tr("Limit for wind speed .");
	tr("Global horizontal (upper) sensor setpoint value.");
	tr("Global north (upper) sensor setpoint value.");
	tr("Global east (upper) sensor setpoint value.");
	tr("Global south (upper) sensor setpoint value.");
	tr("Global west (upper) sensor setpoint value.");
	tr("Dead band value for all sensors.");
	tr("One global horizontal sensor.");
	tr("One global horizontal and for each direction (N, E, S, W) a vertical sensor.");
	tr("Thermostat tolerance heating and cooling mode.");
	tr("Air temperature");
	tr("Radiant temperature");
	tr("Operative temperature");
	tr("Analog");
	tr("Digital");
	tr("Heating Limit.");
	tr("Cooling Limit.");
	tr("IntLoadPerson");
	tr("IntLoadEquipment");
	tr("IntLoadLighting");
	tr("IntLoadOther");
	tr("ControlThermostat");
	tr("Infiltration");
	tr("NaturalVentilation");
	tr("IdealHeatingCooling");
}


QString KeywordListQt::Description( const std::string & category, int keywordId) { 

	std::string description = KeywordList::Description(category.c_str(), keywordId);
	return tr(description.c_str());

}

} // namespace
