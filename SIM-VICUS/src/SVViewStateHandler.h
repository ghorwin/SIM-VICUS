#ifndef SVViewStateHandlerH
#define SVViewStateHandlerH

#include <QObject>

#include "SVViewState.h"

namespace Vic3D {
	class NewPolygonObject;
	class CoordinateSystemObject;
}

class SVNavigationTreeWidget;
class SVPropEditGeometry;

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

	/*! Restores the last viewstate that was set before the current view state. */
	void restoreLastViewState();

	/*! Caches pointer to new polygon object, to allow direct access to object when removing vertexes.
		The pointer is set in constructor of Vic3D::NewPolygonObject, object is not owned.
		DO NOT DELETE the object or do any other crazy stuff with this pointer!
	*/
	Vic3D::NewPolygonObject				*m_newPolygonObject = nullptr;

	/*! Caches pointer to the coordinate system object, to allow direct access to object when removing vertexes.
		The pointer is set in constructor of Vic3D::CoordinateSystemObject, object is not owned.
		DO NOT DELETE the object or do any other crazy stuff with this pointer!
	*/
	Vic3D::CoordinateSystemObject		*m_coordinateSystemObject = nullptr;

	/*! Pointer to navigation tree widget - can be used to retrieve the currently selected node
		from property widgets.
	*/
	SVNavigationTreeWidget				*m_navigationTreeWidget = nullptr;

	/*! Pointer to geometry edit widget - is needed to set the absolute scale factor ( bounding box) on selection change. */
	SVPropEditGeometry					*m_propEditGeometryWidget = nullptr;

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
