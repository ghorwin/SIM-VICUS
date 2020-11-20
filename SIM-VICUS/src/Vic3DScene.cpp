#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QCursor>

#include <VICUS_Project.h>
#include <VICUS_Conversions.h>
#include <VICUS_ViewSettings.h>
#include <VICUS_NetworkLine.h>

#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
#include "Vic3DPickObject.h"
#include "Vic3DGeometryHelpers.h"

#include "SVProjectHandler.h"

const float TRANSLATION_SPEED = 1.2f;
const float MOUSE_ROTATION_SPEED = 0.5f;


namespace Vic3D {



void Vic3DScene::create(ShaderProgram * gridShader, ShaderProgram * buildingShader, ShaderProgram * orbitControllerShader) {
	m_gridShader = gridShader;
	m_buildingShader = buildingShader;
	m_orbitControllerShader = orbitControllerShader;

	// the orbit controller object is static in geometry, so it can be created already here
	m_orbitControllerObject.create(m_orbitControllerShader);
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


}


void Vic3DScene::destroy() {
	m_gridObject.destroy();
	m_orbitControllerObject.destroy();
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


void Vic3DScene::inputEvent(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos) {
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

	// if right mouse button is pressed, mouse delta is translated into first camera perspective rotations
	if (keyboardHandler.buttonDown(Qt::RightButton)) {
		// get and reset mouse delta (pass current mouse cursor position)
		QPoint mouseDelta = keyboardHandler.mouseDelta(QCursor::pos()); // resets the internal position
		const QVector3D LocalUp(0.0f, 0.0f, 1.0f); // same as in Camera::up()
		m_camera.rotate(-MOUSE_ROTATION_SPEED * mouseDelta.x(), LocalUp);
		m_camera.rotate(-MOUSE_ROTATION_SPEED * mouseDelta.y(), m_camera.right());
	}
	if (keyboardHandler.buttonDown(Qt::MidButton)) {

		QPoint mouseDelta = keyboardHandler.mouseDelta(QCursor::pos()); // resets the internal position

		int x = mouseDelta.x();
		int y = mouseDelta.y();
		if(x != 0)
			m_camera.translate(transSpeed * (mouseDelta.x() < 0 ? 1 : -1) * m_camera.right());
		if(y != 0)
			m_camera.translate(transSpeed * (mouseDelta.y() > 0 ? 1 : -1) * m_camera.up());
	}

	else { // not supporting right-and-left mouse button multiclick

		if (keyboardHandler.buttonDown(Qt::LeftButton)) {
			// detect "enter orbital controller mode" switch
			if (!m_orbitControllerActive) {
				// we enter orbital controller mode

				// pick the rotation object
				pick(localMousePos);
				// and store pick point
				m_orbitControllerOrigin = m_pickPoint;
				qDebug() << "Entering orbit controller mode, rotation around" << m_orbitControllerOrigin;
				m_orbitControllerObject.m_transform.setTranslation(m_orbitControllerOrigin);

				// Rotation matrix around origin point

				m_orbitControllerActive = true;
			}
			else {

				QPoint mouseDelta = keyboardHandler.mouseDelta(QCursor::pos()); // resets the internal position
				if (mouseDelta != QPoint(0,0)) {
					// vector from pick point (center of orbit) to camera position
					QVector3D lineOfSight = m_camera.translation() - m_orbitControllerOrigin;

					// create a transformation object
					Transform3D orbitTrans;

					// mouse x translation = rotation around rotation axis

					const QVector3D LocalUp(0.0f, 0.0f, 1.0f); // same as in Camera::up()
					// set rotation around z axis for x-mouse-delta
					orbitTrans.rotate(MOUSE_ROTATION_SPEED * mouseDelta.x(), LocalUp);

					// mouse y translation = rotation around "right" axis

					QVector3D LocalRight = m_camera.right().normalized();
//					QVector3D LocalRight2 = QVector3D::crossProduct(LocalUp, lineOfSight).normalized();
//					qDebug() << LocalRight << "\n" << LocalRight2 << "\n" << LocalRight2-LocalRight;

					// There is a situation where this fails:
					// when the line of sight vector becomes co-linear with the LocalUp vector (i.e. one
					// is tilting the view such that we look directly down), then the cross-product
					// gives us a zero vector (or close to zero due to rounding errors).
					// The rotation around a zero vector (or close to zero) will give somewhat arbitrary results,
					// and specifically destroy our "z-axis is facing up" alignment - the camera appears to
					// "roll".

					// TODO : Dirk, implement a fix to "re-align" the camera such that its local up remains aligned
					//        with the z-axis. Hint: tilt the camera down so that the forward-vector lies in the xy-plane.
					//        An ideally aligned camera should have local up-vector = z-axis vector. If misaligned,
					//        simply "roll" the camera back and reverse the "down tilt" to get the fixed camera rotation.

					// set rotation around "right" axis for y-mouse-delta
					orbitTrans.rotate(MOUSE_ROTATION_SPEED * mouseDelta.y(), LocalRight);

					// rotate vector to camera
					lineOfSight = orbitTrans.toMatrix() * lineOfSight;

					// get new camera location
					QVector3D newCamPos = m_orbitControllerOrigin + lineOfSight;
					//					qDebug() << "Moving camera from " << m_camera.translation() << "to" << newCamPos;

					// record the distance that the camera was moved
					m_mouseMoveDistance += (newCamPos - m_camera.translation()).lengthSquared();
					// move camera
					m_camera.setTranslation(newCamPos);

					// also rotate the camera around the same angles
					m_camera.rotate(MOUSE_ROTATION_SPEED * mouseDelta.x(), LocalUp);
					m_camera.rotate(MOUSE_ROTATION_SPEED * mouseDelta.y(), LocalRight);
				}
			}

		} // left button down

		if (keyboardHandler.buttonReleased(Qt::LeftButton)) {
			// check if the mouse was moved not very far -> we have a mouse click
			if (m_mouseMoveDistance < 20) {
				// TODO : click code
				qDebug() << "Mouse (selection) click received";
			}

			// clear orbit controller flag
			m_orbitControllerActive = false;
			qDebug() << "Leaving orbit controller mode";
		} // left button released
	}

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
}


void Vic3DScene::render() {
	glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

	// set the background color = clear color
	glClearColor(m_background.x(), m_background.y(), m_background.z(), 1.0f);


	// *** grid ***

	// enable depth testing, important for the grid and for the drawing order of several objects
	glEnable(GL_DEPTH_TEST);

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


	// *** opaque background geometry ***


	// tell OpenGL to show only faces whose normal vector points towards us
	glEnable(GL_CULL_FACE);


	// *** opaque building geometry ***


	m_buildingShader->bind();
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[0], m_worldToView);

	// Note: you can't use a QColor here directly and pass it as uniform to a shader expecting a vec3. Qt internally
	//       passes QColor as vec4.
	QVector3D lightCol(m_lightColor.redF(), m_lightColor.greenF(), m_lightColor.blueF());
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[2], lightCol);

	// set view position -
	QVector3D viewPos = m_camera.translation();
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[1], viewPos);

//#define FIXED_LIGHT_POSITION
#ifdef FIXED_LIGHT_POSITION
	// use a fixed light position
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[1], m_lightPos);
#else
	// use view position as light position
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[3], viewPos);
#endif // FIXED_LIGHT_POSITION

	// first draw network with culling enabled
	//glDisable(GL_CULL_FACE); // for now draw with culling disabled
	m_networkGeometryObject.render();

	// then turn culling off, so that we see front and back sides of surfaces
	m_opaqueGeometryObject.render();



	m_buildingShader->release();


	// *** input coordinate system ***

	// x-axis: red
	// y-axis: green
	// z-axis: blue
	// origin ball: purple + yellow




	// *** transparent building geometry ***


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
			VICUS::NetworkLine l(e);
			double radius = 0.5; //e.m_diameterOutside*5; // enlarge diameter, so that we see something
			addCylinder(IBKMK::Vector3D(l.m_x1,l.m_y1,1), IBKMK::Vector3D(l.m_x2,l.m_y2,1), Qt::red,
						radius,
						currentVertexIndex, currentElementIndex,
						m_networkGeometryObject.m_vertexBufferData,
						m_networkGeometryObject.m_colorBufferData,
						m_networkGeometryObject.m_indexBufferData);
		}
		for (const VICUS::NetworkNode & no : n.m_nodes) {
			double radius = 0.8; //e.m_diameterOutside*5; // enlarge diameter, so that we see something
			addSphere(IBKMK::Vector3D(no.m_x,no.m_y,1), Qt::cyan,
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




QColor Vic3DScene::color4Surface(const VICUS::Surface & s) const {
	// for now return always white
	return Qt::white;
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


//	XYPlaneObject.pick(nearPoint, d, p);

//	m_boxObject.pick(nearPoint, d, p);
//	// ... other objects

	// any object accepted a pick?
//	if (p.m_objectId == std::numeric_limits<unsigned int>::max())
//		return; // nothing selected

//	qDebug().nospace() << "Pick successful (Box #"
//					   << p.m_objectId <<  ", Face #" << p.m_faceId << ", t = " << p.m_dist << ") after "
//					   << pickTimer.elapsed() << " ms";

	// Mind: OpenGL-context must be current when we call this function!
//	m_boxObject.highlight(p.m_objectId, p.m_faceId);
}

} // namespace Vic3D
