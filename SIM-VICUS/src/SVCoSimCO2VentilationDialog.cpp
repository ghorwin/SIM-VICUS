#include "SVCoSimCO2VentilationDialog.h"
#include "ui_SVCoSimCO2VentilationDialog.h"

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVPreferencesDialog.h"

#include <QtExt_Directories.h>

#include <JlCompress.h>

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include <NANDRAD_Project.h>

#include <IBK_algorithm.h>
#include <IBK_BuildFlags.h>
#include <IBK_FormatString.h>

#define FMI_INPUT_AmbientTemperature 1
#define FMI_INPUT_RoomAirTemperature 1000
#define FMI_INPUT_MaximumCO2Concentration 2000
#define FMI_INPUT_MinimumAirTemperature 3000
#define FMI_INPUT_MaximumAirTemperature 4000
#define FMI_INPUT_MinimumAirChangeRate 5000
#define FMI_INPUT_MaximumAirChangeRateIncrease 6000
#define FMI_OUTPUT_AirChangeRate 7000
#define FMI_OUTPUT_CO2Concentration 8000


struct SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject {
	// mapping from zone id to zone volume
	std::map<unsigned int, double> m_zoneVolumes;
	// mapping from zone id to zone floor area
	std::map<unsigned int, double> m_zoneFloorAreas;
	// find out all zone template ids:
	// mapping from zone id to corresponding CO2 load template ids
	std::map<unsigned int, QString> m_zoneScheduleNames;
	// template names -> used for connecting external tsv files
	std::map<unsigned int, QString>	m_zoneTemplateNames;
	// schedules for CO2 production
	std::vector<VICUS::Schedule> m_schedules;
	// schedule names for CO2 production
	std::vector<std::string>	m_scheduleNames;
	// fmi definition for input variables
	std::vector<NANDRAD::FMIVariableDefinition> m_inputVariables;
	// fmi definition for output variables
	std::vector<NANDRAD::FMIVariableDefinition> m_outputVariables;
	// simulation start time [s]
	double						m_simulationStartTime = -999;
	// simulation end time [s]
	double						m_simulationEndTime = -999;

	/*! Check if room is covered by ventilation control. */
	static bool hasVentilationControl(const VICUS::Room &room, const VICUS::Project &project);
	/*! Check if room is covered by ventilation model and checks parameters. Throws an exception if something is missing*/
	static void checkVentilationControl(const VICUS::Room &room, const VICUS::Project &project, QString &errmsg);
	/*! Check if room is covered by ventilation model and checks parameters. Fills an error string if something is missing*/
	static void checkVentilation(const VICUS::Room &room, const VICUS::Project &project, QString &errmsg);

	// initialize from VICUS project file
	void init(const VICUS::Project &project);
	// write project into file
	void write(const QString &targetFile);
	/*! This generates all schedules with CO2 load and stores their name. */
	void generateCO2LoadSchedules(const VICUS::Project &project);
	/*! This generates all sfmi input and output variables. */
	void generateFMIDefinitions();
};


void SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject::init(const VICUS::Project & project)
{
	// get simulation start and end time
	m_simulationStartTime = project.m_simulationParameter.m_interval.m_para[NANDRAD::Interval::P_Start].value;
	m_simulationEndTime = project.m_simulationParameter.m_interval.endTime();

	// create string vector for direct acess
	std::vector<std::string>	scheduleNames;
	std::vector<VICUS::Schedule>	schedules;

	// loop over all buildings...
	for(const VICUS::Building &building : project.m_buildings) {

		//..., building levels...
		for(const VICUS::BuildingLevel &buildingLevel : building.m_buildingLevels) {

			//... and rooms
			for(const VICUS::Room &room : buildingLevel.m_rooms) {

				// only register zones with natural ventilation control
				if(!hasVentilationControl(room, project))
					continue;

				// retrieve room parameters
				m_zoneVolumes[room.m_id] = room.m_para[VICUS::Room::P_Volume].value;
				m_zoneFloorAreas[room.m_id] = room.m_para[VICUS::Room::P_Area].value;

				// retrieve zone template
				IBK_ASSERT(room.m_idZoneTemplate != VICUS::INVALID_ID);

				// retrieve zone remplate
				const VICUS::ZoneTemplate * zoneTemplate = dynamic_cast<const VICUS::ZoneTemplate*>
						(VICUS::element(project.m_embeddedDB.m_zoneTemplates,
										 room.m_idZoneTemplate) );

				IBK_ASSERT(zoneTemplate != nullptr);
				// no ventilation control possible
				QString templateName = QString::fromStdString(zoneTemplate->m_displayName.string());
				// remove shift
				templateName.replace(" ", "_");
				m_zoneTemplateNames[room.m_id] = templateName;
			}
		}
	}

	// generate schedules
	generateCO2LoadSchedules(project);
	// generate FMI definitions
	generateFMIDefinitions();
}


void SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject::write(const QString &targetDir)
{
	FUNCID(CO2ComfortVentilationProject::write);
	// write project file

	QString projectFile = targetDir + "/Project.txt";
	// open file
	QFile file(projectFile);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		IBK::IBK_Message(IBK::FormatString("Cannot create/write file '%1' (path does not exists or missing permissions).")
						 .arg(projectFile.toStdString()), IBK::MSG_ERROR, FUNC_ID);
		return;
	}

	// write file
	QTextStream fileData(&file);

	// write zone ids
	fileData << "zoneIds\t" << "=" << "[";

	bool firstId =true;
	for(const std::pair<unsigned int, double> &zoneVolume : m_zoneVolumes) {
		if(!firstId)
			fileData << ",";
		fileData << zoneVolume.first;
		firstId = false;
	}
	fileData << "]\n";

	// write zone volumes
	for(const std::pair<unsigned int, double> &zoneVolume : m_zoneVolumes) {
		fileData << "zoneVolumes[" << zoneVolume.first << "] = ";
		fileData << zoneVolume.second << " m3\n";
	}
	// write zone floor areas
	for(const std::pair<unsigned int, double> &zoneArea : m_zoneFloorAreas) {
		fileData << "zoneFloorAreas[" << zoneArea.first << "] = ";
		fileData << zoneArea.second << " m2\n";
	}
	// register schedules for CO2 load
	for(const std::pair<unsigned int, QString> &zoneSchedule : m_zoneScheduleNames) {
		// skip empty schedules
		if(zoneSchedule.second.isEmpty())
			continue;

		fileData << "zoneScheduleNames[" << zoneSchedule.first << "] = ";
		fileData << "\"" << zoneSchedule.second << "\"\n";
	}
	// write parameters:
	// CO2 tolerance band
	fileData << "CO2ToleranceBand = " << 50.0 << " ppm\n";
	// CO2 start value
	fileData << "startCO2Concentration = " << 1400.0 << " ppm\n";
	// temperature start value
	fileData << "startAirTemperature = " << 20.0 << " C";

	file.close();

	// Write schedues as tsv file

	for(unsigned int i = 0; i < m_schedules.size(); ++i) {

		// skip empty schedules
		if(m_scheduleNames[i].empty())
			continue;

		// create a single file for each schedule
		QString scheduleName(m_scheduleNames[i].c_str());
		QString tsvFile = targetDir + "/" + scheduleName + ".tsv";

		// retrieve data sets
		std::vector<double> timePoints, data;
		// retrieve time points in [h], data in SI units
		m_schedules[i].createYearDataVector(timePoints, data);
		IBK_ASSERT(timePoints.size() == data.size());

		// write file
		QFile file(tsvFile);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			IBK::IBK_Message(IBK::FormatString("Cannot create/write file '%1' (path does not exists or missing permissions).")
							 .arg(tsvFile.toStdString()), IBK::MSG_ERROR, FUNC_ID);
			return;
		}

		// write file header
		QTextStream tsvData(&file);

		// write header
		tsvData << "Time [h]\tCO2ProductionPerFloorArea [kg/m2s]\n";

		// write file
		for(unsigned int j = 0; j < timePoints.size(); ++j) {
			tsvData << timePoints[j] << "\t" << data[j] << "\n";
		}

		file.close();

//		m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg("CO2Balance/" + tsvFile));
	}

	// generate file for ambient CO2 concentration
	QString ambientTsvFile = targetDir + "/AmbientCO2ConcentrationSchedule.tsv";
	QFile ambientCo2file(ambientTsvFile);
	ambientCo2file.open(QIODevice::WriteOnly | QIODevice::Text);

	// write file header
	QTextStream tsvDataAmbientCo2(&ambientCo2file);

	// write header
	tsvDataAmbientCo2 << "Time [h]\tAmbientCO2Concentration [ppm]\n";

	// write file
	tsvDataAmbientCo2 << 0 << "\t" << 400 << "\n";
	tsvDataAmbientCo2 << 8760 << "\t" << 400 << "\n";

	ambientCo2file.close();
}


void SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject::generateCO2LoadSchedules(const VICUS::Project &project)
{
	// create string vector for direct acess
	std::vector<std::string>	scheduleNames;
	std::vector<VICUS::Schedule>	schedules;

	// loop over all buildings...
	for(const VICUS::Building &building : project.m_buildings) {

		//..., building levels...
		for(const VICUS::BuildingLevel &buildingLevel : building.m_buildingLevels) {

			//... and rooms
			for(const VICUS::Room &room : buildingLevel.m_rooms) {


				// only register zones with natural ventilation control
				if(!hasVentilationControl(room, project))
					continue;

				// generate new schedule
				QString scheduleName;

				// no model for person load
				if(room.m_idZoneTemplate == VICUS::INVALID_ID) {
					// register invalid object list
					scheduleNames.push_back(std::string());
					schedules.push_back(VICUS::Schedule());
					continue;
				}

				// retrieve zone remplate
				const VICUS::ZoneTemplate * zoneTemplate = dynamic_cast<const VICUS::ZoneTemplate*>
						(VICUS::element(project.m_embeddedDB.m_zoneTemplates,
										 room.m_idZoneTemplate) );

				// shouöd be checked in initialization
				IBK_ASSERT(zoneTemplate != nullptr);

				// find out id to corresponding person load and occupancy schedule:
				IDType idReference = zoneTemplate->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadPerson];

			//				if(idReference == VICUS::INVALID_ID)
			//					idReference = zoneTemplate->m_idReferences[VICUS::ZoneTemplate::ST_IntLoadOther];

				// no model for person load
				if(idReference == VICUS::INVALID_ID) {
					// register invalid object list
					scheduleNames.push_back(std::string());
					schedules.push_back(VICUS::Schedule());
					continue;
				}

				// find out internal load model
				const VICUS::InternalLoad * internalLoad = dynamic_cast<const VICUS::InternalLoad*>
						(VICUS::element(project.m_embeddedDB.m_internalLoads,
										 idReference) );

				// shouöd be checked in initialization
				IBK_ASSERT(internalLoad != nullptr);
				IBK_ASSERT(internalLoad->m_category == VICUS::InternalLoad::IC_Person);

				// we need an actiobity scehdule [W/Person] (obligatory)...
				unsigned int scheduleId = internalLoad->m_idActivitySchedule;

				const VICUS::Schedule * activitySchedule = dynamic_cast<const VICUS::Schedule*>
						(VICUS::element(project.m_embeddedDB.m_schedules,
												 scheduleId) );
				IBK_ASSERT(activitySchedule != nullptr);

				// ... and an occupancy schedule [---]
				scheduleId = internalLoad->m_idOccupancySchedule;

				const VICUS::Schedule * occupancySchedule = dynamic_cast<const VICUS::Schedule*>
						(VICUS::element(project.m_embeddedDB.m_schedules,
												 scheduleId) );

				IBK_ASSERT(occupancySchedule != nullptr);

				// find out maximum number of persons per area
				double nPersonPerArea = 0;

				switch(internalLoad->m_personCountMethod) {
					case VICUS::InternalLoad::PCM_PersonPerArea: {
						nPersonPerArea = internalLoad->m_para[VICUS::InternalLoad::P_PersonPerArea].value ;
					}
					break;
					case VICUS::InternalLoad::PCM_AreaPerPerson: {
						nPersonPerArea = 1.0 / internalLoad->m_para[VICUS::InternalLoad::P_AreaPerPerson].value;

					}
					break;
					case VICUS::InternalLoad::PCM_PersonCount: {
						nPersonPerArea = internalLoad->m_para[VICUS::InternalLoad::P_PersonCount].value
								/ room.m_para[VICUS::Room::P_Area].value;
					}
					break;
					case VICUS::InternalLoad::NUM_PCM:
					break;
				}

				// calculate CO2 production
				double massDensityCO2 = 1.98; //[kg/m3]
				double emissionRateCO2 = 3.82e-8; // [m3/Ws]

				// calculate CO2 production
				// heatProductionPerArea = nPersonPerArea * occupancy * activity
				VICUS::Schedule CO2LoadPerFloorAreaSchedule = activitySchedule->multiply(nPersonPerArea);
				CO2LoadPerFloorAreaSchedule = CO2LoadPerFloorAreaSchedule.multiply(*occupancySchedule);

				// CO2ProductionPerFloorArea = CO2ProductionPerFloorArea  * massDensityCO2 * emissionRateCO2
				CO2LoadPerFloorAreaSchedule = CO2LoadPerFloorAreaSchedule.multiply(massDensityCO2 * emissionRateCO2);


				// check if schedule exists
				bool found = false;
				// loop over all existing schedules
				for(unsigned int j = 0; j < schedules.size(); ++j) {
					if(CO2LoadPerFloorAreaSchedule.isSimilar(schedules[j])) {
						// explode template name...
						scheduleName = QString::fromStdString(scheduleNames[j]);
						//.. and register as found
						found = true;
						break;
					}
				}
				// create a new schedule
				if(!found) {
					scheduleName = "CO2LoadPerFloorAreaSchedule_" + QString(zoneTemplate->m_displayName.string().c_str());
					// remove shift
					scheduleName.replace(" ", "_");

					std::string scheduleNameStr = scheduleName.toStdString();
					IBK_ASSERT(!scheduleName.isEmpty());
					// register name
					scheduleNameStr = IBK::pick_name(scheduleNameStr,
												  scheduleNames.begin(), scheduleNames.end());
					scheduleName = QString::fromStdString(scheduleNameStr);
						// register schedule
					schedules.push_back(CO2LoadPerFloorAreaSchedule);
					scheduleNames.push_back(scheduleNameStr);
				}

				// loop over all new generated schedules
				m_zoneScheduleNames[room.m_id] = scheduleName;
			}
		}
	}
	// copy schedules
	m_schedules = schedules;
	// copy schedule names
	m_scheduleNames = scheduleNames;

}


void SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject::generateFMIDefinitions()
{
	// generate mapping for output and input variables
	std::map<unsigned int, NANDRAD::FMIVariableDefinition> co2InputVariables;
	std::map<unsigned int, NANDRAD::FMIVariableDefinition> co2OutputVariables;

	// create input variable for ambient temperature
	NANDRAD::FMIVariableDefinition ambientTemp;
	ambientTemp.m_fmiVarName = "AmbientTemperature";
	ambientTemp.m_fmiVarDescription = "AmbientTemperature";
	ambientTemp.m_fmiValueRef = FMI_INPUT_AmbientTemperature;
	ambientTemp.m_objectId = 0;
	ambientTemp.m_varName = "Location.Temperature";
	ambientTemp.m_unit = "K";
	ambientTemp.m_fmiStartValue = 273.15;

	co2InputVariables[ambientTemp.m_fmiValueRef] = ambientTemp;

	// create reference for room air tempertaure
	for(const std::pair<unsigned int, double> &zoneVolume : m_zoneVolumes) {
		// use zone id as unique identifier for FMI value reference
		unsigned int zoneId = zoneVolume.first;

		// create reference for room air tempertaure
		NANDRAD::FMIVariableDefinition inputVar;
		inputVar.m_fmiVarName = "RoomAirTemperature_id_" + IBK::val2string<unsigned int>(zoneId);
		inputVar.m_fmiVarDescription = "RoomAirTemperature";
		inputVar.m_fmiValueRef = FMI_INPUT_RoomAirTemperature + zoneId;
		inputVar.m_fmiTypeName = "input";
		inputVar.m_objectId = zoneId;
		inputVar.m_varName = "Zone.AirTemperature";
		inputVar.m_unit = "K";
		inputVar.m_fmiStartValue = 293.15;

		co2InputVariables[inputVar.m_fmiValueRef] = inputVar;

		// create reference for maximum CO2 concentration
		inputVar.m_fmiVarName = "MaximumCO2Concentration_id_" + IBK::val2string<unsigned int>(zoneId);
		inputVar.m_fmiVarDescription = "MaximumCO2Concentration";
		inputVar.m_fmiValueRef = FMI_INPUT_MaximumCO2Concentration + zoneId;
		inputVar.m_varName.clear();
		inputVar.m_fmiStartValue = 0.0;
		inputVar.m_unit = "mol/mol";
		co2InputVariables[inputVar.m_fmiValueRef] = inputVar;

		// create reference for minimum air temperature
		inputVar.m_fmiVarName = "MinimumAirTemperature_id_" + IBK::val2string<unsigned int>(zoneId);
		inputVar.m_fmiVarDescription = "MinimumAirTemperature";
		inputVar.m_fmiValueRef = FMI_INPUT_MinimumAirTemperature + zoneId;
		inputVar.m_varName = "Zone.VentilationMinAirTemperatureSchedule";
		inputVar.m_fmiStartValue = 293.15;
		inputVar.m_unit = "K";
		co2InputVariables[inputVar.m_fmiValueRef] = inputVar;

		// create reference for maximum air temperature
		inputVar.m_fmiVarName = "MaximumAirTemperature_id_" + IBK::val2string<unsigned int>(zoneId);
		inputVar.m_fmiVarDescription = "MaximumAirTemperature";
		inputVar.m_fmiValueRef = FMI_INPUT_MaximumAirTemperature + zoneId;
		inputVar.m_varName = "Zone.VentilationMaxAirTemperatureSchedule";
		inputVar.m_fmiStartValue = 293.15;
		inputVar.m_unit = "K";
		co2InputVariables[inputVar.m_fmiValueRef] = inputVar;

		// create reference for minimum air change rate
		inputVar.m_fmiVarName = "MinimumAirChangeRate_id_" + IBK::val2string<unsigned int>(zoneId);
		inputVar.m_fmiVarDescription = "MinimumAirChangeRate";
		inputVar.m_fmiValueRef = FMI_INPUT_MinimumAirChangeRate + zoneId;
		inputVar.m_varName = "Zone.VentilationRateSchedule";
		inputVar.m_fmiStartValue = 0.0;
		inputVar.m_unit = "1/s";
		co2InputVariables[inputVar.m_fmiValueRef] = inputVar;

		// create reference for maximum air change rate
		inputVar.m_fmiVarName = "MaximumAirChangeRateIncrease_id_" + IBK::val2string<unsigned int>(zoneId);
		inputVar.m_fmiVarDescription = "MaximumAirChangeRateIncrease";
		inputVar.m_fmiValueRef = FMI_INPUT_MaximumAirChangeRateIncrease + zoneId;
		inputVar.m_varName = "Zone.VentilationRateIncreaseSchedule";
		inputVar.m_fmiStartValue = 0.0;
		inputVar.m_unit = "1/s";
		co2InputVariables[inputVar.m_fmiValueRef] = inputVar;

		// generate input variables
		// create reference for controlled air change rate
		NANDRAD::FMIVariableDefinition outputVar;
		outputVar.m_fmiVarName = "AirChangeRate_id_" + IBK::val2string<unsigned int>(zoneId);
		outputVar.m_fmiVarDescription = "AirChangeRate";
		outputVar.m_fmiValueRef = FMI_OUTPUT_AirChangeRate + zoneId;
		outputVar.m_fmiTypeName = "output";
		outputVar.m_objectId = zoneId;
		outputVar.m_varName = "Zone.VentilationRate";
		outputVar.m_fmiStartValue = 0.0;
		outputVar.m_unit = "1/s";
		co2OutputVariables[outputVar.m_fmiValueRef] = outputVar;

		// create reference for clculated CO2 concentration
		outputVar.m_fmiVarName = "CO2Concentration_id_" + IBK::val2string<unsigned int>(zoneId);
		outputVar.m_fmiVarDescription = "CO2Concentration";
		outputVar.m_fmiValueRef = FMI_OUTPUT_CO2Concentration + zoneId;
		outputVar.m_varName.clear();
		outputVar.m_fmiStartValue = 0.0;
		outputVar.m_unit = "mol/mol";
		co2OutputVariables[outputVar.m_fmiValueRef] = outputVar;
	}

	// create an input and output vector for CO2 ventilation model
	std::vector<NANDRAD::FMIVariableDefinition> inputVars;
	std::vector<NANDRAD::FMIVariableDefinition> outputVars;

	for(const std::pair<unsigned int, NANDRAD::FMIVariableDefinition> &inputVar : co2InputVariables) {
		m_inputVariables.push_back(inputVar.second);
	}
	for(const std::pair<unsigned int, NANDRAD::FMIVariableDefinition> &outputVar : co2OutputVariables) {
		m_outputVariables.push_back(outputVar.second);
	}

}


bool SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject::hasVentilationControl(const VICUS::Room & room, const VICUS::Project &project)
{
	// no zone template
	if(room.m_idZoneTemplate == VICUS::INVALID_ID) {
		// no ventilation control possible
		return false;
	}

	// retrieve zone remplate
	const VICUS::ZoneTemplate * zoneTemplate = dynamic_cast<const VICUS::ZoneTemplate*>
			(VICUS::element(project.m_embeddedDB.m_zoneTemplates,
							 room.m_idZoneTemplate) );

	// shouöd be checked in initialization
	IBK_ASSERT(zoneTemplate != nullptr);

	// find out id to corresponding person load and occupancy schedule:
	IDType idReference = zoneTemplate->m_idReferences[VICUS::ZoneTemplate::ST_ControlVentilationNatural];

	// no model for ventilation control
	if(idReference == VICUS::INVALID_ID) {
		return false;
	}

	return true;
}


void SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject::checkVentilationControl(const VICUS::Room & room, const VICUS::Project & project, QString &errmsg)
{
	// no zone template
	if(room.m_idZoneTemplate == VICUS::INVALID_ID) {
		// no ventilation control possible
		errmsg = "Missing template 'ControlVentilationNatural' for room '" + room.m_displayName + "' Please add missing template!";
		return;
	}

	// retrieve zone remplate
	const VICUS::ZoneTemplate * zoneTemplate = dynamic_cast<const VICUS::ZoneTemplate*>
			(VICUS::element(project.m_embeddedDB.m_zoneTemplates,
							 room.m_idZoneTemplate) );

	// shouöd be checked in initialization
	IBK_ASSERT(zoneTemplate != nullptr);

	// find out id to corresponding person load and occupancy schedule:
	IDType idReference = zoneTemplate->m_idReferences[VICUS::ZoneTemplate::ST_ControlVentilationNatural];

	// no model for ventilation control
	if(idReference == VICUS::INVALID_ID) {
		// no ventilation control possible
		errmsg = "Missing template 'ControlVentilationNatural' for room '" + room.m_displayName + "' Please add missing template!";
		return;
	}

	// should be checked in initialization
	IBK_ASSERT(dynamic_cast<const VICUS::ZoneControlNaturalVentilation*>
			   (VICUS::element(project.m_embeddedDB.m_zoneControlVentilationNatural,
								idReference) ) != nullptr);
}


void SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject::checkVentilation(const VICUS::Room & room, const VICUS::Project & project, QString &errmsg)
{
	// no zone template
	if(room.m_idZoneTemplate == VICUS::INVALID_ID) {
		// no ventilation control possible
		errmsg = "Missing template 'NaturalVentilation' for room '" + room.m_displayName + "' Please add missing template!";
		return;
	}

	// retrieve zone remplate
	const VICUS::ZoneTemplate * zoneTemplate = dynamic_cast<const VICUS::ZoneTemplate*>
			(VICUS::element(project.m_embeddedDB.m_zoneTemplates,
							 room.m_idZoneTemplate) );

	// shouöd be checked in initialization
	IBK_ASSERT(zoneTemplate != nullptr);

	// find out id to corresponding person load and occupancy schedule:
	IDType idReference = zoneTemplate->m_idReferences[VICUS::ZoneTemplate::ST_VentilationNatural];

	// no model for ventilation control
	if(idReference == VICUS::INVALID_ID) {
		// no ventilation control possible
		errmsg = "Missing template 'NaturalVentilation' for room '" + room.m_displayName + "' Please add missing template!";
		return;
	}

	// Check for parameter '
	// find out internal load model
	const VICUS::VentilationNatural * ventilation = dynamic_cast<const VICUS::VentilationNatural*>
			(VICUS::element(project.m_embeddedDB.m_ventilationNatural,
							 idReference) );

	// should be checked in initialization
	IBK_ASSERT(ventilation != nullptr);

	// we need an actiobity schedule for air change rate (obligatory)...
	unsigned int scheduleId = ventilation->m_idSchedule;

	// missing schedule
	if(scheduleId == VICUS::INVALID_ID) {
		// no ventilation control possible
		errmsg = "Missing schedule 'NaturalVentilation' for room '" + room.m_displayName + "'. Please add missing schedule data!";
		return;
	}
}


SVCoSimCO2VentilationDialog::SVCoSimCO2VentilationDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVCoSimCO2VentilationDialog)
{
	m_ui->setupUi(this);
}


SVCoSimCO2VentilationDialog::~SVCoSimCO2VentilationDialog() {
	delete m_ui;
}


int SVCoSimCO2VentilationDialog::exec() {
	m_ui->plainTextEdit->clear();
	m_ui->plainTextEdit->setEnabled(false);
	m_ui->pushButtonLaunchMasterSim->setEnabled(false);
	// check VICUS project for sufficient data input for CO2 ventilation and update button states
	if (!checkProjectData()) {
		// disable buttons
		m_ui->pushButtonGenerate->setEnabled(false);
		return QDialog::exec();
	}
	// enable generate button
	m_ui->pushButtonGenerate->setEnabled(true);
	return QDialog::exec();
}


bool SVCoSimCO2VentilationDialog::checkProjectData() const {
	const VICUS::Project & p = project();

	QString errorColor = SVSettings::instance().m_theme == SVSettings::TT_Dark ? "red" : "darkRed";
	QString successColor = SVSettings::instance().m_theme == SVSettings::TT_Dark ? "lime" : "darkGreen";

	bool success = true;

	if (SVProjectHandler::instance().projectFile().isEmpty()) {
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Please save project first!</span>").arg(errorColor));
		return false;
	}
	// check for correct extension
	if (!SVProjectHandler::instance().projectFile().toLower().endsWith(".vicus")) {
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Expected project file name to end with '.vicus'. Please save project with correct file name!</span>").arg(errorColor));
		return false;
	}

	// check for sufficient project parameters
	bool hasControlledZones = false;
	// check all zones
	// loop over all buildings...
	for(const VICUS::Building &building : p.m_buildings) {

		//..., building levels...
		for(const VICUS::BuildingLevel &buildingLevel : building.m_buildingLevels) {

			//... and rooms
			for(const VICUS::Room &room : buildingLevel.m_rooms) {

				// only register zones with natural ventilation control
				if(!CO2ComfortVentilationProject::hasVentilationControl(room, p))
					continue;

				// check parameters
				if(room.m_para[VICUS::Room::P_Volume].name.empty()) {
					m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Missing parameter 'Volume' for room '%2' Please add missing parameters!</span>").arg(errorColor).arg(room.m_displayName));
					return false;
				}
				if(room.m_para[VICUS::Room::P_Area].name.empty()) {
					m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Missing parameter 'Area' for room '%2' Please add missing parameters!</span>").arg(errorColor).arg(room.m_displayName));
					return false;
				}

				// we need a ventilation template in order to retrieve minimum air change rate
				QString errmsg;

				CO2ComfortVentilationProject::checkVentilation(room, p, errmsg);

				if(!errmsg.isEmpty()){
					m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">%2</span>").arg(errorColor).arg(errmsg));
					return false;
				}

				// check all necessary data for control ventilation
				CO2ComfortVentilationProject::checkVentilationControl(room, p, errmsg);

				if(!errmsg.isEmpty()){
					m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">%2</span>").arg(errorColor).arg(errmsg));
					return false;
				}

				// set control label
				hasControlledZones = true;
			}
		}
	}

	// comfort ventilation does not cover any zone
	if(!hasControlledZones) {
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Minimum one room must be covered by ventilation control. Please add Template 'NaturalVentilationControl' and fill parameters!</span>").arg(errorColor));
		return false;
	}

	if (success)
		m_ui->labelCheckSummary->setText(tr("<span style=\"color:%1\">Project data ok for CO2 balance and ventilation control model.</span>").arg(successColor));

	return success;
}


void SVCoSimCO2VentilationDialog::generateNandradFMU(NANDRAD::Project project, const QString &modelName,
													 const QString &tmpDir, const QString &fmuTargetFile) const
{
	FUNCID(SVCoSimCO2VentilationDialog::generateNandradFMU);

	bool success = true;

	// generate path to Nandrad FMU
	// create fmu directory
	QDir fmuDir(tmpDir);
	fmuDir.mkdir(fmuDir.path());

	QString resourceDir = tmpDir + "/resources";
	fmuDir.mkdir(resourceDir);

	// if we have a target path, copy the referenced climate data file to the new location and modify the path
	IBK::Path resourcePath(resourceDir.toStdString());
	IBK::Path fullClimatePath = project.m_location.m_climateFilePath.withReplacedPlaceholders(project.m_placeholders);
	if (!fullClimatePath.isFile()) {
		IBK_Message(IBK::FormatString("The referenced climate data file '%1' does not exist. Please select a climate data file!")
					.arg(fullClimatePath.str()), IBK::MSG_ERROR, FUNC_ID);
	}

	// target file path
	std::string targetFName = fullClimatePath.filename().str();
	targetFName = IBK::replace_string(targetFName, " ", "_");
	IBK::Path targetClimatePath = resourcePath / targetFName;
	IBK::IBK_Message( IBK::FormatString("Copying climate data file '%1' to '<fmu>/resources'\n").arg(fullClimatePath.filename()),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	if (!IBK::Path::copy(fullClimatePath, targetClimatePath)) {
		IBK_Message(IBK::FormatString("Cannot copy the referenced climate data file '%1' to target directory '%2'.")
					.arg(fullClimatePath.str()).arg(resourcePath.str()), IBK::MSG_ERROR, FUNC_ID);
	}
	// modify reference in project file
	project.m_location.m_climateFilePath = "${Project Directory}/" + targetFName;

	// copy used resources/tsv files

	// schedules with tsv-files
	for (std::map<std::string, std::vector<NANDRAD::LinearSplineParameter> >::iterator
		 it = project.m_schedules.m_annualSchedules.begin();
		 it != project.m_schedules.m_annualSchedules.end(); ++ it)
	{
		// TODO: skip FMI-substituted quantities

		for (NANDRAD::LinearSplineParameter & spline : it->second) {
			if (!spline.m_name.empty() && spline.m_tsvFile.isValid()) {
				// Mind: tsv file path may be relative path to project directory or elsewhere
				IBK::Path tsvFilePath = spline.m_tsvFile.withReplacedPlaceholders(project.m_placeholders);
				// TODO Anne, check if targetPath exists already and issue a warning/error message or
				//      append a new suffix to the tsv-filename to make it unique!
				//      Can happen if two LinearSplines reference tsv files like:
				//         /data/control/setpoints.tsv
				//         /data/control_alternative/setpoints.tsv
				IBK::Path targetPath = resourcePath / tsvFilePath.filename();
				IBK::Path::copy(tsvFilePath, targetPath);
				// change tsv file to point to relative path
				spline.m_tsvFile = IBK::Path("${Project Directory}/" + tsvFilePath.filename().str());
			}
		}
	}

	// network heatexchange spline data
	for (NANDRAD::HydraulicNetwork & n : project.m_hydraulicNetworks)
		for (NANDRAD::HydraulicNetworkElement & elem : n.m_elements) {
			for (int i=0; i<NANDRAD::HydraulicNetworkHeatExchange::NUM_SPL; ++i)
				if (!elem.m_heatExchange.m_splPara[i].m_name.empty() && elem.m_heatExchange.m_splPara[i].m_tsvFile.isValid()) {
					// Mind: tsv file path may be relative path to project directory or elsewhere
					IBK::Path tsvFilePath = elem.m_heatExchange.m_splPara[i].m_tsvFile.withReplacedPlaceholders(project.m_placeholders);
					IBK::Path targetPath = resourcePath / tsvFilePath.filename();
					IBK::Path::copy(tsvFilePath, targetPath);
					// change tsv file to point to relative path
					elem.m_heatExchange.m_splPara[i].m_tsvFile = IBK::Path("${Project Directory}/" + tsvFilePath.filename().str());
				}
		}


	// now all referenced files are stored alongside the project
	// remove not needed Database placeholder from placeholders list (but keep all custom placeholders!)
	auto it = project.m_placeholders.find("Database");
	if (it != project.m_placeholders.end())
		project.m_placeholders.erase(it);

	// now write the project into the export directory, it will always be called "Project.nandrad"
	project.writeXML(resourcePath / "Project.nandrad");
	IBK::IBK_Message( IBK::FormatString("Creating 'Project.nandrad' in '<fmu>/resources'\n"),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);


	// write model description

	QString modelId = "471a3b52-4923-44d8-ab4b-fcdb813c1244";
	QString modelDescription = "NANDRAD Simulation Model";
	QString version = NANDRAD::LONG_VERSION;

	generateModelDescription(tmpDir, modelName, modelId, modelDescription, version,
							 project.m_fmiDescription.m_inputVariables,
							 project.m_fmiDescription.m_outputVariables);


	// generate temporary directories for libraries

	QString installDir = SVSettings::instance().m_installDir;
	QString libraryDir = tmpDir + "/binaries";

	fmuDir.mkdir(libraryDir);

	// copy the binaries
	generateBinaries(installDir, libraryDir, "NandradSolverFMI", modelName, version);

	if (success) {

		// zip up the archive
		IBK::IBK_Message( IBK::FormatString("Compressing folder and creating FMU '%1'.\n").arg(fmuTargetFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		success = JlCompress::compressDir(fmuTargetFile, tmpDir);
		if (!success) {
			IBK::IBK_Message(IBK::FormatString("Error compressing the FMU archive (maybe invalid target path or "
												   "invalid characters used?)."), IBK::MSG_ERROR, FUNC_ID);
		}
	}

	// remove temporary directory structure
	QtExt::Directories::removeDirRecursively(tmpDir);
}


void SVCoSimCO2VentilationDialog::generateMasterSimFile(const SVCoSimCO2VentilationDialog::CO2ComfortVentilationProject & project,
														const QString &targetDir,
														const QString &modelName) const
{

	// create tsv files for all missing schedules

	// create a constant schedule for the first try
	std::set<QString> templateNames;
	for(const std::pair<unsigned int, QString> zoneTemplate : project.m_zoneTemplateNames) {

		const QString &templateName = zoneTemplate.second;
		// template was defined already
		if(std::find(templateNames.begin(), templateNames.end(), templateName) != templateNames.end())
			continue;
		// create schedule name
		QString scheduleName = "MaximumCO2Concentration_" + templateName;
		QString tsvFile = targetDir + "/" + modelName + ".fmus/" + scheduleName + ".tsv";

		// write file for maximum CO2 concentration
		QFile CO2file(tsvFile);
		CO2file.open(QIODevice::WriteOnly | QIODevice::Text);

		{
		// write file header
		QTextStream tsvDataCO2(&CO2file);

		// write header
		tsvDataCO2 << "Time [h]\tMaximumCO2Concentration [ppm]\n";

		// write file
		tsvDataCO2 << 0.0 << "\t" << 1300 << "\n";
		tsvDataCO2 << 8760 << "\t" << 1300 << "\n";
		}
		CO2file.close();
	}


	const char * const MAX_CO2_FILE_TEMPLATE =
			"simulator ${INDEX} 0 MaximumCO2Concentration_${TEMPLATE} #4682b4 \"${NAME}.fmus/MaximumCO2Concentration_${TEMPLATE}.tsv\"\n";

	const char * const CO2_SIMULATOR_TEMPLATE =
			"simulator ${INDEX} 0 CO2ComfortVentilation #a0522d \"${NAME}.fmus/CO2ComfortVentilation.fmu\"\n";

	const char * const NANDRAD_SIMULATOR_TEMPLATE =
			"simulator ${INDEX} 0 NANDRAD #a0522d \"${NAME}.fmus/NANDRAD.fmu\"\n";

	const char * const INPUT_CONNECTION_TEMPLATE =
			"graph NANDRAD.${NAME} CO2ComfortVentilation.${NAME}\n";

	const char * const OUTPUT_CONNECTION_TEMPLATE =
			"graph CO2ComfortVentilation.${NAME} NANDRAD.${NAME}\n";

	const char * const MAX_CO2_CONNECTION_TEMPLATE =
			"graph MaximumCO2Concentration_${TEMPLATE}.MaximumCO2Concentration CO2ComfortVentilation.${NAME} 0 0.000001\n";

	// start with global template
	IBK::Path fPath(":/templates/coSimulation.msim.template");
	QFile f(fPath.c_str());
	f.open(QFile::ReadOnly);
	QTextStream strm(&f);

	QString modelDesc = strm.readAll();

	// generate header

	// ${DATETIME} - 2018-08-01T12:49:19Z
	QDateTime t=QDateTime::currentDateTime();
	QString dt = t.toString(Qt::ISODate);
	modelDesc.replace("${DATETIME}", dt);

	// ${STARTTIME}
	QString startTimeStr = QString("%1 d").arg(project.m_simulationStartTime/3600./24.);
	modelDesc.replace("${STARTTIME}", startTimeStr);

	// ${ENDTIME}
	QString endTimeStr = QString("%1 d").arg(project.m_simulationEndTime/3600./24.);
	modelDesc.replace("${ENDTIME}", endTimeStr);


	// generate simulator section

	QString simulators;
	templateNames.clear();

	// add all tsv files
	int index = 0;
	for (std::map<unsigned int, QString>::const_iterator
		 templateIt = project.m_zoneTemplateNames.begin(); templateIt != project.m_zoneTemplateNames.end();
		 ++templateIt) {

		QString templateName = templateIt->second;
		// skip already registered template names
		if(templateNames.find(templateName) != templateNames.end())
			continue;

		QString tsvFile = MAX_CO2_FILE_TEMPLATE;
		tsvFile.replace("${INDEX}", QString("%1").arg(index++));
		tsvFile.replace("${TEMPLATE}", templateName);
		tsvFile.replace("${NAME}", modelName);
		simulators += tsvFile;

		// register template name
		templateNames.insert(templateName);
	}

	// add simulators
	QString co2simulator = CO2_SIMULATOR_TEMPLATE;
	co2simulator.replace("${INDEX}", QString("%1").arg(index++));
	co2simulator.replace("${NAME}", modelName);
	simulators += co2simulator;

	QString nandradSimulator = NANDRAD_SIMULATOR_TEMPLATE;
	nandradSimulator.replace("${INDEX}", QString("%1").arg(index++));
	nandradSimulator.replace("${NAME}", modelName);
	simulators += nandradSimulator;

	modelDesc.replace("${SIMULATORS}", simulators);


	// generate connection section

	QString connections;

	// connect all inputs for co2 balance
	for (const NANDRAD::FMIVariableDefinition &inputVar : project.m_inputVariables) {

		// try to retrieve variables without NANDRAD equivalent from tsv file
		if(inputVar.m_varName.empty()) {

			unsigned int roomId = inputVar.m_objectId;
			std::map<unsigned int, QString>::const_iterator templateName =
					project.m_zoneTemplateNames.find(roomId);
			IBK_ASSERT(templateName != project.m_zoneTemplateNames.end());

			// connect all tsv files
			if(inputVar.m_fmiVarName.find("MaximumCO2Concentration") != std::string::npos)
			{
				// fill line
				QString connection = MAX_CO2_CONNECTION_TEMPLATE;
				connection.replace("${NAME}", QString::fromStdString(inputVar.m_fmiVarName));
				connection.replace("${TEMPLATE}", templateName->second);
				connections += connection;
			}
			else
				continue;
		}
		else {
			QString connection = INPUT_CONNECTION_TEMPLATE;
			connection.replace("${NAME}", QString::fromStdString(inputVar.m_fmiVarName));
			connections += connection;
		}
	}
	// connect all inputs for nandrad
	for (const NANDRAD::FMIVariableDefinition &outputVar : project.m_outputVariables) {
		// skip variables without NANDRAD equivalent
		if(outputVar.m_varName.empty())
			continue;
		else {
			QString connection = OUTPUT_CONNECTION_TEMPLATE;
			connection.replace("${NAME}", QString::fromStdString(outputVar.m_fmiVarName));
			connections += connection;
		}
	}

	modelDesc.replace("${CONNECTIONS}", connections);

	QFile of(targetDir + "/" + modelName + ".msim");
	of.open(QFile::WriteOnly);
	of.write(modelDesc.toUtf8());
	of.close();
}


void SVCoSimCO2VentilationDialog::generateBinaries(const QString & sourceDir,
												   const QString & targetDir,
												   const QString & sourceName,
												   const QString & targetName,
												   const QString & version) const
{
	FUNCID(SVCoSimCO2VentilationDialog::generateBinaries);
	// copy the binaries

	QDir fmuDir(targetDir);

	// linux
	QString fmuLibFile;

#ifdef IBK_DEBUG
	if(!version.isEmpty()) {
		fmuLibFile = sourceDir + "/lib" + sourceName + ".so" + "." + version;
	}
	else {
		fmuLibFile = sourceDir + "/lib" + sourceName + ".so";
	}
#else
	fmuLibFile = sourceDir + "/lib" + sourceName + ".so";
#endif
	if (QFile(fmuLibFile).exists()) {
		fmuDir.mkdir(targetDir + "/linux64");
		QString targetPath = targetDir + "/linux64/" + targetName + ".so";
		IBK::IBK_Message( IBK::FormatString("Copying Linux FMU lib '%1' to '%2'\n").arg(fmuLibFile.toStdString()).arg(targetPath.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QFile::copy(fmuLibFile, fmuDir.absoluteFilePath(targetPath));
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (linux64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// macos
	fmuLibFile = sourceDir + "/lib" + sourceName + ".dylib";
	if (QFile(fmuLibFile).exists()) {
		fmuDir.mkdir(targetDir + "/darwin64");
		QString targetPath = targetDir + "/darwin64/" + targetName + ".dylib";
		IBK::IBK_Message( IBK::FormatString("Copying MacOS FMU lib '%1' to '%2'\n").arg(fmuLibFile.toStdString()).arg(targetPath.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QFile::copy(fmuLibFile, fmuDir.absoluteFilePath(targetPath));
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (darwin64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

	// win64
	fmuLibFile = sourceDir + "/" + sourceName + ".dll";

	if (QFile(fmuLibFile).exists()) {
		IBK::IBK_Message( IBK::FormatString("Copying Win64 FMU lib '%1'\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		fmuDir.mkdir(targetDir + "/win64");
		QString targetPath = targetDir + "/win64/" + targetName + ".dll";
		QString binTargetPath = fmuDir.absoluteFilePath(targetDir + "/win64/");
		QFile::copy(fmuLibFile, fmuDir.absoluteFilePath(targetPath));

		QStringList copyFiles;
		copyFiles << "msvcp140.dll"
				  << "vcomp140.dll"
				  << "vcruntime140.dll";
		for (int i=0; i<copyFiles.count(); ++i) {
			if (!QFile::exists(copyFiles[i])) {
				IBK::IBK_Message( IBK::FormatString("Missing file '%1' to copy into FMU archive.\n").arg(copyFiles[i].toStdString()),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			}
			else {
				QFile::copy(sourceDir + "/" + copyFiles[i], binTargetPath + "/" + QFileInfo(copyFiles[i]).fileName());
				IBK::IBK_Message( IBK::FormatString("Copying '%1' into FMU archive\n").arg(copyFiles[i].toStdString()),
								  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
			}
		}
	}
	else {
		IBK::IBK_Message( IBK::FormatString("FMU lib file (Win64) '%1' not installed.\n").arg(fmuLibFile.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	}

}

void SVCoSimCO2VentilationDialog::generateModelDescription(const QString & targetDir,
														   const QString & modelName,
														   const QString & modelId,
														   const QString & modelDescription,
														   const QString & version,
														   const std::vector<NANDRAD::FMIVariableDefinition> & inputVars,
														   const std::vector<NANDRAD::FMIVariableDefinition> & outputVars) const
{
	FUNCID(SVCoSimCO2VentilationDialog::generateModelDescription);

	// start with global template
	IBK::Path fPath(":/templates/modelDescription.xml.template");
	QFile f(fPath.c_str());
	f.open(QFile::ReadOnly);
	QTextStream strm(&f);

	QString modelDesc = strm.readAll();

	// ${MODELNAME}
	modelDesc.replace("${MODELNAME}", "\"" + modelName + "\"");

	// ${MODELNAME}
	modelDesc.replace("${DESCRIPTION}", "\"" + modelDescription + "\"");

	// ${GUID}
	modelDesc.replace("${GUID}", "\"{" + modelId + "}\"");

	// ${VERSION}
	modelDesc.replace("${VERSION}", "\"" + version + "\"");

	// ${DATETIME} - 2018-08-01T12:49:19Z
	QDateTime t=QDateTime::currentDateTime();
	QString dt = t.toString(Qt::ISODate);
	modelDesc.replace("${DATETIME}", dt);

	// generate variable and modelStructure section
	QString modelVariables;
	QString modelStructure;

	const char * const INPUT_VAR_TEMPLATE =
			"		<!-- Index of variable = \"${INDEX}\" -->\n"
			"		<ScalarVariable\n"
			"			name=\"${NAME}\"\n"
			"			valueReference=\"${VALUEREF}\"\n"
			"			variability=\"continuous\"\n"
			"			causality=\"input\">\n"
			"			<Real start=\"${STARTVALUE}\" unit=\"${REALVARUNIT}\"/>\n"
			"		</ScalarVariable>\n"
			"\n";

	const char * const OUTPUT_VAR_TEMPLATE =
			"		<!-- Index of variable = \"${INDEX}\" -->\n"
			"		<ScalarVariable\n"
			"			name=\"${NAME}\"\n"
			"			valueReference=\"${VALUEREF}\"\n"
			"			variability=\"continuous\"\n"
			"			causality=\"output\"\n"
			"			initial=\"calculated\">\n"
			"			<Real unit=\"${REALVARUNIT}\"/>\n"
			"		</ScalarVariable>\n"
			"\n";

	// process all variables
	QSet<QString> units;

	// write CO2 model decription file: inputs for NANDRAD are outputs for CO2 ventilation FMU
	int index=1;
	for (std::vector<NANDRAD::FMIVariableDefinition>::const_iterator
		 varIt = inputVars.begin(); varIt != inputVars.end();
		 ++varIt, ++index)
	{
		const NANDRAD::FMIVariableDefinition & varDef = *varIt;
		QString varDesc;
		varDesc = INPUT_VAR_TEMPLATE;
		varDesc.replace("${INDEX}", QString("%1").arg(index));
		varDesc.replace("${NAME}", varDef.m_fmiVarName.c_str());
		varDesc.replace("${VALUEREF}", QString("%1").arg(varDef.m_fmiValueRef));
		varDesc.replace("${STARTVALUE}", QString::number(varDef.m_fmiStartValue));
		varDesc.replace("${REALVARUNIT}", varDef.m_unit.c_str());
		modelVariables += varDesc;
		units.insert(varDef.m_unit.c_str());
	}

	for (std::vector<NANDRAD::FMIVariableDefinition>::const_iterator
		 varIt = outputVars.begin(); varIt != outputVars.end();
		 ++varIt, ++index)
	{
		const NANDRAD::FMIVariableDefinition & varDef = *varIt;
		QString varDesc;
		varDesc = OUTPUT_VAR_TEMPLATE;
		varDesc.replace("${INDEX}", QString("%1").arg(index));
		varDesc.replace("${NAME}", varDef.m_fmiVarName.c_str());
		varDesc.replace("${VALUEREF}", QString("%1").arg(varDef.m_fmiValueRef));
		varDesc.replace("${STARTVALUE}", QString::number(varDef.m_fmiStartValue));
		varDesc.replace("${REALVARUNIT}", varDef.m_unit.c_str());
		modelVariables += varDesc;
		modelStructure += QString(" 			<Unknown index=\"%1\"/>\n").arg(index);
		units.insert(varDef.m_unit.c_str());
	}

	// ${MODELVARIABLES}
	modelDesc.replace("${MODELVARIABLES}", modelVariables);

	// compose unit definitions section
	//		<UnitDefinitions>
	//			<Unit name="C"/>
	//			<Unit name="W/m2"/>
	//		</UnitDefinitions>

	QString unitDefs;
	if (!units.isEmpty()) {
		unitDefs += "	<UnitDefinitions>\n";
		for (QString u : units) {
			unitDefs += "		<Unit name=\"" + u + "\"/>\n";
		}
		unitDefs += "	</UnitDefinitions>\n";
	}

	modelDesc.replace("${UNIT_DEFINITIONS}", unitDefs);

	// ${MODEL_STRUCTURE_OUTPUTS} -
	// 			<Unknown index="1"/>
	//			<Unknown index="2"/>
	modelDesc.replace("${MODEL_STRUCTURE_OUTPUTS}", modelStructure);

	// finally write the file
	IBK::IBK_Message( IBK::FormatString("Creating '<fmu>/modelDescription.xml'\n"),
					  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	QFile of(targetDir + "/modelDescription.xml");
	of.open(QFile::WriteOnly);
	of.write(modelDesc.toUtf8());
	of.close();

}


void SVCoSimCO2VentilationDialog::on_pushButtonGenerate_clicked() {
	FUNCID(SVCoSimCO2VentilationDialog::on_pushButtonGenerate_clicked);

	// Note: content checks have been made already in checkProjectData()
	m_ui->plainTextEdit->setEnabled(true);

	// generate file paths
	QString vicusProjectFile = SVProjectHandler::instance().projectFile();  //  -> "/path/to/project.vicus"
	QFileInfo finfo(vicusProjectFile);
	// parent path
	QString parentPath = finfo.dir().absolutePath();	//  -> "/path/to"
	QString baseName = finfo.baseName();				//  -> "project"

	// get overall base dir
	QString basePath = parentPath + "/" + baseName;		//  -> "/path/to/project"
	QDir baseDir(basePath);

	// remove generation directory if existing
	if (baseDir.exists()) {
		IBK::IBK_Message("Removing existing FMU export directory.", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		QtExt::Directories::removeDirRecursively(baseDir.absolutePath());
	}

	// first create base directory
	baseDir.mkdir(baseDir.path());
	baseDir.mkdir(baseDir.path()+".fmus");

	m_co2FMUBaseDir = basePath + ".fmus/CO2ComfortVentilation";
	m_co2FMUFilePath = basePath + ".fmus/CO2ComfortVentilation.fmu";
	m_nandradFMUBaseDir = basePath + ".fmus/NANDRAD";
	m_nandradFMUFilePath = basePath + ".fmus/NANDRAD.fmu";
	m_msimProjectFilePath = basePath + ".msim";

	// create fmu directory
	QDir fmuDir(m_co2FMUBaseDir);
	fmuDir.mkdir(fmuDir.path());

	QString resourceDir = m_co2FMUBaseDir + "/resources";
	fmuDir.mkdir(resourceDir);


	// store access to VICUS project file
	const VICUS::Project &project = SVProjectHandler::instance().project();

	// generate CO2Balance project:
	CO2ComfortVentilationProject co2BalanceProject;

	co2BalanceProject.init(project);

	// write CO2Balance project file
	co2BalanceProject.write(resourceDir);

	// ModelDescripton.xml
	QString modelName("CO2ComfortVentilation");
	QString modelId("17d077d8-59b4-11ec-8093-3ce1a14c97e0");
	QString modelDescription("Model for combined ventilation control of CO2 and air temperature");
	QString version("1.0.0");

	// generate model description inside FMU base directory
	generateModelDescription(m_co2FMUBaseDir, modelName, modelId, modelDescription, version,
							 co2BalanceProject.m_inputVariables, co2BalanceProject.m_outputVariables);


	QString installDir = SVSettings::instance().m_installDir;
	QString binariesDir = m_co2FMUBaseDir + "/binaries";

	bool success = true;

	// create CO2ComfortVentilation.fmu
	fmuDir.mkdir("binaries");
	generateBinaries(installDir, binariesDir, "CO2ComfortVentilation", "CO2ComfortVentilation", version);

	if (success) {

		// zip up the archive
		IBK::IBK_Message( IBK::FormatString("Compressing folder and creating FMU '%1'.\n").arg(m_co2FMUFilePath.toStdString()),
						  IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
		success = JlCompress::compressDir(m_co2FMUFilePath, m_co2FMUBaseDir);
		if (!success) {
			IBK::IBK_Message(IBK::FormatString("Error compressing the FMU archive (maybe invalid target path or "
												   "invalid characters used?)."), IBK::MSG_ERROR, FUNC_ID);
		}
	}

	// remove temporary directory structure
	QtExt::Directories::removeDirRecursively(m_co2FMUBaseDir);

	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg(m_co2FMUFilePath));

	// NANDRAD FMU

	// generate input and output vars
	std::vector<NANDRAD::FMIVariableDefinition> fmiInputVars;
	std::vector<NANDRAD::FMIVariableDefinition> fmiOutputVars;

	// input variables for CO2 fmu are potential output variables for NANDRAD fmu
	for(const NANDRAD::FMIVariableDefinition &inputVar : co2BalanceProject.m_outputVariables) {
		// skip variables without NANDRAD equivalent
		if(inputVar.m_varName.empty())
			continue;

		fmiInputVars.push_back(inputVar);
		// set causality
		fmiInputVars.back().m_fmiTypeName = "input";
	}
	// input variables for CO2 fmu are potential output variables for NANDRAD fmu
	for(const NANDRAD::FMIVariableDefinition &outputVar : co2BalanceProject.m_inputVariables) {
		// skip variables without NANDRAD equivalent
		if(outputVar.m_varName.empty())
			continue;

		fmiOutputVars.push_back(outputVar);
		// set causality
		fmiOutputVars.back().m_fmiTypeName = "output";
	}



	// generate NANDRAD FMU

	resourceDir = m_nandradFMUBaseDir + "/resources";
	IBK::Path resDir(resourceDir.toStdString());
	if(!resDir.exists()){
		IBK::Path::makePath(resDir);
	}
	QString nandradProjectFile = resourceDir + "/Project.nandrad";
	QStringList errorStack;

	NANDRAD::Project nandradProject;
	try {
		// set placeholders in NANDRAD Project (VICUS-Project does not have placeholders)
		nandradProject.m_placeholders[VICUS::DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::databasesDir()).toStdString());
		nandradProject.m_placeholders[VICUS::USER_DATABASE_PLACEHOLDER_NAME] = IBK::Path((QtExt::Directories::userDataDir()).toStdString());
		// "Project Directory" placeholder is needed to resolve paths to files referenced via relative paths
		nandradProject.m_placeholders["Project Directory"] = parentPath.toStdString();

		project.generateNandradProject(nandradProject, errorStack, nandradProjectFile.toStdString());
	}
	catch (IBK::Exception & ex) {
		// just show a generic error message
		ex.writeMsgStackToError();
		QMessageBox box(this);
		QString fullText = errorStack.join("\n");
		box.setDetailedText(fullText);
		box.setIcon(QMessageBox::Critical);
		box.setText(tr("An error occurred during NANDRAD project generation (see details below)."));
		box.setWindowTitle(tr("NANDRAD Project Generation Error"));
		box.exec();
		return;
	}


	// generate fmu description
	modelName = baseName;

	NANDRAD::FMIDescription fmiDesc;
	fmiDesc.m_modelName = modelName.toStdString();
	fmiDesc.m_inputVariables = fmiInputVars;
	fmiDesc.m_outputVariables = fmiOutputVars;

	nandradProject.m_fmiDescription = fmiDesc;

	generateNandradFMU(nandradProject, modelName, m_nandradFMUBaseDir, m_nandradFMUFilePath);

	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg(m_nandradFMUFilePath));


	// generate MSIM-Project in base path

	IBK::Path parentDir = IBK::Path(vicusProjectFile.toStdString()).parentPath();
	generateMasterSimFile(co2BalanceProject, QString(parentDir.c_str()), modelName);
	m_ui->plainTextEdit->appendPlainText(tr("Generated %1").arg(m_msimProjectFilePath));


	// all successful, enable "Launch MasterSim" button

	m_ui->pushButtonLaunchMasterSim->setEnabled(true);
}


void SVCoSimCO2VentilationDialog::on_pushButtonLaunchMasterSim_clicked() {
	// check if we have MasterSim path configured
	QString masterSimPath = SVSettings::instance().m_masterSimExecutable;

	QFileInfo info(masterSimPath);

#ifdef Q_OS_LINUX
	if(!masterSimPath.isEmpty()) {
		//search file through all pathes
		if(!info.exists() && info.path()==".") {
			// retrieve file path using linux 'which' commmanf
			std::string cmd = "which " + masterSimPath.toStdString();

			char buffer[128];
			std::string result;
			// use pipe for executing command and getting feedback
			FILE* pipe = popen(cmd.c_str(), "r");

			if (!pipe) {
				QMessageBox::critical(this, tr("Error starting external application"), tr("MASTERSIM path '%1' could not be found.")
									  .arg(masterSimPath));
			}
			try {
				while (fgets(buffer, sizeof buffer, pipe) != nullptr)
					result += buffer;
			}
			catch (...) {
				pclose(pipe);
				QMessageBox::critical(this, tr("Error starting external application"), tr("MASTERSIM path '%1' could not be found.")
									  .arg(masterSimPath));
				return;
			}
			pclose(pipe);

			masterSimPath = QString::fromStdString(result).simplified();
			info.setFile(masterSimPath);
		}
	}
#endif

	if (masterSimPath.isEmpty() || !info.exists())
	{
		QMessageBox::information(this, tr("Setup external tool"), tr("Please select first the path to "
																  "MASTERSIM in the preferences dialog!"));

		SVMainWindow::instance().preferencesDialog()->edit(0);
		// TODO Andreas, find a way to show a mode-less window within a dialog's event loop
		QEventLoop loop;
		connect(SVMainWindow::instance().preferencesDialog(), &SVPreferencesDialog::closed, &loop, &QEventLoop::quit);
		loop.exec();

		// ensure that preferences dialog is in foreground
//		stackUnder(SVMainWindow::instance().preferencesDialog());

		masterSimPath = SVSettings::instance().m_masterSimExecutable;

		// still no valid path?
		if (masterSimPath.isEmpty() || !QFileInfo::exists(masterSimPath))
		return;
	}

	// launch MasterSim - run option is only needed for linux, and otherwise it will always be -1
#if defined(Q_OS_LINUX)
	SVSettings::TerminalEmulators runOption = SVSettings::instance().m_terminalEmulator;
#else
	SVSettings::TerminalEmulators runOption = (SVSettings::TerminalEmulators)-1;
#endif

	bool res = SVSettings::startProcess(masterSimPath, QStringList(), m_msimProjectFilePath, runOption);
	if (!res) {
		QMessageBox::critical(this, tr("Error starting external application"), tr("MASTERSIM '%1' could not be started.")
							  .arg(masterSimPath));
	}
}
