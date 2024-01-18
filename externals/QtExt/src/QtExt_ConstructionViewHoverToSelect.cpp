#include "QtExt_ConstructionViewHoverToSelect.h"

#include <QDebug>

namespace QtExt {


ConstructionViewHoverToSelect::ConstructionViewHoverToSelect(QWidget *parent):
	ConstructionView(parent)
{
	ConstructionView::setReadOnly(true);
}

void ConstructionViewHoverToSelect::setHoverProperties(const QPixmap & icon, QColor color) {
	m_icon = icon;
	m_hoverColor = color;
}

void ConstructionViewHoverToSelect::updateEditIcon() {

	if (m_inputData.isEmpty() && m_diagramScene!=nullptr && isEnabled()) {
		// set scene rect and get size
		int w = width();
		int h = height();
		QRectF frame(m_margins, m_margins, w - m_margins * 2, h - m_margins * 2);
		m_diagramScene->setSceneRect(frame);
		QRectF sceneRect = m_diagramScene->sceneRect();
		// add pixmap
		m_iconItem = m_diagramScene->addPixmap(m_icon);
		m_iconItem->setPos(sceneRect.center() - m_iconItem->boundingRect().center());
	}
}


void ConstructionViewHoverToSelect::mouseReleaseEvent(QMouseEvent * event) {
	if (!m_isReadOnly)
		emit sceneClicked();
	ConstructionView::mouseReleaseEvent(event);
}


void ConstructionViewHoverToSelect::enterEvent(QEvent * event) {
	if (m_diagramScene == nullptr || !isEnabled())
		return;

	QColor col1;
	if (m_isReadOnly)
		col1 = QColor("#727571"); // grey
	else
		col1 = m_hoverColor;

	// Create a gradient
	QRectF sceneRect = m_diagramScene->sceneRect();
	QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
	col1.setAlpha(200);
	QColor col2("#ffffff");
	col2.setAlpha(0);
	gradient.setColorAt(0, col2);  // Starting color
	gradient.setColorAt(1, col1);  // Ending color
	QBrush brush(gradient);

	// add background
	QRectF r = sceneRect.adjusted(-0.1*sceneRect.width(), -0.1*sceneRect.height(),
								  0.1*sceneRect.width(), 0.1*sceneRect.height());
	m_transparentRect = m_diagramScene->addRect(r, QPen(Qt::NoPen), brush);

	// add icon if editable
	if (!m_isReadOnly) {
		m_iconItem = m_diagramScene->addPixmap(m_icon);
		m_iconItem->setPos(sceneRect.center() - m_iconItem->boundingRect().center());
	}

	update();

	ConstructionView::enterEvent(event);
}


void ConstructionViewHoverToSelect::leaveEvent(QEvent * event) {
	if (m_diagramScene == nullptr || !isEnabled())
		return;
	if (!m_isReadOnly && m_iconItem->scene()!=nullptr)
		m_diagramScene->removeItem(m_iconItem);
	if (m_transparentRect->scene()!=nullptr)
		m_diagramScene->removeItem(m_transparentRect);

	ConstructionView::leaveEvent(event);
}


} // namespace QtExt
