#ifndef SVNavigationTreeWidgetH
#define SVNavigationTreeWidgetH

#include <QWidget>

namespace Ui {
class SVNavigationTreeWidget;
}

class ModificationInfo;
class QTreeWidgetItem;
class SVSmartSelectDialog;

/*! The tree widget with all building/rooms/etc.. */
class SVNavigationTreeWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVNavigationTreeWidget(QWidget *parent);
	~SVNavigationTreeWidget();

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

signals:
	void removeSelected();
	void hideSelected();
	void showSelected();
	void selectAll();
	void deselectAll();

private slots:
	void on_treeWidget_itemCollapsed(QTreeWidgetItem *item);

	void on_actionRemoveSelected_triggered();

	void on_actionShowSelected_triggered();

	void on_actionHideSelected_triggered();

	void on_actionSelect_all_triggered();

	void on_actionDeselect_all_triggered();

	void on_actionSmartSelect_triggered();

private:
	/*! Recursively collapses all children. */
	void collapseTreeWidgetItem(QTreeWidgetItem * parent);

	/*! Modifies a tree node. */
	void setFlags(unsigned int uniqueID, bool visible, bool selected);

	/*! Relates unique object ID to tree widget item.
		This map is updated whenever the tree widget is modified in onModified().
	*/
	std::map<unsigned int, QTreeWidgetItem*>	m_treeItemMap;

	/*! Ui pointer. */
	Ui::SVNavigationTreeWidget			*m_ui;

	/*! The smart selection dialog. */
	SVSmartSelectDialog					*m_smartSelectDialog = nullptr;
};

#endif // SVNavigationTreeWidgetH

