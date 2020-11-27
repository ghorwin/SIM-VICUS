#ifndef SVGEOMETRYVIEW_H
#define SVGEOMETRYVIEW_H

#include <QWidget>


namespace Vic3D {
	class SceneView;
}

class SVPropertyWidget;
class QSplitter;
class QPushButton;

/*! The main geometry view.
	The 3D scene view is embedded into this widget.
*/
class SVGeometryView : public QWidget {
	Q_OBJECT
public:
	/*! Different basic view modes.
		Depending on the view mode, different actions are available and
		the rendering works differently.
	*/
	enum ViewMode {
		/*! Standard mode - allows scene navigation and selection of
			elements. Rendering is only done when viewport changes or
			when something is selected.
		*/
		VM_Standard,
		VM_EditGeometry,
		NUM_VM
	};

	/*! Different operation modes within geometry mode - switching the mode will
		change the appearance of the property widget and functionality of the scene.
	*/
	enum GeometryEditMode {
		/*! A new polygon is beeing added.
			The movable local coordinate system is being shown and snaps to
			the selected snap position.
		*/
		M_AddPolygon,
		NUM_M
	};

	explicit SVGeometryView(QWidget *parent = nullptr);

	void saveScreenShot(const QString & imgFilePath);

	/*! Provides read-only access to sceneView() so that signals can be connected. */
	const Vic3D::SceneView * sceneView() const { return m_sceneView; }

	void setViewMode(ViewMode m);
	void setGeometryEditMode(GeometryEditMode m);

	/*! Shows the property widget, if it was resized to zero width. */
	void showPropertyWidget();
private:


	/*! The scene view, that shows our world and allows navigation */
	Vic3D::SceneView			*m_sceneView								= nullptr;
	/*! The property widget is located to the right of the view and is layouted in a splitter. */
	SVPropertyWidget			*m_propertyWidget							= nullptr;
	/*! Splitter that contains the scene view and the property widget. */
	QSplitter					*m_splitter									= nullptr;
	/*! This is the toggle-button, that can be used to hide/show the property widget. */
	QPushButton					*m_toggleButton								= nullptr;

};

#endif // SVGEOMETRYVIEW_H
