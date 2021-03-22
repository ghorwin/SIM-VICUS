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

#include "VICUS_KeywordList.h"

#include <map>
#include <limits>
#include <iostream>

#include <IBK_FormatString.h>
#include <IBK_Exception.h>


namespace VICUS {
	/*! Holds the string to print as error when an invalid keyword is encountered. */
	const char * const INVALID_KEYWORD_INDEX_STRING = "KEYWORD_ERROR_STRING: Invalid type index";

	/*! Holds a list of all enum types/categories. */
	const char * const ENUM_TYPES[36] = {
		"Component::ComponentType",
		"Construction::UsageType",
		"Construction::InsulationKind",
		"Construction::MaterialKind",
		"EPDDataset::para_t",
		"InternalLoad::para_t",
		"InternalLoad::Category",
		"InternalLoad::PersonCountMethod",
		"InternalLoad::PowerMethod",
		"KeywordList::MyParameters",
		"Material::para_t",
		"Material::Category",
		"Network::NetworkType",
		"Network::para_t",
		"NetworkComponent::ModelType",
		"NetworkComponent::para_t",
		"NetworkFluid::para_t",
		"NetworkHeatExchange::ModelType",
		"NetworkHeatExchange::para_t",
		"NetworkHeatExchange::splinePara_t",
		"NetworkHeatExchange::References",
		"NetworkNode::NodeType",
		"Outputs::flag_t",
		"PlaneGeometry::type_t",
		"Room::para_t",
		"SurfaceProperties::para_t",
		"SurfaceProperties::Type",
		"ViewSettings::Flags",
		"WindowDivider::para_t",
		"WindowGlazingLayer::type_t",
		"WindowGlazingLayer::para_t",
		"WindowGlazingLayer::splinePara_t",
		"WindowGlazingSystem::modelType_t",
		"WindowGlazingSystem::para_t",
		"WindowGlazingSystem::splinePara_t",
		"ZoneTemplate::SubTemplateType"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<36; ++i) {
			if (enumtype == ENUM_TYPES[i]) return i;
		}
		//std::cerr << "Unknown enumeration type '" << enumtype<< "'." << std::endl;
		return -1;
	}
	

	/*! Returns a keyword string for a given category (typenum) and type number t. */
	const char * theKeyword(int typenum, int t) {
		switch (typenum) {
			// Component::ComponentType
			case 0 :
			switch (t) {
				case 0 : return "OutsideWall";
				case 1 : return "OutsideWallToGround";
				case 2 : return "InsideWall";
				case 3 : return "FloorToCellar";
				case 4 : return "FloorToAir";
				case 5 : return "FloorToGround";
				case 6 : return "Ceiling";
				case 7 : return "SlopedRoof";
				case 8 : return "FlatRoof";
				case 9 : return "ColdRoof";
				case 10 : return "WarmRoof";
				case 11 : return "Miscellaneous";
			} break;
			// Construction::UsageType
			case 1 :
			switch (t) {
				case 0 : return "OutsideWall";
				case 1 : return "OutsideWallToGround";
				case 2 : return "InsideWall";
				case 3 : return "FloorToCellar";
				case 4 : return "FloorToGround";
				case 5 : return "Ceiling";
				case 6 : return "SlopedRoof";
				case 7 : return "FlatRoof";
				case 8 : return "---";
			} break;
			// Construction::InsulationKind
			case 2 :
			switch (t) {
				case 0 : return "NotInsulated";
				case 1 : return "InsideInsulation";
				case 2 : return "CoreInsulation";
				case 3 : return "OutsideInsulation";
				case 4 : return "---";
			} break;
			// Construction::MaterialKind
			case 3 :
			switch (t) {
				case 0 : return "BrickMasonry";
				case 1 : return "NaturalStoneMasonry";
				case 2 : return "Concrete";
				case 3 : return "Wood";
				case 4 : return "FrameWork";
				case 5 : return "Loam";
				case 6 : return "---";
			} break;
			// EPDDataset::para_t
			case 4 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "GWP";
				case 2 : return "ODP";
				case 3 : return "POCP";
				case 4 : return "AP";
				case 5 : return "EP";
				case 6 : return "PENRT";
				case 7 : return "PERT";
			} break;
			// InternalLoad::para_t
			case 5 :
			switch (t) {
				case 0 : return "PersonCount";
				case 1 : return "Power";
				case 2 : return "PowerPerArea";
				case 3 : return "ConvectiveHeatFactor";
				case 4 : return "LatentHeatFactor";
				case 5 : return "LossHeatFactor";
			} break;
			// InternalLoad::Category
			case 6 :
			switch (t) {
				case 0 : return "Person";
				case 1 : return "ElectricEquiment";
				case 2 : return "Lighting";
				case 3 : return "Other";
			} break;
			// InternalLoad::PersonCountMethod
			case 7 :
			switch (t) {
				case 0 : return "PersonPerArea";
				case 1 : return "AreaPerPerson";
				case 2 : return "PersonCount";
			} break;
			// InternalLoad::PowerMethod
			case 8 :
			switch (t) {
				case 0 : return "PowerPerArea";
				case 1 : return "Power";
			} break;
			// KeywordList::MyParameters
			case 9 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "Mass";
			} break;
			// Material::para_t
			case 10 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
				case 3 : return "Mu";
				case 4 : return "W80";
				case 5 : return "Wsat";
			} break;
			// Material::Category
			case 11 :
			switch (t) {
				case 0 : return "Coating";
				case 1 : return "Plaster";
				case 2 : return "Bricks";
				case 3 : return "NaturalStones";
				case 4 : return "Cementitious";
				case 5 : return "Insulations";
				case 6 : return "BuildingBoards";
				case 7 : return "Woodbased";
				case 8 : return "NaturalMaterials";
				case 9 : return "Soils";
				case 10 : return "CladdingSystems";
				case 11 : return "Foils";
				case 12 : return "Miscellaneous";
			} break;
			// Network::NetworkType
			case 12 :
			switch (t) {
				case 0 : return "SinglePipe";
				case 1 : return "DoublePipe";
			} break;
			// Network::para_t
			case 13 :
			switch (t) {
				case 0 : return "TemperatureSetpoint";
				case 1 : return "TemperatureDifference";
				case 2 : return "MaxPressureLoss";
				case 3 : return "ReferencePressure";
				case 4 : return "DefaultFluidTemperature";
				case 5 : return "InitialFluidTemperature";
			} break;
			// NetworkComponent::ModelType
			case 14 :
			switch (t) {
				case 0 : return "SimplePipe";
				case 1 : return "DynamicPipe";
				case 2 : return "ConstantPressurePump";
				case 3 : return "HeatExchanger";
				case 4 : return "HeatPumpIdealCarnot";
			} break;
			// NetworkComponent::para_t
			case 15 :
			switch (t) {
				case 0 : return "HydraulicDiameter";
				case 1 : return "PressureLossCoefficient";
				case 2 : return "PressureHead";
				case 3 : return "PumpEfficiency";
				case 4 : return "Volume";
				case 5 : return "PipeMaxDiscretizationWidth";
				case 6 : return "CarnotEfficiency";
				case 7 : return "CondenserMeanTemperature";
			} break;
			// NetworkFluid::para_t
			case 16 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// NetworkHeatExchange::ModelType
			case 17 :
			switch (t) {
				case 0 : return "TemperatureConstant";
				case 1 : return "TemperatureSpline";
				case 2 : return "HeatLossConstant";
				case 3 : return "HeatLossSpline";
				case 4 : return "HeatLossSplineCondenser";
				case 5 : return "TemperatureZone";
				case 6 : return "TemperatureConstructionLayer";
				case 7 : return "TemperatureFMUInterface";
			} break;
			// NetworkHeatExchange::para_t
			case 18 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "HeatLoss";
				case 2 : return "ExternalHeatTransferCoefficient";
			} break;
			// NetworkHeatExchange::splinePara_t
			case 19 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "HeatLoss";
			} break;
			// NetworkHeatExchange::References
			case 20 :
			switch (t) {
				case 0 : return "ZoneId";
				case 1 : return "ConstructionInstanceId";
			} break;
			// NetworkNode::NodeType
			case 21 :
			switch (t) {
				case 0 : return "Building";
				case 1 : return "Mixer";
				case 2 : return "Source";
			} break;
			// Outputs::flag_t
			case 22 :
			switch (t) {
				case 0 : return "BinaryOutputs";
				case 1 : return "CreateDefaultZoneOutputs";
			} break;
			// PlaneGeometry::type_t
			case 23 :
			switch (t) {
				case 0 : return "Triangle";
				case 1 : return "Rectangle";
				case 2 : return "Polygon";
			} break;
			// Room::para_t
			case 24 :
			switch (t) {
				case 0 : return "Area";
				case 1 : return "Volume";
			} break;
			// SurfaceProperties::para_t
			case 25 :
			switch (t) {
				case 0 : return "Specularity";
				case 1 : return "Roughness";
			} break;
			// SurfaceProperties::Type
			case 26 :
			switch (t) {
				case 0 : return "Plastic";
				case 1 : return "Metal";
				case 2 : return "Glass";
			} break;
			// ViewSettings::Flags
			case 27 :
			switch (t) {
				case 0 : return "GridVisible";
			} break;
			// WindowDivider::para_t
			case 28 :
			switch (t) {
				case 0 : return "Area";
			} break;
			// WindowGlazingLayer::type_t
			case 29 :
			switch (t) {
				case 0 : return "Gas";
				case 1 : return "Glass";
			} break;
			// WindowGlazingLayer::para_t
			case 30 :
			switch (t) {
				case 0 : return "Thickness";
				case 1 : return "Conductivity";
				case 2 : return "MassDensity";
				case 3 : return "Height";
				case 4 : return "Width";
				case 5 : return "LongWaveEmissivityInside";
				case 6 : return "LongWaveEmissivityOutside";
			} break;
			// WindowGlazingLayer::splinePara_t
			case 31 :
			switch (t) {
				case 0 : return "ShortWaveTransmittance";
				case 1 : return "ShortWaveReflectanceOutside";
				case 2 : return "ShortWaveReflectanceInside";
				case 3 : return "Conductivity";
				case 4 : return "DynamicViscosity";
				case 5 : return "HeatCapacity";
			} break;
			// WindowGlazingSystem::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "Simple";
				case 1 : return "Detailed";
			} break;
			// WindowGlazingSystem::para_t
			case 33 :
			switch (t) {
				case 0 : return "ThermalTransmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 34 :
			switch (t) {
				case 0 : return "SHGC";
			} break;
			// ZoneTemplate::SubTemplateType
			case 35 :
			switch (t) {
				case 0 : return "IntLoadPerson";
				case 1 : return "IntLoadEquipment";
				case 2 : return "IntLoadLighting";
				case 3 : return "IntLoadOther";
				case 4 : return "ControlThermostat";
			} break;
		} // switch
		return INVALID_KEYWORD_INDEX_STRING;
	}

	/*! Returns all keywords including deprecated for a given category (typenum) and type number (t). */
	const char * allKeywords(int typenum, int t) {
		switch (typenum) {
			// Component::ComponentType
			case 0 :
			switch (t) {
				case 0 : return "OutsideWall";
				case 1 : return "OutsideWallToGround";
				case 2 : return "InsideWall";
				case 3 : return "FloorToCellar";
				case 4 : return "FloorToAir";
				case 5 : return "FloorToGround";
				case 6 : return "Ceiling";
				case 7 : return "SlopedRoof";
				case 8 : return "FlatRoof";
				case 9 : return "ColdRoof";
				case 10 : return "WarmRoof";
				case 11 : return "Miscellaneous";
			} break;
			// Construction::UsageType
			case 1 :
			switch (t) {
				case 0 : return "OutsideWall";
				case 1 : return "OutsideWallToGround";
				case 2 : return "InsideWall";
				case 3 : return "FloorToCellar";
				case 4 : return "FloorToGround";
				case 5 : return "Ceiling";
				case 6 : return "SlopedRoof";
				case 7 : return "FlatRoof";
				case 8 : return "---";
			} break;
			// Construction::InsulationKind
			case 2 :
			switch (t) {
				case 0 : return "NotInsulated";
				case 1 : return "InsideInsulation";
				case 2 : return "CoreInsulation";
				case 3 : return "OutsideInsulation";
				case 4 : return "---";
			} break;
			// Construction::MaterialKind
			case 3 :
			switch (t) {
				case 0 : return "BrickMasonry";
				case 1 : return "NaturalStoneMasonry";
				case 2 : return "Concrete";
				case 3 : return "Wood";
				case 4 : return "FrameWork";
				case 5 : return "Loam";
				case 6 : return "---";
			} break;
			// EPDDataset::para_t
			case 4 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "GWP";
				case 2 : return "ODP";
				case 3 : return "POCP";
				case 4 : return "AP";
				case 5 : return "EP";
				case 6 : return "PENRT";
				case 7 : return "PERT";
			} break;
			// InternalLoad::para_t
			case 5 :
			switch (t) {
				case 0 : return "PersonCount";
				case 1 : return "Power";
				case 2 : return "PowerPerArea";
				case 3 : return "ConvectiveHeatFactor";
				case 4 : return "LatentHeatFactor";
				case 5 : return "LossHeatFactor";
			} break;
			// InternalLoad::Category
			case 6 :
			switch (t) {
				case 0 : return "Person";
				case 1 : return "ElectricEquiment";
				case 2 : return "Lighting";
				case 3 : return "Other";
			} break;
			// InternalLoad::PersonCountMethod
			case 7 :
			switch (t) {
				case 0 : return "PersonPerArea";
				case 1 : return "AreaPerPerson";
				case 2 : return "PersonCount";
			} break;
			// InternalLoad::PowerMethod
			case 8 :
			switch (t) {
				case 0 : return "PowerPerArea";
				case 1 : return "Power";
			} break;
			// KeywordList::MyParameters
			case 9 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "Mass";
			} break;
			// Material::para_t
			case 10 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
				case 3 : return "Mu";
				case 4 : return "W80";
				case 5 : return "Wsat";
			} break;
			// Material::Category
			case 11 :
			switch (t) {
				case 0 : return "Coating";
				case 1 : return "Plaster";
				case 2 : return "Bricks";
				case 3 : return "NaturalStones";
				case 4 : return "Cementitious";
				case 5 : return "Insulations";
				case 6 : return "BuildingBoards";
				case 7 : return "Woodbased";
				case 8 : return "NaturalMaterials";
				case 9 : return "Soils";
				case 10 : return "CladdingSystems";
				case 11 : return "Foils";
				case 12 : return "Miscellaneous";
			} break;
			// Network::NetworkType
			case 12 :
			switch (t) {
				case 0 : return "SinglePipe";
				case 1 : return "DoublePipe";
			} break;
			// Network::para_t
			case 13 :
			switch (t) {
				case 0 : return "TemperatureSetpoint";
				case 1 : return "TemperatureDifference";
				case 2 : return "MaxPressureLoss";
				case 3 : return "ReferencePressure";
				case 4 : return "DefaultFluidTemperature";
				case 5 : return "InitialFluidTemperature";
			} break;
			// NetworkComponent::ModelType
			case 14 :
			switch (t) {
				case 0 : return "SimplePipe";
				case 1 : return "DynamicPipe";
				case 2 : return "ConstantPressurePump";
				case 3 : return "HeatExchanger";
				case 4 : return "HeatPumpIdealCarnot";
			} break;
			// NetworkComponent::para_t
			case 15 :
			switch (t) {
				case 0 : return "HydraulicDiameter";
				case 1 : return "PressureLossCoefficient";
				case 2 : return "PressureHead";
				case 3 : return "PumpEfficiency";
				case 4 : return "Volume";
				case 5 : return "PipeMaxDiscretizationWidth";
				case 6 : return "CarnotEfficiency";
				case 7 : return "CondenserMeanTemperature";
			} break;
			// NetworkFluid::para_t
			case 16 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// NetworkHeatExchange::ModelType
			case 17 :
			switch (t) {
				case 0 : return "TemperatureConstant";
				case 1 : return "TemperatureSpline";
				case 2 : return "HeatLossConstant";
				case 3 : return "HeatLossSpline";
				case 4 : return "HeatLossSplineCondenser";
				case 5 : return "TemperatureZone";
				case 6 : return "TemperatureConstructionLayer";
				case 7 : return "TemperatureFMUInterface";
			} break;
			// NetworkHeatExchange::para_t
			case 18 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "HeatLoss";
				case 2 : return "ExternalHeatTransferCoefficient";
			} break;
			// NetworkHeatExchange::splinePara_t
			case 19 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "HeatLoss";
			} break;
			// NetworkHeatExchange::References
			case 20 :
			switch (t) {
				case 0 : return "ZoneId";
				case 1 : return "ConstructionInstanceId";
			} break;
			// NetworkNode::NodeType
			case 21 :
			switch (t) {
				case 0 : return "Building";
				case 1 : return "Mixer";
				case 2 : return "Source";
			} break;
			// Outputs::flag_t
			case 22 :
			switch (t) {
				case 0 : return "BinaryOutputs";
				case 1 : return "CreateDefaultZoneOutputs";
			} break;
			// PlaneGeometry::type_t
			case 23 :
			switch (t) {
				case 0 : return "Triangle";
				case 1 : return "Rectangle";
				case 2 : return "Polygon";
			} break;
			// Room::para_t
			case 24 :
			switch (t) {
				case 0 : return "Area";
				case 1 : return "Volume";
			} break;
			// SurfaceProperties::para_t
			case 25 :
			switch (t) {
				case 0 : return "Specularity";
				case 1 : return "Roughness";
			} break;
			// SurfaceProperties::Type
			case 26 :
			switch (t) {
				case 0 : return "Plastic";
				case 1 : return "Metal";
				case 2 : return "Glass";
			} break;
			// ViewSettings::Flags
			case 27 :
			switch (t) {
				case 0 : return "GridVisible";
			} break;
			// WindowDivider::para_t
			case 28 :
			switch (t) {
				case 0 : return "Area";
			} break;
			// WindowGlazingLayer::type_t
			case 29 :
			switch (t) {
				case 0 : return "Gas";
				case 1 : return "Glass";
			} break;
			// WindowGlazingLayer::para_t
			case 30 :
			switch (t) {
				case 0 : return "Thickness";
				case 1 : return "Conductivity";
				case 2 : return "MassDensity";
				case 3 : return "Height";
				case 4 : return "Width";
				case 5 : return "LongWaveEmissivityInside";
				case 6 : return "LongWaveEmissivityOutside";
			} break;
			// WindowGlazingLayer::splinePara_t
			case 31 :
			switch (t) {
				case 0 : return "ShortWaveTransmittance";
				case 1 : return "ShortWaveReflectanceOutside";
				case 2 : return "ShortWaveReflectanceInside";
				case 3 : return "Conductivity";
				case 4 : return "DynamicViscosity";
				case 5 : return "HeatCapacity";
			} break;
			// WindowGlazingSystem::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "Simple";
				case 1 : return "Detailed";
			} break;
			// WindowGlazingSystem::para_t
			case 33 :
			switch (t) {
				case 0 : return "ThermalTransmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 34 :
			switch (t) {
				case 0 : return "SHGC";
			} break;
			// ZoneTemplate::SubTemplateType
			case 35 :
			switch (t) {
				case 0 : return "IntLoadPerson";
				case 1 : return "IntLoadEquipment";
				case 2 : return "IntLoadLighting";
				case 3 : return "IntLoadOther";
				case 4 : return "ControlThermostat";
			} break;
		} // switch
		return INVALID_KEYWORD_INDEX_STRING;
	}

	const char * KeywordList::Description(const char * const enumtype, int t, bool * no_description) {
		if (no_description != nullptr)
			*no_description = false; // we are optimistic
		switch (enum2index(enumtype)) {
			// Component::ComponentType
			case 0 :
			switch (t) {
				case 0 : return "Outside wall construction";
				case 1 : return "Outside wall construction in contact with ground";
				case 2 : return "Interior construction";
				case 3 : return "Floor to basement";
				case 4 : return "Floor in contact with air";
				case 5 : return "Floor in contact with ground";
				case 6 : return "Ceiling construction";
				case 7 : return "Sloped roof construction";
				case 8 : return "Flat roof construction";
				case 9 : return "Flat roof construction (to heated/insulated space)";
				case 10 : return "Flat roof construction (to cold/ventilated space)";
				case 11 : return "Some other component type";
			} break;
			// Construction::UsageType
			case 1 :
			switch (t) {
				case 0 : return "Outside wall construction";
				case 1 : return "Outside wall construction in contact with ground";
				case 2 : return "Interior construction";
				case 3 : return "Floor to basement";
				case 4 : return "Floor in contact with ground";
				case 5 : return "Ceiling construction";
				case 6 : return "Sloped roof construction";
				case 7 : return "Flat roof construction";
				case 8 : return "Miscellaneous";
			} break;
			// Construction::InsulationKind
			case 2 :
			switch (t) {
				case 0 : return "Not insulated";
				case 1 : return "Inside insulated";
				case 2 : return "Core insulation";
				case 3 : return "Outside insulated";
				case 4 : return "Not selected";
			} break;
			// Construction::MaterialKind
			case 3 :
			switch (t) {
				case 0 : return "Brick masonry";
				case 1 : return "Natural stones";
				case 2 : return "Concrete";
				case 3 : return "Wood";
				case 4 : return "Frame construction";
				case 5 : return "Loam";
				case 6 : return "Not selected";
			} break;
			// EPDDataset::para_t
			case 4 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Global Warming Potential.";
				case 2 : return "Depletion potential of the stratospheric ozone layer.";
				case 3 : return "Photochemical Ozone Creation Potential.";
				case 4 : return "Acidification potential.";
				case 5 : return "Eutrophication potential.";
				case 6 : return "Total use of non-renewable primary energy resources.";
				case 7 : return "Total use of renewable primary energy resources .";
			} break;
			// InternalLoad::para_t
			case 5 :
			switch (t) {
				case 0 : return "Person Count.";
				case 1 : return "Power.";
				case 2 : return "Power per area.";
				case 3 : return "Convective Heat Factor.";
				case 4 : return "Latent Heat Factor.";
				case 5 : return "Loss Heat Factor.";
			} break;
			// InternalLoad::Category
			case 6 :
			switch (t) {
				case 0 : return "Person";
				case 1 : return "ElectricEquiment";
				case 2 : return "Lighting";
				case 3 : return "Other";
			} break;
			// InternalLoad::PersonCountMethod
			case 7 :
			switch (t) {
				case 0 : return "Person per m2";
				case 1 : return "m2 per Person";
				case 2 : return "Person count";
			} break;
			// InternalLoad::PowerMethod
			case 8 :
			switch (t) {
				case 0 : return "Power";
				case 1 : return "Power";
			} break;
			// KeywordList::MyParameters
			case 9 :
			switch (t) {
				case 0 : return "Some temperatures";
				case 1 : return "Some mass";
			} break;
			// Material::para_t
			case 10 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
				case 3 : return "Vapor diffusion resistance factor.";
				case 4 : return "Water content in relation to 80% humidity.";
				case 5 : return "Water content at saturation.";
			} break;
			// Material::Category
			case 11 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "Coating";
				case 1 : if (no_description != nullptr) *no_description = true; return "Plaster";
				case 2 : if (no_description != nullptr) *no_description = true; return "Bricks";
				case 3 : if (no_description != nullptr) *no_description = true; return "NaturalStones";
				case 4 : if (no_description != nullptr) *no_description = true; return "Cementitious";
				case 5 : if (no_description != nullptr) *no_description = true; return "Insulations";
				case 6 : if (no_description != nullptr) *no_description = true; return "BuildingBoards";
				case 7 : if (no_description != nullptr) *no_description = true; return "Woodbased";
				case 8 : if (no_description != nullptr) *no_description = true; return "NaturalMaterials";
				case 9 : if (no_description != nullptr) *no_description = true; return "Soils";
				case 10 : if (no_description != nullptr) *no_description = true; return "CladdingSystems";
				case 11 : if (no_description != nullptr) *no_description = true; return "Foils";
				case 12 : if (no_description != nullptr) *no_description = true; return "Miscellaneous";
			} break;
			// Network::NetworkType
			case 12 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "SinglePipe";
				case 1 : if (no_description != nullptr) *no_description = true; return "DoublePipe";
			} break;
			// Network::para_t
			case 13 :
			switch (t) {
				case 0 : return "Temperature for pipe dimensioning algorithm";
				case 1 : return "Temperature difference for pipe dimensioning algorithm";
				case 2 : return "Maximum pressure loss for pipe dimensioning algorithm";
				case 3 : return "Reference pressure applied to reference element";
				case 4 : return "Fluid temperature for hydraulic calculation, else initial temperature";
				case 5 : return "Initial Fluid temperature for thermo-hydraulic calculation";
			} break;
			// NetworkComponent::ModelType
			case 14 :
			switch (t) {
				case 0 : return "Simple pipe at stationary flow conditions with heat exchange";
				case 1 : return "Pipe with a discretized fluid volume and heat exchange";
				case 2 : return "Pump with constant pressure";
				case 3 : return "Simple heat exchanger with given heat flux";
				case 4 : return "Heat pump with unlimited heating power and constant carnot efficiency";
			} break;
			// NetworkComponent::para_t
			case 15 :
			switch (t) {
				case 0 : return "Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes).";
				case 1 : return "Pressure loss coefficient for the component (zeta-value).";
				case 2 : return "Pressure head form a pump.";
				case 3 : return "Pump efficiency.";
				case 4 : return "Water or air volume of the component.";
				case 5 : return "Maximum width of discretized volumes in pipe";
				case 6 : return "Carnot efficiency";
				case 7 : return "Mean fluid temperature in condenser";
			} break;
			// NetworkFluid::para_t
			case 16 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
			} break;
			// NetworkHeatExchange::ModelType
			case 17 :
			switch (t) {
				case 0 : return "Constant ambient temperature";
				case 1 : return "Time-dependent ambient temperature from spline";
				case 2 : return "Constant heat loss";
				case 3 : return "Heat loss from spline";
				case 4 : return "Heat loss of condenser in heat pump model";
				case 5 : return "Zone air temperature";
				case 6 : return "Active construction layer (floor heating)";
				case 7 : return "Temperature from FMU interface, provided heat flux to FMU";
			} break;
			// NetworkHeatExchange::para_t
			case 18 :
			switch (t) {
				case 0 : return "Temperature for heat exchange";
				case 1 : return "Constant heat flux out of the element (heat loss)";
				case 2 : return "External heat transfer coeffient for the outside boundary";
			} break;
			// NetworkHeatExchange::splinePara_t
			case 19 :
			switch (t) {
				case 0 : return "Temperature for heat exchange";
				case 1 : return "Heat flux out of the element (heat loss)";
			} break;
			// NetworkHeatExchange::References
			case 20 :
			switch (t) {
				case 0 : return "ID of coupled zone for thermal exchange";
				case 1 : return "ID of coupled construction instance for thermal exchange";
			} break;
			// NetworkNode::NodeType
			case 21 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "Building";
				case 1 : if (no_description != nullptr) *no_description = true; return "Mixer";
				case 2 : if (no_description != nullptr) *no_description = true; return "Source";
			} break;
			// Outputs::flag_t
			case 22 :
			switch (t) {
				case 0 : return "If true, output files are written in binary format (the default, if flag is missing).";
				case 1 : return "If true, default output definitions for zones are created.";
			} break;
			// PlaneGeometry::type_t
			case 23 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "Triangle";
				case 1 : if (no_description != nullptr) *no_description = true; return "Rectangle";
				case 2 : if (no_description != nullptr) *no_description = true; return "Polygon";
			} break;
			// Room::para_t
			case 24 :
			switch (t) {
				case 0 : return "Floor area of the zone.";
				case 1 : return "Volume of the zone.";
			} break;
			// SurfaceProperties::para_t
			case 25 :
			switch (t) {
				case 0 : return "Specularity of the material.";
				case 1 : return "Roughness of the material.";
			} break;
			// SurfaceProperties::Type
			case 26 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "Plastic";
				case 1 : if (no_description != nullptr) *no_description = true; return "Metal";
				case 2 : if (no_description != nullptr) *no_description = true; return "Glass";
			} break;
			// ViewSettings::Flags
			case 27 :
			switch (t) {
				case 0 : return "Grid is visible";
			} break;
			// WindowDivider::para_t
			case 28 :
			switch (t) {
				case 0 : return "Area of the divider.";
			} break;
			// WindowGlazingLayer::type_t
			case 29 :
			switch (t) {
				case 0 : return "Gas layer";
				case 1 : return "Glass layer";
			} break;
			// WindowGlazingLayer::para_t
			case 30 :
			switch (t) {
				case 0 : return "Thickness of the window layer.";
				case 1 : return "Thermal conductivity of the window layer.";
				case 2 : return "Mass density of the fill-in gas.";
				case 3 : return "Height of the detailed window.";
				case 4 : return "Width of the detailed window.";
				case 5 : return "Emissivity of surface facing outside.";
				case 6 : return "Emissivity of surface facing inside.";
			} break;
			// WindowGlazingLayer::splinePara_t
			case 31 :
			switch (t) {
				case 0 : return "Short wave transmittance at outside directed surface.";
				case 1 : return "Short wave reflectance of surface facing outside.";
				case 2 : return "Short wave reflectance of surface facing inside.";
				case 3 : return "Thermal conductivity of the gas layer.";
				case 4 : return "Dynamic viscosity of the gas layer.";
				case 5 : return "Specific heat capacity of the gas layer.";
			} break;
			// WindowGlazingSystem::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "Standard globbed-layers model.";
				case 1 : return "Detailed window model with layers.";
			} break;
			// WindowGlazingSystem::para_t
			case 33 :
			switch (t) {
				case 0 : return "Thermal transmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 34 :
			switch (t) {
				case 0 : return "Short wave transmittance at outside directed surface.";
			} break;
			// ZoneTemplate::SubTemplateType
			case 35 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "IntLoadPerson";
				case 1 : if (no_description != nullptr) *no_description = true; return "IntLoadEquipment";
				case 2 : if (no_description != nullptr) *no_description = true; return "IntLoadLighting";
				case 3 : if (no_description != nullptr) *no_description = true; return "IntLoadOther";
				case 4 : if (no_description != nullptr) *no_description = true; return "ControlThermostat";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine description for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Description]");
	}

	const char * KeywordList::Unit(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// Component::ComponentType
			case 0 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
				case 6 : return "";
				case 7 : return "";
				case 8 : return "";
				case 9 : return "";
				case 10 : return "";
				case 11 : return "";
			} break;
			// Construction::UsageType
			case 1 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
				case 6 : return "";
				case 7 : return "";
				case 8 : return "";
			} break;
			// Construction::InsulationKind
			case 2 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
			} break;
			// Construction::MaterialKind
			case 3 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
				case 6 : return "";
			} break;
			// EPDDataset::para_t
			case 4 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "kg";
				case 2 : return "kg";
				case 3 : return "kg";
				case 4 : return "kg";
				case 5 : return "kg";
				case 6 : return "W/mK";
				case 7 : return "W/mK";
			} break;
			// InternalLoad::para_t
			case 5 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "W";
				case 2 : return "W/m2";
				case 3 : return "---";
				case 4 : return "---";
				case 5 : return "---";
			} break;
			// InternalLoad::Category
			case 6 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
				case 2 : return "-";
				case 3 : return "-";
			} break;
			// InternalLoad::PersonCountMethod
			case 7 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
				case 2 : return "-";
			} break;
			// InternalLoad::PowerMethod
			case 8 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
			} break;
			// KeywordList::MyParameters
			case 9 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "kg";
			} break;
			// Material::para_t
			case 10 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
				case 3 : return "-";
				case 4 : return "kg/m3";
				case 5 : return "kg/m3";
			} break;
			// Material::Category
			case 11 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
				case 6 : return "";
				case 7 : return "";
				case 8 : return "";
				case 9 : return "";
				case 10 : return "";
				case 11 : return "";
				case 12 : return "";
			} break;
			// Network::NetworkType
			case 12 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Network::para_t
			case 13 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "K";
				case 2 : return "Pa/m";
				case 3 : return "Pa";
				case 4 : return "C";
				case 5 : return "C";
			} break;
			// NetworkComponent::ModelType
			case 14 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
			} break;
			// NetworkComponent::para_t
			case 15 :
			switch (t) {
				case 0 : return "mm";
				case 1 : return "-";
				case 2 : return "Pa";
				case 3 : return "---";
				case 4 : return "m3";
				case 5 : return "m";
				case 6 : return "---";
				case 7 : return "C";
			} break;
			// NetworkFluid::para_t
			case 16 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
			} break;
			// NetworkHeatExchange::ModelType
			case 17 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
				case 6 : return "";
				case 7 : return "";
			} break;
			// NetworkHeatExchange::para_t
			case 18 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "W";
				case 2 : return "W/m2K";
			} break;
			// NetworkHeatExchange::splinePara_t
			case 19 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "W";
			} break;
			// NetworkHeatExchange::References
			case 20 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
			} break;
			// NetworkNode::NodeType
			case 21 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Outputs::flag_t
			case 22 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// PlaneGeometry::type_t
			case 23 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Room::para_t
			case 24 :
			switch (t) {
				case 0 : return "m2";
				case 1 : return "m3";
			} break;
			// SurfaceProperties::para_t
			case 25 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
			} break;
			// SurfaceProperties::Type
			case 26 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// ViewSettings::Flags
			case 27 :
			switch (t) {
				case 0 : return "";
			} break;
			// WindowDivider::para_t
			case 28 :
			switch (t) {
				case 0 : return "m2";
			} break;
			// WindowGlazingLayer::type_t
			case 29 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// WindowGlazingLayer::para_t
			case 30 :
			switch (t) {
				case 0 : return "m";
				case 1 : return "W/mK";
				case 2 : return "kg/m3";
				case 3 : return "m";
				case 4 : return "m";
				case 5 : return "---";
				case 6 : return "---";
			} break;
			// WindowGlazingLayer::splinePara_t
			case 31 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "---";
				case 3 : return "W/mK";
				case 4 : return "kg/ms";
				case 5 : return "J/kgK";
			} break;
			// WindowGlazingSystem::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// WindowGlazingSystem::para_t
			case 33 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 34 :
			switch (t) {
				case 0 : return "---";
			} break;
			// ZoneTemplate::SubTemplateType
			case 35 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default unit for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Unit]");
	}

	const char * KeywordList::Color(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// Component::ComponentType
			case 0 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
				case 7 : return "#FFFFFF";
				case 8 : return "#FFFFFF";
				case 9 : return "#FFFFFF";
				case 10 : return "#FFFFFF";
				case 11 : return "#FFFFFF";
			} break;
			// Construction::UsageType
			case 1 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
				case 7 : return "#FFFFFF";
				case 8 : return "#FFFFFF";
			} break;
			// Construction::InsulationKind
			case 2 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// Construction::MaterialKind
			case 3 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
			} break;
			// EPDDataset::para_t
			case 4 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
				case 7 : return "#FFFFFF";
			} break;
			// InternalLoad::para_t
			case 5 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// InternalLoad::Category
			case 6 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// InternalLoad::PersonCountMethod
			case 7 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// InternalLoad::PowerMethod
			case 8 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// KeywordList::MyParameters
			case 9 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Material::para_t
			case 10 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// Material::Category
			case 11 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
				case 7 : return "#FFFFFF";
				case 8 : return "#FFFFFF";
				case 9 : return "#FFFFFF";
				case 10 : return "#FFFFFF";
				case 11 : return "#FFFFFF";
				case 12 : return "#FFFFFF";
			} break;
			// Network::NetworkType
			case 12 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Network::para_t
			case 13 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// NetworkComponent::ModelType
			case 14 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// NetworkComponent::para_t
			case 15 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
				case 7 : return "#FFFFFF";
			} break;
			// NetworkFluid::para_t
			case 16 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// NetworkHeatExchange::ModelType
			case 17 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
				case 7 : return "#FFFFFF";
			} break;
			// NetworkHeatExchange::para_t
			case 18 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// NetworkHeatExchange::splinePara_t
			case 19 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// NetworkHeatExchange::References
			case 20 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// NetworkNode::NodeType
			case 21 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Outputs::flag_t
			case 22 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// PlaneGeometry::type_t
			case 23 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Room::para_t
			case 24 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SurfaceProperties::para_t
			case 25 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SurfaceProperties::Type
			case 26 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// ViewSettings::Flags
			case 27 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowDivider::para_t
			case 28 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowGlazingLayer::type_t
			case 29 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// WindowGlazingLayer::para_t
			case 30 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
			} break;
			// WindowGlazingLayer::splinePara_t
			case 31 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::para_t
			case 33 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 34 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// ZoneTemplate::SubTemplateType
			case 35 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine color for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Color]");
	}

	double KeywordList::DefaultValue(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// Component::ComponentType
			case 0 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
				case 7 : return std::numeric_limits<double>::quiet_NaN();
				case 8 : return std::numeric_limits<double>::quiet_NaN();
				case 9 : return std::numeric_limits<double>::quiet_NaN();
				case 10 : return std::numeric_limits<double>::quiet_NaN();
				case 11 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Construction::UsageType
			case 1 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
				case 7 : return std::numeric_limits<double>::quiet_NaN();
				case 8 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Construction::InsulationKind
			case 2 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Construction::MaterialKind
			case 3 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// EPDDataset::para_t
			case 4 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
				case 7 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoad::para_t
			case 5 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoad::Category
			case 6 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoad::PersonCountMethod
			case 7 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoad::PowerMethod
			case 8 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// KeywordList::MyParameters
			case 9 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Material::para_t
			case 10 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Material::Category
			case 11 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
				case 7 : return std::numeric_limits<double>::quiet_NaN();
				case 8 : return std::numeric_limits<double>::quiet_NaN();
				case 9 : return std::numeric_limits<double>::quiet_NaN();
				case 10 : return std::numeric_limits<double>::quiet_NaN();
				case 11 : return std::numeric_limits<double>::quiet_NaN();
				case 12 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Network::NetworkType
			case 12 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Network::para_t
			case 13 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkComponent::ModelType
			case 14 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkComponent::para_t
			case 15 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
				case 7 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkFluid::para_t
			case 16 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkHeatExchange::ModelType
			case 17 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
				case 7 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkHeatExchange::para_t
			case 18 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkHeatExchange::splinePara_t
			case 19 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkHeatExchange::References
			case 20 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkNode::NodeType
			case 21 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Outputs::flag_t
			case 22 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// PlaneGeometry::type_t
			case 23 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Room::para_t
			case 24 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SurfaceProperties::para_t
			case 25 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SurfaceProperties::Type
			case 26 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ViewSettings::Flags
			case 27 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowDivider::para_t
			case 28 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingLayer::type_t
			case 29 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingLayer::para_t
			case 30 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingLayer::splinePara_t
			case 31 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::modelType_t
			case 32 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::para_t
			case 33 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::splinePara_t
			case 34 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneTemplate::SubTemplateType
			case 35 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default value for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::DefaultValue]");
	}

	// number of entries in a keyword list
	unsigned int KeywordList::Count(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// Component::ComponentType
			case 0 : return 12;
			// Construction::UsageType
			case 1 : return 9;
			// Construction::InsulationKind
			case 2 : return 5;
			// Construction::MaterialKind
			case 3 : return 7;
			// EPDDataset::para_t
			case 4 : return 8;
			// InternalLoad::para_t
			case 5 : return 6;
			// InternalLoad::Category
			case 6 : return 4;
			// InternalLoad::PersonCountMethod
			case 7 : return 3;
			// InternalLoad::PowerMethod
			case 8 : return 2;
			// KeywordList::MyParameters
			case 9 : return 2;
			// Material::para_t
			case 10 : return 6;
			// Material::Category
			case 11 : return 13;
			// Network::NetworkType
			case 12 : return 2;
			// Network::para_t
			case 13 : return 6;
			// NetworkComponent::ModelType
			case 14 : return 5;
			// NetworkComponent::para_t
			case 15 : return 8;
			// NetworkFluid::para_t
			case 16 : return 3;
			// NetworkHeatExchange::ModelType
			case 17 : return 8;
			// NetworkHeatExchange::para_t
			case 18 : return 3;
			// NetworkHeatExchange::splinePara_t
			case 19 : return 2;
			// NetworkHeatExchange::References
			case 20 : return 2;
			// NetworkNode::NodeType
			case 21 : return 3;
			// Outputs::flag_t
			case 22 : return 2;
			// PlaneGeometry::type_t
			case 23 : return 3;
			// Room::para_t
			case 24 : return 2;
			// SurfaceProperties::para_t
			case 25 : return 2;
			// SurfaceProperties::Type
			case 26 : return 3;
			// ViewSettings::Flags
			case 27 : return 1;
			// WindowDivider::para_t
			case 28 : return 1;
			// WindowGlazingLayer::type_t
			case 29 : return 2;
			// WindowGlazingLayer::para_t
			case 30 : return 7;
			// WindowGlazingLayer::splinePara_t
			case 31 : return 6;
			// WindowGlazingSystem::modelType_t
			case 32 : return 2;
			// WindowGlazingSystem::para_t
			case 33 : return 1;
			// WindowGlazingSystem::splinePara_t
			case 34 : return 1;
			// ZoneTemplate::SubTemplateType
			case 35 : return 5;
		} // switch
		throw IBK::Exception(IBK::FormatString("Invalid enumeration type '%1'.")
			.arg(enumtype), "[KeywordList::Count]");
	}

	// max index for entries sharing a category in a keyword list
	int KeywordList::MaxIndex(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// Component::ComponentType
			case 0 : return 11;
			// Construction::UsageType
			case 1 : return 8;
			// Construction::InsulationKind
			case 2 : return 4;
			// Construction::MaterialKind
			case 3 : return 6;
			// EPDDataset::para_t
			case 4 : return 7;
			// InternalLoad::para_t
			case 5 : return 5;
			// InternalLoad::Category
			case 6 : return 3;
			// InternalLoad::PersonCountMethod
			case 7 : return 2;
			// InternalLoad::PowerMethod
			case 8 : return 1;
			// KeywordList::MyParameters
			case 9 : return 1;
			// Material::para_t
			case 10 : return 5;
			// Material::Category
			case 11 : return 12;
			// Network::NetworkType
			case 12 : return 1;
			// Network::para_t
			case 13 : return 5;
			// NetworkComponent::ModelType
			case 14 : return 4;
			// NetworkComponent::para_t
			case 15 : return 7;
			// NetworkFluid::para_t
			case 16 : return 2;
			// NetworkHeatExchange::ModelType
			case 17 : return 7;
			// NetworkHeatExchange::para_t
			case 18 : return 2;
			// NetworkHeatExchange::splinePara_t
			case 19 : return 1;
			// NetworkHeatExchange::References
			case 20 : return 1;
			// NetworkNode::NodeType
			case 21 : return 2;
			// Outputs::flag_t
			case 22 : return 1;
			// PlaneGeometry::type_t
			case 23 : return 2;
			// Room::para_t
			case 24 : return 1;
			// SurfaceProperties::para_t
			case 25 : return 1;
			// SurfaceProperties::Type
			case 26 : return 2;
			// ViewSettings::Flags
			case 27 : return 0;
			// WindowDivider::para_t
			case 28 : return 0;
			// WindowGlazingLayer::type_t
			case 29 : return 1;
			// WindowGlazingLayer::para_t
			case 30 : return 6;
			// WindowGlazingLayer::splinePara_t
			case 31 : return 5;
			// WindowGlazingSystem::modelType_t
			case 32 : return 1;
			// WindowGlazingSystem::para_t
			case 33 : return 0;
			// WindowGlazingSystem::splinePara_t
			case 34 : return 0;
			// ZoneTemplate::SubTemplateType
			case 35 : return 5;
		} // switch
		throw IBK::Exception(IBK::FormatString("Invalid enumeration type '%1'.")
			.arg(enumtype), "[KeywordList::MaxIndex]");
	}

	const char * KeywordList::Keyword(const char * const enumtype, int t) {
		const char * const kw = theKeyword(enum2index(enumtype), t);
		if (std::string(kw) == INVALID_KEYWORD_INDEX_STRING) {
			throw IBK::Exception(IBK::FormatString("Cannot determine keyword for enumeration type '%1' and index '%2'.")
				.arg(enumtype).arg(t), "[KeywordList::Keyword]");
		}
		return kw;
	}

	bool KeywordList::KeywordExists(const char * const enumtype, const std::string & kw) {
		int typenum = enum2index(enumtype);
		int i = 0;
		int maxIndexInCategory = MaxIndex( enumtype ); 
		for ( ; i <= maxIndexInCategory; ++i ) {
			std::string keywords = allKeywords(typenum, i);
			if (keywords == INVALID_KEYWORD_INDEX_STRING)
				continue;
			std::stringstream strm(keywords);
			int j = 0;
			std::string kws;
			while (strm >> kws) {
				if (kws == kw) {
					return true; // keyword exists
				}
				++j;
			}
		}
		return false; // nothing found keyword doesn't exist.
	}

	int KeywordList::Enumeration(const char * const enumtype, const std::string & kw, bool * deprecated) {
		int typenum = enum2index(enumtype);
		int i = 0;
		int maxIndexInCategory = MaxIndex( enumtype ); 
		for ( ; i <= maxIndexInCategory; ++i ) {
			std::string keywords = allKeywords(typenum, i);
			if (keywords == INVALID_KEYWORD_INDEX_STRING)
				continue;
			std::stringstream strm(keywords);
			int j = 0;
			std::string kws;
			while (strm >> kws) {
				if (kws == kw) {
					if (deprecated != nullptr) {
						*deprecated = (j != 0);
					}
					return i;
				}
				++j;
			}
		} // for ( ; i < maxIndexInCategory; ++i ) {
		throw IBK::Exception(IBK::FormatString("Cannot determine enumeration value for "
			"enumeration type '%1' and keyword '%2'.")
			.arg(enumtype).arg(kw), "[KeywordList::Enumeration]");
	}

	bool KeywordList::CategoryExists(const char * const enumtype) {
		return enum2index(enumtype) != -1;
	}

	void KeywordList::setParameter(IBK::Parameter para[], const char * const enumtype, int n, const double &val) {
		para[n] = IBK::Parameter(Keyword(enumtype, n), val, Unit(enumtype, n));
	}

	void KeywordList::setIntPara(IBK::IntPara para[], const char * const enumtype, int n, const int &val) {
		para[n] = IBK::IntPara(Keyword(enumtype, n), val);
	}

} // namespace VICUS
