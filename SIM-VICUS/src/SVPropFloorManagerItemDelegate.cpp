#include "SVPropFloorManagerItemDelegate.h"

#include <QPainter>
#include <QDebug>
#include <QEvent>

SVPropFloorManagerItemDelegate::SVPropFloorManagerItemDelegate(QWidget * parent) :
	QItemDelegate(parent)
{
}


void SVPropFloorManagerItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
//	// get rectangle of area to paint into
//	QRect targetRect(option.rect);

//	// Note: the first part of the widget (the branch indicator) is not drawn by us, but rather by the tree view itself
//	//       to change that, we would need to derive from QTreeView and overload/re-implement drawRow().
//	//       --> later

//	unsigned int uniqueObjectId = index.data(NodeID).toUInt();

//	if (index.parent() == QModelIndex() || uniqueObjectId == 0) {
//		// check if item is selected/current
//		bool isSelected = option.state & QStyle::State_Selected;
//		QFont f = painter->font();
//		f.setBold(isSelected);
//		painter->setFont(f);
//		painter->drawText(targetRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
//		return;
//	}

//	// TODO : find out if the element we are painting is visible or not
//	bool visible = index.data(VisibleFlag).toBool();

//	const QImage * bulbImg = nullptr;
//	if (visible)
//		bulbImg = &m_lightBulbOn;
//	else
//		bulbImg = &m_lightBulbOff;

//	QRect iconRect(targetRect.x(), targetRect.y(), 16, 16);
//	painter->drawImage(iconRect, *bulbImg, QRect(0,0,16,16));

//	bool selected = index.data(SelectedFlag).toBool();
//	const QImage * selectedImg = nullptr;
//	if (selected)
//		selectedImg = &m_selectedOn;
//	else
//		selectedImg = &m_selectedOff;
//	iconRect.setX(iconRect.x()+18);
//	painter->drawImage(iconRect, *selectedImg, QRect(0,0,16,16));

//	// adjust text rectangle
//	targetRect.setX(targetRect.x()+36);

//	// check if item is selected/current
//	bool isSelected = option.state & QStyle::State_Selected;
//	QFont f = painter->font();
//	f.setBold(isSelected);
//	painter->setFont(f);

//	painter->drawText(targetRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
	QItemDelegate::paint(painter, option, index);
}

QWidget * SVPropFloorManagerItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const {
	if (index.parent() == QModelIndex() && index.column() != 0)
		return nullptr;
	return QItemDelegate::createEditor(parent, option, index);
}


//bool SVPropFloorManagerItemDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index) {
//	if (index.parent() == QModelIndex()) {
//		// top-level index only allows editing in first column
//		if (index.column() == 0)
//			return QItemDelegate::editorEvent(event, model, option, index);
//		else {
//			event->accept();
//			return false;
//		}
//	}

//	return QItemDelegate::editorEvent(event, model, option, index);
//}
