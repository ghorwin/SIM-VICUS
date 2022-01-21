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

#ifndef SVGeometryViewH
#define SVGeometryViewH

#include <QWidget>


namespace Vic3D {
	class SceneView;
}

class SVPropertyWidget;
class SVLocalCoordinateView;
class SVMeasurementWidget;
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

	/*! Resets the camera position to be looking nicely onto the scene.
		\param positionID The ID identifies the position the camera should take.

		0 - nice view
		1 - from north
		2 - from east
		3 - from south
		4 - from west
		5 - from above
		6 - close to selected object's center point

		For variants 1..4 the camera is placed at 2x max bounding box dimension apart from the geometry center at top of the geometry
		looking at the center.
	*/
	void resetCamera(int positionID);

	/*! This function is called from the main application object, when it receives a keypress event.
		It is the central handling function for all scene-related/view-state related global shortcuts
		and dispatches them to either the scene, or the line edit (for number input) or the view state
		manager.

		\note This function is not called, when the focus is currently on a line edit or any other widget
			  that legitimately accepts all character inputs.
		\return Returns true, if the key was accepted and handled.
	*/
	bool handleGlobalKeyPress(Qt::Key k);

	QPoint topLeft();

public slots:
	/*! Connected to view state handler - turns local coordinate system view on/off, depending on
		visibility of the local coordinate system.
	*/
	void onViewStateChanged();

	/*! Called when user types in numbers while placing vertexes. */
	void onNumberKeyPressed(Qt::Key k);

private slots:
	/*! Triggered when the user pressed enter while the line edit is active, or when the place vertex button
		is pressed, or when the user pressed enter while vertex placement in the scene.
	*/
	void coordinateInputFinished();

	void on_actionSnap_toggled(bool);
	void on_actionMeasure_toggled(bool);
	void on_actionXLock_toggled(bool);
	void on_actionYLock_toggled(bool);
	void on_actionZLock_toggled(bool);

private:
	void setupToolBar();

	/*! The scene view, that shows our world and allows navigation */
	Vic3D::SceneView			*m_sceneView								= nullptr;
	QWidget						*m_sceneViewContainerWidget					= nullptr;
	/*! The property widget is located to the right of the view and is layouted in a splitter. */
	SVPropertyWidget			*m_propertyWidget							= nullptr;
	/*! Splitter that contains the scene view and the property widget. */
	QSplitter					*m_splitter									= nullptr;

	QToolBar					*m_toolBar									= nullptr;

	QAction						*m_snapAction								= nullptr;
	QAction						*m_measureAction							= nullptr;
	QAction						*m_xLockAction								= nullptr;
	QAction						*m_yLockAction								= nullptr;
	QAction						*m_zLockAction								= nullptr;

	QLineEdit					*m_lineEditCoordinateInput					= nullptr;
	QAction						*m_actionCoordinateInput					= nullptr;

	/*! Local Coordinate System View Widget */
	SVLocalCoordinateView		*m_localCoordinateSystemView				= nullptr;
	QAction						*m_actionlocalCoordinateSystemCoordinates	= nullptr;
};


#endif // SVGeometryViewH
