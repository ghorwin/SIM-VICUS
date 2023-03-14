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

#ifndef Vic3DKeyboardMouseHandlerH
#define Vic3DKeyboardMouseHandlerH

#include <QPoint>
#include <vector>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace Vic3D {

/*! An example keyboard/mouse handler implementation.
	When a keyboard key has been pressed, the corresponding's key state is updated, same
	when it is release (in this case the state will be "was pressed" until cleared). This
	"was pressed" state is needed in those cases, where a delay of state requesting on the
	client side causes a keyPress and keyRelease Sequence to happen before the states
	where checked.

	In addition, when a mouse button is pressed, the corresponding position is stored, so
	that a mouse move delta kann be determined.

	The handler monitors only keys that it was told to monitor. Mouse buttons are all monitored.
	Modification keys are used just as any other key.

	State management:
	- the states of pressed keys are remembered in state WasPressed until clearWasPressedKeyStates() is called
	- the mouse position is updated in mouseDelta(), which requires the new mouse cursor position to update
	  the mouse position for the next call cycle

	\code
	// usage pattern

	// ... some change of state is registered during the event loop

	// evaluate input handler state

	// process all keys and check if they are either held or where pressed

	if (m_inputHandler.keyDown(Qt::Key_W)) {
		...
	}

	// get and reset mouse delta (pass current mouse cursor position)
	QPoint mouseDelta = m_inputHandler.mouseDelta(QCursor::pos());

	// compute mouse speed
	static const float rotatationSpeed   = 0.1f; // in space units/frame

	float yRot = rotationSpeed * mouseDelta.y();

	...

	// finally, reset "WasPressed" key states
	m_inputHandler.clearWasPressedKeyStates();
	m_inputHandler.resetMouseDelta();
	\endcode
*/
class KeyboardMouseHandler {
public:
	KeyboardMouseHandler();
	virtual ~KeyboardMouseHandler();

	/*! Call this function for each key we are listening to. */
	void addRecognizedKey(Qt::Key k);
	/*! Clears list of recognized keys. */
	void clearRecognizedKeys();

	// Functions to handle key press and mouse press events
	// These function return true, if a recognized key/mouse button was pressed and
	// the scene may need to be updated.
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);

	/*! Called when a key was pressed. */
	bool pressKey(Qt::Key k);
	/*! Called when a key was released. */
	bool releaseKey(Qt::Key k);
	/*! Called when a mousebutton was pressed. */
	bool pressButton(Qt::MouseButton btn, QPoint currentPos);
	/*! Called when a mousebutton was released. */
	bool releaseButton(Qt::MouseButton btn, QPoint currentPos);

	/*! Returns, whether the key is pressed. */
	bool keyDown(Qt::Key k) const;
	/*! Returns, whether the key was pressed in last query interval (but is no longer pressed). */
	bool keyReleased(Qt::Key k) const;
	/*! Returns true, if any of the monitored keys was pressed. */
	bool anyKeyDown() const;

	/*! Returns, whether the mouse button is pressed or was pressed in last query interval. */
	bool buttonDown(Qt::MouseButton btn) const;
	/*! Returns, whether the mouse button was pressed and is now released. */
	bool buttonReleased(Qt::MouseButton btn) const;

	/*! Returns the position (global pos) that was recorded, when a mouse button was pressed.
		Use this function to determine whether the mouse has been moved (by comparing it to the QCursor::pos()).
	*/
	QPoint mouseDownPos() const { return m_mouseDownPos; }

	/*! Returns the position (global pos) that was recorded, when a mouse button was released. */
	QPoint mouseReleasePos() const { return m_mouseReleasePos; }

	/*! Returns the difference between last and current mouse position.
		The retrieved point (x and y distances) should be used to modify state based transformations.
	*/
	QPoint mouseDelta(const QPoint currentPos) const;

	/*! Returns the difference between last and current mouse position and *updates*
		last mouse position to currentPos.
		The retrieved point (x and y distances) should be used to modify state based transformations.
	*/
	QPoint resetMouseDelta(const QPoint currentPos);

	/*! Retrieves the wheel distance (angle in degree) that was added up so far.
		Use this function to query, if the mouse wheel had been turned.
	*/
	int wheelDelta() const;

	/*! Retrieves the wheel distance (angle in degree) that was added up in the last query interval
		and resets it to zero.
	*/
	int resetWheelDelta();

	/*! This resets all key states currently marked as "WasPressed". */
	void clearWasPressedKeyStates();

private:
	enum KeyStates {
		StateNotPressed,
		StateHeld,
		StateWasPressed
	};

	std::vector<Qt::Key>	m_keys;
	std::vector<KeyStates>	m_keyStates;

	KeyStates				m_leftButtonDown;
	KeyStates				m_middleButtonDown;
	KeyStates				m_rightButtonDown;

	QPoint					m_mouseDownPos;
	QPoint					m_mouseReleasePos;

	int						m_wheelDelta;
};

} // namespace Vic3D

#endif // Vic3DKeyboardMouseHandlerH
