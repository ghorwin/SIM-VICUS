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

#include <QtExt_Conversions.h>

#include "SVStyle.h"
#include "SVConstants.h"
#include "SVMainWindow.h"
#include "SVDatabaseEditDialog.h"

SVPropSurfaceHeatingDelegate::SVPropSurfaceHeatingDelegate(QObject * parent) :
	QItemDelegate(parent)
{
}

SVPropSurfaceHeatingDelegate::~SVPropSurfaceHeatingDelegate() {
}


QWidget * SVPropSurfaceHeatingDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option,
												  const QModelIndex & index ) const
{
	return nullptr;
}

#if 0
	// for all columns not handled in the switch the default editor is used.
	switch (index.column()) {
		case 2 : // simply show database dialog for
			return SVMainWindow::instance().dbSurfaceHeatingSystemEditDialog();
		// Construction types
		case ConstructionTableModel::CONSTRUCTION_TYPE :
		{
			ConstructionSelectionWidget* constructionSelector = new ConstructionSelectionWidget(parent, m_db);
			return constructionSelector;
		}

		case ConstructionTableModel::CONSTRUCTION_USAGE :
		{
			QComboBox * combo = new QComboBox(parent);
			combo->addItem(tr("Outside construction"),0);
			combo->addItem(tr("Inside construction"),1);
			combo->addItem(tr("Construction to fixed-temperature zone"),2);
			return combo;
		}

		// area inputs
		case ConstructionTableModel::TOTAL_AREA :
		case ConstructionTableModel::WINDOW_AREA :
		case ConstructionTableModel::ABSORPTION_COEFF :
		case ConstructionTableModel::RUE :
		case ConstructionTableModel::RUI :
		{
			QWidget * editor = QItemDelegate::createEditor(parent, option, index);
			QDoubleSpinBox * spin = dynamic_cast<QDoubleSpinBox *>(editor);
			Q_ASSERT(spin != nullptr);
			spin->setAutoFillBackground(true);
			spin->setMinimum(0);
			spin->setAlignment(Qt::AlignRight  | Qt::AlignVCenter);
			if (index.column() == ConstructionTableModel::RUE ||
				index.column() == ConstructionTableModel::RUI)
			{
				spin->setDecimals(3);
				spin->setMinimum(0.001);
			}
			return editor;
		}

		case ConstructionTableModel::OTHER_TEMP : {
			QWidget * editor = QItemDelegate::createEditor(parent, option, index);
			QLineEdit * lineEdit = dynamic_cast<QLineEdit *>(editor);
			Q_ASSERT(lineEdit != nullptr);
			lineEdit->setAutoFillBackground(true);
			return editor;
		}

		case ConstructionTableModel::ORIENTATION :
		{
			QComboBox * combo = new QComboBox(parent);
			combo->setEditable(true);
			// fill combo box with presets
			combo->addItem("0", 0);
			combo->addItem("90", 90);
			combo->addItem("180", 180);
			combo->addItem("270", 270);
			combo->setAutoFillBackground(true);
			return combo;
		}

		// Inclination
		case ConstructionTableModel::INCLINATION :
		{
			QComboBox * combo = new QComboBox(parent);
			combo->setEditable(true);
			// fill combo box with presets
			combo->addItem("0", 0);
			combo->addItem("45", 45);
			combo->addItem("90", 90);
			combo->setAutoFillBackground(true);
			return combo;
		}

		// Window types
		case ConstructionTableModel::WINDOW_TYPE :
		{
			WindowSelectionWidget* windowSelector = new WindowSelectionWidget(parent, m_db);
			return windowSelector;
		}

		// Shading types
		case ConstructionTableModel::SHADING_TYPE :
		{
			QComboBox * combo = new QComboBox(parent);
			combo->addItem(TH::Database::noShadingText(), 0);
			for (std::map<unsigned int, TH::ShadingType>::const_iterator it = m_db->shadingTypes().begin();
				it != m_db->shadingTypes().end(); ++it)
			{
				if (it->second.isValid())
					combo->addItem(QIcon(":/gfx/ok_16x16.png"), tr("%1 [%2]").arg(it->second.name()).arg(it->first), it->first);
				else
					combo->addItem(QIcon(":/gfx/16x16/error_16x16.png"), tr("%1 [%2]").arg(it->second.name()).arg(it->first), it->first);
			}
			return combo;
		}

		default : ;
	}
	return QItemDelegate::createEditor(parent, option, index);
}

void SVPropSurfaceHeatingDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const {
	// here we determine the editor type, extract the value entered by the user and
	// set it in the model
	QVariant value = index.model()->data(index, Qt::EditRole);
	switch (index.column()) {
		// the type edits
		case ConstructionTableModel::CONSTRUCTION_TYPE : {
			ConstructionSelectionWidget* constructionSelector = dynamic_cast<ConstructionSelectionWidget*>(editor);
			Q_ASSERT(constructionSelector != nullptr);
			unsigned int id = value.toUInt();
			constructionSelector->selectConstructionByID(id);
			break;
		}
		case ConstructionTableModel::WINDOW_TYPE : {
			WindowSelectionWidget* windowSelector = dynamic_cast<WindowSelectionWidget*>(editor);
			Q_ASSERT(windowSelector != nullptr);
			unsigned int id = value.toUInt();
			windowSelector->selectWindowByID(id);
			break;
		}
		case ConstructionTableModel::CONSTRUCTION_USAGE :
		case ConstructionTableModel::SHADING_TYPE :
		{
			QComboBox * combo = dynamic_cast<QComboBox*>(editor);
			Q_ASSERT(combo != nullptr);
			unsigned int id = value.toUInt();
			int idx = combo->findData(id);
			if (idx == -1)
				idx = 0;
			combo->setCurrentIndex(idx);
		} break;

		case ConstructionTableModel::ORIENTATION :
		{
			QComboBox * combo = dynamic_cast<QComboBox *>(editor);
			Q_ASSERT(combo != nullptr);
			int orientation = value.toInt();
			if (orientation == 0) combo->setCurrentIndex(0);
			else if (orientation == 90) combo->setCurrentIndex(1);
			else if (orientation == 180) combo->setCurrentIndex(2);
			else if (orientation == 270) combo->setCurrentIndex(3);
			else
				combo->setEditText(QString::number(value.toDouble()));
			combo->lineEdit()->selectAll();
		} break;

		case ConstructionTableModel::INCLINATION :
		{
			QComboBox * combo = dynamic_cast<QComboBox *>(editor);
			Q_ASSERT(combo != nullptr);
			int incl = value.toInt();
			if (incl == 0) combo->setCurrentIndex(0);
			else if (incl == 45) combo->setCurrentIndex(1);
			else if (incl == 90) combo->setCurrentIndex(2);
			else
				combo->setEditText(QString::number(value.toDouble()));
			combo->lineEdit()->selectAll();
		} break;

		case ConstructionTableModel::TOTAL_AREA :
		case ConstructionTableModel::WINDOW_AREA :
		case ConstructionTableModel::ABSORPTION_COEFF :
		case ConstructionTableModel::RUE :
		case ConstructionTableModel::RUI :
		{
			QDoubleSpinBox * spin = dynamic_cast<QDoubleSpinBox *>(editor);
			Q_ASSERT(spin != nullptr);
			spin->selectAll();
		} break;

		case ConstructionTableModel::OTHER_TEMP :
		{
		} break;
	}
	// Re-use default implementation
	QItemDelegate::setEditorData(editor, index);
}

void SVPropSurfaceHeatingDelegate::setModelData (QWidget * editor, QAbstractItemModel * model,
											  const QModelIndex & index ) const
{
	//const char* FUNC_ID = "SVPropSurfaceHeatingDelegate::setModelData()";

	//	IBK::IBK_Message( IBK::FormatString("SVPropSurfaceHeatingDelegate::setModelData() index: '%1'.\n").arg(index),
	//		IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_STANDARD);
	// here we determine the editor type, extract the value entered by the user and
	// set it in the model
	switch (index.column()) {
		case ConstructionTableModel::CONSTRUCTION_TYPE : {
			ConstructionSelectionWidget* constructionSelector = dynamic_cast<ConstructionSelectionWidget*>(editor);
			Q_ASSERT(constructionSelector != nullptr);
			int id = constructionSelector->selectedConstructionID();
			if(id >= 0)
				model->setData(index, id);
			else if (id == -2) {
				model->setData(index, 0);
			}
			return;
		}
		case ConstructionTableModel::WINDOW_TYPE : {
			WindowSelectionWidget* windowSelector = dynamic_cast<WindowSelectionWidget*>(editor);
			Q_ASSERT(windowSelector != nullptr);
			int id = windowSelector->selectedWindowID();
			if(id >= 0)
				model->setData(index, id);
			else if (id == -2) {
				model->setData(index, 0);
			}
			return;
		}
		case ConstructionTableModel::CONSTRUCTION_USAGE :
		case ConstructionTableModel::SHADING_TYPE :
		{
			QComboBox * combo = dynamic_cast<QComboBox *>(editor);
			Q_ASSERT(combo != nullptr);
			int idx = combo->currentIndex();
			model->setData(index, combo->itemData(idx).toUInt());
			return;
		}

		case ConstructionTableModel::ORIENTATION :
		{
			QComboBox * combo = dynamic_cast<QComboBox *>(editor);
			Q_ASSERT(combo != nullptr);
			int i = combo->currentIndex();
			if (i == 4) {
				// manually entered
				bool ok;
				double orientation = combo->currentText().toDouble(&ok);
				if (!ok) return;
				model->setData(index, orientation);
			}
			else {
				model->setData(index, combo->itemData(i));
			}
			return;
		}

		case ConstructionTableModel::INCLINATION :
		{
			QComboBox * combo = dynamic_cast<QComboBox *>(editor);
			Q_ASSERT(combo != nullptr);
			int i = combo->currentIndex();
			if (i == 3) {
				// manually entered
				bool ok;
				double incl = combo->currentText().toDouble(&ok);
				if (!ok) return;
				model->setData(index, incl);
			}
			else {
				model->setData(index, combo->itemData(i));
			}
			return;
		}
	}
	// for standard editors we simply use the default implementation
	QItemDelegate::setModelData(editor, model, index);
}

void SVPropSurfaceHeatingDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	switch (index.column()) {
		case ConstructionTableModel::CONSTRUCTION_TYPE : {
			CalculationInputWidget* parentwidget = dynamic_cast<CalculationInputWidget*>(this->parent());
			if(parentwidget != nullptr) {
				QRect rect = parentwidget->tableViewRect();
				int width = (rect.width() - option.rect.left()) * 0.7;
				int height = rect.height() - option.rect.top();
				editor->setGeometry(option.rect.left(),option.rect.top(),width,height);
			}
			else {
				int width = 1200;
				int height = 600;
				editor->setGeometry(option.rect.left(),option.rect.top(),width,height);
			}
			break;
		}
		case ConstructionTableModel::WINDOW_TYPE : {
			CalculationInputWidget* parentwidget = dynamic_cast<CalculationInputWidget*>(this->parent());
			if(parentwidget != nullptr) {
				QRect rect = parentwidget->tableViewRect();
				int left = rect.left() + rect.width() * 0.1;
				int width = (rect.width() - left) * 0.9;
				int height = rect.height() - option.rect.top();
				editor->setGeometry(left,option.rect.top(),width,height);
			}
			else {
				int width = 1000;
				int height = 600;
				editor->setGeometry(option.rect.left(),option.rect.top(),width,height);
			}
			break;
		}
		default: QItemDelegate::updateEditorGeometry(editor, option, index); break;
	}

}

#endif

void SVPropSurfaceHeatingDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	QItemDelegate::paint(painter, option, index);
	if (index.column() == 2 || index.column() == 3) {
		QStyleOptionButton button;
		QRect r = option.rect;//getting the rect of the cell
		r.setLeft(r.left() + r.width() - r.height());
		button.rect = r;
		button.text = "...";
		button.state = QStyle::State_Enabled;

		QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
	}
}

bool SVPropSurfaceHeatingDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
	if (index.column() == 2 || index.column() == 3) {
		if( event->type() == QEvent::MouseButtonRelease ) 	{
			QMouseEvent * e = (QMouseEvent *)event;
			int clickX = e->x();
			int clickY = e->y();

			QRect r = option.rect;//getting the rect of the cell
			r.setLeft(r.left() + r.width() - r.height());

			if (r.contains(clickX,clickY)) {
				unsigned int selectedID = index.data(Qt::UserRole).toUInt();
				if (index.column() == 2) {
					selectedID = SVMainWindow::instance().dbSurfaceHeatingSystemEditDialog()->select(selectedID);
					if (selectedID != VICUS::INVALID_ID) {
						model->setData(index, selectedID, Qt::UserRole); // this will trigger an undo event for changing a component instance
						return true;
					}
				}
				else {
					// TODO : Implement zone selection dialog
				}
			}
			else
				return false;
		}
	}

	return false;
}
