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

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

private slots:
	void on_treeWidget_itemCollapsed(QTreeWidgetItem *item);

private:
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

