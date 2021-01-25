#include "SVDebugApplication.h"

#include <IBK_Exception.h>
#include <IBK_messages.h>
#include "OpenGLException.h"

bool SVDebugApplication::notify( QObject *recv, QEvent *e ) {
	FUNCID(SVDebugApplication::notify);

	try {
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

