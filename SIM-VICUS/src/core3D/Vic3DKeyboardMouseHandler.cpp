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

#include "Vic3DKeyboardMouseHandler.h"

#include <cstdlib>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
//#include <QDebug>

namespace Vic3D {

KeyboardMouseHandler::KeyboardMouseHandler() :
	m_leftButtonDown(StateNotPressed),
	m_middleButtonDown(StateNotPressed),
	m_rightButtonDown(StateNotPressed),
	m_wheelDelta(0)
{
}


KeyboardMouseHandler::~KeyboardMouseHandler() {
}



void KeyboardMouseHandler::keyPressEvent(QKeyEvent *event) {
	if (event->isAutoRepeat()) {
		event->ignore();
	}
	else {
		pressKey(static_cast<Qt::Key>((event->key())));
	}
}


void KeyboardMouseHandler::keyReleaseEvent(QKeyEvent *event) {
	if (event->isAutoRepeat())	{
		event->ignore();
	}
	else {
		releaseKey(static_cast<Qt::Key>(event->key()));
	}
	if (!(event->modifiers() & Qt::ShiftModifier))
		releaseKey(Qt::Key_Shift);
	if (!(event->modifiers() & Qt::AltModifier))
		releaseKey(Qt::Key_Alt);
	if (!(event->modifiers() & Qt::ControlModifier))
		releaseKey(Qt::Key_Control);
}


void KeyboardMouseHandler::mousePressEvent(QMouseEvent *event) {
	pressButton(static_cast<Qt::MouseButton>(event->button()), event->globalPos());
}


void KeyboardMouseHandler::mouseReleaseEvent(QMouseEvent *event) {
	releaseButton(static_cast<Qt::MouseButton>(event->button()), event->globalPos());
	// If we have any modifier keys held, set these states.
	// This is a fix if, for example, Shift was pressed while the focus was still elsewhere in the world
	// and now someone clicked with the mouse into the scene.
	// In this case, the key press event was not received by the keyboard mouse handler and hence needs to
	// be set manually.

	if (event->modifiers() & Qt::ShiftModifier)
		pressKey(Qt::Key_Shift);
	if (event->modifiers() & Qt::AltModifier)
		pressKey(Qt::Key_Alt);
	if (event->modifiers() & Qt::ControlModifier)
		pressKey(Qt::Key_Control);
}


void KeyboardMouseHandler::wheelEvent(QWheelEvent *event) {
	QPoint numPixels = event->pixelDelta();
	QPoint numDegrees = event->angleDelta() / 8;

	if (!numPixels.isNull()) {
		// on highres displays, numPixels may be really large, we limit this to 2
		int pixels = std::abs(numPixels.x() )>0 ? numPixels.x() : numPixels.y();
		if (pixels > 2)
			pixels = 2;
		else if (pixels < -2)
			pixels = -2;
		m_wheelDelta += pixels;
	}
	else if (!numDegrees.isNull()) {
		QPoint numSteps = numDegrees / 15;

		// unfortunately alt key changes x and y delta
		m_wheelDelta += std::abs(numSteps.x() )>0 ? numSteps.x() : numSteps.y();
	}
	event->accept();
}


void KeyboardMouseHandler::addRecognizedKey(Qt::Key k) {
	if (std::find(m_keys.begin(), m_keys.end(), k) != m_keys.end())
		return; // already known
	// remember key to be known and expected
	m_keys.push_back(k);
	m_keyStates.push_back(StateNotPressed);
}


void KeyboardMouseHandler::clearRecognizedKeys() {
	m_keys.clear();
	m_keyStates.clear();
}


void KeyboardMouseHandler::clearWasPressedKeyStates() {
	m_leftButtonDown = (m_leftButtonDown == StateWasPressed) ? StateNotPressed  : m_leftButtonDown;
	m_middleButtonDown = (m_middleButtonDown == StateWasPressed) ? StateNotPressed  : m_middleButtonDown;
	m_rightButtonDown = (m_rightButtonDown == StateWasPressed) ? StateNotPressed  : m_rightButtonDown;

	for (unsigned int i=0; i<m_keyStates.size(); ++i)
		m_keyStates[i] = static_cast<KeyStates>(m_keyStates[i] & 1); // toggle "WasPressed" bit -> NotPressed
}

std::vector<Qt::Key> KeyboardMouseHandler::keys() const
{
	return m_keys;
}



bool KeyboardMouseHandler::pressKey(Qt::Key k) {
	for (unsigned int i=0; i<m_keys.size(); ++i) {
		if (m_keys[i] == k) {
			m_keyStates[i] = StateHeld;
//			qDebug() << k << "Held";
			return true;
		}
	}
	return false;
}


bool KeyboardMouseHandler::releaseKey(Qt::Key k) {
	for (unsigned int i=0; i<m_keys.size(); ++i) {
		if (m_keys[i] == k) {
			m_keyStates[i] = StateWasPressed;
//			qDebug() << k << "released";
			return true;
		}
	}
	return false;
}


bool KeyboardMouseHandler::pressButton(Qt::MouseButton btn, QPoint currentPos) {
	switch (btn) {
		case Qt::LeftButton		: m_leftButtonDown = StateHeld; break;
		case Qt::MiddleButton	: m_middleButtonDown = StateHeld; break;
		case Qt::RightButton	: m_rightButtonDown = StateHeld; break;
		default: return false;
	}
	m_mouseDownPos = currentPos;
	return true;
}


bool KeyboardMouseHandler::releaseButton(Qt::MouseButton btn, QPoint currentPos) {
	switch (btn) {
		case Qt::LeftButton		: m_leftButtonDown = StateWasPressed; break;
		case Qt::MiddleButton	: m_middleButtonDown = StateWasPressed; break;
		case Qt::RightButton	: m_rightButtonDown = StateWasPressed; break;
		default: return false;
	}
	m_mouseReleasePos = currentPos;
	return true;
}


QPoint KeyboardMouseHandler::mouseDelta(const QPoint currentPos) const {
	QPoint dist = currentPos - m_mouseDownPos;
	return dist;
}


QPoint KeyboardMouseHandler::resetMouseDelta(const QPoint currentPos) {
	QPoint dist = currentPos - m_mouseDownPos;
	m_mouseDownPos = currentPos;
	return dist;
}


int KeyboardMouseHandler::wheelDelta() const {
	return m_wheelDelta;
}


int KeyboardMouseHandler::resetWheelDelta() {
	int wd = m_wheelDelta;
	m_wheelDelta = 0;
	return wd;
}


bool KeyboardMouseHandler::keyDown(Qt::Key k) const {
	for (unsigned int i=0; i<m_keys.size(); ++i) {
		if (m_keys[i] == k)
			return m_keyStates[i] == StateHeld;
	}
	return false;
}


bool KeyboardMouseHandler::keyReleased(Qt::Key k) const {
	for (unsigned int i=0; i<m_keys.size(); ++i) {
		if (m_keys[i] == k)
			return m_keyStates[i] == StateWasPressed;
	}
	return false;
}


bool KeyboardMouseHandler::anyKeyDown() const {
	for (unsigned int i=0; i<m_keys.size(); ++i)
		if (m_keyStates[i] != StateNotPressed)
			return true;
	return false;
}


bool KeyboardMouseHandler::buttonDown(Qt::MouseButton btn) const {
	switch (btn) {
		case Qt::LeftButton		: return m_leftButtonDown == StateHeld;
		case Qt::MiddleButton	: return m_middleButtonDown == StateHeld;
		case Qt::RightButton	: return m_rightButtonDown == StateHeld;
		default: return false;
	}
}


bool KeyboardMouseHandler::buttonReleased(Qt::MouseButton btn) const {
	switch (btn) {
		case Qt::LeftButton		: return m_leftButtonDown == StateWasPressed;
		case Qt::MiddleButton	: return m_middleButtonDown == StateWasPressed;
		case Qt::RightButton	: return m_rightButtonDown == StateWasPressed;
		default: return false;
	}
}

} // namespace Vic3D
