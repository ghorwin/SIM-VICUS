/*	The NANDRAD data model library.
Copyright (c) 2012-now, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
All rights reserved.

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
	const char * const ENUM_TYPES[39] = {
		"ConstructionInstance::para_t",
		"DailyCycle::interpolation_t",
		"EmbeddedObject::para_t",
		"EmbeddedObject::objectType_t",
		"EmbeddedObjectWindow::para_t",
		"EmbeddedObjectWindow::modelType_t",
		"Interface::location_t",
		"Interface::condition_t",
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
		"OutputDefinition::timeType_t",
		"Schedule::type_t",
		"Schedules::day_t",
		"SerializationTest::test_t",
		"SerializationTest::intPara_t",
		"SimulationParameter::para_t",
		"SimulationParameter::intpara_t",
		"SimulationParameter::flag_t",
		"SolverParameter::para_t",
		"SolverParameter::flag_t",
		"SolverParameter::integrator_t",
		"SolverParameter::lesSolver_t",
		"SolverParameter::precond_t",
		"Zone::type_t",
		"Zone::para_t"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<39; ++i) {
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
			// EmbeddedObjectWindow::para_t
			case 4 :
			switch (t) {
				case 0 : return "GlassFraction";
				case 1 : return "SolarHeatGainCoefficient";
				case 2 : return "ThermalTransmittance";
				case 3 : return "ShadingFactor";
				case 4 : return "LeakageCoefficient";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Detailed";
				case 2 : return "DetailedWithStorage";
			} break;
			// Interface::location_t
			case 6 :
			switch (t) {
				case 0 : return "A";
				case 1 : return "B";
			} break;
			// Interface::condition_t
			case 7 :
			switch (t) {
				case 0 : return "HeatConduction";
				case 1 : return "SolarAbsorption";
				case 2 : return "LongWaveEmission";
				case 3 : return "VaporDiffusion";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 8 :
			switch (t) {
				case 0 : return "PressureCoefficient";
			} break;
			// InterfaceAirFlow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "WindFlow";
			} break;
			// InterfaceHeatConduction::para_t
			case 10 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 12 :
			switch (t) {
				case 0 : return "Emissivity";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 14 :
			switch (t) {
				case 0 : return "AbsorptionCoefficient";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 15 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 16 :
			switch (t) {
				case 0 : return "VaporTransferCoefficient";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 17 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// Interval::para_t
			case 18 :
			switch (t) {
				case 0 : return "Start";
				case 1 : return "End";
				case 2 : return "StepSize";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 19 :
			switch (t) {
				case 0 : return "constant";
				case 1 : return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 20 :
			switch (t) {
				case 0 : return "continuous";
				case 1 : return "cyclic";
			} break;
			// Location::para_t
			case 21 :
			switch (t) {
				case 0 : return "Latitude";
				case 1 : return "Longitude";
				case 2 : return "Albedo";
				case 3 : return "Altitude";
			} break;
			// Material::para_t
			case 22 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// ModelInputReference::referenceType_t
			case 23 :
			switch (t) {
				case 0 : return "Location";
				case 1 : return "Zone";
				case 2 : return "ConstructionInstance";
				case 3 : return "Interface";
				case 4 : return "EmbeddedObject";
				case 5 : return "ActiveObject";
				case 6 : return "Sensor";
				case 7 : return "Schedule";
				case 8 : return "ObjectList";
				case 9 : return "Model";
				case 10 : return "Global";
			} break;
			// OutputDefinition::timeType_t
			case 24 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// Schedule::type_t
			case 25 :
			switch (t) {
				case 0 : return "AllDays";
				case 1 : return "WeekDay";
				case 2 : return "WeekEnd";
				case 3 : return "Holiday";
				case 4 : return "Monday";
				case 5 : return "Tuesday";
				case 6 : return "Wednesday";
				case 7 : return "Thursday";
				case 8 : return "Friday";
				case 9 : return "Saturday";
				case 10 : return "Sunday";
			} break;
			// Schedules::day_t
			case 26 :
			switch (t) {
				case 0 : return "Mon";
				case 1 : return "Tue";
				case 2 : return "Wed";
				case 3 : return "Thu";
				case 4 : return "Fri";
				case 5 : return "Sat";
				case 6 : return "Sun";
			} break;
			// SerializationTest::test_t
			case 27 :
			switch (t) {
				case 0 : return "X1";
				case 1 : return "X2";
			} break;
			// SerializationTest::intPara_t
			case 28 :
			switch (t) {
				case 0 : return "I1";
				case 1 : return "I2";
			} break;
			// SimulationParameter::para_t
			case 29 :
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
			// SimulationParameter::intpara_t
			case 30 :
			switch (t) {
				case 0 : return "StartYear";
			} break;
			// SimulationParameter::flag_t
			case 31 :
			switch (t) {
				case 0 : return "EnableMoistureBalance";
				case 1 : return "EnableCO2Balance";
				case 2 : return "EnableJointVentilation";
				case 3 : return "ExportClimateDataFMU";
			} break;
			// SolverParameter::para_t
			case 32 :
			switch (t) {
				case 0 : return "RelTol";
				case 1 : return "AbsTol";
				case 2 : return "MaxTimeStep";
				case 3 : return "MinTimeStep";
				case 4 : return "InitialTimeStep";
				case 5 : return "NonlinSolverConvCoeff";
				case 6 : return "IterativeSolverConvCoeff";
				case 7 : return "MaxOrder";
				case 8 : return "MaxKrylovDim";
				case 9 : return "MaxNonlinIter";
				case 10 : return "LESBandWidth";
				case 11 : return "PreBandWidth";
				case 12 : return "PreILUWidth";
				case 13 : return "DiscMinDx";
				case 14 : return "DiscDetailLevel";
				case 15 : return "ViewfactorTileWidth";
				case 16 : return "SurfaceDiscretizationDensity";
				case 17 : return "ControlTemperatureTolerance";
				case 18 : return "KinsolRelTol";
				case 19 : return "KinsolAbsTol";
				case 20 : return "KinsolMaxNonlinIter";
				case 21 : return "IntegralWeightsFactor";
			} break;
			// SolverParameter::flag_t
			case 33 :
			switch (t) {
				case 0 : return "DetectMaxTimeStep";
				case 1 : return "KinsolDisableLineSearch";
				case 2 : return "KinsolStrictNewton";
			} break;
			// SolverParameter::integrator_t
			case 34 :
			switch (t) {
				case 0 : return "CVODE";
				case 1 : return "ExplicitEuler";
				case 2 : return "ImplicitEuler";
				case 3 : return "auto";
			} break;
			// SolverParameter::lesSolver_t
			case 35 :
			switch (t) {
				case 0 : return "Dense";
				case 1 : return "KLU";
				case 2 : return "GMRES";
				case 3 : return "BiCGStab";
				case 4 : return "auto";
			} break;
			// SolverParameter::precond_t
			case 36 :
			switch (t) {
				case 0 : return "Band";
				case 1 : return "ILU";
				case 2 : return "auto";
			} break;
			// Zone::type_t
			case 37 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Active";
				case 2 : return "Ground";
			} break;
			// Zone::para_t
			case 38 :
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
			// EmbeddedObjectWindow::para_t
			case 4 :
			switch (t) {
				case 0 : return "GlassFraction";
				case 1 : return "SolarHeatGainCoefficient";
				case 2 : return "ThermalTransmittance";
				case 3 : return "ShadingFactor";
				case 4 : return "LeakageCoefficient";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Detailed";
				case 2 : return "DetailedWithStorage";
			} break;
			// Interface::location_t
			case 6 :
			switch (t) {
				case 0 : return "A";
				case 1 : return "B";
			} break;
			// Interface::condition_t
			case 7 :
			switch (t) {
				case 0 : return "HeatConduction";
				case 1 : return "SolarAbsorption";
				case 2 : return "LongWaveEmission";
				case 3 : return "VaporDiffusion";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 8 :
			switch (t) {
				case 0 : return "PressureCoefficient";
			} break;
			// InterfaceAirFlow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "WindFlow";
			} break;
			// InterfaceHeatConduction::para_t
			case 10 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 12 :
			switch (t) {
				case 0 : return "Emissivity";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 14 :
			switch (t) {
				case 0 : return "AbsorptionCoefficient";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 15 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 16 :
			switch (t) {
				case 0 : return "VaporTransferCoefficient";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 17 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// Interval::para_t
			case 18 :
			switch (t) {
				case 0 : return "Start";
				case 1 : return "End";
				case 2 : return "StepSize";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 19 :
			switch (t) {
				case 0 : return "constant";
				case 1 : return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 20 :
			switch (t) {
				case 0 : return "continuous";
				case 1 : return "cyclic";
			} break;
			// Location::para_t
			case 21 :
			switch (t) {
				case 0 : return "Latitude";
				case 1 : return "Longitude";
				case 2 : return "Albedo";
				case 3 : return "Altitude";
			} break;
			// Material::para_t
			case 22 :
			switch (t) {
				case 0 : return "Density";
				case 1 : return "HeatCapacity";
				case 2 : return "Conductivity";
			} break;
			// ModelInputReference::referenceType_t
			case 23 :
			switch (t) {
				case 0 : return "Location";
				case 1 : return "Zone";
				case 2 : return "ConstructionInstance";
				case 3 : return "Interface";
				case 4 : return "EmbeddedObject";
				case 5 : return "ActiveObject";
				case 6 : return "Sensor";
				case 7 : return "Schedule";
				case 8 : return "ObjectList";
				case 9 : return "Model";
				case 10 : return "Global";
			} break;
			// OutputDefinition::timeType_t
			case 24 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// Schedule::type_t
			case 25 :
			switch (t) {
				case 0 : return "AllDays";
				case 1 : return "WeekDay";
				case 2 : return "WeekEnd";
				case 3 : return "Holiday";
				case 4 : return "Monday";
				case 5 : return "Tuesday";
				case 6 : return "Wednesday";
				case 7 : return "Thursday";
				case 8 : return "Friday";
				case 9 : return "Saturday";
				case 10 : return "Sunday";
			} break;
			// Schedules::day_t
			case 26 :
			switch (t) {
				case 0 : return "Mon";
				case 1 : return "Tue";
				case 2 : return "Wed";
				case 3 : return "Thu";
				case 4 : return "Fri";
				case 5 : return "Sat";
				case 6 : return "Sun";
			} break;
			// SerializationTest::test_t
			case 27 :
			switch (t) {
				case 0 : return "X1";
				case 1 : return "X2";
			} break;
			// SerializationTest::intPara_t
			case 28 :
			switch (t) {
				case 0 : return "I1";
				case 1 : return "I2";
			} break;
			// SimulationParameter::para_t
			case 29 :
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
			// SimulationParameter::intpara_t
			case 30 :
			switch (t) {
				case 0 : return "StartYear";
			} break;
			// SimulationParameter::flag_t
			case 31 :
			switch (t) {
				case 0 : return "EnableMoistureBalance";
				case 1 : return "EnableCO2Balance";
				case 2 : return "EnableJointVentilation";
				case 3 : return "ExportClimateDataFMU";
			} break;
			// SolverParameter::para_t
			case 32 :
			switch (t) {
				case 0 : return "RelTol";
				case 1 : return "AbsTol";
				case 2 : return "MaxTimeStep";
				case 3 : return "MinTimeStep";
				case 4 : return "InitialTimeStep";
				case 5 : return "NonlinSolverConvCoeff";
				case 6 : return "IterativeSolverConvCoeff";
				case 7 : return "MaxOrder";
				case 8 : return "MaxKrylovDim";
				case 9 : return "MaxNonlinIter";
				case 10 : return "LESBandWidth";
				case 11 : return "PreBandWidth";
				case 12 : return "PreILUWidth";
				case 13 : return "DiscMinDx";
				case 14 : return "DiscDetailLevel";
				case 15 : return "ViewfactorTileWidth";
				case 16 : return "SurfaceDiscretizationDensity";
				case 17 : return "ControlTemperatureTolerance";
				case 18 : return "KinsolRelTol";
				case 19 : return "KinsolAbsTol";
				case 20 : return "KinsolMaxNonlinIter";
				case 21 : return "IntegralWeightsFactor";
			} break;
			// SolverParameter::flag_t
			case 33 :
			switch (t) {
				case 0 : return "DetectMaxTimeStep";
				case 1 : return "KinsolDisableLineSearch";
				case 2 : return "KinsolStrictNewton";
			} break;
			// SolverParameter::integrator_t
			case 34 :
			switch (t) {
				case 0 : return "CVODE";
				case 1 : return "ExplicitEuler";
				case 2 : return "ImplicitEuler";
				case 3 : return "auto";
			} break;
			// SolverParameter::lesSolver_t
			case 35 :
			switch (t) {
				case 0 : return "Dense";
				case 1 : return "KLU";
				case 2 : return "GMRES";
				case 3 : return "BiCGStab";
				case 4 : return "auto";
			} break;
			// SolverParameter::precond_t
			case 36 :
			switch (t) {
				case 0 : return "Band";
				case 1 : return "ILU";
				case 2 : return "auto";
			} break;
			// Zone::type_t
			case 37 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Active";
				case 2 : return "Ground";
			} break;
			// Zone::para_t
			case 38 :
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
		if (no_description != NULL)
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
			// EmbeddedObjectWindow::para_t
			case 4 :
			switch (t) {
				case 0 : return "Fraction of glass area versus total area [1 - no frame, 0 - all frame].";
				case 1 : return "Constant transmissibility [0 - opaque, 1 - fully translucent].";
				case 2 : return "Effective heat transfer coefficient (area-weighted U-value of frame and glass).";
				case 3 : return "Shading factor (between 0...1).";
				case 4 : return "Leakage coefficient of joints [m3/sPa^n].";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "Constant model.";
				case 1 : return "Model with detailed layers for calculation of long wave radiation, short wave radiation and gas convection transport.";
				case 2 : return "Model with detailed layers and thermal storage of glass layers.";
			} break;
			// Interface::location_t
			case 6 :
			switch (t) {
				case 0 : return "Interface is situated at left side labeled A.";
				case 1 : return "Interface is situated at right side labeled B.";
			} break;
			// Interface::condition_t
			case 7 :
			switch (t) {
				case 0 : return "Heat conduction boundary condition.";
				case 1 : return "Short-wave solar absorption boundary condition.";
				case 2 : return "Long wave emission (and counter radiation) boundary condition.";
				case 3 : return "Vapor diffusion boundary condition.";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 8 :
			switch (t) {
				case 0 : return "Pressure coeffient.";
			} break;
			// InterfaceAirFlow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "Use results from external wind flow calculation.";
			} break;
			// InterfaceHeatConduction::para_t
			case 10 :
			switch (t) {
				case 0 : return "Constant heat transfer coefficient.";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 12 :
			switch (t) {
				case 0 : return "Constant Long wave emissivity.";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 14 :
			switch (t) {
				case 0 : return "Constant Absorption coefficient [0,...,1].";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 15 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 16 :
			switch (t) {
				case 0 : return "Vapor Transfer Coefficient.";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 17 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// Interval::para_t
			case 18 :
			switch (t) {
				case 0 : return "Start time point.";
				case 1 : return "End time point.";
				case 2 : return "StepSize.";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 19 :
			switch (t) {
				case 0 : if (no_description != NULL) *no_description = true; return "constant";
				case 1 : if (no_description != NULL) *no_description = true; return "linear";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 20 :
			switch (t) {
				case 0 : return "Continuous data";
				case 1 : return "Annual cycle";
			} break;
			// Location::para_t
			case 21 :
			switch (t) {
				case 0 : return "Latitude.";
				case 1 : return "Longitude.";
				case 2 : return "Albedo value [0..100].";
				case 3 : return "Altitude of building as height above NN [m].";
			} break;
			// Material::para_t
			case 22 :
			switch (t) {
				case 0 : return "Dry density of the material.";
				case 1 : return "Specific heat capacity of the material.";
				case 2 : return "Thermal conductivity of the dry material.";
			} break;
			// ModelInputReference::referenceType_t
			case 23 :
			switch (t) {
				case 0 : return "Model references of climate/location models.";
				case 1 : return "Model references inside a room.";
				case 2 : return "Model references a wall.";
				case 3 : return "Model references a wall surface.";
				case 4 : return "Model references an embedded object.";
				case 5 : return "Model references an active object.";
				case 6 : return "Model references a sensor object.";
				case 7 : return "Model references scheduled data.";
				case 8 : return "Model references an object list-specific value.";
				case 9 : return "Model references another generic model.";
				case 10 : return "Model references to global physical quantities.";
			} break;
			// OutputDefinition::timeType_t
			case 24 :
			switch (t) {
				case 0 : return "Write values as calculated at output times";
				case 1 : return "Average values in time (mean value in output step)";
				case 2 : return "Integrate values in time";
			} break;
			// Schedule::type_t
			case 25 :
			switch (t) {
				case 0 : return "All days (Weekend days and Weekdays).";
				case 1 : return "Weekday schedule.";
				case 2 : return "Weekend schedule.";
				case 3 : return "Holiday schedule.";
				case 4 : return "Special Weekday schedule: Monday.";
				case 5 : return "Special Weekday schedule: Tuesday.";
				case 6 : return "Special Weekday schedule: Wednesday.";
				case 7 : return "Special Weekday schedule: Thursday.";
				case 8 : return "Special Weekday schedule: Friday.";
				case 9 : return "Special Weekday schedule: Saturday.";
				case 10 : return "Special Weekday schedule: Sunday.";
			} break;
			// Schedules::day_t
			case 26 :
			switch (t) {
				case 0 : return "Monday.";
				case 1 : return "Tuesday.";
				case 2 : return "Wednesday.";
				case 3 : return "Thursday.";
				case 4 : return "Friday.";
				case 5 : return "Saturday.";
				case 6 : return "Sunday.";
			} break;
			// SerializationTest::test_t
			case 27 :
			switch (t) {
				case 0 : if (no_description != NULL) *no_description = true; return "X1";
				case 1 : if (no_description != NULL) *no_description = true; return "X2";
			} break;
			// SerializationTest::intPara_t
			case 28 :
			switch (t) {
				case 0 : if (no_description != NULL) *no_description = true; return "I1";
				case 1 : if (no_description != NULL) *no_description = true; return "I2";
			} break;
			// SimulationParameter::para_t
			case 29 :
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
				case 11 : return "Ambient temparture for a design day. Parameter that is needed for FMU export.";
			} break;
			// SimulationParameter::intpara_t
			case 30 :
			switch (t) {
				case 0 : return "Start year of the simulation.";
			} break;
			// SimulationParameter::flag_t
			case 31 :
			switch (t) {
				case 0 : return "Flag activating moisture balance calculation if enabled.";
				case 1 : return "Flag activating CO2 balance calculation if enabled.";
				case 2 : return "Flag activating ventilation through joints and openings.";
				case 3 : return "Flag activating FMU export of climate data.";
			} break;
			// SolverParameter::para_t
			case 32 :
			switch (t) {
				case 0 : return "Relative tolerance for solver error check.";
				case 1 : return "Absolute tolerance for solver error check.";
				case 2 : return "Maximum permitted time step for integration.";
				case 3 : return "Minimum accepted time step, before solver aborts with error.";
				case 4 : return "Initial time step";
				case 5 : return "Coefficient reducing nonlinear equation solver convergence limit.";
				case 6 : return "Coefficient reducing iterative equation solver convergence limit.";
				case 7 : return "Maximum order allowed for multi-step solver.";
				case 8 : return "Maximum dimension of Krylov subspace.";
				case 9 : return "Maximum number of nonlinear iterations.";
				case 10 : return "Maximum band width to be used for band LES solver.";
				case 11 : return "Maximum band width to be used for banded preconditioner.";
				case 12 : return "Maximum level of fill-in to be used for ILU preconditioner.";
				case 13 : return "Minimum element width for wall discretization.";
				case 14 : return "Level of detail for wall discretization.";
				case 15 : return "Maximum dimension of a tile for calculation of view factors.";
				case 16 : return "Number of surface discretization elements of a wall in each direction.";
				case 17 : return "Temperature tolerance for ideal heating or cooling.";
				case 18 : return "Relative tolerance for Kinsol solver.";
				case 19 : return "Absolute tolerance for Kinsol solver.";
				case 20 : return "Maximum nonlinear iterations for Kinsol solver.";
				case 21 : return "Optional weighting factor for integral outputs.";
			} break;
			// SolverParameter::flag_t
			case 33 :
			switch (t) {
				case 0 : return "Check schedules to determine minimum distances between steps and adjust MaxTimeStep.";
				case 1 : return "Disable line search for steady state cycles.";
				case 2 : return "Enable strict Newton for steady state cycles.";
			} break;
			// SolverParameter::integrator_t
			case 34 :
			switch (t) {
				case 0 : return "CVODE based solver";
				case 1 : return "Explicit Euler solver";
				case 2 : return "Implicit Euler solver";
				case 3 : return "System selects integrator automatically.";
			} break;
			// SolverParameter::lesSolver_t
			case 35 :
			switch (t) {
				case 0 : return "Dense solver";
				case 1 : return "KLU sparse solver";
				case 2 : return "GMRES iterative solver";
				case 3 : return "BICGSTAB iterative solver";
				case 4 : return "System selects les solver automatically.";
			} break;
			// SolverParameter::precond_t
			case 36 :
			switch (t) {
				case 0 : return "Band preconditioner";
				case 1 : return "Incomplete LU preconditioner";
				case 2 : return "System selects preconditioner automatically.";
			} break;
			// Zone::type_t
			case 37 :
			switch (t) {
				case 0 : return "Zone with constant/predefined temperatures. (schedule)";
				case 1 : return "Zone described by a temperature node in space.";
				case 2 : return "Ground zone (calculates temperature based on standard ToDo Katja).";
			} break;
			// Zone::para_t
			case 38 :
			switch (t) {
				case 0 : return "Temperature of the zone if set constant, or initial temperature for active zones [C].";
				case 1 : return "Relative humidity of the zone if set constant, or initial humidity for active zones [%].";
				case 2 : return "CO2 concentration of the zone if set constant, or initial concentration for active zones [g/m3].";
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
			// EmbeddedObjectWindow::para_t
			case 4 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "W/m2K";
				case 3 : return "---";
				case 4 : return "---";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Interface::location_t
			case 6 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Interface::condition_t
			case 7 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 8 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceAirFlow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceHeatConduction::para_t
			case 10 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 12 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 14 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 15 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 16 :
			switch (t) {
				case 0 : return "s/m";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 17 :
			switch (t) {
				case 0 : return "";
			} break;
			// Interval::para_t
			case 18 :
			switch (t) {
				case 0 : return "d";
				case 1 : return "d";
				case 2 : return "h";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 19 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 20 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Location::para_t
			case 21 :
			switch (t) {
				case 0 : return "Deg";
				case 1 : return "Deg";
				case 2 : return "%";
				case 3 : return "m";
			} break;
			// Material::para_t
			case 22 :
			switch (t) {
				case 0 : return "kg/m3";
				case 1 : return "J/kgK";
				case 2 : return "W/mK";
			} break;
			// ModelInputReference::referenceType_t
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
			// OutputDefinition::timeType_t
			case 24 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Schedule::type_t
			case 25 :
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
			case 26 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
				case 6 : return "";
			} break;
			// SerializationTest::test_t
			case 27 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// SerializationTest::intPara_t
			case 28 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// SimulationParameter::para_t
			case 29 :
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
			// SimulationParameter::intpara_t
			case 30 :
			switch (t) {
				case 0 : return "";
			} break;
			// SimulationParameter::flag_t
			case 31 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SolverParameter::para_t
			case 32 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "min";
				case 3 : return "s";
				case 4 : return "s";
				case 5 : return "---";
				case 6 : return "---";
				case 7 : return "---";
				case 8 : return "---";
				case 9 : return "---";
				case 10 : return "---";
				case 11 : return "---";
				case 12 : return "---";
				case 13 : return "m";
				case 14 : return "---";
				case 15 : return "m";
				case 16 : return "---";
				case 17 : return "K";
				case 18 : return "---";
				case 19 : return "---";
				case 20 : return "---";
				case 21 : return "---";
			} break;
			// SolverParameter::flag_t
			case 33 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// SolverParameter::integrator_t
			case 34 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SolverParameter::lesSolver_t
			case 35 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
			} break;
			// SolverParameter::precond_t
			case 36 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Zone::type_t
			case 37 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Zone::para_t
			case 38 :
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
			// EmbeddedObjectWindow::para_t
			case 4 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Interface::location_t
			case 6 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Interface::condition_t
			case 7 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 8 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceAirFlow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::para_t
			case 10 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 11 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 12 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 13 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 14 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 15 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 16 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 17 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// Interval::para_t
			case 18 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 19 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 20 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Location::para_t
			case 21 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Material::para_t
			case 22 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// ModelInputReference::referenceType_t
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
			// OutputDefinition::timeType_t
			case 24 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Schedule::type_t
			case 25 :
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
			case 26 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
			} break;
			// SerializationTest::test_t
			case 27 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SerializationTest::intPara_t
			case 28 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SimulationParameter::para_t
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
				case 8 : return "#FFFFFF";
				case 9 : return "#FFFFFF";
				case 10 : return "#FFFFFF";
				case 11 : return "#FFFFFF";
			} break;
			// SimulationParameter::intpara_t
			case 30 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// SimulationParameter::flag_t
			case 31 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolverParameter::para_t
			case 32 :
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
			} break;
			// SolverParameter::flag_t
			case 33 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// SolverParameter::integrator_t
			case 34 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolverParameter::lesSolver_t
			case 35 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// SolverParameter::precond_t
			case 36 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Zone::type_t
			case 37 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Zone::para_t
			case 38 :
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
			// EmbeddedObjectWindow::para_t
			case 4 :
			switch (t) {
				case 0 : return 1;
				case 1 : return 0.8;
				case 2 : return 0.8;
				case 3 : return 1;
				case 4 : return 1;
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 5 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Interface::location_t
			case 6 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Interface::condition_t
			case 7 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceAirFlow::splinePara_t
			case 8 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceAirFlow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::para_t
			case 10 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::modelType_t
			case 11 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceLongWaveEmission::para_t
			case 12 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 13 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceSolarAbsorption::para_t
			case 14 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 15 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceVaporDiffusion::para_t
			case 16 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 17 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Interval::para_t
			case 18 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 19 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// LinearSplineParameter::wrapMethod_t
			case 20 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Location::para_t
			case 21 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Material::para_t
			case 22 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ModelInputReference::referenceType_t
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
			// OutputDefinition::timeType_t
			case 24 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Schedule::type_t
			case 25 :
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
			case 26 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SerializationTest::test_t
			case 27 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SerializationTest::intPara_t
			case 28 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::para_t
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
				case 8 : return std::numeric_limits<double>::quiet_NaN();
				case 9 : return std::numeric_limits<double>::quiet_NaN();
				case 10 : return std::numeric_limits<double>::quiet_NaN();
				case 11 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::intpara_t
			case 30 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::flag_t
			case 31 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::para_t
			case 32 :
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
			} break;
			// SolverParameter::flag_t
			case 33 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::integrator_t
			case 34 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::lesSolver_t
			case 35 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::precond_t
			case 36 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::type_t
			case 37 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::para_t
			case 38 :
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
			// EmbeddedObjectWindow::para_t
			case 4 : return 5;
			// EmbeddedObjectWindow::modelType_t
			case 5 : return 3;
			// Interface::location_t
			case 6 : return 2;
			// Interface::condition_t
			case 7 : return 4;
			// InterfaceAirFlow::splinePara_t
			case 8 : return 1;
			// InterfaceAirFlow::modelType_t
			case 9 : return 1;
			// InterfaceHeatConduction::para_t
			case 10 : return 1;
			// InterfaceHeatConduction::modelType_t
			case 11 : return 1;
			// InterfaceLongWaveEmission::para_t
			case 12 : return 1;
			// InterfaceLongWaveEmission::modelType_t
			case 13 : return 1;
			// InterfaceSolarAbsorption::para_t
			case 14 : return 1;
			// InterfaceSolarAbsorption::modelType_t
			case 15 : return 1;
			// InterfaceVaporDiffusion::para_t
			case 16 : return 1;
			// InterfaceVaporDiffusion::modelType_t
			case 17 : return 1;
			// Interval::para_t
			case 18 : return 3;
			// LinearSplineParameter::interpolationMethod_t
			case 19 : return 2;
			// LinearSplineParameter::wrapMethod_t
			case 20 : return 2;
			// Location::para_t
			case 21 : return 4;
			// Material::para_t
			case 22 : return 3;
			// ModelInputReference::referenceType_t
			case 23 : return 11;
			// OutputDefinition::timeType_t
			case 24 : return 3;
			// Schedule::type_t
			case 25 : return 11;
			// Schedules::day_t
			case 26 : return 7;
			// SerializationTest::test_t
			case 27 : return 2;
			// SerializationTest::intPara_t
			case 28 : return 2;
			// SimulationParameter::para_t
			case 29 : return 12;
			// SimulationParameter::intpara_t
			case 30 : return 1;
			// SimulationParameter::flag_t
			case 31 : return 4;
			// SolverParameter::para_t
			case 32 : return 22;
			// SolverParameter::flag_t
			case 33 : return 3;
			// SolverParameter::integrator_t
			case 34 : return 4;
			// SolverParameter::lesSolver_t
			case 35 : return 5;
			// SolverParameter::precond_t
			case 36 : return 3;
			// Zone::type_t
			case 37 : return 3;
			// Zone::para_t
			case 38 : return 6;
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
			// EmbeddedObjectWindow::para_t
			case 4 : return 4;
			// EmbeddedObjectWindow::modelType_t
			case 5 : return 2;
			// Interface::location_t
			case 6 : return 1;
			// Interface::condition_t
			case 7 : return 3;
			// InterfaceAirFlow::splinePara_t
			case 8 : return 0;
			// InterfaceAirFlow::modelType_t
			case 9 : return 0;
			// InterfaceHeatConduction::para_t
			case 10 : return 0;
			// InterfaceHeatConduction::modelType_t
			case 11 : return 0;
			// InterfaceLongWaveEmission::para_t
			case 12 : return 0;
			// InterfaceLongWaveEmission::modelType_t
			case 13 : return 0;
			// InterfaceSolarAbsorption::para_t
			case 14 : return 0;
			// InterfaceSolarAbsorption::modelType_t
			case 15 : return 0;
			// InterfaceVaporDiffusion::para_t
			case 16 : return 0;
			// InterfaceVaporDiffusion::modelType_t
			case 17 : return 0;
			// Interval::para_t
			case 18 : return 2;
			// LinearSplineParameter::interpolationMethod_t
			case 19 : return 1;
			// LinearSplineParameter::wrapMethod_t
			case 20 : return 1;
			// Location::para_t
			case 21 : return 3;
			// Material::para_t
			case 22 : return 2;
			// ModelInputReference::referenceType_t
			case 23 : return 10;
			// OutputDefinition::timeType_t
			case 24 : return 2;
			// Schedule::type_t
			case 25 : return 10;
			// Schedules::day_t
			case 26 : return 6;
			// SerializationTest::test_t
			case 27 : return 1;
			// SerializationTest::intPara_t
			case 28 : return 1;
			// SimulationParameter::para_t
			case 29 : return 11;
			// SimulationParameter::intpara_t
			case 30 : return 0;
			// SimulationParameter::flag_t
			case 31 : return 3;
			// SolverParameter::para_t
			case 32 : return 21;
			// SolverParameter::flag_t
			case 33 : return 2;
			// SolverParameter::integrator_t
			case 34 : return 3;
			// SolverParameter::lesSolver_t
			case 35 : return 4;
			// SolverParameter::precond_t
			case 36 : return 2;
			// Zone::type_t
			case 37 : return 2;
			// Zone::para_t
			case 38 : return 6;
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
