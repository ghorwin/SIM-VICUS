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
	m_ui->tableWidgetWalls->setColumnCount(7);
	m_ui->tableWidgetWalls->setHorizontalHeaderLabels(QStringList()  << tr("Acoustic template A")<< tr("Acoustic Template B")<< tr("Acoustic component")<< tr("actual air sound value")<<tr("normal constraints")<<tr("advanced constraints")<<  tr("button"));
	SVStyle::formatDatabaseTableView(m_ui->tableWidgetWalls);
	m_ui->tableWidgetWalls->setSortingEnabled(false);
	m_ui->tableWidgetWalls->resizeColumnsToContents();



	//for ceilings
	m_ui->tableWidgetCeilings->setColumnCount(7);
	m_ui->tableWidgetCeilings->setHorizontalHeaderLabels(QStringList()  << tr("Acoustic template above")<< tr("Acoustic Template beneth")<< tr("Acoustic component")<< tr("actual impact sound value")<<tr("normal constraints")<<tr("advanced constraints")<<  tr("button"));
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

	QString undoText = "Select surfaces of acoustic component.";

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(undoText, SVUndoTreeNodeState::SelectedState, surfIds, true);
	undo->push();

	SVViewStateHandler::instance().m_geometryView->resetCamera(Vic3D::SceneView::CP_FindSelection);

	close();


}

void SVAcousticConstraintsCheckDialog::checkConstraints(){

	// all the table entries for walls and doors
	std::vector<tableEntry> wallTes;

	// all the table entries for ceilings
	std::vector<tableEntry> ceilingTes;

	auto ii = project().m_componentInstances;

	// iterate over all component instances
	for(const VICUS::ComponentInstance & ci : project().m_componentInstances){
		// struct that holds relevant data for the table + init
		tableEntry te;

		// initialize with no constraints specified, might be overwritten later
		te.normalConstraintViolated = VI_No_Constraint;
		te.advancedConstraintViolated = VI_No_Constraint;

		// skip all that dont have a valid acoustic component
		if(ci.m_idAcousticComponent == VICUS::INVALID_ID)
			continue;
		// skip all that dont have two surfaces
		if(ci.m_idSideASurface == VICUS::INVALID_ID || ci.m_idSideBSurface == VICUS::INVALID_ID || ci.m_sideASurface ==nullptr || ci.m_sideBSurface == nullptr)
			continue;

		// save the surface ids
		te.surfaceAId = ci.m_idSideASurface;
		te.surfaceBId = ci.m_idSideBSurface;

		// search the rooms of the corresponding surfaces
		const VICUS::Room * roomA = dynamic_cast<VICUS::Room *>(ci.m_sideASurface->m_parent);
		const VICUS::Room * roomB = dynamic_cast<VICUS::Room *>(ci.m_sideBSurface->m_parent);
		if(roomA == nullptr || roomB == nullptr)
			continue;

		// skip if they dont have the same acoustic building type
		if(roomA->m_acousticBuildingTypeId != roomB->m_acousticBuildingTypeId)
			continue;

		// check if they are in the same structural unit or not
		bool sameStructuralUnit = (roomA->m_structuralUnit == roomB->m_structuralUnit && roomB->m_structuralUnit != nullptr);

		// get the ids of the acoustic template of the corresponding room
		IDType acousticTemplateAId = roomA->m_idAcousticTemplate;
		IDType acousticTemplateBId = roomB->m_idAcousticTemplate;

		// skip if one of them does not have an acoustic template
		if(acousticTemplateAId == VICUS::INVALID_ID || acousticTemplateBId == VICUS::INVALID_ID)
			continue;

		const SVDatabase & db = SVSettings::instance().m_db;

		// fill table Entry with acoustic template info
		const VICUS::AcousticTemplate * tempA = db.m_acousticTemplates[acousticTemplateAId];
		const VICUS::AcousticTemplate * tempB = db.m_acousticTemplates[acousticTemplateBId];
		te.acousticTemplateAInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(tempA->m_displayName)).arg(tempA->m_id);
		te.acousticTemplateBInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(tempB->m_displayName)).arg(tempB->m_id);

		// check if the part type, if its a ceiling
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

		// if we find a match we save the te to the vector in the end
		bool addTableEntryToVector = false;
		for(std::pair<unsigned int, VICUS::AcousticReferenceComponent>  refEntry: db.m_acousticReferenceComponents){
			VICUS::AcousticReferenceComponent refComp = refEntry.second;
			bool isMatchingRef = false;
			if(isCeiling){
				// only check in one direction
				if(refComp.m_idAcousticTemplateA == acousticTemplateAId &&
						refComp.m_idAcousticTemplateB == acousticTemplateBId){
					// the reference component type must also be for ceilings
					if(refComp.m_type == VICUS::AcousticReferenceComponent::CT_Ceiling){
						isMatchingRef = true;
					}
				}
			} else {
				// allow both directions
				if((refComp.m_idAcousticTemplateA == acousticTemplateAId &&
						refComp.m_idAcousticTemplateB == acousticTemplateBId) ||
						(refComp.m_idAcousticTemplateA == acousticTemplateAId &&
						 refComp.m_idAcousticTemplateB == acousticTemplateBId)){
					// the reference component type must also be for either walls or doors
					if(refComp.m_type == VICUS::AcousticReferenceComponent::CT_Door ||
							refComp.m_type == VICUS::AcousticReferenceComponent::CT_Wall){
						isMatchingRef = true;
					}
				}
			}
			//TODO How to handle Bulding Type Stairs

			if(isMatchingRef){

				addTableEntryToVector = true;

				// check wether the acoustic component satisfies the required limit
				double impactSoundLimit;
				double airSoundLimit;
				if(sameStructuralUnit){
					airSoundLimit = refComp.m_airborneSoundOneStructureUnit;
					impactSoundLimit = refComp.m_impactSoundOneStructureUnit;
				} else {
					airSoundLimit = refComp.m_airborneSoundDifferentStructureUnits;
					impactSoundLimit = refComp.m_impactSoundDifferentStructureUnits;
				}
				// retrieve the acoustic component to check if it fullfills the limits
				const VICUS::AcousticComponent * acComp = db.m_acousticComponents[ci.m_idAcousticComponent];

				// points to the normal or advanced violation info depending on the requirement type of the ref component
				ViolationInfo * normalOrAdvancedInfo = (refComp.m_requirmentType == VICUS::AcousticReferenceComponent::RT_Normal) ?
							&(te.normalConstraintViolated) :
							&(te.advancedConstraintViolated);
				QString * normalOrAdvancedLimitValue = (refComp.m_requirmentType == VICUS::AcousticReferenceComponent::RT_Normal) ?
							&(te.expectedNormalLimit) :
							&(te.expectedAdvancedLimit);

				// fill te struct with acoustic component info
				te.acousticComponentInfo = QString("%1 [%2]").arg(QtExt::MultiLangString2QString(acComp->m_displayName)).arg(acComp->m_id);

				if(isCeiling){
					te.isImpact = true;
					if(acComp->m_airSoundResistenceValue < impactSoundLimit)
						*normalOrAdvancedInfo = VI_Not_Violated;
					else
						*normalOrAdvancedInfo = VI_Violated;
					te.actualValue = QString("%1 dB").arg(acComp->m_impactSoundValue);
					*normalOrAdvancedLimitValue = QString("%1 dB").arg(impactSoundLimit);
				} else {
					te.isImpact = false;
					if(acComp->m_airSoundResistenceValue < airSoundLimit)
						*normalOrAdvancedInfo = VI_Not_Violated;
					else
						*normalOrAdvancedInfo = VI_Violated;
					te.actualValue = QString("%1 dB").arg(acComp->m_airSoundResistenceValue);
					*normalOrAdvancedLimitValue = QString("%1 dB").arg(airSoundLimit);
				}

			}
		}
		if(addTableEntryToVector){
			if(isCeiling) {
				ceilingTes.push_back(te);
			} else {
				wallTes.push_back(te);
			}
		}
	}

	//fill the walls table with the collected wall table entries
	m_ui->tableWidgetWalls->blockSignals(true);
	m_ui->tableWidgetWalls->setRowCount(0);
	int row=0;

	for(tableEntry &  te : wallTes){

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

