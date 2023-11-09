#include "SVAcousticConstraintsCheckDialog.h"
#include "ui_SVAcousticConstraintsCheckDialog.h"

#include "SVStyle.h"
#include <VICUS_Project.h>
#include "SVProjectHandler.h"
#include "QtExt_Conversions.h"
#include "SVUndoTreeNodeState.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "Vic3DSceneView.h"


SVAcousticConstraintsCheckDialog::SVAcousticConstraintsCheckDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::SVAcousticConstraintsCheckDialog)
{
	m_ui->setupUi(this);

	// set up table widgets

	// for walls
	m_ui->tableWidgetWalls->setColumnCount(8);
	m_ui->tableWidgetWalls->setHorizontalHeaderLabels(QStringList()
													  << tr("Acoustic template A")
													  << tr("Acoustic Template B")
													  << tr("Acoustic component")
													  << tr("Actual air sound value")
													  << tr("Normal constraints")
													  << tr("Advanced constraints")
													  << tr("Structure type")
													  << "");
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetWalls);
	m_ui->tableWidgetWalls->setSortingEnabled(false);
	m_ui->tableWidgetWalls->resizeColumnsToContents();


	//for ceilings
	m_ui->tableWidgetCeilings->setColumnCount(8);
	m_ui->tableWidgetCeilings->setHorizontalHeaderLabels(QStringList()
														 << tr("Acoustic template above")
														 << tr("Acoustic Template beneth")
														 << tr("Acoustic component")
														 << tr("Actual impact sound value")
														 << tr("Normal constraints")
														 << tr("Advanced constraints")
														 << tr("Structure type")
														 << "");

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetCeilings);

	m_ui->tableWidgetCeilings->setSortingEnabled(false);
	m_ui->tableWidgetCeilings->resizeColumnsToContents();

	double width = 130;
	m_ui->tableWidgetCeilings->setColumnWidth(CSP_AcousticTemplateB, 200);
	m_ui->tableWidgetCeilings->setColumnWidth(CSP_AcousticComponent, width);
	m_ui->tableWidgetCeilings->setColumnWidth(CSP_ActualAirSoundValue, width);
	m_ui->tableWidgetCeilings->setColumnWidth(CSP_AdvancedConstraints, width);
	m_ui->tableWidgetCeilings->setColumnWidth(CSP_NormalConstraints, width);
	m_ui->tableWidgetCeilings->setColumnWidth(CSP_SameStructure, width);
	m_ui->tableWidgetCeilings->setColumnWidth(CSP_SelectButton, 230);

	m_ui->tableWidgetWalls->setColumnWidth(CSP_AcousticTemplateB, 200);
	m_ui->tableWidgetWalls->setColumnWidth(CSP_AcousticComponent, width);
	m_ui->tableWidgetWalls->setColumnWidth(CSP_ActualAirSoundValue, width);
	m_ui->tableWidgetWalls->setColumnWidth(CSP_AdvancedConstraints, width);
	m_ui->tableWidgetWalls->setColumnWidth(CSP_NormalConstraints, width);
	m_ui->tableWidgetWalls->setColumnWidth(CSP_SameStructure, width);
	m_ui->tableWidgetWalls->setColumnWidth(CSP_SelectButton, 230);

	QHeaderView *headerView = m_ui->tableWidgetCeilings->horizontalHeader();
	headerView->setSectionResizeMode(CSP_AcousticTemplateA, QHeaderView::Stretch);

	headerView = m_ui->tableWidgetWalls->horizontalHeader();
	headerView->setSectionResizeMode(CSP_AcousticTemplateA, QHeaderView::Stretch);

	m_ui->tableWidgetWalls->horizontalHeaderItem(CSP_AcousticTemplateA)->setTextAlignment(Qt::AlignLeft);
	m_ui->tableWidgetWalls->horizontalHeaderItem(CSP_AcousticTemplateB)->setTextAlignment(Qt::AlignLeft);

	m_ui->tableWidgetCeilings->horizontalHeaderItem(CSP_AcousticTemplateA)->setTextAlignment(Qt::AlignLeft);
	m_ui->tableWidgetCeilings->horizontalHeaderItem(CSP_AcousticTemplateB)->setTextAlignment(Qt::AlignLeft);

	m_ui->checkBoxHideCeilings->setChecked(false);
	m_ui->checkBoxHideWalls->setChecked(false);

	resize(1500, 800);

	m_ui->tableWidgetReverberation->setColumnCount(NUM_CRT);

	QStringList headers;
	headers << "Room ID"
			<< "Room name"
			<< "Reverberation time 125 Hz [s]"
			<< "Reverberation time 250 Hz [s]"
			<< "Reverberation time 500 Hz [s]"
			<< "Reverberation time 1000 Hz [s]"
			<< "Reverberation time 2000 Hz [s]"
			<< "Reverberation time 4000 Hz [s]"
			<< "Room name";

	m_ui->tableWidgetReverberation->setHorizontalHeaderLabels(headers);

	SVStyle::formatDatabaseTableView(m_ui->tableWidgetReverberation);

	setWindowTitle(tr("Acoustic component check"));
}


SVAcousticConstraintsCheckDialog::~SVAcousticConstraintsCheckDialog() {
	delete m_ui;
}


bool SVAcousticConstraintsCheckDialog::edit(){
	// Check already on start-up
	checkConstraints();

	int res = exec();
	return res;
}


void SVAcousticConstraintsCheckDialog::showSurfaces(unsigned int surfaceAId, unsigned int surfaceBId) {
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


void SVAcousticConstraintsCheckDialog::checkConstraints() {

	// clear all the table entries for walls and doors
	m_wallTes.clear();

	// clear all the table entries for ceilings
	m_ceilingTes.clear();

	bool airBourneSoundCheck = true;
	bool ceilingCheck = true;

	// iterate over all component instances
	for(const VICUS::ComponentInstance & ci : project().m_componentInstances){

		// skip all that dont have two surfaces
		if(ci.m_idSideASurface == VICUS::INVALID_ID || ci.m_idSideBSurface == VICUS::INVALID_ID ||
				ci.m_sideASurface == nullptr || ci.m_sideBSurface == nullptr)
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

		// Refernece to db
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

		for(std::pair<unsigned int, VICUS::AcousticReferenceComponent>  refEntry : db.m_acousticReferenceComponents){
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
			airBourneSoundEntry.m_isSameStructuralUnit = (roomA->m_structuralUnit != nullptr &&
					roomA->m_structuralUnit == roomB->m_structuralUnit);

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
						airBourneSoundCheck = false;
					}
					airBourneSoundEntry.m_expectedNormalLimit = QString("%1 dB").arg(airSoundLimit);
				}
				else {
					if(airRes >= airSoundLimit)
						airBourneSoundEntry.m_advancedConstraintViolated = VI_Valid;
					else {
						airBourneSoundEntry.m_advancedConstraintViolated = VI_Invalid;
						airBourneSoundCheck = false;
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
							ceilingCheck = false;
						}
						impactSoundEntry.m_expectedNormalLimit = QString("%1 dB").arg(impactSoundLimit);
					}
					else {
						if(impact < impactSoundLimit)
							impactSoundEntry.m_advancedConstraintViolated = VI_Valid;
						else {
							impactSoundEntry.m_advancedConstraintViolated = VI_Invalid;
							ceilingCheck = false;
						}
						impactSoundEntry.m_expectedAdvancedLimit = QString("%1 dB").arg(impactSoundLimit);
					}
				}
			}
		}


		if(addTableEntryAirSoundToVector)
			m_wallTes.push_back(airBourneSoundEntry);

		if(addTableEntryImpactSoundToVector)
			m_ceilingTes.push_back(impactSoundEntry);

	}

	m_ui->labelCeilingCheck->setText(ceilingCheck ? "All acoustic checks for ceilings passed succesfully." : "Acoustic checks of ceilings not passed.");
	m_ui->labelWallCheck->setText(airBourneSoundCheck ? "All acoustic checks for walls passed succesfully." : "Acoustic checks of walls not passed.");

	m_ui->labelIconWall->setPixmap(QPixmap(airBourneSoundCheck    ? ":/gfx/actions/16x16/ok.png" : ":/gfx/actions/16x16/error.png"));
	m_ui->labelIconCeiling->setPixmap(   QPixmap(ceilingCheck ? ":/gfx/actions/16x16/ok.png" : ":/gfx/actions/16x16/error.png"));

	QColor positive(Qt::darkGreen);
	QColor negative(Qt::darkRed);

	positive = SVSettings::instance().m_theme == SVSettings::TT_Dark ? positive.lighter() : positive;
	negative = SVSettings::instance().m_theme == SVSettings::TT_Dark ? negative.lighter() : negative;

	m_ui->labelCeilingCheck->setStyleSheet(QString("QLabel { color : %1; }").arg(ceilingCheck ? positive.name() : negative.name()));
	m_ui->labelWallCheck->setStyleSheet(QString("QLabel { color : %1; }").arg(airBourneSoundCheck ? positive.name() : negative.name()));

	updateTable();
	checkReverberation();
}

void SVAcousticConstraintsCheckDialog::checkReverberation() {
	const VICUS::Project &prj = project();
	const SVDatabase & db = SVSettings::instance().m_db;

	struct ReverberationResult {

		ReverberationResult() {}

		ReverberationResult(unsigned int roomID, const QString &roomName,
							const std::vector<double> &reverberationTime,
							const std::vector<bool> &reverberationEvaluation) :
			m_idRoom(roomID),
			m_roomName(roomName),
			m_reverberationTime(reverberationTime),
			m_evaluation(reverberationEvaluation)
		{}

		// Room ID
		unsigned int	m_idRoom;
		// Room name
		QString			m_roomName;
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

						for (const VICUS::AcousticSoundAbsorptionPartition &layer : aBC->m_soundAbsorptionLayers) {
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

						for (const VICUS::AcousticSoundAbsorptionPartition &layer : aBC->m_soundAbsorptionLayers) {
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

				double reverbTimeGoal = at->m_evaluationOffset + at->m_evaluationFactor * std::log(roomVolume);
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

				tableEntries.push_back(ReverberationResult (r.m_id, r.m_displayName, reverberationTime, reverberationEvaluation));
			}
		}
	}

	m_ui->tableWidgetReverberation->setRowCount(tableEntries.size());

	for (unsigned int i=0; i < tableEntries.size(); ++i) {

		const ReverberationResult &res = tableEntries[i];

		m_ui->tableWidgetReverberation->setItem(i, CRT_RoomID, new QTableWidgetItem(QString::number(res.m_idRoom)));
		m_ui->tableWidgetReverberation->setItem(i, CRT_RoomName, new QTableWidgetItem(res.m_roomName));

		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb125Hz, new QTableWidgetItem(QString::number(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_125Hz])));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb250Hz, new QTableWidgetItem(QString::number(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_250Hz])));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb500Hz, new QTableWidgetItem(QString::number(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_500Hz])));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb1000Hz, new QTableWidgetItem(QString::number(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_1000Hz])));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb2000Hz, new QTableWidgetItem(QString::number(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_2000Hz])));
		m_ui->tableWidgetReverberation->setItem(i, CRT_Reverb4000Hz, new QTableWidgetItem(QString::number(res.m_reverberationTime[VICUS::AcousticSoundAbsorption::SF_4000Hz])));

		QColor valid(Qt::darkGreen);
		QColor invalid(Qt::darkRed);

		if (SVSettings::instance().m_theme == SVSettings::TT_White) {
			valid = valid.lighter(250);
			invalid = invalid.lighter(250);
		}

		for (unsigned int j = 0; j < VICUS::AcousticSoundAbsorption::NUM_SF; ++j) {
			if (res.m_evaluation[j])
				m_ui->tableWidgetReverberation->item(i, 2 + j)->setBackground(valid);
			else
				m_ui->tableWidgetReverberation->item(i, 2 + j)->setBackground(invalid);
		}
	}
}


void SVAcousticConstraintsCheckDialog::updateTable() {


	QTableWidget * tableWidget = nullptr;
	std::vector<tableEntry> * tes = nullptr;
	bool hideNonViolatedEntries = false;


	// filling the table is identical, so only the tableWidget, the data & (hiding entries) changes during the iterations
	for(unsigned int i = 0; i<2 ; ++i){

		switch (i) {
		case 0:
			tableWidget = m_ui->tableWidgetWalls;
			tes = &m_wallTes;
			hideNonViolatedEntries = m_hideWalls;
			break;
		case 1:
			tableWidget = m_ui->tableWidgetCeilings;
			tes = &m_ceilingTes;
			hideNonViolatedEntries = m_hideCeilings;
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
			// select color according to violation info
			QColor normalColor;
			switch (te.m_basicConstraintViolated) {
			case VI_Invalid:
				normalColor = Qt::red;
				break;
			case VI_Valid:
				normalColor = Qt::green;
				break;
			default:
				break;
			}

			QBrush backgroundBrush;
			if (SVSettings::instance().m_theme == SVSettings::TT_White)
				backgroundBrush = QBrush(normalColor.lighter());
			else
				backgroundBrush = QBrush(normalColor);

			item->setBackground(backgroundBrush);
			item->setFlags(Qt::ItemIsEnabled);
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			tableWidget->setItem(row, CSP_NormalConstraints, item);

			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setText(te.m_expectedAdvancedLimit);
			// select color according to violation info
			QColor advancedColor;
			switch (te.m_advancedConstraintViolated) {
			case VI_Invalid:
				advancedColor = Qt::darkRed;
				break;
			case VI_Valid:
				advancedColor = Qt::darkGreen;
				break;
			default:
				break;
			}


			if (SVSettings::instance().m_theme == SVSettings::TT_White)
				backgroundBrush = QBrush(normalColor.lighter());
			else
				backgroundBrush = QBrush(normalColor);

			item->setBackground(backgroundBrush);
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
				SVAcousticConstraintsCheckDialog::showSurfaces(surfaceAId, surfaceBId);
			});

			// table widget takes ownership of btn and memory will be freed on destruction
			tableWidget->setCellWidget(row, CSP_SelectButton, btn);

			tableWidget->resizeRowsToContents();

			++row;
		}

		tableWidget->blockSignals(false);
		//tableWidget->resizeColumnsToContents();

	}
}


void SVAcousticConstraintsCheckDialog::on_pushButtonCheckConstraints_clicked() {
	checkConstraints();
}


void SVAcousticConstraintsCheckDialog::on_checkBoxHideWalls_stateChanged(int arg1) {
	m_hideWalls = arg1;
	// render constraints again
	updateTable();
}


void SVAcousticConstraintsCheckDialog::on_checkBoxHideCeilings_stateChanged(int arg1) {
	m_hideCeilings = arg1;
	// render constraints again
	updateTable();
}


