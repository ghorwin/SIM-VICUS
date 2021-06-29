/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

*/

#include "QtExt_ActiveLabel.h"

#include <QString>
#include <QColor>
#include <QDebug>
#include <QMouseEvent>


namespace QtExt {


ActiveLabel::ActiveLabel(const QString & text) :
	QLabel(text)
{
	initColors();
}

ActiveLabel::ActiveLabel(QWidget * parent) :
	QLabel(parent)
{
	initColors();
}

ActiveLabel::~ActiveLabel() {
}

void ActiveLabel::setHoverColor(const QColor & c) {
	m_hoverColor = c;
}

const QColor & ActiveLabel::hoverColor() const {
	return m_hoverColor;
}

void ActiveLabel::setNormalColor(const QColor & c) {
	m_normalColor = c;
	QPalette pal(palette());
	pal.setColor(QPalette::WindowText, m_normalColor);
	setPalette(pal);
}

const QColor & ActiveLabel::normalColor() const {
	return m_normalColor;
}

void ActiveLabel::click() {
	emit clicked(true);
}

void ActiveLabel::reset() {
	QPalette pal(palette());
	pal.setColor(QPalette::WindowText, m_normalColor);
	setPalette(pal);
}


// *** PROTECTED FUNCTIONS ***

void ActiveLabel::enterEvent(QEvent * event) {
	QPalette pal(palette());
	pal.setColor(QPalette::WindowText, m_hoverColor);
	setPalette(pal);
	QWidget::enterEvent(event);
}

void ActiveLabel::leaveEvent(QEvent * event) {
	QPalette pal(palette());
	pal.setColor(QPalette::WindowText, m_normalColor);
	setPalette(pal);
	QWidget::leaveEvent(event);
}

void ActiveLabel::mousePressEvent(QMouseEvent * event) {
	if (event->button() == Qt::LeftButton) {
		click();
	}
}

// *** PRIVATE FUNCTIONS ***

void ActiveLabel::initColors() {
	m_hoverColor = Qt::white;
	m_normalColor = Qt::lightGray;
	QPalette pal(palette());
	pal.setColor(QPalette::WindowText, m_normalColor);
	setPalette(pal);
}

} // namespace QtExt
