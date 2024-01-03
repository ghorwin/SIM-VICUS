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

#ifndef SVNavigationTreeItemDelegateH
#define SVNavigationTreeItemDelegateH

#include <QStyledItemDelegate>

/*! The item delegate that displays the object state in the navigation tree widget. */
class SVNavigationTreeItemDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	enum DataRoles {
		NodeID = Qt::UserRole,
		VisibleFlag = Qt::UserRole + 1,
		SelectedFlag = Qt::UserRole + 2,
		ItemType = Qt::UserRole + 3,
		InvalidGeometryFlag = Qt::UserRole + 4,
		MissingDrawingFile = Qt::UserRole + 5
	};

	enum TopologyType {
		TT_Building,
		TT_BuildingLevel,
		TT_Room,
		TT_Surface,
		TT_Subsurface,
		NUM_TT
	};

	SVNavigationTreeItemDelegate(QWidget * parent = nullptr);

	// QAbstractItemDelegate interface

	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
	bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index) override;
	void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const override;

private:
	QImage m_lightBulbOn;
	QImage m_lightBulbOff;
	QImage m_selectedOn;
	QImage m_selectedOff;

};




#endif // SVNavigationTreeItemDelegateH
