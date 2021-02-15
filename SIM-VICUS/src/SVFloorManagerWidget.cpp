#include "SVFloorManagerWidget.h"
#include "ui_SVFloorManagerWidget.h"

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVUndoModifyBuilding.h"

SVFloorManagerWidget::SVFloorManagerWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVFloorManagerWidget)
{
	m_ui->setupUi(this);
	QStringList header;
	header << tr("Building/Floor") << tr("Elevation [m]") << tr("Height [m]");
	m_ui->treeWidget->setHeaderLabels(header);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVFloorManagerWidget::onModified);

	onModified(SVProjectHandler::AllModified, nullptr); // update user interface to current project's state
}


SVFloorManagerWidget::~SVFloorManagerWidget() {
	delete m_ui;
}



void SVFloorManagerWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	switch ((SVProjectHandler::ModificationTypes)modificationType) {
		case SVProjectHandler::SolverParametersModified:
		case SVProjectHandler::ClimateLocationModified:
		case SVProjectHandler::GridModified:
		case SVProjectHandler::NetworkModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::ComponentInstancesModified:
			return; // unrelated changes, do nothing

		case SVProjectHandler::NodeStateModified: // need to listen to selection changes
		case SVProjectHandler::BuildingTopologyChanged:
		case SVProjectHandler::AllModified:
		break;
	}

	// update user interface but keep currently focused widget and current selection in QTreeWidget

	QList<QTreeWidgetItem*> sel = m_ui->treeWidget->selectedItems();
	// get ID of currently selected object
	unsigned int selObjectUniqueId = 0;
	if (!sel.isEmpty()) {
		selObjectUniqueId = sel.front()->data(0,Qt::UserRole).toUInt();
	}

	// now populate the tree widget
	m_ui->treeWidget->clear();
	QTreeWidgetItem * selectedItem = nullptr;
	for (const VICUS::Building & b : project().m_buildings) {
		QTreeWidgetItem * item = new QTreeWidgetItem(QStringList() << QString("%1 [%2]").arg(b.m_displayName).arg(b.m_id));
		item->setData(0, Qt::UserRole, b.uniqueID());
		m_ui->treeWidget->addTopLevelItem(item);
		if (selObjectUniqueId == b.uniqueID())
			selectedItem = item;
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			QStringList columns;
			columns << QString("%1 [%2]").arg(bl.m_displayName).arg(bl.m_id);
			columns << QString("%L1").arg(bl.m_elevation, 0, 'g', 2);
			columns << QString("%L1").arg(bl.m_height, 0, 'g', 2);
			QTreeWidgetItem * levelItem = new QTreeWidgetItem(columns);
			levelItem->setData(0, Qt::UserRole, bl.uniqueID());
			item->addChild(levelItem);
			if (selObjectUniqueId == bl.uniqueID())
				selectedItem = levelItem;
		}
	}
	// reselect the previously selected item
	if (selectedItem != nullptr) {
		m_ui->treeWidget->blockSignals(true);
		selectedItem->setSelected(true);
		m_ui->treeWidget->blockSignals(false);
	}
	m_ui->treeWidget->expandAll();

	on_treeWidget_itemSelectionChanged();
}


void SVFloorManagerWidget::on_treeWidget_itemSelectionChanged() {
	// show/hide buttons depending on selection
	QList<QTreeWidgetItem*> sel = m_ui->treeWidget->selectedItems();

	// hide all buttons first
	m_ui->pushButtonAddLevel->setVisible(false);
	m_ui->pushButtonRemoveLevel->setVisible(false);
	m_ui->pushButtonRemoveBuilding->setVisible(false);

	// hide property widgets
	m_ui->groupBoxBuildingProperties->setVisible(false);
	m_ui->groupBoxLevelProperties->setVisible(false);

	if (sel.isEmpty())
		return;

	// get unique ID of selected object
	unsigned int selObjectUniqueId = sel.front()->data(0,Qt::UserRole).toUInt();

	m_currentBuilding = nullptr;
	m_currentBuildingLevel = nullptr;

	// and lookup object in project
	const VICUS::Object * obj = project().objectById(selObjectUniqueId);
	const VICUS::Building * b = dynamic_cast<const VICUS::Building *>(obj);
	if (b != nullptr) {
		m_currentBuilding = b;
		m_ui->groupBoxBuildingProperties->setVisible(true);
		// update group box with properties
		m_ui->lineEditBuildingName->setText(b->m_displayName);

		m_ui->pushButtonAddLevel->setVisible(true);
		m_ui->pushButtonRemoveBuilding->setVisible(true);

		std::set<const VICUS::Object *> selObjs;
		project().selectObjects(selObjs, VICUS::Project::SG_Building, true, true);
		// do we have any levels selected?
		int selectedLevels = 0;
		for (const VICUS::Object * o : selObjs)
			if (dynamic_cast<const VICUS::BuildingLevel*>(o) != nullptr) {
				++selectedLevels;
				if (++selectedLevels > 1)
					break;
			}
		m_ui->pushButtonAssignLevel->setEnabled(selectedLevels != 0);
		if (selectedLevels > 1)
			m_ui->pushButtonAssignLevel->setText(tr("Assign selected levels"));
		else
			m_ui->pushButtonAssignLevel->setText(tr("Assign selected level"));
		return;
	}

	const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel *>(obj);
	if (bl != nullptr) {
		m_currentBuildingLevel = bl;
		m_ui->groupBoxLevelProperties->setVisible(true);
		// update group box with properties
		m_ui->lineEditLevelName->setText(bl->m_displayName);
		m_ui->lineEditLevel->setText(QString("%L1").arg(bl->m_elevation));
		m_ui->lineEditHeight->setText(QString("%L1").arg(bl->m_height));

		m_ui->pushButtonRemoveLevel->setVisible(true);

		std::set<const VICUS::Object *> selObjs;
		project().selectObjects(selObjs, VICUS::Project::SG_Building, true, true);
		// do we have any rooms selected?
		int selectedRooms = 0;
		for (const VICUS::Object * o : selObjs)
			if (dynamic_cast<const VICUS::Room*>(o) != nullptr) {
				++selectedRooms;
				if (++selectedRooms > 1)
					break;
			}
		m_ui->pushButtonAssignRooms->setEnabled(selectedRooms != 0);
		if (selectedRooms > 1)
			m_ui->pushButtonAssignRooms->setText(tr("Assign selected rooms"));
		else
			m_ui->pushButtonAssignRooms->setText(tr("Assign selected room"));
		return;
	}
}


void SVFloorManagerWidget::on_lineEditBuildingName_editingFinished() {
	Q_ASSERT(m_currentBuilding != nullptr);
	// find index of building in project
	for (unsigned int i=0; i<project().m_buildings.size(); ++i) {
		if (&project().m_buildings[i] == m_currentBuilding) {
			// compose undo action for modifying building data
			VICUS::Building building = *m_currentBuilding;
			building.m_displayName = m_ui->lineEditBuildingName->text().trimmed();
			SVUndoModifyBuilding * undo = new SVUndoModifyBuilding(tr("Renaming building"), building, i);
			undo->push();
			return;
		}
	}
}
