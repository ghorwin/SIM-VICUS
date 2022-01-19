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

#include "SVPropSurfaceHeatingDelegate.h"

#include <QPainter>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QMouseEvent>
#include <QApplication>
#include <QTableWidget>
#include <QLineEdit>

#include <SV_Conversions.h>
#include <QtExt_FilterComboBox.h>

#include "SVStyle.h"
#include "SVConstants.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

SVPropSurfaceHeatingDelegate::SVPropSurfaceHeatingDelegate(QObject * parent) :
	QItemDelegate(parent),
	m_view(qobject_cast<QTableWidget*>(parent))
{
}


SVPropSurfaceHeatingDelegate::~SVPropSurfaceHeatingDelegate() {
}


QWidget * SVPropSurfaceHeatingDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & /*option*/,
												  const QModelIndex & index ) const
{
	if (index.column() != 3)
		return nullptr;

	QtExt::FilterComboBox * combo = new QtExt::FilterComboBox(parent);
	combo->setFont(m_view->font());
	combo->lineEdit()->setFont(m_view->font());
	// populate with zones
	for (const VICUS::Building & b : project().m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room & r : bl.m_rooms) {
				combo->addItem(r.m_displayName, r.m_id);
			}
//	for (int i=0; i<combo->count(); ++i)
//		qDebug() << combo->itemData(i).toUInt();

	combo->showPopup();
	return combo;
}


void SVPropSurfaceHeatingDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const {
	if (index.column() != 3)
		return;
	unsigned int zoneID = index.model()->data(index, Qt::UserRole).toUInt();
	QtExt::FilterComboBox * combo = qobject_cast<QtExt::FilterComboBox *>(editor);
	int idx = combo->findData(zoneID, Qt::UserRole);
	for (int i=0; i<combo->count(); ++i)
		if (combo->itemData(i).toUInt() == zoneID) {
			idx = i;
			break;
		}

	combo->setCurrentIndex(idx);
}


void SVPropSurfaceHeatingDelegate::setModelData (QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const {
	if (index.column() != 3)
		return;
	QtExt::FilterComboBox * combo = qobject_cast<QtExt::FilterComboBox *>(editor);
	unsigned int id = combo->currentData().toUInt();
	model->setData(index, id, Qt::UserRole);
	combo->releaseKeyboard();
}


void SVPropSurfaceHeatingDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	QItemDelegate::updateEditorGeometry(editor, option, index);
//	if (index.column() == 3) {
//		QtExt::FilterComboBox * combo = qobject_cast<QtExt::FilterComboBox *>(editor);
//		combo->showPopup();
//	}
}


void SVPropSurfaceHeatingDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	QItemDelegate::paint(painter, option, index);
	if (index.column() == 2) {
		QStyleOptionButton button;
		QRect r = option.rect;//getting the rect of the cell
		r.setLeft(r.left() + r.width() - r.height());
		button.rect = r;
		button.text = "...";
		button.state = QStyle::State_Enabled;

		QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
	}
	else if (index.column() == 3) {

	}
}

bool SVPropSurfaceHeatingDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
	if (index.column() == 2 && event->type() == QEvent::MouseButtonRelease ) 	{
		QMouseEvent * e = (QMouseEvent *)event;
		int clickX = e->x();
		int clickY = e->y();

		QRect r = option.rect;//getting the rect of the cell
		r.setLeft(r.left() + r.width() - r.height());

		if (r.contains(clickX,clickY)) {
			unsigned int selectedID = index.data(Qt::UserRole).toUInt();
			selectedID = SVMainWindow::instance().dbSurfaceHeatingSystemEditDialog()->select(selectedID);
			if (selectedID != VICUS::INVALID_ID) {
				model->setData(index, selectedID, Qt::UserRole); // this will trigger an undo event for changing a component instance
				return true;
			}
		}
		else
			return false;
	}
	if (index.column() == 3 && event->type() == QEvent::MouseButtonDblClick) {
		m_view->openPersistentEditor(m_view->item(index.row(), index.column()));
	}

	return false;
}

