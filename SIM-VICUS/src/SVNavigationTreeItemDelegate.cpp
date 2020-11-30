#include "SVNavigationTreeItemDelegate.h"

#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

#include <set>

SVNavigationTreeItemDelegate::SVNavigationTreeItemDelegate(QWidget * parent) :
	QItemDelegate(parent)
{
	m_lightBulbOn = QImage("://gfx/actions/16x16/help-hint.png");
	m_lightBulbOff = QImage("://gfx/actions/16x16/help-hint_gray.png");

}


void SVNavigationTreeItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
	const QStyleOptionViewItem * opt = &option;

	// TODO : find out if the element we are painting is visible or not
	bool visible = index.data(Qt::UserRole).toBool();

	const QImage * bulbImg = nullptr;
	if (visible)
		bulbImg = &m_lightBulbOn;
	else
		bulbImg = &m_lightBulbOff;

	// get rectangle of area to paint into
	QRect targetRect(option.rect);
	QRect bulbRect(targetRect.x(), targetRect.y(), 16, 16);
	painter->drawImage(bulbRect, *bulbImg, QRect(0,0,16,16));

	// adjust text rectangle
	targetRect.setX(targetRect.x()+18);
	painter->drawText(targetRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
}


bool SVNavigationTreeItemDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index) {
	if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent*>(event);
		if (mouseEvent != nullptr && (mouseEvent->button() & Qt::LeftButton)) {
			QRect targetRect(option.rect);
			QRect bulbRect(targetRect.x(), targetRect.y(), 16, 16);
			if (bulbRect.contains(mouseEvent->x(), mouseEvent->y())) {
				// turn visibility of item on/off
				bool visible = index.data(Qt::UserRole).toBool();

				bool withoutChildren = mouseEvent->modifiers() & Qt::ShiftModifier;

				// compose an undo action that shows/hides objects

			}
		}

	}


	QItemDelegate::editorEvent(event, model, option, index);
}
