#ifndef SVNavigationTreeWidgetH
#define SVNavigationTreeWidgetH

#include <QWidget>

namespace Ui {
class SVNavigationTreeWidget;
}

class ModificationInfo;
class QTreeWidgetItem;

/*! The tree widget with all building/rooms/etc.. */
class SVNavigationTreeWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVNavigationTreeWidget(QWidget *parent);
	~SVNavigationTreeWidget();

	/*! Returns the unique ID of a selected node, or 0, if none is selected. */
	unsigned int selectedNodeID() const;

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

	/*! Connected to view-state handler. */
	void onViewStateChanged();

private slots:
	void on_treeWidget_itemCollapsed(QTreeWidgetItem *item);

	void on_treeWidget_itemSelectionChanged();

	void on_actionRemoveSelected_triggered();

private:
	enum NodeType {
		NT_UndefinedNode,
		NT_Site,
		NT_Building,
		NT_BuildingLevel,
		NT_Room,
		NT_Surface,
		NT_Network,
		NT_NetworkNode,
		NT_NetworkEdge
	};

	/*! Recursively collapses all children. */
	void collapseTreeWidgetItem(QTreeWidgetItem * parent);

	/*! Relates unique object ID to tree widget item.
		This map is updated whenever the tree widget is modified in onModified().
	*/
	std::map<unsigned int, QTreeWidgetItem*>	m_treeItemMap;

	/*! Ui pointer. */
	Ui::SVNavigationTreeWidget			*m_ui;

};

#endif // SVNavigationTreeWidgetH

