#include "QtExt_ClickableLabel.h"

#include <QMouseEvent>

namespace QtExt {

ClickableLabel::ClickableLabel(const QString & text, QWidget* parent):
	QLabel(text, parent)
{
}

ClickableLabel::ClickableLabel(int id, const QString & text, QWidget* parent):
	QLabel(text, parent),
	m_id(id)
{
}

void QtExt::ClickableLabel::mousePressEvent(QMouseEvent * event) {
	if (event->button() == Qt::LeftButton) {
		emit clicked();
		emit clicked(m_id);
	}
}


} // namespace Qt_Ext
