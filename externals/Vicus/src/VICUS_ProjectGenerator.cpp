#include "VICUS_Project.h"

#include <NANDRAD_Project.h>
#include <NANDRAD_Schedule.h>

#include <IBK_algorithm.h>
#include <IBK_math.h>
#include <IBK_physics.h>
#include <IBK_CSVReader.h>
#include <IBK_FluidPhysics.h>
#include <IBK_UnitVector.h>
#include <IBK_StringUtils.h>
#include <IBK_FileUtils.h>

#include <DataIO>

#include <CCM_ClimateDataLoader.h>

#include "VICUS_utilities.h"
#include "VICUS_Constants.h"

#include <fstream>

namespace VICUS {

inline QString MultiLangString2QString(const IBK::MultiLanguageString & mls) {
	return QString::fromStdString(mls.string(IBK::MultiLanguageString::m_language, "en"));
}

/*! Simple valid function. */
template <typename T>
bool isValidTemplate(const std::vector<T> elements, QStringList &errorStack, const QString &name ) {
	bool valid = true;
	for (const  T &e : elements){
		if (!e.isValid() ){
			errorStack << qApp->translate("isValidTemplate", "The %3 with #%1 and name '%2' is not valid! Export failed.")
						  .arg(e.m_id).arg(QString::fromStdString(e.m_displayName.string()).arg(name));
			valid = false;
		}
	}
	return valid;
}


class ModelGeneratorBase {
public:
	ModelGeneratorBase(const VICUS::Project * pro) :
		m_project(pro),
		m_scheduleDB(0),
		m_constructionsDB(0),
		m_materialsDB(0),
		m_boundaryConditionsDB(0)
	{
		m_scheduleDB.setData(pro->m_embeddedDB.m_schedules);
		m_constructionsDB.setData(pro->m_embeddedDB.m_constructions);
		m_materialsDB.setData(pro->m_embeddedDB.m_materials);
		m_boundaryConditionsDB.setData(pro->m_embeddedDB.m_boundaryConditions);
	}

	const AbstractDBElement * findZoneSubTemplate(const Room * r, VICUS::ZoneTemplate::SubTemplateType subType) {
		FUNCID(ModelGeneratorBase::findZoneSubTemplate);
		if (r->m_idZoneTemplate == VICUS::INVALID_ID)
			return nullptr;
		// lookup zone template

		const VICUS::ZoneTemplate * zoneTemplate = VICUS::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);
		if (zoneTemplate == nullptr)
			throw IBK::Exception( qApp->tr("Invalid zone template ID #%1 referenced from zone #%2 '%3'.").arg(r->m_idZoneTemplate)
								  .arg(r->m_id).arg(r->m_displayName).toStdString(), FUNC_ID);

		// check if subType exists
		if (zoneTemplate->m_idReferences[subType] == INVALID_ID)
			return nullptr;

		const AbstractDBElement * subTemplate = nullptr;
		switch (subType) {
		case VICUS::ZoneTemplate::ST_IntLoadPerson:
		case VICUS::ZoneTemplate::ST_IntLoadEquipment:
		case VICUS::ZoneTemplate::ST_IntLoadLighting:
			subTemplate = VICUS::element(m_project->m_embeddedDB.m_internalLoads, zoneTemplate->m_idReferences[subType]);
			break;

		case VICUS::ZoneTemplate::ST_IntLoadOther:
			break;
		case VICUS::ZoneTemplate::ST_ControlThermostat:
			subTemplate = VICUS::element(m_project->m_embeddedDB.m_zoneControlThermostats, zoneTemplate->m_idReferences[subType]);
			break;
		case VICUS::ZoneTemplate::ST_ControlVentilationNatural:
			subTemplate = VICUS::element(m_project->m_embeddedDB.m_zoneControlVentilationNatural, zoneTemplate->m_idReferences[subType]);
			break;
		case VICUS::ZoneTemplate::ST_Infiltration:
			subTemplate = VICUS::element(m_project->m_embeddedDB.m_infiltration, zoneTemplate->m_idReferences[subType]);
			break;
		case VICUS::ZoneTemplate::ST_VentilationNatural:
			subTemplate = VICUS::element(m_project->m_embeddedDB.m_ventilationNatural, zoneTemplate->m_idReferences[subType]);
			break;
		case VICUS::ZoneTemplate::ST_IdealHeatingCooling:
			subTemplate = VICUS::element(m_project->m_embeddedDB.m_zoneIdealHeatingCooling, zoneTemplate->m_idReferences[subType]);
			break;
		case VICUS::ZoneTemplate::ST_ControlShading:
			subTemplate = VICUS::element(m_project->m_embeddedDB.m_zoneControlShading, zoneTemplate->m_idReferences[subType]);
			break;
		case VICUS::ZoneTemplate::NUM_ST:
			break;
		}

		if (subTemplate == nullptr)
			throw IBK::Exception( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
								  .arg(zoneTemplate->m_idReferences[subType])
								  .arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)).toStdString(), FUNC_ID);
		return subTemplate;
	}

	const VICUS::Project *			m_project;
	VICUS::Database<Schedule>		m_scheduleDB;
	VICUS::Database<Construction>	m_constructionsDB;
	VICUS::Database<Material>		m_materialsDB;
	VICUS::Database<BoundaryCondition>	m_boundaryConditionsDB;

	std::map<unsigned int, unsigned int>	m_vicusToNandradIds;
};


class InternalLoadsModelGenerator : public ModelGeneratorBase {
public:
	InternalLoadsModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, std::vector<unsigned int> &usedModelIds, bool enableMoisture, QStringList & errorStack);

	// All definition lists below have the same size and share the same index
	// i.e. m_internalLoadObjects[3] references m_objLists[3] and
	//      m_schedules[3] corresponds also to m_objLists[3]

	std::vector<NANDRAD::InternalLoadsModel>		m_internalLoadObjects;
	std::vector<NANDRAD::InternalMoistureLoadsModel>
	m_internalMoistLoadObjects;
	std::vector<NANDRAD::ObjectList>				m_intLoadObjLists;
	std::vector<std::string>						m_intLoadObjListNames;
	std::vector<NANDRAD::ObjectList>				m_intMoistLoadObjLists;
	std::vector<std::string>						m_intMoistLoadObjListNames;

	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_intLoadObjLists
	std::vector< std::vector<NANDRAD::Schedule> >	m_intLoadSchedules;
	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_intMoistLoadObjLists
	std::vector< std::vector<NANDRAD::Schedule> >	m_intMoistLoadSchedules;
};

class VentilationModelGenerator : public ModelGeneratorBase {
public:

	enum scheduleType_t {
		ST_VentilationRate,
		ST_VentilationRateIncrease,
		ST_VentilationMaxAirTemperature,
		ST_VentilationMinAirTemperature,
		NUM_ST
	};

	VentilationModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, std::vector<unsigned int> &usedModelIds, QStringList & errorStack);

	// All definition lists below have the same size and share the same index

	std::vector<NANDRAD::NaturalVentilationModel>	m_natVentObjects;
	std::vector<NANDRAD::ObjectList>				m_objLists;
	std::vector<std::string>						m_objListNames;

	// store VICUS schedules for similarity check of a new schedule
	// 4 Schedule see scheduleType above
	std::vector< std::vector<Schedule>>				m_generatingSchedules;
	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_objLists
	std::vector< std::vector<NANDRAD::Schedule> >	m_schedules;
};

class ThermostatModelGenerator : public ModelGeneratorBase{
public:
	ThermostatModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, std::vector<unsigned int> &usedModelIds, QStringList & errorStack);

	// All definition lists below have the same size and share the same index

	std::vector<NANDRAD::Thermostat>	m_thermostats;
	std::vector<NANDRAD::ObjectList>	m_objLists;
	std::vector<std::string>			m_objListNames;

	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_objLists
	std::vector< std::vector<NANDRAD::Schedule> >				m_schedules;
	std::vector< std::vector<NANDRAD::LinearSplineParameter>>	m_schedGroupSplines;

	std::map< std::string, IBK::Path>						m_placeholders;

};

class ControlledShadingModelGenerator : public ModelGeneratorBase{
public:


	ControlledShadingModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, std::vector<unsigned int> &usedModelIds, QStringList & errorStack);

	/*! identifies the orientation of the corresponding sensor. */
	enum SensorOrientation {
		S_Horizontal,
		S_North,
		S_East,
		S_South,
		S_West,
		NUM_S
	};

	struct ZoneShadingModel {

		ZoneShadingModel() {}

		std::map<SensorOrientation, NANDRAD::ShadingControlModel>			m_shadingControlModels;
	};

	static SensorOrientation calculateSensorOrientation(const IBKMK::Vector3D & normal, ZoneControlShading::Category category);

	/*! Stores all the models.
		\param key - is zone control model id
		\param entry - SensorShadingModel
	*/
	std::map<unsigned int, ZoneShadingModel>								m_shadingModels;

};

class IdealHeatingCoolingModelGenerator : public ModelGeneratorBase{
public:
	IdealHeatingCoolingModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, std::vector<unsigned int> &usedModelIds, QStringList & errorStack);

	// All definition lists below have the same size and share the same index

	std::vector<NANDRAD::IdealHeatingCoolingModel>	m_idealHeatingCoolings;
	std::vector<NANDRAD::ObjectList>				m_objLists;
	std::vector<std::string>						m_objListNames;

	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_objLists
	//std::vector< std::vector<NANDRAD::Schedule> >	m_schedules;
};

class DataSurfaceHeating {
public:
	DataSurfaceHeating(unsigned int controlledZoneId, unsigned int surfaceHeatingModelId,
					   unsigned int constructionInstanceId, unsigned int externalSuppyId, double area):
		m_controlledZoneId(controlledZoneId),
		m_nandradConstructionInstanceId(constructionInstanceId),
		m_heatingSystemId(surfaceHeatingModelId),
		m_supplySystemId(externalSuppyId),
		m_area(area)
	{}

	unsigned int					m_controlledZoneId;
	unsigned int					m_nandradConstructionInstanceId;
	unsigned int					m_heatingSystemId;
	unsigned int					m_supplySystemId = INVALID_ID;
	double							m_area;

	//key is surface heating model id
	std::map<unsigned int, std::vector<unsigned int>>		m_heatingModelIdToNandradConstructionInstanceIds;
};

class IdealSurfaceHeatingCoolingModelGenerator : public ModelGeneratorBase{
public:
	IdealSurfaceHeatingCoolingModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const std::vector<DataSurfaceHeating> &dataSurfaceHeating, std::vector<unsigned int> &usedModelIds,  QStringList & errorStack);

	// All definition lists below have the same size and share the same index

	std::vector<NANDRAD::IdealSurfaceHeatingCoolingModel>	m_idealSurfaceHeatingCoolings;
	std::vector<NANDRAD::IdealPipeRegisterModel>			m_idealPipeRegister;
	std::vector<NANDRAD::ObjectList>						m_objListsSurface;
	std::vector<NANDRAD::ObjectList>						m_objListsPipe;
	std::vector<std::string>								m_objListNamesSurface;
	std::vector<std::string>								m_objListNamesPipe;

	//std::vector<NANDRAD::LinearSplineParameter>				m_linearSplinePara;
	//std::map<std::string, std::vector<unsigned int>>		m_objListLinearSpline;
	std::vector<NANDRAD::ObjectList>						m_objListLinearSpline;
	std::vector<std::string>								m_objListNamesLinearSplines;
	std::map<std::string, std::vector<NANDRAD::LinearSplineParameter>>	m_constructionIdToNandradSplines;

	std::map< std::string, IBK::Path>						m_placeholders;

	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_objLists
	//std::vector< std::vector<NANDRAD::Schedule> >	m_schedules;
private:

	bool calculateSupplyTemperature(const std::vector<double> &supplySetpoints,const std::vector<double> &outdoorSetpoints,
									const std::vector<double> &outdoorTemperatureSpline, std::vector<double> &supplyTemperature);

};

bool IdealSurfaceHeatingCoolingModelGenerator::calculateSupplyTemperature(const std::vector<double> &supplySetpoints,const std::vector<double> &outdoorSetpoints,
																		  const std::vector<double> &outdoorTemperatureSpline, std::vector<double> &supplyTemperature){
	if(supplySetpoints.size() != outdoorSetpoints.size() && supplySetpoints.size() == 4)
		return false;


	double lowerSupplyHeatLimit = std::min<double>(supplySetpoints[0], supplySetpoints[1]);
	double upperSupplyHeatLimit = std::max<double>(supplySetpoints[0], supplySetpoints[1]);
	double lowerSupplyCoolLimit = std::min<double>(supplySetpoints[2], supplySetpoints[3]);

	double lowerOutHeatLimit = std::min<double>(outdoorSetpoints[0], outdoorSetpoints[1]);
	double upperOutHeatLimit = std::max<double>(outdoorSetpoints[0], outdoorSetpoints[1]);
	double lowerOutCoolLimit = std::min<double>(outdoorSetpoints[2], outdoorSetpoints[3]);
	double upperOutCoolLimit = std::max<double>(outdoorSetpoints[2], outdoorSetpoints[3]);

	if(upperOutHeatLimit > lowerOutCoolLimit)
		return false;

	supplyTemperature.clear();

	double deltaX = outdoorSetpoints[0] - outdoorSetpoints[1];
	double mHeat = 0;
	if(deltaX != 0.)
		mHeat = (supplySetpoints[0] - supplySetpoints[1]) / deltaX;
	double nHeat =  supplySetpoints[0] - mHeat * outdoorSetpoints[0];

	deltaX = outdoorSetpoints[2] - outdoorSetpoints[3];
	double mCool = 0;
	if(deltaX != 0.)
		mCool = (supplySetpoints[2] - supplySetpoints[3]) / deltaX;
	double nCool = supplySetpoints[2] - mCool * outdoorSetpoints[2];

	for(unsigned int i=0; i<outdoorTemperatureSpline.size(); ++i){
		double tOut = outdoorTemperatureSpline[i];
		double tSupply;
		if(tOut <= lowerOutHeatLimit)
			tSupply = upperSupplyHeatLimit;
		else if(tOut > lowerOutHeatLimit && tOut <= upperOutHeatLimit)
			tSupply = mHeat * tOut + nHeat;
		else if(tOut > upperOutHeatLimit && tOut < lowerOutCoolLimit)
			tSupply = lowerSupplyHeatLimit;
		else if(tOut >= lowerOutCoolLimit && tOut < upperOutCoolLimit)
			tSupply = mCool * tOut + nCool;
		else
			tSupply = lowerSupplyCoolLimit;
		supplyTemperature.push_back(tSupply);
	}
	return true;
}

class SupplySystemNetworkModelGenerator : public ModelGeneratorBase{
public:
	SupplySystemNetworkModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const SupplySystem &supply,
				  const std::vector<DataSurfaceHeating> &dataSurfaceHeating,
				  std::vector<unsigned int> &usedModelIds,
				  std::vector<unsigned int> &usedNetworkIds,
				  std::vector<unsigned int> &usedNetworkElementIds,
				  QStringList &errorStack);

	// All definition lists below have the same size and share the same index

	std::vector<NANDRAD::HydraulicNetwork>					m_hydraulicNetworks;
	std::vector<NANDRAD::ObjectList>						m_objLists;
	std::map<std::string, std::vector<NANDRAD::Schedule> >	m_scheduleGroups;
	std::map<std::string, std::vector<NANDRAD::LinearSplineParameter>> m_annualSchedules;
	std::vector<NANDRAD::HeatLoadSummationModel>			m_summationModels;
	std::vector<NANDRAD::NetworkInterfaceAdapterModel>		m_adapterModels;
	std::map< std::string, IBK::Path>						m_placeholders;

	// FMU descriptions for connection of an external supply
	std::vector<NANDRAD::FMIVariableDefinition>				m_inputVariables;
	std::vector<NANDRAD::FMIVariableDefinition>				m_outputVariables;
};

class ConstructionInstanceModelGenerator : public ModelGeneratorBase {
public:

	ConstructionInstanceModelGenerator(const VICUS::Project *pro):
		ModelGeneratorBase(pro)
	{}

	/*! Add all object lists for adding new list later.*/
	void addInputData(const std::vector<NANDRAD::ObjectList> &objLists);

	/*! Generate all Construction Instances. */
	void generate(const std::vector<ComponentInstance> &componentInstances, std::map<unsigned int, unsigned int> &componentInstanceMappings,
				  QStringList &errorStack, std::set<unsigned int> &idSet);

	void generateMaterials();
	void generateConstructions(QStringList &errorStack);

	struct ActiveLayer{

		//ActiveLayer (unsigned int vicusConstructionId, unsigned int )

		unsigned int								m_vicusConstructionId;			//this is the id of the construction (VICUS)
		std::vector<unsigned int>					m_nandradConstructionTypeId;	//new id is set to get a unique construction type id WITH active layer for NANDRAD
		//all types of this construction with active layers; -1 -> no active layer otherwise this is the layer index
		std::vector<int>							m_activeLayers;
	};

	struct GroundZoneExport{
		std::map<int, unsigned int>				m_tempToZoneId;			// key: temperature value in C * 10;	value: zone id
		std::map<unsigned int, unsigned int>	m_tempSchedIdToZoneId;	// key: temperature schedule id;		value: zone id
		std::set<unsigned int>			m_idSet;						// holds all used ids
		NANDRAD::Zone					m_groundZ;						// ground zone
		std::vector<NANDRAD::Schedule>	m_schedules;					// temperature schedules
		NANDRAD::LinearSplineParameter	m_spline;						// linear spline of temperature (annual)
		std::set<QString>				m_objListNames;					// object list name
		NANDRAD::ObjectList				m_objList;
	};

	unsigned int getNandradConstructionTypeId(unsigned int vicusConstructionId, int activeLayerIdx = -1){
		//search if this construction is already included,
		//check if active layer idx also included
		for(unsigned int i=0; i<m_constructionTypeData.size(); ++i){
			ActiveLayer &aL = m_constructionTypeData[i];
			if(aL.m_vicusConstructionId == vicusConstructionId){
				for(unsigned int j=0; j<aL.m_activeLayers.size(); ++j){
					//if we find an active layer we also have a nandrad construction type id already
					if(aL.m_activeLayers[j] == activeLayerIdx)
						return aL.m_nandradConstructionTypeId[j];
				}

				//we found no active layer so add the layer and get a new id for nandrad
				aL.m_activeLayers.push_back(activeLayerIdx);
				aL.m_nandradConstructionTypeId.push_back(checkFreeId(1));
				return aL.m_nandradConstructionTypeId.back();
			}
		}
		//so we found nothing --> add all ...
		//add this construction type data
		ActiveLayer actLayer;
		actLayer.m_activeLayers.push_back(activeLayerIdx);
		actLayer.m_nandradConstructionTypeId.push_back(checkFreeId(1));
		actLayer.m_vicusConstructionId = vicusConstructionId;
		m_constructionTypeData.push_back(actLayer);
		return 	actLayer.m_nandradConstructionTypeId.back();
	}

	std::vector<ActiveLayer>						m_constructionTypeData;

	std::vector<DataSurfaceHeating>					m_surfaceHeatingData;

	std::vector<NANDRAD::ConstructionInstance>		m_constructionInstances;
	std::vector<NANDRAD::Material>					m_materials;
	std::vector<NANDRAD::ConstructionType>			m_constructions;
	std::vector<NANDRAD::WindowGlazingSystem>		m_windowGlazingSystems;

	std::map<unsigned int, ControlledShadingModelGenerator::ZoneShadingModel> * m_shadingModels = nullptr;

	// all ground zone exports: schedules (periods or spline), zones, object lists
	std::map<std::string,std::vector<NANDRAD::Schedule>>				m_schedsMap;
	std::map<std::string,std::vector<NANDRAD::LinearSplineParameter>>	m_splinesMap;
	std::vector<NANDRAD::Zone>											m_zones;
	std::vector<NANDRAD::ObjectList>									m_objectLists;
	std::set<QString>													m_objectListNames;
	std::map< std::string, IBK::Path>									m_placeholders;

	/*! Map of VICUS surface/sub-surface ids to NANDRAD construction instance/embedded object ids.
		These ids are kept in the header of the shading file for later replacement of the ids.
	*/
	std::map<unsigned int, unsigned int>								m_surfaceIdsVicusToNandrad;

private:

	NANDRAD::Interface generateInterface(const VICUS::ComponentInstance & ci, unsigned int bcID,
										 unsigned int interfaceID,  QStringList &errorStack,
										 GroundZoneExport &gze,
										 bool takeASide = true) const;

	void exportSubSurfaces(QStringList & errorStack, const std::vector<VICUS::SubSurface> &subSurfs,
						   const VICUS::ComponentInstance & ci, NANDRAD::ConstructionInstance &cinst, std::set<unsigned int> &idSet,
						   std::map<unsigned int, unsigned int> &surfaceIdsVicusToNandrad) const;

	// get free id for new nandrad construction types
	unsigned int checkFreeId(unsigned int id){

		for(unsigned int i=0; i<m_constructionTypeData.size(); ++i){
			ActiveLayer &aL = m_constructionTypeData[i];
			for(unsigned int j=0; j<aL.m_nandradConstructionTypeId.size(); ++j){
				if(aL.m_nandradConstructionTypeId[j] == id)
					return checkFreeId(++id);
			}
		}
		return id;
	}
};


bool Project::generateShadingFactorsFile(const std::map<unsigned int, unsigned int> &surfaceIdsVicusToNandrad,
										 const IBK::Path & projectFilePath, IBK::Path & shadingFactorFilePath) const
{
	FUNCID(Project::generateShadingFactorsFile);
	IBK::Path basePath(projectFilePath.withoutExtension() + "_shadingFactors");

	// search for available shading file, and if there are several matching shading files, issue an error
	bool found = false;
	IBK::Path vicusShadingFilePath;
	for (const char * const ext : {"tsv", "d6o", "d6b"}) {
		IBK::Path fullPath(basePath.str() + "." + ext);
		if (fullPath.exists()) {
			if (found) {
				IBK::IBK_Message(IBK::FormatString("Found shading factor file '%1', "
												   "but another matching file '%2' exists already!")
								 .arg(fullPath).arg(vicusShadingFilePath), IBK::MSG_ERROR);
				return false;
			}
			found = true;
			vicusShadingFilePath = fullPath;
		}
	}

	if (!found) {
		IBK::IBK_Message(IBK::FormatString("No shading file exported, expected a file '%1'[.tsv, .d6o, .d6b]. Running simulation without shading factors.")
						 .arg(basePath), IBK::MSG_PROGRESS, FUNC_ID);
		return true;
	}
	std::string ext = vicusShadingFilePath.extension();
	// compose path to NANDRAD shading factor file
	shadingFactorFilePath = vicusShadingFilePath.withoutExtension() + "Nandrad." + ext;

	// we only need to convert the shading factors file, if the VICUS file has a newer time stamp
	// Note: for additional security, we could store the original file's hash in the DataIO container and compare that as well
	if (shadingFactorFilePath.exists() && vicusShadingFilePath.lastWriteTime() < shadingFactorFilePath.lastWriteTime()) {
		IBK::IBK_Message(IBK::FormatString("NANDRAD shading factor file '%1' newer than original VICUS shading factor file '%2'. Shading file generation skipped.\n")
						 .arg(shadingFactorFilePath).arg(vicusShadingFilePath), IBK::MSG_PROGRESS);
		return true;
	}

	// depending on the file extensions, we first parse the data into memory and then dump it back out into a suitable
	// NANDRAD input file

	IBK::IBK_Message(IBK::FormatString("Reading shading factor file '%1'\n").arg(vicusShadingFilePath), IBK::MSG_PROGRESS);
	if (ext == "tsv") {
		IBK::CSVReader csv;
		try {
			csv.read(vicusShadingFilePath, false, true); // headerOnly=false, extractUnits=true
			// general error handling
			if (csv.m_nColumns == 0 || csv.m_nRows == 0)
				throw IBK::Exception("Missing data in shading factor file.", FUNC_ID);
			if (csv.m_captions[0] != "Time")
				throw IBK::Exception("Invalid caption of time column.", FUNC_ID);
			IBK::Unit u;
			try {
				u = IBK::Unit(csv.m_units[0]);
			}
			catch (...) { /* do nothing */ }
			if (u.base_id() != IBK_UNIT_ID_SECONDS)
				throw IBK::Exception("Invalid unit of time column, a valid time unit is required.", FUNC_ID);
		} catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			IBK::IBK_Message(IBK::FormatString("Error reading shading factors file '%1'.").arg(vicusShadingFilePath), IBK::MSG_ERROR);
			return false;
		}

		// we now create mapping vector, that relates new column index to original column index in the csv file
		// and also we populate the nums vector; this is needed because the original shading factors file
		// contains additional columns like "Azimuth" and "Altitude", which we need to skip
		std::vector<unsigned int> colIndices;

		// process all columns, but skip the mandatory time column
		bool haveError = false;
		std::string headerLine = "Time [" + csv.m_units[0] + "]";
		colIndices.push_back(0); // we always keep column 0 with the time
		for (unsigned int i=1; i<csv.m_captions.size(); ++i) {
			std::string cap = csv.m_captions[i];
			// skip Aziumuth and Altitude
			if (cap == "Azimuth" || cap == "Altitude")
				continue;
			unsigned int id = VICUS::INVALID_ID;
			std::string displayName;
			try {
				// when reading tsv-files, the headers have format "<id>" or "<id> '<display name>'"
				std::string::size_type pos = cap.find_first_of(" '");
				if (pos != std::string::npos)
					cap = cap.substr(0, pos);
				id = IBK::string2val<unsigned int>(cap); // might throw
				// lookup displayname of referenced VICUS component instances
				const VICUS::Surface * surf = surfaceByID(id);
				const VICUS::SubSurface * subSurf = subSurfaceByID(id);
				if (surf == nullptr && subSurf == nullptr) // can happen, if shading file is out of date and still references an invalid (now missing) surface ID
					throw IBK::Exception("Invalid surface/sub-surface ID", FUNC_ID);
				if (surf != nullptr)
					displayName = surf->m_displayName.toStdString();
				else
					displayName = subSurf->m_displayName.toStdString();
			} catch (...) {
				IBK::IBK_Message( IBK::FormatString("  Invalid surface ID '%1' in shading factor file.").arg(cap), IBK::MSG_ERROR);
				haveError = true;
				continue;
			}
			// lookup id in id-map
			std::map<unsigned int, unsigned int>::const_iterator nandradSurfIt = surfaceIdsVicusToNandrad.find(id);
			if (nandradSurfIt == surfaceIdsVicusToNandrad.end()) {
				IBK::IBK_Message( IBK::FormatString("  Invalid/unknown surface ID '%1' in shading factor file.").arg(id), IBK::MSG_ERROR);
				haveError = true;
				continue;
			}
			colIndices.push_back(i);

			// store NANDRAD ID in file header line followed by display name;  NANDRAD parses the headers and expects first an ID number in the header label
			headerLine += "\t" + IBK::val2string(nandradSurfIt->second) + " '" + displayName + "' [" + csv.m_units[i] + "]";
		}

		// error check
		if (haveError) {
			// if we got errors we can not assign shading factors
			IBK::IBK_Message(IBK::FormatString("There are invalid column headers!"), IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
			return false;
		}

		// write file
		std::ofstream out;
		if (!IBK::open_ofstream(out, shadingFactorFilePath) ) {
			IBK::IBK_Message(IBK::FormatString("Error writing shading file '%1'.").arg(shadingFactorFilePath), IBK::MSG_ERROR, FUNC_ID);
			return false;
		}

		out << headerLine << '\n';

		for (unsigned int row=0; row<csv.m_nRows; ++row) {
			for (unsigned int col=0; col<colIndices.size(); ++col) {
				if (col != 0)
					out << "\t";
				unsigned int originalColIndex = colIndices[col];
				out << csv.m_values[row][originalColIndex];
			}
			out << "\n";
		}

		out.close();
	}
	else {
		// read original DataIO file
		DATAIO::DataIO shadingFile;
		try {
			shadingFile.read(vicusShadingFilePath, false);
			// force conversion from strings to doubles
			for (unsigned int i=0; i<shadingFile.m_timepoints.size(); ++i)
				shadingFile.data(i);
		} catch (IBK::Exception & ex) {
			ex.writeMsgStackToError();
			IBK::IBK_Message(IBK::FormatString("Error reading shading factors file '%1'.").arg(vicusShadingFilePath), IBK::MSG_ERROR);
			return false;
		}

		// we only need to process the m_nums line, substitute the IDs and also generate the quantities line
		std::string newQuantities;
		std::vector<unsigned int> newNums;
		for (unsigned int i=0; i<shadingFile.m_nums.size(); ++i) {
			unsigned int id = shadingFile.m_nums[i];
			std::map<unsigned int, unsigned int>::const_iterator nandradSurfIt = surfaceIdsVicusToNandrad.find(id);
			if (nandradSurfIt == surfaceIdsVicusToNandrad.end()) {
				IBK::IBK_Message( IBK::FormatString("Invalid/unknown surface ID '%1' in shading factor file.").arg(id), IBK::MSG_ERROR);
				return false;
			}
			const VICUS::Surface * surf = surfaceByID(id);
			const VICUS::SubSurface * subSurf = subSurfaceByID(id);
			if (surf == nullptr && subSurf == nullptr) { // can happen, if shading file is out of date and still references an invalid (now missing) surface ID
				IBK::IBK_Message( IBK::FormatString("Invalid/unknown surface ID '%1' in shading factor file.").arg(id), IBK::MSG_ERROR);
				return false;
			}
			std::string displayName;
			if (surf != nullptr)
				displayName = surf->m_displayName.toStdString();
			else
				displayName = subSurf->m_displayName.toStdString();
			// now substitute the ID and compose the quantity string
			if (i != 0)
				newQuantities += " | ";
			if (!displayName.empty())
				newQuantities += displayName;
			else
				newQuantities += IBK::val2string(nandradSurfIt->second);
			newNums.push_back(nandradSurfIt->second);
		}

		shadingFile.m_type = DATAIO::DataIO::T_REFERENCE;
		shadingFile.m_valueUnit = "---"; // unitless (convertible to 1, %)
		shadingFile.m_projectFileName = projectFilePath.str();
		shadingFile.m_quantityKeyword = "ShadingFactor";
		shadingFile.m_spaceType = DATAIO::DataIO::ST_SINGLE;
		shadingFile.m_timeType = DATAIO::DataIO::TT_NONE;
		//		shadingFile.m_startYear = 2007; // TODO : shouldn't we insert the correct start year here?
		shadingFile.m_filename = shadingFactorFilePath;
		shadingFile.m_nums = newNums;
		shadingFile.m_quantity = newQuantities;

		// now write DataIO file
		shadingFile.write();
	}
	IBK::IBK_Message(IBK::FormatString("Created shading factors data file '%1' in NANDRAD format\n").arg(shadingFactorFilePath), IBK::MSG_PROGRESS, FUNC_ID);

	return true;
}


bool Project::exportMappingTable(const IBK::Path &filepath, const std::vector<RoomMapping> &mappings,
								 bool addFloorAreaAndVolume) const
{
	FUNCID(Project::exportMappingTable);
	IBK::Path basePath(filepath.withoutExtension() + "_mappingTable.txt");

	std::ofstream out;
	if (!IBK::open_ofstream(out, basePath)) {
		IBK::IBK_Message(IBK::FormatString("Error writing mapping file '%1'.").arg(basePath), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	out << "VICUS room id\tNANDRAD room id\tVICUS building name\tVICUS building level name\tVICUS room name\tNANDRAD room name\tzone template id\tzone template name" ;

	if(addFloorAreaAndVolume)
		out <<  "\tFloor area [m2]\tVolume [m3]";

	out << std::endl;
	for(const auto &m : mappings){
		out << m.m_idRoomVicus << "\t" << m.m_idRoomNandrad << "\t" << m.m_nameBuildingVicus << "\t" << m.m_nameBuildingLevelVicus << "\t" << m.m_nameRoomVicus << "\t" << m.m_nameRoomNandrad << "\t";
		if(m.m_idZoneTemplateVicus){
			out << m.m_idZoneTemplateVicus << "\t";
			out << m.m_zonetemplateName;
		}
		else
			out << "\t";

		if(addFloorAreaAndVolume)
			out << "\t" << IBK::FormatString("%1\t%2").arg(m.m_floorArea,0,'f',2).arg(m.m_volume,0,'f',2);
		out << std::endl;
	}

	out << "VICUS component instance id\tNANDRAD construction instance id" << std::endl;
	// Todo later
	out.close();
	return true;
}

bool Project::exportAreaAndVolume() {
	return true;
}

void Project::addViewFactorsToNandradZones(NANDRAD::Project & p, const std::vector<Project::RoomMapping> &roomMappings, const std::map<unsigned int, unsigned int> &componentInstanceMapping,
										   const std::map<unsigned int, unsigned int> &vicusToNandradIdMapping, QStringList & errorStack) const {
	for(NANDRAD::Zone & z : p.m_zones){
		RoomMapping rM;
		// select the correct mapping

		// TODO Dirk: Replace these Mappings with maps
		for(const RoomMapping & m : roomMappings){
			if(m.m_idRoomNandrad == z.m_id){
				rM = m;
				break;
			}
		}
		const VICUS::Room * room = roomByID(rM.m_idRoomVicus);

		// save view factors in zone
		std::vector<VICUS::Surface> surfaces = room->m_surfaces;
		for(unsigned int i = 0; i < surfaces.size(); i++){

			if(surfaces[i].geometry().area() < VICUS::MIN_AREA_FOR_EXPORTED_SURFACES)
				continue; // skip too small surfaces

			unsigned int compInstId1 = surfaces[i].m_componentInstance->m_id;
			// get the nandrad ids
			if(componentInstanceMapping.find(compInstId1) == componentInstanceMapping.end()) {
				errorStack.append(tr("Could not find component instance #%1 for view-factor generation!").arg(compInstId1));
				return;
			}

			unsigned int constInstId1Nandrad = componentInstanceMapping.at(compInstId1);

			for(unsigned int j = 0; j < surfaces.size(); j++){

				if(surfaces[j].geometry().area() < VICUS::MIN_AREA_FOR_EXPORTED_SURFACES)
					continue; // skip too small surfaces

				// the view factor from surface[i] to surface[j]
				if(surfaces[i].m_componentInstance == nullptr || surfaces[j].m_componentInstance == nullptr){
					QString name = (surfaces[i].m_componentInstance == nullptr) ? surfaces[i].m_displayName : surfaces[j].m_displayName;
					errorStack.append(tr("Invalid Surface (%1)! Has no component instace!").arg(name));
					return;
				}
				unsigned int compInstId2 = surfaces[j].m_componentInstance->m_id;

				if(compInstId1 == compInstId2)
					continue;

				if(surfaces[i].m_viewFactors.m_values.empty()) {
					errorStack.append(tr("Invalid view factors of surface '%1'. Please re-run view-factor generation.").arg(surfaces[i].m_displayName));
					return;
				}

				std::map<unsigned int, std::vector<double>>::const_iterator itSurf = surfaces[i].m_viewFactors.m_values.find(surfaces[j].m_id);
				if (itSurf == surfaces[i].m_viewFactors.m_values.end()) {
					errorStack.append(tr("Missing view-factors between surface '%1' and surface '%2'! Please re-run view-factor generation!").arg(surfaces[i].m_displayName).arg(surfaces[j].m_displayName));
					return;
				}
				if (itSurf->second.empty()) {
					errorStack.append(tr("Missing view-factors between surface '%1' and surface '%2'! Please re-run view-factor generation!").arg(surfaces[i].m_displayName).arg(surfaces[j].m_displayName));
					return;
				}

				double vF = itSurf->second[0];

				if(componentInstanceMapping.find(compInstId2) == componentInstanceMapping.end()) {
					errorStack.append(tr("Could not find component instance #%1 for view-factor generation!").arg(compInstId2));
					return;
				}

				unsigned int constInstId2Nandrad = componentInstanceMapping.at(compInstId2);

				NANDRAD::Zone::viewFactorPair idPair1 = std::pair<unsigned int, unsigned int>(constInstId1Nandrad, constInstId2Nandrad);
				z.m_viewFactors.push_back(std::pair<NANDRAD::Zone::viewFactorPair, double>(idPair1, vF));

				// First we scan all concections between sub-surface and its parent wall (always 0)
				// Than we also scan for other sub-surfaces inside walls
				for(unsigned int l=0; l < surfaces[j].subSurfaces().size(); ++l) {
					const VICUS::SubSurface &subSurf1 = surfaces[j].subSurfaces()[l];

					// All sub-surfaces <-> sub-surfaces have to be connected
					for (unsigned int k=0; k < surfaces[i].subSurfaces().size(); ++k) {
						const VICUS::SubSurface &subSurf2 = surfaces[i].subSurfaces()[k];

						if (subSurf1.m_viewFactors.m_values.empty())
							continue;

						std::map<unsigned int, std::vector<double>>::const_iterator itSubSurf2 = subSurf1.m_viewFactors.m_values.find(subSurf2.m_id);
						if (itSubSurf2 == subSurf1.m_viewFactors.m_values.end()) {
							errorStack.append(tr("Missing view-factors between surface '%1' and surface '%2'. Please re-run view-factor generation.").arg(subSurf1.m_displayName).arg(subSurf2.m_displayName));
							return;
						}

						if (itSubSurf2->second.empty()) {
							errorStack.append(tr("Missing view-factors between surface '%1' and surface '%2'. Please re-run view-factor generation.").arg(subSurf1.m_displayName).arg(subSurf2.m_displayName));
							return;
						}

						double vFSubSurf2 = itSubSurf2->second[0];

						NANDRAD::Zone::viewFactorPair idPair2 = std::pair<unsigned int, unsigned int>(vicusToNandradIdMapping.at(subSurf1.m_id), vicusToNandradIdMapping.at(subSurf2.m_id));
						z.m_viewFactors.push_back(std::pair<NANDRAD::Zone::viewFactorPair, double>(idPair2, vFSubSurf2));
					}

					if(surfaces[i].m_viewFactors.m_values.empty())
						continue;

					std::map<unsigned int, std::vector<double>>::const_iterator  itSubSurf1 = surfaces[i].m_viewFactors.m_values.find(subSurf1.m_id);
					if (itSubSurf1 == surfaces[i].m_viewFactors.m_values.end()) {
						errorStack.append(tr("Missing view-factors between surface '%1' and surface '%2'. Please re-run view-factor generation.").arg(surfaces[i].m_displayName).arg(subSurf1.m_displayName));
						return;
					}
					if (itSubSurf1->second.empty()) {
						errorStack.append(tr("Missing view-factors between surface '%1' and surface '%2'. Please re-run view-factor generation.").arg(surfaces[i].m_displayName).arg(subSurf1.m_displayName));
						return;
					}

					double vFSubSurf1 = itSubSurf1->second[0];

					NANDRAD::Zone::viewFactorPair idPair3 = std::pair<unsigned int, unsigned int>(constInstId1Nandrad, vicusToNandradIdMapping.at(subSurf1.m_id));
					z.m_viewFactors.push_back(std::pair<NANDRAD::Zone::viewFactorPair, double>(idPair3, vFSubSurf1));
				}
			}
		}
	}
}


void Project::generateBuildingProjectData(const QString &modelName, NANDRAD::Project & p, QStringList & errorStack,
										  std::map<unsigned int, unsigned int> &surfaceIdsVicusToNandrad,
										  std::vector<RoomMapping> &roomMappings, std::map<unsigned int, unsigned int> componentInstanceMapping) const{

	// First mandatory input data checks.
	// We rely on unique IDs being used in the VICUS model
	// Hence, we first check for IDs and inform user about duplicate ID.

	// *** Schedules ***

	// check for unique IDs
	std::set<unsigned int> scheduleIdSet;
	std::map<unsigned int, unsigned int> vicusToNandradIds;
	try {
		checkForUniqueIDs(m_embeddedDB.m_schedules, scheduleIdSet);
	} catch (...) {
		errorStack.append(tr("Duplicate schedule IDs found."));
		return;
	}
	for (const Schedule & sched : m_embeddedDB.m_schedules) {
		std::string err;
		if (!sched.isValid(err, true, p.m_placeholders))
			errorStack.append(tr("Schedule #%1 '%2' is invalid.").arg(sched.m_id).arg(MultiLangString2QString(sched.m_displayName)));
	}
	if (!errorStack.isEmpty())	return;

	// *** Zones ***
	std::vector<unsigned int> usedModelIds;
	std::set<unsigned int> idSet;
	std::vector<const VICUS::Room *> zones;

	// add all the sensors in the ids
	for(unsigned int i = 0; i < ControlledShadingModelGenerator::NUM_S; i++){
		idSet.insert(2000000 + i); // TODO Dirk derzeitige sensor id entfernen wenn der Sensor Dialog fertig ist
	}


	generateNandradZones(zones, idSet, p, errorStack, roomMappings);

	if (!errorStack.isEmpty())	return;

	// *** Generate controlled shadings ***
	ControlledShadingModelGenerator controlledShading(this);
	for (const VICUS::Room * r : zones)
		controlledShading.generate(r, usedModelIds, errorStack);


	// *** Create Construction Instances, Constructions (opak & tranparent) and materials ***

	ConstructionInstanceModelGenerator constrInstaModelGenerator(this);
	// init with shadingModels
	constrInstaModelGenerator.m_shadingModels = & controlledShading.m_shadingModels;
	constrInstaModelGenerator.m_placeholders = p.m_placeholders;
	constrInstaModelGenerator.addInputData(p.m_objectLists);
	constrInstaModelGenerator.generate(m_componentInstances, componentInstanceMapping, errorStack, idSet);
	constrInstaModelGenerator.generateMaterials();
	constrInstaModelGenerator.generateConstructions(errorStack);

	surfaceIdsVicusToNandrad.swap(constrInstaModelGenerator.m_surfaceIdsVicusToNandrad);

	// *** Ideal Surface Heating Systems ***

	//TODO Dirk add annual schedules to nandrad
	IdealSurfaceHeatingCoolingModelGenerator idealSurfaceHeatCoolGenerator(this);
	idealSurfaceHeatCoolGenerator.m_placeholders = p.m_placeholders;
	idealSurfaceHeatCoolGenerator.generate(constrInstaModelGenerator.m_surfaceHeatingData, usedModelIds, errorStack);
	if (!errorStack.isEmpty())	return;


	// *** Surface Heating Systems with external supply ***
	std::map<unsigned int, std::vector<DataSurfaceHeating> > supplyIdToSurfaceHeatings;

	for(DataSurfaceHeating &surfHeat : constrInstaModelGenerator.m_surfaceHeatingData) {
		if(surfHeat.m_supplySystemId == INVALID_ID)
			continue;
		supplyIdToSurfaceHeatings[surfHeat.m_supplySystemId].push_back(surfHeat);
	}

	// create networks for all used supplies
	std::vector<unsigned int> usedNetworkIds;
	// exclude all ids that are explicitely assigned by Vicus Networks
	for (const VICUS::Network &net: m_geometricNetworks) {
		usedNetworkIds.push_back(net.m_id);
	}

	// vector that holds all network element ids, will be filled within the loop
	std::vector<unsigned int> usedNetworkElementIds;

	SupplySystemNetworkModelGenerator supplySystemNetworkModelGenerator(this);
	supplySystemNetworkModelGenerator.m_placeholders = p.m_placeholders;

	// at the moment, we generate a new network element id for all generic networks and predfined networks
	for(std::map<unsigned int, std::vector<DataSurfaceHeating> >::iterator it = supplyIdToSurfaceHeatings.begin(); it != supplyIdToSurfaceHeatings.end(); ++it) {
		// find supply component
		for(const VICUS::SupplySystem &supplySystem : m_embeddedDB.m_supplySystems) {
			if(supplySystem.m_id == it->first) {
				std::vector<DataSurfaceHeating> &surfaceHeatings = it->second;
				supplySystemNetworkModelGenerator.generate(supplySystem, surfaceHeatings, usedModelIds, usedNetworkIds, usedNetworkElementIds, errorStack);
				break;
			}
		}

		if (!errorStack.isEmpty())	return;
	}


	// *** Models based on zone templates ***

	// process all zones
	InternalLoadsModelGenerator internalLoads(this);
	VentilationModelGenerator ventilation(this);
	IdealHeatingCoolingModelGenerator idealHeatCool(this);
	ThermostatModelGenerator thermostats(this);
	thermostats.m_placeholders = p.m_placeholders;

	bool enableMoist = p.m_simulationParameter.m_flags[NANDRAD::SimulationParameter::F_EnableMoistureBalance].isEnabled();
	for (const VICUS::Room * r : zones) {
		internalLoads.generate(r, usedModelIds, enableMoist, errorStack);
		ventilation.generate(r, usedModelIds, errorStack);
		idealHeatCool.generate(r, usedModelIds, errorStack);
		thermostats.generate(r, usedModelIds, errorStack);
	}
	if (!errorStack.isEmpty())	return;

	// transfer data to project

	// *** InternalLoads ***
	p.m_models.m_internalLoadsModels = internalLoads.m_internalLoadObjects;
	p.m_models.m_internalMoistureLoadsModels = internalLoads.m_internalMoistLoadObjects;
	p.m_objectLists.insert(p.m_objectLists.end(), internalLoads.m_intLoadObjLists.begin(), internalLoads.m_intLoadObjLists.end());
	p.m_objectLists.insert(p.m_objectLists.end(), internalLoads.m_intMoistLoadObjLists.begin(), internalLoads.m_intMoistLoadObjLists.end());

	for (unsigned int i=0; i<internalLoads.m_intLoadSchedules.size(); ++i)
		p.m_schedules.m_scheduleGroups[internalLoads.m_intLoadObjLists[i].m_name] = internalLoads.m_intLoadSchedules[i];
	// Note: if moisture is not enabled, m_intMoistLoadSchedules will be empty
	for (unsigned int i=0; i<internalLoads.m_intMoistLoadSchedules.size(); ++i)
		p.m_schedules.m_scheduleGroups[internalLoads.m_intMoistLoadObjLists[i].m_name] =  internalLoads.m_intMoistLoadSchedules[i];

	// *** Ventilation ***
	p.m_models.m_naturalVentilationModels = ventilation.m_natVentObjects;
	p.m_objectLists.insert(p.m_objectLists.end(), ventilation.m_objLists.begin(), ventilation.m_objLists.end());
	for (unsigned int i=0; i<ventilation.m_schedules.size(); ++i)
		p.m_schedules.m_scheduleGroups[ventilation.m_objLists[i].m_name] = ventilation.m_schedules[i];

	// *** Thermostat ***
	p.m_models.m_thermostats = thermostats.m_thermostats;
	p.m_objectLists.insert(p.m_objectLists.end(), thermostats.m_objLists.begin(), thermostats.m_objLists.end());
	for (unsigned int i=0; i<thermostats.m_schedules.size(); ++i)
		if(!thermostats.m_schedules[i].empty())
			p.m_schedules.m_scheduleGroups[thermostats.m_objLists[i].m_name] = thermostats.m_schedules[i];
	for(unsigned int i=0; i<thermostats.m_schedGroupSplines.size(); ++i)
		if(!thermostats.m_schedGroupSplines[i].empty())
			p.m_schedules.m_annualSchedules[thermostats.m_objLists[i].m_name] = thermostats.m_schedGroupSplines[i];

	// *** Controlled Shading ***
	for(std::pair<unsigned int, ControlledShadingModelGenerator::ZoneShadingModel> shadingModels : controlledShading.m_shadingModels){
		for(auto shadingModel : shadingModels.second.m_shadingControlModels){
			p.m_models.m_shadingControlModels.push_back(shadingModel.second);
		}
	}

	// *** Ideal heating cooling ***
	p.m_models.m_idealHeatingCoolingModels = idealHeatCool.m_idealHeatingCoolings;
	p.m_objectLists.insert(p.m_objectLists.end(), idealHeatCool.m_objLists.begin(), idealHeatCool.m_objLists.end());

	// *** Construction Instances, Constructions (opak & tranparent) and materials ***
	p.m_constructionInstances = constrInstaModelGenerator.m_constructionInstances;
	p.m_constructionTypes = constrInstaModelGenerator.m_constructions;
	p.m_materials = constrInstaModelGenerator.m_materials;
	p.m_windowGlazingSystems = constrInstaModelGenerator.m_windowGlazingSystems;
	// for new ground zones
	p.m_objectLists.insert(p.m_objectLists.end(), constrInstaModelGenerator.m_objectLists.begin(), constrInstaModelGenerator.m_objectLists.end());
	p.m_schedules.m_scheduleGroups.insert(constrInstaModelGenerator.m_schedsMap.begin(), constrInstaModelGenerator.m_schedsMap.end());
	p.m_schedules.m_annualSchedules.insert(constrInstaModelGenerator.m_splinesMap.begin(), constrInstaModelGenerator.m_splinesMap.end());
	p.m_zones.insert(p.m_zones.end(), constrInstaModelGenerator.m_zones.begin(), constrInstaModelGenerator.m_zones.end());

	// *** Ideal surface heating cooling ... ***
	p.m_models.m_idealSurfaceHeatingCoolingModels = idealSurfaceHeatCoolGenerator.m_idealSurfaceHeatingCoolings;
	p.m_models.m_idealPipeRegisterModels = idealSurfaceHeatCoolGenerator.m_idealPipeRegister;
	p.m_objectLists.insert(p.m_objectLists.end(), idealSurfaceHeatCoolGenerator.m_objListsSurface.begin(), idealSurfaceHeatCoolGenerator.m_objListsSurface.end());
	p.m_objectLists.insert(p.m_objectLists.end(), idealSurfaceHeatCoolGenerator.m_objListsPipe.begin(), idealSurfaceHeatCoolGenerator.m_objListsPipe.end());
	p.m_objectLists.insert(p.m_objectLists.end(), idealSurfaceHeatCoolGenerator.m_objListLinearSpline.begin(), idealSurfaceHeatCoolGenerator.m_objListLinearSpline.end());
	p.m_schedules.m_annualSchedules.insert(idealSurfaceHeatCoolGenerator.m_constructionIdToNandradSplines.begin(), idealSurfaceHeatCoolGenerator.m_constructionIdToNandradSplines.end());

	// *** Supply systems

	// -> Schedules
	p.m_objectLists.insert(p.m_objectLists.end(), supplySystemNetworkModelGenerator.m_objLists.begin(), supplySystemNetworkModelGenerator.m_objLists.end());
	p.m_schedules.m_scheduleGroups.insert(supplySystemNetworkModelGenerator.m_scheduleGroups.begin(), supplySystemNetworkModelGenerator.m_scheduleGroups.end());
	p.m_schedules.m_annualSchedules.insert(supplySystemNetworkModelGenerator.m_annualSchedules.begin(), supplySystemNetworkModelGenerator.m_annualSchedules.end());

	// -> Summation and adapter models ***
	if(!supplySystemNetworkModelGenerator.m_summationModels.empty())
		p.m_models.m_heatLoadSummationModels.insert(p.m_models.m_heatLoadSummationModels.begin(),
													supplySystemNetworkModelGenerator.m_summationModels.begin(),
													supplySystemNetworkModelGenerator.m_summationModels.end() );

	if(!supplySystemNetworkModelGenerator.m_adapterModels.empty())
		p.m_models.m_networkInterfaceAdapterModels.insert(p.m_models.m_networkInterfaceAdapterModels.begin(),
														  supplySystemNetworkModelGenerator.m_adapterModels.begin(),
														  supplySystemNetworkModelGenerator.m_adapterModels.end() );

	// -> Networks
	p.m_hydraulicNetworks.insert(p.m_hydraulicNetworks.end(),
								 supplySystemNetworkModelGenerator.m_hydraulicNetworks.begin(),
								 supplySystemNetworkModelGenerator.m_hydraulicNetworks.end());

	// -> FMI Descriptions
	if(!supplySystemNetworkModelGenerator.m_inputVariables.empty() ) {
		if(p.m_fmiDescription.m_modelName.empty())
			p.m_fmiDescription.m_modelName = modelName.toStdString();
		p.m_fmiDescription.m_inputVariables.insert(p.m_fmiDescription.m_inputVariables.end(), supplySystemNetworkModelGenerator.m_inputVariables.begin(), supplySystemNetworkModelGenerator.m_inputVariables.end());
	}
	if(!supplySystemNetworkModelGenerator.m_outputVariables.empty() ) {
		if(p.m_fmiDescription.m_modelName.empty())
			p.m_fmiDescription.m_modelName = modelName.toStdString();
		p.m_fmiDescription.m_outputVariables.insert(p.m_fmiDescription.m_outputVariables.end(), supplySystemNetworkModelGenerator.m_outputVariables.begin(), supplySystemNetworkModelGenerator.m_outputVariables.end());
	}


	bool haveLongWaveEmission = false;
	for (NANDRAD::ConstructionInstance &ci : p.m_constructionInstances) {
		double valueA = ci.m_interfaceA.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;
		double valueB = ci.m_interfaceB.m_longWaveEmission.m_para[NANDRAD::InterfaceLongWaveEmission::P_Emissivity].value;

		if ((ci.m_interfaceA.m_zoneId != 0 && valueA > 0.0) ||
				(ci.m_interfaceB.m_zoneId != 0 && valueB > 0.0)) {
			haveLongWaveEmission = true;
			break;
		}
	}
	if (haveLongWaveEmission)
		addViewFactorsToNandradZones(p, roomMappings, componentInstanceMapping, surfaceIdsVicusToNandrad, errorStack);

}



void Project::generateNandradZones(std::vector<const VICUS::Room *> & zones,
								   std::set<unsigned int> & idSet,
								   NANDRAD::Project & p, QStringList & errorStack,
								   std::vector<RoomMapping> &mappings)const
{
	// collect a list of zone references for further processing
	// also create a new zone name 'buildingName'.'buildingLevelName'.'roomName'
	std::map<unsigned int, std::string> roomIdsToRoomNames;
	for (const VICUS::Building & b : m_buildings){
		QString buildingName = b.m_displayName;
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels){
			QString buildingLevelName = bl.m_displayName;
			for (const VICUS::Room & r : bl.m_rooms){
				zones.push_back(&r);
				QString name = QString("%2.%3.%4(ID=%1)").arg(r.m_id).arg(buildingName, buildingLevelName, r.m_displayName);

				RoomMapping mapEle;
				mapEle.m_idBuildingVicus = b.m_id;
				mapEle.m_idBuildingLevelVicus = bl.m_id;
				mapEle.m_idRoomVicus = r.m_id;
				mapEle.m_idRoomNandrad = r.m_id;		// attention NANDRAD id is set in next loop but is the same
				mapEle.m_idZoneTemplateVicus = INVALID_ID;
				mapEle.m_nameRoomNandrad = name.toStdString();
				mapEle.m_nameRoomVicus = r.m_displayName.toStdString();
				mapEle.m_nameBuildingVicus = b.m_displayName.toStdString();
				mapEle.m_nameBuildingLevelVicus = bl.m_displayName.toStdString();

				if(!r.m_para[VICUS::Room::P_Area].empty())
					mapEle.m_floorArea = r.m_para[VICUS::Room::P_Area].value; // in m2
				if(!r.m_para[VICUS::Room::P_Volume].empty())
					mapEle.m_volume = r.m_para[VICUS::Room::P_Volume].value; // in m3

				roomIdsToRoomNames[r.m_id] = name.toStdString();

				// get zone templates names for mapping
				if(r.m_idZoneTemplate != INVALID_ID){
					for(unsigned int i=0; i<m_embeddedDB.m_zoneTemplates.size(); ++i){
						const ZoneTemplate &zt = m_embeddedDB.m_zoneTemplates[i];
						unsigned int idZoneTemp = zt.m_id;
						if(idZoneTemp != r.m_idZoneTemplate)
							continue;

						mapEle.m_idZoneTemplateVicus = idZoneTemp;
						mapEle.m_zonetemplateName = zt.m_displayName.string();
					}
				}
				mappings.push_back(mapEle);
			}
		}
	}

	// TODO Anton: Könnte das nicht passieren?
	// check for unqiue zone IDs
	try {
		checkForUniqueIDs(zones, idSet);
	} catch (...) {
		errorStack.append(tr("Duplicate room IDs found."));
		return;
	}

	// process all zones and generate NANDRAD::Zone objects
	for (const VICUS::Room * r : zones) {
		// first create a NANDRAD zone for the room
		NANDRAD::Zone z;
		z.m_id = r->m_id;
		z.m_displayName = roomIdsToRoomNames[r->m_id];

		// Note: in the code below we expect the parameter's base units to be the same as the default unit for the
		//       populated parameters

		bool isZoneOk = true;
		try {
			r->m_para[VICUS::Room::P_Area].checkedValue("Area", "m2", "m2", 0.1, true, std::numeric_limits<double>::max(), true, nullptr);
		} catch (...) {
			errorStack.append(tr("Zone #%1 '%2' does not have a valid area defined.").arg(r->m_id).arg(r->m_displayName));
			isZoneOk = false;
		}
		try {
			r->m_para[VICUS::Room::P_Volume].checkedValue("Volume", "m3", "m3", 0.1, true, std::numeric_limits<double>::max(), true, nullptr);
		} catch (...) {
			errorStack.append(tr("Zone #%1 '%2' does not have a valid volume defined.").arg(r->m_id).arg(r->m_displayName));
			isZoneOk = false;
		}



		if (!isZoneOk)
			continue;

		NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, r->m_para[VICUS::Room::P_Area].value);
		NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, r->m_para[VICUS::Room::P_Volume].value);

		// for now, zones are always active
		z.m_type = NANDRAD::Zone::ZT_Active;

		// finally append zone
		p.m_zones.push_back(z);
	}
}

/* Important in the NANDRAD project the scheduleQuantity is not added. Otherwise the project is inconsistent. */
void Project::addVicusScheduleToNandradProject(const VICUS::Schedule &schedVic, const std::string &scheduleQuantityName,
											   NANDRAD::Schedules &schedules, const std::string &objListName) const {

	// *** predefinitions ***

	//find a schedule in NANDRAD project with the objListName
	if(schedules.m_scheduleGroups.find(objListName) != schedules.m_scheduleGroups.end()){
		//get schedule vector for this object list name
		std::vector<NANDRAD::Schedule> &scheds = schedules.m_scheduleGroups[objListName];

		for(unsigned int i=0; i<schedVic.m_periods.size(); ++i){
			const VICUS::ScheduleInterval &period = schedVic.m_periods[i];

			for(unsigned int j=0; j<period.m_dailyCycles.size(); ++j){
				const VICUS::DailyCycle &dc = period.m_dailyCycles[j];

				//merge all possible day types
				std::vector<NANDRAD::Schedule::ScheduledDayType> dts = Schedule::mergeDayType(dc.m_dayTypes);

				//loop over all day types of vicus schedule
				for(NANDRAD::Schedule::ScheduledDayType dt : dts){

					bool valuesAdded = false;
					//check if a period with equal start+end date exists
					for(NANDRAD::Schedule &schedNandrad : scheds){

						//now check day types of vicus schedule with nandrad schedule
						//for a nandrad schedule following properties must be equal
						// * day type
						// * start day
						// * end day
						// if this is all equal we can add the timepoint and values to an existing daily cycle with same interpolation method
						// otherwise we add a new daily cycle to the daily cylce vector

						if(dt == schedNandrad.m_type &&
								schedNandrad.m_startDayOfTheYear == period.m_intervalStartDay &&
								((i+1<schedVic.m_periods.size() && schedNandrad.m_endDayOfTheYear == schedVic.m_periods[i+1].m_intervalStartDay-1) ||
								 schedNandrad.m_endDayOfTheYear == 364)){
							//now check if we have daily cylces with equal properties to:
							// * interpolation method
							// * time points
							for(NANDRAD::DailyCycle &dcNandrad : schedNandrad.m_dailyCycles){
								if( ( (dcNandrad.m_interpolation == NANDRAD::DailyCycle::IT_Constant && !schedVic.m_useLinearInterpolation) ||
									  (dcNandrad.m_interpolation == NANDRAD::DailyCycle::IT_Linear && schedVic.m_useLinearInterpolation) ) &&
										dcNandrad.m_timePoints == dc.m_timePoints){
									// now we can add the data
									dcNandrad.m_values.m_values[scheduleQuantityName] = dc.m_values;
									valuesAdded = true;
									break;
								}
							}
							//nothing found to add data
							//create a new daily cycle
							if(!valuesAdded){
								NANDRAD::DailyCycle newDcNandrad;
								newDcNandrad.m_interpolation = schedVic.m_useLinearInterpolation ? NANDRAD::DailyCycle::IT_Linear : NANDRAD::DailyCycle::IT_Linear;
								newDcNandrad.m_timePoints = dc.m_timePoints;
								newDcNandrad.m_values.m_values[scheduleQuantityName] = dc.m_values;
								schedNandrad.m_dailyCycles.push_back(newDcNandrad);
								valuesAdded = true;
							}
						}
						if(valuesAdded)
							break;
					}
					//no schedule found
					//so add a new one
					if(!valuesAdded){
						NANDRAD::Schedule newNandradSched;
						newNandradSched.m_startDayOfTheYear = period.m_intervalStartDay;
						if(i+1<schedVic.m_periods.size())
							newNandradSched.m_endDayOfTheYear = schedVic.m_periods[i+1].m_intervalStartDay - 1;

						//create daily cyle
						NANDRAD::DailyCycle newDcNandrad;
						newDcNandrad.m_interpolation = schedVic.m_useLinearInterpolation ? NANDRAD::DailyCycle::IT_Linear : NANDRAD::DailyCycle::IT_Linear;
						newDcNandrad.m_timePoints = dc.m_timePoints;
						newDcNandrad.m_values.m_values[scheduleQuantityName] = dc.m_values;
						//add daily cycle to schedule
						newNandradSched.m_dailyCycles.push_back(newDcNandrad);

						for(NANDRAD::Schedule::ScheduledDayType dtNandrad : dts){
							newNandradSched.m_type = dtNandrad;
							//add schedule to schedule group
							scheds.push_back(newNandradSched);
						}
					}
				}
			}
		}
	}
	//we do not find a schedule with name = objListName
	//so create a new one
	else{
		std::vector<NANDRAD::Schedule> scheds;
		//create for each period a new NANDRAD schedule
		for(unsigned int i=0; i<schedVic.m_periods.size(); ++i){
			const VICUS::ScheduleInterval &period = schedVic.m_periods[i];
			//find day types for NANDRAD schedule
			for(unsigned int j=0; j<period.m_dailyCycles.size(); ++j){
				const VICUS::DailyCycle &dc = period.m_dailyCycles[j];

				//merge all possible day types
				std::vector<NANDRAD::Schedule::ScheduledDayType> dts = Schedule::mergeDayType(dc.m_dayTypes);

				//create for each day type in merge vector a new NANDRAD schedule
				for(NANDRAD::Schedule::ScheduledDayType dt : dts){
					NANDRAD::Schedule s;
					//set up start day
					if(period.m_intervalStartDay > 0)
						s.m_startDayOfTheYear = period.m_intervalStartDay;
					//set up end day
					//search in next period for a start day
					if(i+1 < schedVic.m_periods.size())
						s.m_endDayOfTheYear = schedVic.m_periods[i+1].m_intervalStartDay - 1;
					s.m_type = dt;
					NANDRAD::DailyCycle dcNANDRAD;
					dcNANDRAD.m_interpolation = schedVic.m_useLinearInterpolation ? NANDRAD::DailyCycle::IT_Linear : NANDRAD::DailyCycle::IT_Constant;
					dcNANDRAD.m_timePoints = dc.m_timePoints;
					dcNANDRAD.m_values.m_values[scheduleQuantityName] = dc.m_values;
					s.m_dailyCycles.push_back(dcNANDRAD);
					scheds.push_back(s);
				}
			}
		}
		//add sched group to NANDRAD project
		schedules.m_scheduleGroups[objListName] = scheds;
	}
}



void InternalLoadsModelGenerator::generate(const Room * r, std::vector<unsigned int> &usedModelIds, bool enableMoisture, QStringList & errorStack) {
	// check if we have a zone template with id to internal loads

	// InternalLoad holds data for sub templates ST_IntLoadPerson, ST_IntLoadEquipment or ST_IntLoadLighting
	// we check if at least one of those is defined
	const InternalLoad * intLoadPerson = nullptr;
	const InternalLoad * intLoadEquipment = nullptr;
	const InternalLoad * intLoadLighting = nullptr;
	try {
		intLoadPerson = dynamic_cast<const InternalLoad*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_IntLoadPerson));
		intLoadEquipment = dynamic_cast<const InternalLoad*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_IntLoadEquipment));
		intLoadLighting = dynamic_cast<const InternalLoad*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_IntLoadLighting));
	}
	catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	// no internal loads defined?
	if (intLoadPerson == nullptr && intLoadEquipment == nullptr && intLoadLighting == nullptr)
		return;

	// generate schedules

	std::string personSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_PersonHeatLoadPerArea) + "Schedule [W/m2]";
	std::string personMoistSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalMoistureLoadsModel::para_t", NANDRAD::InternalMoistureLoadsModel::P_MoistureLoadPerArea) + "Schedule [kg/m2s]";
	std::string equipmentSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_EquipmentHeatLoadPerArea) + "Schedule [W/m2]";
	std::string lightingSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_LightingHeatLoadPerArea) + "Schedule [W/m2]";

	// schedule generation:
	//
	// 1. create basic schedule (name?)
	std::vector<NANDRAD::Schedule> loadScheds; // contains all schedules for this internal loads parametrization
	std::vector<NANDRAD::Schedule> moistScheds; // contains all schedules for this internal moisture loads parametrization

	// 2. initialize with "AllDays" DailyCycle
	NANDRAD::Schedule allDays;
	allDays.m_type = NANDRAD::Schedule::ST_ALLDAYS;
	NANDRAD::DailyCycle dc;
	dc.m_interpolation = NANDRAD::DailyCycle::IT_Constant;
	dc.m_timePoints.push_back(0);
	if (intLoadPerson == nullptr)		dc.m_values.m_values[personSchedName].push_back(0);
	if (intLoadEquipment == nullptr)	dc.m_values.m_values[equipmentSchedName].push_back(0);
	if (intLoadLighting == nullptr)		dc.m_values.m_values[lightingSchedName].push_back(0);

	allDays.m_dailyCycles.push_back( dc );

	// we only need an all day schedule if we miss at least one of the schedules
	if (!dc.m_values.m_values.empty()) {
		loadScheds.push_back(allDays);
	}

	// initialize load fractions with 0
	double personRadiationFraction = 0;
	double equipmentRadiationFraction = 0;
	double lightingRadiationFraction = 0;

	// for each given VICUS schedule, generate a set of schedules and insert them in vector scheds (unless already existing, in which case these are extended)

	// *** Person loads (category = IC_Person)***
	bool moistRateScheduleExists = false;

	if (intLoadPerson != nullptr) {
		if (!intLoadPerson->isValid(m_scheduleDB)) {
			errorStack.append(qApp->tr("Invalid parameters in internal loads model #%1 '%2'").arg(intLoadPerson->m_id)
							  .arg(MultiLangString2QString(intLoadPerson->m_displayName)) );
		}
		else {
			IBK_ASSERT(intLoadPerson->m_category == InternalLoad::IC_Person);

			// TODO Dirk, other fractions!
			personRadiationFraction = 1-intLoadPerson->m_para[InternalLoad::P_ConvectiveHeatFactor].value;

			// - depending on parametrization option (m_personCountMethod) compute W/m2
			double personPerArea = 0;
			switch (intLoadPerson->m_personCountMethod) {
			case VICUS::InternalLoad::PCM_PersonPerArea:
				personPerArea = intLoadPerson->m_para[InternalLoad::P_PersonPerArea].value;
				break;
			case VICUS::InternalLoad::PCM_AreaPerPerson:
				personPerArea = 1./intLoadPerson->m_para[InternalLoad::P_AreaPerPerson].value;
				break;
			case VICUS::InternalLoad::PCM_PersonCount:
				personPerArea = intLoadPerson->m_para[InternalLoad::P_PersonCount].value/r->m_para[VICUS::Room::P_Area].value;
				break;
			case VICUS::InternalLoad::NUM_PCM: ; // just to make compiler happy
			}

			// - retrieve schedules referenced via m_occupancyScheduleId and m_activityScheduleId
			//   valid schedule references have been checked in isValid() already
			const Schedule * activitySchedule = m_scheduleDB[intLoadPerson->m_idActivitySchedule];
			const Schedule * occupancySchedule = m_scheduleDB[intLoadPerson->m_idOccupancySchedule];
			// - multiply all three values for each sample in each schedule -> VICUS::Schedule with time series of time points and values for different day types
			VICUS::Schedule combinedSchedule = activitySchedule->multiply(*occupancySchedule);
			combinedSchedule = combinedSchedule.multiply(personPerArea);

			// - convert and insert VICUS::Schedule to NANDRAD schedule group
			combinedSchedule.insertIntoNandradSchedulegroup(personSchedName, loadScheds);

			// create schedule for moisture load
			if (enableMoisture) {
				const Schedule * moistRateSchedule = m_scheduleDB[intLoadPerson->m_idMoistureProductionRatePerAreaSchedule];
				// we allow missing moisture rate
				if (moistRateSchedule != nullptr) {
					// TODO : check if this is correct - why multiply a m2-based load with an occupancy? This is likely wrong.
					VICUS::Schedule combinedMoistSchedule = moistRateSchedule->multiply(*occupancySchedule);
					combinedMoistSchedule = combinedMoistSchedule.multiply(personPerArea);

					// - convert and insert VICUS::Schedule to NANDRAD schedule group
					combinedMoistSchedule.insertIntoNandradSchedulegroup(personMoistSchedName, moistScheds);

					// register a moisture rate schedule
					moistRateScheduleExists = true;
				}
			}
		}
	}


	// *** Equipment loads (category = IC_ElectricEquiment )***

	if (intLoadEquipment != nullptr) {
		if (!intLoadEquipment->isValid(m_scheduleDB)) {
			errorStack.append(qApp->tr("Invalid parameters in internal loads model #%1 '%2'").arg(intLoadEquipment->m_id)
							  .arg(MultiLangString2QString(intLoadEquipment->m_displayName)) );
		}
		else {
			IBK_ASSERT(intLoadEquipment->m_category == InternalLoad::IC_ElectricEquiment);

			equipmentRadiationFraction = 1-intLoadEquipment->m_para[InternalLoad::P_ConvectiveHeatFactor].value;

			// - depending on parametrization option (m_personCountMethod) compute W/m2
			double powerPerArea = 0;
			switch (intLoadEquipment->m_powerMethod) {
			case VICUS::InternalLoad::PM_PowerPerArea:
				powerPerArea = intLoadEquipment->m_para[InternalLoad::P_PowerPerArea].value;
				break;
			case VICUS::InternalLoad::PM_Power:
				powerPerArea = intLoadEquipment->m_para[InternalLoad::P_Power].value/r->m_para[VICUS::Room::P_Area].value;
				break;
			case VICUS::InternalLoad::NUM_PM: ; // just to make compiler happy
			}

			// - retrieve schedules referenced via
			//   valid schedule references have been checked in isValid() already
			const Schedule * powerManagementSchedule = m_scheduleDB[intLoadEquipment->m_idPowerManagementSchedule];
			// - multiply all three values for each sample in each schedule -> VICUS::Schedule with time series of time points and values for different day types
			VICUS::Schedule combinedSchedule = powerManagementSchedule->multiply(powerPerArea);


			// - convert and insert VICUS::Schedule to NANDRAD schedule group
			combinedSchedule.insertIntoNandradSchedulegroup(equipmentSchedName, loadScheds);
		}
	}


	// *** Lighting loads (category = IC_Lighting )***

	if (intLoadLighting != nullptr) {
		if (!intLoadLighting->isValid(m_scheduleDB)) {
			errorStack.append(qApp->tr("Invalid parameters in internal loads model #%1 '%2'").arg(intLoadLighting->m_id)
							  .arg(MultiLangString2QString(intLoadLighting->m_displayName)) );
		}
		else {
			IBK_ASSERT(intLoadLighting->m_category == InternalLoad::IC_Lighting);

			lightingRadiationFraction = 1-intLoadLighting->m_para[InternalLoad::P_ConvectiveHeatFactor].value;

			// - depending on parametrization option (m_personCountMethod) compute W/m2
			double powerPerArea = 0;
			switch (intLoadLighting->m_powerMethod) {
			case VICUS::InternalLoad::PM_PowerPerArea:
				powerPerArea = intLoadLighting->m_para[InternalLoad::P_PowerPerArea].value;
				break;
			case VICUS::InternalLoad::PM_Power:
				powerPerArea = intLoadLighting->m_para[InternalLoad::P_Power].value/r->m_para[VICUS::Room::P_Area].value;
				break;
			case VICUS::InternalLoad::NUM_PM: ; // just to make compiler happy
			}

			// - retrieve schedules referenced via
			//   valid schedule references have been checked in isValid() already
			const Schedule * powerManagementSchedule = m_scheduleDB[intLoadLighting->m_idPowerManagementSchedule];
			// - multiply all three values for each sample in each schedule -> VICUS::Schedule with time series of time points and values for different day types
			VICUS::Schedule combinedSchedule = powerManagementSchedule->multiply(powerPerArea);

			// - convert and insert VICUS::Schedule to NANDRAD schedule group
			combinedSchedule.insertIntoNandradSchedulegroup(lightingSchedName, loadScheds);
		}
	}


	// only continue if there were no errors so far
	if (!errorStack.isEmpty())
		return;

	// now we have a valid schedule group, yet without object list name

	// generate NANDRAD::InternalLoads object
	NANDRAD::InternalLoadsModel internalLoadsModel;
	internalLoadsModel.m_id = VICUS::uniqueIdWithPredef(usedModelIds, 1);  /*VICUS::uniqueId(m_internalLoadObjects)*/;
	const VICUS::ZoneTemplate * zt = VICUS::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);
	internalLoadsModel.m_displayName = zt->m_displayName.string();
	internalLoadsModel.m_modelType = NANDRAD::InternalLoadsModel::MT_Scheduled;

	NANDRAD::KeywordList::setParameter(internalLoadsModel.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_PersonRadiationFraction, personRadiationFraction);
	NANDRAD::KeywordList::setParameter(internalLoadsModel.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_EquipmentRadiationFraction, equipmentRadiationFraction);
	NANDRAD::KeywordList::setParameter(internalLoadsModel.m_para, "InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_LightingRadiationFraction, lightingRadiationFraction);

	// Now we check if we have already an internal loads model object with exactly the same parameters (except ID and name).
	// Then, we also compare the matching schedule (the internal loads model object and corresponding schedule have same ID).
	// If this schedule is also identitical to our generated schedule, we simply extend the object list by our zone ID
	// otherwise we add load and schedule definitions and generate a new object list.

	bool foundModel = false;
	for (unsigned int i=0; i<m_internalLoadObjects.size(); ++i) {
		if (m_internalLoadObjects[i].equal(internalLoadsModel) &&
				NANDRAD::Schedules::equalSchedules(m_intLoadSchedules[i], loadScheds) ) {
			// insert our zone ID in object list
			m_intLoadObjLists[i].m_filterID.m_ids.insert(r->m_id);
			foundModel = true;
			break;
		}
	}
	if(!foundModel) {
		// append definitions and create new object list
		NANDRAD::ObjectList ol;
		ol.m_name = IBK::pick_name("InternalLoads-" + zt->m_displayName.string(), m_intLoadObjListNames.begin(), m_intLoadObjListNames.end());
		ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ol.m_filterID.m_ids.insert(r->m_id);

		// set object list in new definition
		internalLoadsModel.m_zoneObjectList = ol.m_name;

		// add all definitions
		m_internalLoadObjects.push_back(internalLoadsModel);
		m_intLoadSchedules.push_back(loadScheds);
		m_intLoadObjLists.push_back(ol);
		m_intLoadObjListNames.push_back(ol.m_name);
	}


	if (enableMoisture && moistRateScheduleExists) {
		// generate NANDRAD::InternalMoistureLoads object
		NANDRAD::InternalMoistureLoadsModel internalMoistLoadsModel;
		internalMoistLoadsModel.m_id = VICUS::uniqueIdWithPredef(usedModelIds, 1);  /*VICUS::uniqueId(m_internalLoadObjects)*/;
		const VICUS::ZoneTemplate * zt = VICUS::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);
		internalMoistLoadsModel.m_displayName = zt->m_displayName.string();
		internalMoistLoadsModel.m_modelType = NANDRAD::InternalMoistureLoadsModel::MT_Scheduled;

		// Now we check if we have already an internal moisture loads model object with exactly the same schedule
		// (the internal moisture loads model object and corresponding schedule have same ID).
		// If this schedule is also identitical to our generated schedule, we simply extend the object list by our zone ID
		// otherwise we add load and schedule definitions and generate a new object list.

		bool foundModel = false;
		for (unsigned int i=0; i<m_internalMoistLoadObjects.size(); ++i) {
			if (m_internalMoistLoadObjects[i].equal(internalMoistLoadsModel) &&
					NANDRAD::Schedules::equalSchedules(m_intMoistLoadSchedules[i], moistScheds) ) {
				// insert our zone ID in object list
				m_intMoistLoadObjLists[i].m_filterID.m_ids.insert(r->m_id);
				foundModel = true;
				break;
			}
		}
		if (!foundModel) {
			// append definitions and create new object list
			NANDRAD::ObjectList ol;
			ol.m_name = IBK::pick_name("InternalMoistureLoads-" + zt->m_displayName.string(), m_intMoistLoadObjListNames.begin(), m_intMoistLoadObjListNames.end());
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
			ol.m_filterID.m_ids.insert(r->m_id);

			// set object list in new definition
			internalMoistLoadsModel.m_zoneObjectList = ol.m_name;

			// add all definitions
			m_internalMoistLoadObjects.push_back(internalMoistLoadsModel);
			m_intMoistLoadSchedules.push_back(moistScheds);
			m_intMoistLoadObjLists.push_back(ol);
			m_intMoistLoadObjListNames.push_back(ol.m_name);
		}
	}
}

void VentilationModelGenerator::generate(const Room *r,std::vector<unsigned int> &usedModelIds,  QStringList &errorStack) {
	//	FUNCID(VentilationModelGenerator::generate);

	// check if we have a zone template with id to infiltration or ventilation

	// InternalLoad holds data for sub templates ST_IntLoadPerson, ST_IntLoadEquipment or ST_IntLoadLighting
	// we check if at least one of those is defined
	const Infiltration						* infiltration = nullptr;
	const VentilationNatural				* ventilation  = nullptr;
	const ZoneControlNaturalVentilation		* ctrlVentilation = nullptr;

	unsigned int idSubTempInf = INVALID_ID;
	unsigned int idSubTempVent = INVALID_ID;
	unsigned int idSubTempCtrl = INVALID_ID;

	try {
		infiltration = dynamic_cast<const Infiltration*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_Infiltration));
		ventilation = dynamic_cast<const VentilationNatural*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_VentilationNatural));
		ctrlVentilation = dynamic_cast<const ZoneControlNaturalVentilation*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_ControlVentilationNatural));
	}  catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	// no ventilation defined?
	if (ventilation == nullptr && infiltration == nullptr)
		return;

	const VICUS::ZoneTemplate * zoneTemplate = VICUS::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);

	if(ventilation != nullptr  && !ventilation->isValid(m_scheduleDB))
		errorStack.append( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
						   .arg(ventilation->m_id)
						   .arg(zoneTemplate->m_id)
						   .arg(MultiLangString2QString(zoneTemplate->m_displayName)));

	if(ventilation != nullptr)
		idSubTempVent = ventilation->m_id;

	if(infiltration != nullptr  && !infiltration->isValid())
		errorStack.append( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
						   .arg(infiltration->m_id)
						   .arg(zoneTemplate->m_id)
						   .arg(MultiLangString2QString(zoneTemplate->m_displayName)));

	if(infiltration != nullptr)
		idSubTempInf = infiltration->m_id;

	if(ctrlVentilation != nullptr  && !ctrlVentilation->isValid(m_scheduleDB))
		errorStack.append( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
						   .arg(ctrlVentilation->m_id)
						   .arg(zoneTemplate->m_id)
						   .arg(MultiLangString2QString(zoneTemplate->m_displayName)));

	if(ctrlVentilation != nullptr)
		idSubTempCtrl = ctrlVentilation->m_id;

	if(ventilation == nullptr && ctrlVentilation != nullptr)
		// ToDo Stephan wie komm ich jetzt hier an das Haupttemplate da hier die Kombi nicht stimmt.
		errorStack.append( qApp->tr("A template %1 with a ventilation control must contain a ventilation model!.")
						   .arg("Hier muss die ID des Haupttemplates rein..."));

	// only continue if there were no errors so far
	if (!errorStack.isEmpty())

		return;
	//which system we have?
	//1. only infiltration
	//2. only ventilation
	//3. infiltration and ventilation
	enum VentiType{
		V_Infiltration,
		V_Ventilation,
		V_InfAndVenti,
	};
	bool isInf = idSubTempInf != VICUS::INVALID_ID;
	bool isVenti = idSubTempVent != VICUS::INVALID_ID;
	Q_ASSERT(isInf || isVenti); // one of both must be set, otherwise a check above was missing
	VentiType ventiType = V_InfAndVenti; // default to both
	if (!isVenti)		ventiType = V_Infiltration;
	if(!isInf)			ventiType = V_Ventilation;

	// prüfe den zeitplan auf das maximum
	// bei nur INFILTRATION werden nur die parameter gerprüft
	// bei nur VENTILATION / VENTI*INFI muss das Maximum aus dem Zeitplan (+ConstInfValu) ermittelt werden + Prüfung

	// anschließend wird ein zeitplan nach folgenden schema erzeugt
	// Maximum air change rate comfort - zeitplan aus INFI und/oder VENTI

	NANDRAD::NaturalVentilationModel natVentMod;
	natVentMod.m_id = VICUS::uniqueIdWithPredef(usedModelIds, 1);
	natVentMod.m_displayName = zoneTemplate->m_displayName.string();
	//TODO id and display name

	// model type and constant parameters
	if(ctrlVentilation != nullptr) {
		natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACRDynamicTLimit;
		// copy wind speed parameter
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MaxWindSpeed,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::P_WindSpeedMax].get_value("m/s"));
	}
	else {
		//now add other parameter
		switch(ventiType){
		case VentiType::V_Infiltration:{
			natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Constant;
			switch(infiltration->m_airChangeType){
			case VICUS::Infiltration::AC_normal:{
				NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
												   NANDRAD::NaturalVentilationModel::P_VentilationRate,
												   infiltration->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h"));
			}break;
			case VICUS::Infiltration::AC_n50:{
				double val = infiltration->m_para[VICUS::Infiltration::P_AirChangeRate].get_value("1/h");
				val *= infiltration->m_para[VICUS::Infiltration::P_ShieldingCoefficient].get_value("-");
				NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
												   NANDRAD::NaturalVentilationModel::P_VentilationRate,
												   val);
			}break;
			case VICUS::Infiltration::NUM_AC:	//only for compiler
				break;
			}
		}
			break;
		case VentiType::V_Ventilation:
		case VentiType::V_InfAndVenti: {
			natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Scheduled;
		}
			break;
		}
	}

	// *** schedules ***
	//
	// 1. create basic schedule for ventilation rate (name?)
	std::vector<Schedule> vicusScheds(NUM_ST);
	VICUS::Schedule ventilationSchedule;

	// for control model with constant infiltration we create a constant schedule
	if(ventiType == V_Infiltration) {
		if(ctrlVentilation != nullptr) {
			// check values
			double val = infiltration->m_para[Infiltration::P_AirChangeRate].get_value("1/h");
			double maxVal = ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::P_MaximumAirChangeRateComfort].get_value("1/h");
			if(val > maxVal)  {

				QString errmsg = QString("Error in infiltration model #%1. "
										 "Maximum air change rate (%3 1/h) must be lower than maximum air change rate in comfort model (%2 1/h!)"
										 ).arg(ctrlVentilation->m_id).arg(maxVal).arg(val);
				errorStack.push_back(errmsg);
			}
			else {
				// create a constant schedule
				ventilationSchedule.createConstSchedule(val);
			}
		}
	}
	else {
		const Schedule * ventSched = m_scheduleDB[ventilation->m_idSchedule];
		ventilationSchedule = ventSched->multiply(ventilation->m_para[VentilationNatural::P_AirChangeRate].get_value("1/h"));
		if(ventiType == V_InfAndVenti){
			double infVal = infiltration->m_para[Infiltration::P_AirChangeRate].get_value("1/h");
			if(infiltration->m_airChangeType == Infiltration::AC_n50)
				infVal *= infiltration->m_para[Infiltration::P_ShieldingCoefficient].value;
			ventilationSchedule = ventilationSchedule.add(infVal);
		}

		// check schedule values
		if(ctrlVentilation != nullptr) {
			// check schedule values
			std::vector<double> timepoints, values;

			ventilationSchedule.createYearDataVector(timepoints, values);

			double maxVal = ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::P_MaximumAirChangeRateComfort].get_value("1/h");
			for(double val: values) {
				if(val > maxVal)  {
					QString errmsg = QString("Error in ventilation model #%1. "
											 "Maximum air change rate (%3 1/h) must be lower than maximum air change rate in comfort model (%2 1/h!)"
											 ).arg(ctrlVentilation->m_id).arg(maxVal).arg(val);
					errorStack.push_back(errmsg);
					break;
				}
			}
		}
	}
	// register generating schedule
	vicusScheds[ST_VentilationRate] = ventilationSchedule;

	// only continue if there were no errors so far
	if (!errorStack.isEmpty())
		return;

	// 2. create basic schedules for controlled ventilation
	// we only support modelType 'ScheduledWithBaseACRDynamicTLimit' and therefore
	// convert minimum and maximum air temperatures into schedules, as well as ventilation increase
	// register all other schedules
	Schedule &ventilationIncreaseSchedule = vicusScheds[ST_VentilationRateIncrease];
	Schedule &minAirTempSchedule = vicusScheds[ST_VentilationMinAirTemperature];
	Schedule &maxAirTempSchedule = vicusScheds[ST_VentilationMaxAirTemperature];

	if(ctrlVentilation != nullptr) {
		double maxVentilationRate = ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::P_MaximumAirChangeRateComfort].get_value("1/h");
		// subtract from ventilation schedule
		IBK_ASSERT(!ventilationSchedule.m_periods.empty());
		// create a schedule for ventilation increaes
		ventilationIncreaseSchedule = ventilationSchedule.multiply(-1.0);
		ventilationIncreaseSchedule = ventilationIncreaseSchedule.add(maxVentilationRate);

		// 3. create basic schedule for minimum and maximum air temperature
		if(!ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::P_TemperatureAirMin].empty()) {

			// we both require minimum and maximum air temperature
			IBK_ASSERT(!ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::P_TemperatureAirMax].empty());

			double minAirTemp = ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::P_TemperatureAirMin].get_value("C");
			double maxAirTemp = ctrlVentilation->m_para[VICUS::ZoneControlNaturalVentilation::P_TemperatureAirMax].get_value("C");

			// create constant schedules:
			// minimum air temperature
			minAirTempSchedule.createConstSchedule(minAirTemp);
			// maximum air temperature
			maxAirTempSchedule.createConstSchedule(maxAirTemp);
		}
		else  {
			// ensure existecnce of corresponding schedules
			IBK_ASSERT(ctrlVentilation->m_idSchedules[ZoneControlNaturalVentilation::P_TemperatureAirMin] != VICUS::INVALID_ID);
			IBK_ASSERT(ctrlVentilation->m_idSchedules[ZoneControlNaturalVentilation::P_TemperatureAirMax] != VICUS::INVALID_ID);
			// read schedules:

			// for minimum air temperature
			const Schedule * minAirTempSchedPtr = m_scheduleDB[ctrlVentilation->m_idSchedules[ZoneControlNaturalVentilation::P_TemperatureAirMin]];
			IBK_ASSERT(minAirTempSchedPtr != nullptr);
			minAirTempSchedule = *minAirTempSchedPtr;
			// for maximum air temperature
			const Schedule * maxAirTempSchedPtr = m_scheduleDB[ctrlVentilation->m_idSchedules[ZoneControlNaturalVentilation::P_TemperatureAirMax]];
			IBK_ASSERT(maxAirTempSchedPtr != nullptr);
			maxAirTempSchedule = *maxAirTempSchedPtr;
		}
	}

	// only continue if there were no errors so far
	if (!errorStack.isEmpty())
		return;

	// now we have a valid schedule group, yet without object list name

	// Now we check if we have already an natural ventilation model object with exactly the same parameters (except ID and name, object list).
	// Then, we also compare the matching schedule (the natural ventilation model object and corresponding schedule have same ID).
	// If this schedule is also identitical to our generated schedule, we simply extend the object list by our zone ID
	// otherwise we add model and schedule definitions and generate a new object list.

	bool foundModel = false;
	for (unsigned int i=0; i<m_natVentObjects.size(); ++i) {
		if (m_natVentObjects[i].equal(natVentMod) ) {
			// check similarity of schedules
			const std::vector<Schedule> others =m_generatingSchedules[i];
			bool allSchedulesAreEqual = true;
			for(unsigned int j = 0; j < NUM_ST; ++j) {
				// skip empty schedules
				if(vicusScheds[j].m_periods.empty())
					continue;

				IBK_ASSERT(!others[j].m_periods.empty());
				// schedules are not similar
				if(!others[j].isSimilar(vicusScheds[j])){
					allSchedulesAreEqual = false;
					break;
				}
			}
			if(!allSchedulesAreEqual)
				continue;
			// insert our zone ID in object list
			m_objLists[i].m_filterID.m_ids.insert(r->m_id);
			foundModel = true;
			break;
		}
	}
	if(!foundModel) {
		// append definitions and create new object list
		NANDRAD::ObjectList ol;
		ol.m_name = IBK::pick_name("Ventilation-" + zoneTemplate->m_displayName.string(), m_objListNames.begin(), m_objListNames.end());
		ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ol.m_filterID.m_ids.insert(r->m_id);

		// set object list in new definition
		natVentMod.m_zoneObjectList = ol.m_name;

		// add all definitions
		m_natVentObjects.push_back(natVentMod);
		m_generatingSchedules.push_back(vicusScheds);
		m_objLists.push_back(ol);
		m_objListNames.push_back(ol.m_name);

		std::vector<NANDRAD::Schedule> scheds;
		// register schedules:
		if(!ventilationSchedule.m_periods.empty()) {
			// ventilation
			std::string schedName =  (std::string)NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t",
																				NANDRAD::NaturalVentilationModel::P_VentilationRate) + "Schedule [1/h]";
			ventilationSchedule.insertIntoNandradSchedulegroup(schedName, scheds);
		}
		if(!ventilationIncreaseSchedule.m_periods.empty()) {
			// ventilation increase
			std::string schedName = "VentilationRateIncreaseSchedule [1/h]";
			ventilationIncreaseSchedule.insertIntoNandradSchedulegroup(schedName, scheds);
		}
		if(!minAirTempSchedule.m_periods.empty()) {
			// minimum air temperature
			std::string minTempSchedName =  (std::string)NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t",
																					   NANDRAD::NaturalVentilationModel::P_VentilationMinAirTemperature) + "Schedule [C]";
			minAirTempSchedule.insertIntoNandradSchedulegroup(minTempSchedName, scheds);
		}
		if(!maxAirTempSchedule.m_periods.empty()) {
			// maximum air temperature
			std::string maxTempSchedName =  (std::string)NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t",
																					   NANDRAD::NaturalVentilationModel::P_VentilationMaxAirTemperature) + "Schedule [C]";
			maxAirTempSchedule.insertIntoNandradSchedulegroup(maxTempSchedName, scheds);
		}

		m_schedules.push_back(scheds);
	}
}


void ControlledShadingModelGenerator::generate(const Room *r,std::vector<unsigned int> &usedModelIds,  QStringList &errorStack) {
	const ZoneControlShading* zoneControlShading = nullptr;

	try {
		zoneControlShading = dynamic_cast<const ZoneControlShading*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_ControlShading));
	}  catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	if(zoneControlShading == nullptr)
		return;		// No shading control is defined in zone

	// iterate other all surfaces of the room
	for(const Surface & surface : r->m_surfaces){
		if(surface.subSurfaces().size() == 0){
			// skip if there are no windows
			continue;
		}
		IBKMK::Vector3D normal = surface.geometry().normal();
		SensorOrientation orientation = calculateSensorOrientation(normal, zoneControlShading->m_category);
		if(m_shadingModels.find(zoneControlShading->m_id) != m_shadingModels.end()){
			if(m_shadingModels[zoneControlShading->m_id].m_shadingControlModels.find(orientation) != m_shadingModels[zoneControlShading->m_id].m_shadingControlModels.end())
				// shading model already exists for this surface
				continue;
		} else {
			// create map, since it is the first entry
			m_shadingModels[zoneControlShading->m_id] = ZoneShadingModel();
		}
		//create a new shadingModel and save it in the map
		NANDRAD::ShadingControlModel shadingModel = NANDRAD::ShadingControlModel();
		shadingModel.m_id = VICUS::uniqueId(usedModelIds);
		usedModelIds.push_back(shadingModel.m_id);
		// calc the max and min intensity based on the sensor category
		double maxIntensity = 0;
		// TODO fullfill contraint in UI that maxI needs to be greater than minI -> deadband cannot be 0
		double minIntensity = 0;
		switch(orientation){
		case ControlledShadingModelGenerator::S_Horizontal: {
			maxIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalHorizontal].get_value();
			minIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalHorizontal].get_value() - zoneControlShading->m_para[ZoneControlShading::P_DeadBand].get_value();
			break;
		}
		case ControlledShadingModelGenerator::S_North: {
			maxIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalNorth].get_value();
			minIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalNorth].get_value() - zoneControlShading->m_para[ZoneControlShading::P_DeadBand].get_value();
			break;
		}
		case ControlledShadingModelGenerator::S_East: {
			maxIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalEast].get_value();
			minIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalEast].get_value() - zoneControlShading->m_para[ZoneControlShading::P_DeadBand].get_value();
			break;
		}
		case ControlledShadingModelGenerator::S_South: {
			maxIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalSouth].get_value();
			minIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalSouth].get_value() - zoneControlShading->m_para[ZoneControlShading::P_DeadBand].get_value();
			break;
		}
		case ControlledShadingModelGenerator::S_West: {
			maxIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalWest].get_value();
			minIntensity = zoneControlShading->m_para[ZoneControlShading::P_GlobalWest].get_value() - zoneControlShading->m_para[ZoneControlShading::P_DeadBand].get_value();
			break;
		}
		case ControlledShadingModelGenerator::NUM_S:
			break;

		}
		// set the calculated intensities
		NANDRAD::KeywordList::setParameter(shadingModel.m_para, "ShadingControlModel::para_t",
										   NANDRAD::ShadingControlModel::P_MaxIntensity, maxIntensity);
		NANDRAD::KeywordList::setParameter(shadingModel.m_para, "ShadingControlModel::para_t",
										   NANDRAD::ShadingControlModel::P_MinIntensity, minIntensity);

		// set the sensor id, based on the orientation
		shadingModel.m_sensorId = 2000000 + orientation;

		m_shadingModels.at(zoneControlShading->m_id).m_shadingControlModels[orientation] = shadingModel;

	}

}


ControlledShadingModelGenerator::SensorOrientation ControlledShadingModelGenerator::calculateSensorOrientation(const IBKMK::Vector3D & normal, ZoneControlShading::Category category){
	double altitude = std::asin(normal.m_z) / IBK::DEG2RAD;
	if (altitude < 0){
		altitude += 360;
	}
	double azimut = std::atan2(normal.m_x, ( normal.m_y == 0. ? 1E-8 :normal.m_y ) ) / IBK::DEG2RAD;
	if (azimut < 0)
		azimut += 360;

	if((altitude > 45 && altitude < 135) || category == ZoneControlShading::C_GlobalHorizontalSensor)
		return SensorOrientation::S_Horizontal;
	if(azimut > 45 && azimut <= 135)
		return SensorOrientation::S_East;
	if(azimut > 135 && azimut <= 225)
		return SensorOrientation::S_South;
	if(azimut > 225 && azimut <= 315)
		return SensorOrientation::S_West;
	//else return south
	return SensorOrientation::S_North;

}

void ConstructionInstanceModelGenerator::exportSubSurfaces(QStringList & errorStack, const std::vector<VICUS::SubSurface> &subSurfs,
														   const VICUS::ComponentInstance & ci, NANDRAD::ConstructionInstance &cinst, std::set<unsigned int> &idSet,
														   std::map<unsigned int, unsigned int> &surfaceIdsVicusToNandrad) const{

	double embArea = 0;
	// get area of surface

	Q_ASSERT(ci.m_sideASurface != nullptr || ci.m_sideBSurface != nullptr);

	VICUS::Surface *surf = nullptr;

	if(ci.m_sideASurface == nullptr)
		surf = ci.m_sideBSurface;
	else
		surf = ci.m_sideASurface;

	double area = surf->geometry().area();

	for(const SubSurface &ss : subSurfs){

		if(ss.m_polygon2D.area() < VICUS::MIN_AREA_FOR_EXPORTED_SURFACES)
			continue; // Skip too small surfaces

		NANDRAD::EmbeddedObject emb;
		emb.m_id = VICUS::uniqueId(idSet);
		idSet.insert(emb.m_id);	//add new id to set
		//get surface are of emb. obj.
		NANDRAD::KeywordList::setParameter(emb.m_para, "EmbeddedObject::para_t",
										   NANDRAD::EmbeddedObject::P_Area, ss.m_polygon2D.area());
		double windowOpening = ss.m_polygon2D.area();
		double circumference = ss.m_polygon2D.circumference();
		embArea += windowOpening;
		if(embArea > area){
			errorStack << qApp->tr("Area of sub surfaces is bigger than area of parent surface #%1, '%2'.").arg(surf->m_id)
						  .arg(surf->m_displayName);
			continue;
		}
		emb.m_displayName = IBK::FormatString("%1 (ID=%2)").arg(ss.m_displayName.toStdString()).arg(ss.m_id).str();

		unsigned int subSurfaceComponentId = VICUS::INVALID_ID;
		//find sub surface component instance
		for(const VICUS::SubSurfaceComponentInstance &ssci : m_project->m_subSurfaceComponentInstances){
			if(ssci.m_idSideASurface == ss.m_id || ssci.m_idSideBSurface == ss.m_id){
				subSurfaceComponentId = ssci.m_idSubSurfaceComponent;

				break;
			}
		}
		if(subSurfaceComponentId == VICUS::INVALID_ID){
			errorStack << qApp->tr("No component was assigned to the SubSurface #%1 with name '%2'. Sub surface is not exported.")
						  .arg(ss.m_id).arg(ss.m_displayName);
			continue;
		}
		bool foundSubSurfComp = false;
		//search for sub surface component
		for(const VICUS::SubSurfaceComponent &ssc : m_project->m_embeddedDB.m_subSurfaceComponents){
			if(ssc.m_id == subSurfaceComponentId){
				foundSubSurfComp = true;
				//only simple windows are supported now
				if(ssc.m_type == VICUS::SubSurfaceComponent::CT_Window){
					//ssc.m_idConstructionType
					if(ssc.m_idWindow == VICUS::INVALID_ID){
						errorStack << qApp->tr("The sub surface component with #%1 and name '%2' has no valid window id. Sub surface #%3 is not exported.")
									  .arg(ssc.m_id).arg(QString::fromStdString(ssc.m_displayName.string())).arg(ss.m_id);
						break;
					}
					bool foundWindow =false;

					//search for the window
					for(const Window &winV : m_project->m_embeddedDB.m_windows){
						if(winV.m_id == ssc.m_idWindow){
							if(!winV.isValid()){
								errorStack << qApp->tr("Window #%1 '%2' is not valid.").arg(winV.m_id)
											  .arg(QString::fromStdString(winV.m_displayName.string()));
								continue;
							}
							//save id for glazing system later
							emb.m_window.m_glazingSystemId = winV.m_idGlazingSystem;
							foundWindow = true;

							// check for frame
							double frameArea = 0;
							if(winV.m_methodFrame == Window::M_Fraction)
								frameArea = winV.m_para[Window::P_FrameFraction].get_value() * windowOpening;

							else if(winV.m_methodFrame == Window::M_ConstantWidth){
								frameArea = winV.m_para[Window::P_FrameWidth].get_value() * circumference;
								if(frameArea >= windowOpening){
									errorStack << qApp->tr("Window frame area %3 is greater or equal the window area %4"
														   " of window #%1 '%2' in surface %5 '%6'.")
												  .arg(winV.m_id)
												  .arg(QString::fromStdString(winV.m_displayName.string()))
												  .arg(frameArea)
												  .arg(windowOpening)
												  .arg(surf->m_id)
												  .arg(surf->m_displayName);
									continue;
								}
							}
							if(frameArea > 0){
								emb.m_window.m_frame.m_area.set( "Area", frameArea, IBK::Unit("m2"));

								bool foundFrameMaterial = false;
								for(const Material &mat : m_project->m_embeddedDB.m_materials){
									if(mat.m_id != winV.m_frame.m_idMaterial)
										continue;

									foundFrameMaterial = true;
									if(!mat.isValid()){
										errorStack << qApp->tr("Window frame material #%1 '%2' is not valid.").arg(mat.m_id)
													  .arg(QString::fromStdString(mat.m_displayName.string()));
										continue;
									}
									emb.m_window.m_frame.m_materialId = winV.m_frame.m_idMaterial;
									emb.m_window.m_frame.m_thickness.set( "Thickness",
																		  winV.m_frame.m_para[WindowFrame::P_Thickness].get_value(),
											IBK::Unit("m"));
								}

								if(!foundFrameMaterial){
									errorStack << qApp->tr("Window frame material #%1 not found in database.").arg(winV.m_frame.m_idMaterial);
									continue;
								}
							}

							// check for divider
							double dividerArea = 0;
							if(winV.m_methodDivider == Window::M_Fraction)
								dividerArea = winV.m_para[Window::P_DividerFraction].get_value() * windowOpening;

							else if(winV.m_methodDivider== Window::M_ConstantWidth){
								dividerArea = winV.m_para[Window::P_DividerWidth].get_value() * 1;		///< ToDo Dirk hier muss noch die Länge übergeben werden.
								if(dividerArea + frameArea >= windowOpening){
									errorStack << qApp->tr("Window frame and divider area %3 is greater or equal the window area %4"
														   " of window #%1 '%2' in surface %5 '%6'.")
												  .arg(winV.m_id)
												  .arg(QString::fromStdString(winV.m_displayName.string()))
												  .arg(frameArea+dividerArea)
												  .arg(windowOpening)
												  .arg(surf->m_id)
												  .arg(surf->m_displayName);
									continue;
								}
							}
							if(dividerArea > 0){
								emb.m_window.m_divider.m_area.set( "Area", dividerArea, IBK::Unit("m2"));

								bool foundDividerMaterial = false;
								for(const Material &mat : m_project->m_embeddedDB.m_materials){
									if(mat.m_id != winV.m_divider.m_idMaterial)
										continue;

									foundDividerMaterial = true;
									if(!mat.isValid()){
										errorStack << qApp->tr("Window Divider material #%1 '%2' is not valid.").arg(mat.m_id)
													  .arg(QString::fromStdString(mat.m_displayName.string()));
										continue;
									}
									emb.m_window.m_divider.m_materialId = winV.m_divider.m_idMaterial;
									emb.m_window.m_divider.m_thickness.set( "Thickness",
																			winV.m_divider.m_para[WindowDivider::P_Thickness].get_value(),
											IBK::Unit("m"));
								}

								if(!foundDividerMaterial){
									errorStack << qApp->tr("Window divider material #%1 not found in database.").arg(winV.m_divider.m_idMaterial);
									continue;
								}
							}
							// check if zone has shading
							Q_ASSERT(ss.m_parent != nullptr && ss.m_parent->m_parent != nullptr);
							unsigned int idZoneTemplate = dynamic_cast<VICUS::Room *>(ss.m_parent->m_parent)->m_idZoneTemplate;
							if(idZoneTemplate != VICUS::INVALID_ID){
								const VICUS::ZoneTemplate * zt = VICUS::element(m_project->m_embeddedDB.m_zoneTemplates, idZoneTemplate);
								unsigned int idZoneControlShading = zt->m_idReferences[ZoneTemplate::ST_ControlShading];
								if(idZoneControlShading != VICUS::INVALID_ID){
									//sub surface has controlled shading
									Q_ASSERT(VICUS::element(m_project->m_embeddedDB.m_zoneControlShading, idZoneControlShading) != nullptr);
									const VICUS::ZoneControlShading * zcs = VICUS::element(m_project->m_embeddedDB.m_zoneControlShading, idZoneControlShading);
									NANDRAD::WindowShading shading;
									shading.m_modelType = NANDRAD::WindowShading::MT_Controlled;
									//TODO give user possibility to set Reductionfactor in UI, right now its set default to 0.6
									NANDRAD::KeywordList::setParameter(shading.m_para, "WindowShading::para_t",
																	   NANDRAD::WindowShading::P_ReductionFactor, winV.m_para[Window::P_ReductionFactor].get_value());
									//calc the sensor orientation
									const VICUS::Surface *s = dynamic_cast<const VICUS::Surface*>(ss.m_parent);
									ControlledShadingModelGenerator::SensorOrientation ori = ControlledShadingModelGenerator::calculateSensorOrientation(s->geometry().normal(), zcs->m_category);
									//check if shading model already exists for this ZoneControlShading
									Q_ASSERT(m_shadingModels != nullptr);
									if(m_shadingModels->find(zcs->m_id) == m_shadingModels->end()){
										errorStack.push_back(QString("Error during model generation, no zone control shading found with id %1").arg(zcs->m_id));
									}

									if(m_shadingModels->at(zcs->m_id).m_shadingControlModels.find(ori) == m_shadingModels->at(zcs->m_id).m_shadingControlModels.end()){
										errorStack.push_back(QString("Error during model generation, no model for the orientation %1").arg(ori));
									}
									shading.m_controlModelId = m_shadingModels->at(zcs->m_id).m_shadingControlModels[ori].m_id;
									emb.m_window.m_shading = shading;
								}
							}
							break;
						}
					}
					if(!foundWindow)
						errorStack << qApp->tr("The window object with #%1 was not found in database.")
									  .arg(ssc.m_idWindow).arg(QString::fromStdString(ssc.m_displayName.string()));
				}
				else{
					errorStack << qApp->tr("The sub surface component with #%1 and name '%2' is not supported by the export.")
								  .arg(ssc.m_id).arg(QString::fromStdString(ssc.m_displayName.string()));
					continue;
				}
			}
		}
		if(!foundSubSurfComp){
			errorStack << qApp->tr("No component was found for the sub surface with #%1 and name '%2'. No export of this sub surface.")
						  .arg(ss.m_id).arg(ss.m_displayName);
			continue;
		}

		//add ids for shading file later
		// Attention if we have coupled sub surfaces this is not valid
		surfaceIdsVicusToNandrad[ss.m_id] = emb.m_id;
		//add emb. obj. to nandrad project
		cinst.m_embeddedObjects.push_back(emb);
	}
}

const VICUS::Room* checkForSurfaceParent(const VICUS::Object *obj) {
	if(obj->m_parent == nullptr)
		return nullptr;

	VICUS::Room *r = dynamic_cast<VICUS::Room*>(obj->m_parent);
	if(r != nullptr)
		return r;
	else
		return checkForSurfaceParent(obj->m_parent);
}

NANDRAD::Interface ConstructionInstanceModelGenerator::generateInterface(const VICUS::ComponentInstance & ci, unsigned int bcID,
																		 unsigned int interfaceID,
																		 QStringList &errorStack,
																		 GroundZoneExport &gze, bool takeASide) const
{
	// no boundary condition ID? -> no interface
	if (bcID == VICUS::INVALID_ID)
		return NANDRAD::Interface();

	const VICUS::Surface * s;
	if(takeASide)
		s = ci.m_sideASurface;
	else
		s = ci.m_sideBSurface;

	// lookup boundary condition definition in embedded database
	const VICUS::BoundaryCondition * bc = VICUS::element(m_project->m_embeddedDB.m_boundaryConditions, bcID);
	if (bc == nullptr){

		if(s == nullptr)
			s = !takeASide ? ci.m_sideASurface : ci.m_sideBSurface;

		if(s== nullptr)
			errorStack.append(qApp->tr("Component #%1 has invalid boundary condition ID reference.")
							  .arg(ci.m_idComponent));
		else
			errorStack.append(qApp->tr("Component #%1 has invalid boundary condition ID reference #%2.")
							  .arg(ci.m_idComponent).arg(s->m_id));
		return NANDRAD::Interface();
	}//	bc->m_heatConduction.m_modelType==VICUS::InterfaceHeatConduction::MT_Constant

	if (!bc->isValid(m_scheduleDB))
		errorStack.append(qApp->tr("Boundary condition #%1 has invalid/incomplete parameters.").arg(bc->m_id));

	// do we have a surface to a zone?
	if (s != nullptr) {
		// get the zone that this interface is connected to
		const VICUS::Room * room = checkForSurfaceParent(s);

		if (room == nullptr){
			errorStack.append(qApp->tr("Component instance #%1 references surface #%2 '%3', which is not associated to a zone.")
							  .arg(ci.m_id).arg(s->m_id).arg(s->m_displayName));
			return NANDRAD::Interface();
		}

		// generate a new interface to the zone, which always only includes heat conduction
		NANDRAD::Interface iface;
		iface.m_id = interfaceID;
		iface.m_zoneId = room->m_id;	//we can take the room id directly, because we dont have duplicates

		// only transfer heat conduction parameters
		iface.m_heatConduction.m_modelType = (NANDRAD::InterfaceHeatConduction::modelType_t)bc->m_heatConduction.m_modelType;
		iface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient] = bc->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient];
		iface.m_solarAbsorption = bc->m_solarAbsorption;
		iface.m_longWaveEmission = bc->m_longWaveEmission;
		return iface;
	}
	else {
		//das muss dann übergeben werden
		//		std::map<int, unsigned int>		m_tempToZone;
		//		std::map<unsigned int, unsigned int> m_schedTempToZone;
		//		std::set<unsigned int>			m_idSet;
		//		NANDRAD::Zone					m_groundZ;
		//		std::vector<VICUS::Schedule>	m_schedules;
		//		std::set<QString>				m_objListNames;

		// no surface == true -> must be an interface to the outside or ground/scheduled
		NANDRAD::Interface iface;
		switch (bc->m_heatConduction.m_otherZoneType) {
		case VICUS::InterfaceHeatConduction::OZ_Standard:
			iface.m_zoneId = 0; // outside zone
			break;
		case VICUS::InterfaceHeatConduction::OZ_Constant: {
			// Create or find a ground zone that corresponds to the required temperature.
			// only take first digit after comma into account

			// TODO : Dirk, Problem: ConstTemperature is nicht gesetzt... müsste das nicht schon getestet sein?

			int temperature = int(bc->m_heatConduction.m_para[InterfaceHeatConduction::P_ConstTemperature].get_value(IBK::Unit("C"))*10);

			if(gze.m_tempToZoneId.find(temperature) != gze.m_tempToZoneId.end())
				iface.m_zoneId = gze.m_tempToZoneId[temperature];
			else{
				// create a new ground zone
				gze.m_groundZ.m_id = VICUS::uniqueId(gze.m_idSet);
				gze.m_idSet.insert(gze.m_groundZ.m_id);
				gze.m_groundZ.m_displayName = "GroundZone(ID=" + IBK::val2string(gze.m_groundZ.m_id) + ")";
				gze.m_groundZ.m_type = NANDRAD::Zone::ZT_Constant;
				NANDRAD::KeywordList::setParameter(gze.m_groundZ.m_para, "Zone::para_t",
												   NANDRAD::Zone::P_Temperature, temperature/10.0);
				gze.m_tempToZoneId[temperature] = gze.m_groundZ.m_id;
				iface.m_zoneId = gze.m_groundZ.m_id;
			}
		}
			break;
		case VICUS::InterfaceHeatConduction::OZ_Scheduled:{
			// Create or find a ground zone that corresponds to the required temperature.
			// only take first digit after comma into account
			unsigned int schedId = bc->m_heatConduction.m_idSchedule;
			const VICUS::Schedule *sched = m_scheduleDB[schedId];
			std::string err;
			if(sched != nullptr && sched->isValid(err, true, m_placeholders)){
				if(gze.m_tempSchedIdToZoneId.find(schedId) != gze.m_tempSchedIdToZoneId.end())
					iface.m_zoneId = gze.m_tempSchedIdToZoneId[schedId];
				else{
					// create a new ground zone
					gze.m_groundZ.m_id = VICUS::uniqueId(gze.m_idSet);
					gze.m_idSet.insert(gze.m_groundZ.m_id);
					gze.m_groundZ.m_displayName = "GroundZone" + IBK::val2string(gze.m_groundZ.m_id);
					gze.m_groundZ.m_type = NANDRAD::Zone::ZT_Scheduled;

					iface.m_zoneId = gze.m_groundZ.m_id;

					// create an object list
					NANDRAD::ObjectList &ol = gze.m_objList;
					ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
					ol.m_filterID.m_ids.insert(gze.m_groundZ.m_id);
					QString name = VICUS::uniqueName("GroundZone", gze.m_objListNames);
					gze.m_objListNames.insert(name);
					ol.m_name = name.toStdString();

					// create schedule
					if( sched->m_haveAnnualSchedule){
						gze.m_spline = sched->m_annualSchedule;
					}
					else{
						std::vector<NANDRAD::Schedule>	scheds;
						sched->insertIntoNandradSchedulegroup("TemperatureSchedule [C]", scheds);
						gze.m_schedules = scheds;
					}
					gze.m_tempSchedIdToZoneId[schedId] = gze.m_groundZ.m_id;
				}
			}
			else
				errorStack.append(qApp->tr("Invalid Schedule in boundary condition #%1 '%2'.") //TODO: Dirk mind Language String
								  .arg(bc->m_id).arg(QString::fromStdString(bc->m_displayName.string(IBK::MultiLanguageString::m_language, "en"))));
		}
			break;
		case VICUS::InterfaceHeatConduction::NUM_OZ:
			errorStack.append(qApp->tr("Boundary condition #%1 name '%2' has no valid heat transfer model.") //TODO: Dirk mind Language String
							  .arg(bc->m_id).arg(QString::fromStdString(bc->m_displayName.string(IBK::MultiLanguageString::m_language, "en"))));
			break;

		}//== InterfaceHeatConduction::OZ_Constant
		// generate a new interface to the zone, which always only includes heat conduction

		iface.m_id = interfaceID;
		iface.m_heatConduction.m_modelType = (NANDRAD::InterfaceHeatConduction::modelType_t)bc->m_heatConduction.m_modelType;
		iface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient] = bc->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient];
		iface.m_solarAbsorption = bc->m_solarAbsorption;
		iface.m_longWaveEmission = bc->m_longWaveEmission;
		return iface;
	}
}

void ConstructionInstanceModelGenerator::generate(const std::vector<ComponentInstance> &componentInstances, std::map<unsigned int, unsigned int> &compInstMapping, QStringList &errorStack, std::set<unsigned int> &idSet) {
	FUNCID(ConstructionInstanceModelGenerator::generate);
	std::map<int, unsigned int>				tempToZoneId;		// holds temperature (const) key: temperature values (temperature*10); value ground zone id
	std::map<unsigned int, unsigned int>	schedIdToZoneId;	// key: temperature schedule id (VICUS); value ground zone id
	QStringList warnings;
	// now process all components and generate construction instances
	for (const VICUS::ComponentInstance & compInstaVicus : componentInstances) {
		// Note: component ID may be invalid or component may have been deleted from DB already
		const VICUS::Component * comp = VICUS::element(m_project->m_embeddedDB.m_components, compInstaVicus.m_idComponent);
		if (comp == nullptr){
			errorStack.append(qApp->tr("Component ID #%1 is referenced from component instance with id #%2, but there is no such component.")
							  .arg(compInstaVicus.m_idComponent).arg(compInstaVicus.m_id));
			continue;
		}
		// now generate a construction instance
		NANDRAD::ConstructionInstance constrInstNandrad;
		//search for a free id
		constrInstNandrad.m_id = VICUS::uniqueId(idSet);//compInstaVicus.m_id;
		//now add id to Set
		idSet.insert(constrInstNandrad.m_id);

		// save both ids in the componentInstanceMapping
		compInstMapping[compInstaVicus.m_id] = constrInstNandrad.m_id;

		// set construction instance parameters, area, orientation etc.
		const double SAME_DISTANCE_PARAMETER_ABSTOL = 0.25;//1e-4;
		double area = 0;

		// save surface ids for shading factor file later
		unsigned int surfaceAId = INVALID_ID;
		unsigned int surfaceBId = INVALID_ID;

		bool bothSidesHaveSurfaces = false;
		// we have either one or two surfaces associated
		if (compInstaVicus.m_sideASurface != nullptr) {
			surfaceAId = compInstaVicus.m_sideASurface->m_id;
			// get area of surface A
			area = compInstaVicus.m_sideASurface->geometry().area();
			// do we have surfaces at both sides?
			if (compInstaVicus.m_sideBSurface != nullptr) {
				surfaceBId = compInstaVicus.m_sideBSurface->m_id;
				// have both
				bothSidesHaveSurfaces = true;
				double areaB = compInstaVicus.m_sideBSurface->geometry().area();
				// check if both areas are approximately the same

#define ABS_TEST
#ifdef ABS_TEST
				if (std::fabs(area - areaB) > SAME_DISTANCE_PARAMETER_ABSTOL) {
#else
				const double REL_TOL_AREAS = 0.05;
				// relative error check
				// area is set as reference
				if (std::fabs(areaB-area)/area > REL_TOL_AREAS) {
#endif
					errorStack.append(qApp->tr("Component/construction #%1 references surfaces #%2 '%6' and #%3 '%7', with mismatching "
											   "areas %4 and %5 m2.")
									  .arg(compInstaVicus.m_id).arg(compInstaVicus.m_idSideASurface)
									  .arg(compInstaVicus.m_idSideBSurface)
									  .arg(area).arg(areaB)
									  .arg(compInstaVicus.m_sideASurface->m_displayName,
										   compInstaVicus.m_sideBSurface->m_displayName));
				}

				/// TODO Dirk :	do we need to also store a displayname for each component instance/construction instance?
				///				We could also name internal walls automatically using zone names, such as
				///				"Wall between 'Bath' and 'Kitchen'".
				constrInstNandrad.m_displayName = qApp->tr("Internal wall between surfaces '#%1' and '#%2' (ID=%3)")
						.arg(compInstaVicus.m_sideASurface->m_displayName, compInstaVicus.m_sideBSurface->m_displayName).arg(compInstaVicus.m_idSideASurface).toStdString();
			}
			else {

				// we only have side A, take orientation and inclination from side A
				const VICUS::Surface * s = compInstaVicus.m_sideASurface;

				// set parameters
				NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Inclination, s->geometry().inclination());
				NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Orientation, s->geometry().orientation());

				constrInstNandrad.m_displayName = IBK::FormatString("%1 (ID=%2)").arg(compInstaVicus.m_sideASurface->m_displayName.toStdString())
						.arg(compInstaVicus.m_idSideASurface).str();
			}

			if(area<MIN_AREA_FOR_EXPORTED_SURFACES){
				warnings << qApp->tr("Area of surface '%2' (#%1) is too small. Surface is not exported.")
							.arg(compInstaVicus.m_sideASurface->m_id)
							.arg(compInstaVicus.m_sideASurface->m_displayName);
				continue;
			}
			// set area parameter (computed from side A, but if side B is given as well, the area is the same
			NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, area);

			//for the first time we support only sub surfaces to outside air
			if(!bothSidesHaveSurfaces){
				// sub surface
				const std::vector<SubSurface> & subSurfs = compInstaVicus.m_sideASurface->subSurfaces();
				if(subSurfs.size()>0){
					//we have sub surfaces
					exportSubSurfaces(errorStack, subSurfs, compInstaVicus, constrInstNandrad, idSet, m_surfaceIdsVicusToNandrad);
				}
			}
		}
		else {
			// we must have a side B surface, otherwise this is an invalid component instance
			if (compInstaVicus.m_sideBSurface == nullptr){
				errorStack.append(qApp->tr("Component instance #%1 does neither reference a valid surface on side A nor on side B.")
								  .arg(compInstaVicus.m_id));
				continue;
			}
			surfaceBId = compInstaVicus.m_sideBSurface->m_id;

			const VICUS::Surface * s = compInstaVicus.m_sideBSurface;

			// set parameters
			NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Inclination, s->geometry().inclination());
			NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Orientation, s->geometry().orientation());

			// set area parameter
			area = compInstaVicus.m_sideBSurface->geometry().area();

			if(area<MIN_AREA_FOR_EXPORTED_SURFACES){
				warnings << qApp->tr("Area of surface '%2' (#%1) is too small. Surface is not exported.")
							.arg(compInstaVicus.m_sideBSurface->m_id)
							.arg(compInstaVicus.m_sideBSurface->m_displayName);
				continue;
			}
			NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, area);

			constrInstNandrad.m_displayName = IBK::FormatString("%1 (ID=%2)").arg(compInstaVicus.m_sideBSurface->m_displayName.toStdString())
					.arg(compInstaVicus.m_idSideBSurface).str();

			// sub surface
			const std::vector<SubSurface> & subSurfs = compInstaVicus.m_sideBSurface->subSurfaces();
			if(subSurfs.size()>0){
				//we have sub surfaces
				exportSubSurfaces(errorStack, subSurfs, compInstaVicus, constrInstNandrad, idSet, m_surfaceIdsVicusToNandrad);
			}
		}

		// now generate interfaces

		if(!comp->isValid(m_materialsDB,m_constructionsDB, m_boundaryConditionsDB, m_scheduleDB)){
			errorStack << qApp->tr("Component #%1 '%2' is not valid.").arg(comp->m_id).arg(QString::fromStdString(comp->m_displayName.string()));
			continue;
		}

		// generate an interface, in case of a ground contact a new zone must be model or reference by an existing zone

		GroundZoneExport gze, gze2;
		gze.m_idSet.swap(idSet);
		gze.m_objListNames.swap(m_objectListNames);
		gze.m_tempToZoneId.swap(tempToZoneId);
		gze.m_tempSchedIdToZoneId.swap(schedIdToZoneId);
		constrInstNandrad.m_interfaceA = generateInterface(compInstaVicus, comp->m_idSideABoundaryCondition, 1, errorStack, gze, true);

		gze2.m_idSet.swap(gze.m_idSet);
		gze2.m_objListNames.swap(gze.m_objListNames);
		gze2.m_tempToZoneId.swap(gze.m_tempToZoneId);
		gze2.m_tempSchedIdToZoneId.swap(gze.m_tempSchedIdToZoneId);
		constrInstNandrad.m_interfaceB = generateInterface(compInstaVicus, comp->m_idSideBBoundaryCondition, 2, errorStack, gze2, false);

		idSet.swap(gze2.m_idSet);
		m_objectListNames.swap(gze2.m_objListNames);
		tempToZoneId.swap(gze2.m_tempToZoneId);
		schedIdToZoneId.swap(gze2.m_tempSchedIdToZoneId);

		// a new ground zones was created
		// save all data
		if(gze.m_groundZ.m_id != VICUS::INVALID_ID && gze2.m_groundZ.m_id != VICUS::INVALID_ID)
			errorStack.append(qApp->tr("Construction instance with surface #%1 '%2'"));		/// TODO Dirk
		else if(gze2.m_groundZ.m_id != VICUS::INVALID_ID){
			// only work with gze so through away gze data and take gze2 data
			gze = gze2;
		}

		if(gze.m_groundZ.m_id != VICUS::INVALID_ID){
			// add zone
			m_zones.push_back(gze.m_groundZ);
			// add spline or schedule
			if(!gze.m_schedules.empty()){
				m_schedsMap[gze.m_objList.m_name] = gze.m_schedules;
				// add object list
				m_objectLists.push_back(gze.m_objList);
			}
			else if(gze.m_spline != NANDRAD::LinearSplineParameter()){
				m_splinesMap[gze.m_objList.m_name].push_back(gze.m_spline);
				// add object list
				m_objectLists.push_back(gze.m_objList);
			}
		}
		else{
			// found a surface to outside air
			// so we have to remember the id for the shading factor file
			if( constrInstNandrad.m_interfaceA.m_id != NANDRAD::INVALID_ID &&
					constrInstNandrad.m_interfaceB.m_id != NANDRAD::INVALID_ID &&
					( constrInstNandrad.m_interfaceA.m_zoneId == 0 || constrInstNandrad.m_interfaceB.m_zoneId == 0 )){
				unsigned int vicusId = surfaceAId != INVALID_ID ? surfaceAId : surfaceBId;
				m_surfaceIdsVicusToNandrad[vicusId] = constrInstNandrad.m_id;
			}
		}


		int activeLayerIdx = -1;
		//create surface heating system data
		if(compInstaVicus.m_idSurfaceHeatingControlZone != INVALID_ID && compInstaVicus.m_idSurfaceHeating != INVALID_ID){
			//get an area check for surface heating systems
			if(area <= 1e-4){
				errorStack.append(qApp->tr("Component instance #%1 area is too small. No surface heating/cooling export possible.")
								  .arg(compInstaVicus.m_id));
				continue;
			}
			m_surfaceHeatingData.push_back(DataSurfaceHeating(compInstaVicus.m_idSurfaceHeatingControlZone,
															  compInstaVicus.m_idSurfaceHeating, constrInstNandrad.m_id,
															  compInstaVicus.m_idSupplySystem,
															  area));
			activeLayerIdx = (int)comp->m_activeLayerIndex;
		}

		// store reference to construction type (i.e. to be generated from component)
		constrInstNandrad.m_constructionTypeId = getNandradConstructionTypeId(comp->m_idConstruction, activeLayerIdx);
		// add to list of construction instances
		m_constructionInstances.push_back(constrInstNandrad);
	}

	for(const QString &str : warnings)
		IBK::IBK_Message(IBK::FormatString("%1").arg(str.toStdString()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);


}

void ConstructionInstanceModelGenerator::addInputData(const std::vector<NANDRAD::ObjectList> &objLists){
	m_objectLists = objLists;
	for(unsigned int i=0; i<m_objectLists.size(); ++i)
		m_objectListNames.insert(QString::fromStdString(m_objectLists[i].m_name));
}

void ConstructionInstanceModelGenerator::generateMaterials() {
	// we have constructions and materials already in the embedded database, so we can just copy them over
	for (const VICUS::Material & m : m_project->m_embeddedDB.m_materials) {
		NANDRAD::Material matdata;

		matdata.m_id = m.m_id;
		matdata.m_displayName = m.m_displayName.string(IBK::MultiLanguageString::m_language, "en");

		// now transfer parameters - fortunately, they have the same keywords, what a coincidence :-)
		matdata.m_para[NANDRAD::Material::P_Density] = m.m_para[VICUS::Material::P_Density];
		matdata.m_para[NANDRAD::Material::P_HeatCapacity] = m.m_para[VICUS::Material::P_HeatCapacity];
		matdata.m_para[NANDRAD::Material::P_Conductivity] = m.m_para[VICUS::Material::P_Conductivity];

		// addConstruction to material list
		m_materials.push_back(matdata);
	}
}

void ConstructionInstanceModelGenerator::generateConstructions(QStringList &errorStack) {

	for (const VICUS::Construction & c : m_project->m_embeddedDB.m_constructions) {

		ActiveLayer actLayer;
		//check if we have more construction with different active layers
		for(unsigned int i=0; i<m_constructionTypeData.size(); ++i){
			const ActiveLayer &aL = m_constructionTypeData[i];
			if(aL.m_vicusConstructionId == c.m_id){
				actLayer = aL;
				break;
			}
		}

		//create a new construction type for each active layer
		for(unsigned int i=0; i<actLayer.m_activeLayers.size(); ++i){
			// now create a construction type
			NANDRAD::ConstructionType conType;
			conType.m_id = actLayer.m_nandradConstructionTypeId[i];			//c.m_id;
			conType.m_displayName = c.m_displayName.string(IBK::MultiLanguageString::m_language, "en");
			for (const VICUS::MaterialLayer & ml : c.m_materialLayers) {
				NANDRAD::MaterialLayer mlayer;
				mlayer.m_matId = ml.m_idMaterial;
				mlayer.m_thickness = ml.m_thickness.value;
				conType.m_materialLayers.push_back(mlayer);
			}
			if(actLayer.m_activeLayers[i] != -1)
				conType.m_activeLayerIndex = (unsigned int)actLayer.m_activeLayers[i];

			// add to construction type list
			m_constructions.push_back(conType);

		}

	}

	for(const VICUS::WindowGlazingSystem &w : m_project->m_embeddedDB.m_windowGlazingSystems){
		if(w.m_modelType != VICUS::WindowGlazingSystem::MT_Simple){
			errorStack << qApp->tr("The window glazing system with #%1 and name '%2' is not supported by the export.")
						  .arg(w.m_id).arg(QString::fromStdString(w.m_displayName.string()));
			continue;
		}
		else{
			NANDRAD::WindowGlazingSystem winG;
			if(!w.isValid()){
				errorStack << qApp->tr("The window glazing system with #%1 and name '%2' is not valid. Export failed.")
							  .arg(w.m_id).arg(QString::fromStdString(w.m_displayName.string()));
				continue;
			}
			winG.m_displayName = w.m_displayName.string();
			winG.m_id = w.m_id;
			winG.m_modelType = NANDRAD::WindowGlazingSystem::MT_Simple;
			NANDRAD::KeywordList::setParameter(winG.m_para, "WindowGlazingSystem::para_t",
											   NANDRAD::WindowGlazingSystem::P_ThermalTransmittance,
											   w.uValue());
			winG.m_splinePara[NANDRAD::WindowGlazingSystem::SP_SHGC]= w.m_splinePara[VICUS::WindowGlazingSystem::SP_SHGC];

			m_windowGlazingSystems.push_back(winG);
		}
	}
}

void ThermostatModelGenerator::generate(const Room *r,std::vector<unsigned int> &usedModelIds,  QStringList &errorStack) {

	// check if we have a zone template with id to thermostat

	const ZoneControlThermostat				* thermostat = nullptr;

	try {
		thermostat = dynamic_cast<const ZoneControlThermostat*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_ControlThermostat));
	}  catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	const VICUS::ZoneTemplate * zoneTemplate = VICUS::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);

	// no model defined?
	if (thermostat == nullptr)
		return;

	if (!thermostat->isValid(m_scheduleDB)){
		errorStack.append(qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
						  .arg(zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_ControlThermostat])
				.arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)));
		return;
	}

	// only continue if there were no errors so far
	if (!errorStack.isEmpty())
		return;


	NANDRAD::Thermostat thermo;

	thermo.m_id = VICUS::uniqueIdWithPredef(usedModelIds, thermostat->m_id);
	thermo.m_displayName = "Thermostat_" + zoneTemplate->m_displayName.string();
	thermo.m_controllerType = thermostat->m_controllerType == VICUS::ZoneControlThermostat::CT_Analog ?
				NANDRAD::Thermostat::CT_Analog : NANDRAD::Thermostat::CT_Digital;
	thermo.m_modelType = NANDRAD::Thermostat::MT_Scheduled;		// only this is provided by GUI
	thermo.m_temperatureType = thermostat->m_controlValue == VICUS::ZoneControlThermostat::CV_AirTemperature ?
				NANDRAD::Thermostat::TT_AirTemperature : NANDRAD::Thermostat::TT_OperativeTemperature;

	NANDRAD::KeywordList::setParameter(thermo.m_para, "Thermostat::para_t",
									   NANDRAD::Thermostat::P_TemperatureTolerance,
									   thermostat->m_para[ZoneControlThermostat::P_Tolerance].value);



	// *** schedules ***
	// schedule generation:
	//
	// 1. create basic schedule (name?)
	std::vector<NANDRAD::Schedule> scheds;
	std::vector<NANDRAD::LinearSplineParameter>	splines;
	VICUS::Schedule s;

	const Schedule * heatSched = m_scheduleDB[thermostat->m_idHeatingSetpointSchedule];
	const Schedule * coolSched = m_scheduleDB[thermostat->m_idCoolingSetpointSchedule];

	if(heatSched != nullptr)
		heatSched->insertIntoNandradSchedulegroup( "HeatingSetpointSchedule [C]" , scheds, splines, m_placeholders);
	else{
		s.createConstSchedule(-100);
		s.insertIntoNandradSchedulegroup( "HeatingSetpointSchedule [C]" , scheds);
	}

	if(coolSched != nullptr)
		coolSched->insertIntoNandradSchedulegroup( "CoolingSetpointSchedule [C]" , scheds, splines, m_placeholders);
	else{
		s.createConstSchedule(200);
		s.insertIntoNandradSchedulegroup( "CoolingSetpointSchedule [C]" , scheds);
	}

	// now we have a valid schedule group, yet without object list name

	// Now we check if we have already an thermostat model object with exactly the same parameters (except ID and name, object list).
	// Then, we also compare the matching schedule (the thermostat model object and corresponding schedule have same ID).
	// If this schedule is also identitical to our generated schedule, we simply extend the object list by our zone ID
	// otherwise we add model and schedule definitions and generate a new object list.
	bool foundModel=false;
	for (unsigned int i=0; i<m_thermostats.size(); ++i) {
		if (m_thermostats[i].equal(thermo) &&
				NANDRAD::Schedules::equalSchedules(m_schedules[i], scheds) &&
				NANDRAD::Schedules::equalAnnualSchedules(m_schedGroupSplines[i], splines)) {
			// insert our zone ID in object list
			m_objLists[i].m_filterID.m_ids.insert(r->m_id);
			foundModel = true;
			break;
		}
	}
	if(!foundModel) {
		// append definitions and create new object list
		NANDRAD::ObjectList ol;
		ol.m_name = IBK::pick_name("Thermostat-" + zoneTemplate->m_displayName.string(), m_objListNames.begin(), m_objListNames.end());
		ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ol.m_filterID.m_ids.insert(r->m_id);

		// set object list in new definition
		thermo.m_zoneObjectList = ol.m_name;

		// add all definitions
		m_thermostats.push_back(thermo);
		m_schedules.push_back(scheds);
		m_schedGroupSplines.push_back(splines);
		m_objLists.push_back(ol);
		m_objListNames.push_back(ol.m_name);

	}
}

void IdealSurfaceHeatingCoolingModelGenerator::generate(const std::vector<DataSurfaceHeating> &dataSurfaceHeating, std::vector<unsigned int> &usedModelIds, QStringList &errorStack) {

	// Create a outdoor air temperature data line for calculate the supply fluid temperature later
	IBK::LinearSpline outdoorTemp;

	CCM::ClimateDataLoader ccm;
	IBK::Path climatePath = m_project->m_location.m_climateFilePath;
	climatePath.removeRelativeParts();
	climatePath = climatePath.withReplacedPlaceholders(m_placeholders);

	try {
		ccm.readClimateData(climatePath);

	}  catch (...) {
		errorStack << QString("The climate file is not valid. Heating and cooling curve cannot be calculated. Filepath '%1'")
					  .arg(QString::fromStdString(climatePath.str()));
		return;
	}

	//first vector -> timepoints; second temperature in C
	std::vector<double> timepoints = ccm.m_dataTimePoints;
	if(timepoints.empty())
		for(unsigned int i=0; i< 8760; ++i)
			timepoints.push_back(i);
	else
		for (double &val : timepoints)
			val /=3600;

	outdoorTemp.setValues(timepoints, ccm.m_data[CCM::ClimateDataLoader::Temperature]);
	outdoorTemp.m_extrapolationMethod = IBK::LinearSpline::EM_Constant;

	std::vector<NANDRAD::IdealSurfaceHeatingCoolingModel>	idealSurfHeatCool;
	std::vector<NANDRAD::IdealPipeRegisterModel>			idealPipeRegister;
	std::map<unsigned int, std::vector<NANDRAD::LinearSplineParameter>>	constructionIdToNandradSplines;

	//fill the map for quick work
	for(unsigned int i=0; i<dataSurfaceHeating.size(); ++i){
		const DataSurfaceHeating &dsh = dataSurfaceHeating[i];

		const SurfaceHeating * surfSys =
				VICUS::element(m_project->m_embeddedDB.m_surfaceHeatings, dsh.m_heatingSystemId);
		VICUS::Database<NetworkPipe> pipeDB(0);
		pipeDB.setData(m_project->m_embeddedDB.m_pipes);
		if(surfSys == nullptr || !surfSys->isValid(pipeDB)){
			errorStack.append(qApp->tr("Invalid surface heating/cooling system #%1 referenced from room #%2.")
							  .arg(dsh.m_heatingSystemId)
							  .arg(dsh.m_controlledZoneId)
							  /*.arg(MultiLangString2QString(zoneTemplate->m_displayName))*/);
			continue;
		}

		//create a system for nandrad
		switch(surfSys->m_type){
		case SurfaceHeating::T_Ideal:{
			NANDRAD::IdealSurfaceHeatingCoolingModel nandradSys;
			nandradSys.m_thermostatZoneId = dsh.m_controlledZoneId;
			NANDRAD::KeywordList::setParameter(nandradSys.m_para, "IdealSurfaceHeatingCoolingModel::para_t",
											   NANDRAD::IdealSurfaceHeatingCoolingModel::P_MaxHeatingPowerPerArea,
											   surfSys->m_para[ZoneIdealHeatingCooling::P_HeatingLimit].value);

			NANDRAD::KeywordList::setParameter(nandradSys.m_para, "IdealSurfaceHeatingCoolingModel::para_t",
											   NANDRAD::IdealSurfaceHeatingCoolingModel::P_MaxCoolingPowerPerArea,
											   surfSys->m_para[ZoneIdealHeatingCooling::P_CoolingLimit].value);
			nandradSys.m_displayName = surfSys->m_displayName.string();
			//no id and objlist set now
			nandradSys.m_id = dsh.m_nandradConstructionInstanceId;		//we use this to save the construction instance id
			idealSurfHeatCool.push_back(nandradSys);
		}
			break;
		case SurfaceHeating::T_PipeRegister:{

			if(dsh.m_supplySystemId == INVALID_ID) {
				//standard fluid model
				NANDRAD::HydraulicFluid fluid;
				fluid.defaultFluidWater();

				//get area of the construction instance
				double area= dsh.m_area;
				double pipeSpacing = surfSys->m_para[VICUS::SurfaceHeating::P_PipeSpacing].value;
				double length = area / pipeSpacing;
				//set pipe length to max 120 m
				int numberPipes=1;
				if( length > 100){
					numberPipes = (int)std::ceil( length / 100.);
					length = area / pipeSpacing / (double)numberPipes;
				}
				const VICUS::NetworkPipe * pipe = VICUS::element(m_project->m_embeddedDB.m_pipes, surfSys->m_idPipe);
				double insideDiameter = pipe->diameterInside();

				double maxMassFlux = IBK::PI * insideDiameter * insideDiameter * 0.25 *
						surfSys->m_para[VICUS::SurfaceHeating::P_MaxFluidVelocity].value *
						fluid.m_para[VICUS::NetworkFluid::P_Density].value * numberPipes;

				double uValue = pipe->UValue();

				std::vector<double> supplyTemperatureVec;
				calculateSupplyTemperature(surfSys->m_heatingCoolingCurvePoints.m_values.at("Tsupply"),
										   surfSys->m_heatingCoolingCurvePoints.m_values.at("Tout"),
										   outdoorTemp.y(), supplyTemperatureVec);

				//TODO Dirk->Andreas wie geht das anzuhängen ans NANDRAD projekt?
				//kann sein das ich das erst speichern muss ... gucken ...
				NANDRAD::LinearSplineParameter tSupply("SupplyTemperatureSchedule",NANDRAD::LinearSplineParameter::I_LINEAR,
													   outdoorTemp.x(), supplyTemperatureVec,
													   IBK::Unit("h"),IBK::Unit("C"));

				//m_linearSplinePara.push_back(tSupply);

				NANDRAD::LinearSplineParameter massFlux("MaxMassFluxSchedule", NANDRAD::LinearSplineParameter::I_LINEAR,
														std::vector<double>{0,8760}, std::vector<double>{maxMassFlux, maxMassFlux},
														IBK::Unit("h"),IBK::Unit("kg/s"));
				//m_linearSplinePara.push_back(massFlux);

				// ideal pipe register without network connection

				NANDRAD::IdealPipeRegisterModel nandradSys;
				nandradSys.m_thermostatZoneId = dsh.m_controlledZoneId;
				//always schedule in gui
				nandradSys.m_modelType = NANDRAD::IdealPipeRegisterModel::MT_Scheduled;
				nandradSys.m_fluid = fluid;

				NANDRAD::KeywordList::setIntPara(nandradSys.m_intPara, "IdealPipeRegisterModel::intPara_t",
												 NANDRAD::IdealPipeRegisterModel::IP_NumberParallelPipes, numberPipes);

				NANDRAD::KeywordList::setParameter(nandradSys.m_para, "IdealPipeRegisterModel::para_t",
												   NANDRAD::IdealPipeRegisterModel::P_PipeLength, length);

				NANDRAD::KeywordList::setParameter(nandradSys.m_para, "IdealPipeRegisterModel::para_t",
												   NANDRAD::IdealPipeRegisterModel::P_MaxMassFlux, maxMassFlux);

				NANDRAD::KeywordList::setParameter(nandradSys.m_para, "IdealPipeRegisterModel::para_t",
												   NANDRAD::IdealPipeRegisterModel::P_PipeInnerDiameter, insideDiameter * 1000); //Attention this value is in mm

				NANDRAD::KeywordList::setParameter(nandradSys.m_para, "IdealPipeRegisterModel::para_t",
												   NANDRAD::IdealPipeRegisterModel::P_UValuePipeWall, uValue);


				nandradSys.m_displayName = "Underfloor heating";
				nandradSys.m_id = dsh.m_nandradConstructionInstanceId;

				idealPipeRegister.push_back(nandradSys);
				constructionIdToNandradSplines[dsh.m_nandradConstructionInstanceId] = std::vector<NANDRAD::LinearSplineParameter>{tSupply,massFlux};
			}
		}
			break;
		case SurfaceHeating::NUM_T: break;	//only for compiler
		}
	}

	// Now we check if we have already an ideal conditioning model object with exactly the same parameters (except ID and name, object list).
	// Then, we also compare the matching schedule (the ideal conditioning model object and corresponding schedule have same ID).
	// If this schedule is also identitical to our generated schedule, we simply extend the object list by our zone ID
	// otherwise we add model and schedule definitions and generate a new object list.

	for (unsigned int i=0; i<idealSurfHeatCool.size(); ++i) {
		bool isAdded = false;
		NANDRAD::IdealSurfaceHeatingCoolingModel &ideal = idealSurfHeatCool[i];
		for(unsigned int j=0; j<m_idealSurfaceHeatingCoolings.size(); ++j){
			if(ideal.equal(m_idealSurfaceHeatingCoolings[j]) ){
				m_objListsSurface[j].m_filterID.m_ids.insert(ideal.m_id);
				isAdded = true;
				break;
			}
		}
		if(!isAdded){
			NANDRAD::ObjectList ol;
			ol.m_name = IBK::pick_name("Ideal Surface Heating Construction Instance " + ideal.m_displayName,
									   m_objListNamesSurface.begin(), m_objListNamesSurface.end());
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
			ol.m_filterID.m_ids.insert(ideal.m_id);		//remember we have save the construction instance id in this id

			//get a new unique id for this element
			ideal.m_id = VICUS::uniqueIdWithPredef(usedModelIds, ideal.m_id);

			// set object list in new definition
			ideal.m_constructionObjectList= ol.m_name;

			// add all definitions
			m_idealSurfaceHeatingCoolings.push_back(ideal);
			m_objListsSurface.push_back(ol);
			m_objListNamesSurface.push_back(ol.m_name);
		}
	}

	for (unsigned int i=0; i<idealPipeRegister.size(); ++i) {
		bool isAdded = false;
		NANDRAD::IdealPipeRegisterModel &pipReg = idealPipeRegister[i];
		for(unsigned int j=0; j<m_idealPipeRegister.size(); ++j){
			if(pipReg.equal(m_idealPipeRegister[j]) ){
				m_objListsPipe[j].m_filterID.m_ids.insert(pipReg.m_id);
				isAdded = true;
				break;
			}
		}
		if(!isAdded){
			NANDRAD::ObjectList ol;
			ol.m_name = IBK::pick_name("Ideal Pipe Register Construction Instance " + pipReg.m_displayName,
									   m_objListNamesPipe.begin(), m_objListNamesPipe.end());
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
			ol.m_filterID.m_ids.insert(pipReg.m_id);		//remember we have save the construction instance id in this id

			//get a new unique id for this element
			pipReg.m_id = VICUS::uniqueIdWithPredef(usedModelIds, pipReg.m_id);

			// set object list in new definition
			pipReg.m_constructionObjectList= ol.m_name;

			// add all definitions
			m_idealPipeRegister.push_back(pipReg);
			m_objListsPipe.push_back(ol);
			m_objListNamesPipe.push_back(ol.m_name);
		}
	}

	for(std::map<unsigned int, std::vector<NANDRAD::LinearSplineParameter>>::iterator it=constructionIdToNandradSplines.begin();
		it!=constructionIdToNandradSplines.end();
		++it){
		bool isAdded = false;
		for(std::map<std::string, std::vector<NANDRAD::LinearSplineParameter>>::iterator	it2=m_constructionIdToNandradSplines.begin();
			it2!=m_constructionIdToNandradSplines.end();
			++it2){
			if(it->second==it2->second){
				for(unsigned int i=0; i<m_objListLinearSpline.size(); ++i){
					NANDRAD::ObjectList &ol = m_objListLinearSpline[i];
					if(ol.m_name == it2->first){
						ol.m_filterID.m_ids.insert(it->first);
						break;
					}
				}
				isAdded = true;
				break;
			}
		}

		if(!isAdded){
			NANDRAD::ObjectList ol;
			ol.m_name = "Supply Temperature and mass flux" + IBK::val2string(it->first);
			ol.m_referenceType = NANDRAD::ModelInputReference::MRT_CONSTRUCTIONINSTANCE;
			ol.m_filterID.m_ids.insert(it->first);

			//add object list and nandrad splines
			m_objListLinearSpline.push_back(ol);
			m_constructionIdToNandradSplines[ol.m_name] = it->second;
			m_objListNamesLinearSplines.push_back(ol.m_name);
		}

	}

}




void SupplySystemNetworkModelGenerator::generate(const SupplySystem & supplySystem,
												 const std::vector<DataSurfaceHeating> & dataSurfaceHeating,
												 std::vector<unsigned int> &usedModelIds,
												 std::vector<unsigned int> &usedNetworkIds,
												 std::vector<unsigned int> &usedNetworkElementIds,
												 QStringList &errorStack) {
	FUNCID(SupplySystemNetworkModelGenerator::generate);

	// 1.) Create a new network

	NANDRAD::HydraulicNetwork network;
	network.m_id = VICUS::uniqueIdAdd(usedNetworkIds);


	// 2.) Create network components
	std::vector<unsigned int> allNetworkComponentIds;
	std::vector<unsigned int> allNetworkControllerIds;
	std::vector<unsigned int> allNetworkNodeIds;
	unsigned int systemSplitterNodeId = INVALID_ID; // the node of the splitter (outlet node of supply system = splitter for pipe network)
	unsigned int systemMixerNodeId = INVALID_ID; // the node of the mixer (outlet of the pipe network = inlet of the supply system)

	// create components (only needed if no Sub Network is used)
	NANDRAD::HydraulicNetworkComponent nandradPump;
	NANDRAD::HydraulicNetworkComponent nandradHeater;

	// remember pipe ids
	std::set<unsigned int> allPipeIds;

	// create pipe DB
	Database<NetworkPipe> pipeDB(0);
	pipeDB.setData(m_project->m_embeddedDB.m_pipes);

	// If no Vicus SubNetwork is given: create pump and heater
	if (supplySystem.m_supplyType != SupplySystem::ST_SubNetwork) {

		// create a smass flux controlled pump
		nandradPump.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_ControlledPump;
		nandradPump.m_displayName = "Mass flux controlled pump";
		NANDRAD::KeywordList::setParameter(nandradPump.m_para, "HydraulicNetworkComponent::para_t",
										   NANDRAD::HydraulicNetworkComponent::P_PumpMaximumEfficiency, 1.0);
		NANDRAD::KeywordList::setParameter(nandradPump.m_para, "HydraulicNetworkComponent::para_t",
										   NANDRAD::HydraulicNetworkComponent::P_Volume, 0.01);
		NANDRAD::KeywordList::setParameter(nandradPump.m_para, "HydraulicNetworkComponent::para_t",
										   NANDRAD::HydraulicNetworkComponent::P_MaximumPressureHead, 0.0);
		NANDRAD::KeywordList::setParameter(nandradPump.m_para, "HydraulicNetworkComponent::para_t",
										   NANDRAD::HydraulicNetworkComponent::P_PumpMaximumElectricalPower, 0.0);
		// set id 1 as first element
		nandradPump.m_id = 1;
		// add to component vector
		network.m_components.push_back(nandradPump);

		// create an ideal heater for each network
		nandradHeater.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_IdealHeaterCooler;
		nandradHeater.m_displayName = "Ideal heater";
		// set id 4 as first element
		nandradHeater.m_id = 4;
		// add to component vector
		network.m_components.push_back(nandradHeater);
	}

	// If Vicus SubNetwork is given: transfer it into our nandrad network
	else {

		// create Databases from embedded Databases
		Database<NetworkComponent> dbNetworkComps = Database<NetworkComponent>(1);
		dbNetworkComps.setData(m_project->m_embeddedDB.m_networkComponents);
		Database<NetworkController> dbNetworkCtrl = Database<NetworkController>(1);
		dbNetworkCtrl.setData(m_project->m_embeddedDB.m_networkControllers);
		Database<Schedule> dbSchedules = Database<Schedule>(1); // we dont care
		dbSchedules.setData(m_project->m_embeddedDB.m_schedules);

		// checks
		const SubNetwork *subNetwork = element(m_project->m_embeddedDB.m_subNetworks, supplySystem.m_idSubNetwork);
		if (subNetwork == nullptr) {
			QString msg = qApp->tr("Sub Network with id #%1 does not exist in database").arg(supplySystem.m_idSubNetwork);
			errorStack.append(msg);
			throw IBK::Exception(msg.toStdString(), FUNC_ID);
		}
		if (!subNetwork->isValid(dbNetworkComps, dbNetworkCtrl, dbSchedules)) {
			QString msg = qApp->tr("Sub Network with id #%1 has invalid parameters").arg(supplySystem.m_idSubNetwork);
			errorStack.append(msg);
			throw IBK::Exception(msg.toStdString(), FUNC_ID);
		}

		// *** Transfer sub network data
		std::set<unsigned int> subNetComponentIds;
		std::set<unsigned int> subNetControllerIds;
		std::set<unsigned int> subNetScheduleIds;
		for (const NetworkElement &elem: subNetwork->m_elements) {

			// check and collect components
			const VICUS::NetworkComponent *comp = dbNetworkComps[elem.m_componentId];
			if (comp == nullptr) {
				QString msg = qApp->tr("Network component with id #%1 does not exist in database").arg(elem.m_componentId);
				errorStack.append(msg);
				throw IBK::Exception(msg.toStdString(), FUNC_ID);
			}
			if (!comp->isValid(dbSchedules))
				errorStack.append(qApp->tr("Network component with id #%1 has invalid parameters").arg(elem.m_componentId));

			subNetComponentIds.insert(comp->m_id);

			// check and collect controller
			if (elem.m_controlElementId != INVALID_ID) {
				const VICUS::NetworkController *ctrl = dbNetworkCtrl[elem.m_controlElementId];
				if (ctrl == nullptr) {
					QString msg = qApp->tr("Network controller with id #%1 does not exist in database").arg(elem.m_controlElementId);
					errorStack.append(msg);
					throw IBK::Exception(msg.toStdString(), FUNC_ID);
				}
				if (!ctrl->isValid(dbSchedules))
					errorStack.append(qApp->tr("Network controller with id #%1 has invalid parameters").arg(elem.m_controlElementId));
				else
					subNetControllerIds.insert(ctrl->m_id);
			}

			// collect schedules ids
			for (unsigned int id: comp->m_scheduleIds)
				subNetScheduleIds.insert(id);

		}
		if (!errorStack.empty())
			return;

		// *** Transfer components
		for (unsigned int compId: subNetComponentIds) {
			const VICUS::NetworkComponent *comp = dbNetworkComps[compId];
			NANDRAD::HydraulicNetworkComponent nandradComp;
			nandradComp.m_id = compId; // we use the same id's as in VICUS, that simplifies transfer of elements
			allNetworkComponentIds.push_back(compId);
			nandradComp.m_modelType = NetworkComponent::nandradNetworkComponentModelType(comp->m_modelType);
			nandradComp.m_displayName = comp->m_displayName.string(IBK::MultiLanguageString::m_language, "en");
			comp->nandradNetworkComponentParameter(nandradComp.m_para);
			nandradComp.m_polynomCoefficients = comp->m_polynomCoefficients;
			if (comp->m_pipePropertiesId != INVALID_ID)
				allPipeIds.insert(comp->m_pipePropertiesId);

			network.m_components.push_back(nandradComp);
		}

		// *** Transfer controller
		for (unsigned int ctrId: subNetControllerIds) {
			const VICUS::NetworkController *ctr = dbNetworkCtrl[ctrId];
			NANDRAD::HydraulicNetworkControlElement nandradCtr;
			nandradCtr.m_id = ctrId; // we use the same id's as in VICUS, that simplifies transfer of elements
			allNetworkControllerIds.push_back(ctrId);
			nandradCtr.m_modelType = NANDRAD::HydraulicNetworkControlElement::ModelType(ctr->m_modelType);
			nandradCtr.m_controllerType = NANDRAD::HydraulicNetworkControlElement::ControllerType(ctr->m_controllerType);
			nandradCtr.m_controlledProperty = NANDRAD::HydraulicNetworkControlElement::ControlledProperty(ctr->m_controlledProperty);
			nandradCtr.m_maximumControllerResultValue = ctr->m_maximumControllerResultValue;
			for (unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_P; ++i)
				nandradCtr.m_para[i] = ctr->m_para[i];
			for (unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_ID; ++i)
				nandradCtr.m_idReferences[i] = ctr->m_idReferences[i];

			network.m_controlElements.push_back(nandradCtr);
		}

		// remember all inlet / outlet node ids
		for (const NetworkElement &elem: subNetwork->m_elements) {
			if (elem.m_inletNodeId != SubNetwork::INLET_ID)
				allNetworkNodeIds.push_back(elem.m_inletNodeId);
			if (elem.m_inletNodeId != SubNetwork::OUTLET_ID)
				allNetworkNodeIds.push_back(elem.m_outletNodeId);
		}


		// *** Transfer elements
		std::map<unsigned int, std::vector<unsigned int>> components2ElementsMap;
		for (const NetworkElement &elem: subNetwork->m_elements) {

			NANDRAD::HydraulicNetworkElement nandradElement;
			nandradElement.m_id = uniqueIdAdd(usedNetworkElementIds);
			nandradElement.m_displayName = elem.m_displayName.toStdString();
			nandradElement.m_componentId = elem.m_componentId;
			nandradElement.m_controlElementId = elem.m_controlElementId;

			// replace system inlet/outlet node ids with new node ids, keep other node ids
			if (elem.m_inletNodeId == SubNetwork::INLET_ID) {
				nandradElement.m_inletNodeId = uniqueIdAdd(allNetworkNodeIds);
				systemMixerNodeId = nandradElement.m_inletNodeId; // we remember this node id for pipe connection later
			}
			else
				nandradElement.m_inletNodeId = elem.m_inletNodeId;

			if (elem.m_outletNodeId == SubNetwork::OUTLET_ID) {
				nandradElement.m_outletNodeId = uniqueIdAdd(allNetworkNodeIds);
				systemSplitterNodeId = nandradElement.m_outletNodeId; // we remember this node id for pipe connection later
			}
			else
				nandradElement.m_outletNodeId = elem.m_outletNodeId;

			// retrieve id from the component
			const NetworkComponent *comp = dbNetworkComps[elem.m_componentId];
			if (comp->m_pipePropertiesId != INVALID_ID)
				nandradElement.m_pipePropertiesId = comp->m_pipePropertiesId;
			// for pipes: retrieve parameters from the component
			if (comp->m_modelType == VICUS::NetworkComponent::MT_SimplePipe ||
					comp->m_modelType == VICUS::NetworkComponent::MT_DynamicPipe ){
				NANDRAD::KeywordList::setParameter(nandradElement.m_para, "HydraulicNetworkElement::para_t",
												   NANDRAD::HydraulicNetworkElement::P_Length,
												   comp->m_para[VICUS::NetworkComponent::P_PipeLength].value);
				nandradElement.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes] = comp->m_intPara[VICUS::NetworkComponent::IP_NumberParallelPipes];
			}
			// for PressureLossElement
			if (comp->m_modelType == VICUS::NetworkComponent::MT_PressureLossElement){
				nandradElement.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelElements] =
						comp->m_intPara[VICUS::NetworkComponent::IP_NumberParallelElements];
			}
			// remember elements for each component, which has schedules (for object list)
			if (!comp->m_scheduleIds.empty())
				components2ElementsMap[elem.m_componentId].push_back(nandradElement.m_id);

			// transfer element
			network.m_elements.push_back(nandradElement);
		}

		// set reference element id
		Q_ASSERT(network.m_elements.size()>0);
		network.m_referenceElementId = network.m_elements[0].m_id;

		// *** Transfer schedules and create object lists
		for(auto it=components2ElementsMap.begin(); it!=components2ElementsMap.end(); ++it){

			// get component and required schedule names
			const NetworkComponent *comp = dbNetworkComps[it->first];
			Q_ASSERT(comp!=nullptr);
			NANDRAD::HydraulicNetworkComponent::ModelType modelType = NetworkComponent::nandradNetworkComponentModelType(comp->m_modelType);
			std::vector<std::string> schedNames = NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(modelType);
			Q_ASSERT(schedNames.size() == comp->m_scheduleIds.size()); // this has been checked in isValid() above

			// for each schedule we create an object list and schedule group
			for (unsigned int i=0; i<comp->m_scheduleIds.size(); ++i) {

				// check schedule
				unsigned int schedId = comp->m_scheduleIds[i];
				const Schedule *sched = dbSchedules[schedId];
				if (sched == nullptr) {
					QString msg = qApp->tr("Schedule with id #%1 does not exist in database").arg(schedId);
					errorStack.append(msg);
					continue;
				}
				std::string err;
				if (!sched->isValid(err, true, m_placeholders)){
					errorStack.append(QString::fromStdString(err));
					continue;
				}

				// create object list
				NANDRAD::ObjectList objList;
				std::string objListName = comp->m_displayName.string() + " - " + sched->m_displayName.string();
				bool createNewObjList = true;
				// Note: we cannot have 2 object lists with same name
				// if we have a object list with identical name already, just add the ids to that
				for (NANDRAD::ObjectList &objList: m_objLists) {
					if (objList.m_name == objListName) {
						for (unsigned int elemId: it->second)
							objList.m_filterID.m_ids.insert(elemId);
						createNewObjList = false;
					}

				}
				if (createNewObjList) {
					// if not: create new object list
					objList.m_name = objListName;
					objList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
					for (unsigned int elemId: it->second)
						objList.m_filterID.m_ids.insert(elemId);
					m_objLists.push_back(objList);
				}

				// transfer schedule
				m_scheduleGroups[objListName] = std::vector<NANDRAD::Schedule>();
				std::vector<NANDRAD::LinearSplineParameter> splines;
				sched->insertIntoNandradSchedulegroup(schedNames[i], m_scheduleGroups[objListName], splines, m_placeholders);
				// annual schedules handled separately
				if (!splines.empty())
					m_annualSchedules[objListName].push_back(splines[0]);
			}
		}

	} // end if model type SubNetwork


	// for now set all hydraulic node ids to a height of 0, this can be considered in the future for high buildings
	for (unsigned int id: allNetworkNodeIds)
		network.m_nodes.push_back(NANDRAD::HydraulicNetworkNode(id, 0));

	// create a supply/return pipe for each network
	NANDRAD::HydraulicNetworkComponent nandradSupplyPipe;
	nandradSupplyPipe.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_SimplePipe;
	nandradSupplyPipe.m_displayName = "Supply/return pipe";
	nandradSupplyPipe.m_id = uniqueIdAdd(allNetworkComponentIds);
	network.m_components.push_back(nandradSupplyPipe);

	// create underfloor heating pipe for each network
	NANDRAD::HydraulicNetworkComponent nandradUnderfloorHeatingPipe;
	nandradUnderfloorHeatingPipe.m_modelType = NANDRAD::HydraulicNetworkComponent::MT_SimplePipe;
	nandradUnderfloorHeatingPipe.m_displayName = "Underfloor heating";
	nandradUnderfloorHeatingPipe.m_id = uniqueIdAdd(allNetworkComponentIds);
	network.m_components.push_back(nandradUnderfloorHeatingPipe);


	// 3.) Create network fluid

	// standard fluid model
	NANDRAD::HydraulicFluid fluid;
	fluid.defaultFluidWater();

	network.m_fluid = fluid;


	// 4.) Configurate network

	// retrieve maximum mass flux through the supply branch
	double maxMassFlux = 1.0; // default value in case no value is given

	switch (supplySystem.m_supplyType) {
	case VICUS::SupplySystem::ST_StandAlone:
		maxMassFlux = supplySystem.m_para[VICUS::SupplySystem::P_MaximumMassFlux].value;
		break;
	case VICUS::SupplySystem::ST_UserDefinedFMU:
		maxMassFlux = supplySystem.m_para[VICUS::SupplySystem::P_MaximumMassFluxFMU].value;
		break;
		// not supported, yet
	case VICUS::SupplySystem::ST_SubNetwork:
		maxMassFlux = 1.0; // for the dimensioning of supply pipe lengths, the exact value does not matter
		break;
	case VICUS::SupplySystem::ST_DatabaseFMU: break;
	case VICUS::SupplySystem::NUM_ST: break; // just to make compiler happy
	}

	double defaultFluidTemperature = 20. + 273.15;
	NANDRAD::KeywordList::setParameter(network.m_para, "HydraulicNetwork::para_t",
									   NANDRAD::HydraulicNetwork::P_InitialFluidTemperature, 20.);

	// pre calculations

	// calculate pressure loss, number of pipes and pipe lenght for all surafce heatings
	std::vector<double> pipeLengths(dataSurfaceHeating.size(), 0.0);
	std::vector<double> pressureLosses(dataSurfaceHeating.size(), 0.0);
	std::vector<int> numbersOfPipes(dataSurfaceHeating.size(), 0);
	// store pipe id
	std::vector<unsigned int> surfHeatingPipeIds(dataSurfaceHeating.size(), 0);
	// store maximum pressure loss
	double maxPressureLoss = 0.0;

	for(unsigned int i=0; i<dataSurfaceHeating.size(); ++i){
		const DataSurfaceHeating &dsh = dataSurfaceHeating[i];

		const SurfaceHeating * surfHeating = element(m_project->m_embeddedDB.m_surfaceHeatings, dsh.m_heatingSystemId);

		if(surfHeating == nullptr || !surfHeating->isValid(pipeDB)){
			errorStack.append(qApp->tr("Invalid surface heating/cooling system #%1 referenced from room #%2.")
							  .arg(dsh.m_heatingSystemId)
							  .arg(dsh.m_controlledZoneId)
							  /*.arg(MultiLangString2QString(zoneTemplate->m_displayName))*/);
			continue;
		}

		//create a system for nandrad
		Q_ASSERT(surfHeating->m_type == SurfaceHeating::T_PipeRegister);

		//get area of the construction instance
		double area= dsh.m_area;
		double pipeSpacing = surfHeating->m_para[SurfaceHeating::P_PipeSpacing].value;
		double length = area / pipeSpacing;
		//set pipe length to max 120 m
		int numberPipes=1;
		if( length > 100){
			numberPipes = (int)std::ceil( length / 100.);
			length = area / pipeSpacing / (double)numberPipes;
		}

		// store pipe lenght
		pipeLengths[i] = length;
		// store number of pipes
		numbersOfPipes[i] = numberPipes;
		// store pipe id
		surfHeatingPipeIds[i] = surfHeating->m_idPipe;

		// store vicus pipe id
		allPipeIds.insert(surfHeating->m_idPipe);

		// check pipe
		const NetworkPipe * pipe = pipeDB[surfHeating->m_idPipe];
		if (pipe == nullptr) {
			QString msg = qApp->tr("Pipe with id #%1 does not exist in database").arg(surfHeating->m_idPipe);
			errorStack.append(msg);
			continue;
		}
		if (!pipe->isValid()){
			errorStack.append(qApp->tr("Pipe with id #%1 has invalid parameters").arg(surfHeating->m_idPipe));
			continue;
		}

		// calculate pipe pressure loss
		double density = fluid.m_para[NANDRAD::HydraulicFluid::P_Density].value;
		double diameter = pipe->diameterInside();
		double maxVelocity = maxMassFlux/(density * IBK::PI/4. * diameter * diameter);
		double viscosity = fluid.m_kinematicViscosity.m_values.value(defaultFluidTemperature);
		double reynolds = std::abs(maxVelocity) * diameter / viscosity;
		double roughness = pipe->m_para[VICUS::NetworkPipe::P_RoughnessWall].value;
		double zeta = length/diameter * IBK::FrictionFactorSwamee(reynolds, diameter, roughness);
		double pressureLoss = zeta * density / 2.0 * std::abs(maxVelocity) * maxVelocity;

		// store pressure loss
		pressureLosses[i] = pressureLoss;
		// and update maximum

		maxPressureLoss = std::max(maxPressureLoss, pressureLoss);
	}
	if (!errorStack.empty())
		return;


	// 5.) Create network elements

	NANDRAD::HydraulicNetworkElement pumpElem;
	NANDRAD::HydraulicNetworkElement idealHeaterElem;
	NANDRAD::ObjectList idealHeaterObjectList;
	NANDRAD::ObjectList pumpObjectList;

	// If there is no Vicus Subnetwork: create pump and heater

	if (supplySystem.m_supplyType != SupplySystem::ST_SubNetwork) {

		// create id's for system splitter / mixer
		systemMixerNodeId = uniqueIdAdd(allNetworkNodeIds);
		systemSplitterNodeId = uniqueIdAdd(allNetworkNodeIds);

		// create pump
		pumpElem.m_id = VICUS::uniqueIdAdd(usedNetworkElementIds);
		pumpElem.m_componentId = nandradPump.m_id;
		pumpElem.m_displayName = "Mass flux controlled pump";
		pumpElem.m_inletNodeId = uniqueIdAdd(allNetworkNodeIds);
		pumpElem.m_outletNodeId = systemSplitterNodeId;
		// create a control element: first control element is for mass flux
		pumpElem.m_controlElementId = uniqueIdAdd(allNetworkControllerIds);

		network.m_elements.push_back(pumpElem);

		// set pump as reference
		network.m_referenceElementId = pumpElem.m_id;

		// create mass flux control element
		NANDRAD::HydraulicNetworkControlElement massFluxControl;
		// always reserve 1 for mass flux controller
		massFluxControl.m_id = pumpElem.m_controlElementId;
		massFluxControl.m_modelType = NANDRAD::HydraulicNetworkControlElement::MT_Scheduled;
		massFluxControl.m_controlledProperty = NANDRAD::HydraulicNetworkControlElement::CP_MassFlux;
		massFluxControl.m_controllerType = NANDRAD::HydraulicNetworkControlElement::CT_PController;
		NANDRAD::KeywordList::setParameter(massFluxControl.m_para, "HydraulicNetworkControlElement::para_t",
										   NANDRAD::HydraulicNetworkControlElement::P_Kp, 100000000.);

		network.m_controlElements.push_back(massFluxControl);

		// create a pump object list
		pumpObjectList.m_name = std::string("Pump ") + supplySystem.m_displayName.string();
		pumpObjectList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		pumpObjectList.m_filterID.m_ids.insert(pumpElem.m_id);

		// connect to ideal heating
		idealHeaterElem.m_id = VICUS::uniqueIdAdd(usedNetworkElementIds);
		idealHeaterElem.m_displayName = "Ideal heater";
		idealHeaterElem.m_componentId = nandradHeater.m_id;
		idealHeaterElem.m_inletNodeId = systemMixerNodeId;
		idealHeaterElem.m_outletNodeId = pumpElem.m_inletNodeId;

		network.m_elements.push_back(idealHeaterElem);

		// create a ideal heater object list
		idealHeaterObjectList.m_name = std::string("Ideal heater ") + supplySystem.m_displayName.string();
		idealHeaterObjectList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		idealHeaterObjectList.m_filterID.m_ids.insert(idealHeaterElem.m_id);
	}


	// generate parallel pipe network
	for(unsigned int i=0; i<dataSurfaceHeating.size(); ++i) {

		if (dataSurfaceHeating[i].m_area < 0.5)
			continue;

		// calculate lenght of supply pipe for pressure equailzation
		double lengthSupply = (maxPressureLoss - pressureLosses[i]) / pressureLosses[i] * pipeLengths[i];

		// set initial inlet node id for the underfloor pipe
		unsigned int underfloorPipeInletNodeId = systemSplitterNodeId;

		//		if(!IBK::nearly_equal<4>(lengthSupply, 0.0 ) ) {
		//			// craetea supply pipe and connect to pump
		//			NANDRAD::HydraulicNetworkElement supplyPipeElem;
		//			supplyPipeElem.m_id = VICUS::uniqueIdAdd(usedNetworkElementIds);
		//			// meaningful display name
		//			supplyPipeElem.m_displayName = IBK::FormatString("supply pipe zone id %1 ").arg(dataSurfaceHeating[i].m_controlledZoneId).str();
		//			// connect to pipe properties
		//			supplyPipeElem.m_pipePropertiesId = surfHeatingPipeIds[i];
		//			// connect to component 'Supply pipe'
		//			supplyPipeElem.m_componentId = nandradSupplyPipe.m_id;
		//			supplyPipeElem.m_inletNodeId = systemSplitterNodeId;
		//			supplyPipeElem.m_outletNodeId = uniqueIdAdd(allNetworkNodeIds);
		//			underfloorPipeInletNodeId = supplyPipeElem.m_outletNodeId;

		//			NANDRAD::KeywordList::setParameter(supplyPipeElem.m_para, "HydraulicNetworkElement::para_t",
		//											   NANDRAD::HydraulicNetworkElement::P_Length, lengthSupply);
		//			NANDRAD::KeywordList::setIntPara(supplyPipeElem.m_intPara, "HydraulicNetworkElement::intPara_t",
		//											   NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes, numbersOfPipes[i]);

		//			// add element to hydraulic network
		//			network.m_elements.push_back(supplyPipeElem);
		//		}

		//get lenght and number of pipes for surface heating parametrization
		double length = pipeLengths[i];
		int numberOfPipes = numbersOfPipes[i];

		// create and fill a new network element
		NANDRAD::HydraulicNetworkElement underfloorPipeElem;
		underfloorPipeElem.m_id = VICUS::uniqueIdAdd(usedNetworkElementIds);
		// meaningful display name
		underfloorPipeElem.m_displayName = IBK::FormatString("underfloor pipe zone id %1 ").arg(dataSurfaceHeating[i].m_controlledZoneId).str();
		// connect to pipe properties
		underfloorPipeElem.m_pipePropertiesId = surfHeatingPipeIds[i];
		// connect to component 'Underfloor heating'
		underfloorPipeElem.m_componentId = nandradUnderfloorHeatingPipe.m_id;
		// set a control model with defined id
		underfloorPipeElem.m_controlElementId = uniqueIdAdd(allNetworkControllerIds);

		underfloorPipeElem.m_inletNodeId = underfloorPipeInletNodeId;
		underfloorPipeElem.m_outletNodeId = systemMixerNodeId; // underfloor pipe outlet id is system mixer (=system inlet) id

		NANDRAD::KeywordList::setIntPara(underfloorPipeElem.m_intPara, "HydraulicNetworkElement::intPara_t",
										 NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes, numberOfPipes);
		NANDRAD::KeywordList::setParameter(underfloorPipeElem.m_para, "HydraulicNetworkElement::para_t",
										   NANDRAD::HydraulicNetworkElement::P_Length, length);

		// add heat exchange type
		NANDRAD::HydraulicNetworkHeatExchange heatExchange;
		heatExchange.m_modelType = NANDRAD::HydraulicNetworkHeatExchange::T_TemperatureConstructionLayer;
		heatExchange.m_idReferences[NANDRAD::HydraulicNetworkHeatExchange::ID_ConstructionInstanceId] =
				dataSurfaceHeating[i].m_nandradConstructionInstanceId;

		underfloorPipeElem.m_heatExchange = heatExchange;

		// add element to hydraulic network
		network.m_elements.push_back(underfloorPipeElem);

		// create mass flux control element
		NANDRAD::HydraulicNetworkControlElement thermostatControl;
		// always reserve 1 for mass flux controller
		thermostatControl.m_id = underfloorPipeElem.m_controlElementId;
		thermostatControl.m_modelType = NANDRAD::HydraulicNetworkControlElement::MT_Constant;
		thermostatControl.m_controlledProperty = NANDRAD::HydraulicNetworkControlElement::CP_ThermostatValue;
		thermostatControl.m_idReferences[NANDRAD::HydraulicNetworkControlElement::ID_ThermostatZoneId] =
				dataSurfaceHeating[i].m_controlledZoneId;
		thermostatControl.m_controllerType = NANDRAD::HydraulicNetworkControlElement::CT_PController;
		NANDRAD::KeywordList::setParameter(thermostatControl.m_para, "HydraulicNetworkControlElement::para_t",
										   NANDRAD::HydraulicNetworkControlElement::P_Kp, 1e6);
		thermostatControl.m_maximumControllerResultValue = 0;

		// add to control elements
		network.m_controlElements.push_back(thermostatControl);
	}


	// 6.) Create pipe properties

	for (unsigned int pipeId: allPipeIds) {

		// rertieve poiter for easy access
		const VICUS::NetworkPipe *pipe = pipeDB[pipeId];

		// check
		if (pipe == nullptr) {
			QString msg = qApp->tr("Pipe with id #%1 does not exist in database").arg(pipeId);
			errorStack.append(msg);
			continue;
		}
		if (!pipe->isValid()){
			errorStack.append(qApp->tr("Pipe with id #%1 has invalid parameters").arg(pipeId));
			continue;
		}

		// fill pipe properties
		NANDRAD::HydraulicNetworkPipeProperties nandradPipeProp;

		// copy id number
		nandradPipeProp.m_id = pipe->m_id;
		NANDRAD::KeywordList::setParameter(nandradPipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter,
										   pipe->m_para[VICUS::NetworkPipe::P_DiameterOutside].get_value("mm"));
		NANDRAD::KeywordList::setParameter(nandradPipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter,
										   pipe->diameterInside() * 1000); // m -> mmm
		NANDRAD::KeywordList::setParameter(nandradPipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeRoughness,
										   pipe->m_para[VICUS::NetworkPipe::P_RoughnessWall].get_value("mm"));
		NANDRAD::KeywordList::setParameter(nandradPipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_UValueWall, pipe->UValue());
		NANDRAD::KeywordList::setParameter(nandradPipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_DensityWall,
										   pipe->m_para[VICUS::NetworkPipe::P_DensityWall].value);
		NANDRAD::KeywordList::setParameter(nandradPipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_HeatCapacityWall,
										   pipe->m_para[VICUS::NetworkPipe::P_HeatCapacityWall].value);

		// add property to hydraulic network
		network.m_pipeProperties.push_back(nandradPipeProp);
	}
	if (!errorStack.empty())
		return;

	// add to network vector
	m_hydraulicNetworks.push_back(network);


	if (supplySystem.m_supplyType != SupplySystem::ST_SubNetwork) {

		// 7.) Create interface translation models

		// create summation model
		NANDRAD::HeatLoadSummationModel summationModel;
		summationModel.m_id = VICUS::uniqueIdAdd(usedModelIds);
		usedModelIds.push_back(summationModel.m_id);
		summationModel.m_displayName = std::string("Heat load summation ") + supplySystem.m_displayName.string();
		summationModel.m_objectList = idealHeaterObjectList.m_name;

		// add to model vector
		m_summationModels.push_back(summationModel);

		// create adapter model
		NANDRAD::NetworkInterfaceAdapterModel adapterModel;
		adapterModel.m_id = VICUS::uniqueIdAdd(usedModelIds);
		usedModelIds.push_back(adapterModel.m_id);
		adapterModel.m_displayName = std::string("Network adapter ") + supplySystem.m_displayName.string();
		adapterModel.m_summationModelId = summationModel.m_id;
		// change fluid capacity pareameter name
		adapterModel.m_fluidHeatCapacity.set("FluidHeatCapacity", fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].value,
				fluid.m_para[NANDRAD::HydraulicFluid::P_HeatCapacity].unit());

		// add to model vector
		m_adapterModels.push_back(adapterModel);

		// create a adapter object list
		NANDRAD::ObjectList adapterObjectList;
		adapterObjectList.m_name = adapterModel.m_displayName;
		adapterObjectList.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
		adapterObjectList.m_filterID.m_ids.insert(adapterModel.m_id);


		// 8.) Create schedule parameter

		std::vector<double> maxMassFluxVals(1, 0.0);
		std::vector<double> supplyTempVals(1, 20.);

		if(supplySystem.m_supplyType == VICUS::SupplySystem::ST_StandAlone) {
			maxMassFluxVals[0] = supplySystem.m_para[SupplySystem::P_MaximumMassFlux].value;
			supplyTempVals[0] = supplySystem.m_para[SupplySystem::P_SupplyTemperature].get_value("C");
		}
		else {
			maxMassFluxVals[0] = supplySystem.m_para[SupplySystem::P_MaximumMassFluxFMU].value;
		}

		// fluid mass flux setpoint
		NANDRAD::DailyCycle massFluxSetDailyCycle;
		massFluxSetDailyCycle.m_timePoints.push_back(0);
		massFluxSetDailyCycle.m_values.m_values["MassFluxSetpointSchedule [kg/s]"] = maxMassFluxVals;

		NANDRAD::Schedule massFluxSetSchedule;
		massFluxSetSchedule.m_type = NANDRAD::Schedule::ST_ALLDAYS;
		massFluxSetSchedule.m_dailyCycles.push_back(massFluxSetDailyCycle);

		// fluid mass flux
		NANDRAD::DailyCycle massFluxDailyCycle;
		massFluxDailyCycle.m_timePoints.push_back(0);
		massFluxDailyCycle.m_values.m_values["MassFluxSchedule [kg/s]"] = maxMassFluxVals;

		NANDRAD::Schedule massFluxSchedule;
		massFluxSchedule.m_type = NANDRAD::Schedule::ST_ALLDAYS;
		massFluxSchedule.m_dailyCycles.push_back(massFluxDailyCycle);

		// fluid supply temperature
		NANDRAD::DailyCycle supplyTempDailyCycle;
		supplyTempDailyCycle.m_timePoints.push_back(0);
		supplyTempDailyCycle.m_values.m_values["SupplyTemperatureSchedule [C]"] = supplyTempVals;

		NANDRAD::Schedule supplyTempSchedule;
		supplyTempSchedule.m_type = NANDRAD::Schedule::ST_ALLDAYS;
		supplyTempSchedule.m_dailyCycles.push_back(supplyTempDailyCycle);


		// add new schedule and object list: pump
		m_objLists.push_back(pumpObjectList);
		m_scheduleGroups[pumpObjectList.m_name].push_back(massFluxSetSchedule);
		// ideal heater
		m_objLists.push_back(idealHeaterObjectList);
		m_scheduleGroups[idealHeaterObjectList.m_name].push_back(supplyTempSchedule);
		// adapter model
		m_objLists.push_back(adapterObjectList);
		m_scheduleGroups[adapterObjectList.m_name].push_back(massFluxSchedule);
		m_scheduleGroups[adapterObjectList.m_name].push_back(supplyTempSchedule);


		// 9.) Create FMI description

		switch(supplySystem.m_supplyType) {
		case VICUS::SupplySystem::ST_DatabaseFMU:
		case VICUS::SupplySystem::ST_UserDefinedFMU: {

			unsigned int id = 3 * (m_hydraulicNetworks.size() - 1);

			// add an fmu description for mass flux: pump
			NANDRAD::FMIVariableDefinition inputMassFlux;
			inputMassFlux.m_varName = "NetworkElement.MassFluxSetpointSchedule";
			inputMassFlux.m_objectId = pumpElem.m_id;
			inputMassFlux.m_fmiVarName = "MassFlow";
			inputMassFlux.m_fmiVarDescription = "Demand network mass flow.";
			inputMassFlux.m_unit = "kg/s";
			inputMassFlux.m_fmiTypeName = "input";
			inputMassFlux.m_fmiStartValue = 0.0;
			inputMassFlux.m_fmiValueRef = ++id;

			m_inputVariables.push_back(inputMassFlux);

			// add an fmu description for mass flux: adapter model
			inputMassFlux.m_varName = "Model.MassFluxSchedule";
			inputMassFlux.m_objectId = adapterModel.m_id;
			inputMassFlux.m_fmiVarName = "MassFlow";
			inputMassFlux.m_fmiVarDescription = "Demand network mass flow.";
			inputMassFlux.m_unit = "kg/s";
			inputMassFlux.m_fmiTypeName = "input";
			inputMassFlux.m_fmiStartValue = 0.0;
			inputMassFlux.m_fmiValueRef = id;

			m_inputVariables.push_back(inputMassFlux);

			// add an fmu description for supply temperature: ideal heater
			NANDRAD::FMIVariableDefinition inputSupplyTemp;
			inputSupplyTemp.m_varName = "NetworkElement.SupplyTemperatureSchedule";
			inputSupplyTemp.m_objectId = idealHeaterElem.m_id;
			inputSupplyTemp.m_fmiVarName = "SupplyTemperature";
			inputSupplyTemp.m_unit = "K";
			inputSupplyTemp.m_fmiVarDescription = "Demand network flow temperature.";
			inputSupplyTemp.m_fmiTypeName = "input";
			inputSupplyTemp.m_fmiStartValue = defaultFluidTemperature;
			inputSupplyTemp.m_fmiValueRef = ++id;

			m_inputVariables.push_back(inputSupplyTemp);

			// add an fmu description for supply temperature: adapter model
			inputSupplyTemp.m_varName = "Model.SupplyTemperatureSchedule";
			inputSupplyTemp.m_objectId = adapterModel.m_id;
			inputSupplyTemp.m_fmiVarName = "SupplyTemperature";
			inputSupplyTemp.m_unit = "K";
			inputSupplyTemp.m_fmiVarDescription = "Demand network flow temperature.";
			inputSupplyTemp.m_fmiTypeName = "input";
			inputSupplyTemp.m_fmiStartValue = defaultFluidTemperature;
			inputSupplyTemp.m_fmiValueRef = id;

			m_inputVariables.push_back(inputSupplyTemp);

			// add an fmu description for return temperature
			NANDRAD::FMIVariableDefinition outputReturnTemp;
			outputReturnTemp.m_varName = "Model.ReturnTemperature";
			outputReturnTemp.m_objectId = adapterModel.m_id;
			outputReturnTemp.m_fmiVarName = "ReturnTemperature";
			outputReturnTemp.m_unit = "K";
			outputReturnTemp.m_fmiVarDescription = "Demand network return temperature.";
			outputReturnTemp.m_fmiTypeName = "output";
			outputReturnTemp.m_fmiStartValue = defaultFluidTemperature;
			outputReturnTemp.m_fmiValueRef = ++id;

			m_outputVariables.push_back(outputReturnTemp);

		} break;
		default: break;
		}

	}


}



void IdealHeatingCoolingModelGenerator::generate(const Room * r,std::vector<unsigned int> &usedModelIds,  QStringList & errorStack) {

	// check if we have a zone template with id to ideal heating cooling

	const ZoneIdealHeatingCooling				* idealHeatingCooling = nullptr;

	try {
		idealHeatingCooling = dynamic_cast<const ZoneIdealHeatingCooling*>(findZoneSubTemplate(r, VICUS::ZoneTemplate::ST_IdealHeatingCooling));
	}  catch (IBK::Exception & ex) {
		errorStack.append( QString::fromStdString(ex.what()) );
		return;
	}

	const VICUS::ZoneTemplate * zoneTemplate = VICUS::element(m_project->m_embeddedDB.m_zoneTemplates, r->m_idZoneTemplate);

	// no model defined?
	if (idealHeatingCooling == nullptr)
		return;

	if (!idealHeatingCooling->isValid()){
		errorStack.append(qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
						  .arg(zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_IdealHeatingCooling])
				.arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)));
		return;
	}

	// only continue if there were no errors so far
	if (!errorStack.isEmpty())
		return;


	NANDRAD::IdealHeatingCoolingModel idealHeatCool;
	idealHeatCool.m_id = VICUS::uniqueIdWithPredef(usedModelIds,r->m_id);
	idealHeatCool.m_displayName = "IdealHeatCool_" + zoneTemplate->m_displayName.string();

	// if parameter is not set in VICUS data model, we default to 0
	double maxHeatingPowerPerArea = idealHeatingCooling->m_para[ZoneIdealHeatingCooling::P_HeatingLimit].empty() ?
				0 : idealHeatingCooling->m_para[ZoneIdealHeatingCooling::P_HeatingLimit].value;
	NANDRAD::KeywordList::setParameter(idealHeatCool.m_para, "IdealHeatingCoolingModel::para_t",
									   NANDRAD::IdealHeatingCoolingModel::P_MaxHeatingPowerPerArea,
									   maxHeatingPowerPerArea);

	double maxCoolingPowerPerArea = idealHeatingCooling->m_para[ZoneIdealHeatingCooling::P_CoolingLimit].empty() ?
				0 : idealHeatingCooling->m_para[ZoneIdealHeatingCooling::P_CoolingLimit].value;
	NANDRAD::KeywordList::setParameter(idealHeatCool.m_para, "IdealHeatingCoolingModel::para_t",
									   NANDRAD::IdealHeatingCoolingModel::P_MaxCoolingPowerPerArea,
									   maxCoolingPowerPerArea);


	// *** schedules ***
	// schedule generation:
	//
	// 1. create basic schedule (name?)


	// now we have a valid schedule group, yet without object list name

	// Now we check if we have already an thermostat model object with exactly the same parameters (except ID and name, object list).
	// Then, we also compare the matching schedule (the thermostat model object and corresponding schedule have same ID).
	// If this schedule is also identitical to our generated schedule, we simply extend the object list by our zone ID
	// otherwise we add model and schedule definitions and generate a new object list.
	bool foundModel=false;
	for (unsigned int i=0; i<m_idealHeatingCoolings.size(); ++i) {
		if (m_idealHeatingCoolings[i].equal(idealHeatCool) ){
			// insert our zone ID in object list
			m_objLists[i].m_filterID.m_ids.insert(r->m_id);
			foundModel = true;
			break;
		}
	}


	if (!foundModel) {
		// append definitions and create new object list
		NANDRAD::ObjectList ol;
		ol.m_name = IBK::pick_name("IdealHeatCool-" + zoneTemplate->m_displayName.string(), m_objListNames.begin(), m_objListNames.end());
		ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ol.m_filterID.m_ids.insert(r->m_id);

		// set object list in new definition
		idealHeatCool.m_zoneObjectList = ol.m_name;

		// add all definitions
		m_idealHeatingCoolings.push_back(idealHeatCool);
		m_objLists.push_back(ol);
		m_objListNames.push_back(ol.m_name);
	}

}



// *** NETWORK STUFF ***

void Project::generateNetworkProjectData(NANDRAD::Project & p, QStringList &errorStack, const std::string & nandradProjectPath, unsigned int networkId) const {
	FUNCID(Project::generateNetworkProjectData);

	// if there is no network selected return - this is not an error, but the usual case for simple building energy
	// simulations
	if (VICUS::element(m_geometricNetworks, networkId) == nullptr) {
		errorStack.append("No valid network selected for simulation.");
		return;
	}

	// create temporary copy of network
	VICUS::Network vicusNetwork = *VICUS::element(m_geometricNetworks, networkId);
	// update internal pointer links
	vicusNetwork.updateNodeEdgeConnectionPointers();

	// buildings can only have one connected edge
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_SubStation && node.m_edges.size()>1 ) {
			errorStack.append(tr("Node with id #%1 has more than one edge connected, but is a building.")
							  .arg(node.m_id));
		}
	}
	if (!errorStack.isEmpty())
		throw IBK::Exception("Conversion error.", FUNC_ID);

	// check network type
	if (vicusNetwork.m_type != VICUS::Network::NET_DoublePipe) {
		errorStack.append(tr("This NetworkType is not yet implemented. Use networkType 'DoublePipe'."));
		throw IBK::Exception("This NetworkType is not yet implemented. Use networkType 'DoublePipe'", FUNC_ID);
	}


	if (p.m_zones.empty()) {
		// create dummy zone
		NANDRAD::Zone z;
		z.m_id = 1;
		z.m_displayName = "dummy";
		z.m_type = NANDRAD::Zone::ZT_Active;
		NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, 100);
		NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, 10);
		p.m_zones.push_back(z);
	}

	// *** create Nandrad Network
	std::vector<unsigned int> allElementIds;

	// we do not prove network ids for uniqueness here
	for(const NANDRAD::HydraulicNetwork &network : p.m_hydraulicNetworks) {
		for(const NANDRAD::HydraulicNetworkElement &element : network.m_elements)
			allElementIds.push_back(element.m_id);
	}


	NANDRAD::HydraulicNetwork nandradNetwork;
	nandradNetwork.m_modelType = NANDRAD::HydraulicNetwork::ModelType(vicusNetwork.m_modelType);
	nandradNetwork.m_id = vicusNetwork.m_id;
	nandradNetwork.m_displayName = vicusNetwork.m_displayName.toStdString();
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_DefaultFluidTemperature] =
			vicusNetwork.m_para[VICUS::Network::P_DefaultFluidTemperature];
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_InitialFluidTemperature] =
			vicusNetwork.m_para[VICUS::Network::P_InitialFluidTemperature];
	nandradNetwork.m_para[NANDRAD::HydraulicNetwork::P_ReferencePressure] =
			vicusNetwork.m_para[VICUS::Network::P_ReferencePressure];


	// *** Transfer FLUID from Vicus to Nandrad

	const VICUS::NetworkFluid *fluid = VICUS::element(m_embeddedDB.m_fluids, vicusNetwork.m_idFluid);
	if (fluid == nullptr){
		errorStack.append(tr("Fluid not assigned or fluid with id #%1 does not exist in database!").arg(vicusNetwork.m_idFluid));
		return;
	}
	else if (!fluid->isValid())
		errorStack.append(tr("Fluid with id #%1 has invalid parameters!").arg(vicusNetwork.m_idFluid));
	if (!errorStack.isEmpty())
		throw IBK::Exception(IBK::FormatString("Invalid/missing fluid!"), FUNC_ID);
	nandradNetwork.m_fluid.m_displayName = fluid->m_displayName.string();
	nandradNetwork.m_fluid.m_kinematicViscosity = fluid->m_kinematicViscosity;
	for (int i=0; i<VICUS::NetworkFluid::NUM_P; ++i)
		nandradNetwork.m_fluid.m_para[i] = fluid->m_para[i];


	// create Databases from embedded Databases
	Database<NetworkComponent> dbNetworkComps = Database<NetworkComponent>(1); // we dont care
	dbNetworkComps.setData(m_embeddedDB.m_networkComponents);
	Database<NetworkController> dbNetworkCtrl = Database<NetworkController>(1); // we dont care
	dbNetworkCtrl.setData(m_embeddedDB.m_networkControllers);
	Database<Schedule> dbSchedules = Database<Schedule>(1); // we dont care
	dbSchedules.setData(m_embeddedDB.m_schedules);

	// *** Transfer COMPONENTS from Vicus to Nandrad

	// --> collect sub networks
	std::set<unsigned int> subNetworkIds, componentIdsVicus, controllerIds;
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Mixer)
			continue;
		subNetworkIds.insert(node.m_idSubNetwork);
	}

	// --> collect and check sub networks
	unsigned int maxNumberElements = 1;
	for (unsigned int subId: subNetworkIds){
		const VICUS::SubNetwork *sub = VICUS::element(m_embeddedDB.m_subNetworks, subId);

		// some checks
		if (sub == nullptr){
			errorStack.append(tr("Sub Network with id #%1 does not exist in database").arg(subId));
			continue;
		}
		if (!sub->isValid(dbNetworkComps, dbNetworkCtrl, dbSchedules))
			errorStack.append(tr("Sub Network with id #%1 has invalid parameters").arg(subId));

		// determine maximum number of elements of all sub networks
		if (sub->m_elements.size() > maxNumberElements)
			maxNumberElements = sub->m_elements.size();

		for (const NetworkElement &el: sub->m_elements){
			componentIdsVicus.insert(el.m_componentId);
			if (el.m_controlElementId != NANDRAD::INVALID_ID)
				controllerIds.insert(el.m_controlElementId);
		}
	}
	if (!errorStack.empty())
		return;

	// --> transfer components
	for (unsigned int compId: componentIdsVicus){
		const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, compId);
		if (comp == nullptr){
			errorStack.append(tr("Network Component with id #%1 does not exist in database").arg(compId));
			continue;
		}
		if (!comp->isValid(dbSchedules))
			errorStack.append(tr("Network Component with id #%1 has invalid parameters").arg(compId));

		NANDRAD::HydraulicNetworkComponent nandradComp;
		nandradComp.m_id = comp->m_id;
		nandradComp.m_displayName = comp->m_displayName.string(IBK::MultiLanguageString::m_language, "en");
		nandradComp.m_modelType = VICUS::NetworkComponent::nandradNetworkComponentModelType(comp->m_modelType);
		comp->nandradNetworkComponentParameter(nandradComp.m_para);
		nandradComp.m_polynomCoefficients = comp->m_polynomCoefficients;

		nandradNetwork.m_components.push_back(nandradComp);
	}
	if (!errorStack.empty())
		return;


	// --> transfer controllers
	for (unsigned int ctrId: controllerIds){

		const VICUS::NetworkController *ctr = VICUS::element(m_embeddedDB.m_networkControllers, ctrId);
		if (ctr == nullptr){
			errorStack.append(tr("Network Controller with id #%1 does not exist in database").arg(ctrId));
			continue;
		}
		if (!ctr->isValid(dbSchedules))
			errorStack.append(tr("Network Controller with id #%1 has invalid parameters").arg(ctrId));

		NANDRAD::HydraulicNetworkControlElement nandradCtr;
		nandradCtr.m_id = ctr->m_id;
		nandradCtr.m_modelType = NANDRAD::HydraulicNetworkControlElement::ModelType(ctr->m_modelType);
		nandradCtr.m_controllerType = NANDRAD::HydraulicNetworkControlElement::ControllerType(ctr->m_controllerType);
		nandradCtr.m_controlledProperty = NANDRAD::HydraulicNetworkControlElement::ControlledProperty(ctr->m_controlledProperty);
		nandradCtr.m_maximumControllerResultValue = ctr->m_maximumControllerResultValue;
		for (unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_P; ++i)
			nandradCtr.m_para[i] = ctr->m_para[i];
		for (unsigned int i=0; i<NANDRAD::HydraulicNetworkControlElement::NUM_ID; ++i)
			nandradCtr.m_idReferences[i] = ctr->m_idReferences[i];

		nandradNetwork.m_controlElements.push_back(nandradCtr);
	}
	if (!errorStack.empty())
		return;


	// *** Transform PIPES from Vicus to NANDRAD

	// --> collect all pipeIds used in vicus network
	std::set<unsigned int> pipeIds;
	for (const VICUS::NetworkEdge &edge: vicusNetwork.m_edges){
		if(edge.m_idPipe == VICUS::INVALID_ID)
			errorStack.append(tr("Edge '%1'->'%2' has no referenced pipe").arg(edge.nodeId1()).arg(edge.nodeId2()));
		pipeIds.insert(edge.m_idPipe);
	}
	for (unsigned int compId: componentIdsVicus){
		const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, compId);
		Q_ASSERT(comp != nullptr); // Note: we have checked the existence of comp above already
		// only few components have a pipeId, this is not mandatory
		if (comp->m_pipePropertiesId != INVALID_ID)
			pipeIds.insert(comp->m_pipePropertiesId);
	}
	if (!errorStack.empty())
		return;

	// --> transfer
	for(unsigned int pipeId: pipeIds) {

		const VICUS::NetworkPipe *pipe = VICUS::element(m_embeddedDB.m_pipes, pipeId);
		if (pipe == nullptr){
			errorStack.append(tr("Pipe with id #%1 does not exist in database").arg(pipeId));
			continue;
		}
		if (!pipe->isValid()){
			errorStack.append(tr("Pipe with id #%1 has invalid parameters").arg(pipeId));
			continue;
		}

		NANDRAD::HydraulicNetworkPipeProperties pipeProp;
		pipeProp.m_id = pipe->m_id;

		// set pipe properties
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeOuterDiameter,
										   pipe->m_para[VICUS::NetworkPipe::P_DiameterOutside].get_value("mm"));
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter,
										   pipe->diameterInside() * 1000); // m -> mmm
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_PipeRoughness,
										   pipe->m_para[VICUS::NetworkPipe::P_RoughnessWall].get_value("mm"));
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_UValueWall, pipe->UValue());
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_DensityWall,
										   pipe->m_para[VICUS::NetworkPipe::P_DensityWall].value);
		NANDRAD::KeywordList::setParameter(pipeProp.m_para, "HydraulicNetworkPipeProperties::para_t",
										   NANDRAD::HydraulicNetworkPipeProperties::P_HeatCapacityWall,
										   pipe->m_para[VICUS::NetworkPipe::P_HeatCapacityWall].value);
		nandradNetwork.m_pipeProperties.push_back(pipeProp);
	}
	if (!errorStack.empty())
		return;


	// *** Transfer ELEMENTS from Vicus to Nandrad

	// estimated number of elements
	nandradNetwork.m_elements.reserve(vicusNetwork.m_nodes.size() * maxNumberElements + 2 * vicusNetwork.m_edges.size());

	unsigned int fmiValueRef = 100;

	std::map<unsigned int, std::vector<unsigned int>> componentElementMap; // this map stores the NANDRAD element ids for each NANDRAD/VICUS component id
	std::vector<unsigned int> allNodeIds = {};			// stores all nodeIds of the network (the ids which are used to connect elements)
	std::map<unsigned int, unsigned int> supplyNodeIdMap; // a map that stores for each VICUS geometric node the NANDRAD inlet node
	std::map<unsigned int, unsigned int> returnNodeIdMap; // a map that stores for each VICUS geometric node the NANDRAD outlet node
	std::map<unsigned int, std::vector<unsigned int> > nodeElementsMap; // a map that stores for each VICUS geometric node the NANDRAD element ids (which are part of the according subnetwork)
	double referenceElementHeight = 0;
	std::map<unsigned int, double> heightOfHydraulicNode; // stores for each hydraulic nandrad node the height of the geometric VICUS node

	// iterate over all geometric network nodes
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes) {

		// for each vicus geometric node: store two new node ids (supply and return) for the nandrad network
		unsigned int inletNode = uniqueIdAdd(allNodeIds);
		supplyNodeIdMap[node.m_id] = inletNode;
		unsigned int outletNode = uniqueIdAdd(allNodeIds);
		returnNodeIdMap[node.m_id] = outletNode;

		// store the geodetic height of each hydraulic node
		heightOfHydraulicNode[inletNode] = node.m_position.m_z;
		heightOfHydraulicNode[outletNode] = node.m_position.m_z;

		// if this is a mixer continue
		if (node.m_type == VICUS::NetworkNode::NT_Mixer)
			continue;

		// check if there is a valid sub network
		const VICUS::SubNetwork *sub = VICUS::element(m_embeddedDB.m_subNetworks, node.m_idSubNetwork);
		if (sub == nullptr){
			errorStack.append(tr("Node with id #%1 has not a valid referenced sub network!").arg(node.m_id));
			continue;
		}


		// Check if we have at least one element that has inletNodeId=INLET_ID (this is the sub network inlet)
		// and at least one element with outletNodeId=OUTLET_ID (this is the sub network outlet)
		// Moreover: for the nodes which are not sub network inlet/outlet, but (local) inlet/outlet node ids:
		// create a map that stores new unique ids for them
		bool subInletFound = false;
		bool subOutletFound = false;
		std::map<unsigned int, unsigned int> subNetNodeIdMap;
		for (const NetworkElement &elem: sub->m_elements){

			if (elem.m_inletNodeId == VICUS::SubNetwork::INLET_ID){
				subInletFound = true;
				if (node.m_type == VICUS::NetworkNode::NT_Source)
					subNetNodeIdMap[elem.m_inletNodeId] = returnNodeIdMap[node.m_id];
				else
					subNetNodeIdMap[elem.m_inletNodeId] = supplyNodeIdMap[node.m_id];
			}

			if (elem.m_outletNodeId == VICUS::SubNetwork::OUTLET_ID){
				subOutletFound = true;
				if (node.m_type == VICUS::NetworkNode::NT_Source)
					subNetNodeIdMap[elem.m_outletNodeId] = supplyNodeIdMap[node.m_id];
				else
					subNetNodeIdMap[elem.m_outletNodeId] = returnNodeIdMap[node.m_id];
			}

			if (subNetNodeIdMap.find(elem.m_inletNodeId) == subNetNodeIdMap.end())
				subNetNodeIdMap[elem.m_inletNodeId] = uniqueIdAdd(allNodeIds);
			if (subNetNodeIdMap.find(elem.m_outletNodeId) == subNetNodeIdMap.end())
				subNetNodeIdMap[elem.m_outletNodeId] = uniqueIdAdd(allNodeIds);

		}
		if (!subInletFound)
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 does not have an element with inletNodeId=0, "
												   "This is required for every sub network").arg(node.m_id), FUNC_ID);
		if (!subOutletFound)
			throw IBK::Exception(IBK::FormatString("Sub Network with id #%1 does not have an element with outletNodeId=0, "
												   "This is required for every sub network").arg(node.m_id), FUNC_ID);


		// now we can create the new nandrad elements and map the repsctive nodes
		for (const NetworkElement &elem: sub->m_elements){

			// 1. copy the element and create a unique element id for it
			unsigned int id = uniqueIdAdd(allElementIds);
			NANDRAD::HydraulicNetworkElement nandradElement(id, elem.m_inletNodeId, elem.m_outletNodeId,
															NANDRAD::INVALID_ID, NANDRAD::INVALID_ID,
															elem.m_componentId, elem.m_controlElementId);

			// 2. set the new elements inlet and outlet node id using the map that we created
			nandradElement.m_inletNodeId = subNetNodeIdMap[elem.m_inletNodeId];
			nandradElement.m_outletNodeId = subNetNodeIdMap[elem.m_outletNodeId];

			// 2a. store the geodetic height of each hydraulic node that has not yet been stored
			heightOfHydraulicNode[nandradElement.m_inletNodeId] = node.m_position.m_z;
			heightOfHydraulicNode[nandradElement.m_outletNodeId] = node.m_position.m_z;

			// 3. create display name
			const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, nandradElement.m_componentId);
			Q_ASSERT(comp!=nullptr);
			IBK::FormatString name = IBK::FormatString("%1.%2(ID=%3)").arg(sub->m_displayName.string(IBK::MultiLanguageString::m_language, "en"))
					.arg(elem.m_displayName.toStdString()).arg(node.m_id);
			nandradElement.m_displayName = IBK::replace_string(name.str(), " ", "-");

			// 4. if this is a source node: set the respective reference element id of the network (for pressure calculation)
			if (node.m_type == VICUS::NetworkNode::NT_Source) {
				nandradNetwork.m_referenceElementId = nandradElement.m_id;
				referenceElementHeight = node.m_position.m_z;
			}

			// 5. if this element is the one which shall exchange heat: we copy the respective heat exchange properties from the node
			// we recognize this using the original element id (origElem.m_id)
			if (elem.m_id == sub->m_idHeatExchangeElement) {
				nandradElement.m_heatExchange = node.m_heatExchange;
				try {
					nandradElement.m_heatExchange.checkParameters(p.m_placeholders, p.m_zones, p.m_constructionInstances, false);
				}  catch (IBK::Exception &ex) {
					errorStack.append(tr("Problem in heat exchange definition of node #%1\n%2").arg(node.m_id).arg(QString::fromStdString(ex.msgStack())));
				}

			}

			// 6. SPECIAL CASE: pipes which are used in a sub network for e.g. ground heat exchangers (not the general network edge pipes)
			if (comp->m_modelType == VICUS::NetworkComponent::MT_SimplePipe ||
					comp->m_modelType == VICUS::NetworkComponent::MT_DynamicPipe ){

				// --> retrieve parameters from the component
				NANDRAD::KeywordList::setParameter(nandradElement.m_para, "HydraulicNetworkElement::para_t",
												   NANDRAD::HydraulicNetworkElement::P_Length,
												   comp->m_para[VICUS::NetworkComponent::P_PipeLength].value);
				nandradElement.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes] = comp->m_intPara[VICUS::NetworkComponent::IP_NumberParallelPipes];

				if (vicusNetwork.m_hasHeatExchangeWithGround) {
					// --> set FMI input
					NANDRAD::FMIVariableDefinition inputDefGHX;
					inputDefGHX.m_objectId = nandradElement.m_id;
					inputDefGHX.m_fmiVarName = nandradElement.m_displayName + ".Temperature";
					inputDefGHX.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
																		  NANDRAD::ModelInputReference::MRT_NETWORKELEMENT );
					inputDefGHX.m_varName += ".HeatExchangeTemperature";
					inputDefGHX.m_unit = "C";
					inputDefGHX.m_fmiValueRef = ++fmiValueRef;
					inputDefGHX.m_fmiVarDescription = "Pre-described external temperature";
					inputDefGHX.m_fmiStartValue = vicusNetwork.m_para[VICUS::Network::P_InitialFluidTemperature].value;
					p.m_fmiDescription.m_inputVariables.push_back(inputDefGHX);

					// --> set FMI output
					NANDRAD::FMIVariableDefinition outputDefGHX;
					outputDefGHX.m_objectId = nandradElement.m_id;
					outputDefGHX.m_fmiVarName = nandradElement.m_displayName + ".HeatLoss";
					outputDefGHX.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
																		   NANDRAD::ModelInputReference::MRT_NETWORKELEMENT );
					outputDefGHX.m_varName += ".FlowElementHeatLoss";
					outputDefGHX.m_unit = "W";
					outputDefGHX.m_fmiValueRef = ++fmiValueRef;
					outputDefGHX.m_fmiVarDescription = "Heat flux from flow element into environment";
					outputDefGHX.m_fmiStartValue = 0;
					p.m_fmiDescription.m_outputVariables.push_back(outputDefGHX);
				}
			}

			// PressureLossElement
			if (comp->m_modelType == VICUS::NetworkComponent::MT_PressureLossElement){
				nandradElement.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelElements] =
						comp->m_intPara[VICUS::NetworkComponent::IP_NumberParallelElements];
			}

			// 7. some components store a pipe properties id, so we transfer them to the element
			if (VICUS::NetworkComponent::hasPipeProperties(comp->m_modelType))
				nandradElement.m_pipePropertiesId = comp->m_pipePropertiesId;

			// 8. add element to the nandrad network
			nandradNetwork.m_elements.push_back(nandradElement);

			// 9. we store the element ids for each component, with this info we can create the schedules and object lists
			componentElementMap[elem.m_componentId].push_back(nandradElement.m_id);

			// 10. we store the element id for each geometric node
			nodeElementsMap[node.m_id].push_back(nandradElement.m_id);
		}

	}  // end of iteration over network nodes
	if(!errorStack.empty())
		return;


	// create the nandrad nodes and set their heights relative to the reference element height
	for (auto it=heightOfHydraulicNode.begin(); it!=heightOfHydraulicNode.end(); ++it)
		nandradNetwork.m_nodes.push_back(NANDRAD::HydraulicNetworkNode(it->first, it->second));
	for (NANDRAD::HydraulicNetworkNode &no: nandradNetwork.m_nodes)
		no.m_height -= referenceElementHeight;


	// Only for WorstpointControlledPump: store all element ids for each node of type Building
	// make sure there is only one ControlledPump with that option
	bool foundPump = false;
	for (auto it=nandradNetwork.m_elements.begin(); it!=nandradNetwork.m_elements.end(); ++it) {
		const NANDRAD::HydraulicNetworkControlElement *ctrElem = VICUS::element(nandradNetwork.m_controlElements, it->m_controlElementId);
		if (ctrElem != nullptr &&
				ctrElem->m_controlledProperty == NANDRAD::HydraulicNetworkControlElement::CP_PressureDifferenceWorstpoint) {
			if (foundPump) {
				throw IBK::Exception(IBK::FormatString("Found multiple ControlledPumps with option '%1'. There should be only one!").arg(
										 NANDRAD::KeywordList::Keyword("HydraulicNetworkControlElement::ControlledProperty",
																	   NANDRAD::HydraulicNetworkControlElement::CP_PressureDifferenceWorstpoint)), FUNC_ID);
			}
			foundPump = true;
			// now store the Nandrad element ids for all buildings in the pump element
			for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
				if (node.m_type == VICUS::NetworkNode::NT_SubStation)
					it->m_observedPressureDiffElementIds.m_values[node.m_id] = nodeElementsMap.at(node.m_id);
			}
		}
	}



	// *** Transfer EDGES / PIPE ELEMENTS from Vicus to Nandrad

	// --> Add Component for network pipes
	// get pipe model type from network and assign component type
	VICUS::NetworkComponent::ModelType pipeModelType;
	if (vicusNetwork.m_pipeModel == VICUS::Network::PM_SimplePipe)
		pipeModelType = VICUS::NetworkComponent::MT_SimplePipe;
	else if (vicusNetwork.m_pipeModel == VICUS::Network::PM_DynamicPipe)
		pipeModelType = VICUS::NetworkComponent::MT_DynamicPipe;
	else
		throw IBK::Exception(IBK::FormatString("No valid pipe model type chosen for network with id #%1")
							 .arg(vicusNetwork.m_id), FUNC_ID);

	// this vector contains component ids which are part of NANDRAD
	std::vector<unsigned int> componentIdsNandrad(componentIdsVicus.begin(), componentIdsVicus.end());
	// create and add component
	NANDRAD::HydraulicNetworkComponent networkPipeComponent;
	networkPipeComponent.m_id = uniqueIdAdd(componentIdsNandrad);
	networkPipeComponent.m_displayName = "Network pipe";
	networkPipeComponent.m_modelType = VICUS::NetworkComponent::nandradNetworkComponentModelType(pipeModelType);
	if (networkPipeComponent.m_modelType == NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe){
		if (vicusNetwork.m_para[VICUS::Network::P_MaxPipeDiscretization].empty())
			throw IBK::Exception(IBK::FormatString("Missing Parameter '%1' in network with id #%2")
								 .arg(VICUS::KeywordList::Keyword("Network::para_t", VICUS::Network::P_MaxPipeDiscretization))
								 .arg(vicusNetwork.m_id), FUNC_ID);
		NANDRAD::KeywordList::setParameter(networkPipeComponent.m_para, "HydraulicNetworkComponent::para_t",
										   NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth,
										   vicusNetwork.m_para[VICUS::Network::P_MaxPipeDiscretization].value);
	}
	nandradNetwork.m_components.push_back(networkPipeComponent);


	// find source node and create set of edges, which are ordered according to their distance to the source node
	std::set<const VICUS::NetworkNode *> dummyNodeSet;
	std::vector<VICUS::NetworkEdge *> orderedEdges;
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Source){
			node.setInletOutletNode(dummyNodeSet, reinterpret_cast< std::vector<const VICUS::NetworkEdge *> & >(orderedEdges));
			break;
		}
	}

	// now iterate over edges
	std::map<unsigned int, std::vector<unsigned int> > mapSoil2SupplyPipes;
	std::map<unsigned int, std::vector<unsigned int> > mapSoil2ReturnPipes;
	unsigned int idSoilModel = 0;

	for (VICUS::NetworkEdge *edge: orderedEdges) {

		// check if there is a reference to a pipe from DB
		const VICUS::NetworkPipe *pipe = VICUS::element(m_embeddedDB.m_pipes, edge->m_idPipe);
		if (pipe == nullptr){
			errorStack.append(tr("Edge %1->%2 has no defined pipe from database").arg(edge->m_node1->m_id).arg(edge->m_node2->m_id));
			continue;
		}

		// create name
		//		std::string nodeInletName, nodeOutletName;
		//		if (vicusNetwork.nodeById(edge->m_idNodeInlet)->m_displayName.isEmpty())
		//			nodeInletName = IBK::FormatString("Node#%1").arg(edge->m_idNodeInlet).str();
		//		else
		//			nodeInletName = vicusNetwork.nodeById(edge->m_idNodeInlet)->m_displayName.toStdString();
		//		if (vicusNetwork.nodeById(edge->m_idNodeOutlet)->m_displayName.isEmpty())
		//			nodeOutletName = IBK::FormatString("Node#%1").arg(edge->m_idNodeOutlet).str();
		//		else
		//			nodeOutletName = vicusNetwork.nodeById(edge->m_idNodeOutlet)->m_displayName.toStdString();
		//		IBK::FormatString pipeName = IBK::FormatString("%1->%2(ID=%3)").arg(nodeInletName).arg(nodeOutletName).arg(edge->m_id);

		// add inlet pipe element
		unsigned int inletNode = supplyNodeIdMap[edge->m_idNodeInlet];
		unsigned int outletNode = supplyNodeIdMap[edge->m_idNodeOutlet];
		NANDRAD::HydraulicNetworkElement supplyPipe(uniqueIdAdd(allElementIds),
													inletNode,
													outletNode,
													VICUS::INVALID_ID,
													VICUS::INVALID_ID,
													networkPipeComponent.m_id,
													edge->m_idPipe,
													edge->length());
		supplyPipe.m_displayName = IBK::FormatString("SupplyPipe(ID=%1)").arg(edge->m_id).str();
		supplyPipe.m_heatExchange = edge->m_heatExchange;
		try {
			supplyPipe.m_heatExchange.checkParameters(p.m_placeholders, p.m_zones, p.m_constructionInstances, false);
		}  catch (std::exception &ex) {
			errorStack.append(tr("Problem in heat exchange definition of pipe '%1':\n%2").arg(QString::fromStdString(supplyPipe.m_displayName))
							  .arg(ex.what()));
		}
		nandradNetwork.m_elements.push_back(supplyPipe);
		componentElementMap[networkPipeComponent.m_id].push_back(supplyPipe.m_id);

		// add outlet pipe element
		inletNode = returnNodeIdMap[edge->m_idNodeOutlet];
		outletNode = returnNodeIdMap[edge->m_idNodeInlet];
		NANDRAD::HydraulicNetworkElement returnPipe(uniqueIdAdd(allElementIds),
													inletNode,
													outletNode,
													VICUS::INVALID_ID,
													VICUS::INVALID_ID,
													networkPipeComponent.m_id,
													edge->m_idPipe,
													edge->length());
		returnPipe.m_displayName = IBK::FormatString("ReturnPipe(ID=%1)").arg(edge->m_id).str();
		returnPipe.m_heatExchange = edge->m_heatExchange;
		try {
			returnPipe.m_heatExchange.checkParameters(p.m_placeholders, p.m_zones, p.m_constructionInstances, false);
		}  catch (std::exception &ex) {
			errorStack.append(tr("Problem in heat exchange definition of pipe '%1':\n%2").arg(QString::fromStdString(returnPipe.m_displayName))
							  .arg(ex.what()));
		}
		nandradNetwork.m_elements.push_back(returnPipe);
		componentElementMap[networkPipeComponent.m_id].push_back(returnPipe.m_id);

		// Create FMI Input Output Definitions
		if (vicusNetwork.m_hasHeatExchangeWithGround){

			// create FMI input definitions
			// --> supply pipe
			NANDRAD::FMIVariableDefinition inputDefSupplyPipe;
			inputDefSupplyPipe.m_objectId = supplyPipe.m_id;
			inputDefSupplyPipe.m_fmiVarName = IBK::FormatString("supplyPipe_id%1.Temperature").arg(supplyPipe.m_id).str(); // custom name
			inputDefSupplyPipe.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
																		 NANDRAD::ModelInputReference::MRT_NETWORKELEMENT );
			inputDefSupplyPipe.m_varName += ".HeatExchangeTemperature";
			inputDefSupplyPipe.m_unit = "C";
			inputDefSupplyPipe.m_fmiValueRef = ++fmiValueRef;
			inputDefSupplyPipe.m_fmiVarDescription = "Pre-described external temperature";
			inputDefSupplyPipe.m_fmiStartValue = vicusNetwork.m_para[VICUS::Network::P_InitialFluidTemperature].value;
			p.m_fmiDescription.m_inputVariables.push_back(inputDefSupplyPipe);

			// --> return pipe
			NANDRAD::FMIVariableDefinition inputDefReturnPipe = inputDefSupplyPipe;
			inputDefReturnPipe.m_objectId = returnPipe.m_id;
			inputDefReturnPipe.m_fmiVarName = IBK::FormatString("returnPipe_id%1.Temperature").arg(returnPipe.m_id).str(); // custom name
			inputDefReturnPipe.m_fmiValueRef = ++fmiValueRef;
			p.m_fmiDescription.m_inputVariables.push_back(inputDefReturnPipe);


			// create FMI output definitions
			// --> supply pipe
			NANDRAD::FMIVariableDefinition outputDefSupplyPipe;
			outputDefSupplyPipe.m_objectId = supplyPipe.m_id;
			outputDefSupplyPipe.m_fmiVarName = IBK::FormatString("supplyPipe_id%1.HeatLoss").arg(supplyPipe.m_id).str(); // custom name
			outputDefSupplyPipe.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
																		  NANDRAD::ModelInputReference::MRT_NETWORKELEMENT );
			outputDefSupplyPipe.m_varName += ".FlowElementHeatLoss";
			outputDefSupplyPipe.m_unit = "W";
			outputDefSupplyPipe.m_fmiValueRef = ++fmiValueRef;
			outputDefSupplyPipe.m_fmiVarDescription = "Heat flux from flow element into environment";
			outputDefSupplyPipe.m_fmiStartValue = 0;
			p.m_fmiDescription.m_outputVariables.push_back(outputDefSupplyPipe);

			// --> return pipe
			NANDRAD::FMIVariableDefinition outputDefReturnPipe = outputDefSupplyPipe;
			outputDefReturnPipe.m_objectId = returnPipe.m_id;
			outputDefReturnPipe.m_fmiVarName = IBK::FormatString("returnPipe_id%1.HeatLoss").arg(returnPipe.m_id).str(); // custom name
			outputDefReturnPipe.m_fmiValueRef = ++fmiValueRef;
			p.m_fmiDescription.m_outputVariables.push_back(outputDefReturnPipe);

			edge->m_idNandradSupplyPipe = supplyPipe.m_id;
			edge->m_idNandradReturnPipe = returnPipe.m_id;

			// assign one soil model for each single pipe
			if (vicusNetwork.m_buriedPipeProperties.m_numberOfSoilModels == 0) {
				++idSoilModel;
				mapSoil2SupplyPipes[idSoilModel].push_back(supplyPipe.m_id);
				mapSoil2ReturnPipes[idSoilModel].push_back(returnPipe.m_id);
			}

		}
	}  // end of iteration over edges
	if(!errorStack.empty())
		return;


	// Alternative to assigning one soil model to each single pipe:
	// Assign a given number of soil models to pipes with similar cumulative temperature change indicator
	std::map<unsigned int, std::vector<NetworkEdge *> > shortestPaths;
	if (vicusNetwork.m_hasHeatExchangeWithGround && vicusNetwork.m_buriedPipeProperties.m_numberOfSoilModels != 0) {

		if (vicusNetwork.m_buriedPipeProperties.m_numberOfSoilModels > vicusNetwork.m_edges.size())
			throw IBK::Exception(IBK::FormatString("Number of soil models (=%1) can not be higher than number of "
												   "edges in the network (=%2)")
								 .arg(vicusNetwork.m_buriedPipeProperties.m_numberOfSoilModels).arg(vicusNetwork.m_edges.size()), FUNC_ID);

		// first calculate paths from source to each building and temperature change indicator for each pipe
		Database<NetworkPipe> dbPipes = Database<NetworkPipe>(1); // we dont care
		dbPipes.setData(m_embeddedDB.m_pipes);
		vicusNetwork.calcTemperatureChangeIndicator(*fluid, dbPipes, shortestPaths);

		// reset edges
		// Mind: const-cast is ok here, since member vars are only needed below
		for (NetworkEdge &e: vicusNetwork.m_edges) {
			e.m_idSoil = VICUS::INVALID_ID;
			e.m_cumulativeTempChangeIndicator = -1;
		}

		// iterate over paths and calculate cumTempChangeindicator and min/max values
		double cumTempChangeindicatorMax = 0;
		double cumTempChangeindicatorMin = std::numeric_limits<double>::max();
		for (auto it = shortestPaths.begin(); it != shortestPaths.end(); ++it){

			double cumTempChangeindicator = 0;
			std::vector<NetworkEdge *> &shortestPath = it->second; // for readability
			for (NetworkEdge * edge: shortestPath){

				cumTempChangeindicator += edge->m_tempChangeIndicator;
				if (edge->m_cumulativeTempChangeIndicator < 0)
					edge->m_cumulativeTempChangeIndicator = cumTempChangeindicator;
				if (cumTempChangeindicator > cumTempChangeindicatorMax)
					cumTempChangeindicatorMax = cumTempChangeindicator;
				if (cumTempChangeindicator < cumTempChangeindicatorMin)
					cumTempChangeindicatorMin = cumTempChangeindicator;
			}
		}

		// intervalMaxValues cumTempChangeindicator for each soil model
		unsigned int numSoilModels = vicusNetwork.m_buriedPipeProperties.m_numberOfSoilModels;
		std::vector<double> intervalMaxValues(numSoilModels);
		for (unsigned int i=0; i<numSoilModels; ++i){
			intervalMaxValues[i] = cumTempChangeindicatorMin  +
					(i+1) * (cumTempChangeindicatorMax - cumTempChangeindicatorMin) / numSoilModels;
		}
		// we manually set the max value to the last interval, this avoids rounding errors which could cause a value slightly below the actual max value
		intervalMaxValues[numSoilModels-1] = cumTempChangeindicatorMax;

		// assign soil models to edges
		for (NetworkEdge &edge: vicusNetwork.m_edges){
			if (edge.m_idSoil != VICUS::INVALID_ID)
				continue;
			if (edge.m_cumulativeTempChangeIndicator <= intervalMaxValues[0]){
				edge.m_idSoil = 1;
			}
			else {
				for (unsigned int i=1; i<numSoilModels; ++i){
					if (edge.m_cumulativeTempChangeIndicator <= intervalMaxValues[i]){
						edge.m_idSoil = i+1;
						break;
					}
				}
			}
			// write map
			if (edge.m_idSoil != VICUS::INVALID_ID){
				mapSoil2SupplyPipes[edge.m_idSoil].push_back(edge.m_idNandradSupplyPipe);
				mapSoil2ReturnPipes[edge.m_idSoil].push_back(edge.m_idNandradReturnPipe);
			}
		}
	}


	// write mapping file
	if (vicusNetwork.m_hasHeatExchangeWithGround) {

		std::string projectName = IBK::Path(nandradProjectPath).filename().withoutExtension().str();
		IBK::Path additionalFilesDir(IBK::Path(nandradProjectPath).withoutExtension() + "_networkInfoFiles");
		if (!additionalFilesDir.exists())
			IBK::Path::makePath(additionalFilesDir);

		std::ofstream out;
		IBK::Path filePath = additionalFilesDir / projectName + ".mapping";
		IBK::open_ofstream(out, filePath);
		out << "soilId" << "\t" << "supplyPipeIds" << "\t" << "returnPipeIds" << std::endl;
		for (auto it=mapSoil2SupplyPipes.begin(); it!=mapSoil2SupplyPipes.end(); ++it ){
			unsigned int soilId = it->first;
			out << soilId << "\t";
			for (unsigned int supplyId: mapSoil2SupplyPipes.at(soilId))
				out << supplyId << ",";
			out << "\t";
			for (unsigned int returnId: mapSoil2ReturnPipes.at(soilId))
				out << returnId << ",";
			out << std::endl;
		}
		out.close();

		vicusNetwork.writeNetworkNodesCSV(additionalFilesDir / projectName + "_NetworkNodes.csv");
		vicusNetwork.writeNetworkEdgesCSV(additionalFilesDir / projectName + "_NetworkEdges.csv");

		// write NANDRAD ids for the path of each building
		filePath = additionalFilesDir / projectName + ".paths";
		IBK::open_ofstream(out, filePath);
		for (auto it = shortestPaths.begin(); it != shortestPaths.end(); ++it){
			out << vicusNetwork.nodeById(it->first)->m_displayName.toStdString() << std::endl;
			std::vector<NetworkEdge *> &shortestPath = it->second; // for readability
			for (const NetworkEdge * edge: shortestPath){
				out << edge->m_idNodeInlet << ',' << edge->m_idNodeOutlet << "\t";
			}
			out << std::endl;
		}
		out.close();

	}


	// Note: at this point the componentElementMap references component ids which may only exist in NANDRAD,
	// therefore we can only create NANDRAD::HydraulicNetworkComponent's from it

	// *** Create Object List for each component

	std::map<unsigned int, NANDRAD::ObjectList> objectListMap;
	for (auto it=componentElementMap.begin(); it!=componentElementMap.end(); ++it){

		const NANDRAD::HydraulicNetworkComponent *comp = VICUS::element(nandradNetwork.m_components, it->first);
		Q_ASSERT(comp != nullptr);

		// create and add object list
		NANDRAD::ObjectList objList;
		objList.m_name = comp->m_displayName + " elements";
		objList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		for (unsigned int elementId: it->second)
			objList.m_filterID.m_ids.insert(elementId);
		p.m_objectLists.push_back(objList);

		objectListMap[comp->m_id] = p.m_objectLists.back();
	}


	// *** Transfer SCHEDULES for each VICUS component

	for (unsigned int compId: componentIdsVicus){

		const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, compId);
		Q_ASSERT(comp != nullptr);

		// get a list with required schedule names, they correspond to the component schedule ids
		std::vector<std::string> requiredScheduleNames= NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(
					NetworkComponent::nandradNetworkComponentModelType(comp->m_modelType));

		if (requiredScheduleNames.size() > 0 && requiredScheduleNames.size() != comp->m_scheduleIds.size()){
			std::string names;
			for (const std::string & name: requiredScheduleNames)
				names += name + ", ";
			errorStack.append(tr("Component with id #%1 requires schedules '%2' but only %1 is given")
							  .arg(comp->m_id).arg(names.c_str()).arg(comp->m_scheduleIds.size()));
		}

		const NANDRAD::ObjectList &objList = objectListMap[comp->m_id];

		// add schedules of component to nandrad
		for (unsigned int i = 0; i<comp->m_scheduleIds.size(); ++i){
			const VICUS::Schedule *sched = VICUS::element(m_embeddedDB.m_schedules, comp->m_scheduleIds[i]);
			if (sched == nullptr){
				errorStack.append(tr("Schedule with id #%1, referenced in network component with id #%2"
									 " does not exist").arg(comp->m_scheduleIds[i]).arg(comp->m_id));
				continue;
			}
			std::string err;
			if (!sched->isValid(err, true, p.m_placeholders)){
				errorStack.append(tr("Schedule with id #%1 has invalid parameters").arg(sched->m_id));
				continue;
			}
			if (sched->m_haveAnnualSchedule){
				// copy schedule and check it
				NANDRAD::LinearSplineParameter annualSched = sched->m_annualSchedule;
				//				try {
				//					annualSched.m_tsvFile.withReplacedPlaceholders(p.m_placeholders);
				//					annualSched.checkAndInitialize(requiredScheduleNames[i], IBK::Unit("m"), IBK::Unit("m"), IBK::Unit("m"),
				//												   std::numeric_limits<double>::lowest(), false,
				//												   std::numeric_limits<double>::max(), false, "", true);
				//					annualSched.m_values.clear();
				//				} catch (std::exception &ex) {
				//					errorStack.append(tr("Error reading in schedule file '%1':\n%2")
				//									  .arg(QString::fromStdString(annualSched.m_tsvFile.str())).arg(ex.what()));
				//				}
				p.m_schedules.m_annualSchedules[objList.m_name].push_back(annualSched);
			}
			else
				addVicusScheduleToNandradProject(*sched, requiredScheduleNames[i], p.m_schedules, objList.m_name);
		}
	}
	if(!errorStack.empty())
		return;



	// ** create summation models

	if (m_outputs.m_flags[VICUS::Outputs::F_CreateDefaultNetworkSummationModels].isEnabled()) {

		// --> collect existing ids
		std::vector<unsigned int> allSumModelIds;
		std::set<unsigned int> networkSumModelIds;
		for (const NANDRAD::HeatLoadSummationModel &sumModel: p.m_models.m_heatLoadSummationModels)
			allSumModelIds.push_back(sumModel.m_id);

		// add summation model for each component
		for (auto it=componentElementMap.begin(); it!=componentElementMap.end(); ++it){
			const NANDRAD::HydraulicNetworkComponent *comp = VICUS::element(nandradNetwork.m_components, it->first);
			// we are looking for typical models of heat sinks / sources
			if (comp->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe ||
					comp->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_SimplePipe ||
					comp->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger ||
					comp->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSourceSide ||
					comp->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableIdealCarnotSupplySide ||
					comp->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_HeatPumpOnOffSourceSide ||
					comp->m_modelType == NANDRAD::HydraulicNetworkComponent::MT_HeatPumpVariableSourceSide) {
				// create summation model
				const NANDRAD::ObjectList &objList = objectListMap[comp->m_id];
				NANDRAD::HeatLoadSummationModel sumModel;
				sumModel.m_objectList = objList.m_name;
				sumModel.m_displayName = "Sum of " + objList.m_name;
				sumModel.m_id = uniqueIdAdd(allSumModelIds);
				networkSumModelIds.insert(sumModel.m_id);
				p.m_models.m_heatLoadSummationModels.push_back(sumModel);
			}
		}

		// now add object list for summation models
		NANDRAD::ObjectList objListSumModels;
		objListSumModels.m_name = "Network Summation Models";
		objListSumModels.m_filterID.m_ids = networkSumModelIds;
		objListSumModels.m_referenceType = NANDRAD::ModelInputReference::MRT_MODEL;
		p.m_objectLists.push_back(objListSumModels);

	}

	// we are DONE !!!
	// finally add to nandrad project
	p.m_hydraulicNetworks.push_back(nandradNetwork);

	// we always need one dummy zone
	if (p.m_zones.empty()) {
		NANDRAD::Zone z;
		z.m_type = NANDRAD::Zone::ZT_Active;
		NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Volume, 1);
		NANDRAD::KeywordList::setParameter(z.m_para, "Zone::para_t", NANDRAD::Zone::P_Area, 1);
		p.m_zones.push_back(z);
	}

}

} // namespace VICUS

