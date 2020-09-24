#include "SVThreadBase.h"

#include "SVMainWindow.h"

void SVThreadBase::notifyProgress(int totalItems, int currentItem, QString msg) {
	if (totalItems == -1) {
		emit progressCaptionChanged(msg);
	}
	else {
		emit progressInfoChanged(totalItems, currentItem, msg);
	}
}


void SVThreadBase::addThread() {
	// this call transfers ownership to SVMainWindow
	SVMainWindow::instance().runWorkerThreadImpl(this);
}
