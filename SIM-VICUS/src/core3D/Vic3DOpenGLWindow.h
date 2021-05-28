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

#ifndef Vic3DOpenGLWindowH
#define Vic3DOpenGLWindowH

#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>

#include <QOpenGLDebugLogger>

QT_BEGIN_NAMESPACE
class QOpenGLContext;
QT_END_NAMESPACE

namespace Vic3D {

/*! The OpenGLWindow is very similar to QOpenGLWindow, yet a little more light-weight.
	Also, the functions initializeGL() and paintGL() are protected, as they are in
	the QOpenGLWidget. Thus, you can easily switch to an QOpenGLWidget class later on,
	if you need it.
*/
class OpenGLWindow : public QWindow, protected QOpenGLFunctions {
	Q_OBJECT
public:
	explicit OpenGLWindow(QWindow *parent = nullptr);

public slots:
	/*! Redirects to slot requestUpdate(), which registers an UpdateRequest event in the event loop
		to be issued with next VSync.
	*/
	void renderLater();

	/*! Directly repaints the view right now (this function is called from event() and exposeEvent(). */
	void renderNow();

protected:
	bool event(QEvent *event) override;
	void exposeEvent(QExposeEvent *event) override;
	void resizeEvent(QResizeEvent *) override;

	/*! Called on first show of the window. Re-implement with your own
		OpenGL initialization code.
	*/
	virtual void initializeGL() = 0;

	/*! Called whenever the view port changes (window geometry). Re-implement
		in your own code, for example to update the projection matrix.
		This function is called from resizeEvent() and thus before paintGL().
		\param width Width of window in pixels as returned from width()
		\param height Height of window in pixels as returned from height()
	*/
	virtual void resizeGL(int width, int height) { Q_UNUSED(width) Q_UNUSED(height) }

	/*! Called just after the OpenGL context was made current. Re-implement in
		derived classes to do the actual painting.
	*/
	virtual void paintGL() = 0;

	QOpenGLContext		*m_context;

private slots:

	/*! Receives debug messages from QOpenGLDebugLogger */
	void onMessageLogged(const QOpenGLDebugMessage &msg);


private:
	/*! Helper function to initialize the OpenGL context. */
	void initOpenGL();

	QOpenGLDebugLogger	*m_debugLogger;
};

} // namespace Vic3D

#endif // Vic3DOpenGLWindowH
