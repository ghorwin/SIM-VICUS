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
	const char * const ENUM_TYPES[55] = {
		"ConstructionInstance::para_t",
		"Date::LanguageID",
		"EmbeddedObject::para_t",
		"EmbeddedObject::objectType_t",
		"EmbeddedObjectDoor::para_t",
		"EmbeddedObjectDoor::modelType_t",
		"EmbeddedObjectHole::para_t",
		"EmbeddedObjectHole::modelType_t",
		"EmbeddedObjectWindow::para_t",
		"EmbeddedObjectWindow::modelType_t",
		"Geometry::WindowPositionHandling",
		"Geometry::para_t",
		"Geometry::ThreeDObjectType",
		"Geometry::geometricOutputGrid_t",
		"ImplicitModelFeedback::operation_t",
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
		"Location::para_t",
		"ModelGroup::evaluationType_t",
		"ModelInputReference::referenceType_t",
		"OutputDefinition::fileType_t",
		"OutputDefinition::timeType_t",
		"ParametrizationDefaults::mode_t",
		"ParametrizationDefaults::para_t",
		"ProjectInfo::para_t",
		"ProjectInfo::flag_t",
		"Schedule::type_t",
		"Schedules::day_t",
		"SimulationParameter::para_t",
		"SimulationParameter::intpara_t",
		"SimulationParameter::stringPara_t",
		"SimulationParameter::flag_t",
		"SolverParameter::para_t",
		"SolverParameter::flag_t",
		"SolverParameter::simpleTypesDesc",
		"SolverParameter::integrator_t",
		"SolverParameter::lesSolver_t",
		"SolverParameter::precond_t",
		"Zone::zoneType_t",
		"Zone::location_t",
		"Zone::para_t",
		"Zone::intpara_t",
		"ZoneList::performanceIndicatorType_t"
	};

	/*! Converts a category string to respective enumeration value. */
	int enum2index(const std::string & enumtype) {
		for (int i=0; i<55; ++i) {
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
				case 0 : return "InitialTemperature";
				case 1 : return "InitialRelativeHumidity";
				case 2 : return "Orientation";
				case 3 : return "Inclination";
				case 4 : return "Area";
			} break;
			// Date::LanguageID
			case 1 :
			switch (t) {
				case 0 : return "en";
				case 1 : return "de";
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
			// EmbeddedObjectDoor::para_t
			case 4 :
			switch (t) {
				case 0 : return "LeakageCoefficient";
			} break;
			// EmbeddedObjectDoor::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// EmbeddedObjectHole::para_t
			case 6 :
			switch (t) {
				case 0 : return "LeakageCoefficient";
			} break;
			// EmbeddedObjectHole::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// EmbeddedObjectWindow::para_t
			case 8 :
			switch (t) {
				case 0 : return "GlassFraction";
				case 1 : return "SolarHeatGainCoefficient";
				case 2 : return "ThermalTransmittance";
				case 3 : return "ShadingFactor";
				case 4 : return "LeakageCoefficient";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Detailed";
				case 2 : return "DetailedWithStorage";
			} break;
			// Geometry::WindowPositionHandling
			case 10 :
			switch (t) {
				case 0 : return "Offset";
				case 1 : return "Ratio";
				case 2 : return "Area";
			} break;
			// Geometry::para_t
			case 11 :
			switch (t) {
				case 0 : return "XOffset";
				case 1 : return "YOffset";
				case 2 : return "ZOffset";
				case 3 : return "ZRotation";
			} break;
			// Geometry::ThreeDObjectType
			case 12 :
			switch (t) {
				case 4 : return "triangle";
				case 5 : return "quadrangle";
			} break;
			// Geometry::geometricOutputGrid_t
			case 13 :
			switch (t) {
				case 0 : return "Height";
				case 1 : return "XStepSize";
				case 2 : return "YStepSize";
			} break;
			// ImplicitModelFeedback::operation_t
			case 14 :
			switch (t) {
				case 0 : return "Add";
				case 1 : return "Overwrite";
			} break;
			// Interface::location_t
			case 15 :
			switch (t) {
				case 0 : return "A";
				case 1 : return "B";
			} break;
			// Interface::condition_t
			case 16 :
			switch (t) {
				case 0 : return "HeatConduction";
				case 1 : return "SolarAbsorption";
				case 2 : return "LongWaveEmission";
				case 3 : return "VaporDiffusion";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 17 :
			switch (t) {
				case 0 : return "PressureCoefficient";
			} break;
			// InterfaceAirFlow::modelType_t
			case 18 :
			switch (t) {
				case 0 : return "WindFlow";
			} break;
			// InterfaceHeatConduction::para_t
			case 19 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 21 :
			switch (t) {
				case 0 : return "Emissivity";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 22 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 23 :
			switch (t) {
				case 0 : return "AbsorptionCoefficient";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 25 :
			switch (t) {
				case 0 : return "VaporTransferCoefficient";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 26 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// Interval::para_t
			case 27 :
			switch (t) {
				case 0 : return "Start";
				case 1 : return "End";
				case 2 : return "StepSize";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 28 :
			switch (t) {
				case 0 : return "constant";
				case 1 : return "linear";
			} break;
			// Location::para_t
			case 29 :
			switch (t) {
				case 0 : return "Latitude";
				case 1 : return "Longitude";
				case 2 : return "Orientation";
				case 3 : return "Albedo";
				case 4 : return "Altitude";
			} break;
			// ModelGroup::evaluationType_t
			case 30 :
			switch (t) {
				case 0 : return "Sequential";
				case 1 : return "Cyclic";
				case 2 : return "Coupled";
			} break;
			// ModelInputReference::referenceType_t
			case 31 :
			switch (t) {
				case 0 : return "Location";
				case 1 : return "Zone";
				case 2 : return "ConstructionInstance";
				case 3 : return "Interface";
				case 4 : return "EmbeddedObject";
				case 5 : return "ActiveObject";
				case 6 : return "Sensor";
				case 7 : return "Schedule";
				case 8 : return "Model";
				case 9 : return "Global";
			} break;
			// OutputDefinition::fileType_t
			case 32 :
			switch (t) {
				case 0 : return "DataIO";
				case 1 : return "Report";
			} break;
			// OutputDefinition::timeType_t
			case 33 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// ParametrizationDefaults::mode_t
			case 34 :
			switch (t) {
				case 0 : return "Strict";
				case 1 : return "Lazy";
			} break;
			// ParametrizationDefaults::para_t
			case 35 :
			switch (t) {
				case 0 : return "InitialTemperature";
				case 1 : return "InitialRelativeHumidity";
				case 2 : return "HeatCapacity";
				case 3 : return "HeatTransferCoefficient";
				case 4 : return "Ambient.HeatTransferCoefficient";
				case 5 : return "AbsorptionCoefficient";
				case 6 : return "Ambient.AbsorptionCoefficient";
				case 7 : return "Emissivity";
				case 8 : return "VaporTransferCoefficient";
				case 9 : return "Ambient.VaporTransferCoefficient";
			} break;
			// ProjectInfo::para_t
			case 36 :
			switch (t) {
				case 0 : return "GridStep";
				case 1 : return "CatchStep";
			} break;
			// ProjectInfo::flag_t
			case 37 :
			switch (t) {
				case 0 : return "GridOn";
				case 1 : return "CatchOn";
			} break;
			// Schedule::type_t
			case 38 :
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
			case 39 :
			switch (t) {
				case 0 : return "Mon";
				case 1 : return "Tue";
				case 2 : return "Wed";
				case 3 : return "Thu";
				case 4 : return "Fri";
				case 5 : return "Sat";
				case 6 : return "Sun";
			} break;
			// SimulationParameter::para_t
			case 40 :
			switch (t) {
				case 0 : return "RadiationLoadFraction";
				case 1 : return "UserThermalRadiationFraction";
				case 2 : return "EquipmentThermalLossFraction";
				case 3 : return "EquipmentThermalRadiationFraction";
				case 4 : return "LightingVisibleRadiationFraction";
				case 5 : return "LightingThermalRadiationFraction";
				case 6 : return "DomesticWaterSensitiveHeatGainFraction";
				case 7 : return "AirExchangeRateN50";
				case 8 : return "ShieldingCoefficient";
				case 9 : return "HeatingDesignAmbientTemperature";
			} break;
			// SimulationParameter::intpara_t
			case 41 :
			switch (t) {
				case 0 : return "StartYear";
			} break;
			// SimulationParameter::stringPara_t
			case 42 :
			switch (t) {
				case 0 : return "CoolingDesignClimateDataFile";
				case 1 : return "WallMoistureBalanceCalculationMode";
			} break;
			// SimulationParameter::flag_t
			case 43 :
			switch (t) {
				case 0 : return "EnableMoistureBalance";
				case 1 : return "EnableCO2Balance";
				case 2 : return "EnableJointVentilation";
				case 3 : return "ExportClimateDataFMU";
			} break;
			// SolverParameter::para_t
			case 44 :
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
			case 45 :
			switch (t) {
				case 0 : return "DetectMaxTimeStep";
				case 1 : return "KinsolDisableLineSearch";
				case 2 : return "KinsolStrictNewton";
			} break;
			// SolverParameter::simpleTypesDesc
			case 46 :
			switch (t) {
				case 0 : return "Integrator";
				case 1 : return "LESSolver";
				case 2 : return "Preconditioner";
			} break;
			// SolverParameter::integrator_t
			case 47 :
			switch (t) {
				case 0 : return "CVODE";
				case 1 : return "ExplicitEuler";
				case 2 : return "ImplicitEuler";
				case 3 : return "auto";
			} break;
			// SolverParameter::lesSolver_t
			case 48 :
			switch (t) {
				case 0 : return "BTridiag";
				case 1 : return "Band";
				case 2 : return "Dense";
				case 3 : return "KLU";
				case 4 : return "GMRES";
				case 5 : return "BiCGStab";
				case 6 : return "TFQMR";
				case 7 : return "auto";
			} break;
			// SolverParameter::precond_t
			case 49 :
			switch (t) {
				case 0 : return "BTridiag";
				case 1 : return "Band";
				case 2 : return "ILU";
				case 3 : return "auto";
			} break;
			// Zone::zoneType_t
			case 50 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Active";
				case 2 : return "Detailed";
				case 3 : return "Ground";
			} break;
			// Zone::location_t
			case 51 :
			switch (t) {
				case 0 : return "Inside";
				case 1 : return "Ground";
				case 2 : return "Outside";
			} break;
			// Zone::para_t
			case 52 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "CO2Concentration";
				case 3 : return "InitialTemperature";
				case 4 : return "InitialRelativeHumidity";
				case 5 : return "InitialCO2Concentration";
				case 6 : return "Area";
				case 7 : return "Height";
				case 8 : return "Volume";
				case 9 : return "HeatCapacity";
			} break;
			// Zone::intpara_t
			case 53 :
			switch (t) {
				case 0 : return "AirMaterialReference";
			} break;
			// ZoneList::performanceIndicatorType_t
			case 54 :
			switch (t) {
				case 0 : return "Absolute";
				case 1 : return "HeatingAreaWeighted";
				case 2 : return "AreaWeighted";
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
				case 0 : return "InitialTemperature";
				case 1 : return "InitialRelativeHumidity";
				case 2 : return "Orientation";
				case 3 : return "Inclination";
				case 4 : return "Area";
			} break;
			// Date::LanguageID
			case 1 :
			switch (t) {
				case 0 : return "en";
				case 1 : return "de";
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
			// EmbeddedObjectDoor::para_t
			case 4 :
			switch (t) {
				case 0 : return "LeakageCoefficient";
			} break;
			// EmbeddedObjectDoor::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// EmbeddedObjectHole::para_t
			case 6 :
			switch (t) {
				case 0 : return "LeakageCoefficient";
			} break;
			// EmbeddedObjectHole::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// EmbeddedObjectWindow::para_t
			case 8 :
			switch (t) {
				case 0 : return "GlassFraction";
				case 1 : return "SolarHeatGainCoefficient";
				case 2 : return "ThermalTransmittance";
				case 3 : return "ShadingFactor";
				case 4 : return "LeakageCoefficient";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Detailed";
				case 2 : return "DetailedWithStorage";
			} break;
			// Geometry::WindowPositionHandling
			case 10 :
			switch (t) {
				case 0 : return "Offset";
				case 1 : return "Ratio";
				case 2 : return "Area";
			} break;
			// Geometry::para_t
			case 11 :
			switch (t) {
				case 0 : return "XOffset";
				case 1 : return "YOffset";
				case 2 : return "ZOffset";
				case 3 : return "ZRotation";
			} break;
			// Geometry::ThreeDObjectType
			case 12 :
			switch (t) {
				case 4 : return "triangle";
				case 5 : return "quadrangle";
			} break;
			// Geometry::geometricOutputGrid_t
			case 13 :
			switch (t) {
				case 0 : return "Height";
				case 1 : return "XStepSize";
				case 2 : return "YStepSize";
			} break;
			// ImplicitModelFeedback::operation_t
			case 14 :
			switch (t) {
				case 0 : return "Add";
				case 1 : return "Overwrite";
			} break;
			// Interface::location_t
			case 15 :
			switch (t) {
				case 0 : return "A";
				case 1 : return "B";
			} break;
			// Interface::condition_t
			case 16 :
			switch (t) {
				case 0 : return "HeatConduction";
				case 1 : return "SolarAbsorption";
				case 2 : return "LongWaveEmission";
				case 3 : return "VaporDiffusion";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 17 :
			switch (t) {
				case 0 : return "PressureCoefficient";
			} break;
			// InterfaceAirFlow::modelType_t
			case 18 :
			switch (t) {
				case 0 : return "WindFlow";
			} break;
			// InterfaceHeatConduction::para_t
			case 19 :
			switch (t) {
				case 0 : return "HeatTransferCoefficient";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 21 :
			switch (t) {
				case 0 : return "Emissivity";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 22 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 23 :
			switch (t) {
				case 0 : return "AbsorptionCoefficient";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 25 :
			switch (t) {
				case 0 : return "VaporTransferCoefficient";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 26 :
			switch (t) {
				case 0 : return "Constant";
			} break;
			// Interval::para_t
			case 27 :
			switch (t) {
				case 0 : return "Start";
				case 1 : return "End";
				case 2 : return "StepSize";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 28 :
			switch (t) {
				case 0 : return "constant";
				case 1 : return "linear";
			} break;
			// Location::para_t
			case 29 :
			switch (t) {
				case 0 : return "Latitude";
				case 1 : return "Longitude";
				case 2 : return "Orientation";
				case 3 : return "Albedo";
				case 4 : return "Altitude";
			} break;
			// ModelGroup::evaluationType_t
			case 30 :
			switch (t) {
				case 0 : return "Sequential";
				case 1 : return "Cyclic";
				case 2 : return "Coupled";
			} break;
			// ModelInputReference::referenceType_t
			case 31 :
			switch (t) {
				case 0 : return "Location";
				case 1 : return "Zone";
				case 2 : return "ConstructionInstance";
				case 3 : return "Interface";
				case 4 : return "EmbeddedObject";
				case 5 : return "ActiveObject";
				case 6 : return "Sensor";
				case 7 : return "Schedule";
				case 8 : return "Model";
				case 9 : return "Global";
			} break;
			// OutputDefinition::fileType_t
			case 32 :
			switch (t) {
				case 0 : return "DataIO";
				case 1 : return "Report";
			} break;
			// OutputDefinition::timeType_t
			case 33 :
			switch (t) {
				case 0 : return "None";
				case 1 : return "Mean";
				case 2 : return "Integral";
			} break;
			// ParametrizationDefaults::mode_t
			case 34 :
			switch (t) {
				case 0 : return "Strict";
				case 1 : return "Lazy";
			} break;
			// ParametrizationDefaults::para_t
			case 35 :
			switch (t) {
				case 0 : return "InitialTemperature";
				case 1 : return "InitialRelativeHumidity";
				case 2 : return "HeatCapacity";
				case 3 : return "HeatTransferCoefficient";
				case 4 : return "Ambient.HeatTransferCoefficient";
				case 5 : return "AbsorptionCoefficient";
				case 6 : return "Ambient.AbsorptionCoefficient";
				case 7 : return "Emissivity";
				case 8 : return "VaporTransferCoefficient";
				case 9 : return "Ambient.VaporTransferCoefficient";
			} break;
			// ProjectInfo::para_t
			case 36 :
			switch (t) {
				case 0 : return "GridStep";
				case 1 : return "CatchStep";
			} break;
			// ProjectInfo::flag_t
			case 37 :
			switch (t) {
				case 0 : return "GridOn";
				case 1 : return "CatchOn";
			} break;
			// Schedule::type_t
			case 38 :
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
			case 39 :
			switch (t) {
				case 0 : return "Mon";
				case 1 : return "Tue";
				case 2 : return "Wed";
				case 3 : return "Thu";
				case 4 : return "Fri";
				case 5 : return "Sat";
				case 6 : return "Sun";
			} break;
			// SimulationParameter::para_t
			case 40 :
			switch (t) {
				case 0 : return "RadiationLoadFraction";
				case 1 : return "UserThermalRadiationFraction";
				case 2 : return "EquipmentThermalLossFraction";
				case 3 : return "EquipmentThermalRadiationFraction";
				case 4 : return "LightingVisibleRadiationFraction";
				case 5 : return "LightingThermalRadiationFraction";
				case 6 : return "DomesticWaterSensitiveHeatGainFraction";
				case 7 : return "AirExchangeRateN50";
				case 8 : return "ShieldingCoefficient";
				case 9 : return "HeatingDesignAmbientTemperature";
			} break;
			// SimulationParameter::intpara_t
			case 41 :
			switch (t) {
				case 0 : return "StartYear";
			} break;
			// SimulationParameter::stringPara_t
			case 42 :
			switch (t) {
				case 0 : return "CoolingDesignClimateDataFile";
				case 1 : return "WallMoistureBalanceCalculationMode";
			} break;
			// SimulationParameter::flag_t
			case 43 :
			switch (t) {
				case 0 : return "EnableMoistureBalance";
				case 1 : return "EnableCO2Balance";
				case 2 : return "EnableJointVentilation";
				case 3 : return "ExportClimateDataFMU";
			} break;
			// SolverParameter::para_t
			case 44 :
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
			case 45 :
			switch (t) {
				case 0 : return "DetectMaxTimeStep";
				case 1 : return "KinsolDisableLineSearch";
				case 2 : return "KinsolStrictNewton";
			} break;
			// SolverParameter::simpleTypesDesc
			case 46 :
			switch (t) {
				case 0 : return "Integrator";
				case 1 : return "LESSolver";
				case 2 : return "Preconditioner";
			} break;
			// SolverParameter::integrator_t
			case 47 :
			switch (t) {
				case 0 : return "CVODE";
				case 1 : return "ExplicitEuler";
				case 2 : return "ImplicitEuler";
				case 3 : return "auto";
			} break;
			// SolverParameter::lesSolver_t
			case 48 :
			switch (t) {
				case 0 : return "BTridiag";
				case 1 : return "Band";
				case 2 : return "Dense";
				case 3 : return "KLU";
				case 4 : return "GMRES";
				case 5 : return "BiCGStab";
				case 6 : return "TFQMR";
				case 7 : return "auto";
			} break;
			// SolverParameter::precond_t
			case 49 :
			switch (t) {
				case 0 : return "BTridiag";
				case 1 : return "Band";
				case 2 : return "ILU";
				case 3 : return "auto";
			} break;
			// Zone::zoneType_t
			case 50 :
			switch (t) {
				case 0 : return "Constant";
				case 1 : return "Active";
				case 2 : return "Detailed";
				case 3 : return "Ground";
			} break;
			// Zone::location_t
			case 51 :
			switch (t) {
				case 0 : return "Inside";
				case 1 : return "Ground";
				case 2 : return "Outside";
			} break;
			// Zone::para_t
			case 52 :
			switch (t) {
				case 0 : return "Temperature";
				case 1 : return "RelativeHumidity";
				case 2 : return "CO2Concentration";
				case 3 : return "InitialTemperature";
				case 4 : return "InitialRelativeHumidity";
				case 5 : return "InitialCO2Concentration";
				case 6 : return "Area";
				case 7 : return "Height";
				case 8 : return "Volume";
				case 9 : return "HeatCapacity";
			} break;
			// Zone::intpara_t
			case 53 :
			switch (t) {
				case 0 : return "AirMaterialReference";
			} break;
			// ZoneList::performanceIndicatorType_t
			case 54 :
			switch (t) {
				case 0 : return "Absolute";
				case 1 : return "HeatingAreaWeighted";
				case 2 : return "AreaWeighted";
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
				case 0 : return "Initial temperature of the wall [C].";
				case 1 : return "Initial relative humidity [%].";
				case 2 : return "Orientation of the wall [deg].";
				case 3 : return "Inclination of the wall [deg].";
				case 4 : return "Gross area of the wall [m2].";
			} break;
			// Date::LanguageID
			case 1 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "en";
				case 1 : if (no_description != nullptr) *no_description = true; return "de";
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
			// EmbeddedObjectDoor::para_t
			case 4 :
			switch (t) {
				case 0 : return "Leakage coefficient of joints (between 0...1).";
			} break;
			// EmbeddedObjectDoor::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// EmbeddedObjectHole::para_t
			case 6 :
			switch (t) {
				case 0 : return "Leakage coefficient of joints (between 0...1).";
			} break;
			// EmbeddedObjectHole::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// EmbeddedObjectWindow::para_t
			case 8 :
			switch (t) {
				case 0 : return "Fraction of glass area versus total area [1 - no frame, 0 - all frame].";
				case 1 : return "Constant transmissibility [0 - opaque, 1 - fully translucent].";
				case 2 : return "Effective heat transfer coefficient (area-weighted U-value of frame and glass).";
				case 3 : return "Shading factor (between 0...1).";
				case 4 : return "Leakage coefficient of joints [m3/sPa^n].";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "Constant model.";
				case 1 : return "Model with detailed layers for calculation of long wave radiation, short wave radiation and gas convection transport.";
				case 2 : return "Model with detailed layers and thermal storage of glass layers.";
			} break;
			// Geometry::WindowPositionHandling
			case 10 :
			switch (t) {
				case 0 : return "Window position with constant distance and offset.";
				case 1 : return "Window position with constant aspect ratio.";
				case 2 : return "Window position with constant area ratio.";
			} break;
			// Geometry::para_t
			case 11 :
			switch (t) {
				case 0 : return "Defines x offset from global 0.0.0.";
				case 1 : return "Defines y offset from global 0.0.0.";
				case 2 : return "Defines z offset from global 0.0.0.";
				case 3 : return "Defines rotation (mathematically negative) around z axis.";
			} break;
			// Geometry::ThreeDObjectType
			case 12 :
			switch (t) {
				case 4 : return "triangle based 3d objects";
				case 5 : return "quadrangle based 3d objects";
			} break;
			// Geometry::geometricOutputGrid_t
			case 13 :
			switch (t) {
				case 0 : return "Output grid height [m].";
				case 1 : return "Output grid distance dx [m].";
				case 2 : return "Output grid distance dy [m].";
			} break;
			// ImplicitModelFeedback::operation_t
			case 14 :
			switch (t) {
				case 0 : return "Adds a model input reference to the adressed implicit model.";
				case 1 : return "Overwrites a model input reference of the adressed implicit model.";
			} break;
			// Interface::location_t
			case 15 :
			switch (t) {
				case 0 : return "Interface is situated at left side labeled A.";
				case 1 : return "Interface is situated at right side labeled B.";
			} break;
			// Interface::condition_t
			case 16 :
			switch (t) {
				case 0 : return "Heat conduction boundary condition.";
				case 1 : return "Short-wave solar absorption boundary condition.";
				case 2 : return "Long wave emission (and counter radiation) boundary condition.";
				case 3 : return "Vapor diffusion boundary condition.";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 17 :
			switch (t) {
				case 0 : return "Pressure coeffient.";
			} break;
			// InterfaceAirFlow::modelType_t
			case 18 :
			switch (t) {
				case 0 : return "Use results from external wind flow calculation.";
			} break;
			// InterfaceHeatConduction::para_t
			case 19 :
			switch (t) {
				case 0 : return "Constant heat transfer coefficient [W/m2K].";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 21 :
			switch (t) {
				case 0 : return "Constant Long wave Emissivity [0,...,1].";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 22 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 23 :
			switch (t) {
				case 0 : return "Constant Absorption coefficient [0,...,1].";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 25 :
			switch (t) {
				case 0 : return "Vapor Transfer Coefficient.";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 26 :
			switch (t) {
				case 0 : return "Constant model.";
			} break;
			// Interval::para_t
			case 27 :
			switch (t) {
				case 0 : return "Start time point.";
				case 1 : return "End time point.";
				case 2 : return "StepSize.";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 28 :
			switch (t) {
				case 0 : if (no_description != nullptr) *no_description = true; return "constant";
				case 1 : if (no_description != nullptr) *no_description = true; return "linear";
			} break;
			// Location::para_t
			case 29 :
			switch (t) {
				case 0 : return "Latitude.";
				case 1 : return "Longitude.";
				case 2 : return "Orientation of the Building: 0 deg = surfaces with 0 deg orientation point to north.";
				case 3 : return "Albedo value [0..100].";
				case 4 : return "Altitude of building as height above NN [m].";
			} break;
			// ModelGroup::evaluationType_t
			case 30 :
			switch (t) {
				case 0 : return "Dependend models are connected by a sequence.";
				case 1 : return "Dependend models are connected one cyclic sequences.";
				case 2 : return "Group of fully coupled models.";
			} break;
			// ModelInputReference::referenceType_t
			case 31 :
			switch (t) {
				case 0 : return "Model references of climate/location models.";
				case 1 : return "Model references inside a room.";
				case 2 : return "Model references a wall.";
				case 3 : return "Model references a wall surface.";
				case 4 : return "Model references an embedded object.";
				case 5 : return "Model references an active object.";
				case 6 : return "Model references a sensor object.";
				case 7 : return "Model references of scheduled data.";
				case 8 : return "Model references another generic model.";
				case 9 : return "Model references to global physical quantities.";
			} break;
			// OutputDefinition::fileType_t
			case 32 :
			switch (t) {
				case 0 : return "Write DataIO output format (*.d6o or *.d6b)";
				case 1 : return "Write report output format (*.csv)";
			} break;
			// OutputDefinition::timeType_t
			case 33 :
			switch (t) {
				case 0 : return "Write values as calculated at output times";
				case 1 : return "Average values in time (mean value in output step)";
				case 2 : return "Integrate values in time";
			} break;
			// ParametrizationDefaults::mode_t
			case 34 :
			switch (t) {
				case 0 : return "Defaults are ignored and ALL properties MUST be given in the respective model/object parameter sections.";
				case 1 : return "Certain properties can be omitted as long as defaults are specified.";
			} break;
			// ParametrizationDefaults::para_t
			case 35 :
			switch (t) {
				case 0 : return "Global initial temperature [C].";
				case 1 : return "Global initial relative humidity [%].";
				case 2 : return "Extra heat capacity of a room due to furniture [J/K].";
				case 3 : return "Global heat transfer coefficient [W/m2K].";
				case 4 : return "Global heat transfer coefficient [W/m2K].";
				case 5 : return "Global Absorption coefficient [0,...,1].";
				case 6 : return "Global ambient Absorption coefficient [0,...,1].";
				case 7 : return "Global Long wave Emissivity [0,...,1].";
				case 8 : return "Global vapor transfer coefficient [s/m].";
				case 9 : return "Global ambient vapor transfer coefficient [s/m].";
			} break;
			// ProjectInfo::para_t
			case 36 :
			switch (t) {
				case 0 : return "Step size of grid (for display).";
				case 1 : return "Step size for catch points (snap points).";
			} break;
			// ProjectInfo::flag_t
			case 37 :
			switch (t) {
				case 0 : return "Grid display enabled/disabled.";
				case 1 : return "Snapping/catch enabled/disabled.";
			} break;
			// Schedule::type_t
			case 38 :
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
			case 39 :
			switch (t) {
				case 0 : return "Monday.";
				case 1 : return "Tuesday.";
				case 2 : return "Wednesday.";
				case 3 : return "Thursday.";
				case 4 : return "Friday.";
				case 5 : return "Saturday.";
				case 6 : return "Sunday.";
			} break;
			// SimulationParameter::para_t
			case 40 :
			switch (t) {
				case 0 : return "Percentage of solar radiation gains attributed direcly to room 0..1.";
				case 1 : return "Percentage of heat that is emitted by long wave radiation from persons.";
				case 2 : return "Percentage of energy from equipment load that is not available as thermal heat.";
				case 3 : return "Percentage of heat that is emitted by long wave radiation from equipment.";
				case 4 : return "Percentage of energy from lighting that is transformed into visible short wave radiation.";
				case 5 : return "Percentage of heat that is emitted by long wave radiation from lighting.";
				case 6 : return "Percentage of sensitive heat from domestic water istributed towrads the room.";
				case 7 : return "Air exchange rate resulting from a pressure difference of 50 Pa between inside and outside.";
				case 8 : return "Shielding coefficient for a given location and envelope type.";
				case 9 : return "Ambient temparture for a design day. Parameter that is needed for FMU export.";
			} break;
			// SimulationParameter::intpara_t
			case 41 :
			switch (t) {
				case 0 : return "Start year of the simulation.";
			} break;
			// SimulationParameter::stringPara_t
			case 42 :
			switch (t) {
				case 0 : return "Climate data file for a cooling design day. Parameter that is needed for FMU export.";
				case 1 : return "String including key for wall mositure calculation method, default 'None";
			} break;
			// SimulationParameter::flag_t
			case 43 :
			switch (t) {
				case 0 : return "Flag activating moisture balance calculation if enabled.";
				case 1 : return "Flag activating CO2 balance calculation if enabled.";
				case 2 : return "Flag activating ventilation through joints and openings.";
				case 3 : return "Flag activating FMU export of climate data.";
			} break;
			// SolverParameter::para_t
			case 44 :
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
			case 45 :
			switch (t) {
				case 0 : return "Check schedules to determine minimum distances between steps and adjust MaxTimeStep.";
				case 1 : return "Disable line search for steady state cycles.";
				case 2 : return "Enable strict Newton for steady state cycles.";
			} break;
			// SolverParameter::simpleTypesDesc
			case 46 :
			switch (t) {
				case 0 : return "A selection of integrator engines.";
				case 1 : return "A selection of linear equation solver engines.";
				case 2 : return "A selection of preconditioners for iterative solvers.";
			} break;
			// SolverParameter::integrator_t
			case 47 :
			switch (t) {
				case 0 : return "CVODE based solver";
				case 1 : return "Explicit Euler solver";
				case 2 : return "Implicit Euler solver";
				case 3 : return "System selects integrator automatically.";
			} break;
			// SolverParameter::lesSolver_t
			case 48 :
			switch (t) {
				case 0 : return "Block-tridiagonal solver";
				case 1 : return "Band solver";
				case 2 : return "Dense solver";
				case 3 : return "KLU sparse solver";
				case 4 : return "GMRES iterative solver";
				case 5 : return "BICGSTAB iterative solver";
				case 6 : return "TFQMR iterative solver";
				case 7 : return "System selects les solver automatically.";
			} break;
			// SolverParameter::precond_t
			case 49 :
			switch (t) {
				case 0 : return "Block-tridiagonal preconditioner";
				case 1 : return "Band preconditioner";
				case 2 : return "Incomplete LU preconditioner";
				case 3 : return "System selects preconditioner automatically.";
			} break;
			// Zone::zoneType_t
			case 50 :
			switch (t) {
				case 0 : return "Zone with constant temperatures.";
				case 1 : return "Zone described by a temperature node in space.";
				case 2 : return "Zone with detailed temperature in time and space.";
				case 3 : return "Ground zone with temperatures from a CCD climate data file.";
			} break;
			// Zone::location_t
			case 51 :
			switch (t) {
				case 0 : return "Zone is inside a building.";
				case 1 : return "Zone represents ground.";
				case 2 : return "Zone respresents ambient climate.";
			} break;
			// Zone::para_t
			case 52 :
			switch (t) {
				case 0 : return "Temperature of the zone if set constant, or initial temperature for active zones [C].";
				case 1 : return "Relative humidity of the zone if set constant, or initial humidity for active zones [%].";
				case 2 : return "CO2 concentration of the zone if set constant, or initial concentration for active zones [g/m3].";
				case 3 : return "Initial temperature for active zones [C].";
				case 4 : return "Initial humidity for active zones [%].";
				case 5 : return "Initial concentration for active zones [g/m3].";
				case 6 : return "Area of the ground floor [m2].";
				case 7 : return "Zone height [m].";
				case 8 : return "Zone volume [m3] (computed, if area and height are given).";
				case 9 : return "Extra heat capacity [J/K].";
			} break;
			// Zone::intpara_t
			case 53 :
			switch (t) {
				case 0 : return "ID reference to a mdoel with air parameter calculations.";
			} break;
			// ZoneList::performanceIndicatorType_t
			case 54 :
			switch (t) {
				case 0 : return "Calculate absolute values.";
				case 1 : return "Relate values to complete heated building floor area.";
				case 2 : return "Relate values to complete building floor area.";
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
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "Deg";
				case 3 : return "Deg";
				case 4 : return "m2";
			} break;
			// Date::LanguageID
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
			// EmbeddedObjectDoor::para_t
			case 4 :
			switch (t) {
				case 0 : return "---";
			} break;
			// EmbeddedObjectDoor::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "";
			} break;
			// EmbeddedObjectHole::para_t
			case 6 :
			switch (t) {
				case 0 : return "---";
			} break;
			// EmbeddedObjectHole::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "";
			} break;
			// EmbeddedObjectWindow::para_t
			case 8 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "W/m2K";
				case 3 : return "---";
				case 4 : return "---";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Geometry::WindowPositionHandling
			case 10 :
			switch (t) {
				case 0 : return "-";
				case 1 : return "-";
				case 2 : return "-";
			} break;
			// Geometry::para_t
			case 11 :
			switch (t) {
				case 0 : return "m";
				case 1 : return "m";
				case 2 : return "m";
				case 3 : return "Deg";
			} break;
			// Geometry::ThreeDObjectType
			case 12 :
			switch (t) {
				case 4 : return "-";
				case 5 : return "-";
			} break;
			// Geometry::geometricOutputGrid_t
			case 13 :
			switch (t) {
				case 0 : return "m";
				case 1 : return "m";
				case 2 : return "m";
			} break;
			// ImplicitModelFeedback::operation_t
			case 14 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Interface::location_t
			case 15 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Interface::condition_t
			case 16 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 17 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceAirFlow::modelType_t
			case 18 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceHeatConduction::para_t
			case 19 :
			switch (t) {
				case 0 : return "W/m2K";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 21 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 22 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 23 :
			switch (t) {
				case 0 : return "---";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 25 :
			switch (t) {
				case 0 : return "s/m";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 26 :
			switch (t) {
				case 0 : return "";
			} break;
			// Interval::para_t
			case 27 :
			switch (t) {
				case 0 : return "d";
				case 1 : return "d";
				case 2 : return "h";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 28 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Location::para_t
			case 29 :
			switch (t) {
				case 0 : return "Deg";
				case 1 : return "Deg";
				case 2 : return "Deg";
				case 3 : return "%";
				case 4 : return "m";
			} break;
			// ModelGroup::evaluationType_t
			case 30 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// ModelInputReference::referenceType_t
			case 31 :
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
			} break;
			// OutputDefinition::fileType_t
			case 32 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// OutputDefinition::timeType_t
			case 33 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// ParametrizationDefaults::mode_t
			case 34 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// ParametrizationDefaults::para_t
			case 35 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "J/K";
				case 3 : return "W/m2K";
				case 4 : return "W/m2K";
				case 5 : return "---";
				case 6 : return "---";
				case 7 : return "---";
				case 8 : return "s/m";
				case 9 : return "s/m";
			} break;
			// ProjectInfo::para_t
			case 36 :
			switch (t) {
				case 0 : return "m";
				case 1 : return "m";
			} break;
			// ProjectInfo::flag_t
			case 37 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// Schedule::type_t
			case 38 :
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
			case 39 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
				case 4 : return "";
				case 5 : return "";
				case 6 : return "";
			} break;
			// SimulationParameter::para_t
			case 40 :
			switch (t) {
				case 0 : return "%";
				case 1 : return "---";
				case 2 : return "---";
				case 3 : return "---";
				case 4 : return "---";
				case 5 : return "---";
				case 6 : return "---";
				case 7 : return "1/h";
				case 8 : return "---";
				case 9 : return "C";
			} break;
			// SimulationParameter::intpara_t
			case 41 :
			switch (t) {
				case 0 : return "";
			} break;
			// SimulationParameter::stringPara_t
			case 42 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
			} break;
			// SimulationParameter::flag_t
			case 43 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SolverParameter::para_t
			case 44 :
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
			case 45 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// SolverParameter::simpleTypesDesc
			case 46 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "---";
			} break;
			// SolverParameter::integrator_t
			case 47 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// SolverParameter::lesSolver_t
			case 48 :
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
			// SolverParameter::precond_t
			case 49 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// Zone::zoneType_t
			case 50 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
				case 3 : return "";
			} break;
			// Zone::location_t
			case 51 :
			switch (t) {
				case 0 : return "";
				case 1 : return "";
				case 2 : return "";
			} break;
			// Zone::para_t
			case 52 :
			switch (t) {
				case 0 : return "C";
				case 1 : return "%";
				case 2 : return "g/m3";
				case 3 : return "C";
				case 4 : return "%";
				case 5 : return "g/m3";
				case 6 : return "m2";
				case 7 : return "m";
				case 8 : return "m3";
				case 9 : return "J/K";
			} break;
			// Zone::intpara_t
			case 53 :
			switch (t) {
				case 0 : return "---";
			} break;
			// ZoneList::performanceIndicatorType_t
			case 54 :
			switch (t) {
				case 0 : return "---";
				case 1 : return "---";
				case 2 : return "---";
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
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// Date::LanguageID
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
			// EmbeddedObjectDoor::para_t
			case 4 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// EmbeddedObjectDoor::modelType_t
			case 5 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// EmbeddedObjectHole::para_t
			case 6 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// EmbeddedObjectHole::modelType_t
			case 7 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// EmbeddedObjectWindow::para_t
			case 8 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Geometry::WindowPositionHandling
			case 10 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Geometry::para_t
			case 11 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Geometry::ThreeDObjectType
			case 12 :
			switch (t) {
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
			} break;
			// Geometry::geometricOutputGrid_t
			case 13 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// ImplicitModelFeedback::operation_t
			case 14 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Interface::location_t
			case 15 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Interface::condition_t
			case 16 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// InterfaceAirFlow::splinePara_t
			case 17 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceAirFlow::modelType_t
			case 18 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::para_t
			case 19 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceHeatConduction::modelType_t
			case 20 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceLongWaveEmission::para_t
			case 21 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 22 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceSolarAbsorption::para_t
			case 23 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 24 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceVaporDiffusion::para_t
			case 25 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 26 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// Interval::para_t
			case 27 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 28 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Location::para_t
			case 29 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
			} break;
			// ModelGroup::evaluationType_t
			case 30 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// ModelInputReference::referenceType_t
			case 31 :
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
			} break;
			// OutputDefinition::fileType_t
			case 32 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// OutputDefinition::timeType_t
			case 33 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// ParametrizationDefaults::mode_t
			case 34 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ParametrizationDefaults::para_t
			case 35 :
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
			} break;
			// ProjectInfo::para_t
			case 36 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// ProjectInfo::flag_t
			case 37 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// Schedule::type_t
			case 38 :
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
			case 39 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
				case 4 : return "#FFFFFF";
				case 5 : return "#FFFFFF";
				case 6 : return "#FFFFFF";
			} break;
			// SimulationParameter::para_t
			case 40 :
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
			} break;
			// SimulationParameter::intpara_t
			case 41 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// SimulationParameter::stringPara_t
			case 42 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
			} break;
			// SimulationParameter::flag_t
			case 43 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolverParameter::para_t
			case 44 :
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
			case 45 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// SolverParameter::simpleTypesDesc
			case 46 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// SolverParameter::integrator_t
			case 47 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// SolverParameter::lesSolver_t
			case 48 :
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
			// SolverParameter::precond_t
			case 49 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Zone::zoneType_t
			case 50 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
				case 3 : return "#FFFFFF";
			} break;
			// Zone::location_t
			case 51 :
			switch (t) {
				case 0 : return "#FFFFFF";
				case 1 : return "#FFFFFF";
				case 2 : return "#FFFFFF";
			} break;
			// Zone::para_t
			case 52 :
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
			} break;
			// Zone::intpara_t
			case 53 :
			switch (t) {
				case 0 : return "#FFFFFF";
			} break;
			// ZoneList::performanceIndicatorType_t
			case 54 :
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
			// ConstructionInstance::para_t
			case 0 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Date::LanguageID
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
			// EmbeddedObjectDoor::para_t
			case 4 :
			switch (t) {
				case 0 : return 1;
			} break;
			// EmbeddedObjectDoor::modelType_t
			case 5 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// EmbeddedObjectHole::para_t
			case 6 :
			switch (t) {
				case 0 : return 1;
			} break;
			// EmbeddedObjectHole::modelType_t
			case 7 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// EmbeddedObjectWindow::para_t
			case 8 :
			switch (t) {
				case 0 : return 1;
				case 1 : return 0.8;
				case 2 : return 0.8;
				case 3 : return 1;
				case 4 : return 1;
			} break;
			// EmbeddedObjectWindow::modelType_t
			case 9 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Geometry::WindowPositionHandling
			case 10 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Geometry::para_t
			case 11 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Geometry::ThreeDObjectType
			case 12 :
			switch (t) {
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Geometry::geometricOutputGrid_t
			case 13 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ImplicitModelFeedback::operation_t
			case 14 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Interface::location_t
			case 15 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Interface::condition_t
			case 16 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceAirFlow::splinePara_t
			case 17 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceAirFlow::modelType_t
			case 18 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::para_t
			case 19 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceHeatConduction::modelType_t
			case 20 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceLongWaveEmission::para_t
			case 21 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceLongWaveEmission::modelType_t
			case 22 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceSolarAbsorption::para_t
			case 23 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceSolarAbsorption::modelType_t
			case 24 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceVaporDiffusion::para_t
			case 25 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// InterfaceVaporDiffusion::modelType_t
			case 26 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Interval::para_t
			case 27 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// LinearSplineParameter::interpolationMethod_t
			case 28 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Location::para_t
			case 29 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ModelGroup::evaluationType_t
			case 30 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ModelInputReference::referenceType_t
			case 31 :
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
			} break;
			// OutputDefinition::fileType_t
			case 32 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// OutputDefinition::timeType_t
			case 33 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ParametrizationDefaults::mode_t
			case 34 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ParametrizationDefaults::para_t
			case 35 :
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
			} break;
			// ProjectInfo::para_t
			case 36 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ProjectInfo::flag_t
			case 37 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Schedule::type_t
			case 38 :
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
			case 39 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
				case 4 : return std::numeric_limits<double>::quiet_NaN();
				case 5 : return std::numeric_limits<double>::quiet_NaN();
				case 6 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::para_t
			case 40 :
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
			} break;
			// SimulationParameter::intpara_t
			case 41 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::stringPara_t
			case 42 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SimulationParameter::flag_t
			case 43 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::para_t
			case 44 :
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
			case 45 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::simpleTypesDesc
			case 46 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::integrator_t
			case 47 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// SolverParameter::lesSolver_t
			case 48 :
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
			// SolverParameter::precond_t
			case 49 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::zoneType_t
			case 50 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
				case 3 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::location_t
			case 51 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
				case 1 : return std::numeric_limits<double>::quiet_NaN();
				case 2 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// Zone::para_t
			case 52 :
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
			} break;
			// Zone::intpara_t
			case 53 :
			switch (t) {
				case 0 : return std::numeric_limits<double>::quiet_NaN();
			} break;
			// ZoneList::performanceIndicatorType_t
			case 54 :
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
			// ConstructionInstance::para_t
			case 0 : return 5;
			// Date::LanguageID
			case 1 : return 2;
			// EmbeddedObject::para_t
			case 2 : return 1;
			// EmbeddedObject::objectType_t
			case 3 : return 3;
			// EmbeddedObjectDoor::para_t
			case 4 : return 1;
			// EmbeddedObjectDoor::modelType_t
			case 5 : return 1;
			// EmbeddedObjectHole::para_t
			case 6 : return 1;
			// EmbeddedObjectHole::modelType_t
			case 7 : return 1;
			// EmbeddedObjectWindow::para_t
			case 8 : return 5;
			// EmbeddedObjectWindow::modelType_t
			case 9 : return 3;
			// Geometry::WindowPositionHandling
			case 10 : return 3;
			// Geometry::para_t
			case 11 : return 4;
			// Geometry::ThreeDObjectType
			case 12 : return 2;
			// Geometry::geometricOutputGrid_t
			case 13 : return 3;
			// ImplicitModelFeedback::operation_t
			case 14 : return 2;
			// Interface::location_t
			case 15 : return 2;
			// Interface::condition_t
			case 16 : return 4;
			// InterfaceAirFlow::splinePara_t
			case 17 : return 1;
			// InterfaceAirFlow::modelType_t
			case 18 : return 1;
			// InterfaceHeatConduction::para_t
			case 19 : return 1;
			// InterfaceHeatConduction::modelType_t
			case 20 : return 1;
			// InterfaceLongWaveEmission::para_t
			case 21 : return 1;
			// InterfaceLongWaveEmission::modelType_t
			case 22 : return 1;
			// InterfaceSolarAbsorption::para_t
			case 23 : return 1;
			// InterfaceSolarAbsorption::modelType_t
			case 24 : return 1;
			// InterfaceVaporDiffusion::para_t
			case 25 : return 1;
			// InterfaceVaporDiffusion::modelType_t
			case 26 : return 1;
			// Interval::para_t
			case 27 : return 3;
			// LinearSplineParameter::interpolationMethod_t
			case 28 : return 2;
			// Location::para_t
			case 29 : return 5;
			// ModelGroup::evaluationType_t
			case 30 : return 3;
			// ModelInputReference::referenceType_t
			case 31 : return 10;
			// OutputDefinition::fileType_t
			case 32 : return 2;
			// OutputDefinition::timeType_t
			case 33 : return 3;
			// ParametrizationDefaults::mode_t
			case 34 : return 2;
			// ParametrizationDefaults::para_t
			case 35 : return 10;
			// ProjectInfo::para_t
			case 36 : return 2;
			// ProjectInfo::flag_t
			case 37 : return 2;
			// Schedule::type_t
			case 38 : return 11;
			// Schedules::day_t
			case 39 : return 7;
			// SimulationParameter::para_t
			case 40 : return 10;
			// SimulationParameter::intpara_t
			case 41 : return 1;
			// SimulationParameter::stringPara_t
			case 42 : return 2;
			// SimulationParameter::flag_t
			case 43 : return 4;
			// SolverParameter::para_t
			case 44 : return 22;
			// SolverParameter::flag_t
			case 45 : return 3;
			// SolverParameter::simpleTypesDesc
			case 46 : return 3;
			// SolverParameter::integrator_t
			case 47 : return 4;
			// SolverParameter::lesSolver_t
			case 48 : return 8;
			// SolverParameter::precond_t
			case 49 : return 4;
			// Zone::zoneType_t
			case 50 : return 4;
			// Zone::location_t
			case 51 : return 3;
			// Zone::para_t
			case 52 : return 10;
			// Zone::intpara_t
			case 53 : return 1;
			// ZoneList::performanceIndicatorType_t
			case 54 : return 3;
		} // switch
		throw IBK::Exception(IBK::FormatString("Invalid enumeration type '%1'.")
			.arg(enumtype), "[KeywordList::Count]");
	}

	// max index for entries sharing a category in a keyword list
	int KeywordList::MaxIndex(const char * const enumtype) {
		switch (enum2index(enumtype)) {
			// ConstructionInstance::para_t
			case 0 : return 4;
			// Date::LanguageID
			case 1 : return 1;
			// EmbeddedObject::para_t
			case 2 : return 0;
			// EmbeddedObject::objectType_t
			case 3 : return 2;
			// EmbeddedObjectDoor::para_t
			case 4 : return 0;
			// EmbeddedObjectDoor::modelType_t
			case 5 : return 0;
			// EmbeddedObjectHole::para_t
			case 6 : return 0;
			// EmbeddedObjectHole::modelType_t
			case 7 : return 0;
			// EmbeddedObjectWindow::para_t
			case 8 : return 4;
			// EmbeddedObjectWindow::modelType_t
			case 9 : return 2;
			// Geometry::WindowPositionHandling
			case 10 : return 2;
			// Geometry::para_t
			case 11 : return 3;
			// Geometry::ThreeDObjectType
			case 12 : return 5;
			// Geometry::geometricOutputGrid_t
			case 13 : return 2;
			// ImplicitModelFeedback::operation_t
			case 14 : return 1;
			// Interface::location_t
			case 15 : return 1;
			// Interface::condition_t
			case 16 : return 3;
			// InterfaceAirFlow::splinePara_t
			case 17 : return 0;
			// InterfaceAirFlow::modelType_t
			case 18 : return 0;
			// InterfaceHeatConduction::para_t
			case 19 : return 0;
			// InterfaceHeatConduction::modelType_t
			case 20 : return 0;
			// InterfaceLongWaveEmission::para_t
			case 21 : return 0;
			// InterfaceLongWaveEmission::modelType_t
			case 22 : return 0;
			// InterfaceSolarAbsorption::para_t
			case 23 : return 0;
			// InterfaceSolarAbsorption::modelType_t
			case 24 : return 0;
			// InterfaceVaporDiffusion::para_t
			case 25 : return 0;
			// InterfaceVaporDiffusion::modelType_t
			case 26 : return 0;
			// Interval::para_t
			case 27 : return 2;
			// LinearSplineParameter::interpolationMethod_t
			case 28 : return 1;
			// Location::para_t
			case 29 : return 4;
			// ModelGroup::evaluationType_t
			case 30 : return 2;
			// ModelInputReference::referenceType_t
			case 31 : return 9;
			// OutputDefinition::fileType_t
			case 32 : return 1;
			// OutputDefinition::timeType_t
			case 33 : return 2;
			// ParametrizationDefaults::mode_t
			case 34 : return 1;
			// ParametrizationDefaults::para_t
			case 35 : return 9;
			// ProjectInfo::para_t
			case 36 : return 1;
			// ProjectInfo::flag_t
			case 37 : return 1;
			// Schedule::type_t
			case 38 : return 10;
			// Schedules::day_t
			case 39 : return 6;
			// SimulationParameter::para_t
			case 40 : return 9;
			// SimulationParameter::intpara_t
			case 41 : return 0;
			// SimulationParameter::stringPara_t
			case 42 : return 1;
			// SimulationParameter::flag_t
			case 43 : return 3;
			// SolverParameter::para_t
			case 44 : return 21;
			// SolverParameter::flag_t
			case 45 : return 2;
			// SolverParameter::simpleTypesDesc
			case 46 : return 2;
			// SolverParameter::integrator_t
			case 47 : return 3;
			// SolverParameter::lesSolver_t
			case 48 : return 7;
			// SolverParameter::precond_t
			case 49 : return 3;
			// Zone::zoneType_t
			case 50 : return 3;
			// Zone::location_t
			case 51 : return 2;
			// Zone::para_t
			case 52 : return 9;
			// Zone::intpara_t
			case 53 : return 0;
			// ZoneList::performanceIndicatorType_t
			case 54 : return 2;
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
