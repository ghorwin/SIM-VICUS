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

#include "SVNavigationTreeItemDelegate.h"

#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QTreeView>
#include <QFileDialog>

#include "SVUndoModifySurfaceGeometry.h"
#include "SVUndoTreeNodeState.h"
#include "SVSettings.h"
#include "SVProjectHandler.h"
#include "SVDrawingPropertiesDialog.h"
#include "SVMainWindow.h"
#include "SVUndoModifyDrawingFile.h"


SVNavigationTreeItemDelegate::SVNavigationTreeItemDelegate(QWidget * parent) :
	QStyledItemDelegate(parent)
{
	onStyleChanged();
}

void SVNavigationTreeItemDelegate::onStyleChanged() {
	QIcon bulbOn = QIcon::fromTheme("bulb_on");
	m_lightBulbOn = QPixmap(bulbOn.pixmap(256));
	QIcon bulbOff = QIcon::fromTheme("bulb_off");
	m_lightBulbOff = QPixmap(bulbOff.pixmap(256));
	QIcon checked = QIcon::fromTheme("checkbox_checked");
	m_selectedOn = QPixmap(checked.pixmap(256));
	QIcon unchecked = QIcon::fromTheme("checkbox_unchecked");
	m_selectedOff = QPixmap(unchecked.pixmap(256));
}


void SVNavigationTreeItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
	// get rectangle of area to paint into
	QRect targetRect(option.rect);

	// Note: the first part of the widget (the branch indicator) is not drawn by us, but rather by the tree view itself
	//       to change that, we would need to derive from QTreeView and overload/re-implement drawRow().
	//       --> later

	// root items or items without object Id are never current or selected
	if (index.parent() == QModelIndex() /*! || uniqueObjectId == 0*/) {
		// check if item is selected/current
		bool isSelected = option.state & QStyle::State_Selected;
		QFont f = painter->font();
		f.setBold(isSelected);
		painter->setFont(f);
		painter->drawText(targetRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
		return;
	}

	// if item is current, draw the background
	const QTreeView * treeView = qobject_cast<const QTreeView *>(parent());
	Q_ASSERT(treeView != nullptr);
	bool isCurrent = (index == treeView->currentIndex());
	if (isCurrent) {
		painter->fillRect(targetRect, QColor(33, 174, 191));
	}

	// find out if the element we are painting is visible or not
	bool visible = index.data(VisibleFlag).toBool();

	const QPixmap * bulbImg = nullptr;
	if (visible)
		bulbImg = &m_lightBulbOn;
	else
		bulbImg = &m_lightBulbOff;

	painter->setRenderHint(QPainter::Antialiasing);

	const_cast<QPixmap*>(bulbImg)->setDevicePixelRatio(SVSettings::instance().m_ratio);

	QRect iconSourceRect(0, 0, 256, 256); // Source png is 256 x 256;

	QRect iconRect(targetRect.x(), targetRect.y() + 2, 14, 14);
	painter->drawPixmap(iconRect, *bulbImg, iconSourceRect);

	bool selected = index.data(SelectedFlag).toBool();
	const QPixmap * selectedImg = nullptr;
	if (selected)
		selectedImg = &m_selectedOn;
	else
		selectedImg = &m_selectedOff;

	iconRect.moveLeft(iconRect.x() + 18);
	const_cast<QPixmap*>(selectedImg)->setDevicePixelRatio(SVSettings::instance().m_ratio);
	painter->drawPixmap(iconRect, *selectedImg, iconSourceRect);

	// adjust text rectangle
	targetRect.setX(targetRect.x() + 40);

	// check if item is selected/current
	bool isSelected = option.state & QStyle::State_Selected;
	QFont f = painter->font();
	f.setBold(isSelected);

	bool isMissingFile =  index.data(MissingDrawingFile).toBool();
	f.setItalic(isMissingFile);

	bool isInvalid = index.data(InvalidGeometryFlag).toBool();
	if (isInvalid)
		painter->setPen(QColor(196,0,0));
	else {
		TopologyType t = static_cast<TopologyType>(index.data(ItemType).toInt());
		switch (SVSettings::instance().m_theme) {
			case SVSettings::NUM_TT:
			case SVSettings::TT_White:
				switch (t) {
					case (TT_Building):			painter->setPen(QColor( 78,  87, 135)); break;
					case (TT_BuildingLevel):	painter->setPen(QColor(219, 108,   0)); break;
					case (TT_Room):				painter->setPen(QColor(150,  20,  20)); break;
					case (TT_Subsurface):		painter->setPen(QColor( 70,  80, 125)); break;
					default:					painter->setPen(Qt::black); break;
				}

			break;
			case SVSettings::TT_Dark:
				switch (t) {
					case (TT_Building):			painter->setPen(QColor(150, 140, 190)); break;
					case (TT_BuildingLevel):	painter->setPen(QColor(255, 200, 120)); break;
					case (TT_Room):				painter->setPen(QColor(250, 140, 140));	break;
					case (TT_Subsurface):		painter->setPen(QColor(120, 180, 200)); break;
					default:					painter->setPen(QColor(240, 240, 240)); break;
				}

			break;
		}
	}
	painter->setFont(f);

	painter->drawText(targetRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
}


bool SVNavigationTreeItemDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index) {

	if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent*>(event);
		if (mouseEvent != nullptr && (mouseEvent->button() & Qt::LeftButton)) {
			QRect targetRect(option.rect);
			QRect iconRect(targetRect.x(), targetRect.y(), 16, 16);
			if (iconRect.contains(mouseEvent->x(), mouseEvent->y())) {
				// turn visibility of item on/off
				bool visible = index.data(VisibleFlag).toBool();
				bool withoutChildren = mouseEvent->modifiers() & Qt::ShiftModifier;
				unsigned int nodeID = index.data(NodeID).toUInt();

				// compose an undo action that shows/hides objects
				SVUndoTreeNodeState * action = SVUndoTreeNodeState::createUndoAction(tr("Visibility changed"),
																					 SVUndoTreeNodeState::VisibilityState,
																					 nodeID,
																					 !withoutChildren,
																					 !visible);
				action->push();
				return false; // handled

			}
			iconRect = QRect(targetRect.x() + 18, targetRect.y(), 16, 16);
			if (iconRect.contains(mouseEvent->x(), mouseEvent->y())) {
				// turn visibility of item on/off
				bool selected = index.data(SelectedFlag).toBool();
				bool withoutChildren = mouseEvent->modifiers() & Qt::ShiftModifier;
				unsigned int nodeID = index.data(NodeID).toUInt();
				// compose an undo action that selects/de-selects objects
				SVUndoTreeNodeState * action = SVUndoTreeNodeState::createUndoAction(tr("Selection changed"),
																					 SVUndoTreeNodeState::SelectedState,
																					 nodeID,
																					 !withoutChildren,
																					 !selected);
				action->push();
				return false; // handled

			}
		}

	}
	if (event->type() == QEvent::MouseButtonDblClick) {

		// if it's a drawing, we allow editing some properties
		unsigned int nodeID = index.data(NodeID).toUInt();
		const VICUS::Object *obj = SVProjectHandler::instance().project().objectById(nodeID);
		const VICUS::Drawing *drawing = dynamic_cast<const VICUS::Drawing *>(obj);
		if (drawing != nullptr) {

			VICUS::Drawing newDrawing(*drawing);
			bool result = SVDrawingPropertiesDialog::showDrawingProperties(SVMainWindow::instance().window(), &newDrawing);
			if (result) {

				std::vector<VICUS::Surface> newSurfs;
				std::vector<VICUS::Drawing> newDrawings;

				newDrawings.push_back(newDrawing);
				SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Drawing geometry modified"), newSurfs, newDrawings );
				undo->push();

				SVProjectHandler::instance().setModified( SVProjectHandler::BuildingTopologyChanged );
				return false;
			}
		}

		// if it's a missing drawing file, we allow editing the filepath
		else if (!index.data(MissingDrawingFile).toString().isEmpty()) {
			QFileInfo finfo(index.data(MissingDrawingFile).toString());
			QString drawFilename = QFileDialog::getOpenFileName(
				SVMainWindow::instance().window(),
				tr("Find missing drawing file"),
				finfo.path(),
				tr("SIM-VICUS drawing files (*%1 );;All files (*.*)").arg(SVSettings::instance().m_drawingFileSuffix), nullptr,
				SVSettings::instance().m_dontUseNativeDialogs ? QFileDialog::DontUseNativeDialog : QFileDialog::Options() );
			if (!drawFilename.isEmpty()) {
				IBK::Path drawingFile(drawFilename.toStdString());
				SVUndoModifyDrawingFile *undo = new SVUndoModifyDrawingFile("Drawing file changed", drawingFile);
				undo->push();
				return false;
			}
		}
	}
	return QStyledItemDelegate::editorEvent(event, model, option, index);
}


void SVNavigationTreeItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const {
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);
	// move inside a little
	editor->setGeometry(editor->pos().x() + 34, editor->pos().y(),  editor->width()-34, editor->height());
}


QSize SVNavigationTreeItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
	QSize sh = QStyledItemDelegate::sizeHint(option, index);
	sh.setHeight(16); // enough space for 16x16 icon
	return sh;
}
