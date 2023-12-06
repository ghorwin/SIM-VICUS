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

#ifndef Vic3DSceneH
#define Vic3DSceneH

#include <vector>

#include <QRect>
#include <QVector3D>
#include <QCoreApplication>

#include <VICUS_GridPlane.h>

#include "VICUS_Drawing.h"
#include "VICUS_RotationMatrix.h"
#include "Vic3DCamera.h"
#include "Vic3DGridObject.h"
#include "Vic3DOpaqueGeometryObject.h"
#include "Vic3DTransparentBuildingObject.h"
#include "Vic3DOrbitControllerObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DSmallCoordinateSystemObject.h"
#include "Vic3DWireFrameObject.h"
#include "Vic3DNewGeometryObject.h"
#include "Vic3DNewSubSurfaceObject.h"
#include "Vic3DSurfaceNormalsObject.h"
#include "Vic3DPickObject.h"
#include "Vic3DMeasurementObject.h"
#include "Vic3DRubberbandObject.h"

#include "SVViewState.h"
#include "SVMeasurementWidget.h"

class ModificationInfo;

namespace VICUS {
	class Surface;
}

class SVViewState;
class SVPropEditGeometry;

namespace Vic3D {

class ShaderProgram;
class KeyboardMouseHandler;
class SceneView;

/*! Encapsulates all data for drawing a part of the final picture, including the viewport where the
	scene is rendered in.
*/
class Scene {
	Q_DECLARE_TR_FUNCTIONS(Scene)
public:

	enum HighlightingMode {
		HM_TransparentWithBoxes,
		HM_ColoredSurfaces
	};

	void create(SceneView * parent, std::vector<ShaderProgram> & shaderPrograms);

	/*! Triggered when SVProjectHandler::modified() is emitted. */
	void onModified( int modificationType, ModificationInfo * data );

	void destroy();

	void resize(int width, int height, qreal retinaScale);

	/*! Returns current size. */
	QSize currentSize() const { return m_viewPort.size(); }

	/*! Compines camera matrix and project matrix to form the world2view matrix. */
	void updateWorld2ViewMatrix();

	/*! Lets the scene handle keyboard/mouse input.
		Note: to retrieve the last mouse down/release positions, query the keyboardHandler directly.
		\param keyboardHandler Contains information on current state of keyboard and mouse (and mouse move/scroll deltas since last call).
		\param localMousePos Contains current mouse position (QCursor::pos() converted to local widget coordinates/viewport coordinates).
		\return Returns true, if input causes change in view and needs repainting.
	*/
	bool inputEvent(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos, QPoint & newLocalMousePos);


	/*! Actually renders to the current OpenGL context. */
	void render();

	/*! Updates the view state based on the current operation, which basically means that
		the coloring is being updated.
		\note This function should only be called from Vic3DSceneView::setViewState()!
	*/
	void setViewState(const SVViewState & vs);

	/*! This function can be called to specifically update the coloring and/or the network geometry.
		This function is meant to be called whenever the database element's colors have been changed,
		or in the case of network components, also their geometry-size related properties.
	*/
	void refreshColors();

	/*! Select all objects like surfaces, rooms, buildings, etc. */
	void selectAll();
	/*! When Escape was pressed, all selected objects become un-selected again. */
	void deselectAll();
	/*! Removes all selected geometry (creates an undo-action on the selected geometry). */
	void deleteSelected();
	/*! Makes all selected geometry. */
	void showSelected();
	/*! Shows all selected geometry. */
	void hideSelected();

	/*! Leave any special operation mode ("snap"-mode, "place local coordinate system"-mode, ...).
		This function is called when switching from one mode to the next.
	*/
	void leaveAnySpecialMode();
	/*! Toggles "align coordinate system" mode on. */
	void enterCoordinateSystemAdjustmentMode();
	/*! Leaves the coordinate system alignment mode and returns to previous mode. */
	void leaveCoordinateSystemAdjustmentMode(bool abort);

	/*! Toggles "translate coordinate system" mode on. */
	void enterCoordinateSystemTranslationMode();
	/*! Leaves the coordinate system positioning mode and returns to previous mode. */
	void leaveCoordinateSystemTranslationMode(bool abort);

	/*! Toggles "measurement" mode on. */
	void enterMeasurementMode();
	/*! Leaves the "measurement" mode and returns to previous mode. */
	void leaveMeasurementMode();

	bool m_smallCoordinateSystemObjectVisible = true;
	/*! If true, the surface normals (lines) are shown for each visible surface. */
	bool m_surfaceNormalsVisible = false;

	/*! Getter for Camera. */
	const Camera & camera() const;
	/*! Getter for worldToView Matrix. */
	const QMatrix4x4 & worldToView() const;

	void updatedHighlightingMode(HighlightingMode mode);

private:
	void generateBuildingGeometry();
	void generateTransparentBuildingGeometry(const HighlightingMode &mode = HighlightingMode::HM_TransparentWithBoxes);
	void generateNetworkGeometry();

	void generate2DDrawingGeometry();

	/*! Processes all surfaces and assigns colors based on current object color mode. */
	void recolorObjects(SVViewState::ObjectColorMode ocm, unsigned int id) const;

	/*! Mouse pick handler: collects all pickable objects/surfaces/planes along the line-of-sight and stores all possible
		pick candidates in pickObject.
		\note There is always at least one pick candidate in the list of intersection candidates, which is the intersection with the far plane.
	*/
	void pick(PickObject & pickObject);

	/*! Pick drawing points. */
	void pickDrawings(PickObject & pickObject, const IBKMK::Vector3D &nearPoint,
					  const IBKMK::Vector3D &farPoint, const IBKMK::Vector3D &direction);

	/*! Pick all block depening drawing points. */
	void pickBlockDrawings(PickObject &pickObject, const VICUS::Drawing &d, const std::vector<VICUS::Drawing::Insert> &inserts,
						   const IBKMK::Vector3D &nearPoint, const IBKMK::Vector3D &direction);

	/*! Takes the picked objects and applies the snapping rules.
		Once a snap point has been selected, the local coordinate system is translated to the snap point.

		Snap-to-axis requires an offset point and this depends on the current operation:

		- when drawing, we use the last placed vertex - or the origin, if no vertex has been placed
		- in interactive translation or scaling mode, we use the original coordinate system location when
		  the interactive operation started (stored in m_translateOrigin).
	*/
	void snapLocalCoordinateSystem(const PickObject & pickObject);

	/*! Determines a new local mouse position (local to this viewport) such that a mouse passing over a border
		while dragging/rotating the view is avoided through placement of the mouse to the other side of the window.
	*/
	void adjustCursorDuringMouseDrag(const QPoint & mouseDelta, const QPoint & localMousePos,
									 QPoint & newLocalMousePos, PickObject & pickObject);

	/*! Due something with the mouse click, depending on current operation mode. */
	void handleLeftMouseClick(const KeyboardMouseHandler & keyboardHandler, PickObject & o);

	/*! Selects/deselects objects. */
	void handleSelection(const KeyboardMouseHandler & keyboardHandler, PickObject & o);

	/*! . */
	IBKMK::Vector3D calculateFarPoint(const QPoint & mousPos, const QMatrix4x4 & projectionMatrixInverted);

	/*! Initializes the pan operation. */
	void panStart(const QPoint & localMousePos, PickObject & pickObject, bool reuseDepth);

	/*! Sets a meaningful view state based on current's widget appearance, and whether we have selection or not.
		This function is called when we turn off any intermediate modes, like "align coordinate system"-mode.
	*/
	void setDefaultViewState();

	/*! Cached pointer to parent widget - needed so that we can tell a QObject-based class to send
		out signals.
	*/
	SceneView				*m_parent = nullptr;

	/*! Stores viewport geometry. */
	QRect					m_viewPort;
	/*! Stores minature viewport geometry. */
	QRect					m_smallViewPort;

	/*! Shader program 'Grid' (managed by SceneView). */
	ShaderProgram			*m_gridShader				= nullptr;
	/*! Shader program 'SurfaceNormals' (managed by SceneView). */
	ShaderProgram			*m_surfaceNormalsShader		= nullptr;
	/*! Shader program 'Measurement Line' (managed by SceneView). */
	ShaderProgram			*m_measurementShader		= nullptr;
	/*! Shader program 'Opaque Surfaces' (managed by SceneView). */
	ShaderProgram			*m_buildingShader			= nullptr;
	/*! Shader program 'Orbit controller' (managed by SceneView). */
	ShaderProgram			*m_fixedColorTransformShader	= nullptr;
	/*! Shader program 'Coordinate system' (managed by SceneView). */
	ShaderProgram			*m_coordinateSystemShader	= nullptr;
	/*! Shader program 'Transparent surfaces' (managed by SceneView). */
	ShaderProgram			*m_transparencyShader		= nullptr;
	/*! Shader program 'Rubberband' (managed by SceneView). */
	ShaderProgram			*m_rubberbandShader			= nullptr;

	/*! The projection matrix, updated whenever the viewport geometry changes (in resizeGL() ). */
	QMatrix4x4				m_projection;
	/*! The projection matrix for the small view (the view is a constant square, so this project matrix does not change over time). */
	QMatrix4x4				m_smallViewProjection;
	/*! World transformation matrix generator. */
	Transform3D				m_transform;
	/*! Camera position, orientation and lens data. */
	Camera					m_camera;
	/*! Cached world to view transformation matrix. */
	QMatrix4x4				m_worldToView;

	/*! Position of light source, currently very far above. */
	QVector3D				m_lightPos = QVector3D(100,200,2000);
	/*! Light color. */
	QColor					m_lightColor = Qt::white;

	// *** Drawable objects ***

	/*! The grid draw object. */
	GridObject				m_gridObject;
	/*! A geometry drawing object (transparency only for windows) for building (room) surfaces.*/
	OpaqueGeometryObject	m_buildingGeometryObject;
	/*! todo */
	OpaqueGeometryObject	m_drawingGeometryObject;
	/*! A geometry drawing object (no transparency) for network elements.*/
	OpaqueGeometryObject	m_networkGeometryObject;
	/*! A geometry drawing object for building (room) surfaces.*/
	TransparentBuildingObject	m_transparentBuildingObject;
	/*! A geometry for drawing selected primitives with overlayed wireframe. */
	WireFrameObject			m_selectedGeometryObject;
	/*! Indicator for the center of the orbit controller.
		Only visible when m_orbitControllerActive is true.
	*/
	OrbitControllerObject	m_orbitControllerObject;
	/*! The movable coordinate system. */
	CoordinateSystemObject	m_coordinateSystemObject;
	/*! Object to display newly drawn geometry. */
	NewGeometryObject		m_newGeometryObject;
	/*! Object to display new subsurface geometry. */
	NewSubSurfaceObject		m_newSubSurfaceObject;
	/*! The measurement object. */
	MeasurementObject		m_measurementObject;

	/*! Pointer to measurement widget */
	SVMeasurementWidget		*m_measurementWidget = nullptr;

	/*! The small coordinate system at the bottom/left. */
	SmallCoordinateSystemObject	m_smallCoordinateSystemObject;

	/*! The rubberband object. */
	RubberbandObject		m_rubberbandObject;

	/*! The surface normals (lines) shown for visible surfaces when enabled. */
	SurfaceNormalsObject	m_surfaceNormalsObject;

	/*! Object that shows snap-locations when doing rotation operations. */

	/// TODO Andreas, add vector of snap marker objects, stripped down coordinate system objects with just a single sphere.
	OpaqueGeometryObject	m_rotationMarkerObject;

	/*! Cached surface colors. */
	std::map<unsigned int, QColor> m_surfaceColor;

	// *** Navigation stuff ***

	/*! Struct for exclusive navigation modes.
		While one navigation mode is active, we cannot also start another navigation mode.
	*/
	enum NavigationMode {
		NM_OrbitController,
		NM_Panning,
		NM_FirstPerson,
		NM_InteractiveTranslation,
		NM_InteractiveRotation, // this is set for any axis rotation - which rotation is rotated about is set in the local coordinate system TM_xx bit
		NM_InteractiveScaling, // this is set for any axis - which axis is scaled is set in the local coordinate system TM_xx bit
		NM_RubberbandSelection, // not really a navigation mode, but here the user drags a visible rectangle, hence other navigation operations are disabled
		NUM_NM
	};

	/*! Identifies the current navigation mode.
		Toggled in inputEvent() and reset when anything changes the scene operation mode (like reloading the project etc.)
	*/
	NavigationMode			m_navigationMode = NUM_NM;

	// *** Orbit controller stuff ***

	/*! Stores the distance that the mouse has been moved in the last "Left-mouse button down ... release" interval. */
	float					m_mouseMoveDistance = 0.f;
	/*! Holds the origin of the orbit controller coordinates. */
	QVector3D				m_orbitControllerOrigin;

	// *** Interactive translation/rotation/scale stuff ***

	/*! Reference vector (coordinate system will be rotated around this vector). */
	IBKMK::Vector3D			m_rotationAxis;
	/*! Reference vector (local X of rotation system). "z-axis" is always rotation axis */
	IBKMK::Vector3D			m_rotationVectorX;
	/*! Reference vector (local Y of rotation system). "z-axis" is always rotation axis */
	IBKMK::Vector3D			m_rotationVectorY;
	/*! Relative distance between coordinate center and axis end marker - used as 100% distance for scaling. */
	double					m_nominalScalingDistance;

	/*! Holds coordinates of the snap marker, for interactive three-point-rotation it holds the two points
		already defined for rotation.
		Function updateSnapMarker() updates the transformation matrixes in the snap marker objects.
	*/
	std::vector<IBKMK::Vector3D> m_snapMarkerPoints;

	// other members

	/*! Cached "old" transform of the coordinate system object (needed for "align coordinate system" operation). */
	Transform3D				m_oldCoordinateSystemTransform;

	/*! Point A in "perfect panning" algorithm. */
	IBKMK::Vector3D			m_panCameraStart;
	/*! Point B in "perfect panning" algorithm. */
	IBKMK::Vector3D			m_panFarPointStart;
	/*! Point E in "perfect panning" algorithm. */
	IBKMK::Vector3D			m_panObjectStart;
	/*! Mouse position (local coordinate system) when starting move. */
	QPoint					m_panMousePos;

	QMatrix4x4				m_panOriginalTransformMatrix;

	double					m_panCABARatio;
	/*! Cached distance of object when staring panning. */
	double					m_panObjectDepth = 0.01;

	/*! Cached last scene coloring mode. Updated in onModified() and setViewState(). Used
		to prevent excessive updates of geometry when unrelated view state properties change.
	*/
	SVViewState::ObjectColorMode	m_lastColorMode = SVViewState::OCM_None;
	/*! INVALID_ID means no color filter. */
	unsigned int					m_lastColorObjectID = VICUS::INVALID_ID;
};

} // namespace Vic3D


#endif // Vic3DSceneH
