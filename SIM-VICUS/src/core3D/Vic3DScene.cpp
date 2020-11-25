#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QCursor>

#include <VICUS_Project.h>
#include <VICUS_Conversions.h>
#include <VICUS_ViewSettings.h>
#include <VICUS_NetworkLine.h>
#include <VICUS_Conversions.h>

#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
#include "Vic3DPickObject.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DConstants.h"
#include "Vic3DSceneView.h"

#include "SVProjectHandler.h"

const float TRANSLATION_SPEED = 1.2f;
const float MOUSE_ROTATION_SPEED = 0.5f;


namespace Vic3D {

void Vic3DScene::create(SceneView * parent, std::vector<ShaderProgram> & shaderPrograms) {
	m_parent = parent;
	m_gridShader = &shaderPrograms[SHADER_GRID];
	m_buildingShader = &shaderPrograms[SHADER_OPAQUE_GEOMETRY];
	m_orbitControllerShader = &shaderPrograms[SHADER_LINES];
	m_coordinateSystemShader = &shaderPrograms[SHADER_COORDINATE_SYSTEM];
	m_transparencyShader = &shaderPrograms[SHADER_TRANSPARENT_GEOMETRY];

	// the orbit controller object is static in geometry, so it can be created already here
	m_orbitControllerObject.create(m_orbitControllerShader);

	// same for the coordinate system object
	m_coordinateSystemObject.create(m_coordinateSystemShader);

	m_newPolygonObject.create(m_transparencyShader->shaderProgram(), &m_coordinateSystemObject);
}


void Vic3DScene::onModified(int modificationType, ModificationInfo * data) {

	// no shader - not initialized yet, skip

	if (m_gridShader == nullptr)
		return;

	bool updateGrid = false;
	bool updateNetwork = false;
	bool updateBuilding = false;
	// filter out all modification types that we handle
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		case SVProjectHandler::AllModified :
			updateGrid = true;
			updateBuilding = true;
			updateNetwork = true;
			// clear new polygon drawing object
			/// \todo define what state the scene should go into, when project is reloaded/newly created
			m_newPolygonObject.m_planeGeometry.m_vertexes.clear();
			m_newPolygonObject.updateBuffers();
			break;

		case SVProjectHandler::GridModified :
			updateGrid = true;
			break;

		case SVProjectHandler::NetworkModified :
			updateNetwork = true;
			break;

		default:
			return; // do nothing by default
	}

	// *** initialize camera placement and model placement in the world ***

	QVector3D cameraTrans = VICUS::IBKVector2QVector(SVProjectHandler::instance().viewSettings().m_cameraTranslation);
	m_camera.setTranslation(cameraTrans);
	m_camera.setRotation( SVProjectHandler::instance().viewSettings().m_cameraRotation.toQuaternion() );

	// re-create grid with updated properties
	// since grid object is very small, this function also regenerates the grid line buffers and
	// uploads the data to the GPU
	if (updateGrid)
		m_gridObject.create(m_gridShader);

	// create geometry object (if already existing, nothing happens here)
	if (updateBuilding) {
		m_opaqueGeometryObject.create(m_buildingShader->shaderProgram());

		// transfer data from building geometry to vertex array caches
		generateBuildingGeometry();
	}

	// create network
	if (updateNetwork) {
		// we use the same shader as for building elements
		m_networkGeometryObject.create(m_buildingShader->shaderProgram());

		// transfer data from building geometry to vertex array caches
		generateNetworkGeometry();
	}

	// update all GPU buffers (transfer cached data to GPU)
	m_opaqueGeometryObject.updateBuffers();
	m_networkGeometryObject.updateBuffers();

	// transfer other properties



	// for now put the scene automatically into draw mode
	setOperationMode(OM_Draw);
}


void Vic3DScene::destroy() {
	m_gridObject.destroy();
	m_orbitControllerObject.destroy();
	m_opaqueGeometryObject.destroy();
	m_networkGeometryObject.destroy();
	m_coordinateSystemObject.destroy();
	m_newPolygonObject.destroy();
}


void Vic3DScene::resize(int width, int height, qreal retinaScale) {
	float farDistance = 1000;
	// retrieve far viewing distance from project, if one exists
	if (SVProjectHandler::instance().isValid())
		farDistance = SVProjectHandler::instance().viewSettings().m_farDistance;
	// the projection matrix need to be updated only for window size changes
	m_projection.setToIdentity();
	// create projection matrix, i.e. camera lens
	m_projection.perspective(
				/* vertical angle */ 45.0f,
				/* aspect ratio */   width / float(height),
				/* near */           0.1f,
				/* far */            farDistance
		);
	// Mind: to not use 0.0 for near plane, otherwise depth buffering and depth testing won't work!

	// update cached world2view matrix
	updateWorld2ViewMatrix();

	// update viewport
	m_viewPort = QRect(0, 0, static_cast<int>(width * retinaScale), static_cast<int>(height * retinaScale) );
}


void Vic3DScene::updateWorld2ViewMatrix() {
	// transformation steps:
	//   model space -> transform -> world space
	//   world space -> camera/eye -> camera view
	//   camera view -> projection -> normalized device coordinates (NDC)
	m_worldToView = m_projection * m_camera.toMatrix(); // * m_transform.toMatrix();
}


void Vic3DScene::inputEvent(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos, QPoint & newLocalMousePos) {

	newLocalMousePos = localMousePos;

	/*! Mark the pick-object location as outdated. */
	m_pickObjectIsOutdated = true;

	// we implement the following controls

	// keyboard: translation and rotation works always

	// Handle translations
	QVector3D translation;
	QVector3D rotationAxis;
	if (keyboardHandler.keyDown(Qt::Key_W)) 		translation += m_camera.forward();
	if (keyboardHandler.keyDown(Qt::Key_S)) 		translation -= m_camera.forward();
	if (keyboardHandler.keyDown(Qt::Key_A)) 		translation -= m_camera.right();
	if (keyboardHandler.keyDown(Qt::Key_D)) 		translation += m_camera.right();
	if (keyboardHandler.keyDown(Qt::Key_F)) 		translation -= m_camera.up();
	if (keyboardHandler.keyDown(Qt::Key_R))			translation += m_camera.up();
	if (keyboardHandler.keyDown(Qt::Key_Q))			rotationAxis = QVector3D(.0f,.0f,1.f);
	if (keyboardHandler.keyDown(Qt::Key_E))			rotationAxis = -QVector3D(.0f,.0f,1.f);

	float transSpeed = TRANSLATION_SPEED;
	if (keyboardHandler.keyDown(Qt::Key_Shift))
		transSpeed = 0.1f;
	m_camera.translate(transSpeed * translation);
	m_camera.rotate(transSpeed, rotationAxis);

	// retrieve mouse delta
	QPoint mouseDelta = keyboardHandler.mouseDelta(QCursor::pos());
	int mouse_dx = mouseDelta.x();
	int mouse_dy = mouseDelta.y();

	// if right mouse button is pressed, mouse delta is translated into first camera perspective rotations
	if (keyboardHandler.buttonDown(Qt::RightButton)) {
		// get and reset mouse delta (pass current mouse cursor position)
		const QVector3D LocalUp(0.0f, 0.0f, 1.0f); // same as in Camera::up()
		m_camera.rotate(-MOUSE_ROTATION_SPEED * mouse_dx, LocalUp);
		m_camera.rotate(-MOUSE_ROTATION_SPEED * mouse_dy, m_camera.right());
		// cursor wrap adjustment
		adjustCurserDuringMouseDrag(mouseDelta, localMousePos, newLocalMousePos);
	}

	// middle mouse button moves the geometry
	else if (keyboardHandler.buttonDown(Qt::MidButton)) {
		if (mouse_dx != 0)
			m_camera.translate(transSpeed * (mouse_dx < 0 ? 1 : -1) * m_camera.right());
		if (mouse_dy != 0)
			m_camera.translate(transSpeed * (mouse_dy > 0 ? 1 : -1) * m_camera.up());
		// cursor wrap adjustment
		adjustCurserDuringMouseDrag(mouseDelta, localMousePos, newLocalMousePos);
	}

	else if (keyboardHandler.buttonDown(Qt::LeftButton)) {
		// detect "enter orbital controller mode" switch
		if (!m_orbitControllerActive) {
			// we enter orbital controller mode

			// pick the rotation object
			pick(localMousePos);
			// and store pick point
			m_orbitControllerOrigin = m_pickPoint;
//			qDebug() << "Entering orbit controller mode, rotation around" << m_orbitControllerOrigin;
			m_orbitControllerObject.m_transform.setTranslation(m_orbitControllerOrigin);

			// Rotation matrix around origin point
			m_mouseMoveDistance = true;

			m_orbitControllerActive = true;
		}
		else {

			if (mouseDelta != QPoint(0,0)) {
				// vector from pick point (center of orbit) to camera position
				QVector3D lineOfSight = m_camera.translation() - m_orbitControllerOrigin;

				// create a transformation object
				Transform3D orbitTrans;

				// mouse x translation = rotation around rotation axis

				const QVector3D GlobalUpwardsVector(0.0f, 0.0f, 1.0f);
				// set rotation around z axis for x-mouse-delta
				orbitTrans.rotate(MOUSE_ROTATION_SPEED * mouse_dx, GlobalUpwardsVector);


				// mouse y translation = rotation around "right" axis

				QVector3D LocalRight = m_camera.right().normalized();
				// set rotation around "right" axis for y-mouse-delta
				orbitTrans.rotate(MOUSE_ROTATION_SPEED * mouse_dy, LocalRight);

				// rotate vector to camera
				lineOfSight = orbitTrans.toMatrix() * lineOfSight;

				// rotate the camera around the same angles
				m_camera.rotate(MOUSE_ROTATION_SPEED * mouse_dx, GlobalUpwardsVector);
				m_camera.rotate(MOUSE_ROTATION_SPEED * mouse_dy, LocalRight);

#if 1
				// fix "roll" error due to rounding
				// only do this when we are not viewing the scene from vertically from above/below
				double cosViewAngle = QVector3D::dotProduct(m_camera.forward(), GlobalUpwardsVector);
				if (std::fabs(cosViewAngle) < 0.3) {
					// up and forward vectors should be always in a vertical plane
					// forward and z-axis form a vertical plane with normal
					QVector3D verticalPlaneNormal = QVector3D::crossProduct(m_camera.forward(), GlobalUpwardsVector);
					verticalPlaneNormal.normalize();

					// the camera right angle should always match this normal vector
					double cosBeta = QVector3D::dotProduct(verticalPlaneNormal, m_camera.right().normalized());
					if (cosBeta > -1 && cosBeta < 1) {
						double beta = std::acos(cosBeta)/3.14159265*180;
						// which direction to rotate?
						m_camera.rotate(beta, m_camera.forward());
						double cosBeta2 = QVector3D::dotProduct(verticalPlaneNormal, m_camera.right().normalized());
						if (std::fabs(std::fabs(cosBeta2) - 1) > 1e-5)
							m_camera.rotate(-2*beta, m_camera.forward());
//						cosBeta2 = QVector3D::dotProduct(verticalPlaneNormal, m_camera.right().normalized());
					}
				}
#endif
				// get new camera location
				QVector3D newCamPos = m_orbitControllerOrigin + lineOfSight;
				//					qDebug() << "Moving camera from " << m_camera.translation() << "to" << newCamPos;

				// record the distance that the mouse was moved
				m_mouseMoveDistance += mouse_dx*mouse_dx + mouse_dy*mouse_dy;
				// move camera
				m_camera.setTranslation(newCamPos);

				// cursor wrap adjustment
				adjustCurserDuringMouseDrag(mouseDelta, localMousePos, newLocalMousePos);
			}
		}


	} // left button down

	if (keyboardHandler.buttonReleased(Qt::LeftButton)) {

		/// \todo adjust the THRESHOLD based on DPI/Screenresolution or have it as user option
		// check if the mouse was moved not very far -> we have a mouse click
		if (m_mouseMoveDistance < 5) {
			// TODO : click code
			qDebug() << "Mouse (selection) click received" << m_mouseMoveDistance;
			handleLeftMouseClick();
		}
		else {
			qDebug() << "Leaving orbit controller mode" << m_mouseMoveDistance;
		}

		// clear orbit controller flag
		m_orbitControllerActive = false;
	} // left button released

	// scroll wheel does fast zoom in/out
	int wheelDelta = keyboardHandler.wheelDelta();
	if (wheelDelta != 0) {
		float transSpeed = 8.f;
		if (keyboardHandler.keyDown(Qt::Key_Shift))
			transSpeed = 0.8f;
		m_camera.translate(wheelDelta * transSpeed * m_camera.forward());
	}

	// store camera position in view settings
	SVProjectHandler::instance().viewSettings().m_cameraTranslation = VICUS::QVector2IBKVector(m_camera.translation());
	SVProjectHandler::instance().viewSettings().m_cameraRotation = m_camera.rotation();

	updateWorld2ViewMatrix();


	/// \todo for performance reasons, we should add a flag that signals that the view was indeed changed
	///			otherwise we do a potential lengthy picking and snap operation for nothing

	// if coordinate system is active, perform picking operation and snap coordinate system to grid
	if (m_coordinateSystemActive) {
		m_pickObjectIsOutdated = true;
		// \todo adjust pick algorithm to only check for certain selections
		pick(localMousePos);
		// now determine which grid line is closest
		QVector3D closestPoint;
		if (m_gridObject.closestSnapPoint(m_pickPoint, closestPoint)) {
			// this is in world coordinates, use this as transformation vector for the
			// coordinate system
			m_coordinateSystemObject.m_transform.setTranslation(closestPoint);
		}
	}

	// if we are in draw mode, update the movable coordinate system's location in the new polygon object
	if (m_operationMode == OM_Draw) {
		m_newPolygonObject.updateLastVertex(m_coordinateSystemObject.m_transform.translation());
	}
}


void Vic3DScene::render() {
	glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

	// set the background color = clear color
	glClearColor(m_background.x(), m_background.y(), m_background.z(), 1.0f);

	QVector3D viewPos = m_camera.translation();

	// *** grid ***

	// enable depth testing, important for the grid and for the drawing order of several objects
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	m_gridShader->bind();
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[0], m_worldToView);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[2], m_background);
	m_gridObject.render();
	m_gridShader->release();


	// *** orbit controller indicator ***

	if (m_orbitControllerActive) {
		m_orbitControllerShader->bind();
		m_orbitControllerShader->shaderProgram()->setUniformValue(m_orbitControllerShader->m_uniformIDs[0], m_worldToView);
		m_orbitControllerObject.render();
		m_orbitControllerShader->release();
	}


	// *** movable coordinate system  ***

	if (m_coordinateSystemActive) {
		m_coordinateSystemShader->bind();
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[0], m_worldToView);
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[2], VICUS::QVector3DFromQColor(m_lightColor));
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[1], viewPos);
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[3], viewPos);
		m_coordinateSystemObject.render();
		m_coordinateSystemShader->release();
	}


	// *** opaque background geometry ***

	// tell OpenGL to show only faces whose normal vector points towards us

	/// \todo render dumb background geometry


	// *** opaque building geometry ***

	m_buildingShader->bind();
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[0], m_worldToView);

	// Note: you can't use a QColor here directly and pass it as uniform to a shader expecting a vec3. Qt internally
	//       passes QColor as vec4. Use the converter VICUS::QVector3DFromQColor() for that.
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[2], VICUS::QVector3DFromQColor(m_lightColor));

	// set view position -
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[1], viewPos);

//#define FIXED_LIGHT_POSITION
#ifdef FIXED_LIGHT_POSITION
	// use a fixed light position
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[1], m_lightPos);
#else
	// use view position as light position
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[3], viewPos);
#endif // FIXED_LIGHT_POSITION


	m_networkGeometryObject.render();

	m_opaqueGeometryObject.render();

	m_buildingShader->release();


	// *** transparent building geometry ***

	// tell OpenGL to show all planes
	glDisable(GL_CULL_FACE);
	// disable update of depth test but still use it
	glDepthMask (GL_FALSE);

	// *** new polygon draw object ***

	m_transparencyShader->bind();
	m_transparencyShader->shaderProgram()->setUniformValue(m_transparencyShader->m_uniformIDs[0], m_worldToView);

	m_newPolygonObject.render();

	// re-enable updating of z-buffer
	glDepthMask(GL_TRUE);
}


void Vic3DScene::setOperationMode(Vic3DScene::OperationMode m)	{
	// depending on whether we enter/leave an operation mode, we may have project modifications

	// \todo implement

	m_operationMode = m;
}


void Vic3DScene::generateBuildingGeometry() {
	// get VICUS project data
	const VICUS::Project & p = project();

	// we rebuild the entire geometry here, so this may be slow

	// clear out existing cache

	m_opaqueGeometryObject.m_vertexBufferData.clear();
	m_opaqueGeometryObject.m_colorBufferData.clear();
	m_opaqueGeometryObject.m_indexBufferData.clear();

	m_opaqueGeometryObject.m_vertexBufferData.reserve(100000);
	m_opaqueGeometryObject.m_colorBufferData.reserve(100000);
	m_opaqueGeometryObject.m_indexBufferData.reserve(100000);

	// we now process all surfaces and add their coordinates and
	// normals

	// we also store colors for each surface: hereby, we
	// use the current highlighting-filter object, which relates
	// object properties to colors

	// recursively process all buildings, building levels etc.

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				for (const VICUS::Surface & s : r.m_surfaces) {

					// now we store the surface data into the vertex/color and index buffers
					// the indexes are advanced and the buffers enlarged as needed.
					addSurface(s, currentVertexIndex, currentElementIndex,
							   m_opaqueGeometryObject.m_vertexBufferData,
							   m_opaqueGeometryObject.m_colorBufferData,
							   m_opaqueGeometryObject.m_indexBufferData);
				}
			}
		}
	}
}


void Vic3DScene::generateNetworkGeometry() {
	// get VICUS project data
	const VICUS::Project & p = project();

	// we rebuild the entire geometry here, so this may be slow

	// clear out existing cache

	m_networkGeometryObject.m_vertexBufferData.clear();
	m_networkGeometryObject.m_colorBufferData.clear();
	m_networkGeometryObject.m_indexBufferData.clear();

	m_networkGeometryObject.m_vertexBufferData.reserve(100000);
	m_networkGeometryObject.m_colorBufferData.reserve(100000);
	m_networkGeometryObject.m_indexBufferData.reserve(100000);

	// process all network elements

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	// add cylinders for all pipes
	for (const VICUS::Network & n : p.m_networks) {
		for (const VICUS::NetworkEdge & e : n.m_edges) {
			double radius = 0.5; //e.m_diameterOutside*5; // enlarge diameter, so that we see something
			addCylinder(e.m_node1->m_position, e.m_node2->m_position, Qt::red,
						radius,
						currentVertexIndex, currentElementIndex,
						m_networkGeometryObject.m_vertexBufferData,
						m_networkGeometryObject.m_colorBufferData,
						m_networkGeometryObject.m_indexBufferData);
		}
		for (const VICUS::NetworkNode & no : n.m_nodes) {
			double radius = 0.8; //e.m_diameterOutside*5; // enlarge diameter, so that we see something
			addSphere(no.m_position, Qt::cyan,
						radius,
						currentVertexIndex, currentElementIndex,
						m_networkGeometryObject.m_vertexBufferData,
						m_networkGeometryObject.m_colorBufferData,
						m_networkGeometryObject.m_indexBufferData);
		}
	}



	addSphere(IBKMK::Vector3D(10,0,0), Qt::red,
				0.5,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);

	addSphere(IBKMK::Vector3D(8,2,0), Qt::green,
				0.5,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);
	addSphere(IBKMK::Vector3D(8,-2,0), Qt::blue,
				0.5,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);
	addSphere(IBKMK::Vector3D(6,-4,0), Qt::cyan,
				0.5,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);
	addSphere(IBKMK::Vector3D(6,0,0), Qt::yellow,
				0.5,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);
	addSphere(IBKMK::Vector3D(6,4,0), Qt::magenta,
				0.5,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);
#if 0
	// manually add a cylinder here
	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(10,0,0), Qt::red,
				0.5,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);

	// and another one moved
	addCylinder(IBKMK::Vector3D(5,0,5), IBKMK::Vector3D(10,0,5), Qt::blue,
				1,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);

	// and another one rotated
	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,10,15), Qt::green,
				0.1,
				currentVertexIndex, currentElementIndex,
				m_networkGeometryObject.m_vertexBufferData,
				m_networkGeometryObject.m_colorBufferData,
				m_networkGeometryObject.m_indexBufferData);
#endif
}


void Vic3DScene::pick(const QPoint & localMousePos) {
	// only update if not already up-to-date
	if (!m_pickObjectIsOutdated)
		return;

	// local mouse coordinates
	int my = localMousePos.y();
	int mx = localMousePos.x();

	// viewport dimensions
	qreal halfVpw = m_viewPort.width()/2;
	qreal halfVph = m_viewPort.height()/2;

	// invert world2view matrix, with m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
	bool invertible;
	QMatrix4x4 projectionMatrixInverted = m_worldToView.inverted(&invertible);
	if (!invertible) {
		qWarning()<< "Cannot invert projection matrix.";
		return;
	}

	// mouse position in NDC space, one point on near plane and one point on far plane
	QVector4D nearPos(
				(mx - halfVpw) / halfVpw,
				-1*(my - halfVph) / halfVph,
				-1,
				1.0);

	QVector4D farPos(
				nearPos.x(),
				nearPos.y(),
				1,
				1.0);

	// transform from NDC to model coordinates
	QVector4D nearResult = projectionMatrixInverted*nearPos;
	QVector4D farResult = projectionMatrixInverted*farPos;
	// don't forget normalization!
	nearResult /= nearResult.w();
	farResult /= farResult.w();

	// now do the actual picking - for now we implement a selection
	selectNearestObject(nearResult.toVector3D(), farResult.toVector3D());

	m_pickObjectIsOutdated = false;
}


void Vic3DScene::selectNearestObject(const QVector3D & nearPoint, const QVector3D & farPoint) {
//	QElapsedTimer pickTimer;
//	pickTimer.start();

	// compute view direction (vector for line equation)
	QVector3D d = farPoint - nearPoint;
	IBKMK::Vector3D d2 = VICUS::QVector2IBKVector(d);
	IBKMK::Vector3D nearPoint2 = VICUS::QVector2IBKVector(nearPoint);

	// get intersection with xy plane
	VICUS::PlaneGeometry xyPlane(VICUS::PlaneGeometry::T_Rectangle, IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(1,0,0), IBKMK::Vector3D(0,1,0));
	IBKMK::Vector3D intersectionPoint;
	double t;
	if (xyPlane.intersectsLine(nearPoint2, d2, intersectionPoint, t, true, true)) {
		// got an intersection point, store it
		m_pickPoint = VICUS::IBKVector2QVector(intersectionPoint);
	}

	// create pick object, distance is a value between 0 and 1, so initialize with 2 (very far back) to be on the safe side.
//	PickObject p(2.f, std::numeric_limits<unsigned int>::max());

	// now process all objects and update p to hold the closest hit

	// TODO : apply picking/selection filter, so that only specific objects can be clicked on
}


void Vic3DScene::adjustCurserDuringMouseDrag(const QPoint & mouseDelta, const QPoint & localMousePos, QPoint & newLocalMousePos) {
	// cursor position moves out of window?
	const int WINDOW_MOVE_MARGIN = 50;
	if (localMousePos.x() < WINDOW_MOVE_MARGIN && mouseDelta.x() < 0) {
//						qDebug() << "Resetting mousepos to right side of window.";
		newLocalMousePos.setX(m_viewPort.width()-WINDOW_MOVE_MARGIN);
	}
	else if (localMousePos.x() > (m_viewPort.width()-WINDOW_MOVE_MARGIN) && mouseDelta.x() > 0) {
		//						qDebug() << "Resetting mousepos to right side of window.";
		newLocalMousePos.setX(WINDOW_MOVE_MARGIN);
	}

	if (localMousePos.y() < WINDOW_MOVE_MARGIN && mouseDelta.y() < 0) {
		newLocalMousePos.setY(m_viewPort.height()-WINDOW_MOVE_MARGIN);
	}
	else if (localMousePos.y() > (m_viewPort.height()-WINDOW_MOVE_MARGIN) && mouseDelta.y() > 0) {
		newLocalMousePos.setY(WINDOW_MOVE_MARGIN);
	}
}


void Vic3DScene::handleLeftMouseClick() {
	// if we are in passive mode, we handle the click as "selection"

	// if we are in "draw" mode, we either continue drawing points (i.e. add a point), or
	// start a new polygon

	switch (m_operationMode) {
		case OM_Draw : {
			// signal parent widget that we added a point
			IBKMK::Vector3D p = VICUS::QVector2IBKVector(m_coordinateSystemObject.m_transform.translation());
			m_parent->addPolygonVertex(p);
			// append a vertex (this will automatically update the draw buffer)
			m_newPolygonObject.appendVertex(p);
		} break;
		default :;
	}

}


} // namespace Vic3D
