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
	const char * const ENUM_TYPES[13] = {
		"ConstructionBalanceModel::Results",
		"ConstructionBalanceModel::VectorValuedResults",
		"ConstructionStatesModel::VectorValuedResults",
		"ConstructionStatesModel::Results",
		"Loads::Results",
		"Loads::VectorValuedResults",
		"OutputHandler::OutputFileNames",
		"RoomBalanceModel::Results",
		"RoomBalanceModel::InputReferences",
		"RoomStatesModel::Results",
		"Schedules::Results",
		"ThermalComfortModel::Results",
		"ThermalComfortModel::InputReferences"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<13; ++i) {
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
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return "ThermalLoad";
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return "LayerTemperature";
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "SurfaceTemperatureA";
				case 1 : return "SurfaceTemperatureB";
			} break;
			// Loads::Results
			case 4 :
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
				case 9 : return "MoistureDensity";
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
			case 5 :
			switch (t) {
				case 0 : return "DirectSWRadOnPlane";
				case 1 : return "DiffuseSWRadOnPlane";
				case 2 : return "GlobalSWRadOnPlane";
				case 3 : return "IncidenceAngleOnPlane";
			} break;
			// OutputHandler::OutputFileNames
			case 6 :
			switch (t) {
				case 0 : return "states";
				case 1 : return "loads";
				case 2 : return "fluxes";
				case 3 : return "flux_integrals";
				case 4 : return "misc";
			} break;
			// RoomBalanceModel::Results
			case 7 :
			switch (t) {
				case 0 : return "CompleteThermalLoad";
				case 1 : return "ConstructionHeatConductionLoad";
				case 2 : return "CompleteMoistureLoad";
			} break;
			// RoomBalanceModel::InputReferences
			case 8 :
			switch (t) {
				case 0 : return "WallsHeatConductionLoad";
				case 1 : return "WindowsSWRadLoad";
				case 2 : return "WindowsHeatTransmissionLoad";
				case 3 : return "LWRadBalanceLoad";
				case 4 : return "SWRadBalanceLoad";
				case 5 : return "ConvectiveHeatingsLoad";
				case 6 : return "ConvectiveCoolingsLoad";
				case 7 : return "ConvectiveUsersLoad";
				case 8 : return "ConvectiveEquipmentLoad";
				case 9 : return "ConvectiveLightingLoad";
				case 10 : return "UserVentilationThermalLoad";
				case 11 : return "InfiltrationThermalLoad";
				case 12 : return "AirConditionThermalLoad";
				case 13 : return "DomesticWaterConsumptionSensitiveHeatGain";
			} break;
			// RoomStatesModel::Results
			case 9 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "VaporPressure";
			} break;
			// Schedules::Results
			case 10 :
			switch (t) {
				case 0 : return "HeatingSetPointTemperature";
				case 1 : return "CoolingSetPointTemperature";
				case 2 : return "AirConditionSetPointTemperature";
				case 3 : return "AirConditionSetPointRelativeHumidity";
				case 4 : return "AirConditionSetPointMassFlux";
				case 5 : return "HeatingLoad";
				case 6 : return "ThermalLoad";
				case 7 : return "MoistureLoad";
				case 8 : return "CoolingPower";
				case 9 : return "LightingPower";
				case 10 : return "DomesticWaterSetpointTemperature";
				case 11 : return "DomesticWaterMassFlow";
				case 12 : return "ThermalEnergyLossPerPerson";
				case 13 : return "TotalEnergyProductionPerPerson";
				case 14 : return "MoistureReleasePerPerson";
				case 15 : return "CO2EmissionPerPerson";
				case 16 : return "MassFluxRate";
				case 17 : return "PressureHead";
				case 18 : return "OccupancyRate";
				case 19 : return "EquipmentUtilizationRatio";
				case 20 : return "LightingUtilizationRatio";
				case 21 : return "MaximumSolarRadiationIntensity";
				case 22 : return "UserVentilationAirChangeRate";
				case 23 : return "UserVentilationComfortAirChangeRate";
				case 24 : return "UserVentilationMinimumRoomTemperature";
				case 25 : return "UserVentilationMaximumRoomTemperature";
				case 26 : return "InfiltrationAirChangeRate";
				case 27 : return "ShadingFactor";
			} break;
			// ThermalComfortModel::Results
			case 11 :
			switch (t) {
				case 0 : return "RadiantTemperature";
				case 1 : return "OperativeTemperature";
			} break;
			// ThermalComfortModel::InputReferences
			case 12 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "RadiantTemperature";
				case 2 : return "Area";
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
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return "ThermalLoad";
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return "LayerTemperature";
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "SurfaceTemperatureA";
				case 1 : return "SurfaceTemperatureB";
			} break;
			// Loads::Results
			case 4 :
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
				case 9 : return "MoistureDensity";
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
			case 5 :
			switch (t) {
				case 0 : return "DirectSWRadOnPlane";
				case 1 : return "DiffuseSWRadOnPlane";
				case 2 : return "GlobalSWRadOnPlane";
				case 3 : return "IncidenceAngleOnPlane";
			} break;
			// OutputHandler::OutputFileNames
			case 6 :
			switch (t) {
				case 0 : return "states";
				case 1 : return "loads";
				case 2 : return "fluxes";
				case 3 : return "flux_integrals";
				case 4 : return "misc";
			} break;
			// RoomBalanceModel::Results
			case 7 :
			switch (t) {
				case 0 : return "CompleteThermalLoad";
				case 1 : return "ConstructionHeatConductionLoad";
				case 2 : return "CompleteMoistureLoad";
			} break;
			// RoomBalanceModel::InputReferences
			case 8 :
			switch (t) {
				case 0 : return "WallsHeatConductionLoad";
				case 1 : return "WindowsSWRadLoad";
				case 2 : return "WindowsHeatTransmissionLoad";
				case 3 : return "LWRadBalanceLoad";
				case 4 : return "SWRadBalanceLoad";
				case 5 : return "ConvectiveHeatingsLoad";
				case 6 : return "ConvectiveCoolingsLoad";
				case 7 : return "ConvectiveUsersLoad";
				case 8 : return "ConvectiveEquipmentLoad";
				case 9 : return "ConvectiveLightingLoad";
				case 10 : return "UserVentilationThermalLoad";
				case 11 : return "InfiltrationThermalLoad";
				case 12 : return "AirConditionThermalLoad";
				case 13 : return "DomesticWaterConsumptionSensitiveHeatGain";
			} break;
			// RoomStatesModel::Results
			case 9 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "VaporPressure";
			} break;
			// Schedules::Results
			case 10 :
			switch (t) {
				case 0 : return "HeatingSetPointTemperature";
				case 1 : return "CoolingSetPointTemperature";
				case 2 : return "AirConditionSetPointTemperature";
				case 3 : return "AirConditionSetPointRelativeHumidity";
				case 4 : return "AirConditionSetPointMassFlux";
				case 5 : return "HeatingLoad";
				case 6 : return "ThermalLoad";
				case 7 : return "MoistureLoad";
				case 8 : return "CoolingPower";
				case 9 : return "LightingPower";
				case 10 : return "DomesticWaterSetpointTemperature";
				case 11 : return "DomesticWaterMassFlow";
				case 12 : return "ThermalEnergyLossPerPerson";
				case 13 : return "TotalEnergyProductionPerPerson";
				case 14 : return "MoistureReleasePerPerson";
				case 15 : return "CO2EmissionPerPerson";
				case 16 : return "MassFluxRate";
				case 17 : return "PressureHead";
				case 18 : return "OccupancyRate";
				case 19 : return "EquipmentUtilizationRatio";
				case 20 : return "LightingUtilizationRatio";
				case 21 : return "MaximumSolarRadiationIntensity";
				case 22 : return "UserVentilationAirChangeRate";
				case 23 : return "UserVentilationComfortAirChangeRate";
				case 24 : return "UserVentilationMinimumRoomTemperature";
				case 25 : return "UserVentilationMaximumRoomTemperature";
				case 26 : return "InfiltrationAirChangeRate";
				case 27 : return "ShadingFactor";
			} break;
			// ThermalComfortModel::Results
			case 11 :
			switch (t) {
				case 0 : return "RadiantTemperature";
				case 1 : return "OperativeTemperature";
			} break;
			// ThermalComfortModel::InputReferences
			case 12 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "RadiantTemperature";
				case 2 : return "Area";
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
				case 0 : return "Heat conduction flux across interface A (into construction).";
				case 1 : return "Heat conduction flux across interface B (into construction).";
			} break;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 :
			switch (t) {
				case 0 : return "Optional field fluxes for all material layers with given layer index.";
			} break;
			// ConstructionStatesModel::VectorValuedResults
			case 2 :
			switch (t) {
				case 0 : return "Mean layer temperature for requested quanties.";
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "Surface temperature at interface A.";
				case 1 : return "Surface temperature at interface B.";
			} break;
			// Loads::Results
			case 4 :
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
				case 9 : return "Ambient moisture density.";
				case 10 : return "Ambient CO2 concentration.";
				case 11 : return "Ambient CO2 density.";
				case 12 : return "Solar declination (0 - north).";
				case 13 : return "Solar elevation (0 - at horizont, 90 - directly above).";
				case 14 : return "Solar azimuth (0 - north).";
				case 15 : return "Albedo value of the surrounding [0..1].";
				case 16 : return "Latitude.";
				case 17 : return "Longitude.";
			} break;
			// Loads::VectorValuedResults
			case 5 :
			switch (t) {
				case 0 : return "Direct short wave radiation on a given plane.";
				case 1 : return "Diffuse short wave radiation on a given plane.";
				case 2 : return "Global short wave radiation on a given plane.";
				case 3 : return "The incidence angle of the suns ray onto the surface (0 deg = directly perpendicular).";
			} break;
			// OutputHandler::OutputFileNames
			case 6 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "states";
				case 1 : if (no_description != nullptr) *no_description = true; return "loads";
				case 2 : if (no_description != nullptr) *no_description = true; return "fluxes";
				case 3 : if (no_description != nullptr) *no_description = true; return "flux_integrals";
				case 4 : if (no_description != nullptr) *no_description = true; return "misc";
			} break;
			// RoomBalanceModel::Results
			case 7 :
			switch (t) {
				case 0 : return "Sum of all thermal fluxes into the room and energy sources.";
				case 1 : return "Sum of heat conduction fluxes from construction surfaces into the room.";
				case 2 : return "Sum of all moisture fluxes into the room and moisture sources.";
			} break;
			// RoomBalanceModel::InputReferences
			case 8 :
			switch (t) {
				case 0 : return "Heat load by heat conduction through all enclosing walls.";
				case 1 : return "Heat loads by short wave radiation through all windows of a room.";
				case 2 : return "Heat loads by heat transmission through all windows of a room.";
				case 3 : return "Balance loads by long wave radiation exchange on all window inside surfaces.";
				case 4 : return "Balance loads by short wave radiation exchange on all window inside surfaces.";
				case 5 : return "Heat loads by convective heating.";
				case 6 : return "Heat loss by convective cooling.";
				case 7 : return "Loads by occupancy.";
				case 8 : return "Electic equipment loads.";
				case 9 : return "Heat gains by lighting.";
				case 10 : return "Heat load by air ventilation.";
				case 11 : return "Heat load by infiltration.";
				case 12 : return "Heat load by air conditioning.";
				case 13 : return "Sensitive heat gain towards the room by water consumption.";
			} break;
			// RoomStatesModel::Results
			case 9 :
			switch (t) {
				case 0 : return "Room air temperature.";
				case 1 : return "Room air relative humidity.";
				case 2 : return "Room air vapor pressure.";
			} break;
			// Schedules::Results
			case 10 :
			switch (t) {
				case 0 : return "Setpoint temperature for heating.";
				case 1 : return "Setpoint temperature for cooling.";
				case 2 : return "Setpoint temperature for air conditioning.";
				case 3 : return "Setpoint relative humidity for air conditioning.";
				case 4 : return "Setpoint mass flux for air conditioning.";
				case 5 : return "Heating load.";
				case 6 : return "Thermal load (positive or negative).";
				case 7 : return "Moisture load.";
				case 8 : return "Cooling power.";
				case 9 : return "Lighting power.";
				case 10 : return "Setpoint temperature for domestic water.";
				case 11 : return "Domestic water demand mass flow for the complete zone (hot water and equipment).";
				case 12 : return "Energy of a single persons activities that is not available as thermal heat.";
				case 13 : return "Total energy production of a single persons body at a certain activity.";
				case 14 : return "Moisture release of a single persons body at a certain activity.";
				case 15 : return "CO2 emission mass flux of a single person at a certain activity.";
				case 16 : return "Fraction of real mass flux to maximum  mass flux for different day times.";
				case 17 : return "Supply pressure head of a pump.";
				case 18 : return "Fraction of real occupancy to maximum  occupancy for different day times.";
				case 19 : return "Ratio of usage for existing electric equipment.";
				case 20 : return "Ratio of usage for lighting.";
				case 21 : return "Maximum solar radiation intensity before shading is activated.";
				case 22 : return "Exchange rate for natural ventilation.";
				case 23 : return "Maximum air change rate = offset for user comfort.";
				case 24 : return "Temperature limit over which comfort ventilation is activated.";
				case 25 : return "Temperature limit below which comfort ventilation is activated.";
				case 26 : return "Exchange rate for infiltration.";
				case 27 : return "Shading factor [0...1].";
			} break;
			// ThermalComfortModel::Results
			case 11 :
			switch (t) {
				case 0 : return "Mean surface temperature of all surfaces facing the room.";
				case 1 : return "Operative temperature of the room.";
			} break;
			// ThermalComfortModel::InputReferences
			case 12 :
			switch (t) {
				case 0 : return "Air temperature of the room.";
				case 1 : return "Wall radiant temperature.";
				case 2 : return "Wall surface area";
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
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "C";
			} break;
			// Loads::Results
			case 4 :
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
			case 5 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
				case 2 : return "W/m2";
				case 3 : return "%";
			} break;
			// OutputHandler::OutputFileNames
			case 6 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
			} break;
			// RoomBalanceModel::Results
			case 7 :
			switch (t) {
				case 0 : return "W";
				case 1 : return "W";
				case 2 : return "kg/s";
			} break;
			// RoomBalanceModel::InputReferences
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
				case 9 : return "W";
				case 10 : return "W";
				case 11 : return "W";
				case 12 : return "W";
				case 13 : return "W";
			} break;
			// RoomStatesModel::Results
			case 9 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "Pa";
			} break;
			// Schedules::Results
			case 10 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "C";
				case 2 : return "C";
				case 3 : return "%";
				case 4 : return "kg/s";
				case 5 : return "W";
				case 6 : return "W";
				case 7 : return "g/h";
				case 8 : return "W";
				case 9 : return "W";
				case 10 : return "C";
				case 11 : return "kg/s";
				case 12 : return "W/Person";
				case 13 : return "W/Person";
				case 14 : return "kg/s";
				case 15 : return "kg/s";
				case 16 : return "---";
				case 17 : return "Pa";
				case 18 : return "---";
				case 19 : return "---";
				case 20 : return "---";
				case 21 : return "W/m2";
				case 22 : return "1/h";
				case 23 : return "1/h";
				case 24 : return "C";
				case 25 : return "C";
				case 26 : return "1/h";
				case 27 : return "---";
			} break;
			// ThermalComfortModel::Results
			case 11 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "C";
			} break;
			// ThermalComfortModel::InputReferences
			case 12 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "C";
				case 2 : return "m2";
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
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Loads::Results
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
			case 5 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// OutputHandler::OutputFileNames
			case 6 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// RoomBalanceModel::Results
			case 7 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// RoomBalanceModel::InputReferences
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
				case 9 : return "#FFFFFF";
				case 10 : return "#FFFFFF";
				case 11 : return "#FFFFFF";
				case 12 : return "#FFFFFF";
				case 13 : return "#FFFFFF";
			} break;
			// RoomStatesModel::Results
			case 9 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Schedules::Results
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
				case 25 : return "#FFFFFF";
				case 26 : return "#FFFFFF";
				case 27 : return "#FFFFFF";
			} break;
			// ThermalComfortModel::Results
			case 11 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ThermalComfortModel::InputReferences
			case 12 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
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
			} break;
			// ConstructionStatesModel::Results
			case 3 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Loads::Results
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
			case 5 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// OutputHandler::OutputFileNames
			case 6 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// RoomBalanceModel::Results
			case 7 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// RoomBalanceModel::InputReferences
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
				case 9 : return std::numeric_limits<double>::quiet_NaN();
				case 10 : return std::numeric_limits<double>::quiet_NaN();
				case 11 : return std::numeric_limits<double>::quiet_NaN();
				case 12 : return std::numeric_limits<double>::quiet_NaN();
				case 13 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// RoomStatesModel::Results
			case 9 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Schedules::Results
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
				case 25 : return std::numeric_limits<double>::quiet_NaN();
				case 26 : return std::numeric_limits<double>::quiet_NaN();
				case 27 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ThermalComfortModel::Results
			case 11 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ThermalComfortModel::InputReferences
			case 12 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default value for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::DefaultValue]");
	}

	// number of entries in a keyword list
	unsigned int KeywordList::Count(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// ConstructionBalanceModel::Results
			case 0 : return 2;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 : return 1;
			// ConstructionStatesModel::VectorValuedResults
			case 2 : return 1;
			// ConstructionStatesModel::Results
			case 3 : return 2;
			// Loads::Results
			case 4 : return 18;
			// Loads::VectorValuedResults
			case 5 : return 4;
			// OutputHandler::OutputFileNames
			case 6 : return 5;
			// RoomBalanceModel::Results
			case 7 : return 3;
			// RoomBalanceModel::InputReferences
			case 8 : return 14;
			// RoomStatesModel::Results
			case 9 : return 3;
			// Schedules::Results
			case 10 : return 28;
			// ThermalComfortModel::Results
			case 11 : return 2;
			// ThermalComfortModel::InputReferences
			case 12 : return 3;
		} // switch
		throw IBK::Exception(IBK::FormatString("Invalid enumeration type '%1'.")
			.arg(enumtype), "[KeywordList::Count]");
	}

	// max index for entries sharing a category in a keyword list
	int KeywordList::MaxIndex(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// ConstructionBalanceModel::Results
			case 0 : return 1;
			// ConstructionBalanceModel::VectorValuedResults
			case 1 : return 0;
			// ConstructionStatesModel::VectorValuedResults
			case 2 : return 0;
			// ConstructionStatesModel::Results
			case 3 : return 1;
			// Loads::Results
			case 4 : return 17;
			// Loads::VectorValuedResults
			case 5 : return 3;
			// OutputHandler::OutputFileNames
			case 6 : return 4;
			// RoomBalanceModel::Results
			case 7 : return 2;
			// RoomBalanceModel::InputReferences
			case 8 : return 13;
			// RoomStatesModel::Results
			case 9 : return 2;
			// Schedules::Results
			case 10 : return 27;
			// ThermalComfortModel::Results
			case 11 : return 1;
			// ThermalComfortModel::InputReferences
			case 12 : return 3;
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

} // namespace NANDRAD_MODEL
