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
	if (m_active)
		QLabel::setStyleSheet(m_hoverStyleSheet);
	else
		QLabel::setStyleSheet(m_normalStyleSheet);
}


void ClickableLabel::setActive(bool active) {
	m_active = active;
	if (m_active)
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
	QLabel::setStyleSheet(m_hoverStyleSheet);
}


void ClickableLabel::leaveEvent(QEvent *ev) {
	QLabel::leaveEvent(ev);
	if (!m_active)
		QLabel::setStyleSheet(m_normalStyleSheet);
}


} // namespace Qt_Ext
