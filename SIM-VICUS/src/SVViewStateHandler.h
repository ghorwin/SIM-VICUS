#ifndef SVViewStateHandlerH
#define SVViewStateHandlerH

#include <QObject>

#include "SVViewState.h"

/*! This singleton makes the current UI view state available to all.
	Widgets that need to be informed from view state changes, should
	connect to stateChanged() signal.
*/
class SVViewStateHandler : public QObject {
	Q_OBJECT
public:
	SVViewStateHandler(QObject * parent = nullptr);
	~SVViewStateHandler();

	/*! Returns the instance of the singleton. */
	static SVViewStateHandler & instance();

	/*! Returns the current view state. */
	const SVViewState & viewState() const { return m_viewState; }

	/*! Sets/changes a view state.
		This function creates a backup copy of the previous view state, to be restored
		after an interleaving function.
	*/
	void setViewState(const SVViewState & newViewState);

signals:
	/*! Emitted, when the state has changed. */
	void viewStateChanged();

private:
	/*! The global pointer to the SVViewStateHandler object.
		This pointer is set in the constructor, and cleared in the destructor.
	*/
	static SVViewStateHandler			*m_self;

	/*! Contains the current view state. */
	SVViewState							m_viewState;
	/*! Contains the previous view state (so that is can be restored when an interleaving mode has ended). */
	SVViewState							m_previousViewState;
};

#endif // SVViewStateHandlerH
