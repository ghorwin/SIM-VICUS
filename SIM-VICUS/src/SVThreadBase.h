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

#ifndef SVThreadBaseH
#define SVThreadBaseH

#include <QThread>

/*! Base class for all worked threads in the user interface.

	In addition to re-implementing QThread::run() you should also
	implement stop() such, that a call to this function from
	another thread will make the thread return from run() very
	very shortly afterwards. A good approach is to set a flag within
	stop and poll this flag repeatedly within run().

	Each thread implementation works in its own memory space. It is forbidden
	to access GUI/project memory during thread execution. Instead, implement
	fetch() (which is called from the GUI thread just before run() is triggered)
	and copy all relevant data to your thread object. Then, once run() is completed
	successfully your implementation of store() is called (also from the GUI thread)
	which allows you to copy all results of your threaded operation into the
	main applications memory.
*/
class SVThreadBase : public QThread {
	Q_OBJECT
public:

	/*! Adds thread to global thread queue and starts processing the thread. */
	void addThread();

	/*! Re-implement in derived classes to halt the thread quickly. */
	virtual void stop() = 0;

	/*! Re-implement in derived classes to pull data from singleton classes into thread object.
		This function is called from the main GUI thread. */
	virtual void fetch() {}

	/*! Re-implement in derived classes to store data from thread object into singleton data classes.
		This function is called from the main GUI thread. */
	virtual void store() {}

	/*! Triggered from a worker thread in regular intervals.
		Note that this function is actually executed within the thread.
		\param totalItems	The total number of items to process. This value will
							be used as max value of the progress bar with the special
							case of 0, which means the progress bar is to be left untouched.
							The special case of -1 means, the message is to be interpreted as
							progress caption.
		\param currentItem	The current item being processed, must not be larger than totalItems.
		\param msg			An optional message string to be displayed.
	*/
	void notifyProgress(int totalItems, int currentItem, QString msg);

signals:
	/*! Emitted from notifyProgress, which is called from derived classes.
		Usually, this signal is connected to the north panel in the main window.
	*/
	void progressCaptionChanged(const QString & msg);

	/*! Emitted from notifyProgress, which is called from derived classes.
		Usually, this signal is connected to the north panel in the main window.
	*/
	void progressInfoChanged(int totalItems, int currentItem, const QString & msg);
};

#endif // SVThreadBaseH
