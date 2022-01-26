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

#include "SVPropFloorManagerWidget.h"
#include "ui_SVPropFloorManagerWidget.h"

#include <limits>

#include <IBK_math.h>

#include <QInputDialog>
#include <QMessageBox>

#include <QtExt_Locale.h>

#include <VICUS_Project.h>
#include <VICUS_utilities.h>

#include "SVProjectHandler.h"
#include "SVUndoAddBuilding.h"
#include "SVUndoAddBuildingLevel.h"
#include "SVUndoDeleteBuilding.h"
#include "SVUndoDeleteBuildingLevel.h"
#include "SVUndoModifyBuilding.h"
#include "SVUndoModifyBuildingLevel.h"
#include "SVUndoModifyBuildingTopology.h"
#include "SVPropFloorManagerItemDelegate.h"
#include "SVStyle.h"

SVPropFloorManagerWidget::SVPropFloorManagerWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropFloorManagerWidget)
{
	m_ui->setupUi(this);

	m_ui->verticalLayout->setMargin(0);
	m_ui->verticalLayoutPage->setMargin(0);
	QStringList header;
	header << tr("Building/Floor") << tr("Elevation [m]") << tr("Height [m]");
	m_ui->treeWidget->setHeaderLabels(header);
	m_ui->treeWidget->setItemDelegate(new SVPropFloorManagerItemDelegate);
	SVStyle::formatDatabaseTreeView(m_ui->treeWidget);

	m_ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	m_ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

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
	m_ui->pushButtonAssignLevel->setEnabled(false);
	m_ui->pushButtonAssignRooms->setEnabled(false);

	if (sel.isEmpty())
		return;

	// get unique ID of selected object
	unsigned int selObjectUniqueId = sel.front()->data(0,Qt::UserRole).toUInt();

	m_currentBuilding = nullptr;
	m_currentBuildingLevel = nullptr;

	// and lookup object in project
	const VICUS::Object * obj = project().objectByUniqueId(selObjectUniqueId);
	const VICUS::Building * b = dynamic_cast<const VICUS::Building *>(obj);
	if (b != nullptr) {
		m_currentBuilding = b;

		m_ui->pushButtonAddLevel->setEnabled(true);
		m_ui->pushButtonRemoveBuilding->setEnabled(true);

		std::set<const VICUS::Object *> selObjs;
		project().selectObjects(selObjs, VICUS::Project::SG_Building, true, true);
		// do we have any levels selected?
		int selectedLevels = 0;
		for (const VICUS::Object * o : selObjs)
			if (dynamic_cast<const VICUS::BuildingLevel*>(o) != nullptr) {
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

		m_ui->pushButtonRemoveLevel->setEnabled(true);
		m_ui->pushButtonAddLevel->setEnabled(true);

		std::set<const VICUS::Object *> selObjs;
		project().selectObjects(selObjs, VICUS::Project::SG_Building, true, true);
		// do we have any rooms selected?
		int selectedRooms = 0;
		for (const VICUS::Object * o : selObjs)
			if (dynamic_cast<const VICUS::Room*>(o) != nullptr) {
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


void SVPropFloorManagerWidget::on_pushButtonAddBuilding_clicked() {
	std::set<QString> existingNames;
	for (const VICUS::Building & b : project().m_buildings)
		existingNames.insert(b.m_displayName);
	QString defaultName = VICUS::uniqueName(tr("Building"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building"), tr("New building name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;
	// modify project
	VICUS::Building b;
	b.m_id = VICUS::uniqueId(project().m_buildings);
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
	if (m_currentBuilding == nullptr) {
		Q_ASSERT(m_currentBuildingLevel != nullptr);
		m_currentBuilding = dynamic_cast<VICUS::Building*>(m_currentBuildingLevel->m_parent);
	}
	Q_ASSERT(m_currentBuilding != nullptr);

	unsigned int buildingUniqueID = m_currentBuilding->uniqueID();

	std::set<QString> existingNames;
	for (const VICUS::BuildingLevel & bl : m_currentBuilding->m_buildingLevels)
		existingNames.insert(bl.m_displayName);
	QString defaultName = VICUS::uniqueName(tr("Level"), existingNames);
	QString text = QInputDialog::getText(this, tr("Add building level"), tr("New building level/floor name:"), QLineEdit::Normal, defaultName).trimmed();
	if (text.isEmpty()) return;

	// modify project
	VICUS::BuildingLevel bl;
	bl.m_id = VICUS::uniqueId(m_currentBuilding->m_buildingLevels);
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

	// filter out all rooms
	std::vector<const VICUS::Room*> rooms;
	for (const VICUS::Object * o : objs) {
		const VICUS::Room* r = dynamic_cast<const VICUS::Room*>(o);
		if (r != nullptr)
			rooms.push_back(r);
	}

	// create a copy of the buildings vector and build levels, but skip over all selected rooms
	std::vector<VICUS::Building> buildingsCopy;
	for (const VICUS::Building & b : project().m_buildings) {
		VICUS::Building newB(b); // copy building including uniqueID
		newB.m_buildingLevels.clear();
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			VICUS::BuildingLevel newBl(bl);
			newBl.m_rooms.clear();  // copy building level including uniqueID
			for (const VICUS::Room & r : bl.m_rooms) {
				// if room is not selected, append it
				if (std::find(rooms.begin(), rooms.end(), &r) == rooms.end()) {
					newBl.m_rooms.push_back(r);
				}
			}
			// if this is our selected building level, add all selected rooms to it
			if (&bl == m_currentBuildingLevel) {
				for (const VICUS::Room* r : rooms)
					newBl.m_rooms.push_back(*r);
			}
			// finally add building level
			newB.m_buildingLevels.push_back(newBl);
		}
		buildingsCopy.push_back(newB);
	}

	// now compose an undo action to update the geometry
	SVUndoModifyBuildingTopology * undo = new SVUndoModifyBuildingTopology(tr("Assigned rooms to level"), buildingsCopy);
	undo->push();
}


void SVPropFloorManagerWidget::on_pushButtonAssignLevel_clicked() {
	// collect list of all selected levels and move them to the currently selected building
	// this means we are modifying at least two buildings, but only the topology (since the surfaces are drawn just the same)

	// collect all selected levels, and only if they are visible
	std::set<const VICUS::Object*> objs;
	project().selectObjects(objs, VICUS::Project::SG_Building, true, true);

	// filter out all levels
	std::vector<const VICUS::BuildingLevel*> levels;
	for (const VICUS::Object * o : objs) {
		const VICUS::BuildingLevel* r = dynamic_cast<const VICUS::BuildingLevel*>(o);
		if (r != nullptr)
			levels.push_back(r);
	}

	// create a copy of the buildings vector and build levels, but skip over all selected building levels
	std::vector<VICUS::Building> buildingsCopy;
	for (const VICUS::Building & b : project().m_buildings) {
		VICUS::Building newB(b); // copy building including uniqueID
		newB.m_buildingLevels.clear();
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			// if level is not selected, append it
			if (std::find(levels.begin(), levels.end(), &bl) == levels.end()) {
				newB.m_buildingLevels.push_back(bl);
			}
		}
		// if this is our selected building, add all selected levels to it
		if (&b == m_currentBuilding) {
			for (const VICUS::BuildingLevel* r : levels)
				newB.m_buildingLevels.push_back(*r);
		}
		buildingsCopy.push_back(newB);
	}

	// now compose an undo action to update the geometry
	SVUndoModifyBuildingTopology * undo = new SVUndoModifyBuildingTopology(tr("Assigned levels to building"), buildingsCopy);
	undo->push();
}


void SVPropFloorManagerWidget::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column) {
	// different handling for top-level items and child items
	if (item->parent() == nullptr) {
		if (column != 0)
			return;
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
			if (!ok) {
				return;
			}
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


