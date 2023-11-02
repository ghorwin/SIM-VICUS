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
	m_ui->tableWidgetCeilings->setColumnWidth(ColAcousticTemplateB, 200);
	m_ui->tableWidgetCeilings->setColumnWidth(ColAcousticComponent, width);
	m_ui->tableWidgetCeilings->setColumnWidth(ColActualAirSoundValue, width);
	m_ui->tableWidgetCeilings->setColumnWidth(ColAdvancedConstraints, width);
	m_ui->tableWidgetCeilings->setColumnWidth(ColNormalConstraints, width);
	m_ui->tableWidgetCeilings->setColumnWidth(ColSameStructure, width);
	m_ui->tableWidgetCeilings->setColumnWidth(ColSelectButton, 230);

	m_ui->tableWidgetWalls->setColumnWidth(ColAcousticTemplateB, 200);
	m_ui->tableWidgetWalls->setColumnWidth(ColAcousticComponent, width);
	m_ui->tableWidgetWalls->setColumnWidth(ColActualAirSoundValue, width);
	m_ui->tableWidgetWalls->setColumnWidth(ColAdvancedConstraints, width);
	m_ui->tableWidgetWalls->setColumnWidth(ColNormalConstraints, width);
	m_ui->tableWidgetWalls->setColumnWidth(ColSameStructure, width);
	m_ui->tableWidgetWalls->setColumnWidth(ColSelectButton, 230);

	QHeaderView *headerView = m_ui->tableWidgetCeilings->horizontalHeader();
	headerView->setSectionResizeMode(ColAcousticTemplateA, QHeaderView::Stretch);

	headerView = m_ui->tableWidgetWalls->horizontalHeader();
	headerView->setSectionResizeMode(ColAcousticTemplateA, QHeaderView::Stretch);

	m_ui->tableWidgetWalls->horizontalHeaderItem(ColAcousticTemplateA)->setTextAlignment(Qt::AlignLeft);
	m_ui->tableWidgetWalls->horizontalHeaderItem(ColAcousticTemplateB)->setTextAlignment(Qt::AlignLeft);

	m_ui->tableWidgetCeilings->horizontalHeaderItem(ColAcousticTemplateA)->setTextAlignment(Qt::AlignLeft);
	m_ui->tableWidgetCeilings->horizontalHeaderItem(ColAcousticTemplateB)->setTextAlignment(Qt::AlignLeft);

	m_ui->checkBoxHideCeilings->setChecked(false);
	m_ui->checkBoxHideWalls->setChecked(false);

	resize(1500, 800);

	m_ui->tableWidgetReverberation->setColumnCount(2);

	QStringList headers;
	headers << "Room name" << "Reverberation time [s]";
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
		airBourneSoundEntry.basicConstraintViolated = VI_NoConstraint;
		airBourneSoundEntry.advancedConstraintViolated = VI_NoConstraint;
		impactSoundEntry.basicConstraintViolated = VI_NoConstraint;
		impactSoundEntry.advancedConstraintViolated = VI_NoConstraint;

		// search the rooms of the corresponding surfaces
		const VICUS::Room * roomA = dynamic_cast<VICUS::Room *>(ci.m_sideASurface->m_parent);
		const VICUS::Room * roomB = dynamic_cast<VICUS::Room *>(ci.m_sideBSurface->m_parent);
		if(roomA == nullptr || roomB == nullptr)
			continue;

		// skip if they dont have the same acoustic building type
		if(roomA->m_acousticBuildingTypeId != roomB->m_acousticBuildingTypeId)
			continue;

		// check if they are in the same structural unit or not

		// get the ids of the acoustic template of the corresponding room
		unsigned int acousticTemplateAId = roomA->m_idAcousticTemplate;
		unsigned int acousticTemplateBId = roomB->m_idAcousticTemplate;

		// skip if one of them does not have an acoustic template
		if(acousticTemplateAId == VICUS::INVALID_ID || acousticTemplateBId == VICUS::INVALID_ID)
			continue;

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

			if ( roomA->m_acousticBuildingTypeId != refComp.m_buildingType ||
				 roomB->m_acousticBuildingTypeId != refComp.m_buildingType )
				continue;

			// is true, if the direction between the templates match
			bool impactSoundTemplatesMatch = false;

			// ceiling and ref comp. also is ceiling
			if(isCeiling && refComp.m_type == VICUS::AcousticReferenceComponent::CT_Ceiling)
				impactSoundTemplatesMatch = true;


			// add the walls template
			//we have a match for walls, so fill in information for table widget struct
			const VICUS::AcousticTemplate * tempA = db.m_acousticTemplates[acousticTemplateAId];
			const VICUS::AcousticTemplate * tempB = db.m_acousticTemplates[acousticTemplateBId];
			airBourneSoundEntry.acousticTemplateAInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(tempA->m_displayName)).arg(tempA->m_id);
			airBourneSoundEntry.acousticTemplateBInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(tempB->m_displayName)).arg(tempB->m_id);

			// save the surface ids
			airBourneSoundEntry.surfaceAId = ci.m_idSideASurface;
			airBourneSoundEntry.surfaceBId = ci.m_idSideBSurface;
			airBourneSoundEntry.isSameStructuralUnit = (roomA->m_structuralUnit != nullptr &&
					roomA->m_structuralUnit == roomB->m_structuralUnit);

			// this entry should be saved in the vector
			addTableEntryAirSoundToVector = true;

			// retrieve the limits from the acoustic reference Component
			double impactSoundLimit;
			double airSoundLimit;
			if(airBourneSoundEntry.isSameStructuralUnit){
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
			airBourneSoundEntry.actualValue = QString("%1 dB").arg(airRes);

			// fill te struct with acoustic component info
			airBourneSoundEntry.acousticComponentInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(comp->m_displayName)).arg(comp->m_id);

			// check wether a limit is provided
			if(IBK::near_equal(airSoundLimit, -1)){
				// no limit is provided, set to not violated
				if (refComp.m_requirementType == VICUS::AcousticReferenceComponent::RT_Basic) {
					airBourneSoundEntry.basicConstraintViolated = VI_Valid;
					airBourneSoundEntry.expectedNormalLimit = tr("No limit");
				}
				else {
					airBourneSoundEntry.advancedConstraintViolated = VI_Valid;
					airBourneSoundEntry.expectedAdvancedLimit = tr("No limit");
				}
			} else {
				// there is a limit
				// enter the results of the airborne sound check, depending on the requirement type
				if (refComp.m_requirementType == VICUS::AcousticReferenceComponent::RT_Basic) {

					if(airRes >= airSoundLimit)
						airBourneSoundEntry.basicConstraintViolated = VI_Valid;
					else {
						airBourneSoundEntry.basicConstraintViolated = VI_Invalid;
						airBourneSoundCheck = false;
					}
					airBourneSoundEntry.expectedNormalLimit = QString("%1 dB").arg(airSoundLimit);
				}
				else {
					if(airRes >= airSoundLimit)
						airBourneSoundEntry.advancedConstraintViolated = VI_Valid;
					else {
						airBourneSoundEntry.advancedConstraintViolated = VI_Invalid;
						airBourneSoundCheck = false;
					}
					airBourneSoundEntry.expectedAdvancedLimit = QString("%1 dB").arg(airSoundLimit);
				}
			}
			// check if there war another match for the impact sound (ceiling)
			if(impactSoundTemplatesMatch){

				// we have a match for ceilings, we copy identical data from the wall table entry
				impactSoundEntry.acousticTemplateAInfo = airBourneSoundEntry.acousticTemplateAInfo;
				impactSoundEntry.acousticTemplateBInfo = airBourneSoundEntry.acousticTemplateBInfo;
				impactSoundEntry.surfaceAId = airBourneSoundEntry.surfaceAId;
				impactSoundEntry.surfaceBId = airBourneSoundEntry.surfaceBId;
				impactSoundEntry.isSameStructuralUnit = airBourneSoundEntry.isSameStructuralUnit;
				impactSoundEntry.acousticComponentInfo = airBourneSoundEntry.acousticComponentInfo;

				double impact = con->m_acousticPara[VICUS::Construction::P_ImpactSoundValue].value;
				impactSoundEntry.actualValue = QString("%1 dB").arg(impact);

				// this entry should be saved in the vector
				addTableEntryImpactSoundToVector = true;

				// check wether a limit is provided
				if(IBK::near_equal(impactSoundLimit, -1)){
					// no limit is provided, set to not violated
					if(refComp.m_requirementType == VICUS::AcousticReferenceComponent::RT_Basic){
						impactSoundEntry.basicConstraintViolated = VI_Valid;
						impactSoundEntry.expectedNormalLimit = tr("No limit");
					}
					else {
						impactSoundEntry.advancedConstraintViolated = VI_Valid;
						impactSoundEntry.expectedAdvancedLimit = tr("No limit");
					}
				}
				else {
					// now we modify the teCeil struct in the same way as before with airSound
					if(refComp.m_requirementType == VICUS::AcousticReferenceComponent::RT_Basic){
						if(impact < impactSoundLimit)
							impactSoundEntry.basicConstraintViolated = VI_Valid;
						else {
							impactSoundEntry.basicConstraintViolated = VI_Invalid;
							ceilingCheck = false;
						}
						impactSoundEntry.expectedNormalLimit = QString("%1 dB").arg(impactSoundLimit);
					}
					else {
						if(impact < impactSoundLimit)
							impactSoundEntry.advancedConstraintViolated = VI_Valid;
						else {
							impactSoundEntry.advancedConstraintViolated = VI_Invalid;
							ceilingCheck = false;
						}
						impactSoundEntry.expectedAdvancedLimit = QString("%1 dB").arg(impactSoundLimit);
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

	std::vector<std::pair<QString, double>> entries;

	for (const VICUS::Building &b : prj.m_buildings) {
		for (const VICUS::BuildingLevel &bl : b.m_buildingLevels) {
			for (const VICUS::Room &r : bl.m_rooms) {

				double roomVolume = r.m_volume;
				double reverberationSum = 0.0;

				for (const VICUS::Surface &s : r.m_surfaces) {
					double surfaceArea = s.geometry().area();

					const VICUS::ComponentInstance *ci = s.m_componentInstance;
					if (ci == nullptr)
						continue;

					const VICUS::Component *comp = db.m_components[ci->m_idComponent];
					if (comp == nullptr)
						continue;

					if (ci->m_idSideASurface == r.m_id) {
						const VICUS::AcousticBoundaryCondition *aBC = db.m_acousticBoundaryConditions[comp->m_idSideAAcousticBoundaryCondition];

						for (const VICUS::SoundAbsorptionLayer &layer : aBC->m_soundAbsorptionLayers) {
							double areaFraction = layer.m_para[VICUS::SoundAbsorptionLayer::P_AreaFraction].value;
							const VICUS::AcousticSoundAbsorption *soundAbsorption = db.m_acousticSoundAbsorptions[layer.m_idSoundAbsorption];

							if (soundAbsorption == nullptr)
								continue;

							for (unsigned int i=0; i<VICUS::AcousticSoundAbsorption::NUM_SF; ++i) {
								reverberationSum += areaFraction * soundAbsorption->m_soundAbsorption[i];
							}

						}

						reverberationSum *= surfaceArea;
					}


					if (ci->m_idSideBSurface == r.m_id) {
						const VICUS::AcousticBoundaryCondition *aBC = db.m_acousticBoundaryConditions[comp->m_idSideBAcousticBoundaryCondition];

						for (const VICUS::SoundAbsorptionLayer &layer : aBC->m_soundAbsorptionLayers) {
							double areaFraction = layer.m_para[VICUS::SoundAbsorptionLayer::P_AreaFraction].value;
							const VICUS::AcousticSoundAbsorption *soundAbsorption = db.m_acousticSoundAbsorptions[layer.m_idSoundAbsorption];

							if (soundAbsorption == nullptr)
								continue;

							for (unsigned int i=0; i<VICUS::AcousticSoundAbsorption::NUM_SF; ++i) {
								reverberationSum += areaFraction * soundAbsorption->m_soundAbsorption[i];
							}

						}

						reverberationSum *= surfaceArea;
					}
				}

				double reverberationTime = 0.163 * reverberationSum / roomVolume;
				entries.push_back(std::pair<QString, double>(r.m_displayName, reverberationTime));
			}
		}
	}

	m_ui->tableWidgetReverberation->setRowCount(entries.size());

	for (unsigned int i=0; i < entries.size(); ++i) {
		const std::pair<QString, double> entry = entries[i];
		m_ui->tableWidgetReverberation->setItem(i, 0, new QTableWidgetItem(entry.first));
		m_ui->tableWidgetReverberation->setItem(i, 1, new QTableWidgetItem(QString::number(entry.second)));

		m_ui->tableWidgetReverberation->item(i, 1)->setBackground(Qt::green);
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
			if(hideNonViolatedEntries && te.basicConstraintViolated != ViolationInfo::VI_Invalid &&
					te.advancedConstraintViolated != ViolationInfo::VI_Invalid)
				continue;

			tableWidget->setRowCount(row + 1);

			QTableWidgetItem * item;
			item = new QTableWidgetItem();
			item->setText(te.acousticTemplateAInfo);
			item->setFlags(Qt::ItemIsEnabled);
			tableWidget->setItem(row, ColAcousticTemplateA, item);

			item = new QTableWidgetItem();
			item->setText(te.acousticTemplateBInfo);
			item->setFlags(Qt::ItemIsEnabled);
			tableWidget->setItem(row, ColAcousticTemplateB, item);

			item = new QTableWidgetItem();
			item->setText(te.acousticComponentInfo);
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled);
			tableWidget->setItem(row, ColAcousticComponent, item);

			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setText(te.actualValue);
			item->setFlags(Qt::ItemIsEnabled);
			tableWidget->setItem(row, ColActualAirSoundValue, item);

			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setText(te.expectedNormalLimit);
			// select color according to violation info
			QColor normalColor;
			switch (te.basicConstraintViolated) {
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
			tableWidget->setItem(row, ColNormalConstraints, item);

			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setText(te.expectedAdvancedLimit);
			// select color according to violation info
			QColor advancedColor;
			switch (te.advancedConstraintViolated) {
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
			tableWidget->setItem(row, ColAdvancedConstraints, item);

			item = new QTableWidgetItem();
			item->setText(te.isSameStructuralUnit ? "same structure" : "different structure");
			item->setFlags(Qt::ItemIsEnabled);
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			tableWidget->setItem(row, ColSameStructure, item);

			QPushButton * btn = new QPushButton(tr("Select components in scene..."));

			unsigned int surfaceAId = te.surfaceAId;
			unsigned int surfaceBId = te.surfaceBId;

			// Connecting the clicked signal to a lambda function to pass the surface ids
			QObject::connect(btn, &QPushButton::clicked, [=]() {
				SVAcousticConstraintsCheckDialog::showSurfaces(surfaceAId, surfaceBId);
			});

			// table widget takes ownership of btn and memory will be freed on destruction
			tableWidget->setCellWidget(row, ColSelectButton, btn);

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


