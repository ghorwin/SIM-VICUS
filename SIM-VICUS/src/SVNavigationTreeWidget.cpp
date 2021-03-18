#include "SVNavigationTreeWidget.h"
#include "ui_SVNavigationTreeWidget.h"

#include <QTreeWidgetItem>

#include <VICUS_Project.h>
#include <VICUS_KeywordList.h>

#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "SVNavigationTreeItemDelegate.h"
#include "SVUndoTreeNodeState.h"
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

	// register item delegate that paints the "visible" bulb
	m_ui->treeWidget->setItemDelegate(new SVNavigationTreeItemDelegate(this));

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVNavigationTreeWidget::onModified);

	addAction(m_ui->actionDeselect_all);
}


SVNavigationTreeWidget::~SVNavigationTreeWidget() {
	delete m_ui;
}


void SVNavigationTreeWidget::setFlags(unsigned int uniqueID, bool visible, bool selected) {
	std::map<unsigned int, QTreeWidgetItem*>::iterator treeIt = m_treeItemMap.find(uniqueID);
	if (treeIt == m_treeItemMap.end()) {
		qDebug() << "Error, expected node with ID " << uniqueID << " in tree (tree corruption?)";
		return;
	}
	treeIt->second->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, visible);
	treeIt->second->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, selected);
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

		case SVProjectHandler::NodeStateModified : {
			// we only change data properties of existing nodes and emit itemChanged() signals, so
			// that the view updates its content

//			qDebug() << "Start processing NodeStateModified";

			// first decode the modification info object
			const SVUndoTreeNodeState::ModifiedNodes * info = dynamic_cast<SVUndoTreeNodeState::ModifiedNodes *>(data);
			Q_ASSERT(info != nullptr);

			std::set<unsigned int> modifiedIDs(info->m_nodeIDs.begin(), info->m_nodeIDs.end());

#if 1
			// process all objects in project and skip all, whose ID is not in our list
			for (const VICUS::Building & b : project().m_buildings) {
				if (modifiedIDs.find(b.uniqueID()) != modifiedIDs.end())
					setFlags(b.uniqueID(), b.m_visible, b.m_selected);
				for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
					if (modifiedIDs.find(bl.uniqueID()) != modifiedIDs.end())
						setFlags(bl.uniqueID(), bl.m_visible, bl.m_selected);
					for (const VICUS::Room & r : bl.m_rooms) {
						if (modifiedIDs.find(r.uniqueID()) != modifiedIDs.end())
							setFlags(r.uniqueID(), r.m_visible, r.m_selected);
						for (const VICUS::Surface & s : r.m_surfaces) {
							if (modifiedIDs.find(s.uniqueID()) != modifiedIDs.end())
								setFlags(s.uniqueID(), s.m_visible, s.m_selected);
						}
					}
				}
			}

			for (const VICUS::Surface & s : project().m_plainGeometry) {
				if (modifiedIDs.find(s.uniqueID()) != modifiedIDs.end())
					setFlags(s.uniqueID(), s.m_visible, s.m_selected);
			}

			for (const VICUS::Network & net : project().m_geometricNetworks) {
				if (modifiedIDs.find(net.uniqueID()) != modifiedIDs.end())
					setFlags(net.uniqueID(), net.m_visible, net.m_selected);
				for (const VICUS::NetworkNode & n : net.m_nodes) {
					if (modifiedIDs.find(n.uniqueID()) != modifiedIDs.end())
						setFlags(n.uniqueID(), n.m_visible, n.m_selected);
				}
				for (const VICUS::NetworkEdge & e : net.m_edges) {
					if (modifiedIDs.find(e.uniqueID()) != modifiedIDs.end())
						setFlags(e.uniqueID(), e.m_visible, e.m_selected);
				}
			}
#else
			// old, slow variant because of objectById()

			// process all modified nodes
			for (unsigned int id : info->m_nodeIDs) {
				std::map<unsigned int, QTreeWidgetItem*>::iterator treeIt = m_treeItemMap.find(id);
				Q_ASSERT(treeIt != m_treeItemMap.end());
				// find the object in question
				const VICUS::Object * obj = project().objectById(id);
				bool visible = true;
				bool selected = obj->m_selected;
				const VICUS::Building* b;
				const VICUS::BuildingLevel* bl;
				const VICUS::Room * r;
				const VICUS::Surface * s;
				const VICUS::Network * n;
				const VICUS::NetworkEdge * e;
				const VICUS::NetworkNode * no;
				if ((b = dynamic_cast<const VICUS::Building*>(obj)) != nullptr)
					visible = b->m_visible;
				if ((bl = dynamic_cast<const VICUS::BuildingLevel*>(obj)) != nullptr)
					visible = bl->m_visible;
				if ((r = dynamic_cast<const VICUS::Room*>(obj)) != nullptr)
					visible = r->m_visible;
				if ((s = dynamic_cast<const VICUS::Surface*>(obj)) != nullptr)
					visible = s->m_visible;
				if ((n = dynamic_cast<const VICUS::Network*>(obj)) != nullptr)
					visible = n->m_visible;
				if ((e = dynamic_cast<const VICUS::NetworkEdge*>(obj)) != nullptr)
					visible = e->m_visible;
				if ((no = dynamic_cast<const VICUS::NetworkNode*>(obj)) != nullptr)
					visible = no->m_visible;
				treeIt->second->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, visible);
				treeIt->second->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, selected);
			}
#endif
			m_ui->treeWidget->update();
//			qDebug() << "End processing NodeStateModified";
			return; // nothing else to do here
		}

		default:
			return; // do nothing by default
	}

	// for now, rebuild the entire tree
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
		building->setData(0, SVNavigationTreeItemDelegate::NodeID, b.uniqueID());
		building->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, b.m_visible);
		building->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, b.m_selected);
		root->addChild(building);
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			QTreeWidgetItem * buildingLevel = new QTreeWidgetItem(QStringList() << bl.m_displayName, QTreeWidgetItem::Type);
			m_treeItemMap[bl.uniqueID()] = buildingLevel;
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::NodeID, bl.uniqueID());
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, bl.m_visible);
			buildingLevel->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, bl.m_selected);
			building->addChild(buildingLevel);
			for (const VICUS::Room & r : bl.m_rooms) {
				QTreeWidgetItem * rooms = new QTreeWidgetItem(QStringList() << r.m_displayName, QTreeWidgetItem::Type);
				m_treeItemMap[r.uniqueID()] = rooms;
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
					surface->setData(0, SVNavigationTreeItemDelegate::NodeID, s.uniqueID());
					surface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, s.m_visible);
					surface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, s.m_selected);
				}
			}
		}
	}

	const SVDatabase  & db = SVSettings::instance().m_db;

	// Networks
	for (const VICUS::Network & n : prj.m_geometricNetworks) {
		QTreeWidgetItem * networkItem = new QTreeWidgetItem(QStringList() << tr("Network: %1").arg(QString::fromStdString(n.m_name)), QTreeWidgetItem::Type);
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
			const VICUS::NetworkPipe * pipe = db.m_pipes[e.m_pipeId];
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
	for (const VICUS::Surface & s : prj.m_plainGeometry) {
		QTreeWidgetItem * surface = new QTreeWidgetItem(QStringList() << s.m_displayName, QTreeWidgetItem::Type);
		m_treeItemMap[s.uniqueID()] = surface;
		root->addChild(surface);
		surface->setData(0, SVNavigationTreeItemDelegate::NodeID, s.uniqueID());
		surface->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, s.m_visible);
		surface->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, s.m_selected);
	}

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
	m_smartSelectDialog->exec(); // selection undo actions are created in the dialog, nothing else to do here
}
