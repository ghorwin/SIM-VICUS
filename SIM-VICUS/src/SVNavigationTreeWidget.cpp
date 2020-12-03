#include "SVNavigationTreeWidget.h"
#include "ui_SVNavigationTreeWidget.h"

#include <QTreeWidgetItem>

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "SVNavigationTreeItemDelegate.h"
#include "SVUndoTreeNodeState.h"

SVNavigationTreeWidget::SVNavigationTreeWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVNavigationTreeWidget)
{
	m_ui->setupUi(this);

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVNavigationTreeWidget::onModified);

	SVStyle::formatWidgetWithLayout(this);

	// register item delegate that paints the "visible" bulb
	m_ui->treeWidget->setItemDelegate(new SVNavigationTreeItemDelegate(this));
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
		case SVProjectHandler::GeometryChanged :
			break;

		case SVProjectHandler::NodeStateModified : {
			// we only change data properties of existing nodes and emit itemChanged() signals, so
			// that the view updates its content

			// first decode the modification info object
			const SVUndoTreeNodeState::ModifiedNodes * info = dynamic_cast<SVUndoTreeNodeState::ModifiedNodes *>(data);
			Q_ASSERT(info != nullptr);

			// process all modified nodes
			for (unsigned int id : info->nodeIDs) {
				Q_ASSERT(m_treeItemMap.find(id) != m_treeItemMap.end());
				// find the object in question
				const VICUS::Object * obj = nullptr;
				for (const VICUS::Building & b : project().m_buildings) {
					obj = b.findChild(id);
					if (obj != nullptr)
						break;
				}
				Q_ASSERT(obj != nullptr);
				bool visible = true;
				bool selected = true;
				const VICUS::Building* b;
				const VICUS::BuildingLevel* bl;
				const VICUS::Room * r;
				const VICUS::Surface * s;
				if ((b = dynamic_cast<const VICUS::Building*>(obj)) != nullptr) {
					visible = b->m_visible;
					selected = b->m_selected;
				}
				if ((bl = dynamic_cast<const VICUS::BuildingLevel*>(obj)) != nullptr) {
					visible = bl->m_visible;
					selected = bl->m_selected;
				}
				if ((r = dynamic_cast<const VICUS::Room*>(obj)) != nullptr) {
					visible = r->m_visible;
					selected = r->m_selected;
				}
				if ((s = dynamic_cast<const VICUS::Surface*>(obj)) != nullptr) {
					visible = s->m_visible;
					selected = s->m_selected;
				}
				m_treeItemMap[id]->setData(0, SVNavigationTreeItemDelegate::VisibleFlag, visible);
				m_treeItemMap[id]->setData(0, SVNavigationTreeItemDelegate::SelectedFlag, selected);
			}
			m_ui->treeWidget->update();
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

	// add all childs
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

	for (const VICUS::Network & n : prj.m_networks) {
		QTreeWidgetItem * node = new QTreeWidgetItem(QStringList() << tr("Network: %1").arg(QString::fromStdString(n.m_name)), QTreeWidgetItem::Type);
		root->addChild(node);
		QTreeWidgetItem * enode = new QTreeWidgetItem(QStringList() << tr("Edges"), QTreeWidgetItem::Type);
		node->addChild(enode);
		// Add child nodes for each "clickable" entities
		QTreeWidgetItem * nnode = new QTreeWidgetItem(QStringList() << tr("Nodes"), QTreeWidgetItem::Type);
		node->addChild(nnode);
		// Add child nodes for each "clickable" entities
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
