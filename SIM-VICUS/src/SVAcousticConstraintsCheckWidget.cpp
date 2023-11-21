#include "SVAcousticConstraintsCheckWidget.h"
#include "ui_SVAcousticConstraintsCheckWidget.h"

#include "SVStyle.h"
#include "SVProjectHandler.h"
#include "QtExt_Conversions.h"
#include "SVUndoTreeNodeState.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "Vic3DSceneView.h"

#include <VICUS_Project.h>

SVAcousticConstraintsCheckWidget::SVAcousticConstraintsCheckWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVAcousticConstraintsCheckWidget)
{
	m_ui->setupUi(this);

	// set up table widgets

	// for walls
	m_ui->tableWidgetAirBorneSound->setColumnCount(8);
	m_ui->tableWidgetAirBorneSound->setHorizontalHeaderLabels(QStringList()
													  << tr("Acoustic template A")
													  << tr("Acoustic Template B")
													  << tr("Acoustic component")
													  << tr("Actual air sound value")
													  << tr("Normal constraints")
													  << tr("Advanced constraints")
													  << tr("Structure type")
													  << "");
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetAirBorneSound);
	m_ui->tableWidgetAirBorneSound->setSortingEnabled(false);
	m_ui->tableWidgetAirBorneSound->resizeColumnsToContents();


	//for ceilings
	m_ui->tableWidgetImpactSound->setColumnCount(8);
	m_ui->tableWidgetImpactSound->setHorizontalHeaderLabels(QStringList()
														 << tr("Acoustic template above")
														 << tr("Acoustic Template beneth")
														 << tr("Acoustic component")
														 << tr("Actual impact sound value")
														 << tr("Normal constraints")
														 << tr("Advanced constraints")
														 << tr("Structure type")
														 << "");

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetImpactSound);

	m_ui->tableWidgetImpactSound->setSortingEnabled(false);
	m_ui->tableWidgetReverberation->setSortingEnabled(false);
	m_ui->tableWidgetImpactSound->resizeColumnsToContents();

//	double width = 130;
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_AcousticTemplateB, 200);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_AcousticComponent, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_ActualAirSoundValue, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_AdvancedConstraints, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_NormalConstraints, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_SameStructure, width);
//	m_ui->tableWidgetImpactSound->setColumnWidth(CSP_SelectButton, 230);

//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_AcousticTemplateB, 200);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_AcousticComponent, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_ActualAirSoundValue, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_AdvancedConstraints, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_NormalConstraints, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_SameStructure, width);
//	m_ui->tableWidgetAirBorneSound->setColumnWidth(CSP_SelectButton, 230);

//	QHeaderView *headerView = m_ui->tableWidgetImpactSound->horizontalHeader();
//	headerView->setSectionResizeMode(CSP_AcousticTemplateA, QHeaderView::Stretch);

//	headerView = m_ui->tableWidgetAirBorneSound->horizontalHeader();
//	headerView->setSectionResizeMode(CSP_AcousticTemplateA, QHeaderView::Stretch);

//	m_ui->tableWidgetAirBorneSound->horizontalHeaderItem(CSP_AcousticTemplateA)->setTextAlignment(Qt::AlignLeft);
//	m_ui->tableWidgetAirBorneSound->horizontalHeaderItem(CSP_AcousticTemplateB)->setTextAlignment(Qt::AlignLeft);

//	m_ui->tableWidgetImpactSound->horizontalHeaderItem(CSP_AcousticTemplateA)->setTextAlignment(Qt::AlignLeft);
//	m_ui->tableWidgetImpactSound->horizontalHeaderItem(CSP_AcousticTemplateB)->setTextAlignment(Qt::AlignLeft);

	m_ui->checkBoxHideImpactSound->blockSignals(false);
	m_ui->checkBoxHideAirBourneSound->blockSignals(false);

	m_ui->checkBoxHideAirBourneSound->setChecked(false);
	m_ui->checkBoxHideImpactSound->setChecked(false);

	m_ui->checkBoxHideImpactSound->blockSignals(true);
	m_ui->checkBoxHideAirBourneSound->blockSignals(true);

	resize(1500, 800);

	m_ui->tableWidgetReverberation->setColumnCount(NUM_CRT);

	QStringList headers;
	headers << "Room ID"
			<< "Room name"
			<< "Acoustic template"
			<< "Reverberation goal [s]"
			<< "125 Hz [s]"
			<< "250 Hz [s]"
			<< "500 Hz [s]"
			<< "1000 Hz [s]"
			<< "2000 Hz [s]"
			<< "4000 Hz [s]";

	m_ui->tableWidgetReverberation->setHorizontalHeaderLabels(headers);
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetReverberation);
	setWindowTitle(tr("Acoustic component check"));

	m_ui->tabWidget->setEnabled(false);
}


SVAcousticConstraintsCheckWidget::~SVAcousticConstraintsCheckWidget() {
	delete m_ui;
}


void SVAcousticConstraintsCheckWidget::showSurfaces(unsigned int surfaceAId, unsigned int surfaceBId) {
	std::set<unsigned int> surfIds;
	surfIds.insert(surfaceAId);
	surfIds.insert(surfaceBId);

	std::set<const VICUS::Object *> allObjects;
	project().selectObjects(allObjects, VICUS::Project::SG_All, false, true);
	std::set<unsigned int> selectedObjectIDs;
	for (const VICUS::Object * o : allObjects) {
		if (o->m_selected)
			selectedObjectIDs.insert(o->m_id);
	}

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("De-selecting objects"), SVUndoTreeNodeState::SelectedState, selectedObjectIDs, false);
	undo->push();
	undo = new SVUndoTreeNodeState(tr("Selecting objects"), SVUndoTreeNodeState::SelectedState, surfIds, true);
	undo->push();

	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_FindSelection);
}


void SVAcousticConstraintsCheckWidget::checkConstraints() {

	// clear all the table entries for walls and doors
	m_airBourneSoundTableEntries.clear();

	// clear all the table entries for ceilings
	m_impactSoundTableEntries.clear();

	// iterate over all component instances
	for(const VICUS::ComponentInstance & ci : project().m_componentInstances){

		// skip all that dont have two surfaces
		if(ci.m_idSideASurface == VICUS::INVALID_ID || ci.m_idSideBSurface == VICUS::INVALID_ID ||
				ci.m_sideASurface == nullptr || ci.m_sideBSurface == nullptr)
			continue;

		// search the rooms of the corresponding surfaces
		const VICUS::Room * roomA = dynamic_cast<VICUS::Room *>(ci.m_sideASurface->m_parent);
		const VICUS::Room * roomB = dynamic_cast<VICUS::Room *>(ci.m_sideBSurface->m_parent);
		if(roomA == nullptr || roomB == nullptr)
			continue;

		// skip if they dont have the same acoustic building type
		if(roomA->m_idAcousticBuildingType != roomB->m_idAcousticBuildingType)
			continue;

		// check if they are in the same structural unit or not

		// get the ids of the acoustic template of the corresponding room
		unsigned int acousticTemplateAId = roomA->m_idSoundProtectionTemplate;
		unsigned int acousticTemplateBId = roomB->m_idSoundProtectionTemplate;

		// skip if one of them does not have an acoustic template
		if(acousticTemplateAId == VICUS::INVALID_ID || acousticTemplateBId == VICUS::INVALID_ID)
			continue;

		// struct that holds relevant data for the table
		//one for walls
		tableEntry airBourneSoundEntry;
		//one for ceilings
		tableEntry impactSoundEntry;

		// initialize with no constraints specified, might be overwritten later
		airBourneSoundEntry.m_basicConstraintViolated = VI_NoConstraint;
		airBourneSoundEntry.m_advancedConstraintViolated = VI_NoConstraint;
		impactSoundEntry.m_basicConstraintViolated = VI_NoConstraint;
		impactSoundEntry.m_advancedConstraintViolated = VI_NoConstraint;

		// Reference to db
		const SVDatabase & db = SVSettings::instance().m_db;

		// fill table Entry with acoustic template info
		// check the part type, if its a ceiling
		const VICUS::Component * comp = db.m_components[ci.m_idComponent];
		bool isCeiling = comp->m_type == VICUS::Component::CT_Ceiling;

		// potentially swap templates, as direction is important if it's a ceiling
		if(isCeiling){
			double zA = ci.m_sideASurface->geometry().centerPoint().m_z;
			double zB = ci.m_sideBSurface->geometry().centerPoint().m_z;
			if(zB > zA){
				// now the direction between the 2 surfaces is relevant
				// check which surface is on top
				// reorder so A is on top if B
				unsigned int tmp = acousticTemplateAId;
				acousticTemplateAId = acousticTemplateBId;
				acousticTemplateBId = tmp;
			}
		}

		// iterate over the acoustic reference components and find the one for these surfaces

		// if we find a match we save the teWall to the vector in the end
		bool addTableEntryAirSoundToVector = false;
		bool addTableEntryImpactSoundToVector = false;

		for(std::pair<unsigned int, VICUS::AcousticReferenceComponent> refEntry : db.m_acousticReferenceComponents){
			const VICUS::AcousticReferenceComponent &refComp = refEntry.second;

			// alle elemente rausfiltern wo:
			// Komponente =  Decke und die Ref.komp. keine Decke ist
			// Komponente =  Wand | Tür | Treppe und die Ref.komp. eine Decke ist
			if((isCeiling && refComp.m_type != VICUS::AcousticReferenceComponent::CT_Ceiling) ||
					(!isCeiling && refComp.m_type == VICUS::AcousticReferenceComponent::CT_Ceiling))
				continue;

			// skip wrong templates of ref component
			if(refComp.m_idAcousticTemplateA != acousticTemplateAId ||
					refComp.m_idAcousticTemplateB != acousticTemplateBId)
				continue;

			if ( roomA->m_idAcousticBuildingType != refComp.m_buildingType ||
				 roomB->m_idAcousticBuildingType != refComp.m_buildingType )
				continue;

			// is true, if the direction between the templates match
			bool impactSoundTemplatesMatch = false;

			// ceiling and ref comp. also is ceiling
			if(isCeiling && refComp.m_type == VICUS::AcousticReferenceComponent::CT_Ceiling)
				impactSoundTemplatesMatch = true;


			// add the walls template
			//we have a match for walls, so fill in information for table widget struct
			const VICUS::AcousticSoundProtectionTemplate * tempA = db.m_acousticSoundProtectionTemplates[acousticTemplateAId];
			const VICUS::AcousticSoundProtectionTemplate * tempB = db.m_acousticSoundProtectionTemplates[acousticTemplateBId];

			Q_ASSERT(tempA != nullptr);
			Q_ASSERT(tempA != nullptr);

			airBourneSoundEntry.m_acousticTemplateAInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(tempA->m_displayName)).arg(tempA->m_id);
			airBourneSoundEntry.m_acousticTemplateBInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(tempB->m_displayName)).arg(tempB->m_id);

			// save the surface ids
			airBourneSoundEntry.m_surfaceAId = ci.m_idSideASurface;
			airBourneSoundEntry.m_surfaceBId = ci.m_idSideBSurface;
			airBourneSoundEntry.m_isSameStructuralUnit = (roomA->m_structuralUnit != nullptr && roomB->m_structuralUnit != nullptr &&
					roomA->m_structuralUnit->m_id == roomB->m_structuralUnit->m_id);

			// this entry should be saved in the vector
			addTableEntryAirSoundToVector = true;

			// retrieve the limits from the acoustic reference Component
			double impactSoundLimit;
			double airSoundLimit;
			if(airBourneSoundEntry.m_isSameStructuralUnit){
				airSoundLimit = refComp.m_airborneSoundOneStructureUnit;
				impactSoundLimit = refComp.m_impactSoundOneStructureUnit;
			}
			else {
				airSoundLimit = refComp.m_airborneSoundDifferentStructureUnit;
				impactSoundLimit = refComp.m_impactSoundDifferentStructureUnit;
			}

			const VICUS::Construction *con = db.m_constructions[comp->m_idConstruction];
			if (con == nullptr)
				continue;

			// retrieve the acoustic component to check if it fullfills the limits
			double airRes = con->m_acousticPara[VICUS::Construction::P_AirSoundResistanceValue].value;
			airBourneSoundEntry.m_actualValue = QString("%1 dB").arg(airRes);

			// fill te struct with acoustic component info
			airBourneSoundEntry.m_acousticComponentInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(comp->m_displayName)).arg(comp->m_id);

			// check wether a limit is provided
			if(IBK::near_equal(airSoundLimit, -1)){
				// no limit is provided, set to not violated
				if (refComp.m_requirementType == VICUS::AcousticReferenceComponent::RT_Basic) {
					airBourneSoundEntry.m_basicConstraintViolated = VI_Valid;
					airBourneSoundEntry.m_expectedNormalLimit = tr("No limit");
				}
				else {
					airBourneSoundEntry.m_advancedConstraintViolated = VI_Valid;
					airBourneSoundEntry.m_expectedAdvancedLimit = tr("No limit");
				}
			} else {
				// there is a limit
				// enter the results of the airborne sound check, depending on the requirement type
				if (refComp.m_requirementType == VICUS::AcousticReferenceComponent::RT_Basic) {

					if(airRes >= airSoundLimit)
						airBourneSoundEntry.m_basicConstraintViolated = VI_Valid;
					else {
						airBourneSoundEntry.m_basicConstraintViolated = VI_Invalid;
					}
					airBourneSoundEntry.m_expectedNormalLimit = QString("%1 dB").arg(airSoundLimit);
				}
				else {
					if(airRes >= airSoundLimit)
						airBourneSoundEntry.m_advancedConstraintViolated = VI_Valid;
					else {
						airBourneSoundEntry.m_advancedConstraintViolated = VI_Invalid;
					}
					airBourneSoundEntry.m_expectedAdvancedLimit = QString("%1 dB").arg(airSoundLimit);
				}
			}
			// check if there war another match for the impact sound (ceiling)
			if(impactSoundTemplatesMatch){

				// we have a match for ceilings, we copy identical data from the wall table entry
				impactSoundEntry.m_acousticTemplateAInfo = airBourneSoundEntry.m_acousticTemplateAInfo;
				impactSoundEntry.m_acousticTemplateBInfo = airBourneSoundEntry.m_acousticTemplateBInfo;
				impactSoundEntry.m_surfaceAId = airBourneSoundEntry.m_surfaceAId;
				impactSoundEntry.m_surfaceBId = airBourneSoundEntry.m_surfaceBId;
				impactSoundEntry.m_isSameStructuralUnit = airBourneSoundEntry.m_isSameStructuralUnit;
				impactSoundEntry.m_acousticComponentInfo = airBourneSoundEntry.m_acousticComponentInfo;

				double impact = con->m_acousticPara[VICUS::Construction::P_ImpactSoundValue].value;
				impactSoundEntry.m_actualValue = QString("%1 dB").arg(impact);

				// this entry should be saved in the vector
				addTableEntryImpactSoundToVector = true;

				// check wether a limit is provided
				if(IBK::near_equal(impactSoundLimit, -1)){
					// no limit is provided, set to not violated
					if(refComp.m_requirementType == VICUS::AcousticReferenceComponent::RT_Basic){
						impactSoundEntry.m_basicConstraintViolated = VI_Valid;
						impactSoundEntry.m_expectedNormalLimit = tr("No limit");
					}
					else {
						impactSoundEntry.m_advancedConstraintViolated = VI_Valid;
						impactSoundEntry.m_expectedAdvancedLimit = tr("No limit");
					}
				}
				else {
					// now we modify the teCeil struct in the same way as before with airSound
					if(refComp.m_requirementType == VICUS::AcousticReferenceComponent::RT_Basic){
						if(impact < impactSoundLimit)
							impactSoundEntry.m_basicConstraintViolated = VI_Valid;
						else {
							impactSoundEntry.m_basicConstraintViolated = VI_Invalid;
						}
						impactSoundEntry.m_expectedNormalLimit = QString("%1 dB").arg(impactSoundLimit);
					}
					else {
						if(impact < impactSoundLimit)
							impactSoundEntry.m_advancedConstraintViolated = VI_Valid;
						else {
							impactSoundEntry.m_advancedConstraintViolated = VI_Invalid;
						}
						impactSoundEntry.m_expectedAdvancedLimit = QString("%1 dB").arg(impactSoundLimit);
					}
				}
			}
		}


		if(addTableEntryAirSoundToVector)
			m_airBourneSoundTableEntries.push_back(airBourneSoundEntry);

		if(addTableEntryImpactSoundToVector)
			m_impactSoundTableEntries.push_back(impactSoundEntry);

	}

	updateTable();
	checkReverberation();
}

void SVAcousticConstraintsCheckWidget::checkReverberation() {
	const VICUS::Project &prj = project();
	const SVDatabase & db = SVSettings::instance().m_db;

	struct ReverberationResult {

		ReverberationResult() {}

		ReverberationResult(unsigned int roomID, const QString &roomName, const QString &templateName, double reverbTime,
							const std::vector<double> &reverberationTime,
							const std::vector<bool> &reverberationEvaluation) :
			m_idRoom(roomID),
			m_roomName(roomName),
			m_templateName(templateName),
			m_reverbTime(reverbTime),
			m_reverberationTime(reverberationTime),
			m_evaluation(reverberationEvaluation)
		{}

		// Room ID
		unsigned int				m_idRoom;
		// Room name
		QString						m_roomName;
		// Template name
		QString						m_templateName;
		// Template name
		double						m_reverbTime;
		// Reverberation times
		std::vector<double>			m_reverberationTime;
		// Evaluation
		std::vector<bool>			m_evaluation;
	};

	std::vector<ReverberationResult> tableEntries;

	for (const VICUS::Building &b : prj.m_buildings) {
		for (const VICUS::BuildingLevel &bl : b.m_buildingLevels) {
			for (const VICUS::Room &r : bl.m_rooms) {

				double roomVolume = r.m_para[VICUS::Room::P_Volume].value;

				if (r.m_idAcousticTemplate == VICUS::INVALID_ID)
					continue;

				const VICUS::AcousticTemplate *at = db.m_acousticTemplates[r.m_idAcousticTemplate];

				Q_ASSERT(at != nullptr);

				std::vector<int> frequencies				= {125, 250, 500, 1000, 2000, 4000};
				std::vector<double> reverberationSum		= {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
				std::vector<double> reverberationTime		= {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
				std::vector<bool> reverberationEvaluation	= {false, false, false, false, false, false};

				double totalArea = 0.0;
				for (const VICUS::Surface &s : r.m_surfaces) {
					double surfaceArea = s.areaWithoutSubsurfaces();

					const VICUS::ComponentInstance *ci = s.m_componentInstance;
					if (ci == nullptr)
						continue;

					const VICUS::Component *comp = db.m_components[ci->m_idComponent];
					if (comp == nullptr)
						continue;

					if (ci->m_idSideASurface == s.m_id) {
						const VICUS::AcousticBoundaryCondition *aBC = db.m_acousticBoundaryConditions[comp->m_idSideAAcousticBoundaryCondition];

						if (aBC == nullptr)
							continue;

						for (const VICUS::AcousticSoundAbsorptionPartition &layer : aBC->m_acousticSoundAbsorptionPartitions) {
							double areaFraction = layer.m_para[VICUS::AcousticSoundAbsorptionPartition::P_AreaFraction].value;
							const VICUS::AcousticSoundAbsorption *soundAbsorption = db.m_acousticSoundAbsorptions[layer.m_idSoundAbsorption];

							if (soundAbsorption == nullptr)
								continue;

							for (unsigned int i=0; i<VICUS::AcousticSoundAbsorption::NUM_SF; ++i) {
								reverberationSum[i] += surfaceArea * areaFraction * soundAbsorption->m_soundAbsorption[i];
							}

							totalArea += surfaceArea;
						}
					}

					if (ci->m_idSideBSurface == s.m_id) {
						const VICUS::AcousticBoundaryCondition *aBC = db.m_acousticBoundaryConditions[comp->m_idSideBAcousticBoundaryCondition];

						if (aBC == nullptr)
							continue;

						for (const VICUS::AcousticSoundAbsorptionPartition &layer : aBC->m_acousticSoundAbsorptionPartitions) {
							double areaFraction = layer.m_para[VICUS::AcousticSoundAbsorptionPartition::P_AreaFraction].value;
							const VICUS::AcousticSoundAbsorption *soundAbsorption = db.m_acousticSoundAbsorptions[layer.m_idSoundAbsorption];

							if (soundAbsorption == nullptr)
								continue;

							for (unsigned int i=0; i<VICUS::AcousticSoundAbsorption::NUM_SF; ++i) {
								reverberationSum[i] += surfaceArea * areaFraction * soundAbsorption->m_soundAbsorption[i];
							}

							totalArea += surfaceArea;
						}
					}
				}

				double reverbTimeGoal = at->m_evaluationOffset + at->m_evaluationFactor * std::log10(roomVolume);
				for (unsigned int i=0; i<VICUS::AcousticSoundAbsorption::NUM_SF; ++i) {
					double &reverbTime = reverberationTime[i];

					// reverberation time
					reverbTime = 0.163 * roomVolume / reverberationSum[i];

					// T_Soll in seconds
					double minVal = 0.01 * reverbTimeGoal * at->m_splinePara[VICUS::AcousticTemplate::SP_MinValue].m_values.value(frequencies[i]);
					double maxVal = 0.01 * reverbTimeGoal * at->m_splinePara[VICUS::AcousticTemplate::SP_MaxValue].m_values.value(frequencies[i]);

					// Evaluation
					reverberationEvaluation[i] = ( minVal < reverbTime ) && ( maxVal > reverbTime );
				}

				tableEntries.push_back(ReverberationResult (r.m_id, r.m_displayName, QString::fromStdString(at->m_displayName.encodedString()),
															reverbTimeGoal, reverberationTime, reverberationEvaluation));
			}
		}
	}

	m_ui->tableWidgetReverberation->setRowCount(tableEntries.size());

	bool checkReverberation = true;
	for (unsigned int i=0; i < tableEntries.size(); ++i) {

		const ReverberationResult &res = tableEntries[i];

		m_ui->tableWidgetReverberation->setItem(i, CRT_RoomID, new QTableWidgetItem(QString::number(res.m_idRoom)));
		m_ui->tableWidgetReverberation->setItem(i, CRT_RoomName, new QTableWidgetItem(res.m_roomName));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Template, new QTableWidgetItem(res.m_templateName));

		m_ui->tableWidgetReverberation->setItem(i, CRT_TGoal, new QTableWidgetItem(QString("%1").arg(res.m_reverbTime, 0, 'g', 3)));

		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb125Hz, new QTableWidgetItem(QString("%1").arg(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_125Hz], 0, 'g', 3)));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb250Hz, new QTableWidgetItem(QString("%1").arg(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_250Hz], 0, 'g', 3)));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb500Hz, new QTableWidgetItem(QString("%1").arg(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_500Hz], 0, 'g', 3)));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb1000Hz, new QTableWidgetItem(QString("%1").arg(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_1000Hz], 0, 'g', 3)));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb2000Hz, new QTableWidgetItem(QString("%1").arg(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_2000Hz], 0, 'g', 3)));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb4000Hz, new QTableWidgetItem(QString("%1").arg(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_4000Hz], 0, 'g', 3)));

		m_ui->tableWidgetReverberation->item(i, CRT_TGoal)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb125Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb250Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb500Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb1000Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb2000Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb4000Hz)->setTextAlignment(Qt::AlignCenter);

		m_ui->tableWidgetReverberation->item(i, CRT_Reverb125Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb250Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb500Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb1000Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb2000Hz)->setTextAlignment(Qt::AlignCenter);
		m_ui->tableWidgetReverberation->item(i, CRT_Reverb4000Hz)->setTextAlignment(Qt::AlignCenter);

		QColor valid(Qt::darkGreen);
		QColor invalid(Qt::darkRed);

		if (SVSettings::instance().m_theme == SVSettings::TT_White) {
			valid = valid.lighter(250);
			invalid = invalid.lighter(250);
		}

		for (unsigned int j = 0; j < NUM_CRT; ++j) {
			QTableWidgetItem *item = m_ui->tableWidgetReverberation->item(i, j);
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		}

		for (unsigned int j = 0; j < VICUS::AcousticSoundAbsorption::NUM_SF; ++j) {
			if (res.m_evaluation[j])
				m_ui->tableWidgetReverberation->item(i, CRT_Reverb125Hz + j)->setBackground(valid);
			else {
				m_ui->tableWidgetReverberation->item(i, CRT_Reverb125Hz + j)->setBackground(invalid);
				checkReverberation = false;
			}
		}
	}

	m_ui->labelReverberation->setText(checkReverberation ? "All acoustic checks passed succesfully." : "Acoustic checks not passed.");
	m_ui->labelReverberationIcon->setPixmap( QPixmap(checkReverberation ? ":/gfx/actions/16x16/ok.png" : ":/gfx/actions/16x16/error.png"));

	QColor positive(Qt::darkGreen);
	QColor negative(Qt::darkRed);

	positive = SVSettings::instance().m_theme == SVSettings::TT_Dark ? positive.lighter() : positive;
	negative = SVSettings::instance().m_theme == SVSettings::TT_Dark ? negative.lighter() : negative;

	m_ui->labelReverberation->setStyleSheet(QString("QLabel { color : %1; }").arg(checkReverberation ? positive.name() : negative.name()));

	m_ui->tableWidgetReverberation->resizeColumnsToContents();
}


void SVAcousticConstraintsCheckWidget::updateTable() {

	QTableWidget * tableWidget = nullptr;
	std::vector<tableEntry> * tes = nullptr;
	bool hideNonViolatedEntries = false;

	bool check[NUM_TT] = {true, true};

	// filling the table is identical, so only the tableWidget, the data & (hiding entries) changes during the iterations
	for(unsigned int i = 0; i < NUM_TT ; ++i){

		switch (i) {
		case TT_AirBourneSound:
			tableWidget = m_ui->tableWidgetAirBorneSound;
			tes = &m_airBourneSoundTableEntries;
			hideNonViolatedEntries = m_hideAirBourneSound;
			break;
		case TT_ImpactSound:
			tableWidget = m_ui->tableWidgetImpactSound;
			tes = &m_impactSoundTableEntries;
			hideNonViolatedEntries = m_hideImpactSound;
			break;

		}

		if(tableWidget == nullptr || tes == nullptr)
			return;

		// fill the walls table with the collected wall table entries
		tableWidget->blockSignals(true);
		tableWidget->setRowCount(0);
		int row=0;

		for (tableEntry &te : *tes){

			// if the hide walls selection box is check, skip all entries that dont have a violation
			if(hideNonViolatedEntries && te.m_basicConstraintViolated != ViolationInfo::VI_Invalid &&
					te.m_advancedConstraintViolated != ViolationInfo::VI_Invalid)
				continue;

			tableWidget->setRowCount(row + 1);

			QTableWidgetItem * item;
			item = new QTableWidgetItem();
			item->setText(te.m_acousticTemplateAInfo);
			item->setFlags(Qt::ItemIsEnabled);
			tableWidget->setItem(row, CSP_AcousticTemplateA, item);

			item = new QTableWidgetItem();
			item->setText(te.m_acousticTemplateBInfo);
			item->setFlags(Qt::ItemIsEnabled);
			tableWidget->setItem(row, CSP_AcousticTemplateB, item);

			item = new QTableWidgetItem();
			item->setText(te.m_acousticComponentInfo);
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled);
			tableWidget->setItem(row, CSP_AcousticComponent, item);

			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setText(te.m_actualValue);
			item->setFlags(Qt::ItemIsEnabled);
			tableWidget->setItem(row, CSP_ActualAirSoundValue, item);

			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setText(te.m_expectedNormalLimit);

			QColor valid(Qt::darkGreen);
			QColor invalid(Qt::darkRed);

			if (SVSettings::instance().m_theme == SVSettings::TT_White) {
				valid = valid.lighter(250);
				invalid = invalid.lighter(250);
			}

			// select color according to violation info
			switch (te.m_basicConstraintViolated) {
				case VI_Invalid:
					item->setBackground(invalid);
					check[i] = false;
				break;
				case VI_Valid:		item->setBackground(valid);		break;
				default: break;
			}

			item->setFlags(Qt::ItemIsEnabled);
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			tableWidget->setItem(row, CSP_NormalConstraints, item);

			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setText(te.m_expectedAdvancedLimit);

			// select color according to violation info
			switch (te.m_advancedConstraintViolated) {
				case VI_Invalid:
					item->setBackground(invalid);
					check[i] = false;
				break;
				case VI_Valid:		item->setBackground(valid);		break;
				default: break;
			}

			item->setFlags(Qt::ItemIsEnabled);
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			tableWidget->setItem(row, CSP_AdvancedConstraints, item);

			item = new QTableWidgetItem();
			item->setText(te.m_isSameStructuralUnit ? "same structure" : "different structure");
			item->setFlags(Qt::ItemIsEnabled);
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			tableWidget->setItem(row, CSP_SameStructure, item);

			QPushButton * btn = new QPushButton(tr("Select components in scene..."));

			unsigned int surfaceAId = te.m_surfaceAId;
			unsigned int surfaceBId = te.m_surfaceBId;

			// Connecting the clicked signal to a lambda function to pass the surface ids
			QObject::connect(btn, &QPushButton::clicked, [=]() {
				SVAcousticConstraintsCheckWidget::showSurfaces(surfaceAId, surfaceBId);
			});

			// table widget takes ownership of btn and memory will be freed on destruction
			tableWidget->setCellWidget(row, CSP_SelectButton, btn);

			tableWidget->resizeRowsToContents();

			++row;
		}

		tableWidget->blockSignals(false);
		tableWidget->resizeColumnsToContents();

	}

	m_ui->labelImpactSound->setText(check[TT_ImpactSound] ? "All acoustic checks passed succesfully." : "Acoustic checks not passed.");
	m_ui->labelImpactSoundIcon->setPixmap( QPixmap(check[TT_ImpactSound] ? ":/gfx/actions/16x16/ok.png" : ":/gfx/actions/16x16/error.png"));

	m_ui->labelAirBourneSound->setText(check[TT_AirBourneSound] ? "All acoustic checks passed succesfully." : "Acoustic checks not passed.");
	m_ui->labelAirBourneSoundIcon->setPixmap( QPixmap(check[TT_AirBourneSound] ? ":/gfx/actions/16x16/ok.png" : ":/gfx/actions/16x16/error.png"));

	QColor positive(Qt::darkGreen);
	QColor negative(Qt::darkRed);

	positive = SVSettings::instance().m_theme == SVSettings::TT_Dark ? positive.lighter() : positive;
	negative = SVSettings::instance().m_theme == SVSettings::TT_Dark ? negative.lighter() : negative;

	m_ui->labelImpactSound->setStyleSheet(QString("QLabel { color : %1; }").arg(check[TT_ImpactSound] ? positive.name() : negative.name()));
	m_ui->labelAirBourneSound->setStyleSheet(QString("QLabel { color : %1; }").arg(check[TT_AirBourneSound] ? positive.name() : negative.name()));

	m_ui->gridLayoutMaster->setMargin(0);
}


void SVAcousticConstraintsCheckWidget::on_pushButtonCheckConstraints_clicked() {
	checkConstraints();

	m_ui->tabWidget->setEnabled(true);
}


void SVAcousticConstraintsCheckWidget::on_checkBoxHideWalls_stateChanged(int hideAirBourneSound) {
	m_hideAirBourneSound = (bool)hideAirBourneSound;
	// render constraints again
	updateTable();
}


void SVAcousticConstraintsCheckWidget::on_checkBoxHideImpactSound_stateChanged(int hideImpactSound) {
	m_hideImpactSound = (bool)hideImpactSound;
	// render constraints again
	updateTable();
}


