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
	tr("Maximum width of discretized volumes in pipe");
	tr("Constant temperature");
	tr("Constant heat flux");
	tr("Heat flux from data file");
	tr("Heat exchange with zone");
	tr("Heat exchange with FMU which requires temperature and provides heat flux");
	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Building");
	tr("Mixer");
	tr("Source");
	tr("If true, output files are written in binary format (the default, if flag is missing).");
	tr("If true, default output definitions for zones are created.");
	tr("Triangle");
	tr("Rectangle");
	tr("Polygon");
	tr("Floor area of the zone.");
	tr("Volume of the zone.");
	tr("Specularity of the material.");
	tr("Roughness of the material.");
	tr("Plastic");
	tr("Metal");
	tr("Glass");
	tr("Grid is visible");
	tr("Area of the divider.");
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
}


QString KeywordListQt::Description( const std::string & category, int keywordId) { 

	std::string description = KeywordList::Description(category.c_str(), keywordId);
	return tr(description.c_str());

}

} // namespace
