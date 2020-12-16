#include "SVNavigationTreeWidget.h"
#include "ui_SVNavigationTreeWidget.h"

#include <QTreeWidgetItem>

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "SVNavigationTreeItemDelegate.h"
#include "SVUndoTreeNodeState.h"
#include "SVViewStateHandler.h"

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

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVNavigationTreeWidget::onViewStateChanged);
}


SVNavigationTreeWidget::~SVNavigationTreeWidget() {
	delete m_ui;
}


unsigned int SVNavigationTreeWidget::selectedNodeID() const {
	if (m_ui->treeWidget->selectedItems().isEmpty())
		return 0;
	QTreeWidgetItem * item = m_ui->treeWidget->selectedItems().first();
	return item->data(0, SVNavigationTreeItemDelegate::NodeID).toUInt();
}


void SVNavigationTreeWidget::onModified(int modificationType, ModificationInfo * data) {

	// filter out all modification types that we handle
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		case SVProjectHandler::AllModified :
		case SVProjectHandler::NetworkModified :
		case SVProjectHandler::GeometryChanged :
			/// \todo parse 'data' to determine what has changed and avoid updating entire tree (and losing collapsed state)
			break;

		case SVProjectHandler::NodeStateModified : {
			// we only change data properties of existing nodes and emit itemChanged() signals, so
			// that the view updates its content

			// first decode the modification info object
			const SVUndoTreeNodeState::ModifiedNodes * info = dynamic_cast<SVUndoTreeNodeState::ModifiedNodes *>(data);
			Q_ASSERT(info != nullptr);

			// process all modified nodes
			for (unsigned int id : info->m_nodeIDs) {
				Q_ASSERT(m_treeItemMap.find(id) != m_treeItemMap.end());
				// find the object in question
				const VICUS::Object * obj = project().objectById(id);
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
	root->setData(0, SVNavigationTreeItemDelegate::ItemType, NT_Site);
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

	// Networks
	for (const VICUS::Network & n : prj.m_geometricNetworks) {
		QTreeWidgetItem * networkItem = new QTreeWidgetItem(QStringList() << tr("Network: %1").arg(QString::fromStdString(n.m_name)), QTreeWidgetItem::Type);
		root->addChild(networkItem);
		networkItem->setData(0, SVNavigationTreeItemDelegate::ItemType, NT_Network);
		networkItem->setData(0, SVNavigationTreeItemDelegate::NodeID, n.uniqueID());
		QTreeWidgetItem * enode = new QTreeWidgetItem(QStringList() << tr("Edges"), QTreeWidgetItem::Type);
		enode->setFlags(Qt::ItemIsEnabled); // cannot select "Edges"
		networkItem->addChild(enode);
		// add child nodes for each edge in the network
		/// TODO : Hauke, think about grouping for larger networks
		for (const VICUS::NetworkEdge & e : n.m_edges) {
			QTreeWidgetItem * en = new QTreeWidgetItem(QStringList() << QString("E [%1]").arg(e.uniqueID()), QTreeWidgetItem::Type);
			en->setData(0, SVNavigationTreeItemDelegate::ItemType, NT_NetworkEdge);
			en->setData(0, SVNavigationTreeItemDelegate::NodeID, e.uniqueID());
			enode->addChild(en);
		}

		QTreeWidgetItem * nnode = new QTreeWidgetItem(QStringList() << tr("Nodes"), QTreeWidgetItem::Type);
		nnode->setFlags(Qt::ItemIsEnabled); // cannot select "Nodes"
		networkItem->addChild(nnode);
		// add child nodes for each edge in the network
		/// TODO : Hauke, think about grouping for larger networks
		for (const VICUS::NetworkNode & nod : n.m_nodes) {
			QTreeWidgetItem * no = new QTreeWidgetItem(QStringList() << QString("N [%1]").arg(nod.uniqueID()), QTreeWidgetItem::Type);
			no->setData(0, SVNavigationTreeItemDelegate::ItemType, NT_NetworkNode);
			no->setData(0, SVNavigationTreeItemDelegate::NodeID, nod.uniqueID());
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

	// update the item flags regarding selecting the item
	onViewStateChanged();
}


void SVNavigationTreeWidget::onViewStateChanged() {
	// depending on view state, items can be selected or not
	const SVViewState & vs = SVViewStateHandler::instance().viewState();

	if (vs.m_viewMode == SVViewState::VM_GeometryEditMode)
		m_ui->treeWidget->setSelectionMode(QTreeView::NoSelection);
	else
		m_ui->treeWidget->setSelectionMode(QTreeView::SingleSelection);
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


void SVNavigationTreeWidget::on_treeWidget_itemSelectionChanged() {
	// when user changes the selection, show the appropriate property widget
	if (m_ui->treeWidget->selectedItems().isEmpty())
		return;
	QTreeWidgetItem * item = m_ui->treeWidget->selectedItems().first();

	// we now need to figure out, what kind of icon this is

	NodeType nt = static_cast<NodeType>(item->data(0, SVNavigationTreeItemDelegate::ItemType).toInt());
	SVViewState vs = SVViewStateHandler::instance().viewState();
	switch (nt) {
		case NT_Site : {
			// switch view state to show property widget for side
			vs.m_propertyWidgetMode = SVViewState::PM_SiteProperties;
		} break;
		case NT_Network :{
			vs.m_propertyWidgetMode = SVViewState::PM_EditNetwork;
		} break;
		case NT_NetworkEdge:{
			vs.m_propertyWidgetMode = SVViewState::PM_EditNetworkEdge;
		} break;
		case NT_NetworkNode:{
			vs.m_propertyWidgetMode = SVViewState::PM_EditNetworkNode;
		} break;

		case NT_Network : {
			// switch view state to show property widget for side
			SVViewState vs = SVViewStateHandler::instance().viewState();
			vs.m_propertyWidgetMode = SVViewState::PM_NetworkProperties;
			SVViewStateHandler::instance().setViewState(vs);
		} break;

		/// TODO : show other property widgets
	}
	SVViewStateHandler::instance().setViewState(vs);
}
