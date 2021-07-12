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

#ifndef SVPropSurfaceHeatingDelegateH
#define SVPropSurfaceHeatingDelegateH

#include <QItemDelegate>

class QTableWidget;

/*!	Creates specialized editors for heating system and control zone inputs. */
class SVPropSurfaceHeatingDelegate : public QItemDelegate {
	Q_OBJECT
public:
	/*! Default constructor. */
	SVPropSurfaceHeatingDelegate(QObject * parent);
	/*! Default destructor. */
	~SVPropSurfaceHeatingDelegate() override;

	// QAbstractItemDelegate interface

	QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
	void setEditorData(QWidget * editor, const QModelIndex & index) const override;
	void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
	void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
	bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index) override;

	QTableWidget * m_view;

};




#endif // SVPropSurfaceHeatingDelegateH
