#include "SVPropVertexListWidget.h"
#include "ui_SVPropVertexListWidget.h"

#include <QMessageBox>
#include <QInputDialog>

#include <IBKMK_Vector3D.h>

#include <VICUS_Project.h>
#include <QtExt_LanguageHandler.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVUndoAddSurface.h"
#include "SVGeometryView.h"
#include "SVSettings.h"
#include "SVMainWindow.h"
#include "SVUndoAddBuilding.h"
#include "SVUndoAddBuildingLevel.h"
#include "SVUndoAddZone.h"

#include "Vic3DNewGeometryObject.h"

SVPropVertexListWidget::SVPropVertexListWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropVertexListWidget)
{
	m_ui->setupUi(this);

	QSizePolicy sp_retain = m_ui->groupBoxPolygonVertexes->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(true);
	m_ui->groupBoxPolygonVertexes->setSizePolicy(sp_retain);

	SVViewStateHandler::instance().m_propVertexListWidget = this;

	connect(m_ui->toolButtonEditComponents1, &QToolButton::clicked,
			this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents2, &QToolButton::clicked,
			this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents3, &QToolButton::clicked,
			this, &SVPropVertexListWidget::onEditComponents);
	connect(m_ui->toolButtonEditComponents4, &QToolButton::clicked,
			this, &SVPropVertexListWidget::onEditComponents);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropVertexListWidget::onModified);
}


SVPropVertexListWidget::~SVPropVertexListWidget() {
	delete m_ui;
}


void SVPropVertexListWidget::setup(int newGeometryType) {
	m_ui->groupBoxPolygonVertexes->setVisible(false);
	m_ui->groupBoxZoneProperties->setVisible(false);
	m_ui->groupBoxSurfaceProperties->setVisible(false);
	m_ui->checkBoxAnnonymousGeometry->setVisible(false);
	QString baseName(tr("New surface"));
	switch (newGeometryType) {
		case Vic3D::NewGeometryObject::NGM_Rect :
			m_ui->groupBoxSurfaceProperties->setVisible(true);
			m_ui->checkBoxAnnonymousGeometry->setVisible(true);
			m_ui->pushButtonFinish->setText(tr("Create surface"));
		break;
		case Vic3D::NewGeometryObject::NGM_Polygon :
			m_ui->groupBoxPolygonVertexes->setVisible(true);
			m_ui->groupBoxSurfaceProperties->setVisible(true);
			m_ui->checkBoxAnnonymousGeometry->setVisible(true);
			m_ui->pushButtonFinish->setText(tr("Create surface"));
		break;
		case Vic3D::NewGeometryObject::NGM_ZoneFloor :
			m_ui->groupBoxPolygonVertexes->setVisible(true);
			m_ui->groupBoxZoneProperties->setVisible(true);
			baseName = tr("New zone");
			m_ui->pushButtonFinish->setText(tr("Create zone"));
		break;

		// we do not have other geometries, yet
		default:;
	}

	// populate component combo boxes
	updateBuildingComboBox(); // this will also update the other combo boxes
	updateComponentComboBoxes();

	// compose object names until we found a unique object name
	std::set<QString> existingNames;
	for (const VICUS::Surface & s : project().m_plainGeometry)
		existingNames.insert(s.m_displayName);

	// set new unique object/surface name
	m_ui->lineEditName->setText( VICUS::Project::uniqueName(baseName, existingNames));
	clearPolygonVertexList();
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


void SVPropVertexListWidget::updateBuildingComboBox() {
	// populate the combo boxes
	m_ui->comboBoxBuilding->blockSignals(true);
	unsigned int currentUniqueId = m_ui->comboBoxBuilding->currentData().toUInt();
	m_ui->comboBoxBuilding->clear();

	const VICUS::Project & prj = project();
	int rowOfCurrent = -1;
	for (unsigned int i=0; i<prj.m_buildings.size(); ++i) {
		const VICUS::Building & b = prj.m_buildings[i];
		m_ui->comboBoxBuilding->addItem(b.m_displayName, b.uniqueID());
		if (b.uniqueID() == currentUniqueId)
			rowOfCurrent = (int)i;
	}

	if (rowOfCurrent != -1) {
		m_ui->comboBoxBuilding->setCurrentIndex(rowOfCurrent);
	}
	else {
		m_ui->comboBoxBuilding->setCurrentIndex(m_ui->comboBoxBuilding->count()-1); // Note: if no buildings, nothing will be selected
	}
	m_ui->comboBoxBuilding->blockSignals(false);

	// also update the building levels combo box
	updateBuildingLevelsComboBox();
}


void SVPropVertexListWidget::updateBuildingLevelsComboBox() {
	m_ui->comboBoxBuildingLevel->blockSignals(true);
	unsigned int currentUniqueId = m_ui->comboBoxBuildingLevel->currentData().toUInt();
	m_ui->comboBoxBuildingLevel->clear();
	// only add items if we have a building selected
	if (m_ui->comboBoxBuilding->count() != 0) {
		const VICUS::Project & prj = project();
		unsigned int buildingUniqueID = m_ui->comboBoxBuilding->currentData().toUInt();
		const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(prj.objectById(buildingUniqueID));
		Q_ASSERT(b != nullptr);
		int rowOfCurrent = -1;
		for (unsigned int i=0; i<b->m_buildingLevels.size(); ++i) {
			const VICUS::BuildingLevel & bl = b->m_buildingLevels[i];
			m_ui->comboBoxBuildingLevel->addItem(bl.m_displayName, bl.uniqueID());
			if (bl.uniqueID() == currentUniqueId)
				rowOfCurrent = (int)i;
		}
		if (rowOfCurrent != -1) {
			m_ui->comboBoxBuildingLevel->setCurrentIndex(rowOfCurrent);
		}
		else {
			m_ui->comboBoxBuildingLevel->setCurrentIndex(m_ui->comboBoxBuildingLevel->count()-1); // Note: if none, nothing will be selected
		}

	}
	m_ui->comboBoxBuildingLevel->blockSignals(false);

	// also update the zones combo box
	updateZoneComboBox();
}


void SVPropVertexListWidget::updateZoneComboBox() {
	m_ui->comboBoxZone->blockSignals(true);
	unsigned int currentUniqueId = m_ui->comboBoxZone->currentData().toUInt();
	m_ui->comboBoxZone->clear();
	// only add items if we have a building level selected
	if (m_ui->comboBoxBuildingLevel->count() != 0) {
		const VICUS::Project & prj = project();
		unsigned int buildingLevelUniqueID = m_ui->comboBoxBuildingLevel->currentData().toUInt();
		const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(prj.objectById(buildingLevelUniqueID));
		Q_ASSERT(bl != nullptr);
		int rowOfCurrent = -1;
		for (unsigned int i=0; i<bl->m_rooms.size(); ++i) {
			const VICUS::Room & r = bl->m_rooms[i];
			m_ui->comboBoxZone->addItem(r.m_displayName, r.uniqueID());
			if (r.uniqueID() == currentUniqueId)
				rowOfCurrent = (int)i;
		}
		if (rowOfCurrent != -1) {
			m_ui->comboBoxZone->setCurrentIndex(rowOfCurrent);
		}
		else {
			m_ui->comboBoxZone->setCurrentIndex(m_ui->comboBoxZone->count()-1); // Note: if none, nothing will be selected
		}
	}
	m_ui->comboBoxZone->blockSignals(false);
	updateEnabledStates();
}


void SVPropVertexListWidget::updateComponentComboBoxes() {
	// remember currently selected component IDs
	int floorCompID = -1;
	int ceilingCompID = -1;
	int wallCompID = -1;
	int surfaceCompID = -1;
	if (m_ui->comboBoxComponentFloor->currentIndex() != -1)
		floorCompID = m_ui->comboBoxComponentFloor->currentData().toInt();
	if (m_ui->comboBoxComponentCeiling->currentIndex() != -1)
		ceilingCompID = m_ui->comboBoxComponentCeiling->currentData().toInt();
	if (m_ui->comboBoxComponentWalls->currentIndex() != -1)
		wallCompID = m_ui->comboBoxComponentWalls->currentData().toInt();
	if (m_ui->comboBoxComponent->currentIndex() != -1)
		surfaceCompID = m_ui->comboBoxComponent->currentData().toInt();

	m_ui->comboBoxComponentFloor->clear();
	m_ui->comboBoxComponentCeiling->clear();
	m_ui->comboBoxComponentWalls->clear();
	m_ui->comboBoxComponent->clear();

	std::string langID = QtExt::LanguageHandler::instance().langId().toStdString();
	for (auto & c : SVSettings::instance().m_db.m_components) {
		switch (c.second.m_type) {
			case VICUS::Component::CT_OutsideWall :
			case VICUS::Component::CT_OutsideWallToGround :
			case VICUS::Component::CT_InsideWall :
				m_ui->comboBoxComponentWalls->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
			break;

			case VICUS::Component::CT_FloorToCellar :
			case VICUS::Component::CT_FloorToAir :
			case VICUS::Component::CT_FloorToGround :
				m_ui->comboBoxComponentFloor->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
			break;

			case VICUS::Component::CT_Ceiling :
			case VICUS::Component::CT_SlopedRoof :
			case VICUS::Component::CT_FlatRoof :
			case VICUS::Component::CT_ColdRoof :
			case VICUS::Component::CT_WarmRoof :
				m_ui->comboBoxComponentCeiling->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
			break;

			case VICUS::Component::CT_Miscellaneous :
			case VICUS::Component::NUM_CT:
				m_ui->comboBoxComponentFloor->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
				m_ui->comboBoxComponentCeiling->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
				m_ui->comboBoxComponentWalls->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
			break;
		}

		m_ui->comboBoxComponent->addItem( QString::fromStdString(c.second.m_displayName.string(langID, "en")), c.first);
	}

	// reselect previously selected components
	reselectById(m_ui->comboBoxComponentFloor, floorCompID);
	reselectById(m_ui->comboBoxComponentCeiling, ceilingCompID);
	reselectById(m_ui->comboBoxComponentWalls, wallCompID);
	reselectById(m_ui->comboBoxComponent, surfaceCompID);
}


void SVPropVertexListWidget::addVertex(const IBKMK::Vector3D & p) {
	// Note: the vertex is already in the NewGeometryObject, we only
	//       modify the table widget and update the button enabled states
	int row = m_ui->tableWidgetVertexes->rowCount();
	m_ui->tableWidgetVertexes->setRowCount(row + 1);
	QTableWidgetItem * item = new QTableWidgetItem(QString("%1").arg(row+1));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	m_ui->tableWidgetVertexes->setItem(row,0,item);
	item = new QTableWidgetItem(QString("%L1,%L2,%L3").arg(p.m_x).arg(p.m_y).arg(p.m_z));
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	m_ui->tableWidgetVertexes->setItem(row,1,item);

	m_ui->pushButtonDeleteLast->setEnabled(true);

	m_ui->pushButtonFinish->setEnabled(SVViewStateHandler::instance().m_newGeometryObject->canComplete());
	m_ui->pushButtonFloorDone->setEnabled(SVViewStateHandler::instance().m_newGeometryObject->planeGeometry().isValid());
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
	m_ui->pushButtonFinish->setEnabled(SVViewStateHandler::instance().m_newGeometryObject->canComplete());
	m_ui->pushButtonFloorDone->setEnabled(SVViewStateHandler::instance().m_newGeometryObject->planeGeometry().isValid());

	// continue in place-vertex mode (setting the viewstate also triggers a repaint)
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	SVViewStateHandler::instance().setViewState(vs);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropVertexListWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	// only do something here, if this widget is actually visible
	if (!isVisibleTo(qobject_cast<QWidget*>(parent())) )
		return;
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		/*! We only need to handle changes of the building topology, in all other cases
			the "create new geometry" action is aborted and the widget will be hidden. */
		case SVProjectHandler::BuildingTopologyChanged:
			updateBuildingComboBox(); // this will also update the other combo boxes
		break;
		default:;
	}
}


void SVPropVertexListWidget::clearPolygonVertexList() {
	// clear table widget and disable "delete" and "finish" buttons
	m_ui->tableWidgetVertexes->setRowCount(0);
	m_ui->pushButtonFinish->setEnabled(false);
	m_ui->pushButtonFloorDone->setEnabled(false);
	m_ui->pushButtonDeleteLast->setEnabled(false);
	m_ui->pushButtonDeleteSelected->setEnabled(false);
}


void SVPropVertexListWidget::on_pushButtonDeleteLast_clicked() {
	// remove last vertex from polygon
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	po->removeLastVertex();
}


void SVPropVertexListWidget::on_tableWidgetVertexes_itemSelectionChanged() {
	m_ui->pushButtonDeleteSelected->setEnabled( m_ui->tableWidgetVertexes->currentRow() != -1);
}


void SVPropVertexListWidget::on_pushButtonCancel_clicked() {
	// reset new polygon object, so that it won't be drawn anylonger
	SVViewStateHandler::instance().m_newGeometryObject->clear();
	// signal, that we are no longer in "add vertex" mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropVertexListWidget::on_pushButtonDeleteSelected_clicked() {
	int currentRow = m_ui->tableWidgetVertexes->currentRow();
	Q_ASSERT(currentRow != -1);
	// remove selected vertex from polygon
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	po->removeVertex((unsigned int)currentRow); // this will in turn call removeVertex() above
}


void SVPropVertexListWidget::on_pushButtonFinish_clicked() {
	if (m_ui->lineEditName->text().trimmed().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Please enter a descriptive name!"));
		m_ui->lineEditName->selectAll();
		m_ui->lineEditName->setFocus();
		return;
	}
	// depending on the type of geometry that's being created,
	// perform additional checks
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	switch (po->newGeometryMode()) {
		case Vic3D::NewGeometryObject::NGM_Rect:
		case Vic3D::NewGeometryObject::NGM_Polygon: {
			// compose a surface object based on the current content of the new polygon object
			VICUS::Surface s;
			s.m_displayName = m_ui->lineEditName->text().trimmed();
			s.m_geometry = po->planeGeometry();
			s.m_id = s.uniqueID();

			// we need all properties, unless we create annonymous geometry
			if (m_ui->checkBoxAnnonymousGeometry->isChecked()) {
				s.m_color = QColor("silver");
				// modify project
				SVUndoAddSurface * undo = new SVUndoAddSurface(tr("Added surface '%1'").arg(s.m_displayName), s, 0);
				undo->push();
			}
			else {
				// we need inputs for room (if there is a room, there is also a building and level)
				if (m_ui->comboBoxZone->currentIndex() != -1) {
					unsigned int zoneUUID = m_ui->comboBoxZone->currentData().toUInt();
					s.updateColor(); // set color based on orientation
					// also store component information
					s.m_componentId = m_ui->comboBoxComponent->currentData().toUInt();
					// modify project
					SVUndoAddSurface * undo = new SVUndoAddSurface(tr("Added surface '%1'").arg(s.m_displayName), s, zoneUUID);
					undo->push();
				}
				else {
					QMessageBox::critical(this, QString(), tr("Select a zone first to add the surface to!"));
					return;
				}
			}
		} break;

		case Vic3D::NewGeometryObject::NGM_ZoneExtrusion :

		break;
	}

	// reset view
	on_pushButtonCancel_clicked();
}


void SVPropVertexListWidget::onEditComponents() {
	// ask main window to show database dialog, afterwards update component combos
	SVMainWindow::instance().on_actionDBComponents_triggered();
	// Note: SVMainWindow::instance().on_actionDBComponents_triggered() calls updateComponentCombos() itself, so
	//       no need to call this here
}


void SVPropVertexListWidget::on_toolButtonAddBuilding_clicked() {
	std::set<QString> existingNames;
	for (const VICUS::Building & b : project().m_buildings)
		existingNames.insert(b.m_displayName);
	QString defaultName = VICUS::Project::uniqueName(tr("Building"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building"), tr("New building name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;
	// modify project
	VICUS::Building b;
	b.m_id = VICUS::Project::uniqueId(project().m_buildings);
	b.m_displayName = text;
	SVUndoAddBuilding * undo = new SVUndoAddBuilding(tr("Adding building '%1'").arg(b.m_displayName), b, true);
	undo->push(); // this will update our combo boxes

	// now also select the matching item
	reselectById(m_ui->comboBoxBuilding, (int)b.uniqueID());
}


void SVPropVertexListWidget::on_toolButtonAddBuildingLevel_clicked() {
	// get currently selected building
	unsigned int buildingUniqueID = m_ui->comboBoxBuilding->currentData().toUInt();
	const VICUS::Building * b = dynamic_cast<const VICUS::Building*>(project().objectById(buildingUniqueID));
	Q_ASSERT(b != nullptr);

	std::set<QString> existingNames;
	for (const VICUS::BuildingLevel & bl : b->m_buildingLevels)
		existingNames.insert(bl.m_displayName);
	QString defaultName = VICUS::Project::uniqueName(tr("Level"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building level"), tr("New building level/floor name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;

	// modify project
	VICUS::BuildingLevel bl;
	bl.m_id = VICUS::Project::uniqueId(b->m_buildingLevels);
	bl.m_displayName = text;
	SVUndoAddBuildingLevel * undo = new SVUndoAddBuildingLevel(tr("Adding building level '%1'").arg(bl.m_displayName), buildingUniqueID, bl, true);
	undo->push(); // this will update our combo boxes

	// now also select the matching item
	reselectById(m_ui->comboBoxBuildingLevel, (int)bl.uniqueID());
}


void SVPropVertexListWidget::on_toolButtonAddZone_clicked() {
	// get currently selected building
	unsigned int buildingLevelUniqueID = m_ui->comboBoxBuildingLevel->currentData().toUInt();
	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel*>(project().objectById(buildingLevelUniqueID));
	Q_ASSERT(bl != nullptr);

	std::set<QString> existingNames;
	for (const VICUS::Room & r : bl->m_rooms)
		existingNames.insert(r.m_displayName);
	QString defaultName = VICUS::Project::uniqueName(tr("Room"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add room/zone"), tr("New room/zone name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;

	// modify project
	VICUS::Room r;
	r.m_id = VICUS::Project::uniqueId(bl->m_rooms);
	r.m_displayName = text;
	SVUndoAddZone * undo = new SVUndoAddZone(tr("Adding building zone '%1'").arg(r.m_displayName), buildingLevelUniqueID, r, true);
	undo->push(); // this will update our combo boxes

	// now also select the matching item
	reselectById(m_ui->comboBoxZone, (int)r.uniqueID());
}


void SVPropVertexListWidget::on_checkBoxAnnonymousGeometry_stateChanged(int /*arg1*/) {
	updateEnabledStates();
}


bool SVPropVertexListWidget::createAnnonymousGeometry() const {
	return (m_ui->checkBoxAnnonymousGeometry->isVisibleTo(this) && m_ui->checkBoxAnnonymousGeometry->isChecked());
}

void SVPropVertexListWidget::updateEnabledStates() {
	// if checkbox is visible, we adjust the enabled state of other inputs
	bool annonymousGeometry = createAnnonymousGeometry();
	if (annonymousGeometry) {
		m_ui->groupBoxSurfaceProperties->setEnabled(false);

		m_ui->labelBuilding->setEnabled(false);
		m_ui->comboBoxBuilding->setEnabled(false);
		m_ui->toolButtonAddBuilding->setEnabled(false);

		m_ui->labelBuildingLevel->setEnabled(false);
		m_ui->comboBoxBuildingLevel->setEnabled(false);
		m_ui->toolButtonAddBuildingLevel->setEnabled(false);

		m_ui->labelZone->setEnabled(false);
		m_ui->comboBoxZone->setEnabled(false);
		m_ui->toolButtonAddZone->setEnabled(false);
	}
	else {
		m_ui->groupBoxSurfaceProperties->setEnabled(true);

		// building controls
		if (m_ui->comboBoxBuilding->count() == 0) {
			m_ui->comboBoxBuilding->setEnabled(false);
		}
		else {
			m_ui->comboBoxBuilding->setEnabled(true);
		}
		m_ui->labelBuilding->setEnabled(true);
		m_ui->toolButtonAddBuilding->setEnabled(true);


		// building level controls
		if (m_ui->comboBoxBuildingLevel->count() == 0) {
			m_ui->comboBoxBuildingLevel->setEnabled(false);
		}
		else {
			m_ui->comboBoxBuildingLevel->setEnabled(true);
		}
		// enable tool button to add new levels
		m_ui->toolButtonAddBuildingLevel->setEnabled(m_ui->comboBoxBuilding->count() != 0);
		m_ui->labelBuildingLevel->setEnabled(m_ui->comboBoxBuilding->count() != 0);


		// room controls
		// never enabled when we create zones
		if (m_ui->groupBoxZoneProperties->isVisibleTo(this)) {
			m_ui->labelZone->setEnabled(false);
			m_ui->comboBoxZone->setEnabled(false);
			m_ui->toolButtonAddZone->setEnabled(false);
		}
		else {
			if (m_ui->comboBoxZone->count() == 0) {
				m_ui->comboBoxZone->setEnabled(false);
			}
			else {
				m_ui->comboBoxZone->setEnabled(true);
			}
			// enable tool button to add new zones
			m_ui->toolButtonAddZone->setEnabled(m_ui->comboBoxBuildingLevel->count() != 0);
			m_ui->labelZone->setEnabled(m_ui->comboBoxBuildingLevel->count() != 0);
		}
	}
}


void SVPropVertexListWidget::on_comboBoxBuilding_currentIndexChanged(int /*index*/) {
	updateBuildingLevelsComboBox();
}


void SVPropVertexListWidget::on_comboBoxBuildingLevel_currentIndexChanged(int index) {
	updateZoneComboBox();
}


void SVPropVertexListWidget::on_pushButtonFloorDone_clicked() {
	// we switch to floor-extrusion mode now
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	po->switchTo(Vic3D::NewGeometryObject::NGM_ZoneExtrusion);
	m_ui->pushButtonFloorDone->setEnabled(false);
	m_ui->pushButtonFinish->setEnabled(true);
	m_ui->groupBoxPolygonVertexes->setEnabled(false);
}
