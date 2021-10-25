#include "VICUS_Project.h"

#include <NANDRAD_Project.h>

#include <IBK_algorithm.h>
#include <IBK_physics.h>

#include "VICUS_utilities.h"

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
		m_scheduleDB(0)
	{
		m_scheduleDB.setData(pro->m_embeddedDB.m_schedules);
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

	std::map<unsigned int, unsigned int>	m_vicusToNandradIds;
};


class InternalLoadsModelGenerator : public ModelGeneratorBase {
public:
	InternalLoadsModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, std::vector<unsigned int> &usedModelIds, QStringList & errorStack);

	// All definition lists below have the same size and share the same index
	// i.e. m_internalLoadObjects[3] references m_objLists[3] and
	//      m_schedules[3] corresponds also to m_objLists[3]

	std::vector<NANDRAD::InternalLoadsModel>		m_internalLoadObjects;
	std::vector<NANDRAD::ObjectList>				m_objLists;
	std::vector<std::string>						m_objListNames;

	// Object list name = schedule group name is not stored, since it matches the respective object list
	// name in m_objLists
	std::vector< std::vector<NANDRAD::Schedule> >	m_schedules;
};

class VentilationModelGenerator : public ModelGeneratorBase {
public:
	VentilationModelGenerator(const VICUS::Project * pro) :
		ModelGeneratorBase(pro)
	{}

	void generate(const VICUS::Room * r, std::vector<unsigned int> &usedModelIds, QStringList & errorStack);

	// All definition lists below have the same size and share the same index

	std::vector<NANDRAD::NaturalVentilationModel>	m_natVentObjects;
	std::vector<NANDRAD::ObjectList>				m_objLists;
	std::vector<std::string>						m_objListNames;

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
	std::vector< std::vector<NANDRAD::Schedule> >	m_schedules;
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
					   unsigned int constructionInstanceId, double area):
		m_controlledZoneId(controlledZoneId),
		m_nandradConstructionInstanceId(constructionInstanceId),
		m_heatingSystemId(surfaceHeatingModelId),
		m_area(area)
	{}

	unsigned int					m_controlledZoneId;
	unsigned int					m_nandradConstructionInstanceId;
	unsigned int					m_heatingSystemId;
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
	double upperSupplyCoolLimit = std::max<double>(supplySetpoints[2], supplySetpoints[3]);

	double lowerOutHeatLimit = std::min<double>(outdoorSetpoints[0], outdoorSetpoints[1]);
	double upperOutHeatLimit = std::max<double>(outdoorSetpoints[0], outdoorSetpoints[1]);
	double lowerOutCoolLimit = std::min<double>(outdoorSetpoints[2], outdoorSetpoints[3]);
	double upperOutCoolLimit = std::max<double>(outdoorSetpoints[2], outdoorSetpoints[3]);

	if(upperOutHeatLimit > lowerOutCoolLimit)
		return false;

	supplyTemperature.clear();

	double deltaX = outdoorSetpoints[0] - outdoorSetpoints[1];
	double mHeat = 0;
	if(deltaX != 0)
		mHeat = (supplySetpoints[0] - supplySetpoints[1]) / deltaX;
	double nHeat =  supplySetpoints[0] - mHeat * outdoorSetpoints[0];

	deltaX = outdoorSetpoints[2] - outdoorSetpoints[3];
	double mCool = 0;
	if(deltaX != 0)
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


class ConstructionInstanceModelGenerator : public ModelGeneratorBase {
public:

	ConstructionInstanceModelGenerator(const VICUS::Project *pro):
		ModelGeneratorBase(pro)
	{}

	void generate(const std::vector<ComponentInstance> &componentInstances, QStringList &errorStack,
				  std::map<unsigned int, unsigned int> &vicusToNandradIds, std::set<unsigned int> &idSet);

	void generateMaterials();
	void generateConstructions(QStringList &errorStack);

	struct ActiveLayer{

		//ActiveLayer (unsigned int vicusConstructionId, unsigned int )

		unsigned int								m_vicusConstructionId;	//this is the id of the construction (VICUS)
		std::vector<unsigned int>					m_nandradConstructionTypeId;	//new id is set to get a unique construction type id WITH active layer for NANDRAD
		//all types of this construction with active layers; -1 -> no active layer otherwise this is the layer index
		std::vector<int>							m_activeLayers;
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

private:

	NANDRAD::Interface generateInterface(const VICUS::ComponentInstance & ci, unsigned int bcID,
												  unsigned int interfaceID,  QStringList &errorStack,
												  bool takeASide = true) const;

	void exportSubSurfaces(QStringList & errorStack, const std::vector<VICUS::SubSurface> &subSurfs,
					  const VICUS::ComponentInstance & ci, NANDRAD::ConstructionInstance &cinst, std::set<unsigned int> &idSet) const;

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


void Project::generateBuildingProjectDataNeu(NANDRAD::Project & p, QStringList & errorStack) const{

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
		if (!sched.isValid())
			errorStack.append(tr("Schedule #%1 '%2'.").arg(sched.m_id).arg(MultiLangString2QString(sched.m_displayName)));
	}
	if (!errorStack.isEmpty())	return;


	// *** Zones ***

	std::vector<unsigned int> usedModelIds;
	std::set<unsigned int> idSet;
	std::vector<const VICUS::Room *> zones;
	generateNandradZones(zones, idSet, p, errorStack, vicusToNandradIds);

	if (!errorStack.isEmpty())	return;

	// *** Create Construction Instances, Constructions (opak & tranparent) and materials ***

	ConstructionInstanceModelGenerator constrInstaModelGenerator(this);
	constrInstaModelGenerator.generate(m_componentInstances, errorStack, vicusToNandradIds, idSet);
	constrInstaModelGenerator.generateMaterials();
	constrInstaModelGenerator.generateConstructions(errorStack);

	if (!errorStack.isEmpty())	return;


	// *** Ideal Surface Heating Systems ***

	//TODO Dirk add annual schedules to nandrad
	IdealSurfaceHeatingCoolingModelGenerator idealSurfaceHeatCoolGenerator(this);
	idealSurfaceHeatCoolGenerator.generate(constrInstaModelGenerator.m_surfaceHeatingData, usedModelIds, errorStack);

	// *** Models based on zone templates ***

	// process all zones
	InternalLoadsModelGenerator internalLoads(this);
	VentilationModelGenerator ventilation(this);
	IdealHeatingCoolingModelGenerator idealHeatCool(this);
	ThermostatModelGenerator thermostats(this);
	for (const VICUS::Room * r : zones) {
		internalLoads.generate(r, usedModelIds, errorStack);
		ventilation.generate(r, usedModelIds, errorStack);
		idealHeatCool.generate(r, usedModelIds, errorStack);
		thermostats.generate(r, usedModelIds, errorStack);
	}
	if (!errorStack.isEmpty())	return;

	// transfer data to project

	// *** InternalLoads ***
	p.m_models.m_internalLoadsModels = internalLoads.m_internalLoadObjects;
	p.m_objectLists.insert(p.m_objectLists.end(), internalLoads.m_objLists.begin(), internalLoads.m_objLists.end());
	for (unsigned int i=0; i<internalLoads.m_schedules.size(); ++i)
		p.m_schedules.m_scheduleGroups[internalLoads.m_objLists[i].m_name] = internalLoads.m_schedules[i];

	// *** Ventilation ***
	p.m_models.m_naturalVentilationModels = ventilation.m_natVentObjects;
	p.m_objectLists.insert(p.m_objectLists.end(), ventilation.m_objLists.begin(), ventilation.m_objLists.end());
	for (unsigned int i=0; i<ventilation.m_schedules.size(); ++i)
		p.m_schedules.m_scheduleGroups[ventilation.m_objLists[i].m_name] = ventilation.m_schedules[i];

	// *** Thermostat ***
	p.m_models.m_thermostats = thermostats.m_thermostats;
	p.m_objectLists.insert(p.m_objectLists.end(), thermostats.m_objLists.begin(), thermostats.m_objLists.end());
	for (unsigned int i=0; i<thermostats.m_schedules.size(); ++i)
		p.m_schedules.m_scheduleGroups[thermostats.m_objLists[i].m_name] = thermostats.m_schedules[i];

	// *** Ideal heating cooling ***
	p.m_models.m_idealHeatingCoolingModels = idealHeatCool.m_idealHeatingCoolings;
	p.m_objectLists.insert(p.m_objectLists.end(), idealHeatCool.m_objLists.begin(), idealHeatCool.m_objLists.end());

	// *** Construction Instances, Constructions (opak & tranparent) and materials ***
	p.m_constructionInstances = constrInstaModelGenerator.m_constructionInstances;
	p.m_constructionTypes = constrInstaModelGenerator.m_constructions;
	p.m_materials = constrInstaModelGenerator.m_materials;
	p.m_windowGlazingSystems = constrInstaModelGenerator.m_windowGlazingSystems;

	// *** Ideal surface heating cooling ... ***
	p.m_models.m_idealSurfaceHeatingCoolingModels = idealSurfaceHeatCoolGenerator.m_idealSurfaceHeatingCoolings;
	p.m_models.m_idealPipeRegisterModels = idealSurfaceHeatCoolGenerator.m_idealPipeRegister;
	p.m_objectLists.insert(p.m_objectLists.end(), idealSurfaceHeatCoolGenerator.m_objListsSurface.begin(), idealSurfaceHeatCoolGenerator.m_objListsSurface.end());
	p.m_objectLists.insert(p.m_objectLists.end(), idealSurfaceHeatCoolGenerator.m_objListsPipe.begin(), idealSurfaceHeatCoolGenerator.m_objListsPipe.end());
	p.m_objectLists.insert(p.m_objectLists.end(), idealSurfaceHeatCoolGenerator.m_objListLinearSpline.begin(), idealSurfaceHeatCoolGenerator.m_objListLinearSpline.end());
	p.m_schedules.m_annualSchedules = idealSurfaceHeatCoolGenerator.m_constructionIdToNandradSplines;
}


void Project::generateNandradZones(std::vector<const VICUS::Room *> & zones,
								   std::set<unsigned int> & idSet,
								   NANDRAD::Project & p, QStringList & errorStack,
								   std::map<unsigned int, unsigned int> &vicusToNandradIds)const
{
	// collect a list of zone references for further processing

	for (const VICUS::Building & b : m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room & r : bl.m_rooms)
				zones.push_back(&r);

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
		vicusToNandradIds[r->m_id] = r->m_id;
		z.m_displayName = r->m_displayName.toStdString();

		// Note: in the code below we expect the parameter's base units to be the same as the default unit for the
		//       populated parameters

		bool isZoneOk = true;
		try {
			r->m_para[VICUS::Room::P_Area].checkedValue("Area", "m2", "m2", 0, false, std::numeric_limits<double>::max(), true, nullptr);
		} catch (...) {
			errorStack.append(tr("Zone #%1 '%2' does not have a valid area defined.").arg(r->m_id).arg(r->m_displayName));
			isZoneOk = false;
		}
		try {
			r->m_para[VICUS::Room::P_Volume].checkedValue("Volume", "m3", "m3", 0, true, std::numeric_limits<double>::max(), true, nullptr);
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
								 NANDRAD::Project &p, const std::string &objListName)const {

	// *** predefinitions ***

	//find a schedule in NANDRAD project with the objListName
	if(p.m_schedules.m_scheduleGroups.find(objListName) != p.m_schedules.m_scheduleGroups.end()){
		//get schedule vector for this object list name
		std::vector<NANDRAD::Schedule> &scheds = p.m_schedules.m_scheduleGroups[objListName];

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
		p.m_schedules.m_scheduleGroups[objListName] = scheds;
	}
}



void InternalLoadsModelGenerator::generate(const Room * r, std::vector<unsigned int> &usedModelIds, QStringList & errorStack) {
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
	std::string equipmentSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_EquipmentHeatLoadPerArea) + "Schedule [W/m2]";
	std::string lightingSchedName = (std::string)NANDRAD::KeywordList::Keyword("InternalLoadsModel::para_t", NANDRAD::InternalLoadsModel::P_LightingHeatLoadPerArea) + "Schedule [W/m2]";

	// schedule generation:
	//
	// 1. create basic schedule (name?)
	std::vector<NANDRAD::Schedule> scheds; // contains all schedules for this internal loads parametrization

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

	// we only need an all day schedule if we miss at least on of the schedules
	if (!dc.m_values.m_values.empty())
		scheds.push_back(allDays);

	// initialize load fractions with 0
	double personRadiationFraction = 0;
	double equipmentRadiationFraction = 0;
	double lightingRadiationFraction = 0;

	// for each given VICUS schedule, generate a set of schedules and insert them in vector scheds (unless already existing, in which case these are extended)

	// *** Person loads (category = IC_Person)***

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
			combinedSchedule.insertIntoNandradSchedulegroup(personSchedName, scheds);
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
			combinedSchedule.insertIntoNandradSchedulegroup(equipmentSchedName, scheds);
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
			combinedSchedule.insertIntoNandradSchedulegroup(lightingSchedName, scheds);
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
			NANDRAD::Schedules::equalSchedules(m_schedules[i], scheds) ) {
			// insert our zone ID in object list
			m_objLists[i].m_filterID.m_ids.insert(r->m_id);
			foundModel = true;
			break;
		}
	}
	if(!foundModel) {
		// append definitions and create new object list
		NANDRAD::ObjectList ol;
		ol.m_name = IBK::pick_name("InternalLoads-" + zt->m_displayName.string(), m_objListNames.begin(), m_objListNames.end());
		ol.m_referenceType = NANDRAD::ModelInputReference::MRT_ZONE;
		ol.m_filterID.m_ids.insert(r->m_id);

		// set object list in new definition
		internalLoadsModel.m_zoneObjectList = ol.m_name;

		// add all definitions
		m_internalLoadObjects.push_back(internalLoadsModel);
		m_schedules.push_back(scheds);
		m_objLists.push_back(ol);
		m_objListNames.push_back(ol.m_name);
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
//	try {
//		if (infiltration != nullptr && !infiltration->isValid())
//			throw IBK::Exception( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
//								  .arg(zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_Infiltration])
//								  .arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)).toStdString(), FUNC_ID);
//	}
//	catch (IBK::Exception & ex) {
//		errorStack.append( QString::fromStdString(ex.what()) );
//		return;
//	}

//	try {
//		if (ventilation != nullptr && !ventilation->isValid(m_scheduleDB))
//			throw IBK::Exception( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
//								  .arg(zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_VentilationNatural])
//								  .arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)).toStdString(), FUNC_ID);
//	}
//	catch (IBK::Exception & ex) {
//		errorStack.append( QString::fromStdString(ex.what()) );
//		return;
//	}


//	try {
//		if (ctrlVentilation != nullptr && !ctrlVentilation->isValid())
//			throw IBK::Exception( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
//								  .arg(zoneTemplate->m_idReferences[ZoneTemplate::SubTemplateType::ST_Infiltration])
//								  .arg(zoneTemplate->m_id).arg(MultiLangString2QString(zoneTemplate->m_displayName)).toStdString(), FUNC_ID);
//	}
//	catch (IBK::Exception & ex) {
//		errorStack.append( QString::fromStdString(ex.what()) );
//		return;
//	}

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

	if(ctrlVentilation != nullptr  && !ctrlVentilation->isValid())
		errorStack.append( qApp->tr("Invalid sub template ID #%1 referenced from zone template #%2 '%3'.")
						   .arg(ctrlVentilation->m_id)
						   .arg(zoneTemplate->m_id)
						   .arg(MultiLangString2QString(zoneTemplate->m_displayName)));

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
	VentiType ventiType;
	if(isInf && !isVenti)				ventiType = V_Infiltration;
	else if(!isInf && isVenti)			ventiType = V_Ventilation;
	else if(isInf && isVenti)			ventiType = V_InfAndVenti;

	NANDRAD::NaturalVentilationModel natVentMod;
	natVentMod.m_id = VICUS::uniqueIdWithPredef(usedModelIds, 1);
	natVentMod.m_displayName = zoneTemplate->m_displayName.string();
	//TODO id and display name

	//if we have a controlling than add control parameter
	if(ctrlVentilation != nullptr){
		//set all control values
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MaximumEnviromentAirTemperatureACRLimit,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::ST_TemperatureOutsideMax].get_value("C"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MaximumRoomAirTemperatureACRLimit,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::ST_TemperatureAirMax].get_value("C"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MinimumEnviromentAirTemperatureACRLimit,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::ST_TemperatureOutsideMin].get_value("C"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_MinimumRoomAirTemperatureACRLimit,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::ST_TemperatureAirMin].get_value("C"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_DeltaTemperatureACRLimit,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::ST_TemperatureDifference].get_value("K"));
		NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
										   NANDRAD::NaturalVentilationModel::P_WindSpeedACRLimit,
										   ctrlVentilation->m_para[ZoneControlNaturalVentilation::ST_WindSpeedMax].get_value("m/s"));
	}

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
		case VentiType::V_Ventilation:{
			if(ctrlVentilation == nullptr)
				natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Scheduled;
			else{
				natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR;
				NANDRAD::KeywordList::setParameter(natVentMod.m_para, "NaturalVentilationModel::para_t",
												   NANDRAD::NaturalVentilationModel::P_VentilationRate,
												   0);
			}
		}
		break;
		case VentiType::V_InfAndVenti:{
			if(ctrlVentilation == nullptr)
				//create one schedule with summed value of infiltration + ventilation
				natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_Scheduled;
			else{
				natVentMod.m_modelType = NANDRAD::NaturalVentilationModel::MT_ScheduledWithBaseACR;
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
		}
		break;
	}

	// *** schedules ***
	// schedule generation:
	//
	// 1. create basic schedule (name?)
	std::vector<NANDRAD::Schedule> scheds;
	VICUS::Schedule combinedSchedule;
	if(ventiType != V_Infiltration){
		const Schedule * ventSched = m_scheduleDB[ventilation->m_idSchedule];
		combinedSchedule = ventSched->multiply(ventilation->m_para[VentilationNatural::P_AirChangeRate].value);
		if(!(ventiType == V_Ventilation || (ventiType == V_InfAndVenti && ctrlVentilation != nullptr))){
			double infVal = infiltration->m_para[Infiltration::P_AirChangeRate].value;
			if(infiltration->m_airChangeType == Infiltration::AC_n50)
				infVal *= infiltration->m_para[Infiltration::P_ShieldingCoefficient].value;
			combinedSchedule.add(infVal);
		}
		std::string schedName =  (std::string)NANDRAD::KeywordList::Keyword("NaturalVentilationModel::para_t",
											NANDRAD::NaturalVentilationModel::P_VentilationRate) + "Schedule [1/h]";
		combinedSchedule.insertIntoNandradSchedulegroup(schedName, scheds);
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
		if (m_natVentObjects[i].equal(natVentMod) &&
			NANDRAD::Schedules::equalSchedules(m_schedules[i], scheds) ) {
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
		m_schedules.push_back(scheds);
		m_objLists.push_back(ol);
		m_objListNames.push_back(ol.m_name);
	}
}

void ConstructionInstanceModelGenerator::exportSubSurfaces(QStringList & errorStack, const std::vector<VICUS::SubSurface> &subSurfs,
								const VICUS::ComponentInstance & ci, NANDRAD::ConstructionInstance &cinst, std::set<unsigned int> &idSet) const{

	double embArea = 0;
	// get area of surface
	double areaA = ci.m_sideASurface->geometry().area();

	for(const SubSurface &ss : subSurfs){
		NANDRAD::EmbeddedObject emb;
		emb.m_id = VICUS::uniqueId(idSet);
		idSet.insert(emb.m_id);	//add new id to set
		//get surface are of emb. obj.
		NANDRAD::KeywordList::setParameter(emb.m_para, "EmbeddedObject::para_t",
										   NANDRAD::EmbeddedObject::P_Area, ss.m_polygon2D.area());
		embArea += ss.m_polygon2D.area();
		if(embArea > areaA){
			errorStack << qApp->tr("Area of sub surfaces is bigger than area of parent surface #%1, '%2'.").arg(ci.m_sideASurface->m_id)
						  .arg(ci.m_sideASurface->m_displayName);
			continue;
		}
		emb.m_displayName = ss.m_displayName.toStdString();

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
					for(const VICUS::Window &winV : m_project->m_embeddedDB.m_windows){
						if(winV.m_id == ssc.m_idWindow){
							if(winV.m_idGlazingSystem == VICUS::INVALID_ID){
								errorStack << qApp->tr("The window with #%1 and name '%2' has no valid glazing system. Sub surface #%3 is not exported.")
											  .arg(winV.m_id).arg(QString::fromStdString(winV.m_displayName.string())).arg(ss.m_id);
								break;
							}
							//save id for glazing system later
							emb.m_window.m_glazingSystemId = winV.m_idGlazingSystem;
							foundWindow = true;
							break;
						}
					}
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
		//add emb. obj. to nandrad project
		cinst.m_embeddedObjects.push_back(emb);
		//TODO Dirk Frame einbauen sobald verfügbar
		//TODO Dirk Divider einbauen sobald verfügbar
	}
}

NANDRAD::Interface ConstructionInstanceModelGenerator::generateInterface(const VICUS::ComponentInstance & ci, unsigned int bcID,
											  unsigned int interfaceID,
											  QStringList &errorStack, bool takeASide) const
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
	}
	if (!bc->isValid())
		errorStack.append(qApp->tr("Boundary condition #%1 has invalid/incomplete parameters.").arg(bc->m_id));

	// do we have a surface to a zone?
	if (s != nullptr) {
		// get the zone that this interface is connected to
		const VICUS::Object * obj = s->m_parent;
		const VICUS::Room * room = dynamic_cast<const VICUS::Room *>(obj);
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
		// no surface == true -> must be an interface to the outside

		// generate a new interface to the zone, which always only includes heat conduction
		NANDRAD::Interface iface;
		iface.m_id = interfaceID;
		iface.m_zoneId = 0; // outside zone
		iface.m_heatConduction.m_modelType = (NANDRAD::InterfaceHeatConduction::modelType_t)bc->m_heatConduction.m_modelType;
		iface.m_heatConduction.m_para[NANDRAD::InterfaceHeatConduction::P_HeatTransferCoefficient] = bc->m_heatConduction.m_para[VICUS::InterfaceHeatConduction::P_HeatTransferCoefficient];
		iface.m_solarAbsorption = bc->m_solarAbsorption;
		iface.m_longWaveEmission = bc->m_longWaveEmission;
		return iface;
	}
}

void ConstructionInstanceModelGenerator::generate(const std::vector<ComponentInstance> &componentInstances,
												  QStringList &errorStack, std::map<unsigned int, unsigned int> &vicusToNandradIds,
												  std::set<unsigned int> &idSet)
{
	FUNCID(ConstructionInstanceModelGenerator::generate);
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

		// set construction instance parameters, area, orientation etc.
		const double SAME_DISTANCE_PARAMETER_ABSTOL = 0.25;//1e-4;
		const double REL_TOL_AREAS = 0.05;
		double minArea = 0.1; //1e-4;
		double area = 0;

		bool bothSidesHaveSurfaces = false;
		// we have either one or two surfaces associated
		if (compInstaVicus.m_sideASurface != nullptr) {
			// get area of surface A
			area = compInstaVicus.m_sideASurface->geometry().area();
			// do we have surfaces at both sides?
			if (compInstaVicus.m_sideBSurface != nullptr) {
				// have both
				bothSidesHaveSurfaces = true;
				double areaB = compInstaVicus.m_sideBSurface->geometry().area();
				// check if both areas are approximately the same

#define ABS_TEST;
#if defined (ABS_TEST)
				if (std::fabs(area - areaB) > SAME_DISTANCE_PARAMETER_ABSTOL) {
#else
				// relative error check
				// area is set as reference
				if (std::fabs(areaB-area)/area > REL_TOL_AREAS) {
#endif
					errorStack.append(qApp->tr("Component/construction #%1 references surfaces #%2 '%6' and #%3 '%7', with mismatching "
						   "areas %4 and %5 m2.")
									  .arg(compInstaVicus.m_id).arg(compInstaVicus.m_idSideASurface)
									  .arg(compInstaVicus.m_idSideBSurface)
									  .arg(area).arg(areaB)
									  .arg(compInstaVicus.m_sideASurface->m_displayName)
									  .arg(compInstaVicus.m_sideBSurface->m_displayName));
				}

				/// TODO Dirk :	do we need to also store a displayname for each component instance/construction instance?
				///				We could also name internal walls automatically using zone names, such as
				///				"Wall between 'Bath' and 'Kitchen'".
				constrInstNandrad.m_displayName = qApp->tr("Internal wall between surfaces '#%1' and '#%2'")
						.arg(compInstaVicus.m_sideASurface->m_displayName).arg(compInstaVicus.m_sideBSurface->m_displayName).toStdString();
			}
			else {

				// we only have side A, take orientation and inclination from side A
				const VICUS::Surface * s = compInstaVicus.m_sideASurface;

				// set parameters
				NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Inclination, s->geometry().inclination());
				NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
												   NANDRAD::ConstructionInstance::P_Orientation, s->geometry().orientation());

				constrInstNandrad.m_displayName = compInstaVicus.m_sideASurface->m_displayName.toStdString();
			}

			if(area<minArea){
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
					exportSubSurfaces(errorStack, subSurfs, compInstaVicus, constrInstNandrad, idSet);
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

			const VICUS::Surface * s = compInstaVicus.m_sideBSurface;

			// set parameters
			NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Inclination, s->geometry().inclination());
			NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Orientation, s->geometry().orientation());

			// set area parameter
			area = compInstaVicus.m_sideBSurface->geometry().area();

			if(area<minArea){
				warnings << qApp->tr("Area of surface '%2' (#%1) is too small. Surface is not exported.")
							.arg(compInstaVicus.m_sideBSurface->m_id)
							.arg(compInstaVicus.m_sideBSurface->m_displayName);
				continue;
			}
			NANDRAD::KeywordList::setParameter(constrInstNandrad.m_para, "ConstructionInstance::para_t",
											   NANDRAD::ConstructionInstance::P_Area, area);

			constrInstNandrad.m_displayName = compInstaVicus.m_sideBSurface->m_displayName.toStdString();
			// sub surface
			const std::vector<SubSurface> & subSurfs = compInstaVicus.m_sideBSurface->subSurfaces();
			if(subSurfs.size()>0){
				//we have sub surfaces
				exportSubSurfaces(errorStack, subSurfs, compInstaVicus, constrInstNandrad, idSet);
			}
		}

		// now generate interfaces
		constrInstNandrad.m_interfaceA = generateInterface(compInstaVicus, comp->m_idSideABoundaryCondition, 1, errorStack, true);
		constrInstNandrad.m_interfaceB = generateInterface(compInstaVicus, comp->m_idSideBBoundaryCondition, 2, errorStack, false);

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
															  area));
			activeLayerIdx = (int)comp->m_activeLayerIndex;
//			bool isAdded = false;
//			for(unsigned int i=0; m_surfaceHeatingData.size(); ++i){
//				if(m_surfaceHeatingData[i].m_controlledZoneId == compInstaVicus.m_surfaceHeatingControlZoneID){
//					isAdded = true;
//					m_surfaceHeatingData[i].m_heatingModelIdToNandradConstructionInstanceIds[compInstaVicus.m_surfaceHeatingID].push_back(constrInstNandrad.m_id);
//					break;
//				}
//			}
//			if(isAdded){
//				dsh.m_heatingModelIdToNandradConstructionInstanceIds[compInstaVicus.m_surfaceHeatingID].push_back(constrInstNandrad.m_id);
//				m_surfaceHeatingData.push_back(dsh);
//			}
//			unsigned int zoneId = INVALID_ID;

//			int posDataSH = -1;
//			for(unsigned int i=0; i<m_surfaceSystems.size(); ++i){
//				if(m_surfaceSystems[i].m_zoneId == compInstaVicus.m_surfaceHeatingControlZoneID){
//					//found zone id
//					posDataSH = (int)i;
//					zoneId = compInstaVicus.m_surfaceHeatingControlZoneID;
//					break;
//				}
//			}

//			if(zoneId != INVALID_ID){
//				//add new element
//				if(posDataSH == -1){
//					m_surfaceSystems.push_back(DataSurfaceHeating());
//					posDataSH = m_surfaceSystems.size()-1;
//					m_surfaceSystems.back().m_zoneId = zoneId;
//				}
//				DataSurfaceHeating &dsh = m_surfaceSystems[posDataSH];
//				dsh.m_idSurfaceSystemToIds[ci.m_surfaceHeatingID].m_contructionInstanceIds.push_back(cinst.m_id);
//				dsh.m_idSurfaceSystemToIds[ci.m_surfaceHeatingID].m_areaToIds[area*10000].push_back(cinst.m_id);
//			}
//			else{
//				//todo errorstack füllen
//			}
		}


		// store reference to construction type (i.e. to be generated from component)
		constrInstNandrad.m_constructionTypeId = getNandradConstructionTypeId(comp->m_idConstruction, activeLayerIdx);
		// add to list of construction instances
		m_constructionInstances.push_back(constrInstNandrad);

	}

	for(const QString &str : warnings)
		IBK::IBK_Message(IBK::FormatString("%1").arg(str.toStdString()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);


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
	VICUS::Schedule s;

	const Schedule * heatSched = m_scheduleDB[thermostat->m_idHeatingSetpointSchedule];
	const Schedule * coolSched = m_scheduleDB[thermostat->m_idCoolingSetpointSchedule];

	if(heatSched != nullptr)
		heatSched->insertIntoNandradSchedulegroup( "HeatingSetpointSchedule [C]" , scheds);
	else{
		s.createConstSchedule(-100);
		s.insertIntoNandradSchedulegroup( "HeatingSetpointSchedule [C]" , scheds);
	}

	if(coolSched != nullptr)
		coolSched->insertIntoNandradSchedulegroup( "CoolingSetpointSchedule [C]" , scheds);
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
			NANDRAD::Schedules::equalSchedules(m_schedules[i], scheds) ) {
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
		m_objLists.push_back(ol);
		m_objListNames.push_back(ol.m_name);

	}
}

void IdealSurfaceHeatingCoolingModelGenerator::generate(const std::vector<DataSurfaceHeating> &dataSurfaceHeating, std::vector<unsigned int> &usedModelIds, QStringList &errorStack) {

	// Create a outdoor air temperature data line for calculate the supply fluid temperature later
	IBK::LinearSpline outdoorTemp;
	//first vector -> timepoints; second temperature in C
	std::vector<double> timepoints{0,100,200,300,400,500,600,700,800,900,1000,
												  1100,1200,1300,1400,1500,1600,1700,1800,1900,2000,
												  2900,8760};
	std::vector<double> datapoints{-20,-15,-14,-10,-5,0,5,10,15,20,21,22,23,24,25,26,27,28,29,30,31,35,40};
	outdoorTemp.setValues(timepoints, datapoints);
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
			case SurfaceHeating::T_IdealPipeRegister:{

				NANDRAD::IdealPipeRegisterModel nandradSys;
				nandradSys.m_thermostatZoneId = dsh.m_controlledZoneId;
				//always schedule in gui
				nandradSys.m_modelType = NANDRAD::IdealPipeRegisterModel::MT_Scheduled;
				//standard fluid model
				NANDRAD::HydraulicFluid fluid;
				fluid.defaultFluidWater();
				nandradSys.m_fluid = fluid;

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
						fluid.m_para[VICUS::NetworkFluid::P_Density].value;

				double uValue = pipe->UValue();

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

				std::vector<double> supplyTemperatureVec;
				calculateSupplyTemperature(surfSys->m_heatingCoolingCurvePoints.m_values.at("Tsupply"),
												 surfSys->m_heatingCoolingCurvePoints.m_values.at("Tout"),
												 outdoorTemp.y(), supplyTemperatureVec);

				//TODO Dirk->Andreas wie geht das anzuhängen ans NANDRAD projekt?
				//kann sein das ich das erst speichern muss ... gucken ...
				NANDRAD::LinearSplineParameter tSupply("SupplyTemperatureSchedule",NANDRAD::LinearSplineParameter::I_LINEAR,
													   outdoorTemp.x(), supplyTemperatureVec,
													   IBK::Unit("h"),IBK::Unit("C"));

				nandradSys.m_displayName = "Underfloor heating";
				nandradSys.m_id = dsh.m_nandradConstructionInstanceId;
				//m_linearSplinePara.push_back(tSupply);

				NANDRAD::LinearSplineParameter massFlux("MaxMassFluxSchedule", NANDRAD::LinearSplineParameter::I_LINEAR,
														std::vector<double>{0,8760}, std::vector<double>{maxMassFlux, maxMassFlux},
														IBK::Unit("h"),IBK::Unit("kg/s"));
				//m_linearSplinePara.push_back(massFlux);

				idealPipeRegister.push_back(nandradSys);
				constructionIdToNandradSplines[dsh.m_nandradConstructionInstanceId] = std::vector<NANDRAD::LinearSplineParameter>{tSupply,massFlux};
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

	if(!idealHeatingCooling->m_para[NANDRAD::IdealHeatingCoolingModel::P_MaxHeatingPowerPerArea].empty())
		NANDRAD::KeywordList::setParameter(idealHeatCool.m_para, "IdealHeatingCoolingModel::para_t",
									NANDRAD::IdealHeatingCoolingModel::P_MaxHeatingPowerPerArea,
									idealHeatingCooling->m_para[ZoneIdealHeatingCooling::P_HeatingLimit].value);
	else
		NANDRAD::KeywordList::setParameter(idealHeatCool.m_para, "IdealHeatingCoolingModel::para_t",
									NANDRAD::IdealHeatingCoolingModel::P_MaxHeatingPowerPerArea, 0);
	if(!idealHeatingCooling->m_para[NANDRAD::IdealHeatingCoolingModel::P_MaxCoolingPowerPerArea].empty())
		NANDRAD::KeywordList::setParameter(idealHeatCool.m_para, "IdealHeatingCoolingModel::para_t",
									   NANDRAD::IdealHeatingCoolingModel::P_MaxCoolingPowerPerArea,
									   idealHeatingCooling->m_para[ZoneIdealHeatingCooling::P_CoolingLimit].value);
	else
		NANDRAD::KeywordList::setParameter(idealHeatCool.m_para, "IdealHeatingCoolingModel::para_t",
									NANDRAD::IdealHeatingCoolingModel::P_MaxCoolingPowerPerArea, 0);



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

void Project::generateNetworkProjectData(NANDRAD::Project & p, QStringList &errorStack, const std::string & nandradProjectPath) const {
	FUNCID(Project::generateNetworkProjectData);

	// get selected Vicus Network
	unsigned int networkId = VICUS::INVALID_ID;
	for (const VICUS::Network &net: m_geometricNetworks) {
		if (net.m_selectedForSimulation) {
			networkId = net.m_id;
			// TODO Hauke, multiple (connected) networks?
			break;
		}
	}

	// if there is no network selected return - this is not an error, but the usual case for simple building energy
	// simulations
	if (VICUS::element(m_geometricNetworks, networkId) == nullptr)
		return;

	const VICUS::Network & vicusNetwork = *VICUS::element(m_geometricNetworks, networkId);

	// buildings can only have one connected edge
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Building && node.m_edges.size()>1 ) {
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
	IBK_ASSERT(p.m_hydraulicNetworks.empty());
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
	Database<SubNetwork> dbSubNetworks = Database<SubNetwork>(1);
	dbSubNetworks.setData(m_embeddedDB.m_subNetworks);
	Database<NetworkComponent> dbNetworkComps = Database<NetworkComponent>(1); // we dont care
	dbNetworkComps.setData(m_embeddedDB.m_networkComponents);
	Database<NetworkController> dbNetworkCtrl = Database<NetworkController>(1); // we dont care
	dbNetworkCtrl.setData(m_embeddedDB.m_networkControllers);
	Database<Schedule> dbSchedules = Database<Schedule>(1); // we dont care
	dbSchedules.setData(m_embeddedDB.m_schedules);

	// *** Transfer COMPONENTS from Vicus to Nandrad

	// --> collect sub networks
	std::set<unsigned int> subNetworkIds, componentIds, controllerIds;
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
			componentIds.insert(el.m_componentId);
			if (el.m_controlElementId != NANDRAD::INVALID_ID)
				controllerIds.insert(el.m_controlElementId);
		}
	}
	if (!errorStack.empty())
		return;

	// --> transfer components
	for (unsigned int compId: componentIds){
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
	for (unsigned int compId: componentIds){
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
										   NANDRAD::HydraulicNetworkPipeProperties::P_UValuePipeWall, pipe->UValue());
		nandradNetwork.m_pipeProperties.push_back(pipeProp);
	}
	if (!errorStack.empty())
		return;


	// *** Transfer ELEMENTS from Vicus to Nandrad

	// estimated number of elements
	nandradNetwork.m_elements.reserve(vicusNetwork.m_nodes.size() * maxNumberElements + 2 * vicusNetwork.m_edges.size());

	unsigned int fmiValueRef = 100;

	std::map<unsigned int, std::vector<unsigned int>> componentElementMap; // this map stores the element ids for each component
	std::vector<unsigned int> allNodeIds = {};			// stores all nodeIds of the network (the ids which are used to connect elements)
	std::vector<unsigned int> allElementIds = {};		// stores all element ids of the network
	std::map<unsigned int, unsigned int> supplyNodeIdMap; // a map that stores for each VICUS geometric node the NANDRAD inlet node
	std::map<unsigned int, unsigned int> returnNodeIdMap; // a map that stores for each VICUS geometric node the NANDRAD outlet node

	// iterate over all geometric network nodes
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes) {

		// for each vicus geometric node: store two new node ids (supply and return) for the nandrad network
		supplyNodeIdMap[node.m_id] = uniqueIdAdd(allNodeIds);
		returnNodeIdMap[node.m_id] = uniqueIdAdd(allNodeIds);

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
			NANDRAD::HydraulicNetworkElement newElement(id, elem.m_inletNodeId, elem.m_outletNodeId,
														elem.m_componentId, elem.m_controlElementId);

			// 2. set the new elements inlet and outlet id using the map that we created
			newElement.m_inletNodeId = subNetNodeIdMap[elem.m_inletNodeId];
			newElement.m_outletNodeId = subNetNodeIdMap[elem.m_outletNodeId];

			// 3. get component name in display name
			const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, newElement.m_componentId);
			Q_ASSERT(comp!=nullptr);
			QString name = QString("%1(%2)_#%3").arg(node.m_displayName).arg(elem.m_displayName).arg(elem.m_id);
			newElement.m_displayName = IBK::replace_string(name.toStdString(), " ", "_");

			// 4. if this is a source node: set the respective reference element id of the network (for pressure calculation)
			if (node.m_type == VICUS::NetworkNode::NT_Source)
				nandradNetwork.m_referenceElementId = newElement.m_id;

			// 5. if this element is the one which shall exchange heat: we copy the respective heat exchange properties from the node
			// we recognize this using the original element id (origElem.m_id)
			if (elem.m_id == sub->m_idHeatExchangeElement)
				newElement.m_heatExchange = node.m_heatExchange;

			// 6. SPECIAL CASEs:
			// Ground Heat Exchanger
			if (comp->m_modelType == VICUS::NetworkComponent::MT_HorizontalGroundHeatExchanger){
				// --> retrieve parameters from the component
				NANDRAD::KeywordList::setParameter(newElement.m_para, "HydraulicNetworkElement::para_t",
												   NANDRAD::HydraulicNetworkElement::P_Length,
												   comp->m_para[VICUS::NetworkComponent::P_LengthOfGroundHeatExchangerPipes].value);
				newElement.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelPipes] = comp->m_intPara[VICUS::NetworkComponent::IP_NumberParallelPipes];

				// --> set FMI input
				NANDRAD::FMIVariableDefinition inputDefGHX;
				inputDefGHX.m_objectId = newElement.m_id;
				inputDefGHX.m_fmiVarName = newElement.m_displayName + ".Temperature";
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
				outputDefGHX.m_objectId = newElement.m_id;
				outputDefGHX.m_fmiVarName = newElement.m_displayName + ".HeatLoss";
				outputDefGHX.m_varName = NANDRAD::KeywordList::Keyword("ModelInputReference::referenceType_t",
																	NANDRAD::ModelInputReference::MRT_NETWORKELEMENT );
				outputDefGHX.m_varName += ".FlowElementHeatLoss";
				outputDefGHX.m_unit = "W";
				outputDefGHX.m_fmiValueRef = ++fmiValueRef;
				outputDefGHX.m_fmiVarDescription = "Heat flux from flow element into environment";
				outputDefGHX.m_fmiStartValue = 0;
				p.m_fmiDescription.m_outputVariables.push_back(outputDefGHX);
			}

			// PressureLossElement
			if (comp->m_modelType == VICUS::NetworkComponent::MT_PressureLossElement){
				newElement.m_intPara[NANDRAD::HydraulicNetworkElement::IP_NumberParallelElements] =
						comp->m_intPara[VICUS::NetworkComponent::IP_NumberParallelElements];
			}

			// 7. some components store a pipe properties id, so we transfer them to the element
			if (VICUS::NetworkComponent::hasPipeProperties(comp->m_modelType))
				newElement.m_pipePropertiesId = comp->m_pipePropertiesId;

			// 8. add element to the nandrad network
			nandradNetwork.m_elements.push_back(newElement);

			// 9. we store the element ids for each component, with this info we can create the schedules and object lists
			componentElementMap[elem.m_componentId].push_back(newElement.m_id);
		}


	}  // end of iteration over network nodes

	if(!errorStack.empty())
		return;


	// *** Transfer SCHEDULES from Vicus to Nandrad

	for (auto it=componentElementMap.begin(); it!=componentElementMap.end(); ++it){

		const VICUS::NetworkComponent *comp = VICUS::element(m_embeddedDB.m_networkComponents, it->first);
		Q_ASSERT(comp != nullptr);

		// create and add object list
		NANDRAD::ObjectList objList;
		objList.m_name = comp->m_displayName.string(IBK::MultiLanguageString::m_language, "en") + " elements";
		objList.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
		for (unsigned int elementId: it->second)
			objList.m_filterID.m_ids.insert(elementId);
		p.m_objectLists.push_back(objList);

		// get a list with required schedule names, they correspond to the component schedule ids
		std::vector<std::string> scheduleNames= NANDRAD::HydraulicNetworkComponent::requiredScheduleNames(
					NetworkComponent::nandradNetworkComponentModelType(comp->m_modelType));
		Q_ASSERT(scheduleNames.size() == comp->m_scheduleIds.size());

		// add schedules of component to nandrad
		for (unsigned int i = 0; i<comp->m_scheduleIds.size(); ++i){
			const VICUS::Schedule *sched = VICUS::element(m_embeddedDB.m_schedules, comp->m_scheduleIds[i]);
			if (sched == nullptr){
				errorStack.append(tr("Schedule with id #%1, referenced in network component with id #%2"
									 " does not exist").arg(comp->m_scheduleIds[i]).arg(comp->m_id));
				continue;
			}
			if (!sched->isValid()){
				errorStack.append(tr("Schedule with id #%1 has invalid parameters").arg(sched->m_id));
				continue;
			}
			addVicusScheduleToNandradProject(*sched, scheduleNames[i], p, objList.m_name);
		}
	}
	if(!errorStack.empty())
		return;


	// *** Transfer EDGES / PIPE ELEMENTS from Vicus to Nandrad

	// find source node and create set of edges, which are ordered according to their distance to the source node
	std::set<const VICUS::NetworkNode *> dummyNodeSet;
	std::vector<const VICUS::NetworkEdge *> orderedEdges;
	for (const VICUS::NetworkNode &node: vicusNetwork.m_nodes){
		if (node.m_type == VICUS::NetworkNode::NT_Source){
			node.setInletOutletNode(dummyNodeSet, orderedEdges);
			break;
		}
	}

	// now iterate over edges
	std::vector<unsigned int> compIds(componentIds.begin(), componentIds.end());
	for (const VICUS::NetworkEdge *edge: orderedEdges) {

		// check and transform pipe model type
		NANDRAD::HydraulicNetworkComponent::ModelType pipeModelType = NANDRAD::HydraulicNetworkComponent::NUM_MT;
		switch (edge->m_pipeModel ) {
			case VICUS::NetworkEdge::PM_SimplePipe:{
				pipeModelType = NANDRAD::HydraulicNetworkComponent::MT_SimplePipe;
			} break;
			case VICUS::NetworkEdge::PM_DynamicPipe:{
				pipeModelType = NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe;
			} break;
			case VICUS::NetworkEdge::NUM_PM:{
				errorStack.append(tr("Edge %1->%2 has no valid pipe model type").arg(edge->m_node1->m_id).arg(edge->m_node2->m_id));
				continue;
			}
		}

		// check if a pipe component with this model type exists already
		NANDRAD::HydraulicNetworkComponent pipeComp;
		bool pipeComponentExists = false;
		for (const NANDRAD::HydraulicNetworkComponent &comp: nandradNetwork.m_components){
			if (comp.m_modelType == pipeModelType){
				pipeComp = comp;
				pipeComponentExists = true;
				break;
			}
		}

		// if there was none, add respective component for pipe
		if (!pipeComponentExists){
			NANDRAD::HydraulicNetworkComponent comp;
			comp.m_id = uniqueIdAdd(compIds);
			comp.m_modelType = pipeModelType;
			if (pipeModelType == NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe){

				if (vicusNetwork.m_para[VICUS::Network::P_MaxPipeDiscretization].empty())
					throw IBK::Exception(IBK::FormatString("Missing Parameter '%1' in network with id #%2")
										.arg(VICUS::KeywordList::Keyword("Network::para_t", VICUS::Network::P_MaxPipeDiscretization))
										.arg(vicusNetwork.m_id), FUNC_ID);
				NANDRAD::KeywordList::setParameter(comp.m_para, "HydraulicNetworkComponent::para_t",
													NANDRAD::HydraulicNetworkComponent::P_PipeMaxDiscretizationWidth,
													vicusNetwork.m_para[VICUS::Network::P_MaxPipeDiscretization].value);
			}
			nandradNetwork.m_components.push_back(comp);
			pipeComp = comp;
		}

		// check if there is a reference to a pipe from DB
		const VICUS::NetworkPipe *pipe = VICUS::element(m_embeddedDB.m_pipes, edge->m_idPipe);
		if (pipe == nullptr){
			errorStack.append(tr("Edge %1->%2 has no defined pipe from database").arg(edge->m_node1->m_id).arg(edge->m_node2->m_id));
			continue;
		}

		// create name
		IBK::FormatString pipeName = IBK::FormatString("%1#%2_%3#%4")
				.arg(vicusNetwork.nodeById(edge->m_idNodeInlet)->m_displayName.toStdString())
				.arg(vicusNetwork.nodeById(edge->m_idNodeInlet)->m_id)
				.arg(vicusNetwork.nodeById(edge->m_idNodeOutlet)->m_displayName.toStdString())
				.arg(vicusNetwork.nodeById(edge->m_idNodeOutlet)->m_id);

		// add inlet pipe element
		unsigned int inletNode = supplyNodeIdMap[edge->m_idNodeInlet];
		unsigned int outletNode = supplyNodeIdMap[edge->m_idNodeOutlet];
		NANDRAD::HydraulicNetworkElement supplyPipe(uniqueIdAdd(allElementIds),
													inletNode,
													outletNode,
													pipeComp.m_id,
													edge->m_idPipe,
													edge->length());
		supplyPipe.m_displayName = "SupplyPipe." + pipeName.str();
		supplyPipe.m_heatExchange = edge->m_heatExchange;
		nandradNetwork.m_elements.push_back(supplyPipe);

		// add outlet pipe element
		inletNode = returnNodeIdMap[edge->m_idNodeOutlet];
		outletNode = returnNodeIdMap[edge->m_idNodeInlet];
		NANDRAD::HydraulicNetworkElement returnPipe(uniqueIdAdd(allElementIds),
													inletNode,
													outletNode,
													pipeComp.m_id,
													edge->m_idPipe,
													edge->length());
		returnPipe.m_displayName = "ReturnPipe." + pipeName.str();
		returnPipe.m_heatExchange = edge->m_heatExchange;
		nandradNetwork.m_elements.push_back(returnPipe);


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

//			// einfacher Ansatz: für jede Edge ein Delphin Modell
//			++idSoilModel;
//			mapSoil2SupplyPipes[idSoilModel].push_back(supplyPipe.m_id);
//			mapSoil2ReturnPipes[idSoilModel].push_back(returnPipe.m_id);

		}
	}  // end of iteration over edges
	if(!errorStack.empty())
		return;


	// besserer Ansatz: entlang der Pfade gehen und entsprechend des TempChangeIndicator die Delphin Modelle zuweisen...

	Database<NetworkPipe> dbPipes = Database<NetworkPipe>(1); // we dont care
	dbPipes.setData(m_embeddedDB.m_pipes);
	std::map<unsigned int, std::vector<NetworkEdge *> > shortestPaths;
	vicusNetwork.calcTemperatureChangeIndicator(fluid, dbPipes, shortestPaths);

	std::map<unsigned int, std::vector<unsigned int> > mapSoil2SupplyPipes;
	std::map<unsigned int, std::vector<unsigned int> > mapSoil2ReturnPipes;
	std::vector<unsigned int> allSoilModelIds;
	unsigned int soilModelId = 1;

	for (const NetworkEdge &e: vicusNetwork.m_edges){
		e.m_idSoil = VICUS::INVALID_ID;
		e.m_cumulativeTempChangeIndicator = -1;
	}

	double cumTempChangeindicatorMax = 0;
	double cumTempChangeindicatorMin = std::numeric_limits<double>::max();

	for (auto it = shortestPaths.begin(); it != shortestPaths.end(); ++it){

		double cumTempChangeindicator = 0;
		std::vector<NetworkEdge *> &shortestPath = it->second; // for readability
		for (const NetworkEdge * edge: shortestPath){

			// 1. calculate the cumulative temperature change indicator
			cumTempChangeindicator += edge->m_tempChangeIndicator;

			if (edge->m_cumulativeTempChangeIndicator < 0)
				edge->m_cumulativeTempChangeIndicator = cumTempChangeindicator;

			if (cumTempChangeindicator > cumTempChangeindicatorMax)
				cumTempChangeindicatorMax = cumTempChangeindicator;
			if (cumTempChangeindicator < cumTempChangeindicatorMin)
				cumTempChangeindicatorMin = cumTempChangeindicator;
		}
	}

	unsigned int Nranges = 10;
	std::vector<double> ranges(Nranges);
	for (unsigned int i=0; i<Nranges; ++i){
		ranges[i] = cumTempChangeindicatorMin  +
				(i+1) * (cumTempChangeindicatorMax - cumTempChangeindicatorMin) / Nranges;
	}

	for (const NetworkEdge &edge: vicusNetwork.m_edges){

		if (edge.m_idSoil != VICUS::INVALID_ID)
			continue;

		if (edge.m_cumulativeTempChangeIndicator <= ranges[0]){
			edge.m_idSoil = 1;
		}
		else {
			for (unsigned int i=1; i<Nranges; ++i){
				if (edge.m_cumulativeTempChangeIndicator <= ranges[i]){
					edge.m_idSoil = i+1;
					break;
				}
			}
		}

		if (edge.m_idSoil != VICUS::INVALID_ID){
			mapSoil2SupplyPipes[edge.m_idSoil].push_back(edge.m_idNandradSupplyPipe);
			mapSoil2ReturnPipes[edge.m_idSoil].push_back(edge.m_idNandradReturnPipe);
		}
	}



	// write mapping file
	std::ofstream f;
	std::string file = IBK::Path(nandradProjectPath).withoutExtension().str() + ".mapping";
	f.open(file, std::ofstream::out | std::ofstream::trunc);
	f << "soilId" << "\t" << "supplyPipeIds" << "\t" << "returnPipeIds" << std::endl;
	for (auto it=mapSoil2SupplyPipes.begin(); it!=mapSoil2SupplyPipes.end(); ++it ){
		unsigned int soilId = it->first;
		f << soilId << "\t";
		for (unsigned int supplyId: mapSoil2SupplyPipes.at(soilId))
			f << supplyId << ",";
		f << "\t";
		for (unsigned int returnId: mapSoil2ReturnPipes.at(soilId))
			f << returnId << ",";
		f << std::endl;
	}
	f.close();


	vicusNetwork.writeNetworkNodesCSV(IBK::Path(nandradProjectPath).withoutExtension() + "_NetworkNodes.csv");
	vicusNetwork.writeNetworkEdgesCSV(IBK::Path(nandradProjectPath).withoutExtension() + "_NetworkEdges.csv");


	 // we are DONE !!!
	 // finally add to nandrad project
	p.m_hydraulicNetworks.push_back(nandradNetwork);

}


} // namespace VICUS

