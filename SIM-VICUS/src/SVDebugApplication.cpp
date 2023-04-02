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

#include "SVDebugApplication.h"

#include <QLineEdit>
#include <QDebug>
#include <QKeyEvent>

#include <IBK_Exception.h>
#include <IBK_messages.h>

#include "Vic3DOpenGLException.h"
#include "SVViewStateHandler.h"
#include "SVGeometryView.h"
#include "SVPropVertexListWidget.h"
#include "SVMainWindow.h"

bool SVDebugApplication::notify( QObject *recv, QEvent *e ) {
	FUNCID(SVDebugApplication::notify);

	try {
		QWindow  * windowHandle = nullptr;
		if (m_mainWindow != nullptr)
			windowHandle = m_mainWindow->windowHandle();
		// ignore events send to the outer window (from window manager)
		if (recv != windowHandle) {
			if (e->type() == QEvent::KeyPress) {
//				qDebug() << "KeyPressEvent from " << recv->objectName();
				QKeyEvent * ke = dynamic_cast<QKeyEvent *>(e);
				// if we get a key event, and the currently focused widget is NOT a line edit, call the global key handler
				// function in GeometryView
				QWidget * w = focusWidget();
				if (qobject_cast<QLineEdit*>(w) == nullptr) {
					// are we in "place vertex mode" and have the vertex list property widget open?
					// if so, execute "complete polygon" if possible
					SVViewState vs = SVViewStateHandler::instance().viewState();
					if (vs.m_propertyWidgetMode == SVViewState::PM_VertexList) {
						if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
							if (SVViewStateHandler::instance().m_propVertexListWidget->completePolygonIfPossible())
								return true;
						}
					}
					// maybe a key for the tool bar or snap/lock buttons? Or a scene navigation key?
					if (SVViewStateHandler::instance().m_geometryView != nullptr) {
						if (SVViewStateHandler::instance().m_geometryView->handleGlobalKeyPressEvent(ke))
							return true;
					}
				}
			}
			else if (e->type() == QEvent::KeyRelease) {
				QWidget * w = focusWidget();
				if (qobject_cast<QLineEdit*>(w) == nullptr) {
	//			qDebug() << "KeyReleaseEvent from " << recv->objectName();
					QKeyEvent * ke = dynamic_cast<QKeyEvent *>(e);
					if (SVViewStateHandler::instance().m_geometryView != nullptr) {
						if (SVViewStateHandler::instance().m_geometryView->handleGlobalKeyRelease(ke))
							return true;
					}
				}
			}
		}
		return QApplication::notify( recv, e );
	}
	catch (Vic3D::OpenGLException &ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("Vic3D::OpenGLException caught.", IBK::MSG_ERROR, FUNC_ID);
	}
	catch (IBK::Exception &ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("IBK::Exception caught.", IBK::MSG_ERROR, FUNC_ID);
	}
	catch (std::exception &ex) {
		IBK::IBK_Message( ex.what(), IBK::MSG_ERROR, FUNC_ID);
		IBK::IBK_Message( "std::exception.", IBK::MSG_ERROR, FUNC_ID);
	}

	return false;
}

