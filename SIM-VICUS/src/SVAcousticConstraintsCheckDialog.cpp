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
	m_ui->tableWidgetWalls->setHorizontalHeaderLabels(QStringList()  << tr("Acoustic template A")<< tr("Acoustic Template B")<< tr("Acoustic component")<<tr("Same Structure")<< tr("actual air sound value")<<tr("normal constraints")<<tr("advanced constraints")<<  tr("button"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetWalls);
	m_ui->tableWidgetWalls->setSortingEnabled(false);
	m_ui->tableWidgetWalls->resizeColumnsToContents();



	//for ceilings
	m_ui->tableWidgetCeilings->setColumnCount(8);
	m_ui->tableWidgetCeilings->setHorizontalHeaderLabels(QStringList()  << tr("Acoustic template above")<< tr("Acoustic Template beneth")<< tr("Acoustic component")<< tr("Same Structure")<<tr("actual impact sound value")<<tr("normal constraints")<<tr("advanced constraints")<<  tr("button"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetCeilings);
	m_ui->tableWidgetCeilings->setSortingEnabled(false);
	m_ui->tableWidgetCeilings->resizeColumnsToContents();
}

SVAcousticConstraintsCheckDialog::~SVAcousticConstraintsCheckDialog()
{
	delete m_ui;
}


bool SVAcousticConstraintsCheckDialog::edit(){
	int res = exec();
	return res;

}

void SVAcousticConstraintsCheckDialog::showSurfaces(unsigned int surfaceAId, unsigned int surfaceBId){
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

	close();


}

void SVAcousticConstraintsCheckDialog::checkConstraints(){

	// all the table entries for walls and doors
	std::vector<tableEntry> wallTes;

	// all the table entries for ceilings
	std::vector<tableEntry> ceilingTes;

	// iterate over all component instances
	for(const VICUS::ComponentInstance & ci : project().m_componentInstances){
		// struct that holds relevant data for the table

		//one for walls
		tableEntry teWall;
		//one for ceilings
		tableEntry teCeil;

		// initialize with no constraints specified, might be overwritten later
		teWall.normalConstraintViolated = VI_No_Constraint;
		teWall.advancedConstraintViolated = VI_No_Constraint;
		teCeil.normalConstraintViolated = VI_No_Constraint;
		teCeil.advancedConstraintViolated = VI_No_Constraint;


		// skip all that dont have a valid acoustic component
		if(ci.m_idAcousticComponent == VICUS::INVALID_ID)
			continue;
		// skip all that dont have two surfaces
		if(ci.m_idSideASurface == VICUS::INVALID_ID || ci.m_idSideBSurface == VICUS::INVALID_ID || ci.m_sideASurface ==nullptr || ci.m_sideBSurface == nullptr)
			continue;

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
		IDType acousticTemplateAId = roomA->m_idAcousticTemplate;
		IDType acousticTemplateBId = roomB->m_idAcousticTemplate;

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
				auto tmp = acousticTemplateAId;
				acousticTemplateAId = acousticTemplateBId;
				acousticTemplateBId = tmp;
			}
		}

		// iterate over the acoustic reference components and find the one for these surfaces

		// if we find a match we save the teWall to the vector in the end
		bool addTableEntryWallToVector = false;
		bool addTableEntryCeilToVector = false;

		for(std::pair<unsigned int, VICUS::AcousticReferenceComponent>  refEntry: db.m_acousticReferenceComponents){
			VICUS::AcousticReferenceComponent refComp = refEntry.second;
			// is true, if the direction between the templates match
			bool impactSoundTemplatesMatch = false;
			// is true, if the any the templates match
			bool airborneSoundTemplatesMatch = false;

			if(isCeiling){
				// only check in one direction
				if(refComp.m_idAcousticTemplateA == acousticTemplateAId &&
						refComp.m_idAcousticTemplateB == acousticTemplateBId){
					// the reference component type must also be for ceilings
					if(refComp.m_type == VICUS::AcousticReferenceComponent::CT_Ceiling){
						impactSoundTemplatesMatch = true;
					}
				}
			}
			// allow both directions
			if((refComp.m_idAcousticTemplateA == acousticTemplateAId &&
					refComp.m_idAcousticTemplateB == acousticTemplateBId) ||
					(refComp.m_idAcousticTemplateA == acousticTemplateAId &&
					refComp.m_idAcousticTemplateB == acousticTemplateBId)){
				// if it is a ceiling then the type must be a ceiling as well
				// if its not a ceiling then the type cant be a ceiling
				if((isCeiling && refComp.m_type == VICUS::AcousticReferenceComponent::CT_Ceiling) ||
						(!isCeiling && refComp.m_type != VICUS::AcousticReferenceComponent::CT_Ceiling))
					airborneSoundTemplatesMatch = true;
			}

			if(airborneSoundTemplatesMatch){
				// add the walls template


				//fwe have a match for walls, so fill in information for table widget struct
				const VICUS::AcousticTemplate * tempA = db.m_acousticTemplates[acousticTemplateAId];
				const VICUS::AcousticTemplate * tempB = db.m_acousticTemplates[acousticTemplateBId];
				teWall.acousticTemplateAInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(tempA->m_displayName)).arg(tempA->m_id);
				teWall.acousticTemplateBInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(tempB->m_displayName)).arg(tempB->m_id);
				// save the surface ids
				teWall.surfaceAId = ci.m_idSideASurface;
				teWall.surfaceBId = ci.m_idSideBSurface;
				teWall.isSameStructuralUnit = (roomA->m_structuralUnit == roomB->m_structuralUnit && roomB->m_structuralUnit != nullptr);

				// this entry should be saved in the vector
				addTableEntryWallToVector = true;

				// retrieve the limits from the acoustic reference Component
				double impactSoundLimit;
				double airSoundLimit;
				if(teWall.isSameStructuralUnit){
					airSoundLimit = refComp.m_airborneSoundOneStructureUnit;
					impactSoundLimit = refComp.m_impactSoundOneStructureUnit;
				} else {
					airSoundLimit = refComp.m_airborneSoundDifferentStructureUnits;
					impactSoundLimit = refComp.m_impactSoundDifferentStructureUnits;
				}
				// retrieve the acoustic component to check if it fullfills the limits
				const VICUS::AcousticComponent * acComp = db.m_acousticComponents[ci.m_idAcousticComponent];

				// fill te struct with acoustic component info
				teWall.acousticComponentInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(acComp->m_displayName)).arg(acComp->m_id);

				// enter the results of the airborne sound check, depending on the requirement type
				if(refComp.m_requirmentType == VICUS::AcousticReferenceComponent::RT_Normal){
					if(acComp->m_airSoundResistenceValue < airSoundLimit)
						teWall.normalConstraintViolated = VI_Not_Violated;
					else
						teWall.normalConstraintViolated = VI_Violated;
					teWall.actualValue = QString("%1 dB").arg(acComp->m_airSoundResistenceValue);
					teWall.expectedNormalLimit = QString("%1 dB").arg(airSoundLimit);
				} else {
					if(acComp->m_airSoundResistenceValue < airSoundLimit)
						teWall.advancedConstraintViolated = VI_Not_Violated;
					else
						teWall.advancedConstraintViolated = VI_Violated;
					teWall.actualValue = QString("%1 dB").arg(acComp->m_airSoundResistenceValue);
					teWall.expectedAdvancedLimit = QString("%1 dB").arg(airSoundLimit);
				}

				// TODO Is the assumption correct that a ceiling always has air and impact sound

				// check if there war another match for the impact sound (ceiling)
				if(impactSoundTemplatesMatch){

					// we have a match for ceilings, we copy identical data from the wall table entry
					teCeil.acousticTemplateAInfo = teWall.acousticTemplateAInfo;
					teCeil.acousticTemplateBInfo = teWall.acousticTemplateBInfo;
					teCeil.surfaceAId = teWall.surfaceAId;
					teCeil.surfaceBId = teWall.surfaceBId;
					teCeil.isSameStructuralUnit = teWall.isSameStructuralUnit;
					teCeil.acousticComponentInfo = teWall.acousticComponentInfo;

					// this entry should be saved in the vector
					addTableEntryCeilToVector = true;

					// now we modify the teCeil struct in the same way as before with airSound
					if(refComp.m_requirmentType == VICUS::AcousticReferenceComponent::RT_Normal){
						if(acComp->m_impactSoundValue < impactSoundLimit)
							teCeil.normalConstraintViolated = VI_Not_Violated;
						else
							teCeil.normalConstraintViolated = VI_Violated;
						teCeil.actualValue = QString("%1 dB").arg(acComp->m_impactSoundValue);
						teCeil.expectedNormalLimit = QString("%1 dB").arg(impactSoundLimit);
					} else {
						if(acComp->m_impactSoundValue < impactSoundLimit)
							teCeil.advancedConstraintViolated = VI_Not_Violated;
						else
							teCeil.advancedConstraintViolated = VI_Violated;
						teCeil.actualValue = QString("%1 dB").arg(acComp->m_impactSoundValue);
						teCeil.expectedAdvancedLimit = QString("%1 dB").arg(impactSoundLimit);
					}
				}
			}
		}
		if(addTableEntryWallToVector){
			wallTes.push_back(teWall);
		}
		if(addTableEntryCeilToVector){
			ceilingTes.push_back(teCeil);
		}
	}

	//fill the walls table with the collected wall table entries
	m_ui->tableWidgetWalls->blockSignals(true);
	m_ui->tableWidgetWalls->setRowCount(0);
	int row=0;

	for(tableEntry &  te : wallTes){

		// if the hide walls selection box is check, skip all entries that dont have a violation
		if(m_hideWalls && te.normalConstraintViolated != ViolationInfo::VI_Violated &&
				te.advancedConstraintViolated != ViolationInfo::VI_Violated)
			continue;

		m_ui->tableWidgetWalls->setRowCount(row + 1);

		QTableWidgetItem * item;
		item = new QTableWidgetItem();
		item->setText(te.acousticTemplateAInfo);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetWalls->setItem(row, WTC_acousticTemplateA, item);

		item = new QTableWidgetItem();
		item->setText(te.acousticTemplateBInfo);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetWalls->setItem(row, WTC_acousticTemplateB, item);

		item = new QTableWidgetItem();
		item->setText(te.acousticComponentInfo);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetWalls->setItem(row, WTC_acousticComponent, item);

		item = new QTableWidgetItem();
		item->setIcon(QIcon(te.isSameStructuralUnit ? ":/gfx/actions/16x16/ok.png" : ":/gfx/actions/16x16/error.png"));
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetWalls->setItem(row, WTC_sameStructure, item);

		item = new QTableWidgetItem();
		item->setText(te.actualValue);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetWalls->setItem(row, WTC_actualAirSoundValue, item);

		item = new QTableWidgetItem();
		item->setText(te.expectedNormalLimit);
		// select color according to violation info
		QBrush normalColor;
		switch (te.normalConstraintViolated) {
		case VI_Violated:
			normalColor = Qt::darkRed;
		break;
		case VI_Not_Violated:
			normalColor = Qt::darkGreen;
		break;
		default:
			break;
		}
		item->setBackground(normalColor);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetWalls->setItem(row, WTC_normalConstraints, item);

		item = new QTableWidgetItem();
		item->setText(te.expectedAdvancedLimit);
		// select color according to violation info
		QBrush advancedColor;
		switch (te.advancedConstraintViolated) {
		case VI_Violated:
			advancedColor = Qt::darkRed;
			break;
		case VI_Not_Violated:
			advancedColor = Qt::darkGreen;
			break;
		default:
			break;
		}
		item->setBackground(advancedColor);
		item->setFlags(Qt::ItemIsEnabled);

		m_ui->tableWidgetWalls->setItem(row, WTC_advancedConstraints, item);

		QPushButton * btn = new QPushButton(tr("Show"));

		unsigned int surfaceAId = te.surfaceAId;
		unsigned int surfaceBId = te.surfaceBId;

		// Connecting the clicked signal to a lambda function to pass the surface ids
		QObject::connect(btn, &QPushButton::clicked, [=]() {
			SVAcousticConstraintsCheckDialog::showSurfaces(surfaceAId, surfaceBId);
		});

		// table widget takes ownership of btn and memory will be freed on destruction
		m_ui->tableWidgetWalls->setCellWidget(row, WTC_showButton, btn);

		m_ui->tableWidgetWalls->resizeRowsToContents();

		++row;
	}

	m_ui->tableWidgetWalls->blockSignals(false);
	m_ui->tableWidgetWalls->resizeColumnsToContents();

	// fill the walls table with the collected ceiling table entries
	m_ui->tableWidgetCeilings->blockSignals(true);
	m_ui->tableWidgetCeilings->setRowCount(0);
	row=0;

	for(tableEntry &  te : ceilingTes){

		// if the hide walls selection box is check, skip all entries that dont have a violation
		if(m_hideCeilings && te.normalConstraintViolated != ViolationInfo::VI_Violated &&
				te.advancedConstraintViolated != ViolationInfo::VI_Violated)
			continue;

		m_ui->tableWidgetCeilings->setRowCount(row + 1);

		QTableWidgetItem * item;
		item = new QTableWidgetItem();
		item->setText(te.acousticTemplateAInfo);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetCeilings->setItem(row, WTC_acousticTemplateA, item);

		item = new QTableWidgetItem();
		item->setText(te.acousticTemplateBInfo);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetCeilings->setItem(row, WTC_acousticTemplateB, item);

		item = new QTableWidgetItem();
		item->setText(te.acousticComponentInfo);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetCeilings->setItem(row, WTC_acousticComponent, item);

		item = new QTableWidgetItem();
		item->setIcon(QIcon(te.isSameStructuralUnit ? ":/gfx/actions/16x16/ok.png" : ":/gfx/actions/16x16/error.png"));
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetCeilings->setItem(row, WTC_sameStructure, item);

		item = new QTableWidgetItem();
		item->setText(te.actualValue);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetCeilings->setItem(row, WTC_actualAirSoundValue, item);

		item = new QTableWidgetItem();
		item->setText(te.expectedNormalLimit);
		// select color according to violation info
		QBrush normalColor;
		switch (te.normalConstraintViolated) {
		case VI_Violated:
			normalColor = Qt::darkRed;
		break;
		case VI_Not_Violated:
			normalColor = Qt::darkGreen;
		break;
		default:
			break;
		}
		item->setBackground(normalColor);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetCeilings->setItem(row, WTC_normalConstraints, item);

		item = new QTableWidgetItem();
		item->setText(te.expectedAdvancedLimit);
		// select color according to violation info
		QBrush advancedColor;
		switch (te.advancedConstraintViolated) {
		case VI_Violated:
			advancedColor = Qt::darkRed;
			break;
		case VI_Not_Violated:
			advancedColor = Qt::darkGreen;
			break;
		default:
			break;
		}
		item->setBackground(advancedColor);
		item->setFlags(Qt::ItemIsEnabled);
		m_ui->tableWidgetCeilings->setItem(row, WTC_advancedConstraints, item);

		QPushButton * btn = new QPushButton(tr("Show"));

		unsigned int surfaceAId = te.surfaceAId;
		unsigned int surfaceBId = te.surfaceBId;

		// Connecting the clicked signal to a lambda function to pass the surface ids
		QObject::connect(btn, &QPushButton::clicked, [=]() {
			SVAcousticConstraintsCheckDialog::showSurfaces(surfaceAId, surfaceBId);
		});


		m_ui->tableWidgetCeilings->setCellWidget(row, WTC_showButton, btn);

		m_ui->tableWidgetCeilings->resizeRowsToContents();

		++row;
	}

	m_ui->tableWidgetCeilings->blockSignals(false);
	m_ui->tableWidgetCeilings->resizeColumnsToContents();

}



void SVAcousticConstraintsCheckDialog::on_pushButtonCheckConstraints_clicked() {
		checkConstraints();
}


void SVAcousticConstraintsCheckDialog::on_checkBoxHideWalls_stateChanged(int arg1) {
	m_hideWalls = arg1;
	// execute the search again
	checkConstraints();
}




void SVAcousticConstraintsCheckDialog::on_checkBoxHideCeilings_stateChanged(int arg1) {
	m_hideCeilings = arg1;
	// execute the search again
	checkConstraints();
}

