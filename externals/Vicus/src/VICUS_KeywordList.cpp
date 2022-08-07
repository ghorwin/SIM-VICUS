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
	const char * const ENUM_TYPES[58] = {
		"Component::ComponentType",
		"Construction::UsageType",
		"Construction::InsulationKind",
		"Construction::MaterialKind",
		"EPDDataset::para_t",
		"Infiltration::para_t",
		"Infiltration::AirChangeType",
		"InterfaceHeatConduction::modelType_t",
		"InterfaceHeatConduction::OtherZoneType",
		"InterfaceHeatConduction::para_t",
		"InternalLoad::para_t",
		"InternalLoad::Category",
		"InternalLoad::PersonCountMethod",
		"InternalLoad::PowerMethod",
		"KeywordList::MyParameters",
		"Material::para_t",
		"Material::Category",
		"Network::PipeModel",
		"Network::ModelType",
		"Network::NetworkType",
		"Network::para_t",
		"NetworkBuriedPipeProperties::SoilType",
		"NetworkBuriedPipeProperties::para_t",
		"NetworkComponent::ModelType",
		"NetworkComponent::para_t",
		"NetworkComponent::intPara_t",
		"NetworkController::ModelType",
		"NetworkController::ControlledProperty",
		"NetworkController::ControllerType",
		"NetworkController::para_t",
		"NetworkController::References",
		"NetworkFluid::para_t",
		"NetworkNode::NodeType",
		"NetworkPipe::para_t",
		"OutputDefinition::timeType_t",
		"Outputs::flag_t",
		"Room::para_t",
		"SubSurfaceComponent::SubSurfaceComponentType",
		"SupplySystem::supplyType_t",
		"SupplySystem::para_t",
		"SurfaceHeating::para_t",
		"SurfaceHeating::Type",
		"VentilationNatural::para_t",
		"Window::Method",
		"Window::para_t",
		"WindowDivider::para_t",
		"WindowFrame::para_t",
		"WindowGlazingSystem::modelType_t",
		"WindowGlazingSystem::para_t",
		"WindowGlazingSystem::splinePara_t",
		"ZoneControlNaturalVentilation::para_t",
		"ZoneControlShading::para_t",
		"ZoneControlShading::Category",
		"ZoneControlThermostat::para_t",
		"ZoneControlThermostat::ControlValue",
		"ZoneControlThermostat::ControllerType",
		"ZoneIdealHeatingCooling::para_t",
		"ZoneTemplate::SubTemplateType"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<58; ++i) {
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
			// Infiltration::para_t
			case 5 :
			switch (t) {
				case 0 : return "AirChangeRate";
				case 1 : return "ShieldingCoefficient";
			} break;
			// Infiltration::AirChangeType
			case 6 :
			switch (t) {
				case 0 : return "normal";
				case 1 : return "n50";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "None";
			} break;
			// InterfaceHeatConduction::OtherZoneType
			case 8 :
			switch (t) {
				case 0 : return "Standard";
				case 1 : return "Constant";
				case 2 : return "Scheduled";
			} break;
			// InterfaceHeatConduction::para_t
			case 9 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
				case 1 : return "ConstTemperature";
			} break;
			// InternalLoad::para_t
			case 10 :
			switch (t) {
				case 0 : return "PersonCount";
				case 1 : return "PersonPerArea";
				case 2 : return "AreaPerPerson";
				case 3 : return "Power";
				case 4 : return "PowerPerArea";
				case 5 : return "ConvectiveHeatFactor";
				case 6 : return "LatentHeatFactor";
				case 7 : return "LossHeatFactor";
			} break;
			// InternalLoad::Category
			case 11 :
			switch (t) {
				case 0 : return "Person";
				case 1 : return "ElectricEquiment";
				case 2 : return "Lighting";
				case 3 : return "Other";
			} break;
			// InternalLoad::PersonCountMethod
			case 12 :
			switch (t) {
				case 0 : return "PersonPerArea";
				case 1 : return "AreaPerPerson";
				case 2 : return "PersonCount";
			} break;
			// InternalLoad::PowerMethod
			case 13 :
			switch (t) {
				case 0 : return "PowerPerArea";
				case 1 : return "Power";
			} break;
			// KeywordList::MyParameters
			case 14 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "Mass";
			} break;
			// Material::para_t
			case 15 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
				case 3 : return "Mu";
				case 4 : return "W80";
				case 5 : return "Wsat";
			} break;
			// Material::Category
			case 16 :
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
			// Network::PipeModel
			case 17 :
			switch (t) {
				case 0 : return "SimplePipe";
				case 1 : return "DynamicPipe";
			} break;
			// Network::ModelType
			case 18 :
			switch (t) {
				case 0 : return "HydraulicNetwork";
				case 1 : return "ThermalHydraulicNetwork";
			} break;
			// Network::NetworkType
			case 19 :
			switch (t) {
				case 0 : return "SinglePipe";
				case 1 : return "DoublePipe";
			} break;
			// Network::para_t
			case 20 :
			switch (t) {
				case 0 : return "TemperatureSetpoint";
				case 1 : return "TemperatureDifference";
				case 2 : return "MaxPressureLoss";
				case 3 : return "ReferencePressure";
				case 4 : return "DefaultFluidTemperature";
				case 5 : return "InitialFluidTemperature";
				case 6 : return "MaxPipeDiscretization";
			} break;
			// NetworkBuriedPipeProperties::SoilType
			case 21 :
			switch (t) {
				case 0 : return "Sand";
				case 1 : return "Loam";
				case 2 : return "Silt";
			} break;
			// NetworkBuriedPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "PipeSpacing";
				case 1 : return "PipeDepth";
			} break;
			// NetworkComponent::ModelType
			case 23 :
			switch (t) {
				case 0 : return "SimplePipe";
				case 1 : return "DynamicPipe";
				case 2 : return "ConstantPressurePump";
				case 3 : return "ConstantMassFluxPump";
				case 4 : return "ControlledPump";
				case 5 : return "VariablePressurePump";
				case 6 : return "HeatExchanger";
				case 7 : return "HeatPumpVariableIdealCarnotSourceSide";
				case 8 : return "HeatPumpVariableIdealCarnotSupplySide";
				case 9 : return "HeatPumpVariableSourceSide";
				case 10 : return "HeatPumpOnOffSourceSide";
				case 11 : return "HeatPumpOnOffSourceSideWithBuffer";
				case 12 : return "ControlledValve";
				case 13 : return "IdealHeaterCooler";
				case 14 : return "ConstantPressureLossValve";
				case 15 : return "PressureLossElement";
			} break;
			// NetworkComponent::para_t
			case 24 :
			switch (t) {
				case 0 : return "HydraulicDiameter";
				case 1 : return "PressureLossCoefficient";
				case 2 : return "PressureHead";
				case 3 : return "MassFlux";
				case 4 : return "PumpMaximumEfficiency";
				case 5 : return "FractionOfMotorInefficienciesToFluidStream";
				case 6 : return "MaximumPressureHead";
				case 7 : return "PumpMaximumElectricalPower";
				case 8 : return "DesignPressureHead";
				case 9 : return "DesignMassFlux";
				case 10 : return "PressureHeadReductionFactor";
				case 11 : return "Volume";
				case 12 : return "PipeMaxDiscretizationWidth";
				case 13 : return "CarnotEfficiency";
				case 14 : return "MaximumHeatingPower";
				case 15 : return "PressureLoss";
				case 16 : return "MinimumOutletTemperature";
				case 17 : return "HeatingPowerB0W35";
				case 18 : return "HeatingBufferSupplyTemperature";
				case 19 : return "HeatingBufferReturnTemperature";
				case 20 : return "DHWBufferSupplyTemperature";
				case 21 : return "DHWBufferReturnTemperature";
				case 22 : return "HeatingBufferVolume";
				case 23 : return "DHWBufferVolume";
				case 24 : return "PipeLength";
			} break;
			// NetworkComponent::intPara_t
			case 25 :
			switch (t) {
				case 0 : return "NumberParallelPipes";
				case 1 : return "NumberParallelElements";
			} break;
			// NetworkController::ModelType
			case 26 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// NetworkController::ControlledProperty
			case 27 :
			switch (t) {
				case 0 : return "TemperatureDifference";
				case 1 : return "TemperatureDifferenceOfFollowingElement";
				case 2 : return "ThermostatValue";
				case 3 : return "MassFlux";
				case 4 : return "PumpOperation";
				case 5 : return "PressureDifferenceWorstpoint";
			} break;
			// NetworkController::ControllerType
			case 28 :
			switch (t) {
				case 0 : return "PController";
				case 1 : return "PIController";
				case 2 : return "PIDController";
				case 3 : return "OnOffController";
			} break;
			// NetworkController::para_t
			case 29 :
			switch (t) {
				case 0 : return "Kp";
				case 1 : return "Ki";
				case 2 : return "Kd";
				case 3 : return "TemperatureDifferenceSetpoint";
				case 4 : return "MassFluxSetpoint";
				case 5 : return "HeatLossOfFollowingElementThreshold";
				case 6 : return "RelControllerErrorForIntegratorReset";
				case 7 : return "PressureDifferenceSetpoint";
			} break;
			// NetworkController::References
			case 30 :
			switch (t) {
				case 0 : return "ThermostatZone";
				case 1 : return "Schedule";
			} break;
			// NetworkFluid::para_t
			case 31 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// NetworkNode::NodeType
			case 32 :
			switch (t) {
				case 0 : return "Building";
				case 1 : return "Mixer";
				case 2 : return "Source";
			} break;
			// NetworkPipe::para_t
			case 33 :
			switch (t) {
				case 0 : return "DiameterOutside";
				case 1 : return "ThicknessWall";
				case 2 : return "RoughnessWall";
				case 3 : return "ThermalConductivityWall";
				case 4 : return "HeatCapacityWall";
				case 5 : return "DensityWall";
				case 6 : return "ThicknessInsulation";
				case 7 : return "ThermalConductivityInsulation";
			} break;
			// OutputDefinition::timeType_t
			case 34 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// Outputs::flag_t
			case 35 :
			switch (t) {
				case 0 : return "BinaryFormat";
				case 1 : return "CreateDefaultZoneOutputs";
				case 2 : return "CreateDefaultNetworkOutputs";
				case 3 : return "CreateDefaultNetworkSummationModels";
			} break;
			// Room::para_t
			case 36 :
			switch (t) {
				case 0 : return "Area";
				case 1 : return "Volume";
			} break;
			// SubSurfaceComponent::SubSurfaceComponentType
			case 37 :
			switch (t) {
				case 0 : return "Window";
				case 1 : return "Door";
				case 2 : return "Miscellaneous";
			} break;
			// SupplySystem::supplyType_t
			case 38 :
			switch (t) {
				case 0 : return "StandAlone";
				case 1 : return "SubNetwork";
				case 2 : return "DatabaseFMU";
				case 3 : return "UserDefinedFMU";
			} break;
			// SupplySystem::para_t
			case 39 :
			switch (t) {
				case 0 : return "MaximumMassFlux";
				case 1 : return "SupplyTemperature";
				case 2 : return "MaximumMassFluxFMU";
				case 3 : return "HeatingPowerFMU";
			} break;
			// SurfaceHeating::para_t
			case 40 :
			switch (t) {
				case 0 : return "HeatingLimit";
				case 1 : return "CoolingLimit";
				case 2 : return "PipeSpacing";
				case 3 : return "MaxFluidVelocity";
				case 4 : return "TemperatureDifferenceSupplyReturn";
			} break;
			// SurfaceHeating::Type
			case 41 :
			switch (t) {
				case 0 : return "Ideal";
				case 1 : return "PipeRegister";
			} break;
			// VentilationNatural::para_t
			case 42 :
			switch (t) {
				case 0 : return "AirChangeRate";
			} break;
			// Window::Method
			case 43 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Fraction";
				case 2 : return "ConstantWidth";
			} break;
			// Window::para_t
			case 44 :
			switch (t) {
				case 0 : return "FrameWidth";
				case 1 : return "FrameFraction";
				case 2 : return "DividerWidth";
				case 3 : return "DividerFraction";
			} break;
			// WindowDivider::para_t
			case 45 :
			switch (t) {
				case 0 : return "Thickness";
			} break;
			// WindowFrame::para_t
			case 46 :
			switch (t) {
				case 0 : return "Thickness";
			} break;
			// WindowGlazingSystem::modelType_t
			case 47 :
			switch (t) {
				case 0 : return "Simple";
			} break;
			// WindowGlazingSystem::para_t
			case 48 :
			switch (t) {
				case 0 : return "ThermalTransmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 49 :
			switch (t) {
				case 0 : return "SHGC";
			} break;
			// ZoneControlNaturalVentilation::para_t
			case 50 :
			switch (t) {
				case 0 : return "MaximumAirChangeRateComfort";
				case 1 : return "TemperatureAirMax";
				case 2 : return "TemperatureAirMin";
				case 3 : return "WindSpeedMax";
			} break;
			// ZoneControlShading::para_t
			case 51 :
			switch (t) {
				case 0 : return "GlobalHorizontal";
				case 1 : return "GlobalNorth";
				case 2 : return "GlobalEast";
				case 3 : return "GlobalSouth";
				case 4 : return "GlobalWest";
				case 5 : return "DeadBand";
			} break;
			// ZoneControlShading::Category
			case 52 :
			switch (t) {
				case 0 : return "GlobalHorizontalSensor";
				case 1 : return "GlobalHorizontalAndVerticalSensors";
			} break;
			// ZoneControlThermostat::para_t
			case 53 :
			switch (t) {
				case 0 : return "Tolerance";
				case 1 : return "DeadBand";
			} break;
			// ZoneControlThermostat::ControlValue
			case 54 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "OperativeTemperature";
			} break;
			// ZoneControlThermostat::ControllerType
			case 55 :
			switch (t) {
				case 0 : return "Analog";
				case 1 : return "Digital";
			} break;
			// ZoneIdealHeatingCooling::para_t
			case 56 :
			switch (t) {
				case 0 : return "HeatingLimit";
				case 1 : return "CoolingLimit";
			} break;
			// ZoneTemplate::SubTemplateType
			case 57 :
			switch (t) {
				case 0 : return "IntLoadPerson";
				case 1 : return "IntLoadEquipment";
				case 2 : return "IntLoadLighting";
				case 3 : return "IntLoadOther";
				case 4 : return "ControlThermostat";
				case 5 : return "ControlVentilationNatural";
				case 6 : return "Infiltration";
				case 7 : return "NaturalVentilation";
				case 8 : return "IdealHeatingCooling";
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
			// Infiltration::para_t
			case 5 :
			switch (t) {
				case 0 : return "AirChangeRate";
				case 1 : return "ShieldingCoefficient";
			} break;
			// Infiltration::AirChangeType
			case 6 :
			switch (t) {
				case 0 : return "normal";
				case 1 : return "n50";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "None";
			} break;
			// InterfaceHeatConduction::OtherZoneType
			case 8 :
			switch (t) {
				case 0 : return "Standard";
				case 1 : return "Constant";
				case 2 : return "Scheduled";
			} break;
			// InterfaceHeatConduction::para_t
			case 9 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
				case 1 : return "ConstTemperature";
			} break;
			// InternalLoad::para_t
			case 10 :
			switch (t) {
				case 0 : return "PersonCount";
				case 1 : return "PersonPerArea";
				case 2 : return "AreaPerPerson";
				case 3 : return "Power";
				case 4 : return "PowerPerArea";
				case 5 : return "ConvectiveHeatFactor";
				case 6 : return "LatentHeatFactor";
				case 7 : return "LossHeatFactor";
			} break;
			// InternalLoad::Category
			case 11 :
			switch (t) {
				case 0 : return "Person";
				case 1 : return "ElectricEquiment";
				case 2 : return "Lighting";
				case 3 : return "Other";
			} break;
			// InternalLoad::PersonCountMethod
			case 12 :
			switch (t) {
				case 0 : return "PersonPerArea";
				case 1 : return "AreaPerPerson";
				case 2 : return "PersonCount";
			} break;
			// InternalLoad::PowerMethod
			case 13 :
			switch (t) {
				case 0 : return "PowerPerArea";
				case 1 : return "Power";
			} break;
			// KeywordList::MyParameters
			case 14 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "Mass";
			} break;
			// Material::para_t
			case 15 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
				case 3 : return "Mu";
				case 4 : return "W80";
				case 5 : return "Wsat";
			} break;
			// Material::Category
			case 16 :
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
			// Network::PipeModel
			case 17 :
			switch (t) {
				case 0 : return "SimplePipe";
				case 1 : return "DynamicPipe";
			} break;
			// Network::ModelType
			case 18 :
			switch (t) {
				case 0 : return "HydraulicNetwork";
				case 1 : return "ThermalHydraulicNetwork";
			} break;
			// Network::NetworkType
			case 19 :
			switch (t) {
				case 0 : return "SinglePipe";
				case 1 : return "DoublePipe";
			} break;
			// Network::para_t
			case 20 :
			switch (t) {
				case 0 : return "TemperatureSetpoint";
				case 1 : return "TemperatureDifference";
				case 2 : return "MaxPressureLoss";
				case 3 : return "ReferencePressure";
				case 4 : return "DefaultFluidTemperature";
				case 5 : return "InitialFluidTemperature";
				case 6 : return "MaxPipeDiscretization";
			} break;
			// NetworkBuriedPipeProperties::SoilType
			case 21 :
			switch (t) {
				case 0 : return "Sand";
				case 1 : return "Loam";
				case 2 : return "Silt";
			} break;
			// NetworkBuriedPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "PipeSpacing";
				case 1 : return "PipeDepth";
			} break;
			// NetworkComponent::ModelType
			case 23 :
			switch (t) {
				case 0 : return "SimplePipe";
				case 1 : return "DynamicPipe";
				case 2 : return "ConstantPressurePump";
				case 3 : return "ConstantMassFluxPump";
				case 4 : return "ControlledPump";
				case 5 : return "VariablePressurePump";
				case 6 : return "HeatExchanger";
				case 7 : return "HeatPumpVariableIdealCarnotSourceSide";
				case 8 : return "HeatPumpVariableIdealCarnotSupplySide";
				case 9 : return "HeatPumpVariableSourceSide";
				case 10 : return "HeatPumpOnOffSourceSide";
				case 11 : return "HeatPumpOnOffSourceSideWithBuffer";
				case 12 : return "ControlledValve";
				case 13 : return "IdealHeaterCooler";
				case 14 : return "ConstantPressureLossValve";
				case 15 : return "PressureLossElement";
			} break;
			// NetworkComponent::para_t
			case 24 :
			switch (t) {
				case 0 : return "HydraulicDiameter";
				case 1 : return "PressureLossCoefficient";
				case 2 : return "PressureHead";
				case 3 : return "MassFlux";
				case 4 : return "PumpMaximumEfficiency";
				case 5 : return "FractionOfMotorInefficienciesToFluidStream";
				case 6 : return "MaximumPressureHead";
				case 7 : return "PumpMaximumElectricalPower";
				case 8 : return "DesignPressureHead";
				case 9 : return "DesignMassFlux";
				case 10 : return "PressureHeadReductionFactor";
				case 11 : return "Volume";
				case 12 : return "PipeMaxDiscretizationWidth";
				case 13 : return "CarnotEfficiency";
				case 14 : return "MaximumHeatingPower";
				case 15 : return "PressureLoss";
				case 16 : return "MinimumOutletTemperature";
				case 17 : return "HeatingPowerB0W35";
				case 18 : return "HeatingBufferSupplyTemperature";
				case 19 : return "HeatingBufferReturnTemperature";
				case 20 : return "DHWBufferSupplyTemperature";
				case 21 : return "DHWBufferReturnTemperature";
				case 22 : return "HeatingBufferVolume";
				case 23 : return "DHWBufferVolume";
				case 24 : return "PipeLength";
			} break;
			// NetworkComponent::intPara_t
			case 25 :
			switch (t) {
				case 0 : return "NumberParallelPipes";
				case 1 : return "NumberParallelElements";
			} break;
			// NetworkController::ModelType
			case 26 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// NetworkController::ControlledProperty
			case 27 :
			switch (t) {
				case 0 : return "TemperatureDifference";
				case 1 : return "TemperatureDifferenceOfFollowingElement";
				case 2 : return "ThermostatValue";
				case 3 : return "MassFlux";
				case 4 : return "PumpOperation";
				case 5 : return "PressureDifferenceWorstpoint";
			} break;
			// NetworkController::ControllerType
			case 28 :
			switch (t) {
				case 0 : return "PController";
				case 1 : return "PIController";
				case 2 : return "PIDController";
				case 3 : return "OnOffController";
			} break;
			// NetworkController::para_t
			case 29 :
			switch (t) {
				case 0 : return "Kp";
				case 1 : return "Ki";
				case 2 : return "Kd";
				case 3 : return "TemperatureDifferenceSetpoint";
				case 4 : return "MassFluxSetpoint";
				case 5 : return "HeatLossOfFollowingElementThreshold";
				case 6 : return "RelControllerErrorForIntegratorReset";
				case 7 : return "PressureDifferenceSetpoint";
			} break;
			// NetworkController::References
			case 30 :
			switch (t) {
				case 0 : return "ThermostatZone";
				case 1 : return "Schedule";
			} break;
			// NetworkFluid::para_t
			case 31 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// NetworkNode::NodeType
			case 32 :
			switch (t) {
				case 0 : return "Building";
				case 1 : return "Mixer";
				case 2 : return "Source";
			} break;
			// NetworkPipe::para_t
			case 33 :
			switch (t) {
				case 0 : return "DiameterOutside";
				case 1 : return "ThicknessWall";
				case 2 : return "RoughnessWall";
				case 3 : return "ThermalConductivityWall";
				case 4 : return "HeatCapacityWall";
				case 5 : return "DensityWall";
				case 6 : return "ThicknessInsulation";
				case 7 : return "ThermalConductivityInsulation";
			} break;
			// OutputDefinition::timeType_t
			case 34 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// Outputs::flag_t
			case 35 :
			switch (t) {
				case 0 : return "BinaryFormat";
				case 1 : return "CreateDefaultZoneOutputs";
				case 2 : return "CreateDefaultNetworkOutputs";
				case 3 : return "CreateDefaultNetworkSummationModels";
			} break;
			// Room::para_t
			case 36 :
			switch (t) {
				case 0 : return "Area";
				case 1 : return "Volume";
			} break;
			// SubSurfaceComponent::SubSurfaceComponentType
			case 37 :
			switch (t) {
				case 0 : return "Window";
				case 1 : return "Door";
				case 2 : return "Miscellaneous";
			} break;
			// SupplySystem::supplyType_t
			case 38 :
			switch (t) {
				case 0 : return "StandAlone";
				case 1 : return "SubNetwork";
				case 2 : return "DatabaseFMU";
				case 3 : return "UserDefinedFMU";
			} break;
			// SupplySystem::para_t
			case 39 :
			switch (t) {
				case 0 : return "MaximumMassFlux";
				case 1 : return "SupplyTemperature";
				case 2 : return "MaximumMassFluxFMU";
				case 3 : return "HeatingPowerFMU";
			} break;
			// SurfaceHeating::para_t
			case 40 :
			switch (t) {
				case 0 : return "HeatingLimit";
				case 1 : return "CoolingLimit";
				case 2 : return "PipeSpacing";
				case 3 : return "MaxFluidVelocity";
				case 4 : return "TemperatureDifferenceSupplyReturn";
			} break;
			// SurfaceHeating::Type
			case 41 :
			switch (t) {
				case 0 : return "Ideal";
				case 1 : return "PipeRegister";
			} break;
			// VentilationNatural::para_t
			case 42 :
			switch (t) {
				case 0 : return "AirChangeRate";
			} break;
			// Window::Method
			case 43 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Fraction";
				case 2 : return "ConstantWidth";
			} break;
			// Window::para_t
			case 44 :
			switch (t) {
				case 0 : return "FrameWidth";
				case 1 : return "FrameFraction";
				case 2 : return "DividerWidth";
				case 3 : return "DividerFraction";
			} break;
			// WindowDivider::para_t
			case 45 :
			switch (t) {
				case 0 : return "Thickness";
			} break;
			// WindowFrame::para_t
			case 46 :
			switch (t) {
				case 0 : return "Thickness";
			} break;
			// WindowGlazingSystem::modelType_t
			case 47 :
			switch (t) {
				case 0 : return "Simple";
			} break;
			// WindowGlazingSystem::para_t
			case 48 :
			switch (t) {
				case 0 : return "ThermalTransmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 49 :
			switch (t) {
				case 0 : return "SHGC";
			} break;
			// ZoneControlNaturalVentilation::para_t
			case 50 :
			switch (t) {
				case 0 : return "MaximumAirChangeRateComfort";
				case 1 : return "TemperatureAirMax";
				case 2 : return "TemperatureAirMin";
				case 3 : return "WindSpeedMax";
			} break;
			// ZoneControlShading::para_t
			case 51 :
			switch (t) {
				case 0 : return "GlobalHorizontal";
				case 1 : return "GlobalNorth";
				case 2 : return "GlobalEast";
				case 3 : return "GlobalSouth";
				case 4 : return "GlobalWest";
				case 5 : return "DeadBand";
			} break;
			// ZoneControlShading::Category
			case 52 :
			switch (t) {
				case 0 : return "GlobalHorizontalSensor";
				case 1 : return "GlobalHorizontalAndVerticalSensors";
			} break;
			// ZoneControlThermostat::para_t
			case 53 :
			switch (t) {
				case 0 : return "Tolerance";
				case 1 : return "DeadBand";
			} break;
			// ZoneControlThermostat::ControlValue
			case 54 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "OperativeTemperature";
			} break;
			// ZoneControlThermostat::ControllerType
			case 55 :
			switch (t) {
				case 0 : return "Analog";
				case 1 : return "Digital";
			} break;
			// ZoneIdealHeatingCooling::para_t
			case 56 :
			switch (t) {
				case 0 : return "HeatingLimit";
				case 1 : return "CoolingLimit";
			} break;
			// ZoneTemplate::SubTemplateType
			case 57 :
			switch (t) {
				case 0 : return "IntLoadPerson";
				case 1 : return "IntLoadEquipment";
				case 2 : return "IntLoadLighting";
				case 3 : return "IntLoadOther";
				case 4 : return "ControlThermostat";
				case 5 : return "ControlVentilationNatural";
				case 6 : return "Infiltration";
				case 7 : return "NaturalVentilation";
				case 8 : return "IdealHeatingCooling";
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
			// Infiltration::para_t
			case 5 :
			switch (t) {
				case 0 : return "Air change rate";
				case 1 : return "Shielding coefficient for n50 value";
			} break;
			// Infiltration::AirChangeType
			case 6 :
			switch (t) {
				case 0 : return "normal";
				case 1 : return "n50";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant heat exchange coefficient";
				case 1 : return "No convective heat exchange";
			} break;
			// InterfaceHeatConduction::OtherZoneType
			case 8 :
			switch (t) {
				case 0 : return "Active zone or outside";
				case 1 : return "Zone/ground with constant temperature";
				case 2 : return "Zone/ground with scheduled temperature";
			} break;
			// InterfaceHeatConduction::para_t
			case 9 :
			switch (t) {
				case 0 : return "Convective heat transfer coefficient";
				case 1 : return "Constant temperature of other zone/ground";
			} break;
			// InternalLoad::para_t
			case 10 :
			switch (t) {
				case 0 : return "Person count";
				case 1 : return "Person per area";
				case 2 : return "Area per person";
				case 3 : return "Power";
				case 4 : return "Power per area";
				case 5 : return "Convective heat factor";
				case 6 : return "Latent heat factor";
				case 7 : return "Loss heat factor";
			} break;
			// InternalLoad::Category
			case 11 :
			switch (t) {
				case 0 : return "Person";
				case 1 : return "ElectricEquiment";
				case 2 : return "Lighting";
				case 3 : return "Other";
			} break;
			// InternalLoad::PersonCountMethod
			case 12 :
			switch (t) {
				case 0 : return "Person per m2";
				case 1 : return "m2 per Person";
				case 2 : return "Person count";
			} break;
			// InternalLoad::PowerMethod
			case 13 :
			switch (t) {
				case 0 : return "Power per area";
				case 1 : return "Power";
			} break;
			// KeywordList::MyParameters
			case 14 :
			switch (t) {
				case 0 : return "Some temperatures";
				case 1 : return "Some mass";
			} break;
			// Material::para_t
			case 15 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
				case 3 : return "Vapor diffusion resistance factor.";
				case 4 : return "Water content in relation to 80% humidity.";
				case 5 : return "Water content at saturation.";
			} break;
			// Material::Category
			case 16 :
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
			// Network::PipeModel
			case 17 :
			switch (t) {
				case 0 : return "Pipe with a single fluid volume and with heat exchange";
				case 1 : return "Pipe with a discretized fluid volume and heat exchange";
			} break;
			// Network::ModelType
			case 18 :
			switch (t) {
				case 0 : return "Only Hydraulic calculation with constant temperature";
				case 1 : return "Thermo-hydraulic calculation";
			} break;
			// Network::NetworkType
			case 19 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "SinglePipe";
				case 1 : if (no_description != nullptr) *no_description = true; return "DoublePipe";
			} break;
			// Network::para_t
			case 20 :
			switch (t) {
				case 0 : return "Temperature for pipe dimensioning algorithm";
				case 1 : return "Temperature difference for pipe dimensioning algorithm";
				case 2 : return "Maximum pressure loss for pipe dimensioning algorithm";
				case 3 : return "Reference pressure applied to reference element";
				case 4 : return "Fluid temperature for hydraulic calculation, else initial temperature";
				case 5 : return "Initial Fluid temperature for thermo-hydraulic calculation";
				case 6 : return "Maximum discretization step for dynamic pipe model";
			} break;
			// NetworkBuriedPipeProperties::SoilType
			case 21 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "Sand";
				case 1 : if (no_description != nullptr) *no_description = true; return "Loam";
				case 2 : if (no_description != nullptr) *no_description = true; return "Silt";
			} break;
			// NetworkBuriedPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "Spacing between supply and return pipes";
				case 1 : return "Distance between soil surface and pipes";
			} break;
			// NetworkComponent::ModelType
			case 23 :
			switch (t) {
				case 0 : return "Pipe with a single fluid volume and with heat exchange";
				case 1 : return "Pipe with a discretized fluid volume and heat exchange";
				case 2 : return "Pump with constant/externally defined pressure";
				case 3 : return "Pump with constant/externally defined mass flux";
				case 4 : return "Pump with pressure head controlled based on flow controller";
				case 5 : return "Pump with linear pressure head curve (dp-v controlled pump)";
				case 6 : return "Simple heat exchanger with given heat flux";
				case 7 : return "Heat pump with variable heating power based on carnot efficiency, installed at source side (collector cycle)";
				case 8 : return "Heat pump with variable heating power based on carnot efficiency, installed at supply side";
				case 9 : return "Heat pump with variable heating power based on polynom for COP, installed at source side";
				case 10 : return "On-off-type heat pump based on polynoms for heating power and el. power, installed at source side";
				case 11 : return "On-off-type heat pump based on polynoms for heating power and el. power, installed at source side";
				case 12 : return "Valve with associated control model";
				case 13 : return "Ideal heat exchange model that provides a defined supply temperature to the network and calculates the heat loss/gain";
				case 14 : return "Valve with constant pressure loss";
				case 15 : return "Adiabatic element with pressure loss defined by zeta-value";
			} break;
			// NetworkComponent::para_t
			case 24 :
			switch (t) {
				case 0 : return "Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)";
				case 1 : return "Pressure loss coefficient for the component (zeta-value)";
				case 2 : return "Pump predefined pressure head";
				case 3 : return "Pump predefined mass flux";
				case 4 : return "Pump efficiency in optimal operation point";
				case 5 : return "Fraction of pump heat loss due to inefficiency that heats up the fluid";
				case 6 : return "Pump maximum pressure head at point of minimal mass flow of pump";
				case 7 : return "Pump maximum electrical power at point of optimal operation";
				case 8 : return "Design pressure head of VariablePressureHeadPump";
				case 9 : return "Design mass flux of VariablePressureHeadPump";
				case 10 : return "Factor to reduce pressure head of VariablePressureHeadPump";
				case 11 : return "Water or air volume of the component";
				case 12 : return "Maximum width/length of discretized volumes in pipe";
				case 13 : return "Carnot efficiency eta";
				case 14 : return "Maximum heating power";
				case 15 : return "Pressure loss for valve";
				case 16 : return "Minimum outlet temperature of heat exchanger, used for clipping of heat extraction";
				case 17 : if (no_description != nullptr) *no_description = true; return "HeatingPowerB0W35";
				case 18 : return "Heat pump supply temperature for heating buffer storage";
				case 19 : return "Heat pump return temperature for heating buffer storage";
				case 20 : return "Heat pump supply temperature for DHW buffer storage";
				case 21 : return "Heat pump return temperature for DHW buffer storage";
				case 22 : return "Heat pump heating buffer storage volume";
				case 23 : if (no_description != nullptr) *no_description = true; return "DHWBufferVolume";
				case 24 : return "Length of pipe";
			} break;
			// NetworkComponent::intPara_t
			case 25 :
			switch (t) {
				case 0 : return "Number of parallel pipes in ground heat exchanger";
				case 1 : return "Number of parallel elements";
			} break;
			// NetworkController::ModelType
			case 26 :
			switch (t) {
				case 0 : return "Set points are given as constant parameters";
				case 1 : return "Scheduled set point values";
			} break;
			// NetworkController::ControlledProperty
			case 27 :
			switch (t) {
				case 0 : return "Control temperature difference of this element";
				case 1 : return "Control temperature difference of the following element";
				case 2 : return "Control zone thermostat values";
				case 3 : return "Control mass flux";
				case 4 : return "Control pump operation depending on following element";
				case 5 : return "Control pressure difference at worst point in the network";
			} break;
			// NetworkController::ControllerType
			case 28 :
			switch (t) {
				case 0 : return "PController";
				case 1 : return "PIController";
				case 2 : return "PIDController";
				case 3 : return "OnOffController";
			} break;
			// NetworkController::para_t
			case 29 :
			switch (t) {
				case 0 : return "Kp-parameter";
				case 1 : return "Ki-parameter";
				case 2 : return "Kd-parameter";
				case 3 : return "Target temperature difference";
				case 4 : return "Target mass flux";
				case 5 : return "Threshold value for PumpOperation property when OnOffController is used";
				case 6 : return "Integral part will be set to zero if controller error is above this value";
				case 7 : return "Setpoint of pressure difference for worstpoint controller";
			} break;
			// NetworkController::References
			case 30 :
			switch (t) {
				case 0 : return "ID of zone containing thermostat";
				case 1 : return "ID of schedule";
			} break;
			// NetworkFluid::para_t
			case 31 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
			} break;
			// NetworkNode::NodeType
			case 32 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "Building";
				case 1 : if (no_description != nullptr) *no_description = true; return "Mixer";
				case 2 : if (no_description != nullptr) *no_description = true; return "Source";
			} break;
			// NetworkPipe::para_t
			case 33 :
			switch (t) {
				case 0 : return "Outer diameter (not including optional insulation)";
				case 1 : return "Pipe wall thickness";
				case 2 : return "Pipe wall surface roughness";
				case 3 : return "Thermal conductivity of pipe wall";
				case 4 : return "Specific heat capaciy of pipe wall";
				case 5 : return "Density of pipe wall";
				case 6 : return "Thickness of insulation around pipe";
				case 7 : return "Thermal conductivity of insulation";
			} break;
			// OutputDefinition::timeType_t
			case 34 :
			switch (t) {
				case 0 : return "Write values as calculated at output times.";
				case 1 : return "Average values in time (mean value in output step).";
				case 2 : return "Integrate values in time.";
			} break;
			// Outputs::flag_t
			case 35 :
			switch (t) {
				case 0 : return "If true, output files are written in binary format (the default, if flag is missing).";
				case 1 : return "If true, default output definitions for zones are created.";
				case 2 : return "If true, default output definitions for networks are created.";
				case 3 : return "If true, default summation models and according output definitions for networks are created.";
			} break;
			// Room::para_t
			case 36 :
			switch (t) {
				case 0 : return "Floor usable area of the zone";
				case 1 : return "Volume of the zone";
			} break;
			// SubSurfaceComponent::SubSurfaceComponentType
			case 37 :
			switch (t) {
				case 0 : return "A window";
				case 1 : return "A door";
				case 2 : return "Some other component type";
			} break;
			// SupplySystem::supplyType_t
			case 38 :
			switch (t) {
				case 0 : return "Stand-alone mode with given mass flux and suppply temperature";
				case 1 : return "VIVUS sub network loaded from a database and parametrized by the user";
				case 2 : return "Supply FMU loaded from a database and parametrized by the user";
				case 3 : return "User defined supply FMU";
			} break;
			// SupplySystem::para_t
			case 39 :
			switch (t) {
				case 0 : return "Maximum mass flux into the network, needed for pump control";
				case 1 : return "Constant supply temeprature";
				case 2 : return "Maximum mass flux towards the building.";
				case 3 : return "Procuder heating power";
			} break;
			// SurfaceHeating::para_t
			case 40 :
			switch (t) {
				case 0 : return "Heating limit";
				case 1 : return "Cooling limit";
				case 2 : return "Pipe spacing";
				case 3 : return "Maximum fluid velocity";
				case 4 : return "Temperature difference between supply and return fluid temperature";
			} break;
			// SurfaceHeating::Type
			case 41 :
			switch (t) {
				case 0 : return "Ideal surface conditioning";
				case 1 : return "Water-based surface conditioning: either ideal or part of a hydraulic network";
			} break;
			// VentilationNatural::para_t
			case 42 :
			switch (t) {
				case 0 : return "Air change rate";
			} break;
			// Window::Method
			case 43 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Fraction of area";
				case 2 : return "Constant width";
			} break;
			// Window::para_t
			case 44 :
			switch (t) {
				case 0 : return "Frame width of the window";
				case 1 : return "Frame area fraction of the window";
				case 2 : return "Divider width of the window";
				case 3 : return "Divider area fraction of the window";
			} break;
			// WindowDivider::para_t
			case 45 :
			switch (t) {
				case 0 : return "Divider material thickness.";
			} break;
			// WindowFrame::para_t
			case 46 :
			switch (t) {
				case 0 : return "Frame material thickness.";
			} break;
			// WindowGlazingSystem::modelType_t
			case 47 :
			switch (t) {
				case 0 : return "Standard globbed-layers model";
			} break;
			// WindowGlazingSystem::para_t
			case 48 :
			switch (t) {
				case 0 : return "Thermal transmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 49 :
			switch (t) {
				case 0 : return "Incidence-angle dependent short wave transmittance across glazing system";
			} break;
			// ZoneControlNaturalVentilation::para_t
			case 50 :
			switch (t) {
				case 0 : return "Maximum air change rate for natural ventilation to reach comfort conditions (if possible).";
				case 1 : return "Upper limit for room air temperature.";
				case 2 : return "Lower limit for room air temperature.";
				case 3 : return "Limit for wind speed .";
			} break;
			// ZoneControlShading::para_t
			case 51 :
			switch (t) {
				case 0 : return "Global horizontal (upper) sensor setpoint value.";
				case 1 : return "Global north (upper) sensor setpoint value.";
				case 2 : return "Global east (upper) sensor setpoint value.";
				case 3 : return "Global south (upper) sensor setpoint value.";
				case 4 : return "Global west (upper) sensor setpoint value.";
				case 5 : return "Dead band value for all sensors.";
			} break;
			// ZoneControlShading::Category
			case 52 :
			switch (t) {
				case 0 : return "One global horizontal sensor.";
				case 1 : return "One global horizontal and for each direction (N, E, S, W) a vertical sensor.";
			} break;
			// ZoneControlThermostat::para_t
			case 53 :
			switch (t) {
				case 0 : return "Thermostat tolerance heating and cooling mode";
				case 1 : return "Thermostat dead band (for digital controllers)";
			} break;
			// ZoneControlThermostat::ControlValue
			case 54 :
			switch (t) {
				case 0 : return "Air temperature";
				case 1 : return "Operative temperature";
			} break;
			// ZoneControlThermostat::ControllerType
			case 55 :
			switch (t) {
				case 0 : return "Analog";
				case 1 : return "Digital";
			} break;
			// ZoneIdealHeatingCooling::para_t
			case 56 :
			switch (t) {
				case 0 : return "Heating limit";
				case 1 : return "Cooling limit (positive)";
			} break;
			// ZoneTemplate::SubTemplateType
			case 57 :
			switch (t) {
				case 0 : return "Person loads";
				case 1 : return "Equipment loads";
				case 2 : return "Lighting loads";
				case 3 : return "Other internal loads";
				case 4 : return "Thermostat control";
				case 5 : return "Natural ventilation control";
				case 6 : return "Infiltration loads";
				case 7 : if (no_description != nullptr) *no_description = true; return "NaturalVentilation";
				case 8 : return "Heating/cooling loads";
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
			// Infiltration::para_t
			case 5 :
			switch (t) {
				case 0 : return "1/h";
				case 1 : return "-";
			} break;
			// Infiltration::AirChangeType
			case 6 :
			switch (t) {
				case 0 : return "1/h";
				case 1 : return "1/h";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// InterfaceHeatConduction::OtherZoneType
			case 8 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// InterfaceHeatConduction::para_t
			case 9 :
			switch (t) {
				case 0 : return "W/m2K";
				case 1 : return "C";
			} break;
			// InternalLoad::para_t
			case 10 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "Person/m2";
				case 2 : return "m2/Person";
				case 3 : return "W";
				case 4 : return "W/m2";
				case 5 : return "---";
				case 6 : return "---";
				case 7 : return "---";
			} break;
			// InternalLoad::Category
			case 11 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
				case 2 : return "-";
				case 3 : return "-";
			} break;
			// InternalLoad::PersonCountMethod
			case 12 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
				case 2 : return "-";
			} break;
			// InternalLoad::PowerMethod
			case 13 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
			} break;
			// KeywordList::MyParameters
			case 14 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "kg";
			} break;
			// Material::para_t
			case 15 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
				case 3 : return "-";
				case 4 : return "kg/m3";
				case 5 : return "kg/m3";
			} break;
			// Material::Category
			case 16 :
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
			// Network::PipeModel
			case 17 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Network::ModelType
			case 18 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Network::NetworkType
			case 19 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Network::para_t
			case 20 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "K";
				case 2 : return "Pa/m";
				case 3 : return "Pa";
				case 4 : return "C";
				case 5 : return "C";
				case 6 : return "m";
			} break;
			// NetworkBuriedPipeProperties::SoilType
			case 21 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// NetworkBuriedPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "m";
				case 1 : return "m";
			} break;
			// NetworkComponent::ModelType
			case 23 :
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
				case 13 : return "";
				case 14 : return "";
				case 15 : return "";
			} break;
			// NetworkComponent::para_t
			case 24 :
			switch (t) {
				case 0 : return "mm";
				case 1 : return "---";
				case 2 : return "Bar";
				case 3 : return "kg/s";
				case 4 : return "---";
				case 5 : return "---";
				case 6 : return "Bar";
				case 7 : return "W";
				case 8 : return "Bar";
				case 9 : return "kg/s";
				case 10 : return "---";
				case 11 : return "m3";
				case 12 : return "m";
				case 13 : return "---";
				case 14 : return "W";
				case 15 : return "Bar";
				case 16 : return "C";
				case 17 : return "W";
				case 18 : return "C";
				case 19 : return "C";
				case 20 : return "C";
				case 21 : return "C";
				case 22 : return "m3";
				case 23 : return "m3";
				case 24 : return "m";
			} break;
			// NetworkComponent::intPara_t
			case 25 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
			} break;
			// NetworkController::ModelType
			case 26 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// NetworkController::ControlledProperty
			case 27 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
			} break;
			// NetworkController::ControllerType
			case 28 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// NetworkController::para_t
			case 29 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "---";
				case 3 : return "K";
				case 4 : return "kg/s";
				case 5 : return "W";
				case 6 : return "---";
				case 7 : return "Pa";
			} break;
			// NetworkController::References
			case 30 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
			} break;
			// NetworkFluid::para_t
			case 31 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
			} break;
			// NetworkNode::NodeType
			case 32 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// NetworkPipe::para_t
			case 33 :
			switch (t) {
				case 0 : return "mm";
				case 1 : return "mm";
				case 2 : return "mm";
				case 3 : return "W/mK";
				case 4 : return "J/kgK";
				case 5 : return "kg/m3";
				case 6 : return "mm";
				case 7 : return "W/mK";
			} break;
			// OutputDefinition::timeType_t
			case 34 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Outputs::flag_t
			case 35 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// Room::para_t
			case 36 :
			switch (t) {
				case 0 : return "m2";
				case 1 : return "m3";
			} break;
			// SubSurfaceComponent::SubSurfaceComponentType
			case 37 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// SupplySystem::supplyType_t
			case 38 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SupplySystem::para_t
			case 39 :
			switch (t) {
				case 0 : return "kg/s";
				case 1 : return "C";
				case 2 : return "kg/s";
				case 3 : return "W";
			} break;
			// SurfaceHeating::para_t
			case 40 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
				case 2 : return "m";
				case 3 : return "m/s";
				case 4 : return "K";
			} break;
			// SurfaceHeating::Type
			case 41 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// VentilationNatural::para_t
			case 42 :
			switch (t) {
				case 0 : return "1/h";
			} break;
			// Window::Method
			case 43 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Window::para_t
			case 44 :
			switch (t) {
				case 0 : return "m";
				case 1 : return "---";
				case 2 : return "m";
				case 3 : return "---";
			} break;
			// WindowDivider::para_t
			case 45 :
			switch (t) {
				case 0 : return "m";
			} break;
			// WindowFrame::para_t
			case 46 :
			switch (t) {
				case 0 : return "m";
			} break;
			// WindowGlazingSystem::modelType_t
			case 47 :
			switch (t) {
				case 0 : return "";
			} break;
			// WindowGlazingSystem::para_t
			case 48 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 49 :
			switch (t) {
				case 0 : return "---";
			} break;
			// ZoneControlNaturalVentilation::para_t
			case 50 :
			switch (t) {
				case 0 : return "1/h";
				case 1 : return "C";
				case 2 : return "C";
				case 3 : return "m/s";
			} break;
			// ZoneControlShading::para_t
			case 51 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
				case 2 : return "W/m2";
				case 3 : return "W/m2";
				case 4 : return "W/m2";
				case 5 : return "W/m2";
			} break;
			// ZoneControlShading::Category
			case 52 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
			} break;
			// ZoneControlThermostat::para_t
			case 53 :
			switch (t) {
				case 0 : return "K";
				case 1 : return "K";
			} break;
			// ZoneControlThermostat::ControlValue
			case 54 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
			} break;
			// ZoneControlThermostat::ControllerType
			case 55 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// ZoneIdealHeatingCooling::para_t
			case 56 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
			} break;
			// ZoneTemplate::SubTemplateType
			case 57 :
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
			// Infiltration::para_t
			case 5 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Infiltration::AirChangeType
			case 6 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::OtherZoneType
			case 8 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::para_t
			case 9 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// InternalLoad::para_t
			case 10 :
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
			// InternalLoad::Category
			case 11 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// InternalLoad::PersonCountMethod
			case 12 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// InternalLoad::PowerMethod
			case 13 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// KeywordList::MyParameters
			case 14 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Material::para_t
			case 15 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// Material::Category
			case 16 :
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
			// Network::PipeModel
			case 17 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Network::ModelType
			case 18 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Network::NetworkType
			case 19 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Network::para_t
			case 20 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
			} break;
			// NetworkBuriedPipeProperties::SoilType
			case 21 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// NetworkBuriedPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// NetworkComponent::ModelType
			case 23 :
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
				case 13 : return "#FFFFFF";
				case 14 : return "#FFFFFF";
				case 15 : return "#FFFFFF";
			} break;
			// NetworkComponent::para_t
			case 24 :
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
				case 13 : return "#FFFFFF";
				case 14 : return "#FFFFFF";
				case 15 : return "#FFFFFF";
				case 16 : return "#FFFFFF";
				case 17 : return "#FFFFFF";
				case 18 : return "#FFFFFF";
				case 19 : return "#FFFFFF";
				case 20 : return "#FFFFFF";
				case 21 : return "#FFFFFF";
				case 22 : return "#FFFFFF";
				case 23 : return "#FFFFFF";
				case 24 : return "#FFFFFF";
			} break;
			// NetworkComponent::intPara_t
			case 25 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// NetworkController::ModelType
			case 26 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// NetworkController::ControlledProperty
			case 27 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// NetworkController::ControllerType
			case 28 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// NetworkController::para_t
			case 29 :
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
			// NetworkController::References
			case 30 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// NetworkFluid::para_t
			case 31 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// NetworkNode::NodeType
			case 32 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// NetworkPipe::para_t
			case 33 :
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
			// OutputDefinition::timeType_t
			case 34 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Outputs::flag_t
			case 35 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Room::para_t
			case 36 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SubSurfaceComponent::SubSurfaceComponentType
			case 37 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// SupplySystem::supplyType_t
			case 38 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SupplySystem::para_t
			case 39 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SurfaceHeating::para_t
			case 40 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// SurfaceHeating::Type
			case 41 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// VentilationNatural::para_t
			case 42 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// Window::Method
			case 43 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Window::para_t
			case 44 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// WindowDivider::para_t
			case 45 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowFrame::para_t
			case 46 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::modelType_t
			case 47 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::para_t
			case 48 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 49 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// ZoneControlNaturalVentilation::para_t
			case 50 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// ZoneControlShading::para_t
			case 51 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// ZoneControlShading::Category
			case 52 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ZoneControlThermostat::para_t
			case 53 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ZoneControlThermostat::ControlValue
			case 54 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ZoneControlThermostat::ControllerType
			case 55 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ZoneIdealHeatingCooling::para_t
			case 56 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ZoneTemplate::SubTemplateType
			case 57 :
			switch (t) {
				case 0 : return "#FFBB55";
				case 1 : return "#AA2222";
				case 2 : return "#FFEECC";
				case 3 : return "#602222";
				case 4 : return "#E00010";
				case 5 : return "#00A000";
				case 6 : return "#A0B0FF";
				case 7 : return "#22EE22";
				case 8 : return "#B08000";
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
			// Infiltration::para_t
			case 5 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Infiltration::AirChangeType
			case 6 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::OtherZoneType
			case 8 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::para_t
			case 9 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoad::para_t
			case 10 :
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
			// InternalLoad::Category
			case 11 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoad::PersonCountMethod
			case 12 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoad::PowerMethod
			case 13 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// KeywordList::MyParameters
			case 14 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Material::para_t
			case 15 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Material::Category
			case 16 :
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
			// Network::PipeModel
			case 17 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Network::ModelType
			case 18 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Network::NetworkType
			case 19 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Network::para_t
			case 20 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkBuriedPipeProperties::SoilType
			case 21 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkBuriedPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkComponent::ModelType
			case 23 :
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
				case 13 : return std::numeric_limits<double>::quiet_NaN();
				case 14 : return std::numeric_limits<double>::quiet_NaN();
				case 15 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkComponent::para_t
			case 24 :
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
				case 13 : return std::numeric_limits<double>::quiet_NaN();
				case 14 : return std::numeric_limits<double>::quiet_NaN();
				case 15 : return std::numeric_limits<double>::quiet_NaN();
				case 16 : return std::numeric_limits<double>::quiet_NaN();
				case 17 : return std::numeric_limits<double>::quiet_NaN();
				case 18 : return std::numeric_limits<double>::quiet_NaN();
				case 19 : return std::numeric_limits<double>::quiet_NaN();
				case 20 : return std::numeric_limits<double>::quiet_NaN();
				case 21 : return std::numeric_limits<double>::quiet_NaN();
				case 22 : return std::numeric_limits<double>::quiet_NaN();
				case 23 : return std::numeric_limits<double>::quiet_NaN();
				case 24 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkComponent::intPara_t
			case 25 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkController::ModelType
			case 26 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkController::ControlledProperty
			case 27 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkController::ControllerType
			case 28 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkController::para_t
			case 29 :
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
			// NetworkController::References
			case 30 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkFluid::para_t
			case 31 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkNode::NodeType
			case 32 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkPipe::para_t
			case 33 :
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
			// OutputDefinition::timeType_t
			case 34 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Outputs::flag_t
			case 35 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Room::para_t
			case 36 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SubSurfaceComponent::SubSurfaceComponentType
			case 37 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SupplySystem::supplyType_t
			case 38 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SupplySystem::para_t
			case 39 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SurfaceHeating::para_t
			case 40 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SurfaceHeating::Type
			case 41 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// VentilationNatural::para_t
			case 42 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Window::Method
			case 43 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Window::para_t
			case 44 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowDivider::para_t
			case 45 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowFrame::para_t
			case 46 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::modelType_t
			case 47 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::para_t
			case 48 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::splinePara_t
			case 49 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneControlNaturalVentilation::para_t
			case 50 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneControlShading::para_t
			case 51 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneControlShading::Category
			case 52 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneControlThermostat::para_t
			case 53 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneControlThermostat::ControlValue
			case 54 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneControlThermostat::ControllerType
			case 55 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneIdealHeatingCooling::para_t
			case 56 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneTemplate::SubTemplateType
			case 57 :
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
			// Infiltration::para_t
			case 5 : return 2;
			// Infiltration::AirChangeType
			case 6 : return 2;
			// InterfaceHeatConduction::modelType_t
			case 7 : return 2;
			// InterfaceHeatConduction::OtherZoneType
			case 8 : return 3;
			// InterfaceHeatConduction::para_t
			case 9 : return 2;
			// InternalLoad::para_t
			case 10 : return 8;
			// InternalLoad::Category
			case 11 : return 4;
			// InternalLoad::PersonCountMethod
			case 12 : return 3;
			// InternalLoad::PowerMethod
			case 13 : return 2;
			// KeywordList::MyParameters
			case 14 : return 2;
			// Material::para_t
			case 15 : return 6;
			// Material::Category
			case 16 : return 13;
			// Network::PipeModel
			case 17 : return 2;
			// Network::ModelType
			case 18 : return 2;
			// Network::NetworkType
			case 19 : return 2;
			// Network::para_t
			case 20 : return 7;
			// NetworkBuriedPipeProperties::SoilType
			case 21 : return 3;
			// NetworkBuriedPipeProperties::para_t
			case 22 : return 2;
			// NetworkComponent::ModelType
			case 23 : return 16;
			// NetworkComponent::para_t
			case 24 : return 25;
			// NetworkComponent::intPara_t
			case 25 : return 2;
			// NetworkController::ModelType
			case 26 : return 2;
			// NetworkController::ControlledProperty
			case 27 : return 6;
			// NetworkController::ControllerType
			case 28 : return 4;
			// NetworkController::para_t
			case 29 : return 8;
			// NetworkController::References
			case 30 : return 2;
			// NetworkFluid::para_t
			case 31 : return 3;
			// NetworkNode::NodeType
			case 32 : return 3;
			// NetworkPipe::para_t
			case 33 : return 8;
			// OutputDefinition::timeType_t
			case 34 : return 3;
			// Outputs::flag_t
			case 35 : return 4;
			// Room::para_t
			case 36 : return 2;
			// SubSurfaceComponent::SubSurfaceComponentType
			case 37 : return 3;
			// SupplySystem::supplyType_t
			case 38 : return 4;
			// SupplySystem::para_t
			case 39 : return 4;
			// SurfaceHeating::para_t
			case 40 : return 5;
			// SurfaceHeating::Type
			case 41 : return 2;
			// VentilationNatural::para_t
			case 42 : return 1;
			// Window::Method
			case 43 : return 3;
			// Window::para_t
			case 44 : return 4;
			// WindowDivider::para_t
			case 45 : return 1;
			// WindowFrame::para_t
			case 46 : return 1;
			// WindowGlazingSystem::modelType_t
			case 47 : return 1;
			// WindowGlazingSystem::para_t
			case 48 : return 1;
			// WindowGlazingSystem::splinePara_t
			case 49 : return 1;
			// ZoneControlNaturalVentilation::para_t
			case 50 : return 4;
			// ZoneControlShading::para_t
			case 51 : return 6;
			// ZoneControlShading::Category
			case 52 : return 2;
			// ZoneControlThermostat::para_t
			case 53 : return 2;
			// ZoneControlThermostat::ControlValue
			case 54 : return 2;
			// ZoneControlThermostat::ControllerType
			case 55 : return 2;
			// ZoneIdealHeatingCooling::para_t
			case 56 : return 2;
			// ZoneTemplate::SubTemplateType
			case 57 : return 9;
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
			// Infiltration::para_t
			case 5 : return 1;
			// Infiltration::AirChangeType
			case 6 : return 1;
			// InterfaceHeatConduction::modelType_t
			case 7 : return 1;
			// InterfaceHeatConduction::OtherZoneType
			case 8 : return 2;
			// InterfaceHeatConduction::para_t
			case 9 : return 1;
			// InternalLoad::para_t
			case 10 : return 7;
			// InternalLoad::Category
			case 11 : return 3;
			// InternalLoad::PersonCountMethod
			case 12 : return 2;
			// InternalLoad::PowerMethod
			case 13 : return 1;
			// KeywordList::MyParameters
			case 14 : return 1;
			// Material::para_t
			case 15 : return 5;
			// Material::Category
			case 16 : return 12;
			// Network::PipeModel
			case 17 : return 1;
			// Network::ModelType
			case 18 : return 1;
			// Network::NetworkType
			case 19 : return 1;
			// Network::para_t
			case 20 : return 6;
			// NetworkBuriedPipeProperties::SoilType
			case 21 : return 2;
			// NetworkBuriedPipeProperties::para_t
			case 22 : return 1;
			// NetworkComponent::ModelType
			case 23 : return 15;
			// NetworkComponent::para_t
			case 24 : return 24;
			// NetworkComponent::intPara_t
			case 25 : return 1;
			// NetworkController::ModelType
			case 26 : return 1;
			// NetworkController::ControlledProperty
			case 27 : return 5;
			// NetworkController::ControllerType
			case 28 : return 3;
			// NetworkController::para_t
			case 29 : return 7;
			// NetworkController::References
			case 30 : return 1;
			// NetworkFluid::para_t
			case 31 : return 2;
			// NetworkNode::NodeType
			case 32 : return 2;
			// NetworkPipe::para_t
			case 33 : return 7;
			// OutputDefinition::timeType_t
			case 34 : return 2;
			// Outputs::flag_t
			case 35 : return 3;
			// Room::para_t
			case 36 : return 1;
			// SubSurfaceComponent::SubSurfaceComponentType
			case 37 : return 2;
			// SupplySystem::supplyType_t
			case 38 : return 3;
			// SupplySystem::para_t
			case 39 : return 3;
			// SurfaceHeating::para_t
			case 40 : return 4;
			// SurfaceHeating::Type
			case 41 : return 1;
			// VentilationNatural::para_t
			case 42 : return 0;
			// Window::Method
			case 43 : return 2;
			// Window::para_t
			case 44 : return 3;
			// WindowDivider::para_t
			case 45 : return 0;
			// WindowFrame::para_t
			case 46 : return 0;
			// WindowGlazingSystem::modelType_t
			case 47 : return 0;
			// WindowGlazingSystem::para_t
			case 48 : return 0;
			// WindowGlazingSystem::splinePara_t
			case 49 : return 0;
			// ZoneControlNaturalVentilation::para_t
			case 50 : return 3;
			// ZoneControlShading::para_t
			case 51 : return 5;
			// ZoneControlShading::Category
			case 52 : return 1;
			// ZoneControlThermostat::para_t
			case 53 : return 1;
			// ZoneControlThermostat::ControlValue
			case 54 : return 1;
			// ZoneControlThermostat::ControllerType
			case 55 : return 1;
			// ZoneIdealHeatingCooling::para_t
			case 56 : return 1;
			// ZoneTemplate::SubTemplateType
			case 57 : return 9;
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
