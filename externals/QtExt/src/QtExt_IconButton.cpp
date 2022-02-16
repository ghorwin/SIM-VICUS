/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_IconButton.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QShortcut>
#include <QKeySequence>



namespace QtExt {


IconButton::IconButton(QWidget * parent) :
	QWidget(parent),
	m_shortcut( new QShortcut( parent ) ),
	m_checked(false),
	m_checkable(false),
	m_insideWidget(false),
	m_mouseButtonDown(false)
{
	resize(16,16);
	setMaximumSize(16,16);
	setMinimumSize(16,16);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setVisible(true);
	setFocusPolicy( Qt::StrongFocus );

	m_shortcut->setContext( Qt::WidgetWithChildrenShortcut );
	connect(m_shortcut, SIGNAL(activated()), this, SLOT(shortcutClicked()));
}


IconButton::~IconButton(){
  //qDebug() << "IconButton::~IconButton";
}

void IconButton::setNormalIcons(const QPixmap & normalIcon,
		const QPixmap & activeIcon,
		const QPixmap & disabledIcon)
{
	m_icons[NormalState] = normalIcon;
	m_icons[ActiveState] = activeIcon;
	m_icons[DisabledState] = disabledIcon;
	resize(normalIcon.size());
}

void IconButton::setCheckedIcons(const QPixmap & normalIcon,
		const QPixmap & activeIcon,
		const QPixmap & disabledIcon)
{
	m_checkedIcons[NormalState] = normalIcon;
	m_checkedIcons[ActiveState] = activeIcon;
	m_checkedIcons[DisabledState] = disabledIcon;
}

void IconButton::setEnabled(bool enabled) {
	if (enabled != QWidget::isEnabled()) {
		QWidget::setEnabled(enabled);
		m_shortcut->setEnabled(enabled);
		update();
	}
}

void IconButton::setChecked(bool checked) {
	m_checked = checked;
	update();
}

void IconButton::setCheckable(bool enabled) {
	m_checkable = enabled;
	if (!enabled && m_checked) {
		setChecked(false);
	}
}

bool IconButton::isCheckable() const {
	return m_checkable;
}

void IconButton::paintEvent ( QPaintEvent * event ) {

	Q_UNUSED(event)

	QPainter p(this);

	// widget is disabled?
	if (!isEnabled()) {
		// widget is checked?
		if (m_checked) {
			// checked - disabled
			p.drawPixmap(QPoint(0,0), m_checkedIcons[DisabledState]);
		}
		else {
			// not checked - disabled
			p.drawPixmap(QPoint(0,0), m_icons[DisabledState]);
		}
	}
	else {
		if (m_checked ^ (m_mouseButtonDown && m_insideWidget) ) {
			// checked

			// widget is highlighted?
			if (m_insideWidget)
				p.drawPixmap(QPoint(0,0), m_checkedIcons[ActiveState]);
			else
				p.drawPixmap(QPoint(0,0), m_checkedIcons[NormalState]);
		}
		else {
			// not checked

			// widget is highlighted?
			if (m_insideWidget)
				p.drawPixmap(QPoint(0,0), m_icons[ActiveState]);
			else
				p.drawPixmap(QPoint(0,0), m_icons[NormalState]);
		}
	}
}

void IconButton::enterEvent ( QEvent * event ) {
	Q_UNUSED(event)

	m_insideWidget = true;
	update();
}

void IconButton::leaveEvent ( QEvent * event ) {
	Q_UNUSED(event)

	m_insideWidget = false;
	update();
}

void IconButton::mousePressEvent ( QMouseEvent * event ) {
	if (event->button() == Qt::LeftButton) {
		m_mouseButtonDown = true;
		update(); // this will toggle the check state visually
	}
}

void IconButton::mouseMoveEvent ( QMouseEvent * event ) {
	// check if we are inside the widget or outside
	bool insideWidget = event->x() < width() &&  event->y() < height();
	if (insideWidget != m_insideWidget) {
		m_insideWidget = insideWidget;
		update();
	}
}

void IconButton::mouseReleaseEvent ( QMouseEvent * event ) {
	if (event->button() == Qt::LeftButton) {
		m_mouseButtonDown = false;

		update(); // this will toggle the check state visually

		// only emit the clicked signal if mouse within widget
		if (m_insideWidget) {
			if (m_checkable) {
				m_checked = !m_checked;
				emit clicked( m_checked );
			}
			else {
				emit clicked();
			}
		}
	}
}


void IconButton::setShortCut( const QKeySequence keySequence ){
	m_shortcut->setKey( keySequence );
}

void IconButton::shortcutClicked() {
	// TODO : check if this is still connected if the button itself is disabled?

//	qDebug() << "IconButton::shortcutClicked";

	if (m_checkable) {
		setChecked(!m_checked);
		emit clicked( m_checked );
	} else {
		emit clicked();
	}
}


} // namespace QtExt {
