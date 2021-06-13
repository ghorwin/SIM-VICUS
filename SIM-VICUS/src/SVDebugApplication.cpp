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

bool SVDebugApplication::notify( QObject *recv, QEvent *e ) {
	FUNCID(SVDebugApplication::notify);

	try {
		// if we get a key event, and the currently focused widget is not a line edit, call the global key handler
		// function in GeometryView
		if (e->type() == QEvent::KeyPress) {
			QWidget * w = focusWidget();
			if (qobject_cast<QLineEdit*>(w) == nullptr) {
//				qDebug () << "GlobalKeypressEvent - handled";
				if (SVViewStateHandler::instance().m_geometryView != nullptr) {
					QKeyEvent * ke = dynamic_cast<QKeyEvent *>(e);
					if (SVViewStateHandler::instance().m_geometryView->handleGlobalKeyPress((Qt::Key)ke->key()))
						return true;
				}
			}
//			else {
//				qDebug () << "GlobalKeypressEvent - ignored";
//			}
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

