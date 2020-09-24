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
