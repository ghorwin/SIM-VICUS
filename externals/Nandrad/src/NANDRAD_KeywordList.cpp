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

#include "NANDRAD_KeywordList.h"

#include <map>
#include <limits>
#include <iostream>

#include <IBK_FormatString.h>
#include <IBK_Exception.h>


namespace NANDRAD {
	/*! Holds the string to print as error when an invalid keyword is encountered. */
	const char * const INVALID_KEYWORD_INDEX_STRING = "KEYWORD_ERROR_STRING: Invalid type index";

	/*! Holds a list of all enum types/categories. */
	const char * const ENUM_TYPES[84] = {
		"ConstructionInstance::para_t",
		"DailyCycle::interpolation_t",
		"EmbeddedObject::para_t",
		"EmbeddedObject::objectType_t",
		"HVACControlModel::modelType_t",
		"HVACControlModel::OperatingMode",
		"HydraulicFluid::para_t",
		"HydraulicNetwork::ModelType",
		"HydraulicNetwork::para_t",
		"HydraulicNetworkComponent::ModelType",
		"HydraulicNetworkComponent::para_t",
		"HydraulicNetworkControlElement::ModelType",
		"HydraulicNetworkControlElement::ControlledProperty",
		"HydraulicNetworkControlElement::ControllerType",
		"HydraulicNetworkControlElement::para_t",
		"HydraulicNetworkControlElement::References",
		"HydraulicNetworkElement::para_t",
		"HydraulicNetworkElement::intPara_t",
		"HydraulicNetworkHeatExchange::ModelType",
		"HydraulicNetworkHeatExchange::para_t",
		"HydraulicNetworkHeatExchange::splinePara_t",
		"HydraulicNetworkHeatExchange::References",
		"HydraulicNetworkPipeProperties::para_t",
		"IdealHeatingCoolingModel::para_t",
		"IdealPipeRegisterModel::modelType_t",
		"IdealPipeRegisterModel::para_t",
		"IdealPipeRegisterModel::intPara_t",
		"IdealSurfaceHeatingCoolingModel::para_t",
		"InterfaceAirFlow::splinePara_t",
		"InterfaceAirFlow::modelType_t",
		"InterfaceHeatConduction::modelType_t",
		"InterfaceHeatConduction::para_t",
		"InterfaceLongWaveEmission::modelType_t",
		"InterfaceLongWaveEmission::para_t",
		"InterfaceSolarAbsorption::modelType_t",
		"InterfaceSolarAbsorption::para_t",
		"InterfaceVaporDiffusion::para_t",
		"InterfaceVaporDiffusion::modelType_t",
		"InternalLoadsModel::modelType_t",
		"InternalLoadsModel::para_t",
		"Interval::para_t",
		"KeywordList::MyParameters",
		"LinearSplineParameter::interpolationMethod_t",
		"LinearSplineParameter::wrapMethod_t",
		"Location::para_t",
		"Location::flag_t",
		"Material::para_t",
		"ModelInputReference::referenceType_t",
		"NaturalVentilationModel::modelType_t",
		"NaturalVentilationModel::para_t",
		"OutputDefinition::timeType_t",
		"Schedule::ScheduledDayType",
		"Schedules::day_t",
		"Schedules::flag_t",
		"SerializationTest::test_t",
		"SerializationTest::intPara_t",
		"SerializationTest::splinePara_t",
		"SerializationTest::ReferencedIDTypes",
		"ShadingControlModel::para_t",
		"SimulationParameter::para_t",
		"SimulationParameter::intPara_t",
		"SimulationParameter::flag_t",
		"SolarLoadsDistributionModel::distribution_t",
		"SolarLoadsDistributionModel::para_t",
		"SolverParameter::para_t",
		"SolverParameter::intPara_t",
		"SolverParameter::flag_t",
		"SolverParameter::integrator_t",
		"SolverParameter::lesSolver_t",
		"SolverParameter::precond_t",
		"Thermostat::modelType_t",
		"Thermostat::para_t",
		"Thermostat::TemperatureType",
		"Thermostat::ControllerType",
		"WindowGlazingLayer::type_t",
		"WindowGlazingLayer::para_t",
		"WindowGlazingLayer::splinePara_t",
		"WindowGlazingSystem::modelType_t",
		"WindowGlazingSystem::para_t",
		"WindowGlazingSystem::splinePara_t",
		"WindowShading::modelType_t",
		"WindowShading::para_t",
		"Zone::type_t",
		"Zone::para_t"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<84; ++i) {
			if (enumtype == ENUM_TYPES[i]) return i;
		}
		//std::cerr << "Unknown enumeration type '" << enumtype<< "'." << std::endl;
		return -1;
	}
	

	/*! Returns a keyword string for a given category (typenum) and type number t. */
	const char * theKeyword(int typenum, int t) {
		switch (typenum) {
			// ConstructionInstance::para_t
			case 0 :
			switch (t) {
				case 0 : return "Orientation";
				case 1 : return "Inclination";
				case 2 : return "Area";
			} break;
			// DailyCycle::interpolation_t
			case 1 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Linear";
			} break;
			// EmbeddedObject::para_t
			case 2 :
			switch (t) {
				case 0 : return "Area";
			} break;
			// EmbeddedObject::objectType_t
			case 3 :
			switch (t) {
				case 0 : return "Window";
				case 1 : return "Door";
				case 2 : return "Hole";
			} break;
			// HVACControlModel::modelType_t
			case 4 :
			switch (t) {
				case 0 : return "Heating";
			} break;
			// HVACControlModel::OperatingMode
			case 5 :
			switch (t) {
				case 0 : return "Parallel";
			} break;
			// HydraulicFluid::para_t
			case 6 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// HydraulicNetwork::ModelType
			case 7 :
			switch (t) {
				case 0 : return "HydraulicNetwork";
				case 1 : return "ThermalHydraulicNetwork";
			} break;
			// HydraulicNetwork::para_t
			case 8 :
			switch (t) {
				case 0 : return "DefaultFluidTemperature";
				case 1 : return "InitialFluidTemperature";
				case 2 : return "ReferencePressure";
			} break;
			// HydraulicNetworkComponent::ModelType
			case 9 :
			switch (t) {
				case 0 : return "SimplePipe";
				case 1 : return "DynamicPipe";
				case 2 : return "ConstantPressurePump";
				case 3 : return "ConstantMassFluxPump";
				case 4 : return "ControlledPump";
				case 5 : return "VariablePressurePump";
				case 6 : return "HeatExchanger";
				case 7 : return "HeatPumpIdealCarnotSourceSide";
				case 8 : return "HeatPumpIdealCarnotSupplySide";
				case 9 : return "HeatPumpRealSourceSide";
				case 10 : return "ControlledValve";
				case 11 : return "IdealHeaterCooler";
				case 12 : return "ConstantPressureLossValve";
				case 13 : return "PressureLossElement";
			} break;
			// HydraulicNetworkComponent::para_t
			case 10 :
			switch (t) {
				case 0 : return "HydraulicDiameter";
				case 1 : return "PressureLossCoefficient";
				case 2 : return "PressureHead";
				case 3 : return "MassFlux";
				case 4 : return "PumpEfficiency";
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
			} break;
			// HydraulicNetworkControlElement::ModelType
			case 11 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// HydraulicNetworkControlElement::ControlledProperty
			case 12 :
			switch (t) {
				case 0 : return "TemperatureDifference";
				case 1 : return "TemperatureDifferenceOfFollowingElement";
				case 2 : return "ThermostatValue";
				case 3 : return "MassFlux";
				case 4 : return "PumpOperation";
			} break;
			// HydraulicNetworkControlElement::ControllerType
			case 13 :
			switch (t) {
				case 0 : return "PController";
				case 1 : return "PIController";
				case 2 : return "OnOffController";
			} break;
			// HydraulicNetworkControlElement::para_t
			case 14 :
			switch (t) {
				case 0 : return "Kp";
				case 1 : return "Ki";
				case 2 : return "Kd";
				case 3 : return "TemperatureDifferenceSetpoint";
				case 4 : return "MassFluxSetpoint";
				case 5 : return "HeatLossOfFollowingElementThreshold";
			} break;
			// HydraulicNetworkControlElement::References
			case 15 :
			switch (t) {
				case 0 : return "ThermostatZoneId";
			} break;
			// HydraulicNetworkElement::para_t
			case 16 :
			switch (t) {
				case 0 : return "Length";
			} break;
			// HydraulicNetworkElement::intPara_t
			case 17 :
			switch (t) {
				case 0 : return "NumberParallelPipes";
				case 1 : return "NumberParallelElements";
			} break;
			// HydraulicNetworkHeatExchange::ModelType
			case 18 :
			switch (t) {
				case 0 : return "TemperatureConstant";
				case 1 : return "TemperatureSpline";
				case 2 : return "TemperatureSplineEvaporator";
				case 3 : return "TemperatureZone";
				case 4 : return "TemperatureConstructionLayer";
				case 5 : return "HeatLossConstant";
				case 6 : return "HeatLossSpline";
				case 7 : return "HeatLossSplineCondenser";
			} break;
			// HydraulicNetworkHeatExchange::para_t
			case 19 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "HeatLoss";
				case 2 : return "ExternalHeatTransferCoefficient";
			} break;
			// HydraulicNetworkHeatExchange::splinePara_t
			case 20 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "HeatLoss";
			} break;
			// HydraulicNetworkHeatExchange::References
			case 21 :
			switch (t) {
				case 0 : return "ZoneId";
				case 1 : return "ConstructionInstanceId";
			} break;
			// HydraulicNetworkPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "PipeRoughness";
				case 1 : return "PipeInnerDiameter";
				case 2 : return "PipeOuterDiameter";
				case 3 : return "UValuePipeWall";
			} break;
			// IdealHeatingCoolingModel::para_t
			case 23 :
			switch (t) {
				case 0 : return "MaxHeatingPowerPerArea";
				case 1 : return "MaxCoolingPowerPerArea";
				case 2 : return "Kp";
				case 3 : return "Ki";
			} break;
			// IdealPipeRegisterModel::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// IdealPipeRegisterModel::para_t
			case 25 :
			switch (t) {
				case 0 : return "SupplyTemperature";
				case 1 : return "MaxMassFlux";
				case 2 : return "PipeLength";
				case 3 : return "PipeInnerDiameter";
				case 4 : return "UValuePipeWall";
			} break;
			// IdealPipeRegisterModel::intPara_t
			case 26 :
			switch (t) {
				case 0 : return "NumberParallelPipes";
			} break;
			// IdealSurfaceHeatingCoolingModel::para_t
			case 27 :
			switch (t) {
				case 0 : return "MaxHeatingPowerPerArea";
				case 1 : return "MaxCoolingPowerPerArea";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 28 :
			switch (t) {
				case 0 : return "PressureCoefficient";
			} break;
			// InterfaceAirFlow::modelType_t
			case 29 :
			switch (t) {
				case 0 : return "WindFlow";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 30 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "None";
			} break;
			// InterfaceHeatConduction::para_t
			case 31 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "None";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 33 :
			switch (t) {
				case 0 : return "Emissivity";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 34 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "None";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 35 :
			switch (t) {
				case 0 : return "AbsorptionCoefficient";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 36 :
			switch (t) {
				case 0 : return "VaporTransferCoefficient";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 37 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InternalLoadsModel::modelType_t
			case 38 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// InternalLoadsModel::para_t
			case 39 :
			switch (t) {
				case 0 : return "EquipmentRadiationFraction";
				case 1 : return "PersonRadiationFraction";
				case 2 : return "LightingRadiationFraction";
				case 3 : return "EquipmentHeatLoadPerArea";
				case 4 : return "PersonHeatLoadPerArea";
				case 5 : return "LightingHeatLoadPerArea";
			} break;
			// Interval::para_t
			case 40 :
			switch (t) {
				case 0 : return "Start";
				case 1 : return "End";
				case 2 : return "StepSize";
			} break;
			// KeywordList::MyParameters
			case 41 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "Mass";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 42 :
			switch (t) {
				case 0 : return "constant";
				case 1 : return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 43 :
			switch (t) {
				case 0 : return "continuous";
				case 1 : return "cyclic";
			} break;
			// Location::para_t
			case 44 :
			switch (t) {
				case 0 : return "Latitude";
				case 1 : return "Longitude";
				case 2 : return "Albedo";
				case 3 : return "Altitude";
			} break;
			// Location::flag_t
			case 45 :
			switch (t) {
				case 0 : return "PerezDiffuseRadiationModel";
				case 1 : return "ContinuousShadingFactorData";
			} break;
			// Material::para_t
			case 46 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// ModelInputReference::referenceType_t
			case 47 :
			switch (t) {
				case 0 : return "Location";
				case 1 : return "Zone";
				case 2 : return "ConstructionInstance";
				case 3 : return "EmbeddedObject";
				case 4 : return "Schedule";
				case 5 : return "Model";
				case 6 : return "Network";
				case 7 : return "NetworkElement";
			} break;
			// NaturalVentilationModel::modelType_t
			case 48 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
				case 2 : return "ScheduledWithBaseACR";
			} break;
			// NaturalVentilationModel::para_t
			case 49 :
			switch (t) {
				case 0 : return "VentilationRate";
				case 1 : return "MaxAirTemperature";
				case 2 : return "MinAirTemperature";
				case 3 : return "MaxWindSpeed";
			} break;
			// OutputDefinition::timeType_t
			case 50 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// Schedule::ScheduledDayType
			case 51 :
			switch (t) {
				case 0 : return "AllDays";
				case 1 : return "WeekDay";
				case 2 : return "WeekEnd";
				case 3 : return "Monday";
				case 4 : return "Tuesday";
				case 5 : return "Wednesday";
				case 6 : return "Thursday";
				case 7 : return "Friday";
				case 8 : return "Saturday";
				case 9 : return "Sunday";
				case 10 : return "Holiday";
			} break;
			// Schedules::day_t
			case 52 :
			switch (t) {
				case 0 : return "Mon";
				case 1 : return "Tue";
				case 2 : return "Wed";
				case 3 : return "Thu";
				case 4 : return "Fri";
				case 5 : return "Sat";
				case 6 : return "Sun";
			} break;
			// Schedules::flag_t
			case 53 :
			switch (t) {
				case 0 : return "EnableCyclicSchedules";
			} break;
			// SerializationTest::test_t
			case 54 :
			switch (t) {
				case 0 : return "X1";
				case 1 : return "X2";
			} break;
			// SerializationTest::intPara_t
			case 55 :
			switch (t) {
				case 0 : return "I1";
				case 1 : return "I2";
			} break;
			// SerializationTest::splinePara_t
			case 56 :
			switch (t) {
				case 0 : return "ParameterSet1";
				case 1 : return "ParameterSet2";
			} break;
			// SerializationTest::ReferencedIDTypes
			case 57 :
			switch (t) {
				case 0 : return "SomeStove";
				case 1 : return "SomeOven";
				case 2 : return "SomeHeater";
				case 3 : return "SomeFurnace";
			} break;
			// ShadingControlModel::para_t
			case 58 :
			switch (t) {
				case 0 : return "MaxIntensity";
				case 1 : return "MinIntensity";
			} break;
			// SimulationParameter::para_t
			case 59 :
			switch (t) {
				case 0 : return "InitialTemperature";
				case 1 : return "InitialRelativeHumidity";
				case 2 : return "DomesticWaterSensitiveHeatGainFraction";
				case 3 : return "AirExchangeRateN50";
				case 4 : return "ShieldingCoefficient";
				case 5 : return "HeatingDesignAmbientTemperature";
			} break;
			// SimulationParameter::intPara_t
			case 60 :
			switch (t) {
				case 0 : return "StartYear";
			} break;
			// SimulationParameter::flag_t
			case 61 :
			switch (t) {
				case 0 : return "EnableMoistureBalance";
				case 1 : return "EnableCO2Balance";
				case 2 : return "EnableJointVentilation";
				case 3 : return "ExportClimateDataFMU";
			} break;
			// SolarLoadsDistributionModel::distribution_t
			case 62 :
			switch (t) {
				case 0 : return "AreaWeighted";
				case 1 : return "SurfaceTypeFactor";
				case 2 : return "ViewFactor";
			} break;
			// SolarLoadsDistributionModel::para_t
			case 63 :
			switch (t) {
				case 0 : return "RadiationLoadFractionZone";
				case 1 : return "RadiationLoadFractionFloor";
				case 2 : return "RadiationLoadFractionCeiling";
				case 3 : return "RadiationLoadFractionWalls";
			} break;
			// SolverParameter::para_t
			case 64 :
			switch (t) {
				case 0 : return "RelTol";
				case 1 : return "AbsTol";
				case 2 : return "MaxTimeStep";
				case 3 : return "MinTimeStep";
				case 4 : return "InitialTimeStep";
				case 5 : return "NonlinSolverConvCoeff";
				case 6 : return "IterativeSolverConvCoeff";
				case 7 : return "DiscMinDx";
				case 8 : return "DiscStretchFactor";
				case 9 : return "ViewfactorTileWidth";
				case 10 : return "SurfaceDiscretizationDensity";
				case 11 : return "ControlTemperatureTolerance";
				case 12 : return "KinsolRelTol";
				case 13 : return "KinsolAbsTol";
			} break;
			// SolverParameter::intPara_t
			case 65 :
			switch (t) {
				case 0 : return "PreILUWidth";
				case 1 : return "MaxKrylovDim";
				case 2 : return "MaxNonlinIter";
				case 3 : return "MaxOrder";
				case 4 : return "KinsolMaxNonlinIter";
				case 5 : return "DiscMaxElementsPerLayer";
			} break;
			// SolverParameter::flag_t
			case 66 :
			switch (t) {
				case 0 : return "DetectMaxTimeStep";
				case 1 : return "KinsolDisableLineSearch";
				case 2 : return "KinsolStrictNewton";
			} break;
			// SolverParameter::integrator_t
			case 67 :
			switch (t) {
				case 0 : return "CVODE";
				case 1 : return "ExplicitEuler";
				case 2 : return "ImplicitEuler";
				case 3 : return "auto";
			} break;
			// SolverParameter::lesSolver_t
			case 68 :
			switch (t) {
				case 0 : return "Dense";
				case 1 : return "KLU";
				case 2 : return "GMRES";
				case 3 : return "BiCGStab";
				case 4 : return "auto";
			} break;
			// SolverParameter::precond_t
			case 69 :
			switch (t) {
				case 0 : return "ILU";
				case 1 : return "auto";
			} break;
			// Thermostat::modelType_t
			case 70 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// Thermostat::para_t
			case 71 :
			switch (t) {
				case 0 : return "HeatingSetpoint";
				case 1 : return "CoolingSetpoint";
				case 2 : return "TemperatureTolerance";
				case 3 : return "TemperatureBand";
			} break;
			// Thermostat::TemperatureType
			case 72 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "OperativeTemperature";
			} break;
			// Thermostat::ControllerType
			case 73 :
			switch (t) {
				case 0 : return "Analog";
				case 1 : return "Digital";
			} break;
			// WindowGlazingLayer::type_t
			case 74 :
			switch (t) {
				case 0 : return "Gas";
				case 1 : return "Glass";
			} break;
			// WindowGlazingLayer::para_t
			case 75 :
			switch (t) {
				case 0 : return "Thickness";
				case 1 : return "Conductivity";
				case 2 : return "MassDensity";
				case 3 : return "Height";
				case 4 : return "Width";
				case 5 : return "LongWaveEmissivityInside";
				case 6 : return "P_LongWaveEmissivityOutside";
			} break;
			// WindowGlazingLayer::splinePara_t
			case 76 :
			switch (t) {
				case 0 : return "ShortWaveTransmittance";
				case 1 : return "ShortWaveReflectanceOutside";
				case 2 : return "ShortWaveReflectanceInside";
				case 3 : return "Conductivity";
				case 4 : return "DynamicViscosity";
				case 5 : return "HeatCapacity";
			} break;
			// WindowGlazingSystem::modelType_t
			case 77 :
			switch (t) {
				case 0 : return "Simple";
				case 1 : return "Detailed";
			} break;
			// WindowGlazingSystem::para_t
			case 78 :
			switch (t) {
				case 0 : return "ThermalTransmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 79 :
			switch (t) {
				case 0 : return "SHGC";
			} break;
			// WindowShading::modelType_t
			case 80 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Precomputed";
				case 2 : return "Controlled";
			} break;
			// WindowShading::para_t
			case 81 :
			switch (t) {
				case 0 : return "ReductionFactor";
			} break;
			// Zone::type_t
			case 82 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
				case 2 : return "Active";
				case 3 : return "Ground";
			} break;
			// Zone::para_t
			case 83 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "CO2Concentration";
				case 3 : return "Area";
				case 4 : return "Volume";
				case 5 : return "HeatCapacity";
			} break;
		} // switch
		return INVALID_KEYWORD_INDEX_STRING;
	}

	/*! Returns all keywords including deprecated for a given category (typenum) and type number (t). */
	const char * allKeywords(int typenum, int t) {
		switch (typenum) {
			// ConstructionInstance::para_t
			case 0 :
			switch (t) {
				case 0 : return "Orientation";
				case 1 : return "Inclination";
				case 2 : return "Area";
			} break;
			// DailyCycle::interpolation_t
			case 1 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Linear";
			} break;
			// EmbeddedObject::para_t
			case 2 :
			switch (t) {
				case 0 : return "Area";
			} break;
			// EmbeddedObject::objectType_t
			case 3 :
			switch (t) {
				case 0 : return "Window";
				case 1 : return "Door";
				case 2 : return "Hole";
			} break;
			// HVACControlModel::modelType_t
			case 4 :
			switch (t) {
				case 0 : return "Heating";
			} break;
			// HVACControlModel::OperatingMode
			case 5 :
			switch (t) {
				case 0 : return "Parallel";
			} break;
			// HydraulicFluid::para_t
			case 6 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// HydraulicNetwork::ModelType
			case 7 :
			switch (t) {
				case 0 : return "HydraulicNetwork";
				case 1 : return "ThermalHydraulicNetwork";
			} break;
			// HydraulicNetwork::para_t
			case 8 :
			switch (t) {
				case 0 : return "DefaultFluidTemperature";
				case 1 : return "InitialFluidTemperature";
				case 2 : return "ReferencePressure";
			} break;
			// HydraulicNetworkComponent::ModelType
			case 9 :
			switch (t) {
				case 0 : return "SimplePipe";
				case 1 : return "DynamicPipe";
				case 2 : return "ConstantPressurePump";
				case 3 : return "ConstantMassFluxPump";
				case 4 : return "ControlledPump";
				case 5 : return "VariablePressurePump";
				case 6 : return "HeatExchanger";
				case 7 : return "HeatPumpIdealCarnotSourceSide";
				case 8 : return "HeatPumpIdealCarnotSupplySide";
				case 9 : return "HeatPumpRealSourceSide";
				case 10 : return "ControlledValve";
				case 11 : return "IdealHeaterCooler";
				case 12 : return "ConstantPressureLossValve";
				case 13 : return "PressureLossElement";
			} break;
			// HydraulicNetworkComponent::para_t
			case 10 :
			switch (t) {
				case 0 : return "HydraulicDiameter";
				case 1 : return "PressureLossCoefficient";
				case 2 : return "PressureHead";
				case 3 : return "MassFlux";
				case 4 : return "PumpEfficiency";
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
			} break;
			// HydraulicNetworkControlElement::ModelType
			case 11 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// HydraulicNetworkControlElement::ControlledProperty
			case 12 :
			switch (t) {
				case 0 : return "TemperatureDifference";
				case 1 : return "TemperatureDifferenceOfFollowingElement";
				case 2 : return "ThermostatValue";
				case 3 : return "MassFlux";
				case 4 : return "PumpOperation";
			} break;
			// HydraulicNetworkControlElement::ControllerType
			case 13 :
			switch (t) {
				case 0 : return "PController";
				case 1 : return "PIController";
				case 2 : return "OnOffController";
			} break;
			// HydraulicNetworkControlElement::para_t
			case 14 :
			switch (t) {
				case 0 : return "Kp";
				case 1 : return "Ki";
				case 2 : return "Kd";
				case 3 : return "TemperatureDifferenceSetpoint";
				case 4 : return "MassFluxSetpoint";
				case 5 : return "HeatLossOfFollowingElementThreshold";
			} break;
			// HydraulicNetworkControlElement::References
			case 15 :
			switch (t) {
				case 0 : return "ThermostatZoneId";
			} break;
			// HydraulicNetworkElement::para_t
			case 16 :
			switch (t) {
				case 0 : return "Length";
			} break;
			// HydraulicNetworkElement::intPara_t
			case 17 :
			switch (t) {
				case 0 : return "NumberParallelPipes";
				case 1 : return "NumberParallelElements";
			} break;
			// HydraulicNetworkHeatExchange::ModelType
			case 18 :
			switch (t) {
				case 0 : return "TemperatureConstant";
				case 1 : return "TemperatureSpline";
				case 2 : return "TemperatureSplineEvaporator";
				case 3 : return "TemperatureZone";
				case 4 : return "TemperatureConstructionLayer";
				case 5 : return "HeatLossConstant";
				case 6 : return "HeatLossSpline";
				case 7 : return "HeatLossSplineCondenser";
			} break;
			// HydraulicNetworkHeatExchange::para_t
			case 19 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "HeatLoss";
				case 2 : return "ExternalHeatTransferCoefficient";
			} break;
			// HydraulicNetworkHeatExchange::splinePara_t
			case 20 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "HeatLoss";
			} break;
			// HydraulicNetworkHeatExchange::References
			case 21 :
			switch (t) {
				case 0 : return "ZoneId";
				case 1 : return "ConstructionInstanceId";
			} break;
			// HydraulicNetworkPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "PipeRoughness";
				case 1 : return "PipeInnerDiameter";
				case 2 : return "PipeOuterDiameter";
				case 3 : return "UValuePipeWall";
			} break;
			// IdealHeatingCoolingModel::para_t
			case 23 :
			switch (t) {
				case 0 : return "MaxHeatingPowerPerArea";
				case 1 : return "MaxCoolingPowerPerArea";
				case 2 : return "Kp";
				case 3 : return "Ki";
			} break;
			// IdealPipeRegisterModel::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// IdealPipeRegisterModel::para_t
			case 25 :
			switch (t) {
				case 0 : return "SupplyTemperature";
				case 1 : return "MaxMassFlux";
				case 2 : return "PipeLength";
				case 3 : return "PipeInnerDiameter";
				case 4 : return "UValuePipeWall";
			} break;
			// IdealPipeRegisterModel::intPara_t
			case 26 :
			switch (t) {
				case 0 : return "NumberParallelPipes";
			} break;
			// IdealSurfaceHeatingCoolingModel::para_t
			case 27 :
			switch (t) {
				case 0 : return "MaxHeatingPowerPerArea";
				case 1 : return "MaxCoolingPowerPerArea";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 28 :
			switch (t) {
				case 0 : return "PressureCoefficient";
			} break;
			// InterfaceAirFlow::modelType_t
			case 29 :
			switch (t) {
				case 0 : return "WindFlow";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 30 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "None";
			} break;
			// InterfaceHeatConduction::para_t
			case 31 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "None";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 33 :
			switch (t) {
				case 0 : return "Emissivity";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 34 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "None";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 35 :
			switch (t) {
				case 0 : return "AbsorptionCoefficient";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 36 :
			switch (t) {
				case 0 : return "VaporTransferCoefficient";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 37 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InternalLoadsModel::modelType_t
			case 38 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// InternalLoadsModel::para_t
			case 39 :
			switch (t) {
				case 0 : return "EquipmentRadiationFraction";
				case 1 : return "PersonRadiationFraction";
				case 2 : return "LightingRadiationFraction";
				case 3 : return "EquipmentHeatLoadPerArea";
				case 4 : return "PersonHeatLoadPerArea";
				case 5 : return "LightingHeatLoadPerArea";
			} break;
			// Interval::para_t
			case 40 :
			switch (t) {
				case 0 : return "Start";
				case 1 : return "End";
				case 2 : return "StepSize";
			} break;
			// KeywordList::MyParameters
			case 41 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "Mass";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 42 :
			switch (t) {
				case 0 : return "constant";
				case 1 : return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 43 :
			switch (t) {
				case 0 : return "continuous";
				case 1 : return "cyclic";
			} break;
			// Location::para_t
			case 44 :
			switch (t) {
				case 0 : return "Latitude";
				case 1 : return "Longitude";
				case 2 : return "Albedo";
				case 3 : return "Altitude";
			} break;
			// Location::flag_t
			case 45 :
			switch (t) {
				case 0 : return "PerezDiffuseRadiationModel";
				case 1 : return "ContinuousShadingFactorData";
			} break;
			// Material::para_t
			case 46 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// ModelInputReference::referenceType_t
			case 47 :
			switch (t) {
				case 0 : return "Location";
				case 1 : return "Zone";
				case 2 : return "ConstructionInstance";
				case 3 : return "EmbeddedObject";
				case 4 : return "Schedule";
				case 5 : return "Model";
				case 6 : return "Network";
				case 7 : return "NetworkElement";
			} break;
			// NaturalVentilationModel::modelType_t
			case 48 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
				case 2 : return "ScheduledWithBaseACR";
			} break;
			// NaturalVentilationModel::para_t
			case 49 :
			switch (t) {
				case 0 : return "VentilationRate";
				case 1 : return "MaxAirTemperature";
				case 2 : return "MinAirTemperature";
				case 3 : return "MaxWindSpeed";
			} break;
			// OutputDefinition::timeType_t
			case 50 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// Schedule::ScheduledDayType
			case 51 :
			switch (t) {
				case 0 : return "AllDays";
				case 1 : return "WeekDay";
				case 2 : return "WeekEnd";
				case 3 : return "Monday";
				case 4 : return "Tuesday";
				case 5 : return "Wednesday";
				case 6 : return "Thursday";
				case 7 : return "Friday";
				case 8 : return "Saturday";
				case 9 : return "Sunday";
				case 10 : return "Holiday";
			} break;
			// Schedules::day_t
			case 52 :
			switch (t) {
				case 0 : return "Mon";
				case 1 : return "Tue";
				case 2 : return "Wed";
				case 3 : return "Thu";
				case 4 : return "Fri";
				case 5 : return "Sat";
				case 6 : return "Sun";
			} break;
			// Schedules::flag_t
			case 53 :
			switch (t) {
				case 0 : return "EnableCyclicSchedules";
			} break;
			// SerializationTest::test_t
			case 54 :
			switch (t) {
				case 0 : return "X1";
				case 1 : return "X2";
			} break;
			// SerializationTest::intPara_t
			case 55 :
			switch (t) {
				case 0 : return "I1";
				case 1 : return "I2";
			} break;
			// SerializationTest::splinePara_t
			case 56 :
			switch (t) {
				case 0 : return "ParameterSet1";
				case 1 : return "ParameterSet2";
			} break;
			// SerializationTest::ReferencedIDTypes
			case 57 :
			switch (t) {
				case 0 : return "SomeStove";
				case 1 : return "SomeOven";
				case 2 : return "SomeHeater";
				case 3 : return "SomeFurnace";
			} break;
			// ShadingControlModel::para_t
			case 58 :
			switch (t) {
				case 0 : return "MaxIntensity";
				case 1 : return "MinIntensity";
			} break;
			// SimulationParameter::para_t
			case 59 :
			switch (t) {
				case 0 : return "InitialTemperature";
				case 1 : return "InitialRelativeHumidity";
				case 2 : return "DomesticWaterSensitiveHeatGainFraction";
				case 3 : return "AirExchangeRateN50";
				case 4 : return "ShieldingCoefficient";
				case 5 : return "HeatingDesignAmbientTemperature";
			} break;
			// SimulationParameter::intPara_t
			case 60 :
			switch (t) {
				case 0 : return "StartYear";
			} break;
			// SimulationParameter::flag_t
			case 61 :
			switch (t) {
				case 0 : return "EnableMoistureBalance";
				case 1 : return "EnableCO2Balance";
				case 2 : return "EnableJointVentilation";
				case 3 : return "ExportClimateDataFMU";
			} break;
			// SolarLoadsDistributionModel::distribution_t
			case 62 :
			switch (t) {
				case 0 : return "AreaWeighted";
				case 1 : return "SurfaceTypeFactor";
				case 2 : return "ViewFactor";
			} break;
			// SolarLoadsDistributionModel::para_t
			case 63 :
			switch (t) {
				case 0 : return "RadiationLoadFractionZone";
				case 1 : return "RadiationLoadFractionFloor";
				case 2 : return "RadiationLoadFractionCeiling";
				case 3 : return "RadiationLoadFractionWalls";
			} break;
			// SolverParameter::para_t
			case 64 :
			switch (t) {
				case 0 : return "RelTol";
				case 1 : return "AbsTol";
				case 2 : return "MaxTimeStep";
				case 3 : return "MinTimeStep";
				case 4 : return "InitialTimeStep";
				case 5 : return "NonlinSolverConvCoeff";
				case 6 : return "IterativeSolverConvCoeff";
				case 7 : return "DiscMinDx";
				case 8 : return "DiscStretchFactor";
				case 9 : return "ViewfactorTileWidth";
				case 10 : return "SurfaceDiscretizationDensity";
				case 11 : return "ControlTemperatureTolerance";
				case 12 : return "KinsolRelTol";
				case 13 : return "KinsolAbsTol";
			} break;
			// SolverParameter::intPara_t
			case 65 :
			switch (t) {
				case 0 : return "PreILUWidth";
				case 1 : return "MaxKrylovDim";
				case 2 : return "MaxNonlinIter";
				case 3 : return "MaxOrder";
				case 4 : return "KinsolMaxNonlinIter";
				case 5 : return "DiscMaxElementsPerLayer";
			} break;
			// SolverParameter::flag_t
			case 66 :
			switch (t) {
				case 0 : return "DetectMaxTimeStep";
				case 1 : return "KinsolDisableLineSearch";
				case 2 : return "KinsolStrictNewton";
			} break;
			// SolverParameter::integrator_t
			case 67 :
			switch (t) {
				case 0 : return "CVODE";
				case 1 : return "ExplicitEuler";
				case 2 : return "ImplicitEuler";
				case 3 : return "auto";
			} break;
			// SolverParameter::lesSolver_t
			case 68 :
			switch (t) {
				case 0 : return "Dense";
				case 1 : return "KLU";
				case 2 : return "GMRES";
				case 3 : return "BiCGStab";
				case 4 : return "auto";
			} break;
			// SolverParameter::precond_t
			case 69 :
			switch (t) {
				case 0 : return "ILU";
				case 1 : return "auto";
			} break;
			// Thermostat::modelType_t
			case 70 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// Thermostat::para_t
			case 71 :
			switch (t) {
				case 0 : return "HeatingSetpoint";
				case 1 : return "CoolingSetpoint";
				case 2 : return "TemperatureTolerance";
				case 3 : return "TemperatureBand";
			} break;
			// Thermostat::TemperatureType
			case 72 :
			switch (t) {
				case 0 : return "AirTemperature";
				case 1 : return "OperativeTemperature";
			} break;
			// Thermostat::ControllerType
			case 73 :
			switch (t) {
				case 0 : return "Analog";
				case 1 : return "Digital";
			} break;
			// WindowGlazingLayer::type_t
			case 74 :
			switch (t) {
				case 0 : return "Gas";
				case 1 : return "Glass";
			} break;
			// WindowGlazingLayer::para_t
			case 75 :
			switch (t) {
				case 0 : return "Thickness";
				case 1 : return "Conductivity";
				case 2 : return "MassDensity";
				case 3 : return "Height";
				case 4 : return "Width";
				case 5 : return "LongWaveEmissivityInside";
				case 6 : return "P_LongWaveEmissivityOutside";
			} break;
			// WindowGlazingLayer::splinePara_t
			case 76 :
			switch (t) {
				case 0 : return "ShortWaveTransmittance";
				case 1 : return "ShortWaveReflectanceOutside";
				case 2 : return "ShortWaveReflectanceInside";
				case 3 : return "Conductivity";
				case 4 : return "DynamicViscosity";
				case 5 : return "HeatCapacity";
			} break;
			// WindowGlazingSystem::modelType_t
			case 77 :
			switch (t) {
				case 0 : return "Simple";
				case 1 : return "Detailed";
			} break;
			// WindowGlazingSystem::para_t
			case 78 :
			switch (t) {
				case 0 : return "ThermalTransmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 79 :
			switch (t) {
				case 0 : return "SHGC";
			} break;
			// WindowShading::modelType_t
			case 80 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Precomputed";
				case 2 : return "Controlled";
			} break;
			// WindowShading::para_t
			case 81 :
			switch (t) {
				case 0 : return "ReductionFactor";
			} break;
			// Zone::type_t
			case 82 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
				case 2 : return "Active";
				case 3 : return "Ground";
			} break;
			// Zone::para_t
			case 83 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "CO2Concentration";
				case 3 : return "Area";
				case 4 : return "Volume";
				case 5 : return "HeatCapacity";
			} break;
		} // switch
		return INVALID_KEYWORD_INDEX_STRING;
	}

	const char * KeywordList::Description(const char * const enumtype, int t, bool * no_description) {
		if (no_description != nullptr)
			*no_description = false; // we are optimistic
		switch (enum2index(enumtype)) {
			// ConstructionInstance::para_t
			case 0 :
			switch (t) {
				case 0 : return "Orientation of the wall [deg].";
				case 1 : return "Inclination of the wall [deg].";
				case 2 : return "Gross area of the wall [m2].";
			} break;
			// DailyCycle::interpolation_t
			case 1 :
			switch (t) {
				case 0 : return "Constant values in defined intervals.";
				case 1 : return "Linear interpolation between values.";
			} break;
			// EmbeddedObject::para_t
			case 2 :
			switch (t) {
				case 0 : return "Area of the embedded object [m2].";
			} break;
			// EmbeddedObject::objectType_t
			case 3 :
			switch (t) {
				case 0 : return "Parametrization of a window model.";
				case 1 : return "Parametrization of a door model.";
				case 2 : return "Parametrization of an opening model.";
			} break;
			// HVACControlModel::modelType_t
			case 4 :
			switch (t) {
				case 0 : return "Heating control model";
			} break;
			// HVACControlModel::OperatingMode
			case 5 :
			switch (t) {
				case 0 : return "Parallel operation";
			} break;
			// HydraulicFluid::para_t
			case 6 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
			} break;
			// HydraulicNetwork::ModelType
			case 7 :
			switch (t) {
				case 0 : return "Only Hydraulic calculation with constant temperature";
				case 1 : return "Thermo-hydraulic calculation";
			} break;
			// HydraulicNetwork::para_t
			case 8 :
			switch (t) {
				case 0 : return "Default temperature for HydraulicNetwork models";
				case 1 : return "Initial temperature of the fluid";
				case 2 : return "Reference pressure of network";
			} break;
			// HydraulicNetworkComponent::ModelType
			case 9 :
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
				case 9 : return "On-off-type heat pump based on polynoms, installed at source side";
				case 10 : return "Valve with associated control model";
				case 11 : return "Ideal heat exchange model that provides a defined supply temperature to the network and calculates the heat loss/gain";
				case 12 : return "Valve with constant pressure loss";
				case 13 : return "Adiabatic element with pressure loss defined by zeta-value";
			} break;
			// HydraulicNetworkComponent::para_t
			case 10 :
			switch (t) {
				case 0 : return "Only used for pressure loss calculation with PressureLossCoefficient (NOT for pipes)";
				case 1 : return "Pressure loss coefficient for the component (zeta-value)";
				case 2 : return "Pump predefined pressure head";
				case 3 : return "Pump predefined mass flux";
				case 4 : return "Pump efficiency";
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
			} break;
			// HydraulicNetworkControlElement::ModelType
			case 11 :
			switch (t) {
				case 0 : return "Set points are given as constant parameters";
				case 1 : return "Scheduled set point values";
			} break;
			// HydraulicNetworkControlElement::ControlledProperty
			case 12 :
			switch (t) {
				case 0 : return "Control temperature difference of this element";
				case 1 : return "Control temperature difference of the following element";
				case 2 : return "Control zone thermostat values";
				case 3 : return "Control mass flux";
				case 4 : return "Control pump operation depending on following element";
			} break;
			// HydraulicNetworkControlElement::ControllerType
			case 13 :
			switch (t) {
				case 0 : return "PController";
				case 1 : return "PIController";
				case 2 : return "OnOffController";
			} break;
			// HydraulicNetworkControlElement::para_t
			case 14 :
			switch (t) {
				case 0 : return "Kp-parameter";
				case 1 : return "Ki-parameter";
				case 2 : return "Kd-parameter";
				case 3 : return "Target temperature difference";
				case 4 : return "Target mass flux";
				case 5 : return "Threshold value for PumpOperation property when OnOffController is used";
			} break;
			// HydraulicNetworkControlElement::References
			case 15 :
			switch (t) {
				case 0 : return "ID of zone containing thermostat";
			} break;
			// HydraulicNetworkElement::para_t
			case 16 :
			switch (t) {
				case 0 : return "Pipe length";
			} break;
			// HydraulicNetworkElement::intPara_t
			case 17 :
			switch (t) {
				case 0 : return "Number of parallel pipes";
				case 1 : return "Number of parallel elements";
			} break;
			// HydraulicNetworkHeatExchange::ModelType
			case 18 :
			switch (t) {
				case 0 : return "Difference to constant temperature";
				case 1 : return "Difference to time-dependent temperature from spline";
				case 2 : return "Evaporator medium temperature for heat pump";
				case 3 : return "Difference to zone air temperature";
				case 4 : return "Difference to active construction layer (floor heating)";
				case 5 : return "Constant heat loss";
				case 6 : return "Heat loss from spline";
				case 7 : return "Heat loss of condenser in heat pump model";
			} break;
			// HydraulicNetworkHeatExchange::para_t
			case 19 :
			switch (t) {
				case 0 : return "Temperature for heat exchange";
				case 1 : return "Constant heat flux out of the element (heat loss)";
				case 2 : return "External heat transfer coeffient for the outside boundary";
			} break;
			// HydraulicNetworkHeatExchange::splinePara_t
			case 20 :
			switch (t) {
				case 0 : return "Temperature for heat exchange";
				case 1 : return "Constant heat flux out of the element (heat loss)";
			} break;
			// HydraulicNetworkHeatExchange::References
			case 21 :
			switch (t) {
				case 0 : return "ID of coupled zone for thermal exchange";
				case 1 : return "ID of coupled construction instance for thermal exchange";
			} break;
			// HydraulicNetworkPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "Roughness of pipe material";
				case 1 : return "Inner diameter of pipe";
				case 2 : return "Outer diameter of pipe";
				case 3 : return "Length-specific U-Value of pipe wall incl. insulation";
			} break;
			// IdealHeatingCoolingModel::para_t
			case 23 :
			switch (t) {
				case 0 : return "Maximum heating power per floor area";
				case 1 : return "Maximum cooling power per floor area";
				case 2 : return "Kp-parameter";
				case 3 : return "Ki-parameter";
			} break;
			// IdealPipeRegisterModel::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "Constant supply temperature";
				case 1 : return "Scheduled supply temperature";
			} break;
			// IdealPipeRegisterModel::para_t
			case 25 :
			switch (t) {
				case 0 : return "Medium supply temperature";
				case 1 : return "Maximum mass flux through the pipe";
				case 2 : return "Pipe length";
				case 3 : return "Inner diameter of pipe";
				case 4 : return "Length-specific U-Value of pipe wall incl. insulation";
			} break;
			// IdealPipeRegisterModel::intPara_t
			case 26 :
			switch (t) {
				case 0 : return "Number of parallel pipes";
			} break;
			// IdealSurfaceHeatingCoolingModel::para_t
			case 27 :
			switch (t) {
				case 0 : return "Maximum heating power per surface area";
				case 1 : return "Maximum cooling power per surface area";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 28 :
			switch (t) {
				case 0 : return "Pressure coeffient.";
			} break;
			// InterfaceAirFlow::modelType_t
			case 29 :
			switch (t) {
				case 0 : return "Use results from external wind flow calculation.";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 30 :
			switch (t) {
				case 0 : return "Constant heat exchange coefficient";
				case 1 : return "No convective heat exchange";
			} break;
			// InterfaceHeatConduction::para_t
			case 31 :
			switch (t) {
				case 0 : return "Convective heat transfer coefficient";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "Constant model";
				case 1 : return "No long wave radiation exchange";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 33 :
			switch (t) {
				case 0 : return "Long wave emissivity";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 34 :
			switch (t) {
				case 0 : return "Constant model";
				case 1 : return "No short wave radiation exchange";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 35 :
			switch (t) {
				case 0 : return "Solar absorption coefficient";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 36 :
			switch (t) {
				case 0 : return "Vapor Transfer Coefficient.";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 37 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InternalLoadsModel::modelType_t
			case 38 :
			switch (t) {
				case 0 : return "Constant internal loads";
				case 1 : return "Scheduled internal loads";
			} break;
			// InternalLoadsModel::para_t
			case 39 :
			switch (t) {
				case 0 : return "Percentage of equipment load that is radiant emitted.";
				case 1 : return "Percentage of person load that is radiant emitted.";
				case 2 : return "Percentage of lighting load that is radiant emitted.";
				case 3 : return "Complete equipment load per zone floor area.";
				case 4 : return "Complete person load per zone floor area.";
				case 5 : return "Complete lighting load per zone floor area.";
			} break;
			// Interval::para_t
			case 40 :
			switch (t) {
				case 0 : return "Start time point.";
				case 1 : return "End time point.";
				case 2 : return "StepSize.";
			} break;
			// KeywordList::MyParameters
			case 41 :
			switch (t) {
				case 0 : return "Some temperatures";
				case 1 : return "Some mass";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 42 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "constant";
				case 1 : if (no_description != nullptr) *no_description = true; return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 43 :
			switch (t) {
				case 0 : return "Continuous data";
				case 1 : return "Annual cycle";
			} break;
			// Location::para_t
			case 44 :
			switch (t) {
				case 0 : return "Latitude.";
				case 1 : return "Longitude.";
				case 2 : return "Albedo value [0..100 %].";
				case 3 : return "Altitude of building as height above NN [m].";
			} break;
			// Location::flag_t
			case 45 :
			switch (t) {
				case 0 : return "Use diffuse radiation model for anisotropic radiation (Perez)";
				case 1 : return "If true, shading factors for exterior shading are stored for continuous time points (no cyclic use)";
			} break;
			// Material::para_t
			case 46 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
			} break;
			// ModelInputReference::referenceType_t
			case 47 :
			switch (t) {
				case 0 : return "Model references of climate/location models.";
				case 1 : return "Model references inside a room.";
				case 2 : return "Model references a wall.";
				case 3 : return "Model references an embedded object.";
				case 4 : return "Model references generic scheduled data that is not associated with a specific object type.";
				case 5 : return "Model references of a model object.";
				case 6 : return "Model references of a hydraulic network.";
				case 7 : return "Model references of flow elements of a hydraulic network.";
			} break;
			// NaturalVentilationModel::modelType_t
			case 48 :
			switch (t) {
				case 0 : return "Constant ventilation rate (also can used as infiltration)";
				case 1 : return "Scheduled ventilation rate";
				case 2 : return "Constant basic air exchange (infiltration) with an additional increased air exchange (ventilation) if the control conditions are met.";
			} break;
			// NaturalVentilationModel::para_t
			case 49 :
			switch (t) {
				case 0 : return "Ventilation rate for Constant model";
				case 1 : return "Upper limit of comfort range";
				case 2 : return "Lower limit of comfort range";
				case 3 : return "Maximum wind speed to allow ventilation increase";
			} break;
			// OutputDefinition::timeType_t
			case 50 :
			switch (t) {
				case 0 : return "Write values as calculated at output times.";
				case 1 : return "Average values in time (mean value in output step).";
				case 2 : return "Integrate values in time.";
			} break;
			// Schedule::ScheduledDayType
			case 51 :
			switch (t) {
				case 0 : return "All days (Weekend days and Weekdays).";
				case 1 : return "Weekday schedule.";
				case 2 : return "Weekend schedule.";
				case 3 : return "Special Weekday schedule: Monday.";
				case 4 : return "Special Weekday schedule: Tuesday.";
				case 5 : return "Special Weekday schedule: Wednesday.";
				case 6 : return "Special Weekday schedule: Thursday.";
				case 7 : return "Special Weekday schedule: Friday.";
				case 8 : return "Special Weekday schedule: Saturday.";
				case 9 : return "Special Weekday schedule: Sunday.";
				case 10 : return "Holiday schedule.";
			} break;
			// Schedules::day_t
			case 52 :
			switch (t) {
				case 0 : return "Monday.";
				case 1 : return "Tuesday.";
				case 2 : return "Wednesday.";
				case 3 : return "Thursday.";
				case 4 : return "Friday.";
				case 5 : return "Saturday.";
				case 6 : return "Sunday.";
			} break;
			// Schedules::flag_t
			case 53 :
			switch (t) {
				case 0 : return "If enabled, schedules are treated as annually repeating schedules.";
			} break;
			// SerializationTest::test_t
			case 54 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "X1";
				case 1 : if (no_description != nullptr) *no_description = true; return "X2";
			} break;
			// SerializationTest::intPara_t
			case 55 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "I1";
				case 1 : if (no_description != nullptr) *no_description = true; return "I2";
			} break;
			// SerializationTest::splinePara_t
			case 56 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "ParameterSet1";
				case 1 : if (no_description != nullptr) *no_description = true; return "ParameterSet2";
			} break;
			// SerializationTest::ReferencedIDTypes
			case 57 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "SomeStove";
				case 1 : if (no_description != nullptr) *no_description = true; return "SomeOven";
				case 2 : if (no_description != nullptr) *no_description = true; return "SomeHeater";
				case 3 : if (no_description != nullptr) *no_description = true; return "SomeFurnace";
			} break;
			// ShadingControlModel::para_t
			case 58 :
			switch (t) {
				case 0 : return "Maximum intensity allowed before shading is closed.";
				case 1 : return "Intensity level below which shading is opened.";
			} break;
			// SimulationParameter::para_t
			case 59 :
			switch (t) {
				case 0 : return "Global initial temperature [C].";
				case 1 : return "Global initial relative humidity [%].";
				case 2 : return "Percentage of sensitive heat from domestic water istributed towrads the room.";
				case 3 : return "Air exchange rate resulting from a pressure difference of 50 Pa between inside and outside.";
				case 4 : return "Shielding coefficient for a given location and envelope type.";
				case 5 : return "Ambient temperature for a design day. Parameter that is needed for FMU export.";
			} break;
			// SimulationParameter::intPara_t
			case 60 :
			switch (t) {
				case 0 : return "Start year of the simulation.";
			} break;
			// SimulationParameter::flag_t
			case 61 :
			switch (t) {
				case 0 : return "Flag activating moisture balance calculation if enabled.";
				case 1 : return "Flag activating CO2 balance calculation if enabled.";
				case 2 : return "Flag activating ventilation through joints and openings.";
				case 3 : return "Flag activating FMU export of climate data.";
			} break;
			// SolarLoadsDistributionModel::distribution_t
			case 62 :
			switch (t) {
				case 0 : return "Distribution based on surface area";
				case 1 : return "Distribution based on surface type";
				case 2 : return "Distribution based on zone-specific view factors";
			} break;
			// SolarLoadsDistributionModel::para_t
			case 63 :
			switch (t) {
				case 0 : return "Percentage of solar radiation gains attributed direcly to room [%]";
				case 1 : return "Percentage of surface solar radiation attributed to floor [%]";
				case 2 : return "Percentage of surface solar radiation attributed to roof/ceiling[%]";
				case 3 : return "Percentage of surface solar radiation attributed to walls [%]";
			} break;
			// SolverParameter::para_t
			case 64 :
			switch (t) {
				case 0 : return "Relative tolerance for solver error check.";
				case 1 : return "Absolute tolerance for solver error check.";
				case 2 : return "Maximum permitted time step for integration.";
				case 3 : return "Minimum accepted time step, before solver aborts with error.";
				case 4 : return "Initial time step size (or constant step size for ExplicitEuler integrator).";
				case 5 : return "Coefficient reducing nonlinear equation solver convergence limit.";
				case 6 : return "Coefficient reducing iterative equation solver convergence limit.";
				case 7 : return "Minimum element width for wall discretization.";
				case 8 : return "Stretch factor for variable wall discretizations (0-no disc, 1-equidistance, larger than 1 - variable).";
				case 9 : return "Maximum dimension of a tile for calculation of view factors.";
				case 10 : return "Number of surface discretization elements of a wall in each direction.";
				case 11 : return "Temperature tolerance for ideal heating or cooling.";
				case 12 : return "Relative tolerance for Kinsol solver.";
				case 13 : return "Absolute tolerance for Kinsol solver.";
			} break;
			// SolverParameter::intPara_t
			case 65 :
			switch (t) {
				case 0 : return "Maximum level of fill-in to be used for ILU preconditioner.";
				case 1 : return "Maximum dimension of Krylov subspace.";
				case 2 : return "Maximum number of nonlinear iterations.";
				case 3 : return "Maximum order allowed for multi-step solver.";
				case 4 : return "Maximum nonlinear iterations for Kinsol solver.";
				case 5 : return "Maximum number of elements per layer.";
			} break;
			// SolverParameter::flag_t
			case 66 :
			switch (t) {
				case 0 : return "Check schedules to determine minimum distances between steps and adjust MaxTimeStep.";
				case 1 : return "Disable line search for steady state cycles.";
				case 2 : return "Enable strict Newton for steady state cycles.";
			} break;
			// SolverParameter::integrator_t
			case 67 :
			switch (t) {
				case 0 : return "CVODE based solver";
				case 1 : return "Explicit Euler solver";
				case 2 : return "Implicit Euler solver";
				case 3 : return "Automatic selection of integrator";
			} break;
			// SolverParameter::lesSolver_t
			case 68 :
			switch (t) {
				case 0 : return "Dense solver";
				case 1 : return "KLU sparse solver";
				case 2 : return "GMRES iterative solver";
				case 3 : return "BICGSTAB iterative solver";
				case 4 : return "Automatic selection of linear equation system solver";
			} break;
			// SolverParameter::precond_t
			case 69 :
			switch (t) {
				case 0 : return "Incomplete LU preconditioner";
				case 1 : return "Automatic selection of preconditioner";
			} break;
			// Thermostat::modelType_t
			case 70 :
			switch (t) {
				case 0 : return "Constant set points";
				case 1 : return "Scheduled set points";
			} break;
			// Thermostat::para_t
			case 71 :
			switch (t) {
				case 0 : return "Heating set point";
				case 1 : return "Cooling set point";
				case 2 : return "Control tolerance for temperatures";
				case 3 : return "Offset of lower and upper hysteresis band from set points";
			} break;
			// Thermostat::TemperatureType
			case 72 :
			switch (t) {
				case 0 : return "Air temperature";
				case 1 : return "Operative temperature";
			} break;
			// Thermostat::ControllerType
			case 73 :
			switch (t) {
				case 0 : return "Analog";
				case 1 : return "Digital";
			} break;
			// WindowGlazingLayer::type_t
			case 74 :
			switch (t) {
				case 0 : return "Gas layer";
				case 1 : return "Glass layer";
			} break;
			// WindowGlazingLayer::para_t
			case 75 :
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
			case 76 :
			switch (t) {
				case 0 : return "Short wave transmittance at outside directed surface.";
				case 1 : return "Short wave reflectance of surface facing outside.";
				case 2 : return "Short wave reflectance of surface facing inside.";
				case 3 : return "Thermal conductivity of the gas layer.";
				case 4 : return "Dynamic viscosity of the gas layer.";
				case 5 : return "Specific heat capacity of the gas layer.";
			} break;
			// WindowGlazingSystem::modelType_t
			case 77 :
			switch (t) {
				case 0 : return "Standard globbed-layers model.";
				case 1 : return "Detailed window model with layers.";
			} break;
			// WindowGlazingSystem::para_t
			case 78 :
			switch (t) {
				case 0 : return "Thermal transmittance";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 79 :
			switch (t) {
				case 0 : return "Short wave transmittance at outside directed surface.";
			} break;
			// WindowShading::modelType_t
			case 80 :
			switch (t) {
				case 0 : return "Constant reduction factor.";
				case 1 : return "Precomputed reduction factor.";
				case 2 : return "Reduction factor is computed based on control model";
			} break;
			// WindowShading::para_t
			case 81 :
			switch (t) {
				case 0 : return "Reduction factor (remaining percentage of solar gains if shading is closed).";
			} break;
			// Zone::type_t
			case 82 :
			switch (t) {
				case 0 : return "Zone with constant temperature";
				case 1 : return "Zone with schedule defined temperature";
				case 2 : return "Zone with energy balance equation";
				case 3 : return "Ground zone (calculates temperature based on heat loss to ground model)";
			} break;
			// Zone::para_t
			case 83 :
			switch (t) {
				case 0 : return "Temperature of the zone if set constant";
				case 1 : return "Relative humidity of the zone if set constant";
				case 2 : return "CO2 concentration of the zone if set constant";
				case 3 : return "Net usage area of the ground floor (for area-related outputs and loads)";
				case 4 : return "Zone air volume";
				case 5 : return "Extra heat capacity";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine description for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Description]");
	}

	const char * KeywordList::Unit(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// ConstructionInstance::para_t
			case 0 :
			switch (t) {
				case 0 : return "Deg";
				case 1 : return "Deg";
				case 2 : return "m2";
			} break;
			// DailyCycle::interpolation_t
			case 1 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// EmbeddedObject::para_t
			case 2 :
			switch (t) {
				case 0 : return "m2";
			} break;
			// EmbeddedObject::objectType_t
			case 3 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// HVACControlModel::modelType_t
			case 4 :
			switch (t) {
				case 0 : return "";
			} break;
			// HVACControlModel::OperatingMode
			case 5 :
			switch (t) {
				case 0 : return "";
			} break;
			// HydraulicFluid::para_t
			case 6 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
			} break;
			// HydraulicNetwork::ModelType
			case 7 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// HydraulicNetwork::para_t
			case 8 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "C";
				case 2 : return "Pa";
			} break;
			// HydraulicNetworkComponent::ModelType
			case 9 :
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
			} break;
			// HydraulicNetworkComponent::para_t
			case 10 :
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
			} break;
			// HydraulicNetworkControlElement::ModelType
			case 11 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// HydraulicNetworkControlElement::ControlledProperty
			case 12 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
			} break;
			// HydraulicNetworkControlElement::ControllerType
			case 13 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// HydraulicNetworkControlElement::para_t
			case 14 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "---";
				case 3 : return "K";
				case 4 : return "kg/s";
				case 5 : return "W";
			} break;
			// HydraulicNetworkControlElement::References
			case 15 :
			switch (t) {
				case 0 : return "-";
			} break;
			// HydraulicNetworkElement::para_t
			case 16 :
			switch (t) {
				case 0 : return "m";
			} break;
			// HydraulicNetworkElement::intPara_t
			case 17 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// HydraulicNetworkHeatExchange::ModelType
			case 18 :
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
			// HydraulicNetworkHeatExchange::para_t
			case 19 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "W";
				case 2 : return "W/m2K";
			} break;
			// HydraulicNetworkHeatExchange::splinePara_t
			case 20 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "W";
			} break;
			// HydraulicNetworkHeatExchange::References
			case 21 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
			} break;
			// HydraulicNetworkPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "mm";
				case 1 : return "mm";
				case 2 : return "mm";
				case 3 : return "W/mK";
			} break;
			// IdealHeatingCoolingModel::para_t
			case 23 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
				case 2 : return "---";
				case 3 : return "---";
			} break;
			// IdealPipeRegisterModel::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// IdealPipeRegisterModel::para_t
			case 25 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "kg/s";
				case 2 : return "m";
				case 3 : return "mm";
				case 4 : return "W/mK";
			} break;
			// IdealPipeRegisterModel::intPara_t
			case 26 :
			switch (t) {
				case 0 : return "";
			} break;
			// IdealSurfaceHeatingCoolingModel::para_t
			case 27 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 28 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceAirFlow::modelType_t
			case 29 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 30 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// InterfaceHeatConduction::para_t
			case 31 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 33 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 34 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 35 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 36 :
			switch (t) {
				case 0 : return "s/m";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 37 :
			switch (t) {
				case 0 : return "";
			} break;
			// InternalLoadsModel::modelType_t
			case 38 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// InternalLoadsModel::para_t
			case 39 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "---";
				case 3 : return "W/m2";
				case 4 : return "W/m2";
				case 5 : return "W/m2";
			} break;
			// Interval::para_t
			case 40 :
			switch (t) {
				case 0 : return "d";
				case 1 : return "d";
				case 2 : return "h";
			} break;
			// KeywordList::MyParameters
			case 41 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "kg";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 42 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 43 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Location::para_t
			case 44 :
			switch (t) {
				case 0 : return "Deg";
				case 1 : return "Deg";
				case 2 : return "%";
				case 3 : return "m";
			} break;
			// Location::flag_t
			case 45 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Material::para_t
			case 46 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
			} break;
			// ModelInputReference::referenceType_t
			case 47 :
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
			// NaturalVentilationModel::modelType_t
			case 48 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// NaturalVentilationModel::para_t
			case 49 :
			switch (t) {
				case 0 : return "1/h";
				case 1 : return "C";
				case 2 : return "C";
				case 3 : return "m/s";
			} break;
			// OutputDefinition::timeType_t
			case 50 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Schedule::ScheduledDayType
			case 51 :
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
			} break;
			// Schedules::day_t
			case 52 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
				case 6 : return "";
			} break;
			// Schedules::flag_t
			case 53 :
			switch (t) {
				case 0 : return "";
			} break;
			// SerializationTest::test_t
			case 54 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// SerializationTest::intPara_t
			case 55 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// SerializationTest::splinePara_t
			case 56 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// SerializationTest::ReferencedIDTypes
			case 57 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// ShadingControlModel::para_t
			case 58 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
			} break;
			// SimulationParameter::para_t
			case 59 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "---";
				case 3 : return "1/h";
				case 4 : return "---";
				case 5 : return "C";
			} break;
			// SimulationParameter::intPara_t
			case 60 :
			switch (t) {
				case 0 : return "";
			} break;
			// SimulationParameter::flag_t
			case 61 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SolarLoadsDistributionModel::distribution_t
			case 62 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// SolarLoadsDistributionModel::para_t
			case 63 :
			switch (t) {
				case 0 : return "%";
				case 1 : return "%";
				case 2 : return "%";
				case 3 : return "%";
			} break;
			// SolverParameter::para_t
			case 64 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "min";
				case 3 : return "s";
				case 4 : return "s";
				case 5 : return "---";
				case 6 : return "---";
				case 7 : return "m";
				case 8 : return "---";
				case 9 : return "m";
				case 10 : return "---";
				case 11 : return "K";
				case 12 : return "---";
				case 13 : return "---";
			} break;
			// SolverParameter::intPara_t
			case 65 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
			} break;
			// SolverParameter::flag_t
			case 66 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// SolverParameter::integrator_t
			case 67 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SolverParameter::lesSolver_t
			case 68 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
			} break;
			// SolverParameter::precond_t
			case 69 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Thermostat::modelType_t
			case 70 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Thermostat::para_t
			case 71 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "C";
				case 2 : return "K";
				case 3 : return "K";
			} break;
			// Thermostat::TemperatureType
			case 72 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Thermostat::ControllerType
			case 73 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// WindowGlazingLayer::type_t
			case 74 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// WindowGlazingLayer::para_t
			case 75 :
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
			case 76 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "---";
				case 3 : return "W/mK";
				case 4 : return "kg/ms";
				case 5 : return "J/kgK";
			} break;
			// WindowGlazingSystem::modelType_t
			case 77 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// WindowGlazingSystem::para_t
			case 78 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 79 :
			switch (t) {
				case 0 : return "---";
			} break;
			// WindowShading::modelType_t
			case 80 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// WindowShading::para_t
			case 81 :
			switch (t) {
				case 0 : return "---";
			} break;
			// Zone::type_t
			case 82 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// Zone::para_t
			case 83 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "g/m3";
				case 3 : return "m2";
				case 4 : return "m3";
				case 5 : return "J/K";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default unit for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Unit]");
	}

	const char * KeywordList::Color(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// ConstructionInstance::para_t
			case 0 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// DailyCycle::interpolation_t
			case 1 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// EmbeddedObject::para_t
			case 2 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// EmbeddedObject::objectType_t
			case 3 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// HVACControlModel::modelType_t
			case 4 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// HVACControlModel::OperatingMode
			case 5 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// HydraulicFluid::para_t
			case 6 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// HydraulicNetwork::ModelType
			case 7 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// HydraulicNetwork::para_t
			case 8 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// HydraulicNetworkComponent::ModelType
			case 9 :
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
			// HydraulicNetworkComponent::para_t
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
			} break;
			// HydraulicNetworkControlElement::ModelType
			case 11 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// HydraulicNetworkControlElement::ControlledProperty
			case 12 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// HydraulicNetworkControlElement::ControllerType
			case 13 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// HydraulicNetworkControlElement::para_t
			case 14 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// HydraulicNetworkControlElement::References
			case 15 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// HydraulicNetworkElement::para_t
			case 16 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// HydraulicNetworkElement::intPara_t
			case 17 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// HydraulicNetworkHeatExchange::ModelType
			case 18 :
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
			// HydraulicNetworkHeatExchange::para_t
			case 19 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// HydraulicNetworkHeatExchange::splinePara_t
			case 20 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// HydraulicNetworkHeatExchange::References
			case 21 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// HydraulicNetworkPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// IdealHeatingCoolingModel::para_t
			case 23 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// IdealPipeRegisterModel::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// IdealPipeRegisterModel::para_t
			case 25 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// IdealPipeRegisterModel::intPara_t
			case 26 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// IdealSurfaceHeatingCoolingModel::para_t
			case 27 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 28 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceAirFlow::modelType_t
			case 29 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 30 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::para_t
			case 31 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 32 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 33 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 34 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 35 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 36 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 37 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InternalLoadsModel::modelType_t
			case 38 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// InternalLoadsModel::para_t
			case 39 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// Interval::para_t
			case 40 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// KeywordList::MyParameters
			case 41 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 42 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 43 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Location::para_t
			case 44 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Location::flag_t
			case 45 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Material::para_t
			case 46 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// ModelInputReference::referenceType_t
			case 47 :
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
			// NaturalVentilationModel::modelType_t
			case 48 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// NaturalVentilationModel::para_t
			case 49 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// OutputDefinition::timeType_t
			case 50 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Schedule::ScheduledDayType
			case 51 :
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
			} break;
			// Schedules::day_t
			case 52 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
			} break;
			// Schedules::flag_t
			case 53 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// SerializationTest::test_t
			case 54 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SerializationTest::intPara_t
			case 55 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SerializationTest::splinePara_t
			case 56 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SerializationTest::ReferencedIDTypes
			case 57 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// ShadingControlModel::para_t
			case 58 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SimulationParameter::para_t
			case 59 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// SimulationParameter::intPara_t
			case 60 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// SimulationParameter::flag_t
			case 61 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolarLoadsDistributionModel::distribution_t
			case 62 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// SolarLoadsDistributionModel::para_t
			case 63 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolverParameter::para_t
			case 64 :
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
			// SolverParameter::intPara_t
			case 65 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// SolverParameter::flag_t
			case 66 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// SolverParameter::integrator_t
			case 67 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolverParameter::lesSolver_t
			case 68 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// SolverParameter::precond_t
			case 69 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Thermostat::modelType_t
			case 70 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Thermostat::para_t
			case 71 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Thermostat::TemperatureType
			case 72 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Thermostat::ControllerType
			case 73 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// WindowGlazingLayer::type_t
			case 74 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// WindowGlazingLayer::para_t
			case 75 :
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
			case 76 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::modelType_t
			case 77 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::para_t
			case 78 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::splinePara_t
			case 79 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowShading::modelType_t
			case 80 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// WindowShading::para_t
			case 81 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// Zone::type_t
			case 82 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Zone::para_t
			case 83 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine color for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::Color]");
	}

	double KeywordList::DefaultValue(const char * const enumtype, int t) {
		switch (enum2index(enumtype)) {
			// ConstructionInstance::para_t
			case 0 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// DailyCycle::interpolation_t
			case 1 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// EmbeddedObject::para_t
			case 2 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// EmbeddedObject::objectType_t
			case 3 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HVACControlModel::modelType_t
			case 4 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HVACControlModel::OperatingMode
			case 5 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicFluid::para_t
			case 6 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetwork::ModelType
			case 7 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetwork::para_t
			case 8 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkComponent::ModelType
			case 9 :
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
			// HydraulicNetworkComponent::para_t
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
			} break;
			// HydraulicNetworkControlElement::ModelType
			case 11 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkControlElement::ControlledProperty
			case 12 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkControlElement::ControllerType
			case 13 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkControlElement::para_t
			case 14 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkControlElement::References
			case 15 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkElement::para_t
			case 16 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkElement::intPara_t
			case 17 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkHeatExchange::ModelType
			case 18 :
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
			// HydraulicNetworkHeatExchange::para_t
			case 19 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkHeatExchange::splinePara_t
			case 20 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkHeatExchange::References
			case 21 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// HydraulicNetworkPipeProperties::para_t
			case 22 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// IdealHeatingCoolingModel::para_t
			case 23 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// IdealPipeRegisterModel::modelType_t
			case 24 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// IdealPipeRegisterModel::para_t
			case 25 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// IdealPipeRegisterModel::intPara_t
			case 26 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// IdealSurfaceHeatingCoolingModel::para_t
			case 27 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceAirFlow::splinePara_t
			case 28 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceAirFlow::modelType_t
			case 29 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::modelType_t
			case 30 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::para_t
			case 31 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 32 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceLongWaveEmission::para_t
			case 33 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 34 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceSolarAbsorption::para_t
			case 35 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceVaporDiffusion::para_t
			case 36 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 37 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoadsModel::modelType_t
			case 38 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InternalLoadsModel::para_t
			case 39 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Interval::para_t
			case 40 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// KeywordList::MyParameters
			case 41 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 42 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 43 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Location::para_t
			case 44 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Location::flag_t
			case 45 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Material::para_t
			case 46 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ModelInputReference::referenceType_t
			case 47 :
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
			// NaturalVentilationModel::modelType_t
			case 48 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NaturalVentilationModel::para_t
			case 49 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// OutputDefinition::timeType_t
			case 50 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Schedule::ScheduledDayType
			case 51 :
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
			} break;
			// Schedules::day_t
			case 52 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Schedules::flag_t
			case 53 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SerializationTest::test_t
			case 54 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SerializationTest::intPara_t
			case 55 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SerializationTest::splinePara_t
			case 56 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SerializationTest::ReferencedIDTypes
			case 57 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ShadingControlModel::para_t
			case 58 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::para_t
			case 59 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::intPara_t
			case 60 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::flag_t
			case 61 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolarLoadsDistributionModel::distribution_t
			case 62 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolarLoadsDistributionModel::para_t
			case 63 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::para_t
			case 64 :
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
			// SolverParameter::intPara_t
			case 65 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::flag_t
			case 66 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::integrator_t
			case 67 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::lesSolver_t
			case 68 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::precond_t
			case 69 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Thermostat::modelType_t
			case 70 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Thermostat::para_t
			case 71 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Thermostat::TemperatureType
			case 72 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Thermostat::ControllerType
			case 73 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingLayer::type_t
			case 74 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingLayer::para_t
			case 75 :
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
			case 76 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::modelType_t
			case 77 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::para_t
			case 78 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::splinePara_t
			case 79 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowShading::modelType_t
			case 80 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowShading::para_t
			case 81 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::type_t
			case 82 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::para_t
			case 83 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
		} // switch
		throw IBK::Exception(IBK::FormatString("Cannot determine default value for enumeration type '%1' and index '%2'.")
			.arg(enumtype).arg(t), "[KeywordList::DefaultValue]");
	}

	// number of entries in a keyword list
	unsigned int KeywordList::Count(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// ConstructionInstance::para_t
			case 0 : return 3;
			// DailyCycle::interpolation_t
			case 1 : return 2;
			// EmbeddedObject::para_t
			case 2 : return 1;
			// EmbeddedObject::objectType_t
			case 3 : return 3;
			// HVACControlModel::modelType_t
			case 4 : return 1;
			// HVACControlModel::OperatingMode
			case 5 : return 1;
			// HydraulicFluid::para_t
			case 6 : return 3;
			// HydraulicNetwork::ModelType
			case 7 : return 2;
			// HydraulicNetwork::para_t
			case 8 : return 3;
			// HydraulicNetworkComponent::ModelType
			case 9 : return 14;
			// HydraulicNetworkComponent::para_t
			case 10 : return 16;
			// HydraulicNetworkControlElement::ModelType
			case 11 : return 2;
			// HydraulicNetworkControlElement::ControlledProperty
			case 12 : return 5;
			// HydraulicNetworkControlElement::ControllerType
			case 13 : return 3;
			// HydraulicNetworkControlElement::para_t
			case 14 : return 6;
			// HydraulicNetworkControlElement::References
			case 15 : return 1;
			// HydraulicNetworkElement::para_t
			case 16 : return 1;
			// HydraulicNetworkElement::intPara_t
			case 17 : return 2;
			// HydraulicNetworkHeatExchange::ModelType
			case 18 : return 8;
			// HydraulicNetworkHeatExchange::para_t
			case 19 : return 3;
			// HydraulicNetworkHeatExchange::splinePara_t
			case 20 : return 2;
			// HydraulicNetworkHeatExchange::References
			case 21 : return 2;
			// HydraulicNetworkPipeProperties::para_t
			case 22 : return 4;
			// IdealHeatingCoolingModel::para_t
			case 23 : return 4;
			// IdealPipeRegisterModel::modelType_t
			case 24 : return 2;
			// IdealPipeRegisterModel::para_t
			case 25 : return 5;
			// IdealPipeRegisterModel::intPara_t
			case 26 : return 1;
			// IdealSurfaceHeatingCoolingModel::para_t
			case 27 : return 2;
			// InterfaceAirFlow::splinePara_t
			case 28 : return 1;
			// InterfaceAirFlow::modelType_t
			case 29 : return 1;
			// InterfaceHeatConduction::modelType_t
			case 30 : return 2;
			// InterfaceHeatConduction::para_t
			case 31 : return 1;
			// InterfaceLongWaveEmission::modelType_t
			case 32 : return 2;
			// InterfaceLongWaveEmission::para_t
			case 33 : return 1;
			// InterfaceSolarAbsorption::modelType_t
			case 34 : return 2;
			// InterfaceSolarAbsorption::para_t
			case 35 : return 1;
			// InterfaceVaporDiffusion::para_t
			case 36 : return 1;
			// InterfaceVaporDiffusion::modelType_t
			case 37 : return 1;
			// InternalLoadsModel::modelType_t
			case 38 : return 2;
			// InternalLoadsModel::para_t
			case 39 : return 6;
			// Interval::para_t
			case 40 : return 3;
			// KeywordList::MyParameters
			case 41 : return 2;
			// LinearSplineParameter::interpolationMethod_t
			case 42 : return 2;
			// LinearSplineParameter::wrapMethod_t
			case 43 : return 2;
			// Location::para_t
			case 44 : return 4;
			// Location::flag_t
			case 45 : return 2;
			// Material::para_t
			case 46 : return 3;
			// ModelInputReference::referenceType_t
			case 47 : return 8;
			// NaturalVentilationModel::modelType_t
			case 48 : return 3;
			// NaturalVentilationModel::para_t
			case 49 : return 4;
			// OutputDefinition::timeType_t
			case 50 : return 3;
			// Schedule::ScheduledDayType
			case 51 : return 11;
			// Schedules::day_t
			case 52 : return 7;
			// Schedules::flag_t
			case 53 : return 1;
			// SerializationTest::test_t
			case 54 : return 2;
			// SerializationTest::intPara_t
			case 55 : return 2;
			// SerializationTest::splinePara_t
			case 56 : return 2;
			// SerializationTest::ReferencedIDTypes
			case 57 : return 4;
			// ShadingControlModel::para_t
			case 58 : return 2;
			// SimulationParameter::para_t
			case 59 : return 6;
			// SimulationParameter::intPara_t
			case 60 : return 1;
			// SimulationParameter::flag_t
			case 61 : return 4;
			// SolarLoadsDistributionModel::distribution_t
			case 62 : return 3;
			// SolarLoadsDistributionModel::para_t
			case 63 : return 4;
			// SolverParameter::para_t
			case 64 : return 14;
			// SolverParameter::intPara_t
			case 65 : return 6;
			// SolverParameter::flag_t
			case 66 : return 3;
			// SolverParameter::integrator_t
			case 67 : return 4;
			// SolverParameter::lesSolver_t
			case 68 : return 5;
			// SolverParameter::precond_t
			case 69 : return 2;
			// Thermostat::modelType_t
			case 70 : return 2;
			// Thermostat::para_t
			case 71 : return 4;
			// Thermostat::TemperatureType
			case 72 : return 2;
			// Thermostat::ControllerType
			case 73 : return 2;
			// WindowGlazingLayer::type_t
			case 74 : return 2;
			// WindowGlazingLayer::para_t
			case 75 : return 7;
			// WindowGlazingLayer::splinePara_t
			case 76 : return 6;
			// WindowGlazingSystem::modelType_t
			case 77 : return 2;
			// WindowGlazingSystem::para_t
			case 78 : return 1;
			// WindowGlazingSystem::splinePara_t
			case 79 : return 1;
			// WindowShading::modelType_t
			case 80 : return 3;
			// WindowShading::para_t
			case 81 : return 1;
			// Zone::type_t
			case 82 : return 4;
			// Zone::para_t
			case 83 : return 6;
		} // switch
		throw IBK::Exception(IBK::FormatString("Invalid enumeration type '%1'.")
			.arg(enumtype), "[KeywordList::Count]");
	}

	// max index for entries sharing a category in a keyword list
	int KeywordList::MaxIndex(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// ConstructionInstance::para_t
			case 0 : return 2;
			// DailyCycle::interpolation_t
			case 1 : return 1;
			// EmbeddedObject::para_t
			case 2 : return 0;
			// EmbeddedObject::objectType_t
			case 3 : return 2;
			// HVACControlModel::modelType_t
			case 4 : return 0;
			// HVACControlModel::OperatingMode
			case 5 : return 0;
			// HydraulicFluid::para_t
			case 6 : return 2;
			// HydraulicNetwork::ModelType
			case 7 : return 1;
			// HydraulicNetwork::para_t
			case 8 : return 2;
			// HydraulicNetworkComponent::ModelType
			case 9 : return 13;
			// HydraulicNetworkComponent::para_t
			case 10 : return 15;
			// HydraulicNetworkControlElement::ModelType
			case 11 : return 1;
			// HydraulicNetworkControlElement::ControlledProperty
			case 12 : return 4;
			// HydraulicNetworkControlElement::ControllerType
			case 13 : return 2;
			// HydraulicNetworkControlElement::para_t
			case 14 : return 5;
			// HydraulicNetworkControlElement::References
			case 15 : return 0;
			// HydraulicNetworkElement::para_t
			case 16 : return 0;
			// HydraulicNetworkElement::intPara_t
			case 17 : return 1;
			// HydraulicNetworkHeatExchange::ModelType
			case 18 : return 7;
			// HydraulicNetworkHeatExchange::para_t
			case 19 : return 2;
			// HydraulicNetworkHeatExchange::splinePara_t
			case 20 : return 1;
			// HydraulicNetworkHeatExchange::References
			case 21 : return 1;
			// HydraulicNetworkPipeProperties::para_t
			case 22 : return 3;
			// IdealHeatingCoolingModel::para_t
			case 23 : return 3;
			// IdealPipeRegisterModel::modelType_t
			case 24 : return 1;
			// IdealPipeRegisterModel::para_t
			case 25 : return 4;
			// IdealPipeRegisterModel::intPara_t
			case 26 : return 0;
			// IdealSurfaceHeatingCoolingModel::para_t
			case 27 : return 1;
			// InterfaceAirFlow::splinePara_t
			case 28 : return 0;
			// InterfaceAirFlow::modelType_t
			case 29 : return 0;
			// InterfaceHeatConduction::modelType_t
			case 30 : return 1;
			// InterfaceHeatConduction::para_t
			case 31 : return 0;
			// InterfaceLongWaveEmission::modelType_t
			case 32 : return 1;
			// InterfaceLongWaveEmission::para_t
			case 33 : return 0;
			// InterfaceSolarAbsorption::modelType_t
			case 34 : return 1;
			// InterfaceSolarAbsorption::para_t
			case 35 : return 0;
			// InterfaceVaporDiffusion::para_t
			case 36 : return 0;
			// InterfaceVaporDiffusion::modelType_t
			case 37 : return 0;
			// InternalLoadsModel::modelType_t
			case 38 : return 1;
			// InternalLoadsModel::para_t
			case 39 : return 5;
			// Interval::para_t
			case 40 : return 2;
			// KeywordList::MyParameters
			case 41 : return 1;
			// LinearSplineParameter::interpolationMethod_t
			case 42 : return 1;
			// LinearSplineParameter::wrapMethod_t
			case 43 : return 1;
			// Location::para_t
			case 44 : return 3;
			// Location::flag_t
			case 45 : return 1;
			// Material::para_t
			case 46 : return 2;
			// ModelInputReference::referenceType_t
			case 47 : return 7;
			// NaturalVentilationModel::modelType_t
			case 48 : return 2;
			// NaturalVentilationModel::para_t
			case 49 : return 3;
			// OutputDefinition::timeType_t
			case 50 : return 2;
			// Schedule::ScheduledDayType
			case 51 : return 10;
			// Schedules::day_t
			case 52 : return 6;
			// Schedules::flag_t
			case 53 : return 0;
			// SerializationTest::test_t
			case 54 : return 1;
			// SerializationTest::intPara_t
			case 55 : return 1;
			// SerializationTest::splinePara_t
			case 56 : return 1;
			// SerializationTest::ReferencedIDTypes
			case 57 : return 3;
			// ShadingControlModel::para_t
			case 58 : return 1;
			// SimulationParameter::para_t
			case 59 : return 5;
			// SimulationParameter::intPara_t
			case 60 : return 0;
			// SimulationParameter::flag_t
			case 61 : return 3;
			// SolarLoadsDistributionModel::distribution_t
			case 62 : return 2;
			// SolarLoadsDistributionModel::para_t
			case 63 : return 3;
			// SolverParameter::para_t
			case 64 : return 13;
			// SolverParameter::intPara_t
			case 65 : return 5;
			// SolverParameter::flag_t
			case 66 : return 2;
			// SolverParameter::integrator_t
			case 67 : return 3;
			// SolverParameter::lesSolver_t
			case 68 : return 4;
			// SolverParameter::precond_t
			case 69 : return 1;
			// Thermostat::modelType_t
			case 70 : return 1;
			// Thermostat::para_t
			case 71 : return 3;
			// Thermostat::TemperatureType
			case 72 : return 1;
			// Thermostat::ControllerType
			case 73 : return 1;
			// WindowGlazingLayer::type_t
			case 74 : return 1;
			// WindowGlazingLayer::para_t
			case 75 : return 6;
			// WindowGlazingLayer::splinePara_t
			case 76 : return 5;
			// WindowGlazingSystem::modelType_t
			case 77 : return 1;
			// WindowGlazingSystem::para_t
			case 78 : return 0;
			// WindowGlazingSystem::splinePara_t
			case 79 : return 0;
			// WindowShading::modelType_t
			case 80 : return 2;
			// WindowShading::para_t
			case 81 : return 0;
			// Zone::type_t
			case 82 : return 3;
			// Zone::para_t
			case 83 : return 6;
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

} // namespace NANDRAD
