#include "SVPropVertexListWidget.h"
#include "ui_SVPropVertexListWidget.h"

#include <QMessageBox>

#include <IBKMK_Vector3D.h>

#include <VICUS_Project.h>
#include <QtExt_LanguageHandler.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVUndoAddSurface.h"
#include "SVGeometryView.h"
#include "SVSettings.h"

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
}


SVPropVertexListWidget::~SVPropVertexListWidget() {
	delete m_ui;
}


void SVPropVertexListWidget::setup(int newGeometryType) {
	m_ui->groupBoxPolygonVertexes->setVisible(false);
	m_ui->groupBoxZoneProperties->setVisible(false);
	m_ui->groupBoxSurfaceProperties->setVisible(false);
	QString baseName(tr("New surface"));
	switch (newGeometryType) {
		case Vic3D::NewGeometryObject::NGM_Rect :
			m_ui->groupBoxSurfaceProperties->setVisible(true);
		break;
		case Vic3D::NewGeometryObject::NGM_Polygon :
			m_ui->groupBoxPolygonVertexes->setVisible(true);
			m_ui->groupBoxSurfaceProperties->setVisible(true);
		break;
		case Vic3D::NewGeometryObject::NGM_ZoneFloor :
			m_ui->groupBoxPolygonVertexes->setVisible(true);
			m_ui->groupBoxZoneProperties->setVisible(true);
			baseName = tr("Zone");
		break;
		case Vic3D::NewGeometryObject::NGM_ZoneExtrusion :
			m_ui->groupBoxZoneProperties->setVisible(true);
		break;
	}
	// populate the combo boxes
	m_ui->comboBoxZone->blockSignals(true);
	m_ui->comboBoxBuilding->blockSignals(true);
	m_ui->comboBoxBuildingLevel->blockSignals(true);
	m_ui->comboBoxComponent->blockSignals(true);

	m_ui->comboBoxZone->clear();
	m_ui->comboBoxBuilding->clear();
	m_ui->comboBoxBuildingLevel->clear();
	m_ui->comboBoxComponent->clear();


	m_ui->comboBoxZone->blockSignals(false);
	m_ui->comboBoxBuilding->blockSignals(false);
	m_ui->comboBoxBuildingLevel->blockSignals(false);
	m_ui->comboBoxComponent->blockSignals(false);

	// populate component combo boxes
	updateComponentComboBoxes();

	// generate new unique object/surface name
	unsigned int count = 1;
	// compose object names until we found a unique object name
	std::set<QString> existingNames;
	for (const VICUS::Surface & s : project().m_plainGeometry) {
		existingNames.insert(s.m_displayName);
	}
	QString name = baseName;
	for (;;) {
		// process all surfaces and check if we have already a new surface with our current name
		if (existingNames.find(name) == existingNames.end())
			break;
		name = QString("%1 [%2]").arg(baseName).arg(++count);
	}
	m_ui->lineEditName->setText(name);
	clearPolygonVertexList();
}


void SVPropVertexListWidget::reselectById(QComboBox * combo, int id) const {
	if (id != -1) {
		id = combo->findData(id);
		if (id != -1)
			combo->setCurrentIndex(id);
		else {
			if (combo->count() != 0)
				combo->setCurrentIndex(0);
			else {
				combo->setEnabled(false);
				combo->setCurrentText(tr("<no component>"));
			}
		}

	}

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

	// continue in place-vertex mode (setting the viewstate also triggers a repaint)
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	SVViewStateHandler::instance().setViewState(vs);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropVertexListWidget::clearPolygonVertexList() {
	// clear table widget and disable "delete" and "finish" buttons
	m_ui->tableWidgetVertexes->setRowCount(0);
	m_ui->pushButtonFinish->setEnabled(false);
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
	po->removeVertex((unsigned int)currentRow);
}


void SVPropVertexListWidget::on_pushButtonFinish_clicked() {
	if (m_ui->lineEditName->text().trimmed().isEmpty()) {
		QMessageBox::critical(this, QString(), tr("Please enter a descriptive name!"));
		m_ui->lineEditName->selectAll();
		m_ui->lineEditName->setFocus();
		return;
	}
	// compose a surface object based on the current content of the new polygon object
	VICUS::Surface s;
	s.m_displayName = m_ui->lineEditName->text().trimmed();
	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	s.m_geometry = po->planeGeometry();
	s.m_id = s.uniqueID();
	s.m_color = QColor("silver");

	// reset view
	on_pushButtonCancel_clicked();

	// modify project
	SVUndoAddSurface * undo = new SVUndoAddSurface(tr("Added surface '%1'").arg(s.m_displayName), s, 0);
	undo->push();
}

