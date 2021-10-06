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

#include "SVNavigationTreeWidget.h"
#include "ui_SVNavigationTreeWidget.h"

#include <QTreeWidgetItem>
#include <QElapsedTimer>

#include <VICUS_Project.h>
#include <VICUS_KeywordList.h>

#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "SVNavigationTreeItemDelegate.h"
#include "SVUndoTreeNodeState.h"
#include "SVUndoModifyObjectName.h"
#include "SVViewStateHandler.h"
#include "SVSmartSelectDialog.h"

SVNavigationTreeWidget::SVNavigationTreeWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVNavigationTreeWidget)
{
	m_ui->setupUi(this);

	// give others access to us
	SVViewStateHandler::instance().m_navigationTreeWidget = this;

	SVStyle::formatWidgetWithLayout(this);
	SVStyle::formatDatabaseTreeView(m_ui->treeWidget);

	// register item delegate that paints the "visible" bulb
	m_ui->treeWidget->setItemDelegate(new SVNavigationTreeItemDelegate(this));
	m_ui->treeWidget->setUniformRowHeights(true);


	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVNavigationTreeWidget::onModified);

	addAction(m_ui->actionDeselect_all);
}


SVNavigationTreeWidget::~SVNavigationTreeWidget() {
	delete m_ui;
}


void SVNavigationTreeWidget::onModified(int modificationType, ModificationInfo * data) {
	// filter out all modification types that we handle
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		case SVProjectHandler::AllModified :
		case SVProjectHandler::NetworkModified :
		case SVProjectHandler::BuildingGeometryChanged :
		case SVProjectHandler::BuildingTopologyChanged :
			/// \todo Andreas: parse 'data' to determine what has changed and avoid updating entire tree (and losing collapsed state)
		break;

		case SVProjectHandler::ObjectRenamed : {
			SVUndoModifyObjectName::Data * d = dynamic_cast<SVUndoModifyObjectName::Data *>(data);
			Q_ASSERT(d != nullptr);
			// modify tree item
			QTreeWidgetItem * item = m_treeItemMap[d->m_object->uniqueID()];
			Q_ASSERT(item != nullptr);
			m_ui->treeWidget->blockSignals(true);
			item->setText(0, d->m_object->m_displayName);
			m_ui->treeWidget->blockSignals(false);
			return;
		}

		case SVProjectHandler::NodeStateModified : {
			// we only change data properties of existing nodes and emit itemChanged() signals, so
			// that the view updates its content

			QElapsedTimer timer;
			timer.start();

			// first decode the modification info object
			const SVUndoTreeNodeState::ModifiedNodes * info = dynamic_cast<SVUndoTreeNodeState::ModifiedNodes *>(data);
			Q_ASSERT(info != nullptr);

			for (unsigned int uID : info->m_nodeIDs) {
				const VICUS::Object * o = project().objectById(uID);
				auto itemId = m_treeItemMap.find(uID);
				Q_ASSERT(itemId != m_treeItemMap.end());
				QTreeWidgetItem * item = itemId->second;
				m_ui->treeWidget->blockSignals(true); // prevent side effects from "setData()"
				item->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, o->m_visible);
				item->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, o->m_selected);
				m_ui->treeWidget->blockSignals(false);
			}

			qDebug() << timer.elapsed() << "ms for navigation model node state update.";
			return; // nothing else to do here
		}

		default:
			return; // do nothing by default
	}

	QElapsedTimer timer;
	timer.start();

	// for now, rebuild the entire tree
	m_ui->treeWidget->blockSignals(true);
	m_ui->treeWidget->clear();
	// populate tree widget

	m_treeItemMap.clear();

	// insert root node
	QTreeWidgetItem * root = new QTreeWidgetItem(QStringList() << "Site", QTreeWidgetItem::Type);
//	root->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, true);
//	root->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, true);
	m_ui->treeWidget->addTopLevelItem(root);

	// get project data
	const VICUS::Project & prj = project();

	// Buildings
	for (const VICUS::Building & b : prj.m_buildings) {
		QTreeWidgetItem * building = new QTreeWidgetItem(QStringList() << tr("Building: %1").arg(b.m_displayName), QTreeWidgetItem::Type);
		m_treeItemMap[b.uniqueID()] = building;
		building->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
		building->setData(0, SVNavigationTreeItemDelegate::NodeID, b.uniqueID());
		building->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, b.m_visible);
		building->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, b.m_selected);
		root->addChild(building);
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			QTreeWidgetItem * buildingLevel = new QTreeWidgetItem(QStringList() << bl.m_displayName, QTreeWidgetItem::Type);
			m_treeItemMap[bl.uniqueID()] = buildingLevel;
			buildingLevel->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::NodeID, bl.uniqueID());
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, bl.m_visible);
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, bl.m_selected);
			building->addChild(buildingLevel);
			for (const VICUS::Room & r : bl.m_rooms) {
				QTreeWidgetItem * rooms = new QTreeWidgetItem(QStringList() << r.m_displayName, QTreeWidgetItem::Type);
				m_treeItemMap[r.uniqueID()] = rooms;
				rooms->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
				rooms->setData(0, SVNavigationTreeItemDelegate::NodeID, r.uniqueID());
				rooms->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, r.m_visible);
				rooms->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, r.m_selected);
				if (rooms->text(0).isEmpty())
					rooms->setText(0, tr("unnamed"));
				buildingLevel->addChild(rooms);
				for (const VICUS::Surface & s : r.m_surfaces) {
					QTreeWidgetItem * surface = new QTreeWidgetItem(QStringList() << s.m_displayName, QTreeWidgetItem::Type);
					m_treeItemMap[s.uniqueID()] = surface;
					rooms->addChild(surface);
					surface->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
					surface->setData(0, SVNavigationTreeItemDelegate::NodeID, s.uniqueID());
					surface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, s.m_visible);
					surface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, s.m_selected);
					for (unsigned int holeIdx = 0; holeIdx < s.subSurfaces().size(); ++holeIdx) {
						const VICUS::SubSurface & sub = s.subSurfaces()[holeIdx];

						QTreeWidgetItem * subsurface = new QTreeWidgetItem(QStringList() << sub.m_displayName, QTreeWidgetItem::Type);
						m_treeItemMap[sub.uniqueID()] = subsurface;
						subsurface->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

						// mark invalid subsurfaces in red and give tooltip with error
						if (!s.geometry().holes()[holeIdx].isValid()) {
							subsurface->setForeground(0, QColor(128,0,0));
							subsurface->setToolTip(0, tr("Invalid polygon data"));
						}

						surface->addChild(subsurface);
						subsurface->setData(0, SVNavigationTreeItemDelegate::NodeID, sub.uniqueID());
						subsurface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, sub.m_visible);
						subsurface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, sub.m_selected);
					}
				}
			}
		}
	}

	const SVDatabase  & db = SVSettings::instance().m_db;

	// Networks
	for (const VICUS::Network & n : prj.m_geometricNetworks) {
		QTreeWidgetItem * networkItem = new QTreeWidgetItem(QStringList() << tr("Network: %1").arg(n.m_displayName), QTreeWidgetItem::Type);
		m_treeItemMap[n.uniqueID()] = networkItem;
		root->addChild(networkItem);
		networkItem->setData(0, SVNavigationTreeItemDelegate::NodeID, n.uniqueID());
		networkItem->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, n.m_visible);
		networkItem->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, n.m_selected);
		QTreeWidgetItem * enode = new QTreeWidgetItem(QStringList() << tr("Edges"), QTreeWidgetItem::Type);
		enode->setFlags(Qt::ItemIsEnabled); // cannot select "Edges"
		networkItem->addChild(enode);
		// add child nodes for each edge in the network
		/// TODO : Hauke, think about grouping for larger networks
		for (const VICUS::NetworkEdge & e : n.m_edges) {
			QString name = QString("[%1->%2]").arg(e.nodeId1()).arg(e.nodeId2());
			const VICUS::NetworkPipe * pipe = db.m_pipes[e.m_idPipe];
			if (pipe != nullptr)
				name += " "  + QString::fromStdString(pipe->m_displayName.string());
			QTreeWidgetItem * en = new QTreeWidgetItem(QStringList() << name, QTreeWidgetItem::Type);
			m_treeItemMap[e.uniqueID()] = en;
			en->setData(0, SVNavigationTreeItemDelegate::NodeID, e.uniqueID());
			en->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, e.m_visible);
			en->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, e.m_selected);
			enode->addChild(en);
		}

		QTreeWidgetItem * nnode = new QTreeWidgetItem(QStringList() << tr("Nodes"), QTreeWidgetItem::Type);
		nnode->setFlags(Qt::ItemIsEnabled); // cannot select "Nodes"
		networkItem->addChild(nnode);
		// add child nodes for each edge in the network
		/// TODO : Hauke, think about grouping for larger networks
		for (const VICUS::NetworkNode & nod : n.m_nodes) {
			QString name = QString("[%1] %2").arg(nod.m_id).arg(VICUS::KeywordList::Keyword("NetworkNode::NodeType", nod.m_type));
			QTreeWidgetItem * no = new QTreeWidgetItem(QStringList() << name, QTreeWidgetItem::Type);
			m_treeItemMap[nod.uniqueID()] = no;
			no->setData(0, SVNavigationTreeItemDelegate::NodeID, nod.uniqueID());
			no->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, nod.m_visible);
			no->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, nod.m_selected);
			nnode->addChild(no);
		}
	}

	// Dumb plain geometry
	if (!prj.m_plainGeometry.empty()) {
		QTreeWidgetItem * plainGeo = new QTreeWidgetItem(QStringList() << tr("Obstacles/Shading Geometry"), QTreeWidgetItem::Type);
		plainGeo->setFlags(Qt::ItemIsEnabled);
		plainGeo->setData(0, SVNavigationTreeItemDelegate::NodeID, 0); // not a child node
		plainGeo->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, false);
		plainGeo->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, false);
		root->addChild(plainGeo);
		for (const VICUS::Surface & s : prj.m_plainGeometry) {
			QTreeWidgetItem * surface = new QTreeWidgetItem(QStringList() << s.m_displayName, QTreeWidgetItem::Type);
			m_treeItemMap[s.uniqueID()] = surface;
			plainGeo->addChild(surface);
			surface->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			surface->setData(0, SVNavigationTreeItemDelegate::NodeID, s.uniqueID());
			surface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, s.m_visible);
			surface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, s.m_selected);
		}
	}

	m_ui->treeWidget->blockSignals(false);
	qDebug() << timer.elapsed() << "ms for navigation model reset.";

	m_ui->treeWidget->expandAll();
}



void SVNavigationTreeWidget::collapseTreeWidgetItem(QTreeWidgetItem * parent) {
	for (int i=0; i<parent->childCount(); ++i) {
		QTreeWidgetItem * c = parent->child(i);
		m_ui->treeWidget->blockSignals(true);
		m_ui->treeWidget->collapseItem(c);
		m_ui->treeWidget->blockSignals(false);
		collapseTreeWidgetItem(c);
	}
}


void SVNavigationTreeWidget::on_treeWidget_itemCollapsed(QTreeWidgetItem *item) {
	// also collapse all children
	collapseTreeWidgetItem(item);
}



void SVNavigationTreeWidget::on_actionRemoveSelected_triggered() {
	emit removeSelected();
}

void SVNavigationTreeWidget::on_actionShowSelected_triggered() {
	emit showSelected();
}

void SVNavigationTreeWidget::on_actionHideSelected_triggered() {
	emit hideSelected();
}

void SVNavigationTreeWidget::on_actionSelect_all_triggered() {
	emit selectAll();
}

void SVNavigationTreeWidget::on_actionDeselect_all_triggered() {
	emit deselectAll();
}


void SVNavigationTreeWidget::on_actionSmartSelect_triggered() {
	// show smart select dialog
	if (m_smartSelectDialog == nullptr)
		m_smartSelectDialog = new SVSmartSelectDialog(this);
	m_smartSelectDialog->select(); // selection undo actions are created in the dialog, nothing else to do here
}


void SVNavigationTreeWidget::on_treeWidget_itemChanged(QTreeWidgetItem *item, int /*column*/) {
	// create an undo-action for renaming of the item
	// but first check that item isn't empty
	QString newText = item->text(0).trimmed();
	unsigned int uID = item->data(0, SVNavigationTreeItemDelegate::NodeID).toUInt();
	// lookup object in project data structure
	const VICUS::Object * o = project().objectById(uID);
	if (o == nullptr) {
		qDebug() << "Invalid/unknown uid of tree node item";
		return;
	}
	if (newText.isEmpty()) {
		// reset value
		m_ui->treeWidget->blockSignals(true);
		item->setText(0, o->m_displayName);
		m_ui->treeWidget->blockSignals(false);
		return;
	}
	// create undo action for renaming of object
	SVUndoModifyObjectName * undo = new SVUndoModifyObjectName(tr("Renamed object to '%1'").arg(newText), o, newText);
	undo->push();
}
