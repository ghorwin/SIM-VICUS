#include "QtExt_ClickableLabel.h"

#include <QMouseEvent>

namespace QtExt {

ClickableLabel::ClickableLabel(const QString & text, QWidget* parent):
	QLabel(text, parent)
{
	m_normalStyleSheet = "QLabel { font-weight: normal; }";
	m_hoverStyleSheet = "QLabel { font-weight: bold; }";
	QLabel::setStyleSheet(m_normalStyleSheet);
}


ClickableLabel::ClickableLabel(int id, const QString & text, QWidget* parent):
	QLabel(text, parent),
	m_id(id)
{
	m_normalStyleSheet = "QLabel { font-weight: normal; }";
	m_hoverStyleSheet = "QLabel { font-weight: bold; }";
	QLabel::setStyleSheet(m_normalStyleSheet);
}


void ClickableLabel::setStyleSheet(const QString & normalStyleSheet, const QString & hoverStyleSheet) {
	m_normalStyleSheet = normalStyleSheet;
	m_hoverStyleSheet = hoverStyleSheet;
	if (m_hovered)
		QLabel::setStyleSheet(m_hoverStyleSheet);
	else
		QLabel::setStyleSheet(m_normalStyleSheet);
}


void ClickableLabel::mousePressEvent(QMouseEvent * event) {
	if (event->button() == Qt::LeftButton) {
		emit clicked();
	}
	QLabel::mousePressEvent(event); // to catch QUrl-clicks
}


void ClickableLabel::enterEvent(QEvent * ev) {
	QLabel::enterEvent(ev);
	m_hovered = true;
	QLabel::setStyleSheet(m_hoverStyleSheet);
}


void ClickableLabel::leaveEvent(QEvent *ev) {
	QLabel::leaveEvent(ev);
	m_hovered = false;
	QLabel::setStyleSheet(m_normalStyleSheet);
}


} // namespace Qt_Ext
