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
#include "SVPropZonePropertyDelegate.h"

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

SVPropZonePropertyDelegate::SVPropZonePropertyDelegate(QObject * parent) :
	QItemDelegate(parent)
{
}


SVPropZonePropertyDelegate::~SVPropZonePropertyDelegate() {
}


QWidget * SVPropZonePropertyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & /*option*/,
												  const QModelIndex & index ) const
{
	if (index.column() != 2 && index.column() != 3)
		return nullptr;

	QLineEdit * lineEdit = new QLineEdit(parent);

	return lineEdit;
}


void SVPropZonePropertyDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const {
	if (index.column() != 2 && index.column() != 3)
		return;


	QString input = index.model()->data(index, Qt::DisplayRole).toString();
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>(editor);
	lineEdit->setText(input);
}


void SVPropZonePropertyDelegate::setModelData (QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const {
	if (index.column() != 2 && index.column() != 3)
		return;
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>(editor);
	QString text = lineEdit->text();

	// is text a value
	bool ok;

	double d = text.toDouble(&ok);

	if(ok && d>0)
		model->setData(index, text, Qt::DisplayRole);

	// Warnung noch machen?
}


void SVPropZonePropertyDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	QItemDelegate::updateEditorGeometry(editor, option, index);
//	if (index.column() == 3) {
//		QtExt::FilterComboBox * combo = qobject_cast<QtExt::FilterComboBox *>(editor);
//		combo->showPopup();
//	}
}

