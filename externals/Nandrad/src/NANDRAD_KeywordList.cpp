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
	const char * const ENUM_TYPES[45] = {
		"ConstructionInstance::para_t",
		"DailyCycle::interpolation_t",
		"EmbeddedObject::para_t",
		"EmbeddedObject::objectType_t",
		"InterfaceAirFlow::splinePara_t",
		"InterfaceAirFlow::modelType_t",
		"InterfaceHeatConduction::para_t",
		"InterfaceHeatConduction::modelType_t",
		"InterfaceLongWaveEmission::para_t",
		"InterfaceLongWaveEmission::modelType_t",
		"InterfaceSolarAbsorption::para_t",
		"InterfaceSolarAbsorption::modelType_t",
		"InterfaceVaporDiffusion::para_t",
		"InterfaceVaporDiffusion::modelType_t",
		"Interval::para_t",
		"LinearSplineParameter::interpolationMethod_t",
		"LinearSplineParameter::wrapMethod_t",
		"Location::para_t",
		"Material::para_t",
		"ModelInputReference::referenceType_t",
		"NaturalVentilationModel::modelType_t",
		"NaturalVentilationModel::para_t",
		"OutputDefinition::timeType_t",
		"Schedule::type_t",
		"Schedules::day_t",
		"Schedules::flag_t",
		"SerializationTest::test_t",
		"SerializationTest::intPara_t",
		"ShadingControlModel::modelType_t",
		"ShadingControlModel::para_t",
		"SimulationParameter::para_t",
		"SimulationParameter::intPara_t",
		"SimulationParameter::flag_t",
		"SolverParameter::para_t",
		"SolverParameter::intPara_t",
		"SolverParameter::flag_t",
		"SolverParameter::integrator_t",
		"SolverParameter::lesSolver_t",
		"SolverParameter::precond_t",
		"WindowGlazingSystem::modelType_t",
		"WindowGlazingSystem::para_t",
		"WindowShading::modelType_t",
		"WindowShading::para_t",
		"Zone::type_t",
		"Zone::para_t"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<45; ++i) {
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
			// InterfaceAirFlow::splinePara_t
			case 4 :
			switch (t) {
				case 0 : return "PressureCoefficient";
			} break;
			// InterfaceAirFlow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "WindFlow";
			} break;
			// InterfaceHeatConduction::para_t
			case 6 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 8 :
			switch (t) {
				case 0 : return "Emissivity";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 10 :
			switch (t) {
				case 0 : return "AbsorptionCoefficient";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 12 :
			switch (t) {
				case 0 : return "VaporTransferCoefficient";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// Interval::para_t
			case 14 :
			switch (t) {
				case 0 : return "Start";
				case 1 : return "End";
				case 2 : return "StepSize";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 15 :
			switch (t) {
				case 0 : return "constant";
				case 1 : return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 16 :
			switch (t) {
				case 0 : return "continuous";
				case 1 : return "cyclic";
			} break;
			// Location::para_t
			case 17 :
			switch (t) {
				case 0 : return "Latitude";
				case 1 : return "Longitude";
				case 2 : return "Albedo";
				case 3 : return "Altitude";
			} break;
			// Material::para_t
			case 18 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// ModelInputReference::referenceType_t
			case 19 :
			switch (t) {
				case 0 : return "Location";
				case 1 : return "Zone";
				case 2 : return "ConstructionInstance";
				case 3 : return "EmbeddedObject";
				case 4 : return "ActiveObject";
				case 5 : return "Schedule";
				case 6 : return "ObjectList";
				case 7 : return "Model";
				case 8 : return "Global";
			} break;
			// NaturalVentilationModel::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// NaturalVentilationModel::para_t
			case 21 :
			switch (t) {
				case 0 : return "VentilationRate";
			} break;
			// OutputDefinition::timeType_t
			case 22 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// Schedule::type_t
			case 23 :
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
			case 24 :
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
			case 25 :
			switch (t) {
				case 0 : return "EnableCyclicSchedules";
			} break;
			// SerializationTest::test_t
			case 26 :
			switch (t) {
				case 0 : return "X1";
				case 1 : return "X2";
			} break;
			// SerializationTest::intPara_t
			case 27 :
			switch (t) {
				case 0 : return "I1";
				case 1 : return "I2";
			} break;
			// ShadingControlModel::modelType_t
			case 28 :
			switch (t) {
				case 0 : return "SingleIntensityControlled";
			} break;
			// ShadingControlModel::para_t
			case 29 :
			switch (t) {
				case 0 : return "MaxIntensity";
				case 1 : return "MinIntensity";
			} break;
			// SimulationParameter::para_t
			case 30 :
			switch (t) {
				case 0 : return "InitialTemperature";
				case 1 : return "InitialRelativeHumidity";
				case 2 : return "RadiationLoadFraction";
				case 3 : return "UserThermalRadiationFraction";
				case 4 : return "EquipmentThermalLossFraction";
				case 5 : return "EquipmentThermalRadiationFraction";
				case 6 : return "LightingVisibleRadiationFraction";
				case 7 : return "LightingThermalRadiationFraction";
				case 8 : return "DomesticWaterSensitiveHeatGainFraction";
				case 9 : return "AirExchangeRateN50";
				case 10 : return "ShieldingCoefficient";
				case 11 : return "HeatingDesignAmbientTemperature";
			} break;
			// SimulationParameter::intPara_t
			case 31 :
			switch (t) {
				case 0 : return "StartYear";
			} break;
			// SimulationParameter::flag_t
			case 32 :
			switch (t) {
				case 0 : return "EnableMoistureBalance";
				case 1 : return "EnableCO2Balance";
				case 2 : return "EnableJointVentilation";
				case 3 : return "ExportClimateDataFMU";
			} break;
			// SolverParameter::para_t
			case 33 :
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
			case 34 :
			switch (t) {
				case 0 : return "PreILUWidth";
				case 1 : return "MaxKrylovDim";
				case 2 : return "MaxNonlinIter";
				case 3 : return "MaxOrder";
				case 4 : return "KinsolMaxNonlinIter";
				case 5 : return "DiscMaxElementsPerLayer";
			} break;
			// SolverParameter::flag_t
			case 35 :
			switch (t) {
				case 0 : return "DetectMaxTimeStep";
				case 1 : return "KinsolDisableLineSearch";
				case 2 : return "KinsolStrictNewton";
			} break;
			// SolverParameter::integrator_t
			case 36 :
			switch (t) {
				case 0 : return "CVODE";
				case 1 : return "ExplicitEuler";
				case 2 : return "ImplicitEuler";
				case 3 : return "auto";
			} break;
			// SolverParameter::lesSolver_t
			case 37 :
			switch (t) {
				case 0 : return "Dense";
				case 1 : return "KLU";
				case 2 : return "GMRES";
				case 3 : return "BiCGStab";
				case 4 : return "auto";
			} break;
			// SolverParameter::precond_t
			case 38 :
			switch (t) {
				case 0 : return "ILU";
				case 1 : return "auto";
			} break;
			// WindowGlazingSystem::modelType_t
			case 39 :
			switch (t) {
				case 0 : return "Standard";
			} break;
			// WindowGlazingSystem::para_t
			case 40 :
			switch (t) {
				case 0 : return "ThermalTransmittance";
			} break;
			// WindowShading::modelType_t
			case 41 :
			switch (t) {
				case 0 : return "Standard";
			} break;
			// WindowShading::para_t
			case 42 :
			switch (t) {
				case 0 : return "ReductionFactor";
			} break;
			// Zone::type_t
			case 43 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Active";
				case 2 : return "Ground";
			} break;
			// Zone::para_t
			case 44 :
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
			// InterfaceAirFlow::splinePara_t
			case 4 :
			switch (t) {
				case 0 : return "PressureCoefficient";
			} break;
			// InterfaceAirFlow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "WindFlow";
			} break;
			// InterfaceHeatConduction::para_t
			case 6 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 8 :
			switch (t) {
				case 0 : return "Emissivity";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 10 :
			switch (t) {
				case 0 : return "AbsorptionCoefficient";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 12 :
			switch (t) {
				case 0 : return "VaporTransferCoefficient";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// Interval::para_t
			case 14 :
			switch (t) {
				case 0 : return "Start";
				case 1 : return "End";
				case 2 : return "StepSize";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 15 :
			switch (t) {
				case 0 : return "constant";
				case 1 : return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 16 :
			switch (t) {
				case 0 : return "continuous";
				case 1 : return "cyclic";
			} break;
			// Location::para_t
			case 17 :
			switch (t) {
				case 0 : return "Latitude";
				case 1 : return "Longitude";
				case 2 : return "Albedo";
				case 3 : return "Altitude";
			} break;
			// Material::para_t
			case 18 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// ModelInputReference::referenceType_t
			case 19 :
			switch (t) {
				case 0 : return "Location";
				case 1 : return "Zone";
				case 2 : return "ConstructionInstance";
				case 3 : return "EmbeddedObject";
				case 4 : return "ActiveObject";
				case 5 : return "Schedule";
				case 6 : return "ObjectList";
				case 7 : return "Model";
				case 8 : return "Global";
			} break;
			// NaturalVentilationModel::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Scheduled";
			} break;
			// NaturalVentilationModel::para_t
			case 21 :
			switch (t) {
				case 0 : return "VentilationRate";
			} break;
			// OutputDefinition::timeType_t
			case 22 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// Schedule::type_t
			case 23 :
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
			case 24 :
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
			case 25 :
			switch (t) {
				case 0 : return "EnableCyclicSchedules";
			} break;
			// SerializationTest::test_t
			case 26 :
			switch (t) {
				case 0 : return "X1";
				case 1 : return "X2";
			} break;
			// SerializationTest::intPara_t
			case 27 :
			switch (t) {
				case 0 : return "I1";
				case 1 : return "I2";
			} break;
			// ShadingControlModel::modelType_t
			case 28 :
			switch (t) {
				case 0 : return "SingleIntensityControlled";
			} break;
			// ShadingControlModel::para_t
			case 29 :
			switch (t) {
				case 0 : return "MaxIntensity";
				case 1 : return "MinIntensity";
			} break;
			// SimulationParameter::para_t
			case 30 :
			switch (t) {
				case 0 : return "InitialTemperature";
				case 1 : return "InitialRelativeHumidity";
				case 2 : return "RadiationLoadFraction";
				case 3 : return "UserThermalRadiationFraction";
				case 4 : return "EquipmentThermalLossFraction";
				case 5 : return "EquipmentThermalRadiationFraction";
				case 6 : return "LightingVisibleRadiationFraction";
				case 7 : return "LightingThermalRadiationFraction";
				case 8 : return "DomesticWaterSensitiveHeatGainFraction";
				case 9 : return "AirExchangeRateN50";
				case 10 : return "ShieldingCoefficient";
				case 11 : return "HeatingDesignAmbientTemperature";
			} break;
			// SimulationParameter::intPara_t
			case 31 :
			switch (t) {
				case 0 : return "StartYear";
			} break;
			// SimulationParameter::flag_t
			case 32 :
			switch (t) {
				case 0 : return "EnableMoistureBalance";
				case 1 : return "EnableCO2Balance";
				case 2 : return "EnableJointVentilation";
				case 3 : return "ExportClimateDataFMU";
			} break;
			// SolverParameter::para_t
			case 33 :
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
			case 34 :
			switch (t) {
				case 0 : return "PreILUWidth";
				case 1 : return "MaxKrylovDim";
				case 2 : return "MaxNonlinIter";
				case 3 : return "MaxOrder";
				case 4 : return "KinsolMaxNonlinIter";
				case 5 : return "DiscMaxElementsPerLayer";
			} break;
			// SolverParameter::flag_t
			case 35 :
			switch (t) {
				case 0 : return "DetectMaxTimeStep";
				case 1 : return "KinsolDisableLineSearch";
				case 2 : return "KinsolStrictNewton";
			} break;
			// SolverParameter::integrator_t
			case 36 :
			switch (t) {
				case 0 : return "CVODE";
				case 1 : return "ExplicitEuler";
				case 2 : return "ImplicitEuler";
				case 3 : return "auto";
			} break;
			// SolverParameter::lesSolver_t
			case 37 :
			switch (t) {
				case 0 : return "Dense";
				case 1 : return "KLU";
				case 2 : return "GMRES";
				case 3 : return "BiCGStab";
				case 4 : return "auto";
			} break;
			// SolverParameter::precond_t
			case 38 :
			switch (t) {
				case 0 : return "ILU";
				case 1 : return "auto";
			} break;
			// WindowGlazingSystem::modelType_t
			case 39 :
			switch (t) {
				case 0 : return "Standard";
			} break;
			// WindowGlazingSystem::para_t
			case 40 :
			switch (t) {
				case 0 : return "ThermalTransmittance";
			} break;
			// WindowShading::modelType_t
			case 41 :
			switch (t) {
				case 0 : return "Standard";
			} break;
			// WindowShading::para_t
			case 42 :
			switch (t) {
				case 0 : return "ReductionFactor";
			} break;
			// Zone::type_t
			case 43 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Active";
				case 2 : return "Ground";
			} break;
			// Zone::para_t
			case 44 :
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
			// InterfaceAirFlow::splinePara_t
			case 4 :
			switch (t) {
				case 0 : return "Pressure coeffient.";
			} break;
			// InterfaceAirFlow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "Use results from external wind flow calculation.";
			} break;
			// InterfaceHeatConduction::para_t
			case 6 :
			switch (t) {
				case 0 : return "Constant heat transfer coefficient.";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 8 :
			switch (t) {
				case 0 : return "Constant Long wave emissivity.";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 10 :
			switch (t) {
				case 0 : return "Constant Absorption coefficient [0,...,1].";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 12 :
			switch (t) {
				case 0 : return "Vapor Transfer Coefficient.";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// Interval::para_t
			case 14 :
			switch (t) {
				case 0 : return "Start time point.";
				case 1 : return "End time point.";
				case 2 : return "StepSize.";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 15 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "constant";
				case 1 : if (no_description != nullptr) *no_description = true; return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 16 :
			switch (t) {
				case 0 : return "Continuous data";
				case 1 : return "Annual cycle";
			} break;
			// Location::para_t
			case 17 :
			switch (t) {
				case 0 : return "Latitude.";
				case 1 : return "Longitude.";
				case 2 : return "Albedo value [0..100 %].";
				case 3 : return "Altitude of building as height above NN [m].";
			} break;
			// Material::para_t
			case 18 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
			} break;
			// ModelInputReference::referenceType_t
			case 19 :
			switch (t) {
				case 0 : return "Model references of climate/location models.";
				case 1 : return "Model references inside a room.";
				case 2 : return "Model references a wall.";
				case 3 : return "Model references an embedded object.";
				case 4 : return "Model references an active object.";
				case 5 : return "Model references scheduled data.";
				case 6 : return "Model references an object list-specific value.";
				case 7 : return "Model references another generic model.";
				case 8 : return "Model references to global physical quantities.";
			} break;
			// NaturalVentilationModel::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "Constant ventilation rate";
				case 1 : return "Scheduled ventilation rate";
			} break;
			// NaturalVentilationModel::para_t
			case 21 :
			switch (t) {
				case 0 : return "Ventilation rate";
			} break;
			// OutputDefinition::timeType_t
			case 22 :
			switch (t) {
				case 0 : return "Write values as calculated at output times.";
				case 1 : return "Average values in time (mean value in output step).";
				case 2 : return "Integrate values in time.";
			} break;
			// Schedule::type_t
			case 23 :
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
			case 24 :
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
			case 25 :
			switch (t) {
				case 0 : return "If enabled, schedules are treated as annually repeating schedules.";
			} break;
			// SerializationTest::test_t
			case 26 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "X1";
				case 1 : if (no_description != nullptr) *no_description = true; return "X2";
			} break;
			// SerializationTest::intPara_t
			case 27 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "I1";
				case 1 : if (no_description != nullptr) *no_description = true; return "I2";
			} break;
			// ShadingControlModel::modelType_t
			case 28 :
			switch (t) {
				case 0 : return "Simple hysteretic shading control based on global radiation sensor";
			} break;
			// ShadingControlModel::para_t
			case 29 :
			switch (t) {
				case 0 : return "Maximum intensity allowed before shading is closed.";
				case 1 : return "Intensity level below which shading is opened.";
			} break;
			// SimulationParameter::para_t
			case 30 :
			switch (t) {
				case 0 : return "Global initial temperature [C].";
				case 1 : return "Global initial relative humidity [%].";
				case 2 : return "Percentage of solar radiation gains attributed direcly to room 0..1.";
				case 3 : return "Percentage of heat that is emitted by long wave radiation from persons.";
				case 4 : return "Percentage of energy from equipment load that is not available as thermal heat.";
				case 5 : return "Percentage of heat that is emitted by long wave radiation from equipment.";
				case 6 : return "Percentage of energy from lighting that is transformed into visible short wave radiation.";
				case 7 : return "Percentage of heat that is emitted by long wave radiation from lighting.";
				case 8 : return "Percentage of sensitive heat from domestic water istributed towrads the room.";
				case 9 : return "Air exchange rate resulting from a pressure difference of 50 Pa between inside and outside.";
				case 10 : return "Shielding coefficient for a given location and envelope type.";
				case 11 : return "Ambient temperature for a design day. Parameter that is needed for FMU export.";
			} break;
			// SimulationParameter::intPara_t
			case 31 :
			switch (t) {
				case 0 : return "Start year of the simulation.";
			} break;
			// SimulationParameter::flag_t
			case 32 :
			switch (t) {
				case 0 : return "Flag activating moisture balance calculation if enabled.";
				case 1 : return "Flag activating CO2 balance calculation if enabled.";
				case 2 : return "Flag activating ventilation through joints and openings.";
				case 3 : return "Flag activating FMU export of climate data.";
			} break;
			// SolverParameter::para_t
			case 33 :
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
			case 34 :
			switch (t) {
				case 0 : return "Maximum level of fill-in to be used for ILU preconditioner.";
				case 1 : return "Maximum dimension of Krylov subspace.";
				case 2 : return "Maximum number of nonlinear iterations.";
				case 3 : return "Maximum order allowed for multi-step solver.";
				case 4 : return "Maximum nonlinear iterations for Kinsol solver.";
				case 5 : return "Maximum number of elements per layer.";
			} break;
			// SolverParameter::flag_t
			case 35 :
			switch (t) {
				case 0 : return "Check schedules to determine minimum distances between steps and adjust MaxTimeStep.";
				case 1 : return "Disable line search for steady state cycles.";
				case 2 : return "Enable strict Newton for steady state cycles.";
			} break;
			// SolverParameter::integrator_t
			case 36 :
			switch (t) {
				case 0 : return "CVODE based solver";
				case 1 : return "Explicit Euler solver";
				case 2 : return "Implicit Euler solver";
				case 3 : return "System selects integrator automatically.";
			} break;
			// SolverParameter::lesSolver_t
			case 37 :
			switch (t) {
				case 0 : return "Dense solver";
				case 1 : return "KLU sparse solver";
				case 2 : return "GMRES iterative solver";
				case 3 : return "BICGSTAB iterative solver";
				case 4 : return "System selects les solver automatically.";
			} break;
			// SolverParameter::precond_t
			case 38 :
			switch (t) {
				case 0 : return "Incomplete LU preconditioner";
				case 1 : return "System selects preconditioner automatically.";
			} break;
			// WindowGlazingSystem::modelType_t
			case 39 :
			switch (t) {
				case 0 : return "Standard globbed-layers model.";
			} break;
			// WindowGlazingSystem::para_t
			case 40 :
			switch (t) {
				case 0 : return "Thermal transmittance";
			} break;
			// WindowShading::modelType_t
			case 41 :
			switch (t) {
				case 0 : return "Standard reduction factor.";
			} break;
			// WindowShading::para_t
			case 42 :
			switch (t) {
				case 0 : return "Reduction factor (remaining percentage of solar gains if shading is closed).";
			} break;
			// Zone::type_t
			case 43 :
			switch (t) {
				case 0 : return "Zone with constant/predefined temperatures. (schedule)";
				case 1 : return "Zone described by a temperature node in space.";
				case 2 : return "Ground zone (calculates temperature based on standard).";
			} break;
			// Zone::para_t
			case 44 :
			switch (t) {
				case 0 : return "Temperature of the zone if set constant [C].";
				case 1 : return "Relative humidity of the zone if set constant [%].";
				case 2 : return "CO2 concentration of the zone if set constant [g/m3].";
				case 3 : return "Net usage area of the ground floor [m2] (for area-related outputs and loads).";
				case 4 : return "Zone air volume [m3].";
				case 5 : return "Extra heat capacity [J/K].";
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
			// InterfaceAirFlow::splinePara_t
			case 4 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceAirFlow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceHeatConduction::para_t
			case 6 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 8 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 10 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 12 :
			switch (t) {
				case 0 : return "s/m";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "";
			} break;
			// Interval::para_t
			case 14 :
			switch (t) {
				case 0 : return "d";
				case 1 : return "d";
				case 2 : return "h";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 15 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 16 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Location::para_t
			case 17 :
			switch (t) {
				case 0 : return "Deg";
				case 1 : return "Deg";
				case 2 : return "%";
				case 3 : return "m";
			} break;
			// Material::para_t
			case 18 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
			} break;
			// ModelInputReference::referenceType_t
			case 19 :
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
			// NaturalVentilationModel::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// NaturalVentilationModel::para_t
			case 21 :
			switch (t) {
				case 0 : return "1/h";
			} break;
			// OutputDefinition::timeType_t
			case 22 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Schedule::type_t
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
			} break;
			// Schedules::day_t
			case 24 :
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
			case 25 :
			switch (t) {
				case 0 : return "";
			} break;
			// SerializationTest::test_t
			case 26 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// SerializationTest::intPara_t
			case 27 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// ShadingControlModel::modelType_t
			case 28 :
			switch (t) {
				case 0 : return "";
			} break;
			// ShadingControlModel::para_t
			case 29 :
			switch (t) {
				case 0 : return "W/m2";
				case 1 : return "W/m2";
			} break;
			// SimulationParameter::para_t
			case 30 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "%";
				case 3 : return "---";
				case 4 : return "---";
				case 5 : return "---";
				case 6 : return "---";
				case 7 : return "---";
				case 8 : return "---";
				case 9 : return "1/h";
				case 10 : return "---";
				case 11 : return "C";
			} break;
			// SimulationParameter::intPara_t
			case 31 :
			switch (t) {
				case 0 : return "";
			} break;
			// SimulationParameter::flag_t
			case 32 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SolverParameter::para_t
			case 33 :
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
			case 34 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
			} break;
			// SolverParameter::flag_t
			case 35 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// SolverParameter::integrator_t
			case 36 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SolverParameter::lesSolver_t
			case 37 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
			} break;
			// SolverParameter::precond_t
			case 38 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// WindowGlazingSystem::modelType_t
			case 39 :
			switch (t) {
				case 0 : return "";
			} break;
			// WindowGlazingSystem::para_t
			case 40 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// WindowShading::modelType_t
			case 41 :
			switch (t) {
				case 0 : return "";
			} break;
			// WindowShading::para_t
			case 42 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// Zone::type_t
			case 43 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Zone::para_t
			case 44 :
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
			// InterfaceAirFlow::splinePara_t
			case 4 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceAirFlow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::para_t
			case 6 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 8 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 10 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 12 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// Interval::para_t
			case 14 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 15 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 16 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Location::para_t
			case 17 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Material::para_t
			case 18 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// ModelInputReference::referenceType_t
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
			} break;
			// NaturalVentilationModel::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// NaturalVentilationModel::para_t
			case 21 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// OutputDefinition::timeType_t
			case 22 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Schedule::type_t
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
			} break;
			// Schedules::day_t
			case 24 :
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
			case 25 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// SerializationTest::test_t
			case 26 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SerializationTest::intPara_t
			case 27 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ShadingControlModel::modelType_t
			case 28 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// ShadingControlModel::para_t
			case 29 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SimulationParameter::para_t
			case 30 :
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
			// SimulationParameter::intPara_t
			case 31 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// SimulationParameter::flag_t
			case 32 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolverParameter::para_t
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
				case 8 : return "#FFFFFF";
				case 9 : return "#FFFFFF";
				case 10 : return "#FFFFFF";
				case 11 : return "#FFFFFF";
				case 12 : return "#FFFFFF";
				case 13 : return "#FFFFFF";
			} break;
			// SolverParameter::intPara_t
			case 34 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// SolverParameter::flag_t
			case 35 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// SolverParameter::integrator_t
			case 36 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolverParameter::lesSolver_t
			case 37 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// SolverParameter::precond_t
			case 38 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::modelType_t
			case 39 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowGlazingSystem::para_t
			case 40 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowShading::modelType_t
			case 41 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// WindowShading::para_t
			case 42 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// Zone::type_t
			case 43 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Zone::para_t
			case 44 :
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
			// InterfaceAirFlow::splinePara_t
			case 4 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceAirFlow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::para_t
			case 6 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::modelType_t
			case 7 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceLongWaveEmission::para_t
			case 8 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 9 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceSolarAbsorption::para_t
			case 10 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 11 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceVaporDiffusion::para_t
			case 12 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 13 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Interval::para_t
			case 14 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 15 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 16 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Location::para_t
			case 17 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Material::para_t
			case 18 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ModelInputReference::referenceType_t
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
			} break;
			// NaturalVentilationModel::modelType_t
			case 20 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// NaturalVentilationModel::para_t
			case 21 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// OutputDefinition::timeType_t
			case 22 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Schedule::type_t
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
			} break;
			// Schedules::day_t
			case 24 :
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
			case 25 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SerializationTest::test_t
			case 26 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SerializationTest::intPara_t
			case 27 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ShadingControlModel::modelType_t
			case 28 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ShadingControlModel::para_t
			case 29 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::para_t
			case 30 :
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
			// SimulationParameter::intPara_t
			case 31 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::flag_t
			case 32 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::para_t
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
				case 8 : return std::numeric_limits<double>::quiet_NaN();
				case 9 : return std::numeric_limits<double>::quiet_NaN();
				case 10 : return std::numeric_limits<double>::quiet_NaN();
				case 11 : return std::numeric_limits<double>::quiet_NaN();
				case 12 : return std::numeric_limits<double>::quiet_NaN();
				case 13 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::intPara_t
			case 34 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::flag_t
			case 35 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::integrator_t
			case 36 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::lesSolver_t
			case 37 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::precond_t
			case 38 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::modelType_t
			case 39 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowGlazingSystem::para_t
			case 40 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowShading::modelType_t
			case 41 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// WindowShading::para_t
			case 42 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::type_t
			case 43 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::para_t
			case 44 :
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
			// InterfaceAirFlow::splinePara_t
			case 4 : return 1;
			// InterfaceAirFlow::modelType_t
			case 5 : return 1;
			// InterfaceHeatConduction::para_t
			case 6 : return 1;
			// InterfaceHeatConduction::modelType_t
			case 7 : return 1;
			// InterfaceLongWaveEmission::para_t
			case 8 : return 1;
			// InterfaceLongWaveEmission::modelType_t
			case 9 : return 1;
			// InterfaceSolarAbsorption::para_t
			case 10 : return 1;
			// InterfaceSolarAbsorption::modelType_t
			case 11 : return 1;
			// InterfaceVaporDiffusion::para_t
			case 12 : return 1;
			// InterfaceVaporDiffusion::modelType_t
			case 13 : return 1;
			// Interval::para_t
			case 14 : return 3;
			// LinearSplineParameter::interpolationMethod_t
			case 15 : return 2;
			// LinearSplineParameter::wrapMethod_t
			case 16 : return 2;
			// Location::para_t
			case 17 : return 4;
			// Material::para_t
			case 18 : return 3;
			// ModelInputReference::referenceType_t
			case 19 : return 9;
			// NaturalVentilationModel::modelType_t
			case 20 : return 2;
			// NaturalVentilationModel::para_t
			case 21 : return 1;
			// OutputDefinition::timeType_t
			case 22 : return 3;
			// Schedule::type_t
			case 23 : return 11;
			// Schedules::day_t
			case 24 : return 7;
			// Schedules::flag_t
			case 25 : return 1;
			// SerializationTest::test_t
			case 26 : return 2;
			// SerializationTest::intPara_t
			case 27 : return 2;
			// ShadingControlModel::modelType_t
			case 28 : return 1;
			// ShadingControlModel::para_t
			case 29 : return 2;
			// SimulationParameter::para_t
			case 30 : return 12;
			// SimulationParameter::intPara_t
			case 31 : return 1;
			// SimulationParameter::flag_t
			case 32 : return 4;
			// SolverParameter::para_t
			case 33 : return 14;
			// SolverParameter::intPara_t
			case 34 : return 6;
			// SolverParameter::flag_t
			case 35 : return 3;
			// SolverParameter::integrator_t
			case 36 : return 4;
			// SolverParameter::lesSolver_t
			case 37 : return 5;
			// SolverParameter::precond_t
			case 38 : return 2;
			// WindowGlazingSystem::modelType_t
			case 39 : return 1;
			// WindowGlazingSystem::para_t
			case 40 : return 1;
			// WindowShading::modelType_t
			case 41 : return 1;
			// WindowShading::para_t
			case 42 : return 1;
			// Zone::type_t
			case 43 : return 3;
			// Zone::para_t
			case 44 : return 6;
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
			// InterfaceAirFlow::splinePara_t
			case 4 : return 0;
			// InterfaceAirFlow::modelType_t
			case 5 : return 0;
			// InterfaceHeatConduction::para_t
			case 6 : return 0;
			// InterfaceHeatConduction::modelType_t
			case 7 : return 0;
			// InterfaceLongWaveEmission::para_t
			case 8 : return 0;
			// InterfaceLongWaveEmission::modelType_t
			case 9 : return 0;
			// InterfaceSolarAbsorption::para_t
			case 10 : return 0;
			// InterfaceSolarAbsorption::modelType_t
			case 11 : return 0;
			// InterfaceVaporDiffusion::para_t
			case 12 : return 0;
			// InterfaceVaporDiffusion::modelType_t
			case 13 : return 0;
			// Interval::para_t
			case 14 : return 2;
			// LinearSplineParameter::interpolationMethod_t
			case 15 : return 1;
			// LinearSplineParameter::wrapMethod_t
			case 16 : return 1;
			// Location::para_t
			case 17 : return 3;
			// Material::para_t
			case 18 : return 2;
			// ModelInputReference::referenceType_t
			case 19 : return 8;
			// NaturalVentilationModel::modelType_t
			case 20 : return 1;
			// NaturalVentilationModel::para_t
			case 21 : return 0;
			// OutputDefinition::timeType_t
			case 22 : return 2;
			// Schedule::type_t
			case 23 : return 10;
			// Schedules::day_t
			case 24 : return 6;
			// Schedules::flag_t
			case 25 : return 0;
			// SerializationTest::test_t
			case 26 : return 1;
			// SerializationTest::intPara_t
			case 27 : return 1;
			// ShadingControlModel::modelType_t
			case 28 : return 0;
			// ShadingControlModel::para_t
			case 29 : return 1;
			// SimulationParameter::para_t
			case 30 : return 11;
			// SimulationParameter::intPara_t
			case 31 : return 0;
			// SimulationParameter::flag_t
			case 32 : return 3;
			// SolverParameter::para_t
			case 33 : return 13;
			// SolverParameter::intPara_t
			case 34 : return 5;
			// SolverParameter::flag_t
			case 35 : return 2;
			// SolverParameter::integrator_t
			case 36 : return 3;
			// SolverParameter::lesSolver_t
			case 37 : return 4;
			// SolverParameter::precond_t
			case 38 : return 1;
			// WindowGlazingSystem::modelType_t
			case 39 : return 0;
			// WindowGlazingSystem::para_t
			case 40 : return 0;
			// WindowShading::modelType_t
			case 41 : return 0;
			// WindowShading::para_t
			case 42 : return 0;
			// Zone::type_t
			case 43 : return 2;
			// Zone::para_t
			case 44 : return 6;
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

} // namespace NANDRAD
