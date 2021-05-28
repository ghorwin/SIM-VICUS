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

#include "Vic3DOpenGLWindow.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>

#include <IBK_Exception.h>

namespace Vic3D {

OpenGLWindow::OpenGLWindow(QWindow *parent) :
	QWindow(parent),
	m_context(nullptr),
	m_debugLogger(nullptr)
{
	setSurfaceType(QWindow::OpenGLSurface);
}


void OpenGLWindow::renderLater() {
	// Schedule an UpdateRequest event in the event loop
	// that will be send with the next VSync.
	requestUpdate(); // call public slot requestUpdate()
}


void OpenGLWindow::renderNow() {
	if (!isExposed())
		return;

	// initialize on first call
	if (m_context == nullptr) {
		initOpenGL();
		resizeGL(width(), height());
	}

	m_context->makeCurrent(this);

	paintGL(); // call user code

	m_context->swapBuffers(this);
}


// *** protected functions ***

bool OpenGLWindow::event(QEvent *event) {
	switch (event->type()) {
	case QEvent::UpdateRequest:
		renderNow();
		return true;
	default:
		return QWindow::event(event);
	}
}


void OpenGLWindow::exposeEvent(QExposeEvent * /*event*/) {
//	qDebug() << "OpenGLWindow::exposeEvent()";
	renderNow(); // update right now

	// Note: if were just to request an update on next sync, i.e. by
	//       calling renderLater() (or requestUpdate()) we get
	//       white glitches when enlarging the window. Since we don't want that,
	//       we simply render right away so that the new window size
	//       is already reflected by the adjusted viewport we render into.
}


void OpenGLWindow::resizeEvent(QResizeEvent * event) {
//	qDebug() << "OpenGLWindow::resizeEvent()";
	QWindow::resizeEvent(event);

	if (!isExposed())
		return;

	// initialize on first call
	if (m_context == nullptr)
		initOpenGL();

	resizeGL(width(), height());
}


void OpenGLWindow::onMessageLogged(const QOpenGLDebugMessage &msg) {
	QString prefix;

	// Format based on severity
	switch (msg.severity()) {
		case QOpenGLDebugMessage::NotificationSeverity:
			prefix += "++++";
		break;
		case QOpenGLDebugMessage::HighSeverity:
			prefix += "+++";
		break;
		case QOpenGLDebugMessage::MediumSeverity:
			prefix += "++";
		break;
		case QOpenGLDebugMessage::LowSeverity:
			prefix += "+";
		break;
		case QOpenGLDebugMessage::InvalidSeverity:
			prefix += "+?+";
		break;
		case QOpenGLDebugMessage::AnySeverity: // no prefix
		break;
	}

	prefix += " [";

	// Format based on source
#define CASE(c) case QOpenGLDebugMessage::c: prefix += #c; break
	switch (msg.source())
	{
	  CASE(APISource);
	  CASE(WindowSystemSource);
	  CASE(ShaderCompilerSource);
	  CASE(ThirdPartySource);
	  CASE(ApplicationSource);
	  CASE(OtherSource);
	  CASE(InvalidSource);
	  CASE(AnySource);
	}
 #undef CASE

	prefix += ":";

	// Format based on type
 #define CASE(c) case QOpenGLDebugMessage::c: prefix += #c; break
	switch (msg.type())
	{
		CASE(ErrorType);
		CASE(DeprecatedBehaviorType);
		CASE(UndefinedBehaviorType);
		CASE(PortabilityType);
		CASE(PerformanceType);
		CASE(OtherType);
		CASE(MarkerType);
		CASE(GroupPushType);
		CASE(GroupPopType);
		CASE(InvalidType);
		CASE(AnyType);
	}
#undef CASE

	prefix += "] ";
	qDebug().noquote().nospace() << prefix << msg.message() << "\n";

}


void OpenGLWindow::initOpenGL() {
	Q_ASSERT(m_context == nullptr);

	m_context = new QOpenGLContext(this);
	m_context->setFormat(requestedFormat());
	m_context->create();

	if (!m_context->makeCurrent(this)) {
		throw IBK::Exception("Cannot make OpenGL context current (driver problem?).", "[OpenGLWindow::initOpenGL]");
	}
	Q_ASSERT(m_context->isValid());

	initializeOpenGLFunctions();

#ifdef GL_DEBUG
	if (m_context->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
		qDebug() << "GL_KHR_debug extension available";
	else
		qWarning() << "GL_KHR_debug extension *not* available";
	m_debugLogger = new QOpenGLDebugLogger(this);
	if (m_debugLogger->initialize()) {
		qDebug() << "Debug Logger initialized\n";
		connect(m_debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(onMessageLogged(QOpenGLDebugMessage)));
		m_debugLogger->startLogging();
	}
	qDebug() << "DepthBufferSize = " << m_context->format().depthBufferSize();
#endif // GL_DEBUG

	initializeGL(); // call user code
}

} // namespace Vic3D
