#ifndef Vic3DSceneH
#define Vic3DSceneH

#include <vector>

#include <QRect>
#include <QVector3D>
#include <QCoreApplication>

#include "Vic3DCamera.h"
#include "Vic3DGridObject.h"
#include "Vic3DOpaqueGeometryObject.h"
#include "Vic3DOrbitControllerObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DWireFrameObject.h"
#include "Vic3DNewPolygonObject.h"
#include "Vic3DPickObject.h"

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
class Vic3DScene {
	Q_DECLARE_TR_FUNCTIONS(Vic3DScene)
public:

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
		\param keyboardHandler Contains information on current state of keyboard and mouse (and mouse move/scroll deltas since last call).
		\param localMousePos Contains mouse position of last left mouse button press or release.
		\return Returns true, if input causes change in view and needs repainting.
	*/
	bool inputEvent(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos, QPoint & newLocalMousePos);

	/*! Actually renders to the current OpenGL context. */
	void render();

	/*! Updates the view state based on the current operation.
		\note This function should only be called from Vic3DSceneView::setViewState()!
	*/
	void setViewState(const SVViewState & vs);

	/*! When Escape was pressed, all selected objects become un-selected again. */
	void deselectAll();
	/*! Removes all selected geometry (creates an undo-action on the selected geometry. */
	void deleteSelected();

	/*! Toggles "align coordinate system" mode on. */
	void enterCoordinateSystemAdjustmentMode();
	/*! Leaves the coordinate system alignment/positioning mode and returns to previous mode. */
	void leaveCoordinateSystemAdjustmentMode(bool abort);

private:
	void generateBuildingGeometry();
	void generateNetworkGeometry();

	/*! Mouse pick handler: collects all surfaces along the pick line and stores first intersection point's
		coordinates in m_pickPoint.
	*/
	void pick(PickObject & pickObject);

	/*! Determine which objects/planes are selected and color them accordingly.
		nearPoint and farPoint define the current ray and are given in model coordinates.
		If an object is picked, selectedNodeID indicates the objects unique ID.
	*/
	void selectNearestObject(const QVector3D & nearPoint, const QVector3D & farPoint, PickObject & pickObject);

	/*! Takes the picked objects and applies the snapping rules.
		Once a snap point has been selected, the local coordinate system is translated to the snap point.
	*/
	void snapLocalCoordinateSystem(const PickObject & pickObject);

	/*! Determines a new local mouse position (local to this viewport) such that a mouse passing over a border
		while dragging/rotating the view is avoided through placement of the mouse to the other side of the window.
	*/
	void adjustCurserDuringMouseDrag(const QPoint & mouseDelta, const QPoint & localMousePos, QPoint & newLocalMousePos);

	/*! Due something with the mouse click, depending on current operation mode. */
	void handleLeftMouseClick(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos);

	/*! Selects/deselects objects. */
	void handleSelection(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos);

	/*! Cached pointer to parent widget - needed so that we can tell a QObject-based class to send
		out signals.
	*/
	SceneView				*m_parent = nullptr;

	/*! Stores viewport geometry. */
	QRect					m_viewPort;

	/*! Shader program 'Grid' (managed by SceneView). */
	ShaderProgram			*m_gridShader				= nullptr;
	/*! Shader program 'Opaque Surfaces' (managed by SceneView). */
	ShaderProgram			*m_buildingShader			= nullptr;
	/*! Shader program 'Orbit controller' (managed by SceneView). */
	ShaderProgram			*m_fixedColorTransformShader	= nullptr;
	/*! Shader program 'Coordinate system' (managed by SceneView). */
	ShaderProgram			*m_coordinateSystemShader	= nullptr;
	/*! Shader program 'Transparent surfaces' (managed by SceneView). */
	ShaderProgram			*m_transparencyShader		= nullptr;

	/*! The projection matrix, updated whenever the viewport geometry changes (in resizeGL() ). */
	QMatrix4x4				m_projection;
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
	/*! A geometry drawing object (no transparency) for building (room) surfaces.*/
	OpaqueGeometryObject	m_opaqueGeometryObject;
	/*! A geometry drawing object (no transparency) for network elements.*/
	OpaqueGeometryObject	m_networkGeometryObject;
	/*! A geometry for drawing selected primitives with overlayed wireframe. */
	WireFrameObject			m_selectedGeometryObject;
	/*! Indicator for the center of the orbit controller.
		Only visible when m_orbitControllerActive is true.
	*/
	OrbitControllerObject	m_orbitControllerObject;
	/*! The movable coordinate system. */
	CoordinateSystemObject	m_coordinateSystemObject;
	/*! Object to display newly drawn geometry. */
	NewPolygonObject		m_newPolygonObject;


	// *** Orbit controller stuff ***

	/*! Stores the distance that the mouse has been moved in the last "Left-mouse button down ... release" interval. */
	float					m_mouseMoveDistance = 0.f;
	/*! If true, the orbit controller is active and the rotation axis is to be shown. */
	bool					m_orbitControllerActive = false;
	/*! Holds the origin of the orbit controller coordinates. */
	QVector3D				m_orbitControllerOrigin;

	// other members

	/*! Cached "old" transform of the coordinate system object (needed for "align coordinate system" operation). */
	Transform3D				m_oldCoordinateSystemTransform;

};

} // namespace Vic3D


#endif // Vic3DSceneH
