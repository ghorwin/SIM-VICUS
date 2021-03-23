#include "SVDebugApplication.h"

#include <QLineEdit>
#include <QDebug>
#include <QKeyEvent>

#include <IBK_Exception.h>
#include <IBK_messages.h>

#include "OpenGLException.h"
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
	catch (OpenGLException &ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("IBK::Exception caught.", IBK::MSG_ERROR, FUNC_ID);
		m_aboutToTerminate = true;
		/// \todo Andreas: do emergency project save - to avoid data loss
		QApplication::exit(1); // can't go on, quit here
	}
	catch (IBK::Exception &ex) {
		ex.writeMsgStackToError();
		IBK::IBK_Message("IBK::Exception caught.", IBK::MSG_ERROR, FUNC_ID);
	}
	catch (std::exception &ex) {
		IBK::IBK_Message( ex.what(), IBK::MSG_ERROR, FUNC_ID);
		IBK::IBK_Message( "IBK::Exception caught.", IBK::MSG_ERROR, FUNC_ID);
	}

	return false;
}

