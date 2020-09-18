/*	NANDRAD Solver Framework and Model Implementation.

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

#include <iostream>

#include <IBK_MessageHandler.h>
#include <IBK_MessageHandlerRegistry.h>
#include <IBK_messages.h>

// include solver control framework and integrator
#include <SOLFRA_SolverControlFramework.h>

// include header for command line argument parser
#include <NANDRAD_ArgsParser.h>
#include <NANDRAD_Project.h>

// include model implementation class
#include "NM_NandradModel.h"

//#define TEST_PROJECT_WRITING

//#define SERIALIZATION_TEST
#ifdef SERIALIZATION_TEST
#include <NANDRAD_SerializationTest.h>
#include <NANDRAD_Utilities.h>
#include <tinyxml.h>
#endif // SERIALIZATION_TEST

const char * const PROGRAM_INFO =
	"NANDRAD Solver\n"
	"All rights reserved.\n\n"
	"The NANDRAD Development Team:\n"
	"Anne Paepcke, Andreas Nicolai\n"
	"Contact: \n"
	"  anne.paepcke [at] tu-dresden.de\n"
	"  andreas.nicolai [at] tu-dresden.de\n\n";

#ifdef TEST_PROJECT_WRITING
void createSim01(NANDRAD::Project &prj){

	//project info
	prj.m_projectInfo.m_comment = "Yes we can! TF01";
	prj.m_projectInfo.m_created = "Now!";
	prj.m_projectInfo.m_lastEdited = "Yesterday";

	//create a zone
	NANDRAD::Zone zone;
	zone.m_id = 1;
	zone.m_displayName = "TF01";
	zone.m_type = NANDRAD::Zone::ZT_Active;
	zone.m_para[NANDRAD::Zone::P_Area].set("Area", 10, IBK::Unit("m2"));
	zone.m_para[NANDRAD::Zone::P_Volume].set("Volume", 30, IBK::Unit("m3"));
	zone.m_para[NANDRAD::Zone::P_Temperature].set("Temperature", 5, IBK::Unit("C"));
	//add zone to prj
	prj.m_zones.push_back(zone);

	prj.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", 2015);

	prj.m_location.m_climateFileName = IBK::Path("${Project Directory}/climate/testClimate.epw");

	prj.m_location.m_para[NANDRAD::Location::P_Albedo].set("Albedo", 0.2, IBK::Unit("---"));


	// outputs

	NANDRAD::Outputs outputs;


	// grids

	NANDRAD::OutputGrid grid;
	grid.m_name = "hourly";

	NANDRAD::Interval intVal;
	//intVal.m_para[NANDRAD::Interval::IP_END].set("End", 1, IBK::Unit("d"));
	intVal.m_para[NANDRAD::Interval::P_StepSize].set("StepSize", 1, IBK::Unit("h"));
	grid.m_intervals.push_back(intVal);

	outputs.m_grids.push_back(grid);
	prj.m_outputs.m_grids.push_back(grid);

	grid.m_name = "minutely";
	grid.m_intervals.clear();

	intVal.m_para[NANDRAD::Interval::P_Start].set("Start", 90720, IBK::Unit("min"));
	intVal.m_para[NANDRAD::Interval::P_End].set("End", 92160, IBK::Unit("min"));
	intVal.m_para[NANDRAD::Interval::P_StepSize].set("StepSize", 1, IBK::Unit("min"));
	grid.m_intervals.push_back(intVal);

	outputs.m_grids.push_back(grid);
	prj.m_outputs.m_grids.push_back(grid);


	NANDRAD::OutputDefinition outDef;
	outDef.m_quantity = "Temperature";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "All zones";
	prj.m_outputs.m_definitions.push_back(outDef);

	outDef.m_quantity = "ElevationAngle";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "Location";
	prj.m_outputs.m_definitions.push_back(outDef);

	outDef.m_quantity = "AzimuthAngle";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "Location";
	prj.m_outputs.m_definitions.push_back(outDef);

	outDef.m_quantity = "ElevationAngle";
	outDef.m_gridName = "minutely";
	outDef.m_objectListName = "Location";
	prj.m_outputs.m_definitions.push_back(outDef);

	outDef.m_quantity = "AzimuthAngle";
	outDef.m_gridName = "minutely";
	outDef.m_objectListName = "Location";
	prj.m_outputs.m_definitions.push_back(outDef);

	// Object lists (needed by outputs)
	NANDRAD::ObjectList ol;
	ol.m_name = "All zones";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Location";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
	prj.m_objectLists.push_back(ol);
}

void createSim02(NANDRAD::Project &prj){

	//project info
	prj.m_projectInfo.m_comment = "Yes we can! TF02";
	prj.m_projectInfo.m_created = "Now!";
	prj.m_projectInfo.m_lastEdited = "Yesterday";

	//create a zone
	NANDRAD::Zone zone;
	zone.m_id = 1;
	zone.m_displayName = "TF02";
	zone.m_type = NANDRAD::Zone::ZT_Active;
	zone.m_para[NANDRAD::Zone::P_Area].set("Area", 10, IBK::Unit("m2"));
	zone.m_para[NANDRAD::Zone::P_Volume].set("Volume", 30, IBK::Unit("m3"));
	zone.m_para[NANDRAD::Zone::P_Temperature].set("Temperature", 5, IBK::Unit("C"));
	//add zone to prj
	prj.m_zones.push_back(zone);

	prj.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", 2015);
	//	prj.m_solverParameter.initDefaults();

	prj.m_location.m_climateFileName = IBK::Path("climate/testClimate.epw");
	prj.m_location.m_para[NANDRAD::Location::P_Latitude].set("Latitude", 51, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::P_Longitude].set("Longitude",13, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::P_Altitude].set("Altitude",100, IBK::Unit("m"));
	prj.m_location.m_para[NANDRAD::Location::P_Albedo].set("Albedo", 0.2, IBK::Unit("---"));


	std::vector<double> inclination {90,90,90,90,90,90,90,90,60,60,60,60,60,60,60,60,30,30,30,30,30,30,30,30,45,45,45,45,45,45,45,45,0};
	std::vector<double> orientation {0,45,90,135,180,225,270,315,0,45,90,135,180,225,270,315,0,45,90,135,180,225,270,315,0,45,90,135,180,225,270,315,0};
	std::vector<std::string> name {"North", "NorthEast", "East", "SouthEast", "South", "SouthWest", "West", "NorthWest"};

	unsigned int conId=2;
	for (size_t i=0; i<inclination.size(); ++i) {

		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::P_ORIENTATION].set("Orientation", orientation[i], IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_INCLINATION].set("Inclination", inclination[i], IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_AREA].set("Area", 1, IBK::Unit("m2"));
		conInsta.m_displayName = name[i%name.size()] + "_" + IBK::val2string(inclination[i]);

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 1;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_Constant;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		//add first interface
		conInsta.m_interfaceA = interface;

		// Outside interface
		interface.m_id = conId++;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_Constant;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_Constant;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 1, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaceB = interface;
		conInsta.m_constructionTypeId = 10001;
		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);
	}

	//construction type
	NANDRAD::ConstructionType conType;
	conType.m_id = prj.m_constructionInstances.back().m_constructionTypeId;
	conType.m_displayName = "Exterior wall";

	//material
	NANDRAD::Material mat;
	mat.m_id = 1001;
	mat.m_displayName = "Concrete";
	mat.m_para[NANDRAD::Material::P_Density].set("Density", 2100, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::P_HeatCapacity].set("HeatCapacity", 840, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::P_Conductivity].set("Conductivity", 2.1, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);
	// Left side (A) = concrete (inside), right side (B) insulation (outside)
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.2, mat.m_id));

	mat.m_id = 1002;
	mat.m_displayName = "Insulation";
	mat.m_para[NANDRAD::Material::P_Density].set("Density", 30, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::P_HeatCapacity].set("HeatCapacity", 1500, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::P_Conductivity].set("Conductivity", 0.04, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.1, mat.m_id));

	// add construction type
	prj.m_constructionTypes.push_back(conType);

	//outputs

	NANDRAD::OutputGrid grid;
	grid.m_name = "hourly";

	NANDRAD::Interval intVal;
	//intVal.m_para[NANDRAD::Interval::IP_END].set("End", 1, IBK::Unit("d"));
	intVal.m_para[NANDRAD::Interval::P_StepSize].set("StepSize", 1, IBK::Unit("h"));
	grid.m_intervals.push_back(intVal);

	NANDRAD::Outputs outputs;
	outputs.m_grids.push_back(grid);
	prj.m_outputs.m_grids.push_back(grid);

	NANDRAD::OutputDefinition outDef;
	outDef.m_quantity = "Temperature";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "All zones";
	outDef.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
	prj.m_outputs.m_definitions.push_back(outDef);

	outDef.m_quantity = "ElevationAngle";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "Location";
	outDef.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
	prj.m_outputs.m_definitions.push_back(outDef);

	outDef.m_quantity = "AzimuthAngle";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "Location";
	outDef.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
	prj.m_outputs.m_definitions.push_back(outDef);

	outDef.m_quantity = "SWRadDiffuse";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "Interface";
	outDef.m_timeType = NANDRAD::OutputDefinition::OTT_INTEGRAL;
	prj.m_outputs.m_definitions.push_back(outDef);

	outDef.m_quantity = "SWRadDirect";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "Interface";
	outDef.m_timeType = NANDRAD::OutputDefinition::OTT_INTEGRAL;
	prj.m_outputs.m_definitions.push_back(outDef);


	// Object lists (needed by outputs)
	NANDRAD::ObjectList ol;
	ol.m_name = "All zones";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Location";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Interface";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_INTERFACE;
	prj.m_objectLists.push_back(ol);
}

/* ToDo Stephan ID-Space checken nicht das da welche ineinander laufen. */

void createSim03(NANDRAD::Project &prj){

	//project info
	prj.m_projectInfo.m_comment = "Yes we can! TF03";
	prj.m_projectInfo.m_created = "Now!";
	prj.m_projectInfo.m_lastEdited = "Yesterday";

	//create a zone
	NANDRAD::Zone zone;
	zone.m_id = 1;
	zone.m_displayName = "TF03.1";
	zone.m_type = NANDRAD::Zone::ZT_Active;
	zone.m_para[NANDRAD::Zone::P_Area].set("Area", 10, IBK::Unit("m2"));
	zone.m_para[NANDRAD::Zone::P_Volume].set("Volume", 0.0001, IBK::Unit("m3"));
	zone.m_para[NANDRAD::Zone::P_Temperature].set("Temperature", 5, IBK::Unit("C"));
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF03.2";
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF03.3";
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF03.4";
	//add zone to prj
	prj.m_zones.push_back(zone);

	prj.m_simulationParameter.m_intPara[NANDRAD::SimulationParameter::IP_StartYear].set("StartYear", 2015);
	//	prj.m_solverParameter.initDefaults();

	prj.m_location.m_climateFileName = IBK::Path("climate/testClimate.epw");
	prj.m_location.m_para[NANDRAD::Location::P_Latitude].set("Latitude", 51, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::P_Longitude].set("Longitude",13, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::P_Altitude].set("Altitude",100, IBK::Unit("m"));
	prj.m_location.m_para[NANDRAD::Location::P_Albedo].set("Albedo", 0.2, IBK::Unit("---"));

	unsigned int conId=2;
	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::P_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 1;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_Constant;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_Constant;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_Constant;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaceA = interface;

		// Outside interface
		interface.m_id = conId++;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_Constant;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_Constant;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaceB = interface;
		conInsta.m_constructionTypeId = 10001;
		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaceA.m_id = conId++;
		conInsta.m_interfaceB.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::P_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaceA.m_id = conId++;
		conInsta.m_interfaceB.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::P_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaceA.m_id = conId++;
		conInsta.m_interfaceB.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::P_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaceA.m_id = conId++;
		conInsta.m_interfaceB.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::P_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaceA.m_id = conId++;
		conInsta.m_interfaceB.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::P_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::P_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}
#if 0

	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_constructionTypeId = 10002;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 2;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);

		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}
	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_constructionTypeId = 10003;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 3;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);

		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}
	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_constructionTypeId = 10004;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 4;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);

		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}
#endif
	//construction type

	//material
	NANDRAD::Material mat;
	mat.m_id = 1001;
	mat.m_displayName = "Brick";
	mat.m_para[NANDRAD::Material::P_Density].set("Density", 2000, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::P_HeatCapacity].set("HeatCapacity", 1000, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::P_Conductivity].set("Conductivity", 1.2, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);
	// Left side (A) = concrete (inside), right side (B) insulation (outside)

	mat.m_id = 1002;
	mat.m_displayName = "Insulation";
	mat.m_para[NANDRAD::Material::P_Density].set("Density", 50, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::P_HeatCapacity].set("HeatCapacity", 1000, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::P_Conductivity].set("Conductivity", 0.04, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);

	mat.m_id = 1003;
	mat.m_displayName = "Board";
	mat.m_para[NANDRAD::Material::P_Density].set("Density", 800, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::P_HeatCapacity].set("HeatCapacity", 1500, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::P_Conductivity].set("Conductivity", 0.14, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);

	// add construction type
	NANDRAD::ConstructionType conType;
	conType.m_id = 10001;
	conType.m_displayName = "Construction 1";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.2, prj.m_materials[0].m_id));
	prj.m_constructionTypes.push_back(conType);

	conType.m_materialLayers.clear();
	conType.m_id = 10002;
	conType.m_displayName = "Construction 2";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.1, prj.m_materials[1].m_id));
	prj.m_constructionTypes.push_back(conType);

	conType.m_materialLayers.clear();
	conType.m_id = 10003;
	conType.m_displayName = "Construction 3";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.005, prj.m_materials[2].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.1, prj.m_materials[1].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.2, prj.m_materials[0].m_id));
	prj.m_constructionTypes.push_back(conType);

	conType.m_materialLayers.clear();
	conType.m_id = 10004;
	conType.m_displayName = "Construction 4";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.2, prj.m_materials[0].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.1, prj.m_materials[1].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.005, prj.m_materials[2].m_id));
	prj.m_constructionTypes.push_back(conType);

	//outputs

	NANDRAD::OutputGrid grid;
	grid.m_name = "hourly";

	NANDRAD::Interval intVal;
	//intVal.m_para[NANDRAD::Interval::IP_END].set("End", 1, IBK::Unit("d"));
	intVal.m_para[NANDRAD::Interval::P_StepSize].set("StepSize", 1, IBK::Unit("h"));
	grid.m_intervals.push_back(intVal);

	NANDRAD::Outputs outputs;
	outputs.m_grids.push_back(grid);
	prj.m_outputs.m_grids.push_back(grid);

	NANDRAD::OutputDefinition outDef;
	outDef.m_quantity = "Temperature";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "All zones";
	outDef.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
	prj.m_outputs.m_definitions.push_back(outDef);

	// Object lists (needed by outputs)
	NANDRAD::ObjectList ol;
	ol.m_name = "All zones";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Location";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Interface";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_INTERFACE;
	prj.m_objectLists.push_back(ol);
}
#if 0

void createSim04(NANDRAD::Project &prj){

	//project info
	prj.m_projectInfo.m_comment = "Yes we can! TF04";
	prj.m_projectInfo.m_created = "Now!";
	prj.m_projectInfo.m_version = "NANDRAD 2.0";
	prj.m_projectInfo.m_lastEdited = "Yesterday";

	//create a zone
	NANDRAD::Zone zone;
	zone.m_id = 1;
	zone.m_displayName = "TF04.1";
	zone.m_type = NANDRAD::Zone::ZT_ACTIVE;
	zone.m_para[NANDRAD::Zone::ZP_AREA].set("Area", 10, IBK::Unit("m2"));
	zone.m_para[NANDRAD::Zone::ZP_VOLUME].set("Volume", 30, IBK::Unit("m3"));
	zone.m_para[NANDRAD::Zone::ZP_TEMPERATURE].set("Temperature", 5, IBK::Unit("C"));
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF04.2";
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF04.3 A";
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF04.3 B";
	//add zone to prj
	prj.m_zones.push_back(zone);

	prj.m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].set("StartYear", 2015);
	//	prj.m_solverParameter.initDefaults();

	prj.m_location.m_climateFileName = IBK::Path("climate/testClimate.epw");
	prj.m_location.m_para[NANDRAD::Location::LP_LATITUDE].set("Latitude", 51, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::LP_LONGITUDE].set("Longitude",13, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::LP_ALTITUDE].set("Altitude",100, IBK::Unit("m"));
	prj.m_location.m_para[NANDRAD::Location::LP_ALBEDO].set("Albedo", 0.2, IBK::Unit("---"));

	unsigned int conId=2;
	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 1;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface

		//conInsta.m_interfaces.push_back(interface);

		conInsta.m_constructionTypeId = 10001;
		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		//conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces.push_back(interface);
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		conInsta.m_constructionTypeId = 10002;
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		conInsta.m_interfaces.pop_back();
		conInsta.m_constructionTypeId = 10001;
		prj.m_constructionInstances.push_back(conInsta);
	}

	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_constructionTypeId = 10001;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 2;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);

		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		conInsta.m_constructionTypeId = 10002;
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		conInsta.m_constructionTypeId = 10001;
		prj.m_constructionInstances.push_back(conInsta);
	}

	//TF04.3
	{
		//Zone A
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_constructionTypeId = 10003;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North A";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 3;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);

		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South A";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East A";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof A";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor A";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		//Zone B
		conInsta.m_constructionTypeId = 10004;
		conInsta.m_interfaces[0].m_zoneId = 4;
		conInsta.m_displayName = "North B";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South B";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West B";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof B";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor B";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);


		conInsta.m_displayName = "Partition";
		conInsta.m_constructionTypeId = 10001;
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_interfaces[0].m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set(2.5, IBK::Unit("W/m2K"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}

	//construction type

	//material
	NANDRAD::Material mat;
	mat.m_id = 1001;
	mat.m_displayName = "Brick";
	mat.m_para[NANDRAD::Material::MP_DENSITY].set("Density", 2000, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::MP_HEAT_CAPACITY].set("HeatCapacity", 1000, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::MP_CONDUCTIVITY].set("Conductivity", 1.2, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);
	// Left side (A) = concrete (inside), right side (B) insulation (outside)

	mat.m_id = 1002;
	mat.m_displayName = "Insulation";
	mat.m_para[NANDRAD::Material::MP_DENSITY].set("Density", 50, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::MP_HEAT_CAPACITY].set("HeatCapacity", 1000, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::MP_CONDUCTIVITY].set("Conductivity", 0.04, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);

	mat.m_id = 1003;
	mat.m_displayName = "Board";
	mat.m_para[NANDRAD::Material::MP_DENSITY].set("Density", 800, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::MP_HEAT_CAPACITY].set("HeatCapacity", 1500, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::MP_CONDUCTIVITY].set("Conductivity", 0.14, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);

	// add construction type
	NANDRAD::ConstructionType conType;
	conType.m_id = 10001;
	conType.m_displayName = "Construction 1";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.2, prj.m_materials[0].m_id));
	prj.m_constructionTypes.push_back(conType);

	conType.m_materialLayers.clear();
	conType.m_id = 10002;
	conType.m_displayName = "Construction 2";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.1, prj.m_materials[1].m_id));
	prj.m_constructionTypes.push_back(conType);

	conType.m_materialLayers.clear();
	conType.m_id = 10003;
	conType.m_displayName = "Construction 3";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.005, prj.m_materials[2].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.1, prj.m_materials[1].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.2, prj.m_materials[0].m_id));
	prj.m_constructionTypes.push_back(conType);

	conType.m_materialLayers.clear();
	conType.m_id = 10004;
	conType.m_displayName = "Construction 4";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.2, prj.m_materials[0].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.1, prj.m_materials[1].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.005, prj.m_materials[2].m_id));
	prj.m_constructionTypes.push_back(conType);

	//outputs

	NANDRAD::OutputGrid grid;
	grid.m_name = "hourly";

	NANDRAD::Interval intVal;
	//intVal.m_para[NANDRAD::Interval::IP_END].set("End", 1, IBK::Unit("d"));
	intVal.m_para[NANDRAD::Interval::IP_STEPSIZE].set("StepSize", 1, IBK::Unit("h"));
	grid.m_intervals.push_back(intVal);

	NANDRAD::Outputs outputs;
	outputs.m_grids.push_back(grid);
	prj.m_outputs.m_grids.push_back(grid);

	NANDRAD::OutputDefinition outDef;
	outDef.m_quantity = "Temperature";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "All zones";
	outDef.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
	prj.m_outputs.m_definitions.push_back(outDef);

	NANDRAD::OutputDefinition outDef2;
	outDef2.m_quantity = "Location.ElevationAngle";
	outDef2.m_gridName = "hourly";
	outDef2.m_objectListName = "climate";

	prj.m_outputs.m_definitions.push_back(outDef2);

	// Object lists (needed by outputs)
	NANDRAD::ObjectList ol;
	ol.m_name = "All zones";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Location";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Interface";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_INTERFACE;
	prj.m_objectLists.push_back(ol);
}

/* ToDo Stephan ich hab die Zone TF05.4 schon mal angelegt da muss dann die Lüftung rein die auf einen Zonentemperatursetwert reagiert. */

void createSim05(NANDRAD::Project &prj){

	//project info
	prj.m_projectInfo.m_comment = "Yes we can! TF05";
	prj.m_projectInfo.m_created = "Now!";
	prj.m_projectInfo.m_version = "NANDRAD 2.0";
	prj.m_projectInfo.m_lastEdited = "Yesterday";

	//create a zone
	NANDRAD::Zone zone;
	zone.m_id = 1;
	zone.m_displayName = "TF05.1";
	zone.m_type = NANDRAD::Zone::ZT_ACTIVE;
	zone.m_para[NANDRAD::Zone::ZP_AREA].set("Area", 10, IBK::Unit("m2"));
	zone.m_para[NANDRAD::Zone::ZP_VOLUME].set("Volume", 0.0001, IBK::Unit("m3"));
	zone.m_para[NANDRAD::Zone::ZP_TEMPERATURE].set("Temperature", 5, IBK::Unit("C"));
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF05.2";
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF05.3";
	//add zone to prj
	prj.m_zones.push_back(zone);
	++zone.m_id;
	zone.m_displayName = "TF05.4";
	//add zone to prj
	prj.m_zones.push_back(zone);

	prj.m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].set("StartYear", 2015);
	//	prj.m_solverParameter.initDefaults();

	prj.m_location.m_climateFileName = IBK::Path("climate/testClimate.epw");
	prj.m_location.m_para[NANDRAD::Location::LP_LATITUDE].set("Latitude", 51, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::LP_LONGITUDE].set("Longitude",13, IBK::Unit("Deg"));
	prj.m_location.m_para[NANDRAD::Location::LP_ALTITUDE].set("Altitude",100, IBK::Unit("m"));
	prj.m_location.m_para[NANDRAD::Location::LP_ALBEDO].set("Albedo", 0.2, IBK::Unit("---"));

	unsigned int conId=2;
	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 1;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);
		conInsta.m_constructionTypeId = 10001;
		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}

	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_constructionTypeId = 10001;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 2;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);

		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}
	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_constructionTypeId = 10001;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 3;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);

		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}
	{
		NANDRAD::ConstructionInstance conInsta;
		conInsta.m_constructionTypeId = 10001;
		conInsta.m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		conInsta.m_displayName = "North";

		// create interfaces
		// Inside interface
		NANDRAD::Interface interface;
		interface.m_id = conId++;
		interface.m_zoneId = 4;
		interface.m_location = NANDRAD::Interface::IT_A;
		interface.m_heatConduction.m_modelType = NANDRAD::InterfaceHeatConduction::MT_CONSTANT;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 2.5, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		//add first interface
		conInsta.m_interfaces.push_back(interface);

		// Outside interface
		interface.m_id = conId++;
		interface.m_location = NANDRAD::Interface::IT_B;
		interface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient].set("HeatTransferCoefficient", 8, IBK::Unit("W/m2K"));
		interface.m_longWaveEmission.m_modelType = NANDRAD::InterfaceLongWaveEmission::MT_CONSTANT;
		interface.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].set("Emissivity", 0, IBK::Unit("---"));
		interface.m_solarAbsorption.m_modelType = NANDRAD::InterfaceSolarAbsorption::MT_CONSTANT;
		interface.m_solarAbsorption.m_para[NANDRAD::InterfaceSolarAbsorption::P_AbsorptionCoefficient].set("AbsorptionCoefficient", 0, IBK::Unit("---"));
		interface.m_zoneId = 0;
		//add second interface
		conInsta.m_interfaces.push_back(interface);

		//add construction instance
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "South";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 6, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "East";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "West";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 270, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 90, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 15, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Roof";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);

		conInsta.m_displayName = "Floor";
		conInsta.m_id = conId++;
		conInsta.m_interfaces[0].m_id = conId++;
		conInsta.m_interfaces[1].m_id = conId++;
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_ORIENTATION].set("Orientation", 0, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_INCLINATION].set("Inclination", 180, IBK::Unit("Deg"));
		conInsta.m_para[NANDRAD::ConstructionInstance::CP_AREA].set("Area", 10, IBK::Unit("m2"));
		prj.m_constructionInstances.push_back(conInsta);
	}

	//construction type

	//material
	NANDRAD::Material mat;
	mat.m_id = 1001;
	mat.m_displayName = "Brick";
	mat.m_para[NANDRAD::Material::MP_DENSITY].set("Density", 2000, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::MP_HEAT_CAPACITY].set("HeatCapacity", 1000, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::MP_CONDUCTIVITY].set("Conductivity", 1.2, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);
	// Left side (A) = concrete (inside), right side (B) insulation (outside)

	mat.m_id = 1002;
	mat.m_displayName = "Good Insulation";
	mat.m_para[NANDRAD::Material::MP_DENSITY].set("Density", 50, IBK::Unit("kg/m3"));
	mat.m_para[NANDRAD::Material::MP_HEAT_CAPACITY].set("HeatCapacity", 1000, IBK::Unit("J/kgK"));
	mat.m_para[NANDRAD::Material::MP_CONDUCTIVITY].set("Conductivity", 0.02, IBK::Unit("W/mK"));
	prj.m_materials.push_back(mat);

	// add construction type
	NANDRAD::ConstructionType conType;
	conType.m_id = 10001;
	conType.m_displayName = "Construction 1";
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.2, prj.m_materials[0].m_id));
	conType.m_materialLayers.push_back(NANDRAD::MaterialLayer(0.3, prj.m_materials[1].m_id));
	prj.m_constructionTypes.push_back(conType);

	NANDRAD::DailyCycle daily;
	daily.m_timeUnit = IBK::Unit("h");
	daily.m_interpolation = NANDRAD::DailyCycle::IT_CONSTANT;
	daily.m_timePoints = std::vector<double>{0};
	daily.m_values.m_values["InfiltrationAirChangeRate"]= std::vector<double>(1,2);

	NANDRAD::Schedule sched;
	sched.m_type = NANDRAD::Schedule::ST_ALLDAYS;
	sched.m_dailyCycles.push_back(daily);

	prj.m_schedules.m_scheduleGroups["All zones"].push_back(sched);
	prj.m_schedules.m_holidays.insert( IBK::Time::fromDateTimeFormat("27.02.1976 01:00:00"));
	prj.m_schedules.m_weekEndDays.insert(NANDRAD::Schedules::SD_SATURDAY);
	prj.m_schedules.m_weekEndDays.insert(NANDRAD::Schedules::SD_SUNDAY);

	//outputs
	NANDRAD::OutputGrid grid;
	grid.m_name = "hourly";

	NANDRAD::Interval intVal;
	//intVal.m_para[NANDRAD::Interval::IP_END].set("End", 1, IBK::Unit("d"));
	intVal.m_para[NANDRAD::Interval::IP_STEPSIZE].set("StepSize", 1, IBK::Unit("h"));
	grid.m_intervals.push_back(intVal);

	NANDRAD::Outputs outputs;
	outputs.m_grids.push_back(grid);
	prj.m_outputs.m_grids.push_back(grid);

	NANDRAD::OutputDefinition outDef;
	outDef.m_quantity = "Temperature";
	outDef.m_gridName = "hourly";
	outDef.m_objectListName = "All zones";
	outDef.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
	prj.m_outputs.m_definitions.push_back(outDef);

	// Object lists (needed by outputs)
	NANDRAD::ObjectList ol;
	ol.m_name = "All zones";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Location";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_LOCATION;
	prj.m_objectLists.push_back(ol);

	ol.m_name = "Interface";
	ol.m_filterID.setEncodedString("*"); // all
	ol.m_referenceType = NANDRAD::ModelInputReference::MRT_INTERFACE;
	prj.m_objectLists.push_back(ol);
}
#endif
#endif

int main(int argc, char * argv[]) {
	FUNCID(main);

	NANDRAD::LinearSplineParameter linSp;

	linSp.m_xUnit = IBK::Unit("h");
	linSp.m_yUnit = IBK::Unit("C");
	linSp.m_values.setValues(std::vector<double>({0, 2, 4 }), std::vector<double>({0, 2, 4 }) );
	linSp.m_name = "wurstsuppe";

	std::vector< NANDRAD::LinearSplineParameter > linSpline;

	linSpline.emplace_back(linSp);

	std::map<std::string, std::vector< NANDRAD::LinearSplineParameter > > splineMap;

	splineMap["wurst"] = linSpline;

//		splineMap.insert(splineMap.end(),std::pair< std::string, std::vector< NANDRAD::LinearSplineParameter > > ("wurst",linSpline));

	NANDRAD::Project prj;
	NANDRAD::Schedules sched;

	prj.m_schedules.m_annualSchedules = splineMap;

	IBK::Path path ("test.nandrad");

	prj.writeXML(path);

#ifdef TEST_PROJECT_WRITING

	NANDRAD::Project prj;
	createSim01(prj);
	prj.writeXML(IBK::Path("SimQuality1.xml"));

	prj = NANDRAD::Project();
	createSim02(prj);
	prj.writeXML(IBK::Path("SimQuality2.xml"));

	prj = NANDRAD::Project();
	createSim03(prj);
	prj.writeXML(IBK::Path("SimQuality3.xml"));

//	prj = NANDRAD::Project();
//	createSim04(prj);
//	prj.writeXML(IBK::Path("SimQuality4.xml"));

//	prj = NANDRAD::Project();
//	createSim05(prj);
//	prj.writeXML(IBK::Path("SimQuality5.xml"));

	// now read and write projects again to check if all data is read correctly back in
	std::vector<std::string> projects = {"SimQuality1.xml", "SimQuality2.xml", "SimQuality3.xml", "SimQuality4.xml", "SimQuality5.xml"};

	for (const std::string & p : projects) {
		// now create a new project, read back the file and write it again with a different name
		NANDRAD::Project prj;
		try {
			prj.readXML(IBK::Path(p));
			prj.writeXML(IBK::Path(p).withoutExtension() + "_2.xml");
			std::cout << "Wrote '" << p << "'." << std::endl;
		} catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
		}
	}
	return EXIT_SUCCESS;
#endif // TEST_PROJECT_WRITING
#ifdef SERIALIZATION_TEST
	NANDRAD::SerializationTest st;
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "NandradProject" );
	doc.LinkEndChild(root);

	// write all project parts
	st.writeXML(root);
	IBK::Path filenamePath("serializationtest.xml");
	doc.SaveFile( filenamePath.str() );

	// now read in the file in separate object and compare

	NANDRAD::SerializationTest st2;
	TiXmlDocument doc2;
	std::map<std::string,IBK::Path> placeholders;
	TiXmlElement * xmlElem = NANDRAD::openXMLFile(placeholders, filenamePath, "NandradProject", doc2);
	if (!xmlElem)
		return 1; // empty project, this means we are using only defaults

	// we read our subsections from this handle
	TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

	try {
		// Project Info
		xmlElem = xmlRoot.FirstChild("SerializationTest").Element();
		st2.readXML(xmlElem);
	}
	catch (IBK::Exception &ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message(IBK::FormatString("Error in project file '%1'.")
			.arg(filenamePath), IBK::MSG_ERROR, FUNC_ID);
	}

	return 0;

#endif // SERIALIZATION_TEST

	try {
		// a stopwatch to measure time needed for solver initialization
		IBK::StopWatch initWatch;

		// *** Command line parsing ***
		NANDRAD::ArgsParser args;
		args.parse(argc, argv);
		// handle default arguments like help and man-page requests, which are printed to std::cout
		if (args.handleDefaultFlags(std::cout))
			// stop if help/man-page requested
			return EXIT_SUCCESS;
		if (args.flagEnabled(IBK::SolverArgsParser::DO_VERSION)) {
			std::cout << PROGRAM_INFO << std::endl;
			NANDRAD_MODEL::NandradModel::printVersionStrings();
			SOLFRA::SolverControlFramework::printVersionInfo();
			IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			return EXIT_SUCCESS;
		}

		// check if errors are present, error messages are written to std::cerr
		if (args.handleErrors(std::cerr))
			return EXIT_FAILURE;

		// *** create main model instance ***
		NANDRAD_MODEL::NandradModel model;

		// *** create directory structure ***
		model.setupDirectories(args);
		// now we have a log directory and can write our messages to the log file

		// *** setup message handler ***

		unsigned int verbosityLevel = IBK::string2val<unsigned int>(args.option(IBK::SolverArgsParser::DO_VERBOSITY_LEVEL));
		IBK::MessageHandler * messageHandlerPtr = IBK::MessageHandlerRegistry::instance().messageHandler();
		messageHandlerPtr->setConsoleVerbosityLevel((int)verbosityLevel);
		messageHandlerPtr->setLogfileVerbosityLevel((int)verbosityLevel);
		messageHandlerPtr->m_contextIndentation = 48;
		std::string errmsg;

		IBK::Path logfile = model.dirs().m_logDir / "screenlog.txt";
		bool success = messageHandlerPtr->openLogFile(logfile.str(), args.m_restart, errmsg);
		if (!success) {
			IBK::IBK_Message(errmsg, IBK::MSG_WARNING, FUNC_ID);
			IBK::IBK_Message("Cannot create log file, outputs will only be printed on screen.", IBK::MSG_WARNING, FUNC_ID);
		}

		// *** write program/copyright info ***
		IBK::IBK_Message(PROGRAM_INFO, IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		NANDRAD_MODEL::NandradModel::printVersionStrings();
		SOLFRA::SolverControlFramework::printVersionInfo();
		IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// *** OpenMP Info ***
#if defined(_OPENMP)
		IBK::IBK_Message( IBK::FormatString("OpenMP parallelization enabled: using %1 thread(s)\n\n")
						  .arg(args.m_numParallelThreads), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		if (!args.hasOption(IBK::SolverArgsParser::GO_PARALLEL_THREADS)) {
			IBK::IBK_Message( IBK::FormatString("Number of threads selected from variable OMP_NUM_THREADS = %1\n\n")
							  .arg(args.m_numParallelThreads), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		}
#endif // _OPENMP

		// *** Initialize model. ***

		// init model (first read project, then initialize model)
		model.init(args);
		IBK::IBK_Message( IBK::FormatString("Model initialization complete, duration: %1\n\n").arg(initWatch.diff_str()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);

		// *** Run model through solver control framework ***
		IBK::IBK_Message("Creating solver framework\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		SOLFRA::SolverControlFramework solver(&model);
		solver.m_useStepStatistics = args.flagEnabled(IBK::SolverArgsParser::DO_STEP_STATS);
		solver.m_logDirectory = model.dirs().m_logDir;
		solver.m_stopAfterSolverInit = args.flagEnabled(IBK::SolverArgsParser::GO_TEST_INIT);
		solver.m_restartFilename = model.dirs().m_varDir / "restart.bin";

		// depending on the restart settings, either run from start or continue simulation
		if (args.m_restartFrom) {
			IBK::IBK_Message("Continuing computation from selected check point\n\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			solver.restartFrom(args.m_restartTime);
		}
		else if (args.m_restart) {
			IBK::IBK_Message("Continuing computation from last recorded check point\n\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			solver.restart();
		}
		else {
			solver.run();
			if (!solver.m_stopAfterSolverInit)
				solver.writeMetrics();
			else {
				IBK::IBK_Message( IBK::FormatString("Total initialization time: %1\n").arg(initWatch.diff_str()), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			}
		}
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("Critical error, simulation aborted.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}
	catch (std::exception& ex) {
		IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
		IBK::IBK_Message("Critical error, simulation aborted.", IBK::MSG_ERROR, FUNC_ID);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

