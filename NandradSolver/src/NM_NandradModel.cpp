/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
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

#include "NM_NandradModel.h"

#include "NM_ConstantZoneModel.h"
#include "NM_Constants.h"
#include "NM_CoolingsLoadModel.h"
#include "NM_EquipmentLoadModel.h"
#include "NM_FMU.h"
#include "NM_FMU2ExportModel.h"
#include "NM_FMU2ImportModel.h"
#include "NM_FMU2ModelDescription.h"
#include "NM_HeatingsLoadModel.h"
#include "NM_IdealCoolingModel.h"
#include "NM_IdealHeatingModel.h"
#include "NM_InfiltrationModel.h"
#include "NM_KeywordList.h"
#include "NM_LightingLoadModel.h"
#include "NM_Loads.h"
#include "NM_NandradModelImpl.h"
#include "NM_NaturalVentilationLoadModel.h"
#include "NM_OutputFile.h"
#include "NM_RoomStatesModel.h"
#include "NM_ScheduleDays.h"
#include "NM_UsersThermalLoadModel.h"
#include "NM_UserModel.h"
#include "NM_WindowsLoadModel.h"

#include <NANDRAD_Constants.h>
#include <NANDRAD_DailyCycle.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_ObjectList.h>
#include <NANDRAD_Project.h>

#include <SOLFRA_IntegratorSundialsCVODE.h>
#include <SOLFRA_IntegratorImplicitEuler.h>
#include <SOLFRA_JacobianDense.h>
#include <SOLFRA_JacobianSparse.h>
#include <SOLFRA_LESGMRES.h>
#include <SOLFRA_LESBand.h>
#include <SOLFRA_LESGMRES.h>
#include <SOLFRA_LESDense.h>
#include <SOLFRA_PrecondBand.h>
#include <SOLFRA_PrecondILU.h>
#include <IBK_Exception.h>
#include <IBK_FormatString.h>

#include <IBK_physics.h>

#if 0
#ifdef _WIN32
	#include <iowin32.h>
#endif
#endif

#ifdef _WIN32
	#include <windows.h>
#else
	#include <dirent.h>
#endif

#include <minizip.h>

#include <IBK_Constants.h>
#include <IBK_messages.h>
#include <IBK_Version.h>

#include <MM_Version.h>

#include <DataIO>

#include <CCM_ClimateDataLoader.h>
#include <CCM_SolarRadiationModel.h>
#include <CCM_Constants.h>


namespace NANDRAD_MODEL {
// include the next file within the NANDRAD_MODEL namespace
#include "NM_NandradCoSimAdapterAndWrapper.include"

const int WRITEBUFFERSIZE = 8192;

NandradModel::NandradModel() :
	m_impl(new NandradModelImpl)
{
}

NandradModel::~NandradModel() {
	delete m_impl;
}


void NandradModel::init(const NANDRAD::ArgsParser & args) {
	const char * const FUNC_ID = "[NandradModel::init]";
	// relay initialization to model implementation object
	m_impl->init(args, this); // give NandradModel access to this object, which implements SOLFRA::ModelInterface

	// create integrator
	SOLFRA::IntegratorInterface * integrator = m_impl->integratorInterface();
	if (integrator == NULL)
		throw IBK::Exception("Missing implementation for selected integrator type.", FUNC_ID);
	// create les solver
	SOLFRA::LESInterface * lesSolver = m_impl->lesInterface();
	if (lesSolver == NULL && integrator->identifier() != std::string("Explicit Euler"))
		throw IBK::Exception("Missing implementation for selected LES solver.", FUNC_ID);
}

unsigned int NandradModel::n() const { return m_impl->n(); }

const double * NandradModel::y0() const { return m_impl->y0(); }

double NandradModel::t0() const { return m_impl->t0(); }

double NandradModel::dt0() const { return m_impl->dt0(); }

double NandradModel::tEnd() const { return m_impl->tEnd(); }

double NandradModel::t() const { return m_impl->t(); }

const double * NandradModel::y() const { return m_impl->y(); }

const NANDRAD::Project & NandradModel::project() const {
	return m_impl->project();
}


FMU2ExportModel * NandradModel::fmu2ExportModel() {
	return m_impl->m_FMU2ExportModel;
}

FMU2ImportModel * NandradModel::fmu2ImportModel() {
	return m_impl->m_FMU2ImportModel;
}


std::vector<OutputFile*> & NandradModel::outputFiles() {
	return m_impl->m_outputFiles;
}


SOLFRA::ModelInterface::CalculationResult NandradModel::setTime(double t) {
	return m_impl->setTime(t);
}

SOLFRA::ModelInterface::CalculationResult NandradModel::setY(const double * y) {
	return m_impl->setY(y);
}

SOLFRA::ModelInterface::CalculationResult NandradModel::ydot(double * ydot) {
	return m_impl->ydot(ydot);
}

void NandradModel::writeOutputs(double t_out, const double * y_out) {
	m_impl->writeOutputs(t_out,y_out);
}

std::string NandradModel::simTime2DateTimeString(double t) const {
	// add start time offset to t and then call parent function
	int startYear = m_impl->project().m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].value;
	t += m_impl->project().m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START].value;
	return IBK::Time(startYear, t).toShortDateFormat();
}

void NandradModel::stepCompleted(double t, const double * y ) {
	m_impl->stepCompleted(t,y);
}

void NandradModel::setupDirectories(const NANDRAD::ArgsParser & args, const IBK::Path & projectFile)
{
	const char * const FUNC_ID = "[NandradModel::setupDirectories]";

	try {
		// ***** Check if project file exists *****
		if (!projectFile.isFile())
			throw IBK::Exception( IBK::FormatString("Project file '%1' does not exist (or access denied).").arg(projectFile), FUNC_ID);

		// ***** Create directory structure for solver log and output files *****
		m_impl->m_projectFilePath = projectFile;

		if (args.hasOption(IBK::SolverArgsParser::GO_OUTPUT_DIR)) {
			m_impl->m_dirs.create( IBK::Path(args.option(IBK::SolverArgsParser::GO_OUTPUT_DIR)) );
		}
		else {
			m_impl->m_dirs.create(m_impl->m_projectFilePath.withoutExtension());
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Initialization of Nandrad Model for project '%1' failed.")
			.arg(projectFile), FUNC_ID);
	}
	catch (std::exception & ex) {
		throw IBK::Exception(IBK::FormatString("Initialization of Nandrad Model for project '%1' failed "
			"with error message:%2\n").arg(projectFile).arg(ex.what()), FUNC_ID);
	}
}


const Directories & NandradModel::dirs() const {
	return m_impl->m_dirs;
}


SOLFRA::LESInterface * NandradModel::lesInterface() {
	return m_impl->lesInterface();
}

SOLFRA::JacobianInterface * NandradModel::jacobianInterface() {
	return m_impl->jacobianInterface();
}


SOLFRA::IntegratorInterface * NandradModel::integratorInterface() {
	return m_impl->integratorInterface();
}


SOLFRA::PrecondInterface * NandradModel::preconditionerInterface() {
	return m_impl->preconditionerInterface();
}

SOLFRA::OutputScheduler * NandradModel::outputScheduler() {
	return m_impl;
}

#ifdef MODIFY_WEIGHT_FACTORS
NandradModel::CalculationResult NandradModel::calculateErrorWeights(const double *y, double *weights) {
	m_impl->calculateErrorWeights(y, weights);
	return CalculationSuccess;
}

bool NandradModel::hasErrorWeightsFunction() {
	return m_impl->hasErrorWeightsFunction();
}
#endif


std::size_t NandradModel::serializationSize() const {
	// nothing to serialize
	size_t s = 0;
	return s;
}


void NandradModel::serialize(void* & dataPtr) const {
	// nothing to serialize
}


void NandradModel::deserialize(void* & dataPtr) {
	// nothing to serialize
}

void NandradModel::writeMetrics(double simtime, std::ostream * metricsFile) {
	m_impl->writeMetrics(simtime, metricsFile);
}

void NandradModel::printVersionStrings() {
	const char * const FUNC_ID = "[ModelImpl::printVersionStrings]";

	// print compiler and version information
	IBK::Version::printCompilerVersion();
	IBK::IBK_Message("\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("NANDRAD solver version                           " + std::string(NANDRAD_MODEL::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("NANDRAD file format library version              " + std::string(NANDRAD::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("Material model and file format library version   " + std::string(MM::MM_LIB_VERSION()) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("IBK library version                              " + std::string(IBK::VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("DATAIO library version                           " + std::string(DATAIO::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	IBK::IBK_Message("CCM library version                              " + std::string(CCM::LONG_VERSION) + "\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
}


void NandradModel::createHeatingDesignDayCalculationProject()  const {
	const char *const FUNC_ID = "[NandradModel::createHeatingDesignDayCalculationProject]";
	// project for reading and writing

	try {
		NANDRAD::Project project;
		project.initDefaults(NULL);
		project.readXML(m_impl->m_projectFilePath);

		// manipulate start and end date
		IBK::Parameter endTimePara("EndTimeInSeconds", 3600.0 * 24.0 * 7.0, "s");
		// convert ro end unit

		std::string categoryTimePara = "Interval::para_t";
		std::string startParaName  = NANDRAD::KeywordList::Keyword(
										categoryTimePara.c_str(),
										NANDRAD::Interval::IP_START);
		std::string startParaUnit  = NANDRAD::KeywordList::Unit(
										categoryTimePara.c_str(),
										NANDRAD::Interval::IP_START);
		std::string endParaName  = NANDRAD::KeywordList::Keyword(
										categoryTimePara.c_str(),
										NANDRAD::Interval::IP_END);
		std::string endParaUnit  = NANDRAD::KeywordList::Unit(
										categoryTimePara.c_str(),
										NANDRAD::Interval::IP_END);

		project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START] =
			IBK::Parameter(startParaName, 0.0, startParaUnit);

		// convert to io-unit
		double endTime = endTimePara.get_value(endParaUnit);
		project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_END] =
			IBK::Parameter(endParaName, endTime, endParaUnit);

		// we now need the temperature of the constant design outside zone
		if(project.m_simulationParameter.m_para[NANDRAD::SimulationParameter::SP_HEATINGDESIGNAMBIENTTEMPERATURE].name.empty() )
		{
			throw IBK::Exception("Parameter 'HeatingDesignAmbientTemperature' is undefined inside SimulationParameter block!",
				FUNC_ID);
		}

		// create a constant ambient climate
		const double designAmbientTemperature = project.m_simulationParameter.m_para
			[NANDRAD::SimulationParameter::SP_HEATINGDESIGNAMBIENTTEMPERATURE]
			.get_value("C");

		CCM::ClimateDataLoader climateDataLoader;
		climateDataLoader.m_city = "DesignCity";
		climateDataLoader.m_country = "DesignCountry";
		// set two time points as constants
		climateDataLoader.m_dataTimePoints.push_back(0.);
		climateDataLoader.m_dataTimePoints.push_back(365. * 24. * 3600.);
		// set data: temperature
		climateDataLoader.m_data[CCM::ClimateDataLoader::Temperature].push_back(designAmbientTemperature);
		climateDataLoader.m_data[CCM::ClimateDataLoader::Temperature].push_back(designAmbientTemperature);
		// relative humidity
		climateDataLoader.m_data[CCM::ClimateDataLoader::RelativeHumidity].push_back(80.);
		climateDataLoader.m_data[CCM::ClimateDataLoader::RelativeHumidity].push_back(80.);
		// direct short wave radiation
		climateDataLoader.m_data[CCM::ClimateDataLoader::DirectRadiationNormal].push_back(0.0);
		climateDataLoader.m_data[CCM::ClimateDataLoader::DirectRadiationNormal].push_back(0.0);
		// diffuse short wave radiation
		climateDataLoader.m_data[CCM::ClimateDataLoader::DiffuseRadiationHorizontal].push_back(0.0);
		climateDataLoader.m_data[CCM::ClimateDataLoader::DiffuseRadiationHorizontal].push_back(0.0);
		// wind direction
		climateDataLoader.m_data[CCM::ClimateDataLoader::WindDirection].push_back(0.0);
		climateDataLoader.m_data[CCM::ClimateDataLoader::WindDirection].push_back(0.0);
		// wind velocity
		climateDataLoader.m_data[CCM::ClimateDataLoader::WindVelocity].push_back(0.0);
		climateDataLoader.m_data[CCM::ClimateDataLoader::WindVelocity].push_back(0.0);
		// long wave counter radiation
		climateDataLoader.m_data[CCM::ClimateDataLoader::LongWaveCounterRadiation].push_back(0.0);
		climateDataLoader.m_data[CCM::ClimateDataLoader::LongWaveCounterRadiation].push_back(0.0);
		// atmospheric pressure
		climateDataLoader.m_data[CCM::ClimateDataLoader::AirPressure].push_back(101325.);
		climateDataLoader.m_data[CCM::ClimateDataLoader::AirPressure].push_back(101325.);
		// rain flux density
		climateDataLoader.m_data[CCM::ClimateDataLoader::Rain].push_back(0.0);
		climateDataLoader.m_data[CCM::ClimateDataLoader::Rain].push_back(0.0);

		IBK::Path climateDir = m_impl->m_projectFilePath.parentPath() / IBK::Path("climate.designDay");

		if(!climateDir.exists() ) {
			if(!IBK::Path::makePath(climateDir) ) {
				throw IBK::Exception( IBK::FormatString("Cannot create climate directory '%1' for design day calculation "
					"from project file '%2'.")
					.arg(climateDir.str())
					.arg(m_impl->m_projectFilePath), FUNC_ID);
			}
		}
		// create a new project place holder for climate
		IBK::Path climateFile = climateDir / IBK::Path("HeatingDesignDay.c6b");
		IBK::Path absClimateFile = climateFile.withReplacedPlaceholders(project.m_placeholders);
		IBK_ASSERT(absClimateFile.parentPath().exists());
		IBK_ASSERT(absClimateFile.exists());
		climateDataLoader.writeClimateDataIBK(absClimateFile);

		// and add a reference inside location object
		climateFile.insertPlaceholder(IBK::PLACEHOLDER_CLIMATE_DIR, climateDir);
		project.m_location.m_climateFileName = climateFile.str();
		project.m_location.m_climateFileDisplayName = climateFile.filename().str();

		// store old placeholder map
		std::map<std::string, IBK::Path> oldPlaceHolderMap = project.m_placeholders;
		// change placeholder for climate directory
		project.m_placeholders[IBK::PLACEHOLDER_CLIMATE_DIR] = climateDir;
		// now set climate dir to climate placeholder
		climateDir = climateFile.parentPath();

		// check if we need to copy a file for shading data
		if (!project.m_location.m_shadingFactorFileName.str().empty()) {

			// copy into new climate file
			IBK::Path shadingFactorBaseFile(project.m_location.m_shadingFactorFileName);
			shadingFactorBaseFile = shadingFactorBaseFile.withReplacedPlaceholders(oldPlaceHolderMap);

			if (!IBK::Path::copy(shadingFactorBaseFile,
				climateDir.withReplacedPlaceholders(project.m_placeholders) ) ) {
				throw IBK::Exception(IBK::FormatString(
					"Cannot copy file '%1!")
					.arg(shadingFactorBaseFile), FUNC_ID);
			}
			// create name including placeholders
			project.m_location.m_shadingFactorFileName = (climateDir
				/ shadingFactorBaseFile.filename()).c_str();
		}


		// check if we need to copy file for ground climate data
		for (std::map<unsigned int, NANDRAD::Zone>::iterator zoneIt =
			project.m_zones.begin(); zoneIt != project.m_zones.end(); ++zoneIt) {

			// ground zone enforced climate data copy
			if (zoneIt->second.m_zoneType == NANDRAD::Zone::ZT_GROUND) {

				IBK::Path climateBaseDir(zoneIt->second.m_climateFileName);
				climateBaseDir = climateBaseDir.withReplacedPlaceholders(oldPlaceHolderMap);
				// error: file was copied already -> more than one ground zone defined
				IBK_ASSERT(!IBK::Path(climateDir / "Temperature.ccd").exists());

				// copy into new climate file

				if (!IBK::Path::copy((climateBaseDir / "Temperature.ccd"),
					climateDir.withReplacedPlaceholders(project.m_placeholders))) {
					throw IBK::Exception(IBK::FormatString(
						"Missing file file 'Temperature.ccd' inside climate path '%1' "
						"of zone with id #%1!")
						.arg(climateBaseDir)
						.arg(zoneIt->first), FUNC_ID);
				}
				// isnert placeholder for climate dir
				zoneIt->second.m_climateFileName = climateDir;
			}
		}


		std::string categoryHeatingStringPara = "IdealHeatingModel::StringParameters";
		std::string categoryControlMode = "IdealHeatingModel::HeatingControlMode";
		std::string categoryHeatingDesignPara = "IdealHeatingModel::HeatingDesignParameters";
		std::string categoryHeatingPara = "IdealHeatingModel::InputReferences";
		std::string categoryInfiltrationPara = "InfiltrationModel::InputReferences";
		std::string categoryInfiltrationResults = "InfiltrationModel::VectorValuedResults";

		std::map<std::string, IBK::Parameter> spaceTypesInfiltrationAirChangeRate;
		std::map<std::string, IBK::Parameter> spaceTypesSetPointTemperature;
		// find ideal heated space types
		for(unsigned int i = 0; i < project.m_spaceTypes.m_spaceTypes.size(); ++i) {

			NANDRAD::SpaceType &spaceType = project.m_spaceTypes.m_spaceTypes[i];

			// store infiltration air change rate for all space types
			std::map<std::string, IBK::Parameter>::iterator fIt =
				spaceType.m_genericParaConst.find(KeywordList::Keyword(categoryHeatingDesignPara.c_str(),
				IdealHeatingModel::HDP_HeatingDesignInfiltrationAirChangeRate) );
			// store parameter
			if(fIt != spaceType.m_genericParaConst.end() ) {
				// store space type and ventilation air change rate
				spaceTypesInfiltrationAirChangeRate[spaceType.m_name] = fIt->second;
			}

			/// set teh correct heating control mode
			IdealHeatingModel::HeatingControlMode heatingControlMode =
				IdealHeatingModel::HCM_None;

			// find parameter heating mode
			std::map<std::string, std::string>::const_iterator
				sIt = spaceType.m_genericParaString.find(KeywordList::Keyword(categoryHeatingStringPara.c_str(),
						IdealHeatingModel::SP_HeatingControlMode) );

			if(sIt != spaceType.m_genericParaString.end() )
				heatingControlMode = (IdealHeatingModel::HeatingControlMode)
						KeywordList::Enumeration(categoryControlMode.c_str(), sIt->second );

			// we only store heating control mode
			std::map<std::string, std::string> newStringPara;

			// for all heated space types aenforce the exitsnce of the space type
			// parameters 'HeatingDesignSetpointTemperature' and
			// 'HeatingDesignInfiltrationAirChangeRate'
			// find parameter heating mode
			fIt = spaceType.m_genericParaConst.find(KeywordList::Keyword(categoryHeatingDesignPara.c_str(),
				IdealHeatingModel::HDP_HeatingDesignSetpointTemperature) );
			// error: parameter does not exist
			if(fIt == spaceType.m_genericParaConst.end() ) {
				continue;
			}
			// store space type and temperature
			spaceTypesSetPointTemperature[spaceType.m_name] = fIt->second;


			// activate ideal heating model for current space type
			if(heatingControlMode == IdealHeatingModel::HCM_None) {
				newStringPara[KeywordList::Keyword(categoryHeatingStringPara.c_str(),
							IdealHeatingModel::SP_HeatingControlMode)] =
							KeywordList::Keyword(categoryControlMode.c_str(),
							IdealHeatingModel::HCM_AirTemperature);
			}
			else {
				newStringPara[KeywordList::Keyword(categoryHeatingStringPara.c_str(),
							IdealHeatingModel::SP_HeatingControlMode)] =
							KeywordList::Keyword(categoryControlMode.c_str(), heatingControlMode);
			}
			// clear all parameters to surpress user and equipment loads calculation
			spaceType.m_genericParaConst.clear();
			spaceType.m_genericParaFlags.clear();
			spaceType.m_genericParaString = newStringPara;
		}

		// manipulate schedules: use a constant AllDays heating control temperatutre and ventilation air change rate
		std::string heatingSetpointParaName = KeywordList::Keyword(
			categoryHeatingPara.c_str(), IdealHeatingModel::InputRef_HeatingSetPointTemperature);
		std::string heatingSetpointParaUnit = KeywordList::Unit(
			categoryHeatingPara.c_str(), IdealHeatingModel::InputRef_HeatingSetPointTemperature);

		std::string infAirChangeRateParaName = KeywordList::Keyword(
			categoryInfiltrationPara.c_str(), InfiltrationModel::InputRef_InfiltrationAirChangeRate);
		std::string infAirChangeRateParaUnit = KeywordList::Unit(
			categoryInfiltrationPara.c_str(), InfiltrationModel::InputRef_InfiltrationAirChangeRate);

		// construct a basic schedule group
		NANDRAD::ScheduleGroup basicScheduleGroup;
		// fill a schedule for all  space type groups
		for(unsigned int i = 0; i < project.m_spaceTypes.m_spaceTypes.size(); ++i) {

			const std::string spaceTypeName = project.m_spaceTypes.m_spaceTypes[i].m_name;

			std::map<std::string, IBK::Parameter>::iterator setPointIt
				= spaceTypesSetPointTemperature.find(spaceTypeName);
			std::map<std::string, IBK::Parameter>::iterator infAirChangeRateIt
				= spaceTypesInfiltrationAirChangeRate.find(spaceTypeName);

			// design a suitable daily cycle: set an infiltration air change rate
			NANDRAD::Interval constantDesignDayInterval;
			// we have a zone with indiltration: add infiltration air change rate as schedule parameter
			if(infAirChangeRateIt != spaceTypesInfiltrationAirChangeRate.end()) {
				double designInfiltrationAirChangeRate = infAirChangeRateIt->second.
					get_value(infAirChangeRateParaUnit);
				// set a constant air change rate for all infilötrated zones
				constantDesignDayInterval.m_genericParaConst[infAirChangeRateParaName]
				= IBK::Parameter(infAirChangeRateParaName,designInfiltrationAirChangeRate,infAirChangeRateParaUnit);
			}
			// we have a heated zone: add setpoint temperature as schedule parameter
			if(setPointIt != spaceTypesSetPointTemperature.end()) {
				double designSetpointTemperature = setPointIt->second
					.get_value(heatingSetpointParaUnit);
				// set a constant temperature for all heated zones
				constantDesignDayInterval.m_genericParaConst[heatingSetpointParaName]
					= IBK::Parameter(heatingSetpointParaName,designSetpointTemperature,heatingSetpointParaUnit);
			}
			// skip empty intervals
			if(constantDesignDayInterval.m_genericParaConst.empty())
				continue;

			// don't set end parameter
			NANDRAD::DailyCycle constantDailyCycle;
			constantDailyCycle.m_intervals.push_back(constantDesignDayInterval);

			// only use a dailly schedule
			NANDRAD::Schedule alldaysSchedule;
			alldaysSchedule.m_type = NANDRAD::Schedule::ST_ALLDAYS;
			alldaysSchedule.m_dailyCycles.push_back(constantDailyCycle);

			// fill with the previously composed schedule
			basicScheduleGroup.m_spaceTypeGroups[spaceTypeName][NANDRAD::Schedule::ST_ALLDAYS]
			= alldaysSchedule;
		}

		// manipulate all schedules
		project.m_schedulesReference.clear();
		project.m_schedules.m_scheduleGroups.clear();
		project.m_schedules.m_annualSchedules.m_parameters.clear();
		project.m_schedules.m_scheduleGroups.push_back(basicScheduleGroup);

		// delete all object lists
		project.m_objectLists.m_objectLists.clear();

		// compose outputs for all active zones
		NANDRAD::ObjectList zonesObjectList;
		zonesObjectList.m_filterType = NANDRAD::ModelInputReference::MRT_ZONE;
		zonesObjectList.m_filterID.m_allIDs = true;
		zonesObjectList.m_name = "Active zones";

		// compose object list for infiltrated zones
		NANDRAD::ObjectList infiltrationModelObjectList;
		infiltrationModelObjectList.m_filterType = NANDRAD::ModelInputReference::MRT_ZONE;
		for(std::map<std::string, IBK::Parameter>::const_iterator it
					= spaceTypesInfiltrationAirChangeRate.begin();
				it != spaceTypesInfiltrationAirChangeRate.end(); ++it)
		{
			infiltrationModelObjectList.m_filterSpaceType.push_back(it->first);
		}
		infiltrationModelObjectList.m_name = "Infiltrated zones";

		// compose an object list fornthe constant outside zone
		NANDRAD::ObjectList outsideZoneObjectList;
		outsideZoneObjectList.m_filterType = NANDRAD::ModelInputReference::MRT_LOCATION;
		outsideZoneObjectList.m_name = "Outside zone";

		// add to object list definition
		project.m_objectLists.m_objectLists[zonesObjectList.m_name]				= zonesObjectList;
		project.m_objectLists.m_objectLists[infiltrationModelObjectList.m_name]	= infiltrationModelObjectList;
		project.m_objectLists.m_objectLists[outsideZoneObjectList.m_name]		= outsideZoneObjectList;

		unsigned int maxModelId = 0;
		for(unsigned int i = 0; i < project.m_models.m_models.size(); ++i)
		{
			maxModelId = std::max( maxModelId, project.m_models.m_models[i].m_id);
		}
		// delete all generic models
		project.m_models.m_models.clear();
		project.m_modelGroups.m_modelGroups.clear();

		// add infiltration model
		NANDRAD::Model infiltrationModel;
		infiltrationModel.m_modelIdName = "InfiltrationModel";
		infiltrationModel.m_id = maxModelId + 1;
		infiltrationModel.m_displayName = infiltrationModel.m_modelIdName;
		// construct a feedback to all infiltrated zones
		NANDRAD::ImplicitModelFeedback modelFeedback;
		modelFeedback.m_objectList = infiltrationModelObjectList.m_name;
		modelFeedback.m_quantity = KeywordList::Keyword(categoryInfiltrationResults.c_str(),
			InfiltrationModel::VVR_InfiltrationBalanceFlux);
		modelFeedback.m_targetName = modelFeedback.m_quantity;
		modelFeedback.m_operation = NANDRAD::ImplicitModelFeedback::IFO_ADD;

		infiltrationModel.m_implicitModelFeedbacks.push_back(modelFeedback);

		// add to models section
		project.m_models.m_models.push_back(infiltrationModel);

		// delete all outputs
		project.m_outputs.m_outputDefinitions.clear();
		project.m_outputsReference.clear();

		// compose outputs for all quantities of interest: room air temperature,
		// ideal heating convective loads, set point temperature, constant ouside temperature,
		// zone volume: interval one day
		NANDRAD::OutputGrid grid;
		grid.m_name = "Weekly";

		std::string stepSizeParaName = NANDRAD::KeywordList::Keyword(categoryTimePara.c_str(),
			NANDRAD::Interval::IP_STEPSIZE);

		NANDRAD::Interval interval;
		interval.m_genericParaConst[startParaName]		= IBK::Parameter(startParaName, 0.0, startParaUnit);
		interval.m_genericParaConst[endParaName]		= IBK::Parameter(endParaName, endTime, endParaUnit);
		interval.m_genericParaConst[stepSizeParaName]	= IBK::Parameter(stepSizeParaName, endTime, endParaUnit);
		// add to grid definition
		grid.m_intervals.push_back(interval);

		// add to output definition
		project.m_outputs.m_grids.push_back(grid);

		// compose output definitions
		NANDRAD::OutputDefinition outputDef;
		outputDef.m_gridName = grid.m_name;
		outputDef.m_objectListName = zonesObjectList.m_name;
		outputDef.m_quantity = KeywordList::Keyword("RoomStatesModel::Results", RoomStatesModel::R_AirTemperature);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("UserModel::InputReferences", UserModel::InputRef_Area);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("IdealHeatingModel::InputReferences",
			IdealHeatingModel::InputRef_HeatingSetPointTemperature);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("HeatingsLoadModel::Results",
			HeatingsLoadModel::R_HeatingsLoad);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("NaturalVentilationLoadModel::Results",
			NaturalVentilationLoadModel::R_InfiltrationThermalLoad);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_objectListName = outsideZoneObjectList.m_name;
		outputDef.m_quantity = KeywordList::Keyword("Loads::Results",
			Loads::R_Temperature);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		// now write project to file
		IBK::Path targetFile = dirs().m_heatingDesignRootDir;
		targetFile.addExtension("nandrad");
		project.writeXML(targetFile);

	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error creating file for heating design calculation from project file %1!")
			.arg(m_impl->m_projectFilePath.str()), FUNC_ID);
	}
}


void NandradModel::createCoolingDesignDayCalculationProject()  const {
	const char *const FUNC_ID = "[NandradModel::createCoolingDesignDayCalculationProject]";
	// project for reading and writing

	try {
		NANDRAD::Project project;
		project.initDefaults(NULL);
		project.readXML(m_impl->m_projectFilePath);

		// manipulate start and end date: cyclic simulation (2 weeks)
		IBK::Parameter endTimePara("EndTimeInSeconds", 3600.0 * 24.0 * 7.0 * 2.0, "s");
		// convert ro end unit

		std::string categoryTimePara = "Interval::para_t";
		std::string startParaName  = NANDRAD::KeywordList::Keyword(
										categoryTimePara.c_str(),
										NANDRAD::Interval::IP_START);
		std::string startParaUnit  = NANDRAD::KeywordList::Unit(
										categoryTimePara.c_str(),
										NANDRAD::Interval::IP_START);
		std::string endParaName  = NANDRAD::KeywordList::Keyword(
										categoryTimePara.c_str(),
										NANDRAD::Interval::IP_END);
		std::string endParaUnit  = NANDRAD::KeywordList::Unit(
										categoryTimePara.c_str(),
										NANDRAD::Interval::IP_END);

		project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_START] =
			IBK::Parameter(startParaName, 0.0, startParaUnit);

		// convert to io-unit
		double endTime = endTimePara.get_value(endParaUnit);
		project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::IP_END] =
			IBK::Parameter(endParaName, endTime, endParaUnit);

		// we now need the design climate file
		IBK::Path designClimateDataFile =
			IBK::Path(project.m_simulationParameter.m_stringPara[NANDRAD::SimulationParameter::SSP_COOLINGDESIGNCLIMATEDATAFILE]);
		// no simulation parameter was defined
		if(designClimateDataFile.str().empty() )
		{
			designClimateDataFile = IBK::Path(project.m_location.m_climateFileName)
									.withReplacedPlaceholders(project.m_placeholders);
			IBK::IBK_Message(IBK::FormatString("SimulationParameter  'CoolingDesignClimateDataFile' is undefined. "
				"We use the simulation climate data file '%1' instead.")
				.arg(designClimateDataFile.str()),
				IBK::MSG_WARNING,
				FUNC_ID);
		}
		else if(!designClimateDataFile.exists()) {
			throw IBK::Exception(IBK::FormatString("Requested design day climate data file %1 is undefined!")
				.arg(designClimateDataFile.str()), FUNC_ID);
		}

		// load data file
		CCM::ClimateDataLoader climateDataLoaderOrig;

		// try to read climate data file
		try {
			climateDataLoaderOrig.readClimateData(designClimateDataFile);
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading climate data from file '%1")
				.arg(designClimateDataFile.str() ),FUNC_ID);
		}

		// create schedule days for week day and weekend check
		ScheduleDays scheduleDays;
		scheduleDays.init(project.m_schedules, project.m_simulationParameter);
		// store year
		unsigned int year = project.m_simulationParameter.m_intpara[NANDRAD::SimulationParameter::SIP_YEAR].value;
		// find all mondays
		std::set<unsigned int> mondays = scheduleDays.decodeScheduleType(NANDRAD::Schedule::ST_MONDAY);
		double maxTemperature = 0.0;
		unsigned int maxTemperatureWeekMonday = 0;

		// find pout hotest week
		for(std::set<unsigned int>::const_iterator it = mondays.begin();
						it != mondays.end(); ++it) {

			double averageTemp = 0.0;
			for(unsigned int i = 0; i < 7; ++i) {
				unsigned int day = *it + i;
				// correct day
				if(day >= 365) {
					day -= 365;
				}
				// calculate time point
				double t = (double) day * 24. * 3600.;
				// sum up temperature for each hour
				for(unsigned int j = 0; j < 24; ++j) {
					climateDataLoaderOrig.setTime(year,t);
					averageTemp += climateDataLoaderOrig.m_currentData[CCM::ClimateDataLoader::Temperature];
					t += 3600.;
				}
			}
			// calculate average
			averageTemp /= (24. * 7.);
			if(averageTemp > maxTemperature) {
				maxTemperatureWeekMonday = *it;
				maxTemperature = averageTemp;
			}
		}

		// compose cyclic climate for maximum teperature week
		std::vector<double>	cyclicClimateData[CCM::ClimateDataLoader::NumClimateComponents];

		double meanOutsideTemperature = 0.0;
		// find out first day of cyclic climate
		unsigned int startReferenceDay = 0;
		// find first day >= maxTemperatureWeekMonday
		while(startReferenceDay < maxTemperatureWeekMonday)
			startReferenceDay += 7;
		unsigned int maxReferenceDay = maxTemperatureWeekMonday + 6;

		unsigned int referenceDay = startReferenceDay;
		for(unsigned int i = 0; i < 7; ++i) {
			// calculate time point
			double tReference	= (double) referenceDay * 24. * 3600.;
			// sum up temperature for each hour
			for(unsigned int j = 0; j < 24; ++j) {
				climateDataLoaderOrig.setTime(year,tReference);
				// copy values
				for(unsigned int k = 0; k < CCM::ClimateDataLoader::NumClimateComponents; ++k) {
					cyclicClimateData[k].push_back(climateDataLoaderOrig.m_currentData[k]);
				}
				meanOutsideTemperature += climateDataLoaderOrig.
											m_currentData[CCM::ClimateDataLoader::Temperature];
				tReference += 3600.;
			}
			++referenceDay;
			if(referenceDay > maxReferenceDay)
				referenceDay -= 7;
			if(referenceDay == 365)
				referenceDay = 0;
		}
		meanOutsideTemperature/= (24. * 7.);

		// store week with maximum temperature inside cliamte data loader file
		CCM::ClimateDataLoader climateDataLoaderNew;
		climateDataLoaderNew.m_city = "DesignCity";
		climateDataLoaderNew.m_country = "DesignCountry";

		// fill complete year with weekly data
		unsigned int currentDay = 0;

		while(currentDay < 365) {

			for(unsigned int i = 0; i < 7; ++i) {
				// calculate time point
				double t			= (double) currentDay * 24. * 3600.;
				// sum up temperature for each hour
				for(unsigned int j = 0; j < 24; ++j) {
					// store time point
					climateDataLoaderNew.m_dataTimePoints.push_back(t);
					t += 3600.;
				}
				// copy values
				for(unsigned int j = 0; j < CCM::ClimateDataLoader::NumClimateComponents; ++j) {
					climateDataLoaderNew.m_data[j].insert(climateDataLoaderNew.m_data[j].end(),
						cyclicClimateData[j].begin() + i * 24, cyclicClimateData[j].begin() + (i + 1) * 24);
				}
				++currentDay;
				// last time point
				if(currentDay == 365) {
					unsigned int referenceDay = currentDay % 7;
					unsigned int dataIdx = referenceDay * 24;
					t	= (double) currentDay * 24. * 3600.;
					// copy data
					climateDataLoaderNew.m_dataTimePoints.push_back(t);
					// copy values
					for(unsigned int j = 0; j < CCM::ClimateDataLoader::NumClimateComponents; ++j) {
						climateDataLoaderNew.m_data[j].push_back(cyclicClimateData[j][dataIdx]);
					}
					break;
				}
			}
		}

		// for now, try to read each supported ccd file individually
		IBK::Path climateDir = m_impl->m_projectFilePath.parentPath() / IBK::Path("climate.designDay");

		if(!climateDir.exists() ) {
			if(!IBK::Path::makePath(climateDir) ) {
				throw IBK::Exception( IBK::FormatString("Cannot create climate directory '%1' for design day calculation "
					"from project file '%2'.")
					.arg(climateDir.str())
					.arg(m_impl->m_projectFilePath), FUNC_ID);
			}
		}

		// set project directory place holder
		climateDir.insertPlaceholder(IBK::PLACEHOLDER_PROJECT_DIR, m_impl->m_projectFilePath.parentPath());

		IBK::Path climateFile = climateDir / IBK::Path("CoolingDesignDay.c6b");
		IBK::Path absClimateFile = climateFile.withReplacedPlaceholders(project.m_placeholders);
		IBK_ASSERT(absClimateFile.parentPath().exists()) ;

		climateDataLoaderNew.writeClimateDataIBK(absClimateFile);

		// and add a reference inside location object
		climateFile.insertPlaceholder(IBK::PLACEHOLDER_CLIMATE_DIR, climateDir);
		project.m_location.m_climateFileName = climateFile.str();
		project.m_location.m_climateFileDisplayName = climateFile.filename().str();

		// store old placeholder map
		std::map<std::string, IBK::Path> oldPlaceHolderMap = project.m_placeholders;
		// change placeholder for climate directory
		project.m_placeholders[IBK::PLACEHOLDER_CLIMATE_DIR] = climateDir;
		// now set climate dir to climate placeholder
		climateDir = climateFile.parentPath();

		// check if we need to copy a file for shading data
		if (!project.m_location.m_shadingFactorFileName.str().empty()) {

			// copy into new climate file
			IBK::Path shadingFactorBaseFile(project.m_location.m_shadingFactorFileName);
			shadingFactorBaseFile = shadingFactorBaseFile.withReplacedPlaceholders(oldPlaceHolderMap);

			if (!IBK::Path::copy(shadingFactorBaseFile,
				climateDir.withReplacedPlaceholders(project.m_placeholders))) {
				throw IBK::Exception(IBK::FormatString(
					"Cannot copy file '%1!")
					.arg(shadingFactorBaseFile), FUNC_ID);
			}
			// create name including placeholders
			project.m_location.m_shadingFactorFileName = (climateDir
				/ shadingFactorBaseFile.filename()).c_str();
		}


		// check if we need to copy file for ground climate data
		for (std::map<unsigned int, NANDRAD::Zone>::iterator zoneIt =
			project.m_zones.begin(); zoneIt != project.m_zones.end(); ++zoneIt) {

			// ground zone enforced climate data copy
			if (zoneIt->second.m_zoneType == NANDRAD::Zone::ZT_GROUND) {

				IBK::Path climateBaseDir(zoneIt->second.m_climateFileName);
				climateBaseDir = climateBaseDir.withReplacedPlaceholders(oldPlaceHolderMap);
				// error: file was copied already -> more than one ground zone defined
				IBK_ASSERT(!IBK::Path(climateDir / "Temperature.ccd").exists());

				// copy into new climate file

				if (!IBK::Path::copy((climateBaseDir / "Temperature.ccd"),
					climateDir.withReplacedPlaceholders(project.m_placeholders))) {
					throw IBK::Exception(IBK::FormatString(
						"Missing file file 'Temperature.ccd' inside climate path '%1' "
						"of zone with id #%1!")
						.arg(climateBaseDir)
						.arg(zoneIt->first), FUNC_ID);
				}
				// isnert placeholder for climate dir
				zoneIt->second.m_climateFileName = climateDir;
			}
		}

		// find out all cooled zones
		std::string categoryCoolingStringPara = "IdealCoolingModel::StringParameters";
		std::string categoryControlMode		= "IdealCoolingModel::CoolingControlMode";
		std::string categoryCoolingDesignPara = "IdealCoolingModel::CoolingDesignParameters";
		std::string categoryCoolingPara = "IdealCoolingModel::InputReferences";
		std::string categoryUserPara	= "UserModel::InputReferences";
		std::string categoryEquipmentPara = "ElectricEquipmentModel::InputReferences";
		std::string categoryLightingPara = "LightingModel::InputReferences";

		std::map<std::string, IBK::Parameter> spaceTypesSetPointTemperature;
		// find ideal cooled space types
		for(unsigned int i = 0; i < project.m_spaceTypes.m_spaceTypes.size(); ++i) {

			NANDRAD::SpaceType &spaceType = project.m_spaceTypes.m_spaceTypes[i];

			/// set teh correct heating control mode
			IdealCoolingModel::CoolingControlMode coolingControlMode =
				IdealCoolingModel::CCM_None;

			// find parameter heating mode
			std::map<std::string, std::string>::const_iterator sIt =
				spaceType.m_genericParaString.find(KeywordList::Keyword(categoryCoolingStringPara.c_str(),
				IdealCoolingModel::SP_CoolingControlMode) );

			if(sIt != spaceType.m_genericParaString.end() )
				coolingControlMode = (IdealCoolingModel::CoolingControlMode)
						KeywordList::Enumeration(categoryControlMode.c_str(), sIt->second );

			// for all cooled space types aenforce the exitsnce of the space type
			// parameter 'CoolingDesignSetpointTemperature'
			// find parameter cooling control mode
			std::map<std::string, IBK::Parameter>::iterator fIt = spaceType.
				m_genericParaConst.find(KeywordList::Keyword(categoryCoolingDesignPara.c_str(),
				IdealCoolingModel::CDP_CoolingDesignSetpointTemperature) );
			// error: parameter does not exist
			if(fIt == spaceType.m_genericParaConst.end() ) {
				continue;
			}
			// store space type and temperature
			spaceTypesSetPointTemperature[spaceType.m_name] = fIt->second;


			// we only store cooling control mode ...
			std::map<std::string, std::string> newStringPara;
			// ... and parameter related to occupancy and electric loads
			std::map<std::string, IBK::Parameter> newConstantPara;

			// activate ideal cooling model for current space type
			if(coolingControlMode == IdealCoolingModel::CCM_None) {
				newStringPara[KeywordList::Keyword(categoryCoolingStringPara.c_str(),
							IdealCoolingModel::SP_CoolingControlMode)] =
							KeywordList::Keyword(categoryControlMode.c_str(),
							IdealCoolingModel::CCM_AirTemperature);
			}
			else {
				newStringPara[KeywordList::Keyword(categoryCoolingStringPara.c_str(),
							IdealCoolingModel::SP_CoolingControlMode)] =
							KeywordList::Keyword(categoryControlMode.c_str(), coolingControlMode);
			}
			// renew all string parameters
			spaceType.m_genericParaString = newStringPara;

			// find out which parameters to copy
			for(std::map<std::string, IBK::Parameter>::const_iterator it =
				spaceType.m_genericParaConst.begin(); it !=
				spaceType.m_genericParaConst.end(); ++it) {

				// filter all needed references to internal load models
				if(KeywordList::KeywordExists(categoryCoolingPara.c_str(), it->first) ||
				   KeywordList::KeywordExists(categoryUserPara.c_str(), it->first) ||
				   KeywordList::KeywordExists(categoryEquipmentPara.c_str(), it->first) ||
				   KeywordList::KeywordExists(categoryLightingPara.c_str(), it->first) ) {

				   newConstantPara[it->first] = it->second;
				}
			}

			// renew all constant parameters
			spaceType.m_genericParaConst = newConstantPara;
		}

		// design a suitable daily cycle for each week day
		std::map<NANDRAD::Schedule::type_t, NANDRAD::Schedule> weeklySchedules;

		// now add cooling setpoint temperature = minimum outside temperature + 5 K for each
		// hour of day
		std::string categoryIntvalPara = "Interval::para_t";
		unsigned int weekDay = maxTemperatureWeekMonday % 7;

		for(unsigned int i = 0; i < 7; ++i) {
			// fill a daily cycle
			NANDRAD::DailyCycle designDayDailyCycle;

			for(unsigned int j = 0; j < 24; ++j ) {
				NANDRAD::Interval designDayInterval;
				// set interval end
				if(j < 23) {
					std::string endParaName = NANDRAD::KeywordList::Keyword(categoryIntvalPara.c_str(),
						NANDRAD::Interval::IP_END);

					designDayInterval.m_para[NANDRAD::Interval::IP_END]
						= IBK::Parameter(endParaName, j + 1, "h");
				}
				// copy hourly set cooling setpoint temperature values
				std::string setPointParaName = KeywordList::Keyword(categoryCoolingPara.c_str(),
					IdealCoolingModel::InputRef_CoolingSetPointTemperature);
				double setPointValue         = cyclicClimateData[CCM::ClimateDataLoader::Temperature][i*24 + j];
				setPointValue               -= 5.;

				designDayInterval.m_genericParaConst[setPointParaName]
					= IBK::Parameter(setPointParaName, setPointValue, "C");
				// add to daily cycle
				designDayDailyCycle.m_intervals.push_back(designDayInterval);
			}
			// find a correcponding schedule
			NANDRAD::Schedule designDaySchedule;
			// get type
			NANDRAD::Schedule::type_t scheduleType = (NANDRAD::Schedule::type_t)
				(NANDRAD::Schedule::ST_MONDAY + weekDay);
			designDaySchedule.m_type = scheduleType;
			designDaySchedule.m_dailyCycles.push_back(designDayDailyCycle);

			weeklySchedules[scheduleType] = designDaySchedule;
			// go further
			++weekDay;
			if(weekDay >= 7)
				weekDay -= 7;
		}

		// define a basic schedule group
		NANDRAD::ScheduleGroup basicScheduleGroup;
		for(std::map<std::string, IBK::Parameter>::const_iterator
			setPointIt = spaceTypesSetPointTemperature.begin();
			setPointIt != spaceTypesSetPointTemperature.end();
			++setPointIt) {

			std::string spaceTypeName = setPointIt->first;

			NANDRAD::ScheduleGroup::ScheduleMap designDaySpaceTypeGroup;
			// get maximum temperature
			double maxSetpointTemperature = setPointIt->second.value;
			// add the set point schedule groups
			for(std::map<NANDRAD::Schedule::type_t, NANDRAD::Schedule> ::iterator
				setPointScheduleIt = weeklySchedules.begin();
				setPointScheduleIt != weeklySchedules.end(); ++setPointScheduleIt) {

				NANDRAD::Schedule::type_t scheduleType = setPointScheduleIt->first;
				NANDRAD::Schedule designDaySchedule;
				designDaySchedule.m_type = scheduleType;
				// add all daily cycles for the description of cooling setpoint temperature
				for(unsigned int cycle = 0; cycle < setPointScheduleIt->second.m_dailyCycles.size();
					++cycle) {
					const NANDRAD::DailyCycle &setPointDailyCycle =
						setPointScheduleIt->second.m_dailyCycles[cycle];
					NANDRAD::DailyCycle designDayDailyCycle;
					// set amximum between ambient temperature adapted value and given set point
					for(unsigned int intval = 0; intval < setPointDailyCycle.m_intervals.size();
							++intval) {
						const NANDRAD::Interval &interval = setPointDailyCycle.m_intervals[intval];
						IBK_ASSERT(interval.m_genericParaConst.size() == 1);
						// construct a new interval
						NANDRAD::Interval designDayInterval = interval;
						designDayInterval.m_genericParaConst.begin()->second.value =
							std::max(interval.m_genericParaConst.begin()->second.value,
							maxSetpointTemperature);
						designDayDailyCycle.m_intervals.push_back(designDayInterval);
					}
					designDaySchedule.m_dailyCycles.push_back(designDayDailyCycle);
				}
				designDaySpaceTypeGroup[scheduleType] = designDaySchedule;
			}
			basicScheduleGroup.m_spaceTypeGroups[spaceTypeName] = designDaySpaceTypeGroup;
		}

		// compose a new set of schedule groups
		std::vector<NANDRAD::ScheduleGroup> designDayScheduleGroups;
		// construct schedules for cooling design day
		for(unsigned int group = 0; group !=  project.m_schedules.m_scheduleGroups.size(); ++group) {

			const NANDRAD::ScheduleGroup &scheduleGroup = project.m_schedules.m_scheduleGroups[group];
			NANDRAD::ScheduleGroup designDayScheduleGroup;

			// fill a schedule for all  space type groups
			for(std::map<std::string,  NANDRAD::ScheduleGroup::ScheduleMap>::const_iterator
				spaceTypeGroupIt = scheduleGroup.m_spaceTypeGroups.begin();
				spaceTypeGroupIt != scheduleGroup.m_spaceTypeGroups.end();
				++spaceTypeGroupIt) {

				const std::string spaceTypeName = spaceTypeGroupIt->first;

				// search for suitable category
				NANDRAD::ScheduleGroup::ScheduleMap designDaySpaceTypeGroup;

				for(NANDRAD::ScheduleGroup::ScheduleMap::const_iterator
					scheduleIt = spaceTypeGroupIt->second.begin();
					scheduleIt != spaceTypeGroupIt->second.end(); ++scheduleIt) {

					NANDRAD::Schedule::type_t scheduleType = scheduleIt->first;
					// compose a design day schedule of the same schedule type
					NANDRAD::Schedule designDaySchedule;
					designDaySchedule.m_type = scheduleType;
					// for each schedule check all intervals for suitable parameters
					for(unsigned int j = 0; j < scheduleIt->second.m_dailyCycles.size(); ++j) {
						const NANDRAD::DailyCycle &dailyCycle = scheduleIt->second.m_dailyCycles[j];

						NANDRAD::DailyCycle designDayDailyCycle;
						for(unsigned int k = 0; k < dailyCycle.m_intervals.size(); ++k) {

							const NANDRAD::Interval &intval = dailyCycle.m_intervals[k];
							NANDRAD::Interval designDayInterval;

							// copy predefined parameter
							for(unsigned int p = 0; p < NANDRAD::Interval::NUM_IP; ++p)
								designDayInterval.m_para[p] = intval.m_para[p];

							for(std::map<std::string, IBK::Parameter>::const_iterator
								paraIt = intval.m_genericParaConst.begin();
								paraIt != intval.m_genericParaConst.end(); ++paraIt) {

								// filter all needed references to internal load models
								if(KeywordList::KeywordExists(categoryUserPara.c_str(), paraIt->first) ||
									KeywordList::KeywordExists(categoryEquipmentPara.c_str(), paraIt->first) ||
									KeywordList::KeywordExists(categoryLightingPara.c_str(), paraIt->first) ) {

										designDayInterval.m_genericParaConst[paraIt->first] = paraIt->second;
								}
							}
							// store non-empty intervals
							if(!designDayInterval.m_genericParaConst.empty() ) {
								designDayDailyCycle.m_intervals.push_back(designDayInterval);
							}
						}
						// store schedule
						if(!designDayDailyCycle.m_intervals.empty() ) {
							designDaySchedule.m_dailyCycles.push_back(designDayDailyCycle);
						}
					}
					// store schedule
					if(!designDaySchedule.m_dailyCycles.empty() ) {
						designDaySpaceTypeGroup[scheduleType] = designDaySchedule;
					}
				}

				// skip empty groups
				// store space type group
				if(designDaySpaceTypeGroup.empty() )
					continue;

				designDayScheduleGroup.m_spaceTypeGroups[spaceTypeName] = designDaySpaceTypeGroup;
				// special treatment for basic schedule group
				if(group == 0) {
					// decide were to insert schedule
					std::map<std::string, NANDRAD::ScheduleGroup::ScheduleMap>::iterator basicSpaceTypeGroupIt  =
						basicScheduleGroup.m_spaceTypeGroups.find(spaceTypeName);
					// we merge all currently defined space type groups into basic group
					if(basicSpaceTypeGroupIt != basicScheduleGroup.m_spaceTypeGroups.end() ) {
						//check which schedule is already defined
						for(std::map<NANDRAD::Schedule::type_t, NANDRAD::Schedule> ::iterator
							scheduleIt = designDaySpaceTypeGroup.begin();
							scheduleIt != designDaySpaceTypeGroup.end(); ++scheduleIt) {

							NANDRAD::Schedule::type_t scheduleType = scheduleIt->first;
							// check if the schedule is part of basic schedule group and the corresponding
							// space type group
							std::map<NANDRAD::Schedule::type_t, NANDRAD::Schedule> ::iterator
								basicScheduleIt = basicSpaceTypeGroupIt->second.find(scheduleType);
							// yes: add all daily cycles
							if(basicScheduleIt != basicSpaceTypeGroupIt->second.end()) {
								basicScheduleIt->second.m_dailyCycles.insert(
									basicScheduleIt->second.m_dailyCycles.end(),
									scheduleIt->second.m_dailyCycles.begin(),
									scheduleIt->second.m_dailyCycles.end() );
							}
							// no: add newly designed schedule to space type group
							else {
								basicSpaceTypeGroupIt->second[scheduleType] = scheduleIt->second;
							}
						}
					}
					else {
						// simply add all schedules if space type group is still undefined
						basicScheduleGroup.m_spaceTypeGroups[spaceTypeName] = designDaySpaceTypeGroup;
					}
				}
			}
			// a basic schedule group
			if(group == 0) {
				if(!basicScheduleGroup.m_spaceTypeGroups.empty()) {
					designDayScheduleGroups.push_back(basicScheduleGroup);
				}
			}
			// not a basic schedule group
			else {
				if(!designDayScheduleGroup.m_spaceTypeGroups.empty()) {
					designDayScheduleGroups.push_back(designDayScheduleGroup);
				}
			}
		}

		// manipulate all schedules
		project.m_schedules.m_scheduleGroups = designDayScheduleGroups;
		project.m_schedulesReference.clear();
		project.m_schedules.m_annualSchedules.m_parameters.clear();

		// manipulate models
		project.m_models.m_models.clear();
		project.m_modelGroups.m_modelGroups.clear();

		// compose outputs for all active zones
		NANDRAD::ObjectList zonesObjectList;
		zonesObjectList.m_filterType = NANDRAD::ModelInputReference::MRT_ZONE;
		zonesObjectList.m_filterID.m_allIDs = true;
		zonesObjectList.m_name = "Active zones";

		// compose an object list fornthe constant outside zone
		NANDRAD::ObjectList outsideZoneObjectList;
		outsideZoneObjectList.m_filterType = NANDRAD::ModelInputReference::MRT_LOCATION;
		outsideZoneObjectList.m_name = "Outside zone";

		// add to object list definition
		project.m_objectLists.m_objectLists[zonesObjectList.m_name]				= zonesObjectList;
		project.m_objectLists.m_objectLists[outsideZoneObjectList.m_name]		= outsideZoneObjectList;

		// delete all outputs
		project.m_outputs.m_grids.clear();
		project.m_outputs.m_outputDefinitions.clear();
		project.m_outputsReference.clear();

		// compose outputs for all quantities of interest: room air temperature,
		// ideal heating convective loads, set point temperature, constant ouside temperature,
		// zone volume: interval one day
		NANDRAD::OutputGrid grid;
		grid.m_name = "Hourly";

		std::string stepSizeParaName = NANDRAD::KeywordList::Keyword(categoryTimePara.c_str(),
			NANDRAD::Interval::IP_STEPSIZE);

		NANDRAD::Interval interval;
		// we start monitoring after 1 week
		IBK::Parameter startPara(startParaName, 0.0, startParaUnit);
		startPara.value = 7. * 24. * 3600.;
		interval.m_genericParaConst[startParaName]		= startPara;
		interval.m_genericParaConst[endParaName]		= IBK::Parameter(endParaName, endTime, endParaUnit);
		interval.m_genericParaConst[stepSizeParaName]	= IBK::Parameter(stepSizeParaName, 1, "h");
		// add to grid definition
		grid.m_intervals.push_back(interval);

		// add to output definition
		project.m_outputs.m_grids.push_back(grid);

		// compose output definitions
		NANDRAD::OutputDefinition outputDef;
		outputDef.m_gridName = grid.m_name;
		outputDef.m_objectListName = zonesObjectList.m_name;
		outputDef.m_quantity = KeywordList::Keyword("RoomStatesModel::Results", RoomStatesModel::R_AirTemperature);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("UserModel::InputReferences", UserModel::InputRef_Area);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("IdealCoolingModel::InputReferences",
			IdealCoolingModel::InputRef_CoolingSetPointTemperature);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("CoolingsLoadModel::Results",
			CoolingsLoadModel::R_CoolingsLoad);
		// set integral mean as time type
		outputDef.m_timeType = NANDRAD::OutputDefinition::OTT_MEAN;
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		// reset time type
		outputDef.m_timeType = NANDRAD::OutputDefinition::OTT_NONE;
		outputDef.m_quantity = KeywordList::Keyword("UsersThermalLoadModel::Results",
			UsersThermalLoadModel::R_SensitiveUsersLoad);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("EquipmentLoadModel::Results",
			EquipmentLoadModel::R_EquipmentLoad);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("LightingLoadModel::Results",
			LightingLoadModel::R_LightingLoad);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_quantity = KeywordList::Keyword("WindowsLoadModel::Results",
			WindowsLoadModel::R_WindowsSWRadLoad);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		outputDef.m_objectListName = outsideZoneObjectList.m_name;
		outputDef.m_quantity = KeywordList::Keyword("Loads::Results",
			Loads::R_Temperature);
		// add to output definitions
		project.m_outputs.m_outputDefinitions.push_back(outputDef);

		// correct solver options
		project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MIN_DT].value =
			std::min(project.m_solverParameter.m_para[NANDRAD::SolverParameter::SP_MIN_DT].value, 1e-12);

		// now write project to file
		IBK::Path targetFile = dirs().m_coolingDesignRootDir;
		targetFile.addExtension("nandrad");
		project.writeXML(targetFile);

	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error creating file for cooling design calculation from project file %1!")
			.arg(m_impl->m_projectFilePath.str()), FUNC_ID);
	}
}


void NandradModel::export2FMU(const IBK::Path & installDir, const IBK::Path & projectFile, const IBK::Path & targetFile,
	const std::string &reportFileExtension)
{
	const char * FUNC_ID = "[NandradModel::export2FMU]";

	struct DirRemover {
		DirRemover(const IBK::Path& dir) : m_dir(dir)
		{}
		~DirRemover() {
			// Check if files within directory are still accessed by other process, for example Virus scanner
			IBK::Path::remove(m_dir);
		}
		const IBK::Path& m_dir;
	};

	IBK::Path tempDir(m_impl->m_dirs.m_tmpDir.str());
	tempDir /= "NandradFMU";
	// if export directory exists, remove it first
	if (tempDir.isDirectory()) {
		IBK::Path::remove(tempDir);
	}
	if (!IBK::Path::makePath(tempDir)) {
		throw IBK::Exception(IBK::FormatString("Cannot create directory: '%1'\n").arg(tempDir.absolutePath()), FUNC_ID);
	}
	DirRemover dirRemover(tempDir); (void)dirRemover;


	try {
		IBK::Path fmuFile(targetFile);
		IBK::Path fmuDirectory(targetFile);
		fmuDirectory = fmuDirectory.absolutePath();
		fmuDirectory = fmuDirectory.parentPath();

		if (!fmuDirectory.isDirectory()) {
			IBK::IBK_Message( IBK::FormatString("Creating FMU export directory for package: '%1'\n").arg(fmuDirectory.absolutePath()),
							  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
			IBK::Path::makePath(fmuDirectory);
		}
		// store all file pathes inside container for later compession
		std::vector<IBK::Path> fmuFiles;

		if(fmu2ExportModel() == NULL || fmu2ImportModel() == NULL) {
			throw IBK::Exception( IBK::FormatString("Model is not initialized!\n"), FUNC_ID);
		}

		// create directory structure
		IBK::Path binariesDir = tempDir / "binaries";
#if defined (_WIN64)
		binariesDir /= "win64";
#elif defined (_WIN32)
		binariesDir /= "win32";
#else
		binariesDir /= "linux32";
#endif
		IBK::Path::makePath(binariesDir);
		IBK::Path resourceDir = tempDir / "resources";
		IBK::Path::makePath(resourceDir);

		// correct path
		IBK::Path licenseDir = installDir / IBK::Path("resources");
		// correct path
		if (!licenseDir.isDirectory() && installDir.branchCount() >= 3)
			licenseDir = installDir / IBK::Path("../../../doc");
		if (!licenseDir.isDirectory() && installDir.branchCount() >= 2)
			licenseDir = installDir / IBK::Path("../../doc");
		IBK_ASSERT(licenseDir.isDirectory());

		// copy library files, only for MINGW32
#ifdef __MINGW32__
		if (!IBK::Path::copy((installDir / "DataIO.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "DataIO.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "DataIO.dll");

		if (!IBK::Path::copy((installDir / "CCM.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "CCM.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "CCM.dll");

		if (!IBK::Path::copy((installDir / "TiCPP.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "TiCPP.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "TiCPP.dll");

		if(!IBK::Path::copy((installDir / "IBK.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "IBK.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "IBK.dll");

		if(!IBK::Path::copy((installDir / "IBKMK.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "IBKMK.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "IBKMK.dll");

		if(!IBK::Path::copy((installDir / "IntegratorFramework.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "IntegratorFramework.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "IntegratorFramework.dll");

		if(!IBK::Path::copy((installDir / "WallModel.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "WallModel.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "WallModel.dll");

		if(!IBK::Path::copy((installDir / "DelphinLight.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "DelphinLight.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "DelphinLight.dll");

		if(!IBK::Path::copy((installDir / "Materials.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "Materials.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "Materials.dll");

		if(!IBK::Path::copy((installDir / "Zeppelin.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "Zeppelin.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "Zeppelin.dll");

		if(!IBK::Path::copy((installDir / "Nandrad.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "Nandrad.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "Nandrad.dll");

		if(!IBK::Path::copy((installDir / "NandradModel.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "NandradModel.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "NandradModel.dll");

		if(!IBK::Path::copy((installDir / "zlib.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "zlib.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "zlib.dll");

#endif // __MINGW32__

		// for VC compiler, copy dynamic runtime libs
#ifdef _MSC_VER

		// Note: we expect VC Dlls to be copied into the corresponding binary directory, because path depths varies depending on
		//	     wether we compile with CMake or from VC Development environment.
		if (!IBK::Path::copy((installDir / "msvcp140.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "msvcp140.dll").absolutePath().str()), FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "msvcp140.dll");

		if (!IBK::Path::copy((installDir / "vcruntime140.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "vcruntime140.dll").absolutePath().str()), FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "vcruntime140.dll");

#endif // _MSC_VER


	// for mingw compiler, copy std libs and gcc dll
#ifdef __MINGW32__

		if(!IBK::Path::copy((installDir / "libstdc++-6.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "libstdc++-6.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "libstdc++-6.dll");

		if(!IBK::Path::copy((installDir / "libgcc_s_dw2-1.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "libgcc_s_dw2-1.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "libgcc_s_dw2-1.dll");

#endif // __MINGW32__

#if defined(_WIN32)
		if (!IBK::Path::copy((installDir / "NandradFMI2.dll"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "NandradFMI2.dll").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "NandradFMI2.dll");
#else
		if (!IBK::Path::copy((installDir / "libNandradSolverFMI_v2.so"), binariesDir)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy library: '%1'\n").arg((installDir / "libNandradSolverFMI_v2.so").absolutePath().str()),
							  FUNC_ID);
		}
		fmuFiles.push_back(binariesDir / "libNandradSolverFMI_v2.so");
#endif // _WIN32

		// copy license information
		if (!IBK::Path::copy((licenseDir), resourceDir)) {
			throw IBK::Exception(IBK::FormatString("Cannot copy directory: '%1'\n").arg((licenseDir).absolutePath().str()),
				FUNC_ID);
		}
		// copy all files to file list

#if defined(_WIN32)
		WIN32_FIND_DATAW filelist;
		HANDLE h = FindFirstFileW((licenseDir / "*").wstr().c_str(), &filelist);

		if (INVALID_HANDLE_VALUE == h)
		{
			throw IBK::Exception(IBK::FormatString("Cannot copy directory: '%1'\n")
				.arg((licenseDir).absolutePath().str()),
				FUNC_ID);
		}

		// List all the files in the directory with some info about them.
		do
		{
			std::wstring fileNameW(filelist.cFileName);
			IBK::Path filePath(IBK::Path(IBK::WstringToUTF8(fileNameW.c_str())));
			// skip invalid files
			if (!(resourceDir / filePath).isFile())
				continue;
			fmuFiles.push_back(resourceDir / filePath);
		}
		while (FindNextFileW(h, &filelist) != 0);

		DWORD errmsg = GetLastError();
		if (errmsg != ERROR_NO_MORE_FILES)
		{
			throw IBK::Exception(IBK::FormatString("Cannot copy directory: '%1'\n")
				.arg((licenseDir).absolutePath().str()),
				FUNC_ID);
		}

		FindClose(h);
#else

		DIR *dp;
		struct dirent *ep;
		dp = opendir(licenseDir.str().c_str());

		if (dp == NULL) {
			throw IBK::Exception(IBK::FormatString("Cannot copy directory: '%1'\n")
				.arg((licenseDir).absolutePath().str()),
				FUNC_ID);
		}
		while ((ep = readdir(dp))) {
			// skip invalid files
			if (!(resourceDir / ep->d_name).isFile())
				continue;
			fmuFiles.push_back(resourceDir / ep->d_name);
		}
#endif

		// copy resource files
		// project file must be named "Project.nandrad"
		IBK::Path projectDestFile(resourceDir / "Project.nandrad");
		if (!IBK::Path::copy(IBK::Path(projectFile), projectDestFile)) {
			throw IBK::Exception( IBK::FormatString("Cannot copy project file: '%1'\n").arg(projectFile),
							  FUNC_ID);
		}
		fmuFiles.push_back(projectDestFile);

		// open file for change of resource pathes
		NANDRAD::Project project;
		// initialize project data structure with default values
		project.initDefaults(NULL);
		// read from original file
		project.readXML(projectFile);

		// store all construction materials
		std::set<IBK::Path> materialFiles;

		// retrieve construction file sources
		for( unsigned int i = 0; i < project.m_constructionTypeReferences.size(); ++i) {
			IBK::Path constructionSrcFile = IBK::Path(project.m_constructionTypeReferences[i].m_filename);
			// replace place holder
			if(constructionSrcFile.hasPlaceholder()) {
				constructionSrcFile = constructionSrcFile.withReplacedPlaceholders(m_impl->project().m_placeholders);
			}
			// define destination directory
			IBK::Path constructionDestFile = resourceDir / constructionSrcFile.filename();
			if(!IBK::Path::copy(constructionSrcFile, constructionDestFile)) {
				// require construction files
				throw IBK::Exception( IBK::FormatString("Cannot copy construction file: '%1'\n")
									.arg(constructionSrcFile.filename()), FUNC_ID);
			}
			// load construction file
			DELPHIN_LIGHT::ConstructionType conType;
			conType.readXML(constructionDestFile.str());

			// now get all materials
			for( unsigned int j = 0; j < conType.m_materialLayers.size(); ++j) {
				IBK::Path materialSrcFile = conType.m_materialLayers[j].m_matRef.m_filename;
				// replace place holder
				if(materialSrcFile.hasPlaceholder()) {
					materialSrcFile = materialSrcFile.withReplacedPlaceholders(m_impl->project().m_placeholders);
					materialSrcFile = materialSrcFile.withReplacedPlaceholders(conType.m_placeholders);
				}
				// directly store material file
				materialFiles.insert(materialSrcFile);
				// define destination directory
				IBK::Path materialDestFile = resourceDir / materialSrcFile.filename();
				// set new file path
				conType.m_materialLayers[j].m_matRef.m_filename = materialDestFile;
				conType.m_materialLayers[j].m_matRef.m_filename.insertPlaceholder(IBK::PLACEHOLDER_PROJECT_DIR,
					resourceDir);
			}
			// clear placeholders
			conType.m_placeholders.clear();
			// store new information
			conType.writeXML(constructionDestFile.str());

			// store new construction type path
			project.m_constructionTypeReferences[i].m_filename =
				constructionDestFile;
			// add 'Project Directory' place holde
			project.m_constructionTypeReferences[i].m_filename.insertPlaceholder(IBK::PLACEHOLDER_PROJECT_DIR,
				resourceDir);

			// and store construction file for compression
			fmuFiles.push_back(constructionDestFile);
		}

		// retreive material file sources
		for(std::set<IBK::Path>::iterator it = materialFiles.begin(); it != materialFiles.end(); ++it) {
			IBK::Path materialSrcFile = *it;
			// define destination directory
			IBK::Path materialDestFile = resourceDir / materialSrcFile.filename();
			// copy source file
			if(!IBK::Path::copy(materialSrcFile, materialDestFile)) {
				// require construction files
				throw IBK::Exception( IBK::FormatString("Cannot copy material file: '%1'\n")
									.arg(materialSrcFile.filename()), FUNC_ID);
			}

			// and store material file for compression
			fmuFiles.push_back(materialDestFile);
		}

		// retreive climate file
		// if climate data may be imported we do not need this
		// task any longer

		// retreive climate data location from project file
		IBK::Path climateSrcFile = IBK::Path(m_impl->project().m_location.m_climateFileName);
		// replace place holder
		if(climateSrcFile.hasPlaceholder()) {
			climateSrcFile = climateSrcFile.withReplacedPlaceholders(m_impl->project().m_placeholders);
		}

		// only add c6b data files for climate
		IBK::Path climateFile = climateSrcFile;
		// change extension the case we ordered a wrong data format
		if(climateFile.extension() != "c6b") {
			try {
				// create climate file at resourece directory
				climateFile = resourceDir / climateFile.filename();

				// create a *c6b file
				if (climateFile.extension() == "epw") {
					CCM::ClimateDataLoader climateDataLoader;
					climateDataLoader.readClimateDataEPW(climateSrcFile.withReplacedPlaceholders(project.m_placeholders));
					// rename climate file
					climateFile = climateFile.withoutExtension();
					climateFile.addExtension("c6b");
					// write content inro climate file
					climateDataLoader.writeClimateDataIBK(climateFile);
				}
				else if (climateFile.extension() == "wac") {
					CCM::ClimateDataLoader climateDataLoader;
					climateDataLoader.readClimateDataWAC(climateSrcFile.withReplacedPlaceholders(project.m_placeholders));
					// rename climate file
					climateFile = climateFile.withoutExtension();
					climateFile.addExtension("c6b");
					// write content inro climate file
					climateDataLoader.writeClimateDataIBK(climateFile);
				}
				fmuFiles.push_back(climateFile);
			}
			catch (IBK::Exception &ex) {
				throw IBK::Exception(ex, IBK::FormatString(
					"Error converting climate data file '%1'!")
					.arg(climateSrcFile.c_str()), FUNC_ID);
			}
		}
		// just copy file
		else {
			if (!IBK::Path::copy(climateFile, resourceDir)) {
				// require ccd files
				throw IBK::Exception(IBK::FormatString("Cannot copy climate file: '%1'\n").arg(climateFile.absolutePath())
					, FUNC_ID);
			}
			else {
				fmuFiles.push_back(resourceDir / climateFile.filename());
			}
		}
		// replace recource directory by project directory
		IBK::Path climateDestRelativePath = resourceDir / climateFile.filename();
		climateDestRelativePath.insertPlaceholder(IBK::PLACEHOLDER_PROJECT_DIR,
			resourceDir);
		project.m_location.m_climateFileName = climateDestRelativePath.str();

		if (!m_impl->project().m_location.m_shadingFactorFileName.str().empty()) {
			// retreive shading data location from project file
			IBK::Path shadingFile = IBK::Path(m_impl->project().m_location.m_shadingFactorFileName);
			// replace place holder
			if (shadingFile.hasPlaceholder()) {
				shadingFile = shadingFile.withReplacedPlaceholders(m_impl->project().m_placeholders);
			}
			// copy file
			if (!IBK::Path::copy(shadingFile, resourceDir)) {
				// require ccd files
				throw IBK::Exception(IBK::FormatString("Cannot copy shading factor file: '%1'\n").arg(shadingFile.absolutePath())
					, FUNC_ID);
			}
			else {
				fmuFiles.push_back(resourceDir / shadingFile.filename());
			}
			// replace recource directory by project directory
			IBK::Path shadingDestRelativePath = resourceDir / shadingFile.filename();
			shadingDestRelativePath.insertPlaceholder(IBK::PLACEHOLDER_PROJECT_DIR,
				resourceDir);
			project.m_location.m_shadingFactorFileName = shadingDestRelativePath;
		}

		// retreive schedules location from project file
		IBK::Path scheduleFile = project.m_schedulesReference.m_filename;
		if(scheduleFile.isValid() ) {
			scheduleFile = scheduleFile.withReplacedPlaceholders(m_impl->project().m_placeholders);
			if(!IBK::Path::copy(scheduleFile, resourceDir)) {
				// require ccd files
				throw IBK::Exception( IBK::FormatString("Cannot copy schedules file: '%1'\n").arg(scheduleFile.absolutePath())
										, FUNC_ID);
			}
			else {
				fmuFiles.push_back(resourceDir / scheduleFile.filename());
			}
			// replace recource directory by project directory
			IBK::Path schedulesDestRelativePath = resourceDir / scheduleFile.filename();
			schedulesDestRelativePath.insertPlaceholder(IBK::PLACEHOLDER_PROJECT_DIR,
				resourceDir);
			project.m_schedulesReference.m_filename = schedulesDestRelativePath;
		}


		// retreive outputs location from project file
		IBK::Path outputsFile = project.m_outputsReference.m_filename;
		if(outputsFile.isValid() ) {
			outputsFile = outputsFile.withReplacedPlaceholders(m_impl->project().m_placeholders);
			if(!IBK::Path::copy(outputsFile, resourceDir)) {
				// require ccd files
				throw IBK::Exception( IBK::FormatString("Cannot copy outputs file: '%1'\n").arg(outputsFile.absolutePath())
										, FUNC_ID);
			}
			else {
				fmuFiles.push_back(resourceDir / outputsFile.filename());
			}
			// replace recource directory by project directory
			IBK::Path outputsDestRelativePath = resourceDir / outputsFile.filename();
			outputsDestRelativePath.insertPlaceholder(IBK::PLACEHOLDER_PROJECT_DIR,
				resourceDir);
			project.m_outputsReference.m_filename = outputsDestRelativePath;
		}

		// find all ground zones
		std::set<IBK::Path> groundZoneTemperatureFiles;
		for (std::map<unsigned int, NANDRAD::Zone>::iterator
			zoneIt = project.m_zones.begin();
			zoneIt != project.m_zones.end();
			++zoneIt) {
			// only ground zone are of interes
			if (zoneIt->second.m_zoneType != NANDRAD::Zone::ZT_GROUND)
				continue;
			IBK_ASSERT(!zoneIt->second.m_climateFileName.str().empty());
			// retreive shading data location from project file
			IBK::Path temperatureFile = zoneIt->second.m_climateFileName;
			// replace place holder
			if (temperatureFile.hasPlaceholder()) {
				temperatureFile = temperatureFile.withReplacedPlaceholders(m_impl->project().m_placeholders);
			}
			// select file
			groundZoneTemperatureFiles.insert(temperatureFile/"Temperature.ccd");
			// replace recource directory by project directory
			IBK::Path tempDestRelativePath = resourceDir / temperatureFile.filename();
			tempDestRelativePath.insertPlaceholder(IBK::PLACEHOLDER_PROJECT_DIR,
				resourceDir);
			zoneIt->second.m_climateFileName = tempDestRelativePath.parentPath();
		}

		for (std::set<IBK::Path>::const_iterator
			tempFileIt = groundZoneTemperatureFiles.begin();
			tempFileIt != groundZoneTemperatureFiles.end();
			++tempFileIt) {
			// copy all temperture files into resource directory
			if (!IBK::Path::copy(*tempFileIt, resourceDir)) {
				// require ccd files
				throw IBK::Exception(IBK::FormatString("Cannot copy ground zone temperature file: '%1'\n").arg(tempFileIt->absolutePath())
					, FUNC_ID);
			}
			else {
				fmuFiles.push_back(resourceDir / tempFileIt->filename());
			}
		}

		// clear all place holdes
		project.m_placeholders.clear();
		// when all resources are copied and all pathes are set
		// rewrite project file
		project.writeXML(projectDestFile);


		// create model description
		IBK::Path modelDescriptionFile = tempDir / "modelDescription.xml";

		FMU2ModelDescription modelDescription(*this, *fmu2ExportModel(), *fmu2ImportModel());
		modelDescription.write(modelDescriptionFile);
		fmuFiles.insert(fmuFiles.begin(),modelDescriptionFile);

		// substitute this function by an external one later
		compressFMUFiles(fmuFiles, fmuFile, tempDir, 1, false);

		// add fmu export report
		char delimiter ;
		if(reportFileExtension == "tsv") {
			delimiter = '\t';
		}
		else if(reportFileExtension == "csv") {
			delimiter = ';';
		}
		// we choose tab as default
		else if(reportFileExtension == "dsv") {
			delimiter = '\t';
		}
		else {
			throw IBK::Exception(IBK::FormatString("Unsupported report file format '*.%1'")
				.arg(reportFileExtension), FUNC_ID);
		}
		IBK::Path reportFile = IBK::Path(targetFile).withoutExtension();
		reportFile.addExtension(reportFileExtension);
		writeFMUReport(reportFile, delimiter);
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error creating fmu '%1'.").arg(targetFile),
						  FUNC_ID);
	}
	IBK::IBK_Message("FMU Export complete.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
}


void NandradModel::createModelicaFMUAdapterAndWrapper(const IBK::Path & fmuFile)  {

	const char * const FUNC_ID = "[NandradModel::createModelicaFMUAdapterAndWrapper]";

	IBK::Path modelFileName = fmuFile.filename().withoutExtension();
	IBK::Path targetDir = fmuFile.parentPath().absolutePath();

#ifdef _WIN32
	targetDir/= std::string("GB2NANDRAD_") + IBK::WstringToUTF8(modelFileName.wstr());
#else
	targetDir/= std::string("GB2NANDRAD_") + modelFileName.str();
#endif

	if (!targetDir.isDirectory() ) {
		IBK::IBK_Message( IBK::FormatString("Creating Modelica FMU wrapper directory for package: '%1'\n")
			.arg(targetDir.absolutePath()),
							IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
		IBK::Path::makePath(targetDir.absolutePath());
	}


	if (!fmuFile.exists()) {
		throw IBK::Exception( IBK::FormatString("Error Modelica FMU wrapper directory for FMU file '%1': "
			"File does not exist!\n").arg(fmuFile.absolutePath()), FUNC_ID);
	}

	// prepare all strings for input and output names, ids and connections
	const std::vector<FMU2QuantityDescription> &importQuantityDescriptions
		= fmu2ImportModel()->FMU2ImportQuantityDescriptions();
	const std::vector<FMU2QuantityDescription> &exportQuantityDescriptions
		= fmu2ExportModel()->FMU2ExportQuantityDescriptions();

	std::string zoneInputNames, zoneOutputNames;
	std::vector<unsigned int> HVACZoneIds, HotWaterZoneIds, GridZoneIds;

	for(unsigned int i = 0; i < importQuantityDescriptions.size(); ++i)
	{
		//FMU2QuantityDescription quantityDesc = exportQuantityDescriptions[i];
		FMU2QuantityDescription quantityDesc = importQuantityDescriptions[i];

		// only HVAC or Hot water consumptionn component possible
		if(quantityDesc.m_name ==
			KeywordList::Keyword("FMU2QuantityDescription::FMUImportQuantity",
				FMU2QuantityDescription::IQ_ConvectiveThermalLoad) ) {

			for(unsigned int j = 0; j < quantityDesc.m_indexKeys.size(); ++j) {
				HVACZoneIds.push_back(quantityDesc.m_indexKeys[j].m_keyValue);
			}
		}
		else if (quantityDesc.m_name ==
			KeywordList::Keyword("FMU2QuantityDescription::FMUImportQuantity",
				FMU2QuantityDescription::IQ_DomesticWaterTemperature)) {

			for (unsigned int j = 0; j < quantityDesc.m_indexKeys.size(); ++j) {
				HotWaterZoneIds.push_back(quantityDesc.m_indexKeys[j].m_keyValue);
			}
		}


		// only vector valued quantities
		for(unsigned int j = 0; j < quantityDesc.m_indexKeys.size(); ++j)
		{
			// create a string for the reference id
			std::string idStr	= IBK::val2string<unsigned int>(quantityDesc.m_varIDs[j]);
			// create an indexString <quantityName>_id<Id-number>
			std::string modifiedIndexKey = quantityDesc.m_indexKeys[j].toString();
			// remove '='
			std::size_t pos = modifiedIndexKey.find("=");
			if(pos != std::string::npos)
				modifiedIndexKey.replace(pos, 1, "_");

			// create a name string
			// last entry
			std::string inputName;
			if(i == importQuantityDescriptions.size() - 1
				&& j == quantityDesc.m_indexKeys.size() - 1) {
				inputName = INPUT_NAME_END;
			}
			else {
				inputName = INPUT_NAME;
			}
			inputName = IBK::replace_string(inputName, "${NAME}", quantityDesc.m_name + std::string("_")
									+ modifiedIndexKey, IBK::ReplaceFirst);
			zoneInputNames += inputName;
		}
	}
	for(unsigned int i = 0; i < exportQuantityDescriptions.size(); ++i)
	{
		//FMU2QuantityDescription quantityDesc = exportQuantityDescriptions[i];
		FMU2QuantityDescription quantityDesc = exportQuantityDescriptions[i];

		// vector valued quantity
		if(!quantityDesc.m_indexKeys.empty())
		{
			// also electric component possible
			if(quantityDesc.m_name ==
				KeywordList::Keyword("FMU2QuantityDescription::FMUExportQuantity",
					FMU2QuantityDescription::EQ_ElectricPowerConsumption) ) {
				for(unsigned int j = 0; j < quantityDesc.m_indexKeys.size(); ++j) {
					GridZoneIds.push_back(quantityDesc.m_indexKeys[j].m_keyValue);
				}
			}

			for(unsigned int j = 0; j < quantityDesc.m_indexKeys.size(); ++j)
			{
				// create a string for the reference id
				std::string idStr	= IBK::val2string<unsigned int>(quantityDesc.m_varIDs[j]);
				// create an indexString <quantityName>_id<Id-number>
				std::string modifiedIndexKey = quantityDesc.m_indexKeys[j].toString();
				// remove '='
				std::size_t pos = modifiedIndexKey.find("=");
				if(pos != std::string::npos)
					modifiedIndexKey.replace(pos, 1, "_");
				// create a name string
				std::string outputName;
				// last entry
				if(i == exportQuantityDescriptions.size() - 1
					&& j == quantityDesc.m_indexKeys.size() - 1) {
					outputName = OUTPUT_NAME_END;
				}
				else {
					outputName = OUTPUT_NAME;
				}
				outputName = IBK::replace_string(outputName, "${NAME}", quantityDesc.m_name + std::string("_")
										+ modifiedIndexKey, IBK::ReplaceFirst);
				zoneOutputNames += outputName;
			}
		}
	}


	std::string packageFiles;
	// create fmu modelica file from template string
	std::string packageStr = PACKAGE_MO;
	std::string logoStr = LOGO1;
	logoStr += LOGO2;
	logoStr += LOGO3;
	logoStr += LOGO4;
	logoStr += LOGO5;

#ifdef _WIN32
	// replace model name
	packageStr = IBK::replace_string(packageStr, "${MODEL_NAME}", IBK::WstringToUTF8(modelFileName.wstr()),
		IBK::ReplaceAll);
#else
	// replace model name
	packageStr = IBK::replace_string(packageStr, "${MODEL_NAME}", modelFileName.str(),
		IBK::ReplaceFirst);
#endif

	// replace logo
	packageStr = IBK::replace_string(packageStr, "${LOGO}", logoStr,
		IBK::ReplaceAll);

	// replace maximum zone index
	packageStr = IBK::replace_string(packageStr, "${MAX_ZONE_INDEX_HVAC}",
		IBK::val2string<unsigned int> ((unsigned int) HVACZoneIds.size() ),
		IBK::ReplaceAll);
	packageStr = IBK::replace_string(packageStr, "${MAX_ZONE_INDEX_HWC}",
		IBK::val2string<unsigned int>((unsigned int) HotWaterZoneIds.size()),
		IBK::ReplaceAll);
	packageStr = IBK::replace_string(packageStr, "${MAX_ZONE_INDEX_GRID}",
		IBK::val2string<unsigned int> ((unsigned int) GridZoneIds.size() ),
		IBK::ReplaceAll);

	std::string zoneInputsAdStr, zoneOutputsAdStr, zoneInputsFMUStr, zoneOutputsFMUStr;
	// create input connectors for HVAC copmponents
	for(unsigned int i = 0; i < HVACZoneIds.size(); ++i) {
		std::string zoneInputAdStr = HVAC_ZONE_INPUTS_ADAPTER;
		zoneInputAdStr = IBK::replace_string(zoneInputAdStr, "${ZONE_ID}", IBK::val2string<unsigned int>(HVACZoneIds[i]),
			IBK::ReplaceAll);
		zoneInputsAdStr += zoneInputAdStr;

		std::string zoneInputFMUStr = HVAC_ZONE_INPUTS_FMU;
		zoneInputFMUStr = IBK::replace_string(zoneInputFMUStr, "${ZONE_ID}", IBK::val2string<unsigned int>(HVACZoneIds[i]),
			IBK::ReplaceAll);
		zoneInputsFMUStr += zoneInputFMUStr;
	}
	// Hot water copmponents
	for (unsigned int i = 0; i < HotWaterZoneIds.size(); ++i) {
		std::string zoneInputAdStr = HWC_ZONE_INPUTS_ADAPTER;
		zoneInputAdStr = IBK::replace_string(zoneInputAdStr, "${ZONE_ID}", IBK::val2string<unsigned int>(HotWaterZoneIds[i]),
			IBK::ReplaceAll);
		zoneInputsAdStr += zoneInputAdStr;

		std::string zoneInputFMUStr = HWC_ZONE_INPUTS_FMU;
		zoneInputFMUStr = IBK::replace_string(zoneInputFMUStr, "${ZONE_ID}", IBK::val2string<unsigned int>(HotWaterZoneIds[i]),
			IBK::ReplaceAll);
		zoneInputsFMUStr += zoneInputFMUStr;
	}
	// and electric grid componentes
	for(unsigned int i = 0; i < GridZoneIds.size(); ++i) {
		std::string zoneInputAdStr = GRID_ZONE_INPUTS_ADAPTER;
		zoneInputAdStr = IBK::replace_string(zoneInputAdStr, "${ZONE_ID}", IBK::val2string<unsigned int>(GridZoneIds[i]),
			IBK::ReplaceAll);
		zoneInputsAdStr += zoneInputAdStr;

		std::string zoneInputFMUStr = GRID_ZONE_INPUTS_FMU;
		zoneInputFMUStr = IBK::replace_string(zoneInputFMUStr, "${ZONE_ID}", IBK::val2string<unsigned int>(GridZoneIds[i]),
			IBK::ReplaceAll);
		zoneInputsFMUStr += zoneInputFMUStr;
	}

	// create output connectors
	for(unsigned int i = 0; i < HVACZoneIds.size(); ++i) {
		std::string zoneOutputAdStr = HVAC_ZONE_OUTPUTS_ADAPTER;
		zoneOutputAdStr = IBK::replace_string(zoneOutputAdStr, "${ZONE_ID}", IBK::val2string<unsigned int>(HVACZoneIds[i]),
			IBK::ReplaceAll);
		zoneOutputsAdStr += zoneOutputAdStr;

		std::string zoneOutputFMUStr = HVAC_ZONE_OUTPUTS_FMU;
		zoneOutputFMUStr = IBK::replace_string(zoneOutputFMUStr, "${ZONE_ID}", IBK::val2string<unsigned int>(HVACZoneIds[i]),
			IBK::ReplaceAll);
		zoneOutputsFMUStr += zoneOutputFMUStr;
	}
	for (unsigned int i = 0; i < HotWaterZoneIds.size(); ++i) {
		std::string zoneOutputAdStr = HWC_ZONE_OUTPUTS_ADAPTER;
		zoneOutputAdStr = IBK::replace_string(zoneOutputAdStr, "${ZONE_ID}", IBK::val2string<unsigned int>(HotWaterZoneIds[i]),
			IBK::ReplaceAll);
		zoneOutputsAdStr += zoneOutputAdStr;

		std::string zoneOutputFMUStr = HWC_ZONE_OUTPUTS_FMU;
		zoneOutputFMUStr = IBK::replace_string(zoneOutputFMUStr, "${ZONE_ID}", IBK::val2string<unsigned int>(HotWaterZoneIds[i]),
			IBK::ReplaceAll);
		zoneOutputsFMUStr += zoneOutputFMUStr;
	}

	// replace placeholders for input and output connectors
	packageStr = IBK::replace_string(packageStr, "${ZONE_INPUTS_ADAPTER}", zoneInputsAdStr,
		IBK::ReplaceFirst);
	packageStr = IBK::replace_string(packageStr, "${ZONE_INPUTS_FMU}", zoneInputsFMUStr,
		IBK::ReplaceFirst);

	packageStr = IBK::replace_string(packageStr, "${ZONE_OUTPUTS_ADAPTER}", zoneOutputsAdStr,
		IBK::ReplaceAll);
	packageStr = IBK::replace_string(packageStr, "${ZONE_OUTPUTS_FMU}", zoneOutputsFMUStr,
		IBK::ReplaceAll);

	std::string zoneConnectionsAdStr, zoneConnectionsWrStr;

	// create string for zone connections
	for(unsigned int i = 0; i < HVACZoneIds.size(); ++i) {
		std::string zoneConnectionAdStr = HVAC_ZONE_CONNECTIONS_ADAPTER;
		// replace zone id and index
		zoneConnectionAdStr = IBK::replace_string(zoneConnectionAdStr, "${ZONE_ID}",
			IBK::val2string<unsigned int> (HVACZoneIds[i]),
			IBK::ReplaceAll);
		zoneConnectionAdStr = IBK::replace_string(zoneConnectionAdStr, "${ZONE_INDEX}",
			IBK::val2string<unsigned int> (i + 1),
			IBK::ReplaceAll);
		// add to connections string
		zoneConnectionsAdStr += zoneConnectionAdStr;

		std::string zoneConnectionWrStr = HVAC_ZONE_CONNECTIONS_WRAPPER;
		// replace zone id and index
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${ZONE_ID}",
			IBK::val2string<unsigned int> (HVACZoneIds[i]),
			IBK::ReplaceAll);
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${ZONE_INDEX}",
			IBK::val2string<unsigned int> (i + 1),
			IBK::ReplaceAll);
#ifdef _WIN32
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${MODEL_NAME}", IBK::WstringToUTF8(modelFileName.wstr()),
			IBK::ReplaceAll);
#else
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${MODEL_NAME}", modelFileName.str(),
			IBK::ReplaceAll);
#endif
		// add to connections string
		zoneConnectionsWrStr += zoneConnectionWrStr;
	}
	for (unsigned int i = 0; i < HotWaterZoneIds.size(); ++i) {
		std::string zoneConnectionAdStr = HWC_ZONE_CONNECTIONS_ADAPTER;
		// replace zone id and index
		zoneConnectionAdStr = IBK::replace_string(zoneConnectionAdStr, "${ZONE_ID}",
			IBK::val2string<unsigned int>(HotWaterZoneIds[i]),
			IBK::ReplaceAll);
		zoneConnectionAdStr = IBK::replace_string(zoneConnectionAdStr, "${ZONE_INDEX}",
			IBK::val2string<unsigned int>(i + 1),
			IBK::ReplaceAll);
		zoneConnectionAdStr = IBK::replace_string(zoneConnectionAdStr, "${RHO_WATER}",
			IBK::val2string<double>(IBK::RHO_W),
			IBK::ReplaceAll);
		// add to connections string
		zoneConnectionsAdStr += zoneConnectionAdStr;

		std::string zoneConnectionWrStr = HWC_ZONE_CONNECTIONS_WRAPPER;
		// replace zone id and index
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${ZONE_ID}",
			IBK::val2string<unsigned int>(HotWaterZoneIds[i]),
			IBK::ReplaceAll);
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${ZONE_INDEX}",
			IBK::val2string<unsigned int>(i + 1),
			IBK::ReplaceAll);
#ifdef _WIN32
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${MODEL_NAME}", IBK::WstringToUTF8(modelFileName.wstr()),
			IBK::ReplaceAll);
#else
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${MODEL_NAME}", modelFileName.str(),
			IBK::ReplaceAll);
#endif
		// add to connections string
		zoneConnectionsWrStr += zoneConnectionWrStr;
	}
	for(unsigned int i = 0; i < GridZoneIds.size(); ++i) {
		std::string zoneConnectionAdStr = GRID_ZONE_CONNECTIONS_ADAPTER;
		// replace zone id and index
		zoneConnectionAdStr = IBK::replace_string(zoneConnectionAdStr, "${ZONE_ID}",
			IBK::val2string<unsigned int> (GridZoneIds[i]),
			IBK::ReplaceAll);
		zoneConnectionAdStr = IBK::replace_string(zoneConnectionAdStr, "${ZONE_INDEX}",
			IBK::val2string<unsigned int> (i + 1),
			IBK::ReplaceAll);
		// add to connections string
		zoneConnectionsAdStr += zoneConnectionAdStr;

		std::string zoneConnectionWrStr = GRID_ZONE_CONNECTIONS_WRAPPER;
		// replace zone id and index
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${ZONE_ID}",
			IBK::val2string<unsigned int> (GridZoneIds[i]),
			IBK::ReplaceAll);
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${ZONE_INDEX}",
			IBK::val2string<unsigned int> (i + 1),
			IBK::ReplaceAll);
#ifdef _WIN32
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${MODEL_NAME}", IBK::WstringToUTF8(modelFileName.wstr()),
			IBK::ReplaceAll);
#else
		zoneConnectionWrStr = IBK::replace_string(zoneConnectionWrStr, "${MODEL_NAME}", modelFileName.str(),
			IBK::ReplaceAll);
#endif
		// add to connections string
		zoneConnectionsWrStr += zoneConnectionWrStr;
	}


	// add missing connections
	packageStr = IBK::replace_string(packageStr, "${ZONE_CONNECTIONS_ADAPTER}", zoneConnectionsAdStr,
		IBK::ReplaceFirst);
	packageStr = IBK::replace_string(packageStr, "${ZONE_CONNECTIONS_WRAPPER}", zoneConnectionsWrStr,
		IBK::ReplaceFirst);


	// write to file
	IBK::Path packageFile = targetDir / "package.mo";
#ifdef _WIN32
	#if defined(_MSC_VER)
		std::wstring packageFileStr = packageFile.wstr();
		std::ofstream packageMoOut(packageFileStr.c_str());
	#else
		std::string  packageFileStrAnsi = IBK::WstringToANSI(packageFile.wstr(), false);
		std::ofstream packageMoOut(packageFileStrAnsi.c_str());
	#endif
#else
	// write desig data to file
	std::string packageFileStr = packageFile.str();
	std::ofstream packageMoOut(packageFileStr.c_str()); // UTF8
#endif

	packageMoOut << packageStr;


	IBK::Path packageOrderFile = targetDir / "package.order";
#ifdef _WIN32
	#if defined(_MSC_VER)
		std::wstring packageOrderFileStr = packageOrderFile.wstr();
		std::ofstream packageOrderOut(packageOrderFileStr.c_str());

		packageOrderOut << 	std::string("GB2NANDRAD_") << IBK::WstringToUTF8(modelFileName.wstr())
			<< std::string("_Adapter") << std::endl;
		packageOrderOut << 	std::string("GB2NANDRAD_") << IBK::WstringToUTF8(modelFileName.wstr())
			<< std::string("_Wrapper") << std::endl;
	#else
		std::string packageOrderFileStrAnsi = IBK::WstringToANSI(packageOrderFile.wstr(), false);
		std::ofstream packageOrderOut(packageOrderFileStrAnsi.c_str());

		packageOrderOut << 	std::string("GB2NANDRAD_") << IBK::WstringToUTF8(modelFileName.wstr())
			<< std::string("_Adapter") << std::endl;
		packageOrderOut << 	std::string("GB2NANDRAD_") << IBK::WstringToUTF8(modelFileName.wstr())
			<< std::string("_Wrapper") << std::endl;
	#endif
#else
	std::string packageOrderFileStr = packageOrderFile.str();
	std::ofstream packageOrderOut(packageOrderFileStr.c_str()); // UTF8

	packageOrderOut << 	std::string("GB2NANDRAD_") << modelFileName.str() << std::string("_Adapter") << std::endl;
	packageOrderOut << 	std::string("GB2NANDRAD_") << modelFileName.str() << std::string("_Wrapper") << std::endl;
#endif
}


void NandradModel::writeDesignDaySummary( )  const {

	const char * const FUNC_ID = "[NandradModel::writeDesignDaySummary]";

	const IBK::Path& summaryPath = dirs().m_resultsDir;
	const IBK::Path& heatingDesignResultsPath = dirs().m_heatingDesignResultsDir;
	const IBK::Path& coolingDesignResultsPath = dirs().m_coolingDesignResultsDir;

	std::string heatingLoadsFileName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
					NANDRAD::ModelInputReference::MRT_ZONE) + std::string("s");
	// add quantity name to output file names
	heatingLoadsFileName += std::string("_") + KeywordList::Keyword("HeatingsLoadModel::Results",
		HeatingsLoadModel::R_HeatingsLoad);
	// add objectList name
	heatingLoadsFileName += std::string("_objectList(Active_zones).d6o");

	std::string coolingLoadsFileName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
					NANDRAD::ModelInputReference::MRT_ZONE) + std::string("s");
	// add quantity name to output file names
	coolingLoadsFileName += std::string("_") + KeywordList::Keyword("CoolingsLoadModel::Results",
		CoolingsLoadModel::R_CoolingsLoad);
	// add objectList name
	coolingLoadsFileName += std::string("_objectList(Active_zones).d6o");

	std::string areaFileName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
					NANDRAD::ModelInputReference::MRT_ZONE) + std::string("s");
	// add quantity name to output file names
	areaFileName += std::string("_") + KeywordList::Keyword("UserModel::InputReferences",
		UserModel::InputRef_Area);
	// add objectList name
	areaFileName += std::string("_objectList(Active_zones).d6o");

	std::vector<unsigned int> zoneIds;
	// read last values
	DATAIO::DataIO heatingFile;
	try {
		heatingFile.read(heatingDesignResultsPath / heatingLoadsFileName);
		// error
		if (heatingFile.nValues() == 0 || heatingFile.m_timepoints.empty() )
			throw IBK::Exception("Empty file!", FUNC_ID);

		zoneIds = heatingFile.m_nums;
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading design data from file %1!").arg(heatingLoadsFileName),
			FUNC_ID);
	}

	DATAIO::DataIO coolingFile;
	try {
		coolingFile.read(coolingDesignResultsPath / coolingLoadsFileName);
		// error
		if (coolingFile.nValues() == 0 || coolingFile.m_timepoints.empty() )
			throw IBK::Exception("Empty file!", FUNC_ID);

		// error: not the same data quantity
		if (zoneIds != coolingFile.m_nums )	{
			throw IBK::Exception(IBK::FormatString("Data do not suite to cooling calculation!")
				, FUNC_ID);
		}
	}
	catch (IBK::Exception &ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading design data from file %1!").arg(coolingLoadsFileName),
			FUNC_ID);
	}

	// retrive last time poing
	if (heatingFile.m_timepoints.empty()) {
		throw IBK::Exception(IBK::FormatString("Error reading design data from file %1: "
			"data file is empty!").arg(coolingLoadsFileName),
			FUNC_ID);
	}
	unsigned int lastTimeIdx = (unsigned int) heatingFile.m_timepoints.size() - 1;
	const double * lastHeatingLoad;
	try {
		lastHeatingLoad = heatingFile.data(lastTimeIdx);
	}
	catch (IBK::Exception & ex) {
		// an error occured
		throw IBK::Exception(ex, "Error accessing heating load data.", FUNC_ID);
	}
	std::string heatingUnit = heatingFile.m_valueUnit;

	// retrive maximum value of last week
	if (coolingFile.m_timepoints.empty()) {
		throw IBK::Exception(IBK::FormatString("Error reading design data from file %1: "
			"data file is empty!").arg(coolingLoadsFileName),
			FUNC_ID);
	}
	std::vector<double> minCoolingLoad(zoneIds.size(),0.0);
	try {
		for (unsigned int i = 0; i < coolingFile.m_timepoints.size(); ++i) {
			//double timePoint = coolingFile.m_timepoints[i];
			for (unsigned int j = 0; j < zoneIds.size(); ++j) {

				double coolingLoad = coolingFile.data(i)[j];
				minCoolingLoad[j] = std::min(minCoolingLoad[j],coolingLoad);
			}
		}
	}
	catch (IBK::Exception & ex) {
		// an error occured
		throw IBK::Exception(ex, "Error accessing cooling load data.", FUNC_ID);
	}

	std::string coolingUnit = coolingFile.m_valueUnit;

#ifdef _WIN32
	#if defined(_MSC_VER)
		// write desig data to file
		std::wstring summaryfile =(summaryPath / "DesignDay.tsv").wstr();
		std::ofstream out(summaryfile.c_str());
	#else
		// write desig data to file
		std::string summaryfileAnsi = IBK::WstringToANSI((summaryPath / "DesignDay.tsv").wstr(), false);
		std::ofstream out(summaryfileAnsi.c_str());
	#endif
#else
	// write desig data to file
	std::string summaryfile =(summaryPath / "DesignDay.tsv").str();
	std::ofstream out(summaryfile.c_str()); // UTF8
#endif

	out << std::setw(7) << std::right << "ZoneID" << "\t";
	out << std::setw(30) << std::right << "HeatingDesignThermalLoad [" << heatingUnit << "]" << "\t";
	out << std::setw(30) << std::right << "CoolingDesignThermalLoad [" << coolingUnit << "]";
	out << std::endl;
	// for all values reserve one line
	for(unsigned int i = 0; i < zoneIds.size(); ++i) {
		out << std::setw(7) << std::right << zoneIds[i] << "\t";
		out << std::setw(30) << std::right << lastHeatingLoad[i] << "\t";
		out << std::setw(30) << std::right << minCoolingLoad[i];
		out << std::endl;
	}
}

void NandradModel::writeFMUReport(const IBK::Path & targetFile, char delimiter)  const {

	const char * const FUNC_ID = "NandradModel::writeFMUReport";

	// store list of exported zones with zone area
	std::map<unsigned int, std::string> exportedZoneWithNames;
	std::map<unsigned int, double>		exportedZoneWithAreas;

	try {
		std::string category = "FMU2QuantityDescription::FMUInterfaceDefinition";

		// store all corressponing zones
		for(std::map<unsigned int, NANDRAD::Zone>::const_iterator zoneIt =
			project().m_zones.begin(); zoneIt != project().m_zones.end(); ++zoneIt) {

			unsigned int zoneId = zoneIt->first;
			const NANDRAD::Zone &zone = zoneIt->second;
			// skip constant zones
			if(zone.m_zoneType == NANDRAD::Zone::ZT_CONSTANT || zone.m_zoneType == NANDRAD::Zone::ZT_GROUND)
				continue;
			// check if we belong to space type of the export list
			IBK_ASSERT(zone.m_spaceTypeRef != NULL);
			const NANDRAD::SpaceType &spaceType = *zone.m_spaceTypeRef;
			// check if teh space type is provided for FMU export
			std::map<std::string, std::string> ::const_iterator spaceTypeIt =
				spaceType.m_genericParaString.find("FMUInterfaceDefinition");
			// no export
			if(spaceTypeIt == spaceType.m_genericParaString.end() )
				continue;

			// default: no FMU interfaces
			std::set<FMU2QuantityDescription::FMUInterfaceDefinition> interfaceDefs;
			std::vector<std::string>								  interfaceTokens;

			// interface definition provides different values sepearted by ','
			IBK::explode(spaceTypeIt->second, interfaceTokens, ',');
			// now extract all definitions

			for(unsigned int j = 0; j < interfaceTokens.size(); ++j) {
				FMU2QuantityDescription::FMUInterfaceDefinition interfaceDef =
					FMU2QuantityDescription::ID_None;
				// only use trimmed string value
				IBK::trim(interfaceTokens[j]);
				// extract intergface definition
				interfaceDef = (FMU2QuantityDescription::FMUInterfaceDefinition)
					KeywordList::Enumeration(category.c_str(), interfaceTokens[j]);
				// skip interface none
				if(interfaceDef == FMU2QuantityDescription::ID_None)
					continue;
				// store definition
				interfaceDefs.insert(interfaceDef);
			}

			// no export
			if(interfaceDefs.find(FMU2QuantityDescription::ID_HeatingScenario1)
				== interfaceDefs.end())
				continue;

			// store space type name
			exportedZoneWithNames[zoneId] = zone.m_displayName;
			// check if we got area information
			if(zone.m_para[NANDRAD::Zone::ZP_AREA].name.empty() ) {
				throw IBK::Exception(IBK::FormatString("Exported zone with id %1 does not define a zone area!")
					.arg(zoneId), FUNC_ID);
			}
			// store zone area in [m2]
			exportedZoneWithAreas[zoneId] = zone.m_para[NANDRAD::Zone::ZP_AREA].value;
		}
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error generating FMU for all zones from space type interface definition!",
			FUNC_ID);
	}

	std::map<unsigned int, double>		exportedZoneWithHeatingDesign;
	std::map<unsigned int, double>		exportedZoneWithCoolingDesign;
	std::stringstream designHeaderPart;
	// next block: designinformation
	try {
		std::string header;
		IBK::Path designFile = m_impl->m_dirs.m_resultsDir / "DesignDay.tsv";
		// we calculated a design day
		if(designFile.exists() ) {
#ifdef _WIN32
	#if defined(_MSC_VER)
		// read design data from file
		std::wstring designfilestr = designFile.wstr();
		std::ifstream in(designfilestr.c_str());
	#else
		std::string designfilestrAnsi = IBK::WstringToANSI(designFile.wstr(), false);
		std::ifstream in(designfilestrAnsi.c_str());
	#endif
#else
			// write desig data to file
			std::string designfilestr = designFile.str();
			std::ifstream in(designfilestr.c_str()); // UTF8
#endif
			unsigned int zoneId;
			std::vector<std::string> tokens;
			// ignore first line
			std::string line;
			while(std::getline(in,line)) {
				if(line.find("ZoneID") != std::string::npos)
					break;
			}
			// get header
			header = line;
			// register all reimeianing ids
			std::set<unsigned int> remainingIds;

			for(std::map<unsigned int, std::string>::const_iterator it =
				exportedZoneWithNames.begin(); it != exportedZoneWithNames.end(); ++it) {
				remainingIds.insert(it->first);
			}

			while(std::getline(in,line)) {
				// skip empty lines
				if(line.empty())
					continue;
				// now read zone id
				IBK::explode(line, tokens, " \t", 0);
				// tokens size is weong
				if(tokens.size() != 3) {
					throw IBK::Exception("Wrong format for design data file!", FUNC_ID);
				}

				// zone id
				zoneId = IBK::string2val<unsigned int>(tokens[0]);
				// zone was registered alread
				if(exportedZoneWithHeatingDesign.find(zoneId) !=
					exportedZoneWithHeatingDesign.end() ||
					exportedZoneWithCoolingDesign.find(zoneId) !=
					exportedZoneWithCoolingDesign.end()) {
						throw IBK::Exception(IBK::FormatString("Zone with id %1 "
							"is registered twice inside design data file!").arg(zoneId), FUNC_ID);
				}
				// now check if the zone exported:
				std::map<unsigned int, std::string>::iterator fIt =
					exportedZoneWithNames.find(zoneId);
				// no?
				if( fIt ==  exportedZoneWithNames.end() )
					continue;
				// yes!
				exportedZoneWithHeatingDesign[zoneId] =
					IBK::string2val<double>(tokens[1]);
				exportedZoneWithCoolingDesign[zoneId] =
					IBK::string2val<double>(tokens[2]);

				// remove ids from remaining list
				remainingIds.erase(zoneId);
			}
			// did we reach all zones?
			if(!remainingIds.empty()) {

				std::string missigIdString;
				// find the missing zone ids
				for(std::set<unsigned int>::const_iterator idIt = remainingIds.begin();
					idIt != remainingIds.end(); ++idIt) {
					if(idIt != remainingIds.begin()) {
						missigIdString += std::string(",");
					}
					missigIdString +=  IBK::val2string<unsigned int>(*idIt);
				}

				throw IBK::Exception(IBK::FormatString("Incomplete design data file: "
					"no definition of heating and cooling design data for zones with ids %1!")
					.arg(missigIdString), FUNC_ID);
			}
			// create header for design day calculation
			std::string heatingUnit, coolingUnit;
			// retreive units from header
			std::size_t pos1 = header.find("HeatingDesignThermalLoad");
			// wrong header
			 if(pos1 == std::string::npos) {
				throw IBK::Exception(IBK::FormatString("Wrong header inside design data file: "
					"Missing entry 'HeatingDesignThermalLoad'!"), FUNC_ID);
			 }
			// reduce string
			header.erase(0, pos1);
			pos1 = header.find_first_of("[");
			// wrong header
				if(pos1 == std::string::npos) {
				throw IBK::Exception(IBK::FormatString("Wrong header inside design data file: "
					"Missing unit for 'HeatingDesignThermalLoad'!"), FUNC_ID);
				}
			header.erase(0,pos1 + 1);
			heatingUnit = header;
			pos1 = heatingUnit.find_first_of("]");
			// wrong header
			if(pos1 == std::string::npos) {
				throw IBK::Exception(IBK::FormatString("Wrong header inside design data file: "
					"Missing unit for 'HeatingDesignThermalLoad'!"), FUNC_ID);
			}
			heatingUnit.erase(pos1, std::string::npos);
			IBK::trim(heatingUnit);
			// check the second argument
			std::size_t pos2 = header.find("CoolingDesignThermalLoad");
			if(pos2 == std::string::npos) {
				throw IBK::Exception(IBK::FormatString("Wrong header inside design data file: "
					"Missing unit for 'CoolingDesignThermalLoad'!"), FUNC_ID);
			}
			header.erase(0,pos2);
			pos2 = header.find_first_of("[");
			// now second unit
			 if(pos2 == std::string::npos) {
				throw IBK::Exception(IBK::FormatString("Wrong header inside design data file: "
					"Missing entry 'CoolingDesignThermalLoad'!"), FUNC_ID);
			 }
			// wrong header
			header.erase(0,pos2 + 1);

			coolingUnit = header;
			pos2 = coolingUnit.find_first_of("]");
			// wrong header
			if(pos2 == std::string::npos) {
				throw IBK::Exception(IBK::FormatString("Wrong header inside design data file: "
					"Missing unit for 'CoolingDesignThermalLoad'!"), FUNC_ID);
			}
			coolingUnit.erase(pos2, std::string::npos);
			IBK::trim(coolingUnit);

			designHeaderPart << std::setw(30) << std::right << "HeatingDesignThermalLoad [" << heatingUnit << "]" << delimiter;
			designHeaderPart << std::setw(30) << std::right << "CoolingDesignThermalLoad [" << coolingUnit << "]";
		}
	}
	catch(IBK::Exception &ex) {
		throw IBK::Exception(ex, "Error retrieving design information for FMU export!",
			FUNC_ID);
	}

	// write report
#ifdef _WIN32
	#if defined(_MSC_VER)
		std::wstring targetFileStr = targetFile.wstr();
		std::ofstream out(targetFileStr.c_str());
	#else
		std::string targetFileStrAnsi = IBK::WstringToANSI(targetFile.wstr(), false);
		std::ofstream out(targetFileStrAnsi.c_str());
	#endif
#else
	std::string targetFileStr = targetFile.str();
	std::ofstream out(targetFileStr.c_str()); // UTF8
#endif


	std::string areaUnit = NANDRAD::KeywordList::Unit("Zone::para_t",NANDRAD::Zone::ZP_AREA);
	// write information int file
	out << std::setw(10) << std::right << "ZoneIndex" << delimiter;
	out << std::setw(20) << std::right << "ZoneName" << delimiter;
	out << std::setw(7) << std::right << "ZoneID" << delimiter;
	out << std::setw(20) << std::right << "ZoneSurfaceArea [" << areaUnit << "]" << delimiter;

	if(!designHeaderPart.str().empty()) {
		out << std::right << designHeaderPart.str() << delimiter;
	}
	out << std::setw(35) << std::right << "DomesticWaterConsumptionAvailable" << std::endl;


	std::map<unsigned int, std::string>::const_iterator nameIt =
		exportedZoneWithNames.begin();

	unsigned int index = 1;
	for(; nameIt != exportedZoneWithNames.end(); ++nameIt, ++index) {
		unsigned int id = nameIt->first;

		out << std::setw(10) << std::right << index << delimiter;
		out << std::setw(20) << std::right << nameIt->second << delimiter;
		out << std::setw(7) << std::right << id << delimiter;
		out << std::setw(20) << std::right << exportedZoneWithAreas[id] << delimiter;

		// retreive heating and cooling design load
		if(!exportedZoneWithHeatingDesign.empty() &&
			!exportedZoneWithCoolingDesign.empty() ) {
			out << std::setw(30) << std::right << exportedZoneWithHeatingDesign[id] << delimiter;
			out << std::setw(30) << std::right << exportedZoneWithCoolingDesign[id] << delimiter;
		}
		out << std::setw(35) << std::right << 0;
		out << std::endl;
	}
}


void NandradModel::compressFMUFiles(std::vector<IBK::Path> &files,
							const IBK::Path& outputFile,
							const IBK::Path& rootDir,
							int compressionLevel, bool saveFullPath)  const
{
	const char * const FUNC_ID = "[NandradModel::compressFMUFiles]";
	IBK_ASSERT(!files.empty());


	IBK::IBK_Message(IBK::FormatString("Creating FMU archive '%1'.\n").arg(outputFile), IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	// create file path arguments relatively to root directory
	std::vector<char *> args;
	args.resize(3 + files.size());
	IBK::Path relativeOutputPath = outputFile.relativePath(rootDir);
	args[1] = (char *)relativeOutputPath.c_str();
	std::string oArg("-o");
	args[2] = (char *)oArg.c_str();

	std::vector<IBK::Path> relativeFilePathes(files.size());
	for (unsigned int i = 0; i < files.size(); ++i) {
		relativeFilePathes[i] = files[i].relativePath(rootDir);
	}
	for (unsigned int i = 0; i < relativeFilePathes.size(); ++i) {
		args[i + 3] = (char*)relativeFilePathes[i].c_str();
	}

	// change working directory
	IBK::Path workingDir = IBK::Path::current();
	IBK::Path::setCurrent(rootDir);
	// zip files relatively from root directory: this will preserve the internal
	// file structure inside fmu-archive
	minizip((int) args.size(), &args[0]);
	// revert working directory change
	IBK::Path::setCurrent(workingDir);
}


} // namespace NANDRAD_MODEL
