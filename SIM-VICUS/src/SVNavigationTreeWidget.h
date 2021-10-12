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

#ifndef SVNavigationTreeWidgetH
#define SVNavigationTreeWidgetH

#include <QWidget>

namespace Ui {
class SVNavigationTreeWidget;
}

class ModificationInfo;
class QTreeWidgetItem;
class SVSmartSelectDialog;
class SVNavigationTreeItemDelegate;

/*! The tree widget with all building/rooms/etc.. */
class SVNavigationTreeWidget : public QWidget {
	Q_OBJECT

public:
	explicit SVNavigationTreeWidget(QWidget *parent);
	~SVNavigationTreeWidget();

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data);

	/*! Expand node if not yet done and scroll to view. */
	void scrollToObject(unsigned int uniqueID);

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

	void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

	void on_actionInvertSelection_triggered();

private:
	/*! Recursively collapses all children. */
	void collapseTreeWidgetItem(QTreeWidgetItem * parent);

	/*! Relates unique object ID to tree widget item.
		This map is updated whenever the tree data is modified entirely in onModified().
	*/
	std::map<unsigned int, QTreeWidgetItem*>	m_treeItemMap;

	/*! Ui pointer. */
	Ui::SVNavigationTreeWidget					*m_ui;

	/*! The smart selection dialog. */
	SVSmartSelectDialog							*m_smartSelectDialog = nullptr;

	/*! The item delegate that renders the tree items. */
	SVNavigationTreeItemDelegate				*m_navigationTreeItemDelegate = nullptr;
};

#endif // SVNavigationTreeWidgetH
