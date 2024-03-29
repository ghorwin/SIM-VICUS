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
#include "SVUndoModifyRoomZoneTemplateAssociation.h"
#include "SVViewStateHandler.h"
#include "SVSmartSelectDialog.h"
#include "SVPropEditGeometry.h"


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
	m_navigationTreeItemDelegate = new SVNavigationTreeItemDelegate(m_ui->treeWidget); // Mind: treeView must be parent of delegate, see SVNavigationTreeItemDelegate::paint()
	m_ui->treeWidget->setItemDelegate(m_navigationTreeItemDelegate);
	m_ui->treeWidget->setUniformRowHeights(true);

	m_ui->actionSmartSelect->setShortcut(QKeySequence((int)Qt::CTRL + Qt::Key_F));
	m_ui->actionInvertSelection->setShortcut(QKeySequence((int)Qt::CTRL + Qt::Key_I));

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVNavigationTreeWidget::onModified);

	addAction(m_ui->actionDeselectAll);

	m_ui->actionDeselectAll->setIcon(QIcon::fromTheme("deselect_items"));
	m_ui->actionSelectAll->setIcon(QIcon::fromTheme("select_items"));
	m_ui->actionShowSelected->setIcon(QIcon::fromTheme("show_items"));
	m_ui->actionHideSelected->setIcon(QIcon::fromTheme("hide_items"));
	m_ui->actionRemoveSelected->setIcon(QIcon::fromTheme("delete_items"));
	m_ui->actionSmartSelect->setIcon(QIcon::fromTheme("smart_select"));
	m_ui->actionInvertSelection->setIcon(QIcon::fromTheme("invert_selection"));

}


SVNavigationTreeWidget::~SVNavigationTreeWidget() {
	delete m_ui;
}


void SVNavigationTreeWidget::addChildSurface(QTreeWidgetItem *item, const VICUS::Surface &s) {

	for (unsigned int holeIdx = 0; holeIdx < s.geometry().holes().size(); ++holeIdx) {

		const VICUS::PlaneGeometry::Hole &h = s.geometry().holes()[holeIdx];

		const VICUS::Object *obj = SVProjectHandler::instance().project().objectById(h.m_idObject);

		const VICUS::SubSurface *subSurf = dynamic_cast<const VICUS::SubSurface*>(obj);

		if(subSurf != nullptr) {
			QTreeWidgetItem * subsurface = new QTreeWidgetItem(QStringList() << subSurf->m_displayName, QTreeWidgetItem::Type);
			m_treeItemMap[subSurf->m_id] = subsurface;
			subsurface->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

			// mark invalid subsurfaces in red and give tooltip with error
			if (!s.geometry().holes()[holeIdx].m_holeGeometry.isValid()) {
				subsurface->setForeground(0, QColor(128,0,0));
				subsurface->setToolTip(0, tr("Invalid polygon data"));
			}

			item->addChild(subsurface);
			subsurface->setData(0, SVNavigationTreeItemDelegate::NodeID, subSurf->m_id);
			subsurface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, subSurf->m_visible);
			subsurface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, subSurf->m_selected);
			subsurface->setData(0, SVNavigationTreeItemDelegate::ItemType, SVNavigationTreeItemDelegate::TT_Subsurface);
			if (!subSurf->m_polygon2D.isValid()) {
				subsurface->setData(0, SVNavigationTreeItemDelegate::InvalidGeometryFlag, true);
				subsurface->setData(0, Qt::ToolTipRole, tr("Invalid sub-surface polygon"));
			}

			continue;
		}

		const VICUS::Surface *surf = dynamic_cast<const VICUS::Surface*>(obj);

		if(surf != nullptr) {
			QTreeWidgetItem * childSurface = new QTreeWidgetItem(QStringList() << surf->m_displayName, QTreeWidgetItem::Type);
			m_treeItemMap[surf->m_id] = childSurface;
			childSurface->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

			// mark invalid subsurfaces in red and give tooltip with error
			if (!s.geometry().holes()[holeIdx].m_holeGeometry.isValid()) {
				childSurface->setForeground(0, QColor(128,0,0));
				childSurface->setToolTip(0, tr("Invalid polygon data"));
			}

			item->addChild(childSurface);
			childSurface->setData(0, SVNavigationTreeItemDelegate::NodeID, surf->m_id);
			childSurface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, surf->m_visible);
			childSurface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, surf->m_selected);
			if (!surf->polygon3D().isValid()) {
				childSurface->setData(0, SVNavigationTreeItemDelegate::InvalidGeometryFlag, true);
				childSurface->setData(0, Qt::ToolTipRole, tr("Invalid child-surface polygon"));
			}

			for(const VICUS::Surface &childSurf : s.childSurfaces())
				addChildSurface(childSurface, childSurf);
		}

	}
}


void SVNavigationTreeWidget::onModified(int modificationType, ModificationInfo * data) {
	// filter out all modification types that we handle
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
	case SVProjectHandler::AllModified :
	case SVProjectHandler::NetworkGeometryChanged :
	case SVProjectHandler::DrawingModified :
	case SVProjectHandler::BuildingGeometryChanged :
		break;
	case SVProjectHandler::BuildingTopologyChanged : {
		/// \todo Andreas: parse 'data' to determine what has changed and avoid updating entire tree (and losing collapsed state)
		SVUndoModifyRoomZoneTemplateAssociation::Data * d = dynamic_cast<SVUndoModifyRoomZoneTemplateAssociation::Data *>(data);

		// not a RoomZoneTemplateAssociation? In this case update the entire tree
		if (d == nullptr)
			break;	// leave switch case and update entire tree
		return; // do nothing, this topology change does not require rebuild of nav tree
	}

	case SVProjectHandler::ObjectRenamed : {
		SVUndoModifyObjectName::Data * d = dynamic_cast<SVUndoModifyObjectName::Data *>(data);
		Q_ASSERT(d != nullptr);
		// modify tree item
		QTreeWidgetItem * item = m_treeItemMap[d->m_object->m_id];
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

		for (unsigned int ID : info->m_nodeIDs) {
			if(ID == 0) { // Special handling for plain geometry
				auto itemId = m_treeItemMap.find(0);
				QTreeWidgetItem * item = itemId->second;
				Q_ASSERT(item!=nullptr);
				m_ui->treeWidget->blockSignals(true); // prevent side effects from "setData()"
				item->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, project().m_plainGeometry.m_visible);
				item->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, project().m_plainGeometry.m_selected);
				m_ui->treeWidget->blockSignals(false);
			}
			else {
				const VICUS::Object * o = project().objectById(ID);
				if (o == nullptr) {
					qCritical() << "Object with ID" << ID << "does not exist!";
					continue;
				}
				auto itemId = m_treeItemMap.find(ID);
				if (itemId == m_treeItemMap.end()) {
					qCritical() << "Tree node for object with ID" << ID << "does not exist!";
					continue;
				}
				QTreeWidgetItem * item = itemId->second;
				m_ui->treeWidget->blockSignals(true); // prevent side effects from "setData()"
				item->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, o->m_visible);
				item->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, o->m_selected);
			}
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
	root->setFlags(Qt::ItemIsEnabled);
	//	root->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, true);
	//	root->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, true);
	m_ui->treeWidget->addTopLevelItem(root);

	// get project data
	const VICUS::Project & prj = project();

	// Buildings
	for (const VICUS::Building & b : prj.m_buildings) {
		QTreeWidgetItem * building = new QTreeWidgetItem(QStringList() << b.m_displayName, QTreeWidgetItem::Type);
		m_treeItemMap[b.m_id] = building;
		building->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
		building->setData(0, SVNavigationTreeItemDelegate::NodeID, b.m_id);
		building->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, b.m_visible);
		building->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, b.m_selected);
		building->setData(0, SVNavigationTreeItemDelegate::ItemType, SVNavigationTreeItemDelegate::TT_Building);
		root->addChild(building);
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			QTreeWidgetItem * buildingLevel = new QTreeWidgetItem(QStringList() << bl.m_displayName, QTreeWidgetItem::Type);
			m_treeItemMap[bl.m_id] = buildingLevel;
			buildingLevel->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::NodeID, bl.m_id);
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, bl.m_visible);
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, bl.m_selected);
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::ItemType, SVNavigationTreeItemDelegate::TT_BuildingLevel);
			building->addChild(buildingLevel);
			for (const VICUS::Room & r : bl.m_rooms) {
				QTreeWidgetItem * rooms = new QTreeWidgetItem(QStringList() << r.m_displayName, QTreeWidgetItem::Type);
				m_treeItemMap[r.m_id] = rooms;
				rooms->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
				rooms->setData(0, SVNavigationTreeItemDelegate::NodeID, r.m_id);
				rooms->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, r.m_visible);
				rooms->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, r.m_selected);
				rooms->setData(0, SVNavigationTreeItemDelegate::InvalidGeometryFlag, false);
				rooms->setData(0, SVNavigationTreeItemDelegate::ItemType, SVNavigationTreeItemDelegate::TT_Room);
				if (rooms->text(0).isEmpty())
					rooms->setText(0, tr("unnamed"));
				buildingLevel->addChild(rooms);
				for (const VICUS::Surface & s : r.m_surfaces) {
					QTreeWidgetItem * surface = new QTreeWidgetItem(QStringList() << s.m_displayName, QTreeWidgetItem::Type);
					m_treeItemMap[s.m_id] = surface;
					rooms->addChild(surface);
					surface->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
					surface->setData(0, SVNavigationTreeItemDelegate::NodeID, s.m_id);
					surface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, s.m_visible);
					surface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, s.m_selected);
					surface->setData(0, SVNavigationTreeItemDelegate::ItemType, SVNavigationTreeItemDelegate::TT_Surface);
					if (!s.geometry().isValid()) {
						surface->setData(0, SVNavigationTreeItemDelegate::InvalidGeometryFlag, true);
						surface->setData(0, Qt::ToolTipRole, tr("Invalid polygon/hole geometry"));
					}

					addChildSurface(surface, s);
				}
			}
		}
	}

	const SVDatabase  & db = SVSettings::instance().m_db;

	// Networks
	for (const VICUS::Network & n : prj.m_geometricNetworks) {
		QTreeWidgetItem * networkItem = new QTreeWidgetItem(QStringList() << n.m_displayName, QTreeWidgetItem::Type);
		m_treeItemMap[n.m_id] = networkItem;
		root->addChild(networkItem);
		networkItem->setData(0, SVNavigationTreeItemDelegate::NodeID, n.m_id);
		networkItem->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, n.m_visible);
		networkItem->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, n.m_selected);
		QTreeWidgetItem * enode = new QTreeWidgetItem(QStringList() << tr("Edges"), QTreeWidgetItem::Type);
		enode->setFlags(Qt::ItemIsEnabled); // cannot select "Edges"
		enode->setData(0, SVNavigationTreeItemDelegate::NodeID, 0);
		networkItem->addChild(enode);
		// add child nodes for each edge in the network
		/// TODO : Hauke, think about grouping for larger networks
		for (const VICUS::NetworkEdge & e : n.m_edges) {
			QString name = QString("[%1->%2]").arg(e.nodeId1()).arg(e.nodeId2());
			const VICUS::NetworkPipe * pipe = db.m_pipes[e.m_idPipe];
			if (pipe != nullptr)
				name += " "  + QString::fromStdString(pipe->m_displayName.string());
			QTreeWidgetItem * en = new QTreeWidgetItem(QStringList() << name, QTreeWidgetItem::Type);
			m_treeItemMap[e.m_id] = en;
			en->setData(0, SVNavigationTreeItemDelegate::NodeID, e.m_id);
			en->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, e.m_visible);
			en->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, e.m_selected);
			enode->addChild(en);
		}

		QTreeWidgetItem * nnode = new QTreeWidgetItem(QStringList() << tr("Nodes"), QTreeWidgetItem::Type);
		nnode->setFlags(Qt::ItemIsEnabled); // cannot select "Nodes"
		nnode->setData(0, SVNavigationTreeItemDelegate::NodeID, 0);
		networkItem->addChild(nnode);
		// add child nodes for each edge in the network
		/// TODO : Hauke, think about grouping for larger networks
		for (const VICUS::NetworkNode & nod : n.m_nodes) {
			QString name = QString("[%1] %2 %3").arg(nod.m_id).arg(VICUS::KeywordList::Keyword("NetworkNode::NodeType", nod.m_type)).arg(nod.m_displayName);
			QTreeWidgetItem * no = new QTreeWidgetItem(QStringList() << name, QTreeWidgetItem::Type);
			m_treeItemMap[nod.m_id] = no;
			no->setData(0, SVNavigationTreeItemDelegate::NodeID, nod.m_id);
			no->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, nod.m_visible);
			no->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, nod.m_selected);
			nnode->addChild(no);
		}
	}

	// DXF Drawings
	// missing file?
	if (prj.m_drawings.empty()) {
		IBK::Path absDrawFilePath = SVProjectHandler::instance().replacePathPlaceholders(prj.m_drawingFilePath);
		if (absDrawFilePath.isValid() && !absDrawFilePath.exists()) {
			QTreeWidgetItem * drawing = new QTreeWidgetItem(QStringList() << tr("Missing drawing file '%1'").arg(QString::fromStdString(absDrawFilePath.str())), QTreeWidgetItem::Type);
			drawing->setData(0, SVNavigationTreeItemDelegate::MissingDrawingFile, QString::fromStdString(absDrawFilePath.str()));
			drawing->setToolTip(0, tr("Double click to find missing file."));
			m_ui->treeWidget->addTopLevelItem(drawing);
		}
	}
	// actual drawings
	else {
		for (const VICUS::Drawing & d : prj.m_drawings) {
			QTreeWidgetItem * drawing = new QTreeWidgetItem(QStringList() << "Drawing", QTreeWidgetItem::Type);
			drawing->setToolTip(0, tr("Double click to edit offset and scaling."));
			m_ui->treeWidget->addTopLevelItem(drawing);
			// TODO should drawing have name & id
			QTreeWidgetItem * drawingItem = new QTreeWidgetItem(QStringList() << d.m_displayName, QTreeWidgetItem::Type);
			m_treeItemMap[d.m_id] = drawingItem;
			//m_treeItemMap[d.m_id] = drawingItem;
			drawing->addChild(drawingItem);
			drawingItem->setData(0, SVNavigationTreeItemDelegate::NodeID, d.m_id);
			drawingItem->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, d.m_visible);
			drawingItem->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, d.m_selected);

			// add child nodes for each edge in the network
			for (const VICUS::DrawingLayer & l : d.m_drawingLayers) {
				const QString &name = l.m_displayName;
				QTreeWidgetItem * ln = new QTreeWidgetItem(QStringList() << name, QTreeWidgetItem::Type);
				m_treeItemMap[l.m_id] = ln;
				// first fill with dummy data
				ln->setData(0, SVNavigationTreeItemDelegate::NodeID, l.m_id);
				ln->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, l.m_visible);
				ln->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, l.m_selected);
				drawingItem->addChild(ln);
			}
		}
	}

	// Dumb plain geometry
	if (!prj.m_plainGeometry.m_surfaces.empty()) {
		QTreeWidgetItem * plainGeo = new QTreeWidgetItem(QStringList() << tr("Obstacles/Shading Geometry"), QTreeWidgetItem::Type);
		m_treeItemMap[0] = plainGeo;
		plainGeo->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
		plainGeo->setData(0, SVNavigationTreeItemDelegate::NodeID, 0); // not a child node
		plainGeo->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, prj.m_plainGeometry.m_visible);
		plainGeo->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, prj.m_plainGeometry.m_selected);
		root->addChild(plainGeo);


		for (const VICUS::Surface & s : prj.m_plainGeometry.m_surfaces) {
			QTreeWidgetItem * surface = new QTreeWidgetItem(QStringList() << s.m_displayName, QTreeWidgetItem::Type);
			m_treeItemMap[s.m_id] = surface;
			plainGeo->addChild(surface);
			surface->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
			surface->setData(0, SVNavigationTreeItemDelegate::NodeID, s.m_id);
			surface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, prj.m_plainGeometry.m_visible && s.m_visible);
			surface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, prj.m_plainGeometry.m_selected && s.m_selected);
		}
	}

	m_ui->treeWidget->blockSignals(false);
	qDebug() << timer.elapsed() << "ms for navigation model reset.";

	m_ui->treeWidget->expandAll();
}


void SVNavigationTreeWidget::scrollToObject(unsigned int uniqueID) {
	auto objPtrIt = m_treeItemMap.find(uniqueID);
	Q_ASSERT(objPtrIt != m_treeItemMap.end());
	QTreeWidgetItem * item = objPtrIt->second;
	m_ui->treeWidget->expandItem(item->parent());
	m_ui->treeWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter);
	m_ui->treeWidget->setCurrentItem(item);
}


void SVNavigationTreeWidget::onStyleChanged() {
	m_navigationTreeItemDelegate->onStyleChanged();
	update();
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

void SVNavigationTreeWidget::on_actionSelectAll_triggered() {
	emit selectAll();
}


void SVNavigationTreeWidget::on_actionDeselectAll_triggered() {

	// This slot is triggered first - as top level action - when user presses Escape. However, depending on context,
	// we have different possible actions, for example, when editing geometry, Escape should cancel the current transformation.

	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (!vs.inPropertyEditingMode() &&
			vs.m_sceneOperationMode == SVViewState::OM_SelectedGeometry &&
			vs.m_propertyWidgetMode == SVViewState::PM_EditGeometry)
	{
		if (SVViewStateHandler::instance().m_propEditGeometryWidget->handleGlobalKeyPress(Qt::Key_Escape))
			return;
	}

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
	unsigned int ID = item->data(0, SVNavigationTreeItemDelegate::NodeID).toUInt();
	// lookup object in project data structure
	const VICUS::Object * o = project().objectById(ID);
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


void SVNavigationTreeWidget::on_actionInvertSelection_triggered() {
	// invert selection, i.e. select all surfaces currently not selected

	// we process all objects in the project() and store unique IDs of all objects that are currently not
	std::set<const VICUS::Object *> allObjects;
	project().selectObjects(allObjects, VICUS::Project::SG_All, false, true);
	std::set<unsigned int> selectedObjectIDs;
	std::set<unsigned int> deselectedObjectIDs;
	for (const VICUS::Object * o : allObjects) {
		if (o->m_selected)
			selectedObjectIDs.insert(o->m_id);
		else
			deselectedObjectIDs.insert(o->m_id);
	}

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selecting objects"), SVUndoTreeNodeState::SelectedState, deselectedObjectIDs, true);
	undo->push();
	undo = new SVUndoTreeNodeState(tr("De-selecting objects"), SVUndoTreeNodeState::SelectedState, selectedObjectIDs, false);
	undo->push();
}
