#include "SVPropFloorManagerWidget.h"
#include "ui_SVFloorManagerWidget.h"

#include <limits>

#include <IBK_math.h>

#include <QInputDialog>
#include <QMessageBox>

#include <QtExt_Locale.h>

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVUndoAddBuilding.h"
#include "SVUndoAddBuildingLevel.h"
#include "SVUndoDeleteBuilding.h"
#include "SVUndoDeleteBuildingLevel.h"
#include "SVUndoModifyBuilding.h"
#include "SVUndoModifyBuildingLevel.h"

SVPropFloorManagerWidget::SVPropFloorManagerWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropFloorManagerWidget)
{
	m_ui->setupUi(this);
	QStringList header;
	header << tr("Building/Floor") << tr("Elevation [m]") << tr("Height [m]");
	m_ui->treeWidget->setHeaderLabels(header);

	m_ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

	m_ui->lineEditHeight->setup(0, std::numeric_limits<double>::max(), tr("A positive number is required."), false, false);
	m_ui->lineEditLevel->setup(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), tr("A number is required."), false, false);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropFloorManagerWidget::onModified);

	onModified(SVProjectHandler::AllModified, nullptr); // update user interface to current project's state
}


SVPropFloorManagerWidget::~SVPropFloorManagerWidget() {
	delete m_ui;
}



void SVPropFloorManagerWidget::onModified(int modificationType, ModificationInfo * /*data*/) {
	switch ((SVProjectHandler::ModificationTypes)modificationType) {
		case SVProjectHandler::SolverParametersModified:
		case SVProjectHandler::ClimateLocationModified:
		case SVProjectHandler::GridModified:
		case SVProjectHandler::NetworkModified:
		case SVProjectHandler::ComponentInstancesModified:
			return; // unrelated changes, do nothing

		case SVProjectHandler::NodeStateModified: // need to listen to selection changes
		case SVProjectHandler::BuildingGeometryChanged:
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
	m_ui->treeWidget->blockSignals(true);
	m_ui->treeWidget->clear();
	QTreeWidgetItem * selectedItem = nullptr;
	for (const VICUS::Building & b : project().m_buildings) {
		QTreeWidgetItem * item = new QTreeWidgetItem(QStringList() << b.m_displayName);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		if (selectedItem == nullptr)
			selectedItem = item; // by default, always select the first one
		item->setData(0, Qt::UserRole, b.uniqueID());
		m_ui->treeWidget->addTopLevelItem(item);
		if (selObjectUniqueId == b.uniqueID())
			selectedItem = item;
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			QStringList columns;
			columns << bl.m_displayName;
			columns << QString("%L1").arg(bl.m_elevation, 0, 'g', 2);
			columns << QString("%L1").arg(bl.m_height, 0, 'g', 2);
			QTreeWidgetItem * levelItem = new QTreeWidgetItem(columns);
			levelItem->setData(0, Qt::UserRole, bl.uniqueID());
			levelItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
			item->addChild(levelItem);
			if (selObjectUniqueId == bl.uniqueID())
				selectedItem = levelItem;
		}
	}
	// reselect the previously selected item
	if (selectedItem != nullptr) {
		selectedItem->setSelected(true);
	}
	m_ui->treeWidget->expandAll();
	m_ui->treeWidget->blockSignals(false);

	on_treeWidget_itemSelectionChanged();
}


void SVPropFloorManagerWidget::on_treeWidget_itemSelectionChanged() {
	// show/hide buttons depending on selection
	QList<QTreeWidgetItem*> sel = m_ui->treeWidget->selectedItems();

	// hide all buttons first
	m_ui->pushButtonAddLevel->setEnabled(false);
	m_ui->pushButtonRemoveLevel->setEnabled(false);
	m_ui->pushButtonRemoveBuilding->setEnabled(false);

	if (sel.isEmpty())
		return;

	// get unique ID of selected object
	unsigned int selObjectUniqueId = sel.front()->data(0,Qt::UserRole).toUInt();

	m_currentBuilding = nullptr;
	m_currentBuildingLevel = nullptr;

	// default appearance - disabled building properties
	m_ui->stackedWidget->setEnabled(false);
	m_ui->stackedWidget->setCurrentIndex(0);
	m_ui->lineEditBuildingName->setText("");

	// and lookup object in project
	const VICUS::Object * obj = project().objectById(selObjectUniqueId);
	const VICUS::Building * b = dynamic_cast<const VICUS::Building *>(obj);
	if (b != nullptr) {
		m_currentBuilding = b;
		m_ui->stackedWidget->setEnabled(true);
		m_ui->stackedWidget->setCurrentIndex(0);
		// update group box with properties
		m_ui->lineEditBuildingName->setText(b->m_displayName);

		m_ui->pushButtonAddLevel->setEnabled(true);
		m_ui->pushButtonRemoveBuilding->setEnabled(true);

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
		m_ui->stackedWidget->setEnabled(true);
		m_ui->stackedWidget->setCurrentIndex(1);
		// update group box with properties
		m_ui->lineEditLevelName->setText(bl->m_displayName);
		m_ui->lineEditLevel->setText(QString("%L1").arg(bl->m_elevation));
		m_ui->lineEditHeight->setText(QString("%L1").arg(bl->m_height));

		m_ui->pushButtonRemoveLevel->setEnabled(true);

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


void SVPropFloorManagerWidget::on_lineEditBuildingName_editingFinished() {
	// guard against focus-out events calling with function when switching stacked widget
	if (m_currentBuilding == nullptr)
		return;
	// find index of building in project
	for (unsigned int i=0; i<project().m_buildings.size(); ++i) {
		if (&project().m_buildings[i] == m_currentBuilding) {
			QString newName = m_ui->lineEditBuildingName->text().trimmed();
			if (newName == m_currentBuilding->m_displayName)
				return; // nothing changed
			// compose undo action for modifying building data
			VICUS::Building building = *m_currentBuilding;
			building.m_buildingLevels.clear(); // no need to store these, since they are not modified
			building.m_displayName = newName;
			SVUndoModifyBuilding * undo = new SVUndoModifyBuilding(tr("Renamed building"), building, i, true);
			undo->push();
			return;
		}
	}
}


void SVPropFloorManagerWidget::on_lineEditLevelName_editingFinished() {
	// guard against focus-out events calling with function when switching stacked widget
	if (m_currentBuildingLevel == nullptr)
		return;
	m_ui->lineEditLevel->setFocus();
	// find index of building level in project
	for (unsigned int j=0; j<project().m_buildings.size(); ++j) {
		const VICUS::Building & b = project().m_buildings[j];
		for (unsigned int i=0; i<b.m_buildingLevels.size(); ++i) {
			if (&b.m_buildingLevels[i] == m_currentBuildingLevel) {
				QString newName = m_ui->lineEditLevelName->text().trimmed();
				if (newName == m_currentBuildingLevel->m_displayName)
					return; // nothing changed
				// compose undo action for modifying building data
				VICUS::BuildingLevel bl = *m_currentBuildingLevel;
				bl.m_rooms.clear(); // no need to store these, since they are not modified
				bl.m_displayName = newName;
				SVUndoModifyBuildingLevel * undo = new SVUndoModifyBuildingLevel(tr("Renamed building level"), bl, j, i, true);
				undo->push();
				m_ui->lineEditLevel->setFocus();
				return;
			}
		} // loop levels
	} // loop buildings
}


void SVPropFloorManagerWidget::on_lineEditLevel_editingFinishedSuccessfully() {
	// guard against focus-out events calling with function when switching stacked widget
	if (m_currentBuildingLevel == nullptr)
		return;
	// find index of building level in project
	for (unsigned int j=0; j<project().m_buildings.size(); ++j) {
		const VICUS::Building & b = project().m_buildings[j];
		for (unsigned int i=0; i<b.m_buildingLevels.size(); ++i) {
			if (&b.m_buildingLevels[i] == m_currentBuildingLevel) {
				double val = m_ui->lineEditLevel->value();
				if (IBK::near_equal(val, m_currentBuildingLevel->m_elevation, 1e-6))
					return; // nothing changed
				// compose undo action for modifying building data
				VICUS::BuildingLevel bl = *m_currentBuildingLevel;
				bl.m_rooms.clear(); // no need to store these, since they are not modified
				bl.m_elevation = val;
				SVUndoModifyBuildingLevel * undo = new SVUndoModifyBuildingLevel(tr("Modified building level"), bl, j, i, true);
				undo->push();
				m_ui->lineEditHeight->setFocus();
				return;
			}
		} // loop levels
	} // loop buildings
}



void SVPropFloorManagerWidget::on_lineEditHeight_editingFinishedSuccessfully() {
	// guard against focus-out events calling with function when switching stacked widget
	if (m_currentBuildingLevel == nullptr)
		return;
	// find index of building level in project
	for (unsigned int j=0; j<project().m_buildings.size(); ++j) {
		const VICUS::Building & b = project().m_buildings[j];
		for (unsigned int i=0; i<b.m_buildingLevels.size(); ++i) {
			if (&b.m_buildingLevels[i] == m_currentBuildingLevel) {
				double val = m_ui->lineEditHeight->value();
				if (IBK::near_equal(val, m_currentBuildingLevel->m_height, 1e-6))
					return; // nothing changed
				// compose undo action for modifying building data
				VICUS::BuildingLevel bl = *m_currentBuildingLevel;
				bl.m_rooms.clear(); // no need to store these, since they are not modified
				bl.m_height = val;
				SVUndoModifyBuildingLevel * undo = new SVUndoModifyBuildingLevel(tr("Modified building level"), bl, j, i, true);
				undo->push();
				return;
			}
		} // loop levels
	} // loop buildings
}



void SVPropFloorManagerWidget::on_pushButtonAddBuilding_clicked() {
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
	undo->push(); // this will update our tree widget
	// now select the newly created building node

	Q_ASSERT(m_ui->treeWidget->topLevelItemCount() > 0);
	QTreeWidgetItem * lastTopLevelNode = m_ui->treeWidget->topLevelItem( m_ui->treeWidget->topLevelItemCount()-1);
	m_ui->treeWidget->blockSignals(true);
	m_ui->treeWidget->selectionModel()->clearSelection();
	lastTopLevelNode->setSelected(true);
	m_ui->treeWidget->blockSignals(false);
	on_treeWidget_itemSelectionChanged();
}



void SVPropFloorManagerWidget::on_pushButtonAddLevel_clicked() {
	// get currently selected building
	Q_ASSERT(m_currentBuilding != nullptr);
	unsigned int buildingUniqueID = m_currentBuilding->uniqueID();

	std::set<QString> existingNames;
	for (const VICUS::BuildingLevel & bl : m_currentBuilding->m_buildingLevels)
		existingNames.insert(bl.m_displayName);
	QString defaultName = VICUS::Project::uniqueName(tr("Level"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building level"), tr("New building level/floor name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;

	// modify project
	VICUS::BuildingLevel bl;
	bl.m_id = VICUS::Project::uniqueId(m_currentBuilding->m_buildingLevels);
	bl.m_displayName = text;
	SVUndoAddBuildingLevel * undo = new SVUndoAddBuildingLevel(tr("Adding building level '%1'").arg(bl.m_displayName), buildingUniqueID, bl, true);
	undo->push(); // this will update our tree widget

	// find newly added tree node

	QTreeWidgetItem * blNode = nullptr;
	for (int i=0; i<m_ui->treeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem * buildingNode = m_ui->treeWidget->topLevelItem(i);
		if (buildingNode->data(0, Qt::UserRole).toUInt() == buildingUniqueID) {
			blNode = buildingNode->child( buildingNode->childCount()-1);
			break;
		}
	}
	Q_ASSERT(blNode != nullptr);
	m_ui->treeWidget->blockSignals(true);
	m_ui->treeWidget->selectionModel()->clearSelection();
	blNode->setSelected(true);
	m_ui->treeWidget->blockSignals(false);
	on_treeWidget_itemSelectionChanged();
}


void SVPropFloorManagerWidget::on_pushButtonRemoveBuilding_clicked() {
	Q_ASSERT(m_currentBuilding != nullptr);
	// warn user if building has building levels
	if (!m_currentBuilding->m_buildingLevels.empty()) {
		int res = QMessageBox::question(this, QString(), tr("The selected building still contains building levels, which will also be removed including all rooms they contain. Continue?"),
							  QMessageBox::Yes | QMessageBox::No);
		if (res != QMessageBox::Yes)
			return;
	}
	// compose undo action

	// find index in project's building vector
	for (unsigned int i=0; i<project().m_buildings.size(); ++i) {
		if (&project().m_buildings[i] == m_currentBuilding) {
			SVUndoDeleteBuilding * undo = new SVUndoDeleteBuilding(tr("Removed building"), i);
			undo->push();
			return;
		}
	}
}


void SVPropFloorManagerWidget::on_pushButtonRemoveLevel_clicked() {
	Q_ASSERT(m_currentBuildingLevel != nullptr);
	// warn user if building level has rooms
	if (!m_currentBuildingLevel->m_rooms.empty()) {
		int res = QMessageBox::question(this, QString(), tr("The selected building level still contains rooms, which will also be removed including all surfaces they contain. Continue?"),
							  QMessageBox::Yes | QMessageBox::No);
		if (res != QMessageBox::Yes)
			return;
	}
	// compose undo action

	// get parent building object (Note: do not use m_currentBuilding here!)
	const VICUS::Building * building = dynamic_cast<const VICUS::Building *>(m_currentBuildingLevel->m_parent);

	// find index in project's building vector
	for (unsigned int i=0; i<project().m_buildings.size(); ++i) {
		if (&project().m_buildings[i] == building) {
			// now we have the index of the modified building, now also find the index of the deleted building level
			for (unsigned int j=0; j<building->m_buildingLevels.size(); ++j) {
				if (&building->m_buildingLevels[j] == m_currentBuildingLevel) {
					SVUndoDeleteBuildingLevel * undo = new SVUndoDeleteBuildingLevel(tr("Removed building level"), i, j);
					undo->push();
					return;
				}
			} // loop levels
		}
	} // loop buildings
}


void SVPropFloorManagerWidget::on_pushButtonAssignRooms_clicked() {
	// collect list of all selected rooms and move them to the currently selected building level
	// this means we are modifying one or more buildings, but only the topology (since the surfaces are drawn just the same)

	// collect all selected rooms, and only if they are visible
	std::set<const VICUS::Object*> objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, true, true);

	// create a copy of the buildings vector


}

void SVPropFloorManagerWidget::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column) {
	// different handling for top-level items and child items
	if (item->parent() == nullptr) {
		Q_ASSERT(column == 0);
		QString newName = item->text(0);
		if (newName == m_currentBuilding->m_displayName)
			return; // nothing changed
		for (unsigned int i=0; i<project().m_buildings.size(); ++i) {
			if (&project().m_buildings[i] == m_currentBuilding) {
				// compose undo action for modifying building data
				VICUS::Building building = *m_currentBuilding;
				building.m_buildingLevels.clear(); // no need to store these, since they are not modified
				building.m_displayName = newName;
				SVUndoModifyBuilding * undo = new SVUndoModifyBuilding(tr("Renamed building"), building, i, true);
				undo->push();
				return;
			}
		}
		return;
	}
	Q_ASSERT(m_currentBuildingLevel != nullptr);
	switch (column) {
		case 0 : {
			QString newName = item->text(0);
			if (newName == m_currentBuildingLevel->m_displayName)
				return; // nothing changed
			// find index of building level in project
			for (unsigned int j=0; j<project().m_buildings.size(); ++j) {
				const VICUS::Building & b = project().m_buildings[j];
				for (unsigned int i=0; i<b.m_buildingLevels.size(); ++i) {
					if (&b.m_buildingLevels[i] == m_currentBuildingLevel) {
						// compose undo action for modifying building data
						VICUS::BuildingLevel bl = *m_currentBuildingLevel;
						bl.m_rooms.clear(); // no need to store these, since they are not modified
						bl.m_displayName = newName;
						SVUndoModifyBuildingLevel * undo = new SVUndoModifyBuildingLevel(tr("Renamed building level"), bl, j, i, true);
						undo->push();
						return;
					}
				} // loop levels
			} // loop buildings
		} break;

		case 1 : {
			bool ok;
			double val = QtExt::Locale().toDoubleWithFallback(item->text(column), &ok);
			if (!ok)
				return; // ignore, delegate handles highlighting of invalid values
			// find index of building level in project
			for (unsigned int j=0; j<project().m_buildings.size(); ++j) {
				const VICUS::Building & b = project().m_buildings[j];
				for (unsigned int i=0; i<b.m_buildingLevels.size(); ++i) {
					if (&b.m_buildingLevels[i] == m_currentBuildingLevel) {
						if (IBK::near_equal(val, m_currentBuildingLevel->m_elevation, 1e-6))
							return; // nothing changed
						// compose undo action for modifying building data
						VICUS::BuildingLevel bl = *m_currentBuildingLevel;
						bl.m_rooms.clear(); // no need to store these, since they are not modified
						bl.m_elevation = val;
						SVUndoModifyBuildingLevel * undo = new SVUndoModifyBuildingLevel(tr("Modified floor elevation"), bl, j, i, true);
						undo->push();
						return;
					}
				} // loop levels
			} // loop buildings
		} break;

		case 2 : {
			bool ok;
			double val = QtExt::Locale().toDoubleWithFallback(item->text(column), &ok);
			if (!ok)
				return; // ignore, delegate handles highlighting of invalid values
			// find index of building level in project
			for (unsigned int j=0; j<project().m_buildings.size(); ++j) {
				const VICUS::Building & b = project().m_buildings[j];
				for (unsigned int i=0; i<b.m_buildingLevels.size(); ++i) {
					if (&b.m_buildingLevels[i] == m_currentBuildingLevel) {
						if (IBK::near_equal(val, m_currentBuildingLevel->m_height, 1e-6))
							return; // nothing changed
						// compose undo action for modifying building data
						VICUS::BuildingLevel bl = *m_currentBuildingLevel;
						bl.m_rooms.clear(); // no need to store these, since they are not modified
						bl.m_height = val;
						SVUndoModifyBuildingLevel * undo = new SVUndoModifyBuildingLevel(tr("Modified floor height"), bl, j, i, true);
						undo->push();
						return;
					}
				} // loop levels
			} // loop buildings
		} break;
	}

}
