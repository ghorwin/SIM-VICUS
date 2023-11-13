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

#include "NM_KeywordList.h"

#include <map>
#include <limits>
#include <iostream>

#include <IBK_FormatString.h>
#include <IBK_Exception.h>


namespace NANDRAD_MODEL {
	/*! Holds the string to print as error when an invalid keyword is encountered. */
	const char * const INVALID_KEYWORD_INDEX_STRING = "KEYWORD_ERROR_STRING: Invalid type index";

	/*! Holds a list of all enum types/categories. */
	const char * const ENUM_TYPES[23] = {
		"ConstructionBalanceModel::Results",
		"ConstructionBalanceModel::VectorValuedResults",
		"ConstructionStatesModel::VectorValuedResults",
		"ConstructionStatesModel::Results",
		"HeatLoadSummationModel::Results",
		"IdealHeatingCoolingModel::VectorValuedResults",
		"IdealPipeRegisterModel::VectorValuedResults",
		"IdealSurfaceHeatingCoolingModel::VectorValuedResults",
		"InternalLoadsModel::VectorValuedResults",
		"InternalMoistureLoadsModel::VectorValuedResults",
		"KeywordList::MyParameters",
		"Loads::Results",
		"Loads::VectorValuedResults",
		"NaturalVentilationModel::VectorValuedResults",
		"NetworkInterfaceAdapterModel::Results",
		"OutputHandler::OutputFileNames",
		"RoomBalanceModel::Results",
		"RoomRadiationLoadsModel::Results",
		"RoomStatesModel::Results",
		"Schedules::KnownQuantities",
		"ThermalComfortModel::Results",
		"ThermostatModel::VectorValuedResults",
		"WindowModel::Results"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<23; ++i) {
			if (enumtype == ENUM_TYPES[i]) return i;
		}
		//std::cerr << "Unknown enumeration type '" << enumtype<< "'." << std::endl;
		return -1;
	}
	

	/*! Returns a keyword string for a given category (typenum) and type number t. */
	const char * theKeyword(int typenum, int t) {
		switch (typenum) {
			// ConstructionBalanceModel::Results
			case 0 :
			switch (t) {
				case 0 : return "FluxHeatConductionA";
				case 1 : return "FluxHeatConductionB";
				case 2 : return "FluxHeatConductionAreaSpecificA";
				case 3 : return "FluxHeatConductionAreaSpecificB";
				case 4 : return "FluxShortWaveRadiationA";
				case 5 : return "FluxShortWaveRadiationB";
				case 6 : return "FluxLongWaveRadiationA";
				case 7 : return "FluxLongWaveRadiationB";
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return "ThermalLoad";
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return "ElementTemperature";
				case 1 : return "EmittedLongWaveRadiationA";
				case 2 : return "EmittedLongWaveRadiationB";
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "SurfaceTemperatureA";
				case 1 : return "SurfaceTemperatureB";
				case 2 : return "SolarRadiationFluxA";
				case 3 : return "SolarRadiationFluxB";
				case 4 : return "LongWaveRadiationFluxA";
				case 5 : return "LongWaveRadiationFluxB";
				case 6 : return "EmittedLongWaveRadiationFluxA";
				case 7 : return "EmittedLongWaveRadiationFluxB";
			} break;
			// HeatLoadSummationModel::Results
			case 4 :
			switch (t) {
				case 0 : return "TotalHeatLoad";
			} break;
			// IdealHeatingCoolingModel::VectorValuedResults
			case 5 :
			switch (t) {
				case 0 : return "IdealHeatingLoad";
				case 1 : return "IdealCoolingLoad";
			} break;
			// IdealPipeRegisterModel::VectorValuedResults
			case 6 :
			switch (t) {
				case 0 : return "MassFlux";
				case 1 : return "ActiveLayerThermalLoad";
				case 2 : return "ReturnTemperature";
			} break;
			// IdealSurfaceHeatingCoolingModel::VectorValuedResults
			case 7 :
			switch (t) {
				case 0 : return "ActiveLayerThermalLoad";
			} break;
			// InternalLoadsModel::VectorValuedResults
			case 8 :
			switch (t) {
				case 0 : return "TotalElectricalPower";
				case 1 : return "EquipmentElectricalPower";
				case 2 : return "LightingElectricalPower";
				case 3 : return "ConvectiveEquipmentHeatLoad";
				case 4 : return "ConvectivePersonHeatLoad";
				case 5 : return "ConvectiveLightingHeatLoad";
				case 6 : return "RadiantEquipmentHeatLoad";
				case 7 : return "RadiantPersonHeatLoad";
				case 8 : return "RadiantLightingHeatLoad";
			} break;
			// InternalMoistureLoadsModel::VectorValuedResults
			case 9 :
			switch (t) {
				case 0 : return "MoistureLoad";
				case 1 : return "MoistureEnthalpyFlux";
			} break;
			// KeywordList::MyParameters
			case 10 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "Mass";
			} break;
			// Loads::Results
			case 11 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "SWRadDirectNormal";
				case 3 : return "SWRadDiffuseHorizontal";
				case 4 : return "LWSkyRadiation";
				case 5 : return "WindDirection";
				case 6 : return "WindVelocity";
				case 7 : return "AirPressure";
				case 8 : return "VaporPressure";
				case 9 : return "AbsoluteHumidity";
				case 10 : return "CO2Concentration";
				case 11 : return "CO2Density";
				case 12 : return "DeclinationAngle";
				case 13 : return "ElevationAngle";
				case 14 : return "AzimuthAngle";
				case 15 : return "Albedo";
				case 16 : return "Latitude";
				case 17 : return "Longitude";
			} break;
			// Loads::VectorValuedResults
			case 12 :
			switch (t) {
				case 0 : return "DirectSWRadOnPlane";
				case 1 : return "DiffuseSWRadOnPlane";
				case 2 : return "GlobalSWRadOnPlane";
				case 3 : return "IncidenceAngleOnPlane";
			} break;
			// NaturalVentilationModel::VectorValuedResults
			case 13 :
			switch (t) {
				case 0 : return "VentilationRate";
				case 1 : return "VentilationHeatFlux";
				case 2 : return "VentilationMoistureMassFlux";
			} break;
			// NetworkInterfaceAdapterModel::Results
			case 14 :
			switch (t) {
				case 0 : return "ReturnTemperature";
			} break;
			// OutputHandler::OutputFileNames
			case 15 :
			switch (t) {
				case 0 : return "states";
				case 1 : return "loads";
				case 2 : return "load_integrals";
				case 3 : return "fluxes";
				case 4 : return "flux_integrals";
				case 5 : return "network";
				case 6 : return "network_elements";
				case 7 : return "misc";
			} break;
			// RoomBalanceModel::Results
			case 16 :
			switch (t) {
				case 0 : return "CompleteThermalLoad";
				case 1 : return "VentilationHeatLoad";
				case 2 : return "ConvectiveEquipmentHeatLoad";
				case 3 : return "ConvectivePersonHeatLoad";
				case 4 : return "ConvectiveLightingHeatLoad";
				case 5 : return "IdealHeatingLoad";
				case 6 : return "IdealCoolingLoad";
				case 7 : return "ConstructionHeatConductionLoad";
				case 8 : return "WindowHeatConductionLoad";
				case 9 : return "WindowSolarRadiationLoad";
				case 10 : return "NetworkHeatLoad";
				case 11 : return "EquipmentElectricalPower";
				case 12 : return "LightingElectricalPower";
				case 13 : return "TotalElectricalPower";
				case 14 : return "CompleteMoistureLoad";
			} break;
			// RoomRadiationLoadsModel::Results
			case 17 :
			switch (t) {
				case 0 : return "WindowSolarRadiationFluxSum";
			} break;
			// RoomStatesModel::Results
			case 18 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "VaporPressure";
				case 3 : return "AbsoluteHumidity";
				case 4 : return "SpecificHumidity";
			} break;
			// Schedules::KnownQuantities
			case 19 :
			switch (t) {
				case 0 : return "VentilationRateSchedule";
				case 1 : return "VentilationRateIncreaseSchedule";
				case 2 : return "VentilationMaxAirTemperatureSchedule";
				case 3 : return "VentilationMinAirTemperatureSchedule";
				case 4 : return "EquipmentHeatLoadPerAreaSchedule";
				case 5 : return "PersonHeatLoadPerAreaSchedule";
				case 6 : return "MoistureLoadPerAreaSchedule";
				case 7 : return "LightingHeatLoadPerAreaSchedule";
				case 8 : return "HeatingSetpointSchedule";
				case 9 : return "CoolingSetpointSchedule";
				case 10 : return "CondenserMeanTemperatureSchedule";
				case 11 : return "CondenserOutletSetpointSchedule";
				case 12 : return "EvaporatorMeanTemperatureSchedule";
				case 13 : return "MaxMassFluxSchedule";
				case 14 : return "MassFluxSchedule";
				case 15 : return "MassFluxSetpointSchedule";
				case 16 : return "TemperatureDifferenceSetpointSchedule";
				case 17 : return "HeatPumpOnOffSignalSchedule";
				case 18 : return "DomesticHotWaterDemandSchedule";
				case 19 : return "SupplyTemperatureSchedule";
				case 20 : return "PressureHeadSchedule";
				case 21 : return "PressureLossSchedule";
				case 22 : return "TemperatureSchedule";
			} break;
			// ThermalComfortModel::Results
			case 20 :
			switch (t) {
				case 0 : return "OperativeTemperature";
			} break;
			// ThermostatModel::VectorValuedResults
			case 21 :
			switch (t) {
				case 0 : return "HeatingControlValue";
				case 1 : return "CoolingControlValue";
				case 2 : return "ThermostatHeatingSetpoint";
				case 3 : return "ThermostatCoolingSetpoint";
			} break;
			// WindowModel::Results
			case 22 :
			switch (t) {
				case 0 : return "FluxHeatConductionA";
				case 1 : return "FluxHeatConductionB";
				case 2 : return "FluxShortWaveRadiationA";
				case 3 : return "FluxShortWaveRadiationB";
				case 4 : return "SurfaceTemperatureA";
				case 5 : return "SurfaceTemperatureB";
				case 6 : return "ShadingFactor";
			} break;
		} // switch
		return INVALID_KEYWORD_INDEX_STRING;
	}

	/*! Returns all keywords including deprecated for a given category (typenum) and type number (t). */
	const char * allKeywords(int typenum, int t) {
		switch (typenum) {
			// ConstructionBalanceModel::Results
			case 0 :
			switch (t) {
				case 0 : return "FluxHeatConductionA";
				case 1 : return "FluxHeatConductionB";
				case 2 : return "FluxHeatConductionAreaSpecificA";
				case 3 : return "FluxHeatConductionAreaSpecificB";
				case 4 : return "FluxShortWaveRadiationA";
				case 5 : return "FluxShortWaveRadiationB";
				case 6 : return "FluxLongWaveRadiationA";
				case 7 : return "FluxLongWaveRadiationB";
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return "ThermalLoad";
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return "ElementTemperature";
				case 1 : return "EmittedLongWaveRadiationA";
				case 2 : return "EmittedLongWaveRadiationB";
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "SurfaceTemperatureA";
				case 1 : return "SurfaceTemperatureB";
				case 2 : return "SolarRadiationFluxA";
				case 3 : return "SolarRadiationFluxB";
				case 4 : return "LongWaveRadiationFluxA";
				case 5 : return "LongWaveRadiationFluxB";
				case 6 : return "EmittedLongWaveRadiationFluxA";
				case 7 : return "EmittedLongWaveRadiationFluxB";
			} break;
			// HeatLoadSummationModel::Results
			case 4 :
			switch (t) {
				case 0 : return "TotalHeatLoad";
			} break;
			// IdealHeatingCoolingModel::VectorValuedResults
			case 5 :
			switch (t) {
				case 0 : return "IdealHeatingLoad";
				case 1 : return "IdealCoolingLoad";
			} break;
			// IdealPipeRegisterModel::VectorValuedResults
			case 6 :
			switch (t) {
				case 0 : return "MassFlux";
				case 1 : return "ActiveLayerThermalLoad";
				case 2 : return "ReturnTemperature";
			} break;
			// IdealSurfaceHeatingCoolingModel::VectorValuedResults
			case 7 :
			switch (t) {
				case 0 : return "ActiveLayerThermalLoad";
			} break;
			// InternalLoadsModel::VectorValuedResults
			case 8 :
			switch (t) {
				case 0 : return "TotalElectricalPower";
				case 1 : return "EquipmentElectricalPower";
				case 2 : return "LightingElectricalPower";
				case 3 : return "ConvectiveEquipmentHeatLoad";
				case 4 : return "ConvectivePersonHeatLoad";
				case 5 : return "ConvectiveLightingHeatLoad";
				case 6 : return "RadiantEquipmentHeatLoad";
				case 7 : return "RadiantPersonHeatLoad";
				case 8 : return "RadiantLightingHeatLoad";
			} break;
			// InternalMoistureLoadsModel::VectorValuedResults
			case 9 :
			switch (t) {
				case 0 : return "MoistureLoad";
				case 1 : return "MoistureEnthalpyFlux";
			} break;
			// KeywordList::MyParameters
			case 10 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "Mass";
			} break;
			// Loads::Results
			case 11 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "SWRadDirectNormal";
				case 3 : return "SWRadDiffuseHorizontal";
				case 4 : return "LWSkyRadiation";
				case 5 : return "WindDirection";
				case 6 : return "WindVelocity";
				case 7 : return "AirPressure";
				case 8 : return "VaporPressure";
				case 9 : return "AbsoluteHumidity";
				case 10 : return "CO2Concentration";
				case 11 : return "CO2Density";
				case 12 : return "DeclinationAngle";
				case 13 : return "ElevationAngle";
				case 14 : return "AzimuthAngle";
				case 15 : return "Albedo";
				case 16 : return "Latitude";
				case 17 : return "Longitude";
			} break;
			// Loads::VectorValuedResults
			case 12 :
			switch (t) {
				case 0 : return "DirectSWRadOnPlane";
				case 1 : return "DiffuseSWRadOnPlane";
				case 2 : return "GlobalSWRadOnPlane";
				case 3 : return "IncidenceAngleOnPlane";
			} break;
			// NaturalVentilationModel::VectorValuedResults
			case 13 :
			switch (t) {
				case 0 : return "VentilationRate";
				case 1 : return "VentilationHeatFlux";
				case 2 : return "VentilationMoistureMassFlux";
			} break;
			// NetworkInterfaceAdapterModel::Results
			case 14 :
			switch (t) {
				case 0 : return "ReturnTemperature";
			} break;
			// OutputHandler::OutputFileNames
			case 15 :
			switch (t) {
				case 0 : return "states";
				case 1 : return "loads";
				case 2 : return "load_integrals";
				case 3 : return "fluxes";
				case 4 : return "flux_integrals";
				case 5 : return "network";
				case 6 : return "network_elements";
				case 7 : return "misc";
			} break;
			// RoomBalanceModel::Results
			case 16 :
			switch (t) {
				case 0 : return "CompleteThermalLoad";
				case 1 : return "VentilationHeatLoad";
				case 2 : return "ConvectiveEquipmentHeatLoad";
				case 3 : return "ConvectivePersonHeatLoad";
				case 4 : return "ConvectiveLightingHeatLoad";
				case 5 : return "IdealHeatingLoad";
				case 6 : return "IdealCoolingLoad";
				case 7 : return "ConstructionHeatConductionLoad";
				case 8 : return "WindowHeatConductionLoad";
				case 9 : return "WindowSolarRadiationLoad";
				case 10 : return "NetworkHeatLoad";
				case 11 : return "EquipmentElectricalPower";
				case 12 : return "LightingElectricalPower";
				case 13 : return "TotalElectricalPower";
				case 14 : return "CompleteMoistureLoad";
			} break;
			// RoomRadiationLoadsModel::Results
			case 17 :
			switch (t) {
				case 0 : return "WindowSolarRadiationFluxSum";
			} break;
			// RoomStatesModel::Results
			case 18 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "VaporPressure";
				case 3 : return "AbsoluteHumidity";
				case 4 : return "SpecificHumidity";
			} break;
			// Schedules::KnownQuantities
			case 19 :
			switch (t) {
				case 0 : return "VentilationRateSchedule";
				case 1 : return "VentilationRateIncreaseSchedule";
				case 2 : return "VentilationMaxAirTemperatureSchedule";
				case 3 : return "VentilationMinAirTemperatureSchedule";
				case 4 : return "EquipmentHeatLoadPerAreaSchedule";
				case 5 : return "PersonHeatLoadPerAreaSchedule";
				case 6 : return "MoistureLoadPerAreaSchedule";
				case 7 : return "LightingHeatLoadPerAreaSchedule";
				case 8 : return "HeatingSetpointSchedule";
				case 9 : return "CoolingSetpointSchedule";
				case 10 : return "CondenserMeanTemperatureSchedule";
				case 11 : return "CondenserOutletSetpointSchedule";
				case 12 : return "EvaporatorMeanTemperatureSchedule";
				case 13 : return "MaxMassFluxSchedule";
				case 14 : return "MassFluxSchedule";
				case 15 : return "MassFluxSetpointSchedule";
				case 16 : return "TemperatureDifferenceSetpointSchedule";
				case 17 : return "HeatPumpOnOffSignalSchedule";
				case 18 : return "DomesticHotWaterDemandSchedule";
				case 19 : return "SupplyTemperatureSchedule";
				case 20 : return "PressureHeadSchedule";
				case 21 : return "PressureLossSchedule";
				case 22 : return "TemperatureSchedule";
			} break;
			// ThermalComfortModel::Results
			case 20 :
			switch (t) {
				case 0 : return "OperativeTemperature";
			} break;
			// ThermostatModel::VectorValuedResults
			case 21 :
			switch (t) {
				case 0 : return "HeatingControlValue";
				case 1 : return "CoolingControlValue";
				case 2 : return "ThermostatHeatingSetpoint";
				case 3 : return "ThermostatCoolingSetpoint";
			} break;
			// WindowModel::Results
			case 22 :
			switch (t) {
				case 0 : return "FluxHeatConductionA";
				case 1 : return "FluxHeatConductionB";
				case 2 : return "FluxShortWaveRadiationA";
				case 3 : return "FluxShortWaveRadiationB";
				case 4 : return "SurfaceTemperatureA";
				case 5 : return "SurfaceTemperatureB";
				case 6 : return "ShadingFactor";
			} break;
		} // switch
		return INVALID_KEYWORD_INDEX_STRING;
	}

	const char * KeywordList::Description(const char * const enumtype, int t, bool * no_description) {
		if (no_description != nullptr)
			*no_description = false; // we are optimistic
		switch (enum2index(enumtype)) {
			// ConstructionBalanceModel::Results
			case 0 :
			switch (t) {
				case 0 : return "Heat conduction flux across interface A (into construction)";
				case 1 : return "Heat conduction flux across interface B (into construction)";
				case 2 : return "Heat conduction flux density across interface A (into construction)";
				case 3 : return "Heat conduction flux density across interface B (into construction)";
				case 4 : return "Short wave radiation flux across interface A (into construction)";
				case 5 : return "Short wave radiation flux across interface B (into construction)";
				case 6 : return "Long wave radiation flux across interface A (into construction)";
				case 7 : return "Long wave radiation flux across interface B (into construction)";
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return "Optional field fluxes for all material layers with given layer index";
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return "Finite-volume mean element temperature";
				case 1 : return "Emitted internal long-wave radiation from side A.";
				case 2 : return "Emitted internal long-wave radiation from side B.";
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "Surface temperature at interface A";
				case 1 : return "Surface temperature at interface B";
				case 2 : return "Solar radiation flux density into surface A";
				case 3 : return "Solar radiation flux density into surface B";
				case 4 : return "Absorbed minus emitted ambient long wave radiation flux density for surface A";
				case 5 : return "Absorbed minus emitted ambient long wave radiation flux density for surface B";
				case 6 : return "Emitted long wave radiation flux density for surface A (sum of all emissions to all other inside surface)";
				case 7 : return "Emitted long wave radiation flux density for surface B (sum of all emissions to all other inside surface)";
			} break;
			// HeatLoadSummationModel::Results
			case 4 :
			switch (t) {
				case 0 : return "Sum of heat load";
			} break;
			// IdealHeatingCoolingModel::VectorValuedResults
			case 5 :
			switch (t) {
				case 0 : return "Ideal convective heat load";
				case 1 : return "Ideal convective cooling load";
			} break;
			// IdealPipeRegisterModel::VectorValuedResults
			case 6 :
			switch (t) {
				case 0 : return "Controlled mass flow";
				case 1 : return "Active layer thermal load";
				case 2 : return "Return temperature from pipe register";
			} break;
			// IdealSurfaceHeatingCoolingModel::VectorValuedResults
			case 7 :
			switch (t) {
				case 0 : return "Active layer thermal load";
			} break;
			// InternalLoadsModel::VectorValuedResults
			case 8 :
			switch (t) {
				case 0 : return "Total electrical power from equipment and lighting per zone";
				case 1 : return "Electrical power due to electric equipment usage per zone";
				case 2 : return "Electrical power due to lighting per zone";
				case 3 : return "Convective heat load due to electric equipment usage per zone";
				case 4 : return "Convective heat load due to person occupance per zone";
				case 5 : return "Convective lighting heat load per zone";
				case 6 : return "Radiant heat load due to electric equipment usage per zone";
				case 7 : return "Radiant heat load due to person occupance per zone";
				case 8 : return "Radiant lighting heat load per zone";
			} break;
			// InternalMoistureLoadsModel::VectorValuedResults
			case 9 :
			switch (t) {
				case 0 : return "Total moisture load per zone";
				case 1 : return "Moisture enthalpy flux per zone (sensible and latent heat)";
			} break;
			// KeywordList::MyParameters
			case 10 :
			switch (t) {
				case 0 : return "Some temperatures";
				case 1 : return "Some mass";
			} break;
			// Loads::Results
			case 11 :
			switch (t) {
				case 0 : return "Outside temperature.";
				case 1 : return "Relative humidity.";
				case 2 : return "Direct short-wave radiation flux density in normal direction.";
				case 3 : return "Diffuse short-wave radiation flux density on horizontal surface.";
				case 4 : return "Long wave sky radiation.";
				case 5 : return "Wind direction (0 - north).";
				case 6 : return "Wind velocity.";
				case 7 : return "Air pressure.";
				case 8 : return "Ambient vapor pressure.";
				case 9 : return "Absolute air humidity.";
				case 10 : return "Ambient CO2 concentration.";
				case 11 : return "Ambient CO2 density.";
				case 12 : return "Solar declination (0 - north).";
				case 13 : return "Solar elevation (0 - at horizont, 90 - zenith).";
				case 14 : return "Solar azimuth (0 - north).";
				case 15 : return "Albedo value of the surrounding [0..1].";
				case 16 : return "Latitude.";
				case 17 : return "Longitude.";
			} break;
			// Loads::VectorValuedResults
			case 12 :
			switch (t) {
				case 0 : return "Direct short wave radiation on a given plane.";
				case 1 : return "Diffuse short wave radiation on a given plane.";
				case 2 : return "Global short wave radiation on a given plane.";
				case 3 : return "The incidence angle of the suns ray onto the surface (0 deg = directly perpendicular).";
			} break;
			// NaturalVentilationModel::VectorValuedResults
			case 13 :
			switch (t) {
				case 0 : return "Natural ventilation/infiltration air change rate";
				case 1 : return "Natural ventilation/infiltration heat flux";
				case 2 : return "Natural ventilation/infiltration moisture mass flux";
			} break;
			// NetworkInterfaceAdapterModel::Results
			case 14 :
			switch (t) {
				case 0 : return "Return temperature.";
			} break;
			// OutputHandler::OutputFileNames
			case 15 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "states";
				case 1 : if (no_description != nullptr) *no_description = true; return "loads";
				case 2 : if (no_description != nullptr) *no_description = true; return "load_integrals";
				case 3 : if (no_description != nullptr) *no_description = true; return "fluxes";
				case 4 : if (no_description != nullptr) *no_description = true; return "flux_integrals";
				case 5 : if (no_description != nullptr) *no_description = true; return "network";
				case 6 : if (no_description != nullptr) *no_description = true; return "network_elements";
				case 7 : if (no_description != nullptr) *no_description = true; return "misc";
			} break;
			// RoomBalanceModel::Results
			case 16 :
			switch (t) {
				case 0 : return "Sum of all thermal fluxes into the room and energy sources";
				case 1 : return "Natural ventilation/infiltration heat flux into the room";
				case 2 : return "Equipment heat load inside the room";
				case 3 : return "Person heat load inside the room";
				case 4 : return "Lighting heat load inside the room";
				case 5 : return "Ideal heating load";
				case 6 : return "Ideal cooling load (positive)";
				case 7 : return "Sum of heat conduction fluxes from construction surfaces into the room";
				case 8 : return "Sum of heat conduction fluxes through windows into the room";
				case 9 : return "Sum of solar radiation fluxes through windows into the room (only the fraction applied to room volume)";
				case 10 : return "Sum of heat load from components of a hydraulic network into the room (only the fraction applied to room volume)";
				case 11 : return "Equipment electrical power inside the room";
				case 12 : return "Lighting electrical power inside the room";
				case 13 : return "Total electrical power from equipment and lighting inside the room";
				case 14 : return "Sum of all moisture fluxes into the room and moisture sources";
			} break;
			// RoomRadiationLoadsModel::Results
			case 17 :
			switch (t) {
				case 0 : return "Sum of all short wave radiation fluxes across all windows of a zone (positive into zone).";
			} break;
			// RoomStatesModel::Results
			case 18 :
			switch (t) {
				case 0 : return "Room air temperature";
				case 1 : return "Room air relative humidity";
				case 2 : return "Room air vapor pressure";
				case 3 : return "Absolute air humidity per volume";
				case 4 : return "Mass specific air humidity";
			} break;
			// Schedules::KnownQuantities
			case 19 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "VentilationRateSchedule";
				case 1 : if (no_description != nullptr) *no_description = true; return "VentilationRateIncreaseSchedule";
				case 2 : if (no_description != nullptr) *no_description = true; return "VentilationMaxAirTemperatureSchedule";
				case 3 : if (no_description != nullptr) *no_description = true; return "VentilationMinAirTemperatureSchedule";
				case 4 : if (no_description != nullptr) *no_description = true; return "EquipmentHeatLoadPerAreaSchedule";
				case 5 : if (no_description != nullptr) *no_description = true; return "PersonHeatLoadPerAreaSchedule";
				case 6 : if (no_description != nullptr) *no_description = true; return "MoistureLoadPerAreaSchedule";
				case 7 : if (no_description != nullptr) *no_description = true; return "LightingHeatLoadPerAreaSchedule";
				case 8 : if (no_description != nullptr) *no_description = true; return "HeatingSetpointSchedule";
				case 9 : if (no_description != nullptr) *no_description = true; return "CoolingSetpointSchedule";
				case 10 : if (no_description != nullptr) *no_description = true; return "CondenserMeanTemperatureSchedule";
				case 11 : if (no_description != nullptr) *no_description = true; return "CondenserOutletSetpointSchedule";
				case 12 : if (no_description != nullptr) *no_description = true; return "EvaporatorMeanTemperatureSchedule";
				case 13 : if (no_description != nullptr) *no_description = true; return "MaxMassFluxSchedule";
				case 14 : if (no_description != nullptr) *no_description = true; return "MassFluxSchedule";
				case 15 : if (no_description != nullptr) *no_description = true; return "MassFluxSetpointSchedule";
				case 16 : if (no_description != nullptr) *no_description = true; return "TemperatureDifferenceSetpointSchedule";
				case 17 : if (no_description != nullptr) *no_description = true; return "HeatPumpOnOffSignalSchedule";
				case 18 : if (no_description != nullptr) *no_description = true; return "DomesticHotWaterDemandSchedule";
				case 19 : if (no_description != nullptr) *no_description = true; return "SupplyTemperatureSchedule";
				case 20 : if (no_description != nullptr) *no_description = true; return "PressureHeadSchedule";
				case 21 : if (no_description != nullptr) *no_description = true; return "PressureLossSchedule";
				case 22 : if (no_description != nullptr) *no_description = true; return "TemperatureSchedule";
			} break;
			// ThermalComfortModel::Results
			case 20 :
			switch (t) {
				case 0 : return "Operative temperature";
			} break;
			// ThermostatModel::VectorValuedResults
			case 21 :
			switch (t) {
				case 0 : return "Heating control signal";
				case 1 : return "Cooling control signal";
				case 2 : return "Heating setpoint";
				case 3 : return "Cooling setpoint";
			} break;
			// WindowModel::Results
			case 22 :
			switch (t) {
				case 0 : return "Heat conduction flux across interface A (into window)";
				case 1 : return "Heat conduction flux across interface B (into window)";
				case 2 : return "Short wave radiation flux across interface A (into window)";
				case 3 : return "Short wave radiation flux across interface B (into window)";
				case 4 : return "Surface temperature at interface A";
				case 5 : return "Surface temperature at interface B";
				case 6 : return "Computed reduction factor due to shading";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine description for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Description]");
	}

	const char * KeywordList::Unit(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// ConstructionBalanceModel::Results
			case 0 :
			switch (t) {
				case 0 : return "W";
				case 1 : return "W";
				case 2 : return "W/m2";
				case 3 : return "W/m2";
				case 4 : return "W";
				case 5 : return "W";
				case 6 : return "W";
				case 7 : return "W";
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return "W";
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "W";
				case 2 : return "W";
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "C";
				case 2 : return "W/m2";
				case 3 : return "W/m2";
				case 4 : return "W/m2";
				case 5 : return "W/m2";
				case 6 : return "W/m2";
				case 7 : return "W/m2";
			} break;
			// HeatLoadSummationModel::Results
			case 4 :
			switch (t) {
				case 0 : return "W";
			} break;
			// IdealHeatingCoolingModel::VectorValuedResults
			case 5 :
			switch (t) {
				case 0 : return "W";
				case 1 : return "W";
			} break;
			// IdealPipeRegisterModel::VectorValuedResults
			case 6 :
			switch (t) {
				case 0 : return "kg/s";
				case 1 : return "W";
				case 2 : return "C";
			} break;
			// IdealSurfaceHeatingCoolingModel::VectorValuedResults
			case 7 :
			switch (t) {
				case 0 : return "W";
			} break;
			// InternalLoadsModel::VectorValuedResults
			case 8 :
			switch (t) {
				case 0 : return "W";
				case 1 : return "W";
				case 2 : return "W";
				case 3 : return "W";
				case 4 : return "W";
				case 5 : return "W";
				case 6 : return "W";
				case 7 : return "W";
				case 8 : return "W";
			} break;
			// InternalMoistureLoadsModel::VectorValuedResults
			case 9 :
			switch (t) {
				case 0 : return "kg/s";
				case 1 : return "W";
			} break;
			// KeywordList::MyParameters
			case 10 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "kg";
			} break;
			// Loads::Results
			case 11 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "W/m2";
				case 3 : return "W/m2";
				case 4 : return "W/m2";
				case 5 : return "Deg";
				case 6 : return "m/s";
				case 7 : return "Pa";
				case 8 : return "Pa";
				case 9 : return "kg/m3";
				case 10 : return "---";
				case 11 : return "kg/m3";
				case 12 : return "Deg";
				case 13 : return "Deg";
				case 14 : return "Deg";
				case 15 : return "---";
				case 16 : return "Deg";
				case 17 : return "Deg";
			} break;
			// Loads::VectorValuedResults
			case 12 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
				case 2 : return "W/m2";
				case 3 : return "Deg";
			} break;
			// NaturalVentilationModel::VectorValuedResults
			case 13 :
			switch (t) {
				case 0 : return "1/h";
				case 1 : return "W";
				case 2 : return "kg/s";
			} break;
			// NetworkInterfaceAdapterModel::Results
			case 14 :
			switch (t) {
				case 0 : return "C";
			} break;
			// OutputHandler::OutputFileNames
			case 15 :
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
			// RoomBalanceModel::Results
			case 16 :
			switch (t) {
				case 0 : return "W";
				case 1 : return "W";
				case 2 : return "W";
				case 3 : return "W";
				case 4 : return "W";
				case 5 : return "W";
				case 6 : return "W";
				case 7 : return "W";
				case 8 : return "W";
				case 9 : return "W";
				case 10 : return "W";
				case 11 : return "W";
				case 12 : return "W";
				case 13 : return "W";
				case 14 : return "kg/s";
			} break;
			// RoomRadiationLoadsModel::Results
			case 17 :
			switch (t) {
				case 0 : return "W";
			} break;
			// RoomStatesModel::Results
			case 18 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "Pa";
				case 3 : return "kg/m3";
				case 4 : return "kg/kg";
			} break;
			// Schedules::KnownQuantities
			case 19 :
			switch (t) {
				case 0 : return "1/h";
				case 1 : return "1/h";
				case 2 : return "C";
				case 3 : return "C";
				case 4 : return "W/m2";
				case 5 : return "W/m2";
				case 6 : return "kg/m2s";
				case 7 : return "W/m2";
				case 8 : return "C";
				case 9 : return "C";
				case 10 : return "C";
				case 11 : return "C";
				case 12 : return "C";
				case 13 : return "kg/s";
				case 14 : return "kg/s";
				case 15 : return "kg/s";
				case 16 : return "K";
				case 17 : return "---";
				case 18 : return "W";
				case 19 : return "C";
				case 20 : return "Pa";
				case 21 : return "Pa";
				case 22 : return "C";
			} break;
			// ThermalComfortModel::Results
			case 20 :
			switch (t) {
				case 0 : return "C";
			} break;
			// ThermostatModel::VectorValuedResults
			case 21 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "C";
				case 3 : return "C";
			} break;
			// WindowModel::Results
			case 22 :
			switch (t) {
				case 0 : return "W";
				case 1 : return "W";
				case 2 : return "W";
				case 3 : return "W";
				case 4 : return "C";
				case 5 : return "C";
				case 6 : return "---";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default unit for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Unit]");
	}

	const char * KeywordList::Color(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// ConstructionBalanceModel::Results
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
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// ConstructionStatesModel::Results
			case 3 :
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
			// HeatLoadSummationModel::Results
			case 4 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// IdealHeatingCoolingModel::VectorValuedResults
			case 5 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// IdealPipeRegisterModel::VectorValuedResults
			case 6 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// IdealSurfaceHeatingCoolingModel::VectorValuedResults
			case 7 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InternalLoadsModel::VectorValuedResults
			case 8 :
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
			// InternalMoistureLoadsModel::VectorValuedResults
			case 9 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// KeywordList::MyParameters
			case 10 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Loads::Results
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
				case 13 : return "#FFFFFF";
				case 14 : return "#FFFFFF";
				case 15 : return "#FFFFFF";
				case 16 : return "#FFFFFF";
				case 17 : return "#FFFFFF";
			} break;
			// Loads::VectorValuedResults
			case 12 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// NaturalVentilationModel::VectorValuedResults
			case 13 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// NetworkInterfaceAdapterModel::Results
			case 14 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// OutputHandler::OutputFileNames
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
			// RoomBalanceModel::Results
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
				case 13 : return "#FFFFFF";
				case 14 : return "#FFFFFF";
			} break;
			// RoomRadiationLoadsModel::Results
			case 17 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// RoomStatesModel::Results
			case 18 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// Schedules::KnownQuantities
			case 19 :
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
			} break;
			// ThermalComfortModel::Results
			case 20 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// ThermostatModel::VectorValuedResults
			case 21 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// WindowModel::Results
			case 22 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine color for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Color]");
	}

	double KeywordList::DefaultValue(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// ConstructionBalanceModel::Results
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
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ConstructionStatesModel::Results
			case 3 :
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
			// HeatLoadSummationModel::Results
			case 4 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// IdealHeatingCoolingModel::VectorValuedResults
			case 5 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// IdealPipeRegisterModel::VectorValuedResults
			case 6 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// IdealSurfaceHeatingCoolingModel::VectorValuedResults
			case 7 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoadsModel::VectorValuedResults
			case 8 :
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
			// InternalMoistureLoadsModel::VectorValuedResults
			case 9 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// KeywordList::MyParameters
			case 10 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Loads::Results
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
				case 13 : return std::numeric_limits<double>::quiet_NaN();
				case 14 : return std::numeric_limits<double>::quiet_NaN();
				case 15 : return std::numeric_limits<double>::quiet_NaN();
				case 16 : return std::numeric_limits<double>::quiet_NaN();
				case 17 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Loads::VectorValuedResults
			case 12 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NaturalVentilationModel::VectorValuedResults
			case 13 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NetworkInterfaceAdapterModel::Results
			case 14 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// OutputHandler::OutputFileNames
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
			// RoomBalanceModel::Results
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
				case 13 : return std::numeric_limits<double>::quiet_NaN();
				case 14 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// RoomRadiationLoadsModel::Results
			case 17 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// RoomStatesModel::Results
			case 18 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Schedules::KnownQuantities
			case 19 :
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
			} break;
			// ThermalComfortModel::Results
			case 20 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ThermostatModel::VectorValuedResults
			case 21 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowModel::Results
			case 22 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default value for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::DefaultValue]");
	}

	// number of entries in a keyword list
	unsigned int KeywordList::Count(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// ConstructionBalanceModel::Results
			case 0 : return 8;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 : return 1;
			// ConstructionStatesModel::VectorValuedResults
			case 2 : return 3;
			// ConstructionStatesModel::Results
			case 3 : return 8;
			// HeatLoadSummationModel::Results
			case 4 : return 1;
			// IdealHeatingCoolingModel::VectorValuedResults
			case 5 : return 2;
			// IdealPipeRegisterModel::VectorValuedResults
			case 6 : return 3;
			// IdealSurfaceHeatingCoolingModel::VectorValuedResults
			case 7 : return 1;
			// InternalLoadsModel::VectorValuedResults
			case 8 : return 9;
			// InternalMoistureLoadsModel::VectorValuedResults
			case 9 : return 2;
			// KeywordList::MyParameters
			case 10 : return 2;
			// Loads::Results
			case 11 : return 18;
			// Loads::VectorValuedResults
			case 12 : return 4;
			// NaturalVentilationModel::VectorValuedResults
			case 13 : return 3;
			// NetworkInterfaceAdapterModel::Results
			case 14 : return 1;
			// OutputHandler::OutputFileNames
			case 15 : return 8;
			// RoomBalanceModel::Results
			case 16 : return 15;
			// RoomRadiationLoadsModel::Results
			case 17 : return 1;
			// RoomStatesModel::Results
			case 18 : return 5;
			// Schedules::KnownQuantities
			case 19 : return 23;
			// ThermalComfortModel::Results
			case 20 : return 1;
			// ThermostatModel::VectorValuedResults
			case 21 : return 4;
			// WindowModel::Results
			case 22 : return 7;
		} // switch
		throw IBK::Exception(IBK::FormatString("Invalid enumeration type '%1'.")
			.arg(enumtype), "[KeywordList::Count]");
	}

	// max index for entries sharing a category in a keyword list
	int KeywordList::MaxIndex(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// ConstructionBalanceModel::Results
			case 0 : return 7;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 : return 0;
			// ConstructionStatesModel::VectorValuedResults
			case 2 : return 2;
			// ConstructionStatesModel::Results
			case 3 : return 7;
			// HeatLoadSummationModel::Results
			case 4 : return 0;
			// IdealHeatingCoolingModel::VectorValuedResults
			case 5 : return 1;
			// IdealPipeRegisterModel::VectorValuedResults
			case 6 : return 2;
			// IdealSurfaceHeatingCoolingModel::VectorValuedResults
			case 7 : return 0;
			// InternalLoadsModel::VectorValuedResults
			case 8 : return 8;
			// InternalMoistureLoadsModel::VectorValuedResults
			case 9 : return 1;
			// KeywordList::MyParameters
			case 10 : return 1;
			// Loads::Results
			case 11 : return 17;
			// Loads::VectorValuedResults
			case 12 : return 3;
			// NaturalVentilationModel::VectorValuedResults
			case 13 : return 2;
			// NetworkInterfaceAdapterModel::Results
			case 14 : return 0;
			// OutputHandler::OutputFileNames
			case 15 : return 7;
			// RoomBalanceModel::Results
			case 16 : return 14;
			// RoomRadiationLoadsModel::Results
			case 17 : return 0;
			// RoomStatesModel::Results
			case 18 : return 4;
			// Schedules::KnownQuantities
			case 19 : return 22;
			// ThermalComfortModel::Results
			case 20 : return 0;
			// ThermostatModel::VectorValuedResults
			case 21 : return 3;
			// WindowModel::Results
			case 22 : return 7;
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

} // namespace NANDRAD_MODEL
