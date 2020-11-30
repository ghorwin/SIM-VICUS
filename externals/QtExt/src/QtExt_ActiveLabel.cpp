/*	Authors: H. Fechner, A. Nicolai

	This file is part of the QtExt Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

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
