#ifndef SVGeometryViewH
#define SVGeometryViewH

#include <QWidget>


namespace Vic3D {
	class SceneView;
}

class SVPropertyWidget;
class SVLocalCoordinateView;
class QSplitter;
class QToolBar;
class QAction;
class QLineEdit;
class QDockWidget;

/*! The main geometry view.
	The 3D scene view is embedded into this widget.
*/
class SVGeometryView : public QWidget {
	Q_OBJECT
public:

	explicit SVGeometryView(QWidget *parent = nullptr);

	void saveScreenShot(const QString & imgFilePath);

	/*! Provides read-only access to sceneView() so that signals can be connected. */
	const Vic3D::SceneView * sceneView() const { return m_sceneView; }

	/*! Sets the focus to the scene view, so that keyboard input is received by scene. */
	void focusSceneView();

	/*! Triggers a repaint of the scene view - useful, if scene view composition changes
		outside of project data modification.
	*/
	void refreshSceneView();

public slots:
	/*! Connected to view-state handler. */
	void onViewStateChanged();

	/*! Called when user types in numbers while placing vertexes. */
	void onNumberKeyPressed(Qt::Key k);

private slots:
	/*! Triggered when the user pressed enter while the line edit is active, or when the place vertex button
		is pressed, or when the user pressed enter while vertex placement in the scene.
	*/
	void coordinateInputFinished();

	void on_actionSnap_toggled(bool);
	void on_actionXLock_toggled(bool);
	void on_actionYLock_toggled(bool);
	void on_actionZLock_toggled(bool);

private:
	void setupToolBar();

	void setupDockWidget();

	/*! The scene view, that shows our world and allows navigation */
	Vic3D::SceneView			*m_sceneView								= nullptr;
	QWidget						*m_sceneViewContainerWidget					= nullptr;
	/*! Local Coordinate System View Widget */
	QWidget						*m_localCoordinateSystemView				= nullptr;
	/*! The property widget is located to the right of the view and is layouted in a splitter. */
	SVPropertyWidget			*m_propertyWidget							= nullptr;
	/*! Splitter that contains the scene view and the property widget. */
	QSplitter					*m_splitter									= nullptr;


	QToolBar					*m_toolBar									= nullptr;

	QWidget						*m_dockWidget								= nullptr;

	QAction						*m_snapAction								= nullptr;
	QAction						*m_xLockAction								= nullptr;
	QAction						*m_yLockAction								= nullptr;
	QAction						*m_zLockAction								= nullptr;

	QLineEdit					*m_lineEditCoordinateInput					= nullptr;
	QAction						*m_actionCoordinateInput					= nullptr;

};

#endif // SVGeometryViewH
