/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#include "SVPropVertexListWidget.h"
#include "ui_SVPropVertexListWidget.h"

#include <QMessageBox>
#include <QInputDialog>

#include <IBK_physics.h>

#include <IBKMK_Vector3D.h>
#include <IBKMK_Triangulation.h>
#include <IBKMK_3DCalculations.h>
#include <IBKMK_2DCalculations.h>

#include <QtExt_LanguageHandler.h>
#include <SV_Conversions.h>

#include <VICUS_Project.h>
#include <VICUS_KeywordList.h>
#include <VICUS_utilities.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVUndoAddSurface.h"
#include "SVUndoModifySurfaceGeometry.h"
#include "SVGeometryView.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVUndoAddBuilding.h"
#include "SVUndoAddBuildingLevel.h"
#include "SVUndoAddZone.h"

#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"


SVPropVertexListWidget::SVPropVertexListWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropVertexListWidget)
{
	m_ui->setupUi(this);
	m_ui->verticalLayout->setMargin(0);
	for (int i=0; i<4; ++i)
		m_ui->stackedWidget->widget(i)->layout()->setMargin(0);

	m_ui->lineEditZoneHeight->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(),
									tr("Zone height in [m]."),true, true);

	SVViewStateHandler::instance().m_propVertexListWidget = this;

	on_radioButtonRoofHeight_toggled(m_ui->radioButtonRoofHeight->isChecked());
	on_checkBoxFlapTile_toggled(m_ui->checkBoxFlapTile->isChecked());

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropVertexListWidget::onModified);

	connect(m_ui->pushButtonCancel1, &QPushButton::clicked, this, &SVPropVertexListWidget::onCancel);
	connect(m_ui->pushButtonCancel2, &QPushButton::clicked, this, &SVPropVertexListWidget::onCancel);
	connect(m_ui->pushButtonCancel3, &QPushButton::clicked, this, &SVPropVertexListWidget::onCancel);
	connect(m_ui->pushButtonCancel4, &QPushButton::clicked, this, &SVPropVertexListWidget::onCancel);

	connect(m_ui->toolButtonAddBuilding2, &QToolButton::clicked, this, &SVPropVertexListWidget::on_toolButtonAddBuilding_clicked);
	connect(m_ui->toolButtonAddBuilding3, &QToolButton::clicked, this, &SVPropVertexListWidget::on_toolButtonAddBuilding_clicked);

	connect(m_ui->toolButtonAddBuildingLevel2, &QToolButton::clicked, this, &SVPropVertexListWidget::on_toolButtonAddBuildingLevel_clicked);
	connect(m_ui->toolButtonAddBuildingLevel3, &QToolButton::clicked, this, &SVPropVertexListWidget::on_toolButtonAddBuildingLevel_clicked);

	connect(m_ui->toolButtonEditSubSurfComponents, &QToolButton::clicked, this, &SVPropVertexListWidget::onEditSubSurfaceComponents);

	connect(m_ui->toolButtonEditComponents1, &QToolButton::clicked, this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents2, &QToolButton::clicked, this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents3, &QToolButton::clicked, this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents4, &QToolButton::clicked, this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents5, &QToolButton::clicked, this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents7, &QToolButton::clicked, this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents8, &QToolButton::clicked, this, &SVPropVertexListWidget::onEditComponents);

	updateButtonStates(); // see class comment

	// when we have a project loaded, initialize combo boxes accordingly
	if (SVProjectHandler::instance().isValid()) {

		updateBuildingComboBox(m_ui->comboBoxBuilding);
		updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel, m_ui->comboBoxBuilding);
		updateZoneComboBox(m_ui->comboBoxZone, m_ui->comboBoxBuildingLevel);

		updateBuildingComboBox(m_ui->comboBoxBuilding2);
		updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel2, m_ui->comboBoxBuilding2);

		updateBuildingComboBox(m_ui->comboBoxBuilding3);
		updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel3, m_ui->comboBoxBuilding3);

		updateSurfaceComboBox(m_ui->comboBoxSurface);
	}
}



SVPropVertexListWidget::~SVPropVertexListWidget() {
	delete m_ui;
}


void SVPropVertexListWidget::setup(int newGeometryType) {
	// switch to vertex list widget
	m_ui->stackedWidget->setCurrentIndex(0);
	// clear vertex table widget and disable "delete" and "finish" buttons
	m_ui->tableWidgetVertexes->setRowCount(0);
	m_ui->pushButtonCompletePolygon->setEnabled(false);
	m_ui->pushButtonDeleteLast->setEnabled(false);
	m_ui->pushButtonDeleteSelected->setEnabled(false);

	// initialize new geometry object
	switch ((Vic3D::NewGeometryObject::NewGeometryMode)newGeometryType) {
		case Vic3D::NewGeometryObject::NGM_Rect :
			SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_Rect);
		break;

		case Vic3D::NewGeometryObject::NGM_Polygon :
		case Vic3D::NewGeometryObject::NGM_Zone :
		case Vic3D::NewGeometryObject::NGM_Roof :
			SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_Polygon);
		break;

		case Vic3D::NewGeometryObject::NUM_NGM: ; // just for the compiler
	}
	m_geometryMode = newGeometryType;
}




void SVPropVertexListWidget::updateComponentComboBoxes() {
	updateComponentComboBox(m_ui->comboBoxComponent, -1);
	updateComponentComboBox(m_ui->comboBoxComponentWalls, 0);
	updateComponentComboBox(m_ui->comboBoxComponentFloor, 1);
	updateComponentComboBox(m_ui->comboBoxComponentCeiling, 2);

	updateComponentComboBox(m_ui->comboBoxComponentWall3, 0);
	updateComponentComboBox(m_ui->comboBoxComponentFloor3, 1);
	updateComponentComboBox(m_ui->comboBoxComponentRoof3, 2);

	updateSubSurfaceComponentComboBox(m_ui->comboBoxSubSurfComponent);
}


void SVPropVertexListWidget::addVertex(const IBKMK::Vector3D & p) {
	// Note: the vertex is already in the NewGeometryObject, we only
	//       modify the table widget and update the button enabled states
	int row = m_ui->tableWidgetVertexes->rowCount();
	m_ui->tableWidgetVertexes->setRowCount(row + 1);
	QTableWidgetItem * item = new QTableWidgetItem(QString("%1").arg(row+1));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	m_ui->tableWidgetVertexes->setItem(row,0,item);
	item = new QTableWidgetItem(QString("%L1 | %L2 | %L3").arg(p.m_x).arg(p.m_y).arg(p.m_z));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	m_ui->tableWidgetVertexes->setItem(row,1,item);

	m_ui->pushButtonDeleteLast->setEnabled(true);

	// we may now switch to another mode
	m_ui->pushButtonCompletePolygon->setEnabled(SVViewStateHandler::instance().m_newGeometryObject->canComplete());
}


void SVPropVertexListWidget::removeVertex(unsigned int idx) {
	// Note: the vertex has already been removed in the NewGeometryObject, we only
	//       modify the table widget and update the button enabled states
	int rows = m_ui->tableWidgetVertexes->rowCount();
	Q_ASSERT(rows > 0);
	Q_ASSERT((int)idx < m_ui->tableWidgetVertexes->rowCount());
	// now remove selected row from table widget
	m_ui->tableWidgetVertexes->removeRow((int)idx);
	m_ui->pushButtonDeleteLast->setEnabled(rows > 1);

	// disable/enable "Complete polygon" button, depending on wether the surface is complete
	m_ui->pushButtonCompletePolygon->setEnabled(SVViewStateHandler::instance().m_newGeometryObject->canComplete());

	// repaint the scene
	SVViewStateHandler::instance().m_geometryView->refreshSceneView();
}


void SVPropVertexListWidget::setZoneHeight(double dist) {
	m_ui->lineEditZoneHeight->blockSignals(true);
	m_ui->lineEditZoneHeight->setText(QString("%L1").arg(dist));
	m_ui->lineEditZoneHeight->blockSignals(false);
	if (!SVViewStateHandler::instance().m_newGeometryObject->m_interactiveZoneExtrusionMode)
		m_ui->pushButtonPickZoneHeight->setChecked(false);
}


bool SVPropVertexListWidget::completePolygonIfPossible() {
	switch (m_ui->stackedWidget->currentIndex()) {
		case 0 :
			if (m_ui->pushButtonCompletePolygon->isEnabled()) {
				m_ui->pushButtonCompletePolygon->click();
				return true;
			}
		break;

		case 1 :
			m_ui->pushButtonCreateSurface->click();
			return true;

		case 2 :
			m_ui->pushButtonCreateZone->click();
			return true;

		case 3 :
			m_ui->pushButtonCreateRoof->click();
			return true;
	}
	return false;
}


// *** SLOTS ***

void SVPropVertexListWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;

	// we always update our combo boxes whenever anything related to the building topology has changed
	switch (mod) {
		case SVProjectHandler::BuildingTopologyChanged:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::AllModified:
			updateBuildingComboBox(m_ui->comboBoxBuilding);
			updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel, m_ui->comboBoxBuilding);
			updateZoneComboBox(m_ui->comboBoxZone, m_ui->comboBoxBuildingLevel);

			updateBuildingComboBox(m_ui->comboBoxBuilding2);
			updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel2, m_ui->comboBoxBuilding2);

			updateBuildingComboBox(m_ui->comboBoxBuilding3);
			updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel3, m_ui->comboBoxBuilding3);

			updateSurfaceComboBox(m_ui->comboBoxSurface);
		break;
		default: ;// nothing to be done here
	}

	updateButtonStates();
	// We only need to handle changes of the building topology, in all other cases
	// the "create new geometry" action is aborted and the widget will be hidden.
	if (mod != SVProjectHandler::BuildingTopologyChanged) {
		// clear the new geometry object
		SVViewStateHandler::instance().m_newGeometryObject->clear();
		// and reset view state, if we are still in vertex list mode
		SVViewState vs = SVViewStateHandler::instance().viewState();
		if (vs.m_propertyWidgetMode == SVViewState::PM_VertexList) {
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
			vs.m_propertyWidgetMode = SVViewState::PM_AddEditGeometry;
			// reset locks
			vs.m_locks = SVViewState::NUM_L;

			// take xy plane out of snap option mask
			vs.m_snapEnabled = true;
			// now tell all UI components to toggle their view state
			SVViewStateHandler::instance().setViewState(vs);
		}
	}
}


void SVPropVertexListWidget::on_pushButtonDeleteLast_clicked() {
	// remove last vertex from polygon
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	po->removeLastVertex();
}


void SVPropVertexListWidget::on_tableWidgetVertexes_itemSelectionChanged() {
	m_ui->pushButtonDeleteSelected->setEnabled( m_ui->tableWidgetVertexes->currentRow() != -1);
}


void SVPropVertexListWidget::on_pushButtonDeleteSelected_clicked() {
	int currentRow = m_ui->tableWidgetVertexes->currentRow();
	Q_ASSERT(currentRow != -1);
	// remove selected vertex from polygon
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	po->removeVertex((unsigned int)currentRow); // this will in turn call removeVertex() above
}


void SVPropVertexListWidget::on_pushButtonCompletePolygon_clicked() {
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	// switch to different stacked widget, depending on geometry to be created
	switch ((Vic3D::NewGeometryObject::NewGeometryMode)m_geometryMode) {
		case Vic3D::NewGeometryObject::NGM_Rect:
		case Vic3D::NewGeometryObject::NGM_Polygon:
			m_ui->stackedWidget->setCurrentIndex(1);
			updateComponentComboBoxes(); // update all component combo boxes in surface page
			m_ui->lineEditName->setText(tr("Surface"));
			po->m_passiveMode = true; // disallow changes to surface geometry
			updateButtonStates();
		break;

		case Vic3D::NewGeometryObject::NGM_Zone:
			m_ui->stackedWidget->setCurrentIndex(2);

			updateComponentComboBoxes(); // update all component combo boxes in zone page
			po->setNewGeometryMode(Vic3D::NewGeometryObject::NGM_Zone);
			// transfer zone height into line edit, if we have already a building level defined
			on_comboBoxBuildingLevel2_currentIndexChanged(0); // index argument does not matter, not used
			on_lineEditZoneHeight_editingFinishedSuccessfully();
			m_ui->lineEditNameZone->setText(tr("Room"));
		break;

		case Vic3D::NewGeometryObject::NGM_Roof: {
			m_ui->stackedWidget->setCurrentIndex(3);
			updateComponentComboBoxes(); // update all component combo boxes in roof page
			const VICUS::PlaneGeometry & pg = po->planeGeometry();
			if (pg.polygon3D().type() != IBKMK::Polygon2D::T_Rectangle)
				m_ui->comboBoxRoofType->setCurrentIndex(4); // not a rectangle, select complex roof
			po->setNewGeometryMode(Vic3D::NewGeometryObject::NGM_Roof);
			m_ui->lineEditNameRoof->setText(tr("Roof"));

			// get floor polyline from roof and save this for later
			Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;

			if (po->planeGeometry().polygon3D().vertexes().empty())
				return; // TODO Dirk, error handling? can this actually happen? If not, make it an assert

			m_roofPolygon = po->planeGeometry().polygon3D().vertexes();
			m_currentIdxOfStartpoint = 0;
			updateRoofGeometry();
		}
		break;
		case Vic3D::NewGeometryObject::NUM_NGM: ; // just for the compiler
	}
}


void SVPropVertexListWidget::onCancel() {
	// reset new polygon object, so that it won't be drawn anylonger
	SVViewStateHandler::instance().m_newGeometryObject->clear();
	// signal, that we are no longer in "add vertex" mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	vs.m_propertyWidgetMode = SVViewState::PM_AddEditGeometry;
	// reset locks
	vs.m_locks = SVViewState::NUM_L;

	// take xy plane out of snap option mask
	vs.m_snapEnabled = true;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropVertexListWidget::onEditComponents() {
	// ask main window to show database dialog, afterwards update component combos
	SVMainWindow::instance().on_actionDBComponents_triggered();
	// Note: SVMainWindow::instance().on_actionDBComponents_triggered() calls updateComponentCombos() itself, so
	//       no need to call this here
}

void SVPropVertexListWidget::onEditSubSurfaceComponents() {
	// ask main window to show database dialog, afterwards update component combos
	SVMainWindow::instance().on_actionDBSubSurfaceComponents_triggered();
	// Note: SVMainWindow::instance().on_actionDBComponents_triggered() calls updateComponentCombos() itself, so
	//       no need to call this here
}


void SVPropVertexListWidget::on_toolButtonAddBuilding_clicked() {
	std::set<QString> existingNames;
	for (const VICUS::Building & b : project().m_buildings)
		existingNames.insert(b.m_displayName);
	QString defaultName = VICUS::uniqueName(tr("Building"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building"), tr("New building name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;
	// modify project
	VICUS::Building b;
	b.m_id = project().nextUnusedID();
	b.m_displayName = text;
	SVUndoAddBuilding * undo = new SVUndoAddBuilding(tr("Adding building '%1'").arg(b.m_displayName), b, true);
	undo->push(); // this will update our combo boxes and also call updateButtonStates() indirectly

	// now also select the matching item
	if (sender() == m_ui->toolButtonAddBuilding)
		reselectById(m_ui->comboBoxBuilding, (int)b.m_id);
	else if (sender() == m_ui->toolButtonAddBuilding2)
		reselectById(m_ui->comboBoxBuilding2, (int)b.m_id);
	else if (sender() == m_ui->toolButtonAddBuilding3)
		reselectById(m_ui->comboBoxBuilding3, (int)b.m_id);
	else {
		Q_ASSERT(false);
	}
}


void SVPropVertexListWidget::on_toolButtonAddBuildingLevel_clicked() {
	QComboBox * buildingCombo = nullptr;
	QComboBox * buildingLevelCombo;
	if (sender() == m_ui->toolButtonAddBuildingLevel) {
		buildingCombo = m_ui->comboBoxBuilding;
		buildingLevelCombo = m_ui->comboBoxBuildingLevel;
	}
	else if (sender() == m_ui->toolButtonAddBuildingLevel2) {
		buildingCombo = m_ui->comboBoxBuilding2;
		buildingLevelCombo = m_ui->comboBoxBuildingLevel2;
	}
	else if (sender() == m_ui->toolButtonAddBuildingLevel3) {
		buildingCombo = m_ui->comboBoxBuilding3;
		buildingLevelCombo = m_ui->comboBoxBuildingLevel3;
	}
	else {
		Q_ASSERT(false);
	}

	// get currently selected building
	unsigned int buildingID = buildingCombo->currentData().toUInt();
	const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(project().objectById(buildingID));
	Q_ASSERT(b != nullptr);

	std::set<QString> existingNames;
	for (const VICUS::BuildingLevel & bl : b->m_buildingLevels)
		existingNames.insert(bl.m_displayName);
	QString defaultName = VICUS::uniqueName(tr("Level"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building level"), tr("New building level/floor name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;

	// modify project
	VICUS::BuildingLevel bl;
	bl.m_id = project().nextUnusedID();
	bl.m_displayName = text;
	SVUndoAddBuildingLevel * undo = new SVUndoAddBuildingLevel(tr("Adding building level '%1'").arg(bl.m_displayName), buildingID, bl, true);
	undo->push(); // this will update our combo boxes and also call updateButtonStates() indirectly

	// now also select the matching item
	reselectById(buildingLevelCombo, (int)bl.m_id);
}


void SVPropVertexListWidget::on_toolButtonAddZone_clicked() {
	// get currently selected building
	unsigned int buildingLevelID = m_ui->comboBoxBuildingLevel->currentData().toUInt();
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(project().objectById(buildingLevelID));
	Q_ASSERT(bl != nullptr);

	std::set<QString> existingNames;
	for (const VICUS::Room & r : bl->m_rooms)
		existingNames.insert(r.m_displayName);
	QString defaultName = VICUS::uniqueName(tr("Room"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add room/zone"), tr("New room/zone name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;

	// modify project
	VICUS::Room r;
	r.m_id = project().nextUnusedID();
	r.m_displayName = text;
	SVUndoAddZone * undo = new SVUndoAddZone(tr("Adding building zone '%1'").arg(r.m_displayName), buildingLevelID, r, true);
	undo->push(); // this will update our combo boxes and also call updateButtonStates() indirectly

	// now also select the matching item
	reselectById(m_ui->comboBoxZone, (int)r.m_id);
}


void SVPropVertexListWidget::on_checkBoxAnnonymousGeometry_stateChanged(int /*arg1*/) {
	updateButtonStates();
}


void SVPropVertexListWidget::on_comboBoxBuilding_currentIndexChanged(int /*index*/) {
	updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel, m_ui->comboBoxBuilding);
	updateZoneComboBox(m_ui->comboBoxZone, m_ui->comboBoxBuildingLevel);
	updateButtonStates();
}


void SVPropVertexListWidget::on_comboBoxBuildingLevel_currentIndexChanged(int /*index*/) {
	updateZoneComboBox(m_ui->comboBoxZone, m_ui->comboBoxBuildingLevel);
	if (m_ui->comboBoxBuildingLevel->count() == 0)
		return;
	unsigned int buildingLevelID = m_ui->comboBoxBuildingLevel->currentData().toUInt();
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(project().objectById(buildingLevelID));
	// also transfer nominal height into zone-height line edit
	if (bl != nullptr) {
		m_ui->lineEditZoneHeight->setValue(bl->m_height);
		// only trigger zone height editing finished, when we are in new vertex mode
		// Mind: widget may be hidden
		SVViewState vs = SVViewStateHandler::instance().viewState();
		if (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex)
			on_lineEditZoneHeight_editingFinishedSuccessfully();
	}
	updateButtonStates();
}


void SVPropVertexListWidget::on_pushButtonCreateSurface_clicked() {

	if (m_ui->lineEditName->text().trimmed().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Please enter a descriptive name!"));
		m_ui->lineEditName->selectAll();
		m_ui->lineEditName->setFocus();
		return;
	}
	// depending on the type of geometry that's being created,
	// perform additional checks
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	IBK_ASSERT(po->m_passiveMode == true);
	IBK_ASSERT(po->newGeometryMode() == Vic3D::NewGeometryObject::NGM_Rect ||
			   po->newGeometryMode() == Vic3D::NewGeometryObject::NGM_Polygon);

	// compose a surface object based on the current content of the new polygon object
	VICUS::Surface s;
	s.m_id = project().nextUnusedID();
	s.m_displayName = m_ui->lineEditName->text().trimmed();
	s.setPolygon3D( po->planeGeometry().polygon3D() );

	// we need all properties, unless we create annonymous geometry
	if (m_ui->checkBoxAnnonymousGeometry->isChecked()) {
		s.m_displayColor = s.m_color = QColor("#206000");
		// modify project
		SVUndoAddSurface * undo = new SVUndoAddSurface(tr("Added surface '%1'").arg(s.m_displayName), s, 0);
		undo->push();
	}
	else {
		// we differentiate between a normal surface and a sub surface that is added
		if (createSubSurfaceGeometry() ) {
			// project all the points onto the selected surface
			// we also need to check that the polygon can be projected
			// process all selected surface
			std::vector<VICUS::Surface> modSurfaces; // here, we store only the modified surfaces
			if (m_ui->comboBoxSurface->currentIndex() == -1) {
				QMessageBox::critical(this, QString(), tr("First select a surface in which the sub surface is added!"));
				return;
			}

			unsigned int surfId = m_ui->comboBoxSurface->currentData().toUInt();
			const VICUS::Surface * surf = dynamic_cast<const VICUS::Surface*>(project().objectById(surfId));

			// we make a surface copy
			VICUS::Surface newSurf(*surf);

			if(surf == nullptr){
				QMessageBox::critical(this, QString(), tr("Surface ID is not valid!"));
				return;
			}

			// we take a copy of our old subsurfaces
			std::vector<VICUS::SubSurface> subSurfs = newSurf.subSurfaces();

			// populate a vector with existing and remaining subsurface component instances
			std::vector<VICUS::SubSurfaceComponentInstance> subSurfaceComponentInstances = project().m_subSurfaceComponentInstances;

			const IBKMK::Vector3D &offset = newSurf.geometry().offset();
			const IBKMK::Vector3D &normal = newSurf.geometry().normal();

			VICUS::SubSurface newSubsurface;
			newSubsurface.m_id = project().nextUnusedID();
			newSubsurface.m_displayName = m_ui->lineEditName->text().trimmed();
			newSubsurface.m_color = QColor(96,96,255,64);
#if POLYGON2D
			// we make new projection
			for (unsigned int i=0; i<s.geometry().polygon().vertexes().size(); ++i) {
				IBKMK::Vector3D projectedPoint;
				const IBKMK::Vector3D &p = s.geometry().polygon().vertexes()[i];
				// we need to be sure that the point is in our plane
				IBKMK::pointProjectedOnPlane(offset, normal, p, projectedPoint);
				// now we take the projected point
				IBKMK::Vector2D::point2D subSurfPoint;
				IBKMK::planeCoordinates(offset, newSurf.geometry().localX(), newSurf.geometry().localY(), projectedPoint, subSurfPoint.m_x, subSurfPoint.m_y);

				newSubsurface.m_polygon2D.addVertex(subSurfPoint);
			}

			// also create subsurface component instances
			// but only if we have a valid subsurface component selected
			if (m_ui->comboBoxSubSurfComponent->count() != 0 &&
				m_ui->comboBoxSubSurfComponent->currentIndex() != -1)
			{
				VICUS::SubSurfaceComponentInstance subInstance;
				subInstance.m_id = project().nextUnusedID();
				subInstance.m_idSubSurfaceComponent = m_ui->comboBoxSubSurfComponent->currentData().toUInt();
				subInstance.m_idSideASurface = newSubsurface.m_id;
				//subInstance.m_sideASubSurface = &newSubsurface;
				subInstance.m_idSideBSurface = VICUS::INVALID_ID; // currently, all our new windows are outside windows
				subSurfaceComponentInstances.push_back(subInstance);
			}

			subSurfs.push_back(newSubsurface);
			newSurf.setSubSurfaces(subSurfs);
			modSurfaces.push_back(newSurf);

			SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Added sub-surfaces/windows"),
				modSurfaces, &subSurfaceComponentInstances);
			undo->push();
#endif
		}
		else {

			// we need inputs for room (if there is a room, there is also a building and level)
			if (m_ui->comboBoxZone->currentIndex() == -1) {
				QMessageBox::critical(this, QString(), tr("First select a zone to add the surface to!"));
				return;
			}
			unsigned int zoneUUID = m_ui->comboBoxZone->currentData().toUInt();
			Q_ASSERT(zoneUUID != 0);

			s.initializeColorBasedOnInclination(); // set color based on orientation
			s.m_color = s.m_displayColor;
			// also store component information
			VICUS::ComponentInstance compInstance;
			compInstance.m_id = VICUS::uniqueId(project().m_componentInstances);
			compInstance.m_idComponent = m_ui->comboBoxComponent->currentData().toUInt();
			// for now we assume that the zone's surface is connected to the b-side of the component
			compInstance.m_idSideBSurface = s.m_id;
			// modify project
			SVUndoAddSurface * undo = new SVUndoAddSurface(tr("Added surface '%1'").arg(s.m_displayName), s, zoneUUID, &compInstance);
			undo->push();
		}
	}

	// reset lock axis
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_locks = SVViewState::NUM_L;
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropVertexListWidget::on_lineEditZoneHeight_editingFinishedSuccessfully() {
	// Guard against call when aborting/focus is lost during undo!
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	if (po->newGeometryMode() != Vic3D::NewGeometryObject::NGM_Zone)
		return;

	// read entered line height and if valid set new height in scene view (and move local coordinate system accordingly)
	double val = m_ui->lineEditZoneHeight->value();
	po->setZoneHeight(val);
	// we need to trigger a redraw here
	SVViewStateHandler::instance().m_geometryView->refreshSceneView();
}


void SVPropVertexListWidget::on_pushButtonPickZoneHeight_clicked() {
	// enable interactive zone extrusion mode
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	// check if interactive mode is already enabled
	po->m_interactiveZoneExtrusionMode = !po->m_interactiveZoneExtrusionMode;
}


void SVPropVertexListWidget::on_comboBoxBuilding2_currentIndexChanged(int /*index*/) {
	updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel2, m_ui->comboBoxBuilding2);
	updateButtonStates();
}


void SVPropVertexListWidget::on_comboBoxBuildingLevel2_currentIndexChanged(int /*index*/) {
	if (m_ui->comboBoxBuildingLevel2->count() == 0)
		return;
	unsigned int buildingLevelID = m_ui->comboBoxBuildingLevel2->currentData().toUInt();
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(project().objectById(buildingLevelID));
	// also transfer nominal height into zone-height line edit
	if (bl != nullptr) {
		m_ui->lineEditZoneHeight->setValue(bl->m_height);
		// only trigger zone height editing finished, when we are in new vertex mode
		// Mind: widget may be hidden
		SVViewState vs = SVViewStateHandler::instance().viewState();
		if (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex)
			on_lineEditZoneHeight_editingFinishedSuccessfully();
	}
	updateButtonStates();
}


void SVPropVertexListWidget::on_pushButtonCreateZone_clicked() {
	// we need a building level
	if (m_ui->comboBoxBuildingLevel2->currentIndex() == -1) {
		QMessageBox::critical(this, QString(), tr("First select a building level to add the zone/room to!"));
		return;
	}
	// tricky part starts now
	// 1. we need to get a list of surfaces and make their normal vectors point outwards
	// 2. we need to assign colors to the surfaces and default components based on
	//    inclination
	// 3. we need to create an undo-action

	// get floor and ceiling polygons from geometry object

	// take the polygon
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	VICUS::Polygon3D floor(po->planeGeometry().polygon3D());
	IBK_ASSERT(po->generatedGeometry().size() == 1);
	VICUS::Polygon3D ceiling(po->generatedGeometry()[0].polygon3D());
	// Note: both polygons still have the same normal vector!

	// compute offset vector
	IBKMK::Vector3D offset = ceiling.vertexes()[0] - floor.vertexes()[0];
	// now check if ceiling is offset in same direction as normal vector of floor plane?
	double dotProduct = offset.scalarProduct(floor.normal());
	if (dotProduct > 0) {
		// same direction, we need to reverse floor polygon
		floor.flip();
	}
	else {
		// opposite direction, we need to reverse the ceiling polygon
		ceiling.flip();
	}

	std::vector<VICUS::ComponentInstance> componentInstances;
	unsigned int nextID = project().nextUnusedID();
	VICUS::Room r;
	r.m_id = nextID;
	r.m_displayName = m_ui->lineEditNameZone->text().trimmed();
	// now we can create the surfaces for top and bottom
	// compose a surface object based on the current content of the new polygon object
	VICUS::Surface sFloor;
	sFloor.m_displayName = tr("Floor");
	sFloor.m_id = ++nextID;
	VICUS::Surface sCeiling;
	sCeiling.m_displayName = tr("Ceiling");
	sCeiling.m_id = ++nextID;
	// if the ceiling has a normal vector pointing up, we take it as ceiling, otherwise it's going to be the floor
	if (IBKMK::Vector3D(0,0,1).scalarProduct(ceiling.normal()) > 0) {
		sCeiling.setPolygon3D(ceiling);
		sFloor.setPolygon3D(floor);
	}
	else {
		sCeiling.setPolygon3D(floor);
		sFloor.setPolygon3D(ceiling);
	}

	sFloor.initializeColorBasedOnInclination();
	// get the smallest yet free ID for component instances/construction instances
	unsigned int conInstID = VICUS::largestUniqueId(project().m_componentInstances);
	// Note: surface is attached to "Side A"
	componentInstances.push_back(VICUS::ComponentInstance(++conInstID,
		 m_ui->comboBoxComponentFloor->currentData().toUInt(), sFloor.m_id, VICUS::INVALID_ID));

	sCeiling.initializeColorBasedOnInclination();
	// Note: surface is attached to "Side A"
	componentInstances.push_back(VICUS::ComponentInstance(++conInstID,
		 m_ui->comboBoxComponentCeiling->currentData().toUInt(), sCeiling.m_id, VICUS::INVALID_ID));

	r.m_surfaces.push_back(sFloor);
	r.m_surfaces.push_back(sCeiling);

	// now loop around the circle and create planes for wall segments
	// we take the floor polygon

	unsigned int nVert = floor.vertexes().size();
	unsigned int wallComponentID = m_ui->comboBoxComponentWalls->currentData().toUInt();
	for (unsigned int i=0; i<nVert; ++i) {
		// mind the winding order
		// when looked from above, floor vertexes go clock-wise,
		// and ceiling vertices go anti-clockwise
		unsigned int vIdx2 = (i+1) % nVert;

		IBKMK::Vector3D p0 = floor.vertexes()[ vIdx2 ];
		IBKMK::Vector3D p1 = floor.vertexes()[ i ];
		IBKMK::Vector3D p2 = floor.vertexes()[ vIdx2 ] + offset;	//take offset as last point for rectangle; rounding errors by vector-sum?

		VICUS::Surface sWall;
		sWall.m_id = ++nextID;
		sWall.m_displayName = tr("Wall %1").arg(i+1);
		sWall.setPolygon3D( VICUS::Polygon3D(VICUS::Polygon2D::T_Rectangle, p0, p1, p2) );
		sWall.initializeColorBasedOnInclination();
		// wall surface is attached to "Side A"
		componentInstances.push_back(VICUS::ComponentInstance(++conInstID,
													  wallComponentID, sWall.m_id, VICUS::INVALID_ID));

		r.m_surfaces.push_back(sWall);
	}

	double area = sFloor.geometry().area();
	VICUS::KeywordList::setParameter(r.m_para, "Room::para_t", VICUS::Room::P_Area, area);
	VICUS::KeywordList::setParameter(r.m_para, "Room::para_t", VICUS::Room::P_Volume, area*offset.magnitude());

	// transfer default colors
	for (VICUS::Surface & s : r.m_surfaces)
		s.m_color = s.m_displayColor;

	// now create the undo action
	unsigned int buildingLevelUid = m_ui->comboBoxBuildingLevel2->currentData().toUInt();
	Q_ASSERT(buildingLevelUid != 0);
	SVUndoAddZone * undo = new SVUndoAddZone(tr("Adding new zone '%1'").arg(r.m_displayName),
											 buildingLevelUid,
											 r, false, &componentInstances);
	undo->push();

	// reset lock axis
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_locks = SVViewState::NUM_L;
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropVertexListWidget::on_comboBoxBuilding3_currentIndexChanged(int /*index*/) {
	updateBuildingLevelsComboBox(m_ui->comboBoxBuildingLevel3, m_ui->comboBoxBuilding3);
}


void SVPropVertexListWidget::on_comboBoxBuildingLevel3_currentIndexChanged(int /*index*/) {
	if (m_ui->comboBoxBuildingLevel3->count() == 0)
		return;
	unsigned int buildingLevelID = m_ui->comboBoxBuildingLevel3->currentData().toUInt();
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(project().objectById(buildingLevelID));
	// also transfer nominal height into zone-height line edit
	if (bl != nullptr) {
		m_ui->lineEditRoofHeight->setValue(bl->m_height);
		// only trigger zone height editing finished, when we are in new vertex mode
		// Mind: widget may be hidden
		SVViewState vs = SVViewStateHandler::instance().viewState();
		if (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex)
			on_lineEditRoofHeight_editingFinishedSuccessfully();
	}
	updateButtonStates();
}


void SVPropVertexListWidget::on_lineEditRoofHeight_editingFinishedSuccessfully() {
	// Guard against call when aborting/focus is lost during undo!
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	if (po->newGeometryMode() != Vic3D::NewGeometryObject::NGM_Roof)
		return;

	updateRoofGeometry();
	//TODO hole geometrie
}


void SVPropVertexListWidget::on_comboBoxRoofType_currentIndexChanged(int /*index*/) {
	updateRoofGeometry();
}


void SVPropVertexListWidget::on_radioButtonRoofHeight_toggled(bool checked) {
	m_ui->lineEditRoofHeight->setEnabled(checked);
	m_ui->lineEditRoofInclination->setEnabled(!checked);
	updateRoofGeometry();
}


void SVPropVertexListWidget::on_checkBoxFlapTile_toggled(bool checked) {
	m_ui->lineEditFlapTileHeight->setEnabled(checked);
	updateRoofGeometry();
}


void SVPropVertexListWidget::on_pushButtonCreateRoof_clicked() {
	// we need a building level
	if (m_ui->comboBoxBuildingLevel3->currentIndex() == -1) {
		QMessageBox::critical(this, QString(), tr("First select a building level to add the roof zone to!"));
		return;
	}

	// get floor polygon from geometry object
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	const VICUS::Polygon3D & floor = po->planeGeometry().polygon3D();

	// generate floor surface (no component assigned!)
	unsigned int nextID = project().nextUnusedID();
	VICUS::Room r;
	r.m_id = nextID;
	r.m_displayName = m_ui->lineEditNameRoof->text().trimmed();
	// now we can create the surfaces for top and bottom
	// compose a surface object based on the current content of the new polygon object
	VICUS::Surface sFloor;
	sFloor.m_displayName = tr("Floor");
	sFloor.m_id = ++nextID;
	sFloor.setPolygon3D(floor);
	sFloor.m_displayColor = QColor(150,50,20,255);	//for floor

	r.m_surfaces.push_back(sFloor);

	std::vector<VICUS::ComponentInstance> componentInstances;
	unsigned int compInstID = VICUS::largestUniqueId(project().m_componentInstances);
	// now process all other generated surfaces and create roof surfaces
	unsigned int roofSurfaceCount = 0;
	unsigned int wallCount = 0;
	for (unsigned int i=0; i<po->generatedGeometry().size(); ++i) {
		VICUS::Surface sRoof;
		sRoof.m_id = ++nextID;
		sRoof.m_displayName = tr("Wall surface");//.arg(++roofSurfaceCount);
		sRoof.setPolygon3D(po->generatedGeometry()[i].polygon3D());
		sRoof.m_displayColor = QColor(200,200,140,255);	//for walls

		unsigned int componentID = VICUS::INVALID_ID;
		if (m_ui->comboBoxComponentRoof3->count() > 0)
			componentID = m_ui->comboBoxComponentRoof3->currentData().toUInt();
		// special handling for flap tile
		//if (m_ui->checkBoxFlapTile->isChecked()) {
		// TODO Dirk: for flap-tile surfaces adjust sRoof.m_displayName and componentID
		switch ((Vic3D::NewGeometryObject::RoofInputData::RoofType)m_ui->comboBoxRoofType->currentIndex()) {
			case Vic3D::NewGeometryObject::RoofInputData::SinglePitchRoof:{
				unsigned int roofElements = 1;
				if (i< roofElements) {
					sRoof.m_displayName =  tr("Roof surface %1").arg(++roofSurfaceCount);
					if (m_ui->comboBoxComponentRoof3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentRoof3->currentData().toUInt();
					sRoof.m_displayColor = QColor("#566094");	//for roofs
				}
				else {
					sRoof.m_displayName =  tr("Wall surface %1").arg(++wallCount);
					if (m_ui->comboBoxComponentWall3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentWall3->currentData().toUInt();
				}
			}
			break;
			case Vic3D::NewGeometryObject::RoofInputData::DoublePitchRoof:{
				unsigned int roofElements = 2;
				if (i< roofElements) {
					sRoof.m_displayName =  tr("Roof surface %1").arg(++roofSurfaceCount);
					if (m_ui->comboBoxComponentRoof3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentRoof3->currentData().toUInt();
					sRoof.m_displayColor = QColor("#566094");	//for roofs
				}
				else{
					sRoof.m_displayName =  tr("Wall surface %1").arg(++wallCount);
					if (m_ui->comboBoxComponentWall3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentWall3->currentData().toUInt();
				}
			}
			break;
			case Vic3D::NewGeometryObject::RoofInputData::MansardRoof:{
				unsigned int roofElements = 4;
				if (i< roofElements) {
					sRoof.m_displayName =  tr("Roof surface %1").arg(++roofSurfaceCount);
					if (m_ui->comboBoxComponentRoof3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentRoof3->currentData().toUInt();
					sRoof.m_displayColor = QColor("#566094");	//for roofs
				}
				else {
					sRoof.m_displayName =  tr("Wall surface %1").arg(++wallCount);
					if (m_ui->comboBoxComponentWall3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentWall3->currentData().toUInt();
				}
			}
			break;
			case Vic3D::NewGeometryObject::RoofInputData::HipRoof:{
				unsigned int roofElements = 4;
				if (i< roofElements) {
					sRoof.m_displayName =  tr("Roof surface %1").arg(++roofSurfaceCount);
					if (m_ui->comboBoxComponentRoof3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentRoof3->currentData().toUInt();
					sRoof.m_displayColor = QColor("#566094");	//for roofs
				}
				else {
					sRoof.m_displayName =  tr("Wall surface %1").arg(++wallCount);
					if (m_ui->comboBoxComponentWall3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentWall3->currentData().toUInt();
				}
			}
			break;
			case Vic3D::NewGeometryObject::RoofInputData::Complex:{
				//TODO: Dirk später noch den Kniestock beachten
				if (true) {
					sRoof.m_displayName =  tr("Roof surface %1").arg(++roofSurfaceCount);
					if (m_ui->comboBoxComponentRoof3->count() == 0)
						componentID = VICUS::INVALID_ID;
					else
						componentID = m_ui->comboBoxComponentRoof3->currentData().toUInt();
					sRoof.m_displayColor = QColor("#566094");	//for roofs
				}
//				else if(i>0){
//					sRoof.m_displayName =  tr("Wall surface %1").arg(++wallCount);
//					if (m_ui->comboBoxComponentWall3->count() == 0)
//						componentID = VICUS::INVALID_ID;
//					else
//						componentID = m_ui->comboBoxComponentWall3->currentData().toUInt();
//				}
			}
			break;
		}
		//}

		if (componentID != VICUS::INVALID_ID) {
			// new surfaces are attached to "Side A"
			componentInstances.push_back(
						VICUS::ComponentInstance(++compInstID, componentID, sRoof.m_id, VICUS::INVALID_ID));
		}

		// add surface to roof zone
		r.m_surfaces.push_back(sRoof);
	}

	//TODO Dirk hier muss noch die Floor Schaltfläche rein sobald Andreas die nachgerüstet hat
	if (m_ui->comboBoxComponentWall3->count() == 0)
		componentInstances.push_back(
				VICUS::ComponentInstance(++compInstID, VICUS::INVALID_ID, sFloor.m_id, VICUS::INVALID_ID));
	else
		componentInstances.push_back(
				VICUS::ComponentInstance(++compInstID, m_ui->comboBoxComponentWall3->currentData().toUInt(), sFloor.m_id, VICUS::INVALID_ID));

	double area = sFloor.geometry().area();
	VICUS::KeywordList::setParameter(r.m_para, "Room::para_t", VICUS::Room::P_Area, area);

	// compute roof volume
	r.calculateVolume();

	// transfer default colors
	for (VICUS::Surface & s : r.m_surfaces)
		s.m_color = s.m_displayColor;

	// now create the undo action
	unsigned int buildingLevelUid = m_ui->comboBoxBuildingLevel3->currentData().toUInt();
	Q_ASSERT(buildingLevelUid != 0);
	SVUndoAddZone * undo = new SVUndoAddZone(tr("Adding roof zone '%1'").arg(r.m_displayName),
											 buildingLevelUid,
											 r, false, &componentInstances);
	undo->push();
}


// *** PRIVATE MEMBERS ***





void SVPropVertexListWidget::updateBuildingComboBox(QComboBox * combo) {
	// populate the combo boxes
	combo->blockSignals(true);
	unsigned int currentUniqueId = combo->currentData().toUInt();
	combo->clear();

	const VICUS::Project & prj = project();
	int rowOfCurrent = -1;
	for (unsigned int i=0; i<prj.m_buildings.size(); ++i) {
		const VICUS::Building & b = prj.m_buildings[i];
		combo->addItem(b.m_displayName, b.m_id);
		combo->setEnabled(true);
		if (b.m_id == currentUniqueId)
			rowOfCurrent = (int)i;
	}

	if (rowOfCurrent != -1) {
		combo->setCurrentIndex(rowOfCurrent);
	}
	else {
		combo->setCurrentIndex(combo->count()-1); // Note: if no buildings, nothing will be selected
	}
	combo->blockSignals(false);
}


void SVPropVertexListWidget::updateBuildingLevelsComboBox(QComboBox * combo, const QComboBox * buildingCombo) {
	combo->blockSignals(true);
	unsigned int currentUniqueId = combo->currentData().toUInt();
	combo->clear();
	// only add items if we have a building selected
	if (buildingCombo->count() != 0) {
		const VICUS::Project & prj = project();
		unsigned int buildingID = buildingCombo->currentData().toUInt();
		const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(prj.objectById(buildingID));
		Q_ASSERT(b != nullptr);
		int rowOfCurrent = -1;
		for (unsigned int i=0; i<b->m_buildingLevels.size(); ++i) {
			const VICUS::BuildingLevel & bl = b->m_buildingLevels[i];
			combo->addItem(bl.m_displayName, bl.m_id);
			combo->setEnabled(true);
			if (bl.m_id == currentUniqueId)
				rowOfCurrent = (int)i;
		}
		if (rowOfCurrent != -1) {
			combo->setCurrentIndex(rowOfCurrent);
		}
		else {
			combo->setCurrentIndex(combo->count()-1); // Note: if none, nothing will be selected
		}

	}
	combo->blockSignals(false);
}


void SVPropVertexListWidget::updateZoneComboBox(QComboBox * combo, const QComboBox * buildingLevelCombo) {
	combo->blockSignals(true);
	unsigned int currentUniqueId = combo->currentData().toUInt();
	combo->clear();
	// only add items if we have a building level selected
	if (buildingLevelCombo->count() != 0) {
		const VICUS::Project & prj = project();
		unsigned int buildingLevelID = buildingLevelCombo->currentData().toUInt();
		const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(prj.objectById(buildingLevelID));
		Q_ASSERT(bl != nullptr);
		int rowOfCurrent = -1;
		for (unsigned int i=0; i<bl->m_rooms.size(); ++i) {
			const VICUS::Room & r = bl->m_rooms[i];
			combo->addItem(r.m_displayName, r.m_id);
			combo->setEnabled(true);
			if (r.m_id == currentUniqueId)
				rowOfCurrent = (int)i;
		}
		if (rowOfCurrent != -1) {
			combo->setCurrentIndex(rowOfCurrent);
		}
		else {
			combo->setCurrentIndex(combo->count()-1); // Note: if none, nothing will be selected
		}
	}
	combo->blockSignals(false);
}

void SVPropVertexListWidget::updateSurfaceComboBox(QComboBox * combo, bool onlySelected) {
	combo->blockSignals(true);

	std::set<const VICUS::Object*> selectedObjs;
	// first we get how many surfaces are selected
	project().selectObjects(selectedObjs, VICUS::Project::SG_All, onlySelected, true);

	// if no surfaces are selected
	bool noSelectedSurfaces = selectedObjs.empty();
	m_ui->checkBoxSelectedSurfaces->setEnabled(!noSelectedSurfaces);
	if(noSelectedSurfaces) {
		m_ui->checkBoxSelectedSurfaces->setChecked(false);
		project().selectObjects(selectedObjs, VICUS::Project::SG_All, false, true);
	}

	unsigned int currentUniqueId = combo->currentData().toUInt();
	combo->clear();
	int rowOfCurrent = -1;
	for(const VICUS::Object *obj : selectedObjs) {
		unsigned int i=0;
		const VICUS::Surface *surf = dynamic_cast<const VICUS::Surface*>(obj);
		if(surf != nullptr) {
			combo->addItem(surf->m_displayName, surf->m_id);
			combo->setEnabled(true);
			if (surf->m_id == currentUniqueId)
				rowOfCurrent = (int)i;
			i++;
		}
	}
	if (rowOfCurrent != -1) {
		combo->setCurrentIndex(rowOfCurrent);
	}
	else {
		combo->setCurrentIndex(combo->count()-1); // Note: if none, nothing will be selecte
	}
	combo->blockSignals(false);
}


bool SVPropVertexListWidget::reselectById(QComboBox * combo, int id) const {
	combo->setEnabled(true);
	if (id != -1) {
		id = combo->findData(id);
		if (id != -1) {
			combo->setCurrentIndex(id);
			return true;
		}
	}
	if (combo->count() != 0)
		combo->setCurrentIndex(0);
	else {
		combo->setEnabled(false);
	}
	return false;
}


bool SVPropVertexListWidget::createAnnonymousGeometry() const {
	return (m_ui->checkBoxAnnonymousGeometry->isVisibleTo(this) && m_ui->checkBoxAnnonymousGeometry->isChecked());
}

bool SVPropVertexListWidget::createSubSurfaceGeometry() const {
	return (m_ui->checkBoxSubSurfaceGeometry->isVisibleTo(this) && m_ui->checkBoxSubSurfaceGeometry->isChecked());
}


void SVPropVertexListWidget::updateButtonStates() {
	// update states of Surface, Zone, Roof pages

	// ** page surface **

	// if checkbox is visible, we adjust the enabled state of other inputs
	bool annonymousGeometry = createAnnonymousGeometry();
	if (annonymousGeometry) {
		m_ui->labelAddBuilding->setEnabled(false);
		m_ui->comboBoxBuilding->setEnabled(false);
		m_ui->toolButtonAddBuilding->setEnabled(false);

		m_ui->labelAddBuildingLevel->setEnabled(false);
		m_ui->comboBoxBuildingLevel->setEnabled(false);
		m_ui->toolButtonAddBuildingLevel->setEnabled(false);

		m_ui->labelAddZone->setEnabled(false);
		m_ui->comboBoxZone->setEnabled(false);
		m_ui->toolButtonAddZone->setEnabled(false);

		m_ui->labelComponent->setEnabled(false);
		m_ui->comboBoxComponent->setEnabled(false);
		m_ui->toolButtonEditComponents1->setEnabled(false);

		m_ui->checkBoxSubSurfaceGeometry->setEnabled(false);
		m_ui->comboBoxSurface->setEnabled(false);

		m_ui->labelSurface->setEnabled(false);
		m_ui->labelSubSurfaceComponent->setEnabled(false);
		m_ui->comboBoxSubSurfComponent->setEnabled(false);
		m_ui->toolButtonEditSubSurfComponents->setEnabled(false);

		m_ui->checkBoxSelectedSurfaces->setEnabled(false);

	}
	else {
		bool subSurf = createSubSurfaceGeometry();

		m_ui->labelComponent->setEnabled(!subSurf);
		m_ui->comboBoxComponent->setEnabled(!subSurf);
		m_ui->toolButtonEditComponents1->setEnabled(!subSurf);

		// building controls
		m_ui->labelAddBuilding->setEnabled(!subSurf);
		m_ui->comboBoxBuilding->setEnabled(!subSurf && m_ui->comboBoxBuilding->count() != 0);
		m_ui->toolButtonAddBuilding->setEnabled(true);

		// building level controls
		m_ui->labelAddBuildingLevel->setEnabled(!subSurf && m_ui->comboBoxBuilding->count() != 0);
		m_ui->comboBoxBuildingLevel->setEnabled(!subSurf && m_ui->comboBoxBuildingLevel->count() != 0);
		m_ui->toolButtonAddBuildingLevel->setEnabled(!subSurf && m_ui->comboBoxBuilding->count() != 0);

		// zone controls
		m_ui->labelAddZone->setEnabled(!subSurf && m_ui->comboBoxBuildingLevel->count() != 0);
		m_ui->comboBoxZone->setEnabled(!subSurf && m_ui->comboBoxZone->count() != 0);
		m_ui->toolButtonAddZone->setEnabled(!subSurf && m_ui->comboBoxBuildingLevel->count() != 0);

		// sub surface controls
		// m_ui->checkBoxSubSurfaceGeometry->setChecked(true);
		// if no surfaces are selected
		std::set<const VICUS::Object*> selectedObjs;
		// first we get how many surfaces are selected
		project().selectObjects(selectedObjs, VICUS::Project::SG_All, true, true);
		bool noSelectedSurfaces = selectedObjs.empty();
		m_ui->checkBoxSelectedSurfaces->setEnabled(subSurf && !noSelectedSurfaces);
		m_ui->checkBoxSelectedSurfaces->setChecked(subSurf && !noSelectedSurfaces);

		m_ui->checkBoxSubSurfaceGeometry->setEnabled(true);

		m_ui->labelSurface->setEnabled(subSurf);
		m_ui->comboBoxSurface->setEnabled(subSurf);
		m_ui->labelSubSurfaceComponent->setEnabled(subSurf);
		m_ui->comboBoxSubSurfComponent->setEnabled(subSurf);
		m_ui->toolButtonEditSubSurfComponents->setEnabled(subSurf);
	}

	// ** page zone **

	// building controls
	m_ui->comboBoxBuilding2->setEnabled(m_ui->comboBoxBuilding2->count() != 0);
	m_ui->toolButtonAddBuilding2->setEnabled(true);

	// building level controls
	m_ui->labelBuildingLevel_2->setEnabled(m_ui->comboBoxBuilding2->count() != 0);
	m_ui->comboBoxBuildingLevel2->setEnabled(m_ui->comboBoxBuildingLevel2->count() != 0);
	m_ui->toolButtonAddBuildingLevel2->setEnabled(m_ui->comboBoxBuilding2->count() != 0);


	// ** page roof **

	// building controls
	m_ui->comboBoxBuilding3->setEnabled(m_ui->comboBoxBuilding3->count() != 0);
	m_ui->toolButtonAddBuilding3->setEnabled(true);

	// building level controls
	m_ui->labelBuildingLevel_3->setEnabled(m_ui->comboBoxBuilding3->count() != 0);
	m_ui->comboBoxBuildingLevel3->setEnabled(m_ui->comboBoxBuildingLevel3->count() != 0);
	m_ui->toolButtonAddBuildingLevel3->setEnabled(m_ui->comboBoxBuilding3->count() != 0);
}


void SVPropVertexListWidget::updateComponentComboBox(QComboBox * combo, int type) {
	// remember currently selected component IDs
	int compID = -1;
	if (combo->currentIndex() != -1)
		compID = combo->currentData().toInt();

	combo->clear();

	std::string langID = QtExt::LanguageHandler::instance().langId().toStdString();
	for (auto & c : SVSettings::instance().m_db.m_components) {
		switch (c.second.m_type) {
			case VICUS::Component::CT_OutsideWall :
			case VICUS::Component::CT_OutsideWallToGround :
			case VICUS::Component::CT_InsideWall :
				if (type == -1 || type == 0)
					combo->addItem( QtExt::MultiLangString2QString(c.second.m_displayName), c.first);
			break;

			case VICUS::Component::CT_FloorToCellar :
			case VICUS::Component::CT_FloorToAir :
			case VICUS::Component::CT_FloorToGround :
				if (type == -1 || type == 1)
					combo->addItem( QtExt::MultiLangString2QString(c.second.m_displayName), c.first);
			break;

			case VICUS::Component::CT_Ceiling :
			case VICUS::Component::CT_SlopedRoof :
			case VICUS::Component::CT_FlatRoof :
			case VICUS::Component::CT_ColdRoof :
			case VICUS::Component::CT_WarmRoof :
				if (type == -1 || type == 2)
					combo->addItem( QtExt::MultiLangString2QString(c.second.m_displayName), c.first);
			break;

			case VICUS::Component::CT_Miscellaneous :
			case VICUS::Component::NUM_CT:
				combo->addItem( QtExt::MultiLangString2QString(c.second.m_displayName), c.first);
			break;
		}
	}

	// reselect previously selected components
	reselectById(combo, compID);
}

void SVPropVertexListWidget::updateSubSurfaceComponentComboBox(QComboBox * combo) {
	// remember currently selected component IDs
	int compID = -1;
	if (combo->currentIndex() != -1)
		compID = combo->currentData().toInt();

	combo->clear();

	std::string langID = QtExt::LanguageHandler::instance().langId().toStdString();
	for (auto & c : SVSettings::instance().m_db.m_subSurfaceComponents) {
		combo->addItem( QtExt::MultiLangString2QString(c.second.m_displayName), c.first);
	}

	// reselect previously selected components
	reselectById(combo, compID);
}

void SVPropVertexListWidget::updateRoofGeometry() {
	// Guard against call when aborting/focus is lost during undo!
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	if (po->newGeometryMode() != Vic3D::NewGeometryObject::NGM_Roof)
		return;

	// use saved polygon for further calculation

	Vic3D::NewGeometryObject::RoofInputData roofData;

	// get all data from UI
	roofData.m_type = (Vic3D::NewGeometryObject::RoofInputData::RoofType)m_ui->comboBoxRoofType->currentIndex();
	roofData.m_angle = m_ui->lineEditRoofInclination->value(); // in Deg
	roofData.m_height = m_ui->lineEditRoofHeight->value(); // in m
	roofData.m_flapTileHeight = m_ui->lineEditFlapTileHeight->value(); // in m
	roofData.m_hasFlapTile = m_ui->checkBoxFlapTile->isChecked();
	roofData.m_isHeightPredefined = m_ui->radioButtonRoofHeight->isChecked();
	roofData.m_rotate = false;

	//m_polygonRotation = !m_polygonRotation; // reset flag until next click

	// Only the first 4 points are used for the roof creation.
	// The exception is the case COMPLEX. In this case all points are used.

	std::vector<IBKMK::Vector3D>	polyline;
	if(roofData.m_type == Vic3D::NewGeometryObject::RoofInputData::Complex)
		polyline = m_roofPolygon;
	else
		for(unsigned int i=0; i<4; ++i)
			polyline.push_back(m_roofPolygon[(i + m_currentIdxOfStartpoint)%4]);

	po->setRoofGeometry(roofData, polyline);

	// we need to trigger a redraw here
	SVViewStateHandler::instance().m_geometryView->refreshSceneView();
}


void SVPropVertexListWidget::on_pushButtonRotateFloorPolygon_clicked() {

	// rotate polygon to get a new roof shape
	/*
		Note that four positions are important for a single pitch roof,
		and only two positions are important for a gable roof.
		Nevertheless, the baseline can consist of more than four points.
		This should also not be destroyed, because in the Complex model
		these are still needed. Therefore, the polyline must be considered
		only for the first 4 points when rotating.
		Therefore, the start index must always be updated as well.
	*/

	m_currentIdxOfStartpoint = (m_currentIdxOfStartpoint + 1)%4;

	//m_polygonRotation = true; // set temporarily to true
	updateRoofGeometry();
}

void SVPropVertexListWidget::on_toolButtonEditSubSurfComponents_clicked() {
	updateButtonStates();
}


void SVPropVertexListWidget::on_checkBoxSubSurfaceGeometry_stateChanged(int arg1) {
	updateButtonStates();
}


void SVPropVertexListWidget::on_checkBoxSelectedSurfaces_stateChanged(int arg1) {
	updateSurfaceComboBox(m_ui->comboBoxSurface, arg1);
}

