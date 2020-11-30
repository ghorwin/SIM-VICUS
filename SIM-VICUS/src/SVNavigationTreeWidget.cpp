#include "SVNavigationTreeWidget.h"
#include "ui_SVNavigationTreeWidget.h"

#include <QTreeWidgetItem>

#include <VICUS_Project.h>

#include "SVProjectHandler.h"
#include "SVStyle.h"
#include "SVNavigationTreeItemDelegate.h"

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
			break;

		default:
			return; // do nothing by default
	}

	// for now, rebuild the entire tree
	m_ui->treeWidget->clear();
	// populate tree widget

	// insert root node
	QTreeWidgetItem * root = new QTreeWidgetItem(QStringList() << "Site", QTreeWidgetItem::Type);
	m_ui->treeWidget->addTopLevelItem(root);

	// get project data
	const VICUS::Project & prj = project();

	// add all childs
	for (const VICUS::Building & b : prj.m_buildings) {
		QTreeWidgetItem * building = new QTreeWidgetItem(QStringList() << tr("Building: %1").arg(b.m_displayName), QTreeWidgetItem::Type);
		building->setData(0, Qt::UserRole, b.m_id);
		root->addChild(building);
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			QTreeWidgetItem * buildingLevel = new QTreeWidgetItem(QStringList() << bl.m_displayName, QTreeWidgetItem::Type);
			building->addChild(buildingLevel);
			for (const VICUS::Room & r : bl.m_rooms) {
				QTreeWidgetItem * rooms = new QTreeWidgetItem(QStringList() << r.m_displayName, QTreeWidgetItem::Type);
				if (rooms->text(0).isEmpty())
					rooms->setText(0, tr("unnamed"));
				buildingLevel->addChild(rooms);
				for (const VICUS::Surface & s : r.m_surfaces) {
					QTreeWidgetItem * surface = new QTreeWidgetItem(QStringList() << s.m_displayName, QTreeWidgetItem::Type);
					rooms->addChild(surface);
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
