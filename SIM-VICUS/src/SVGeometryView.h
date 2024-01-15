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

#include "Vic3DSceneView.h"

namespace Vic3D {
class SceneView;
}

class SVPropertyWidget;
class SVLocalCoordinateView;
class SVMeasurementWidget;
class SVColorLegend;
class SVSnapOptionsDialog;
class QSplitter;
class QToolBar;
class QAction;
class QLineEdit;
class QDockWidget;
class ModificationInfo;

namespace Ui {
class SVGeometryView;
}

/*! The main geometry view.
	The 3D scene view is embedded into this widget.

	The geometry view has two toolbars - one at the right where users can quickly switch between
	different property widget modes, and one at the bottom that is only shown when the
	local coordinate system is active.
*/
class SVGeometryView : public QWidget {
	Q_OBJECT
public:
	/*! C'tor. */
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
	void resetCamera(Vic3D::SceneView::CameraPosition positionID);

	/*! This function is called from the main application object, when it receives a keypress event.
		It is the central handling function for all scene-related/view-state related global shortcuts
		and dispatches them to either the scene, or the line edit (for number input) or the view state
		manager.

		\note This function is not called, when the focus is currently on a line edit or any other widget
			  that legitimately accepts all character inputs.
		\return Returns true, if the key was accepted and handled.
	*/
	bool handleGlobalKeyPressEvent(QKeyEvent * ke);

	bool handleGlobalKeyRelease(QKeyEvent * ke);

	/*! Moves the measurement Widget to the bottom right of the scene view. */
	void moveTransparentSceneWidgets();

	/*! Hides measurement widget and untoggles the button. */
	void hideMeasurementWidget();

	void switch2AddGeometry();

	void switch2BuildingParametrization();

	void switch2NetworkParametrization();

	/*! Sets all actions in button bar to unchecked state */
	void uncheckAllActionsInButtonBar();

	SVColorLegend * colorLegend();

	/*! This set stores all parent widgets that may have focus themselves or their children in order to
		receive navigation key events for the scene.
		Usually this is the geometryview itself, and the log dock widget and the navigation panel.
		Pointers are in inserted by the individual classes in their constructors.
	*/
	QSet<const QWidget*>	m_focusRootWidgets;

public slots:
	/*! Handles selection changes and enables/disables button states. */
	void onModified(int modificationType, ModificationInfo *);

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

	/*! Triggered when style is changed in Preferences. */
	void onStyleChanged();

	void on_actionTranslateGeometry_triggered();
	void on_actionRotateGeometry_triggered();
	void on_actionScaleGeometry_triggered();
	void on_actionAlignGeometry_triggered();
	void on_actionCopyGeometry_triggered();

	void on_actionMeasure_triggered(bool checked);
	void on_actionSnap_triggered(bool on);

	void on_actionXLock_triggered(bool checked);
	void on_actionYLock_triggered(bool checked);
	void on_actionZLock_triggered(bool checked);

	void on_actionBuildingParametrization_triggered();
	void on_actionAddGeometry_triggered();
	void on_actionNetworkParametrization_triggered();
	void on_actionSiteParametrization_triggered();
	void on_actionAcousticParametrization_triggered();
	void on_actionShowResults_triggered();

	void on_actionStructuralUnits_triggered();

protected:
	/*! Resize event adjusts the position of the measurements widget, needed when geometry view is resized
		without changing scene size (by moving left splitter).
	*/
	void resizeEvent(QResizeEvent *event);

private:
	Ui::SVGeometryView			*m_ui;

	void setupToolBar();

	/*! The scene view, that shows our world and allows navigation */
	Vic3D::SceneView			*m_sceneView								= nullptr;
	QWidget						*m_sceneViewContainerWidget					= nullptr;

	/*! Pointer to measurement widget */
	SVMeasurementWidget			*m_measurementWidget = nullptr;

	/*! Pointer to color legend widget */
	SVColorLegend				*m_colorLegend = nullptr;

	/*! Pointer to snap option widget */
	SVSnapOptionsDialog			*m_snapOptionsDialog = nullptr;

	QLineEdit					*m_lineEditCoordinateInput					= nullptr;
	QAction						*m_actionCoordinateInput					= nullptr;

	/*! Local Coordinate System View Widget */
	SVLocalCoordinateView		*m_localCoordinateSystemView				= nullptr;
	QAction						*m_actionlocalCoordinateSystemCoordinates	= nullptr;

};



#endif // SVGeometryViewH
