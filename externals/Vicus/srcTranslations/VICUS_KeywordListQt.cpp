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

	tr("Solar absorption coefficient.");
	tr("Constant Long wave emissivity.");
	tr("Visible absorption coefficient.");
	tr("Constant heat transfer coefficient.");
	tr("Constant model.");
	tr("Heat transfer coefficient depends on wind velocity.");
	tr("Dry density of the material.");
	tr("Global Warming Potential.");
	tr("Depletion potential of the stratospheric ozone layer.");
	tr("Photochemical Ozone Creation Potential.");
	tr("Acidification potential.");
	tr("Eutrophication potential.");
	tr("Global Warming Potential.");
	tr("Global Warming Potential.");
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
	tr("Dry density of the material.");
	tr("Specific heat capacity of the material.");
	tr("Thermal conductivity of the dry material.");
	tr("Building");
	tr("Mixer");
	tr("Source");
	tr("Triangle");
	tr("Rectangle");
	tr("Polygon");
	tr("All");
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


QString KeywordListQt::Description( const std::string & category, unsigned int keywordId) { 

	std::string description = KeywordList::Description(category.c_str(), keywordId);
	return tr(description.c_str());

}

} // namespace
