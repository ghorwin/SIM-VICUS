#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QCursor>
#include <QPalette>

#include <VICUS_Project.h>
#include <VICUS_Conversions.h>
#include <VICUS_ViewSettings.h>
#include <VICUS_NetworkLine.h>
#include <VICUS_Conversions.h>

#include <IBKMK_3DCalculations.h>

#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
#include "Vic3DPickObject.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DConstants.h"
#include "Vic3DSceneView.h"

#include "SVProjectHandler.h"
#include "SVPropEditGeometry.h"
#include "SVViewStateHandler.h"
#include "SVViewState.h"
#include "SVSettings.h"
#include "SVUndoTreeNodeState.h"
#include "SVUndoDeleteSelected.h"

const float TRANSLATION_SPEED = 1.2f;
const float MOUSE_ROTATION_SPEED = 0.5f;

/// \todo adjust the THRESHOLD based on DPI/Screenresolution or have it as user option
const float MOUSE_MOVE_DISTANCE_ORBIT_CONTROLLER = 1;

/*! Plane definition for the xy Plane. */
const VICUS::PlaneGeometry xyPlane(VICUS::PlaneGeometry::T_Triangle, IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(1,0,0), IBKMK::Vector3D(0,1,0));

namespace Vic3D {

void Vic3DScene::create(SceneView * parent, std::vector<ShaderProgram> & shaderPrograms) {
	m_parent = parent;
	m_gridShader = &shaderPrograms[SHADER_GRID];
	m_buildingShader = &shaderPrograms[SHADER_OPAQUE_GEOMETRY];
	m_fixedColorTransformShader = &shaderPrograms[SHADER_LINES];
	m_coordinateSystemShader = &shaderPrograms[SHADER_COORDINATE_SYSTEM];
	m_transparencyShader = &shaderPrograms[SHADER_TRANSPARENT_GEOMETRY];

	// the orbit controller object is static in geometry, so it can be created already here
	m_orbitControllerObject.create(m_fixedColorTransformShader);

	// same for the coordinate system object
	m_coordinateSystemObject.create(m_coordinateSystemShader);

	// we create the new geometry object here, but data is added once it is used
	m_newGeometryObject.create(m_fixedColorTransformShader);
}


void Vic3DScene::onModified(int modificationType, ModificationInfo * data) {

	// no shader - not initialized yet, skip modified event
	if (m_gridShader == nullptr)
		return;

	bool updateGrid = false;
	bool updateNetwork = false;
	bool updateBuilding = false;
	bool updateCamera = false;
	bool updateSelection = false;
	// filter out all modification types that we handle
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		case SVProjectHandler::AllModified : {
			updateGrid = true;
			updateBuilding = true;
			updateNetwork = true;
			updateCamera = true;
			updateSelection = true;
			// clear new polygon drawing object
			m_newGeometryObject.clear();
			// set scene operation mode to "normal"
			SVViewState vs = SVViewStateHandler::instance().viewState();
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
			vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
			SVViewStateHandler::instance().setViewState(vs);
		} break;

		case SVProjectHandler::BuildingGeometryChanged :
			updateBuilding = true;
			updateSelection = true;
			break;

		case SVProjectHandler::GridModified :
			updateGrid = true;
			break;

		case SVProjectHandler::NetworkModified :
			updateNetwork = true;
			updateSelection = true;
			break;

		case SVProjectHandler::SelectionModified :
			updateSelection = true;
			break;

		// *** selection and visibility properties changed ***
		case SVProjectHandler::NodeStateModified : {
			// we need to update the colors of some building elements
			unsigned int smallestVertexIndex = m_opaqueGeometryObject.m_vertexBufferData.size();
			unsigned int largestVertexIndex = 0;
			// first decode the modification info object
			const SVUndoTreeNodeState::ModifiedNodes * info = dynamic_cast<SVUndoTreeNodeState::ModifiedNodes *>(data);
			Q_ASSERT(info != nullptr);

			bool selectionModified = false;

			// process all modified nodes
			for (unsigned int id : info->m_nodeIDs) {
				// find the object in question
				const VICUS::Object * obj = project().objectById(id);

				bool visible = true;
				// is this ID a surface?
				const VICUS::Surface * s = dynamic_cast<const VICUS::Surface*>(obj);
				if (s != nullptr) {

					// get vertex start address of selected node
					Q_ASSERT(m_opaqueGeometryObject.m_vertexStartMap.find(id) != m_opaqueGeometryObject.m_vertexStartMap.end());
					unsigned int vertexStart = m_opaqueGeometryObject.m_vertexStartMap[id];
					smallestVertexIndex = std::min(smallestVertexIndex, vertexStart);
					// now update the color buffer for this surface
					updateColors(*s, vertexStart, m_opaqueGeometryObject.m_colorBufferData);
					largestVertexIndex = std::min(smallestVertexIndex, vertexStart);
					visible = s->m_visible;
				}

				// is it a NetworkNode or NetworkEdge?
				const VICUS::NetworkEdge * edge = dynamic_cast<const VICUS::NetworkEdge*>(obj);
				const VICUS::NetworkNode * node= dynamic_cast<const VICUS::NetworkNode*>(obj);
				if (edge != nullptr || node != nullptr) {
					// get vertex start address of selected node/edge
					Q_ASSERT(m_networkGeometryObject.m_vertexStartMap.find(id) != m_networkGeometryObject.m_vertexStartMap.end());
					unsigned int vertexStart = m_networkGeometryObject.m_vertexStartMap[id];
					smallestVertexIndex = std::min(smallestVertexIndex, vertexStart);
					// now update the color buffer for this object depending on type
					if (edge != nullptr) {
						QColor col = Qt::red;
						if (!edge->m_visible || edge->m_selected)
							col.setAlpha(0);
						updateCylinderColors(col, vertexStart, m_networkGeometryObject.m_colorBufferData);
					}
					else {
						QColor col = node->m_visualizationColor;
						if (!node->m_visible || node->m_selected)
							col.setAlpha(0);
						updateSphereColors(col, vertexStart, m_networkGeometryObject.m_colorBufferData);
					}
					largestVertexIndex = std::min(smallestVertexIndex, vertexStart);

					if (edge != nullptr)
						visible = edge->m_visible;
					else
						visible = node->m_visible;
				}

				// update selection set, but only keep visible and selected objects in the set
				if (obj->m_selected && visible) {
					if (m_selectedGeometryObject.m_selectedObjects.insert(obj).second)
						selectionModified = true;
				}
				else {
					std::set<const VICUS::Object*>::const_iterator it = m_selectedGeometryObject.m_selectedObjects.find(obj);
					if (it != m_selectedGeometryObject.m_selectedObjects.end()) {
						m_selectedGeometryObject.m_selectedObjects.erase(*it);
						selectionModified = true;
					}
				}
			}

			// finally, transfer only the modified portion of the color buffer to GPU memory
			m_opaqueGeometryObject.updateColorBuffer(smallestVertexIndex, largestVertexIndex-smallestVertexIndex);

			// we only need to update the selection object, but only if:
			// - the modification state was "SelectedState"
			// - and indeed a selection was changed
			//
			// Note: actually, selected surfaces can be de-selected by hiding them - we do not want to move/alter
			//       invisible geometry
			if (selectionModified) {
				updateSelection = true;
			}
		} break;

		default:
			return; // do nothing by default
	}

	// *** initialize camera placement and model placement in the world ***

	if (updateCamera) {
		QVector3D cameraTrans = VICUS::IBKVector2QVector(SVProjectHandler::instance().viewSettings().m_cameraTranslation);
		m_camera.setTranslation(cameraTrans);
		m_camera.setRotation( SVProjectHandler::instance().viewSettings().m_cameraRotation.toQuaternion() );
	}

	// re-create grid with updated properties
	// since grid object is very small, this function also regenerates the grid line buffers and
	// uploads the data to the GPU
	if (updateGrid)
		m_gridObject.create(m_gridShader);

	if (updateSelection) {
		// update selected objects
		m_selectedGeometryObject.create(m_fixedColorTransformShader);
		m_selectedGeometryObject.updateBuffers();

		// if we are in "Geometry editing" mode, we also show and update the property widget
		SVViewState vs = SVViewStateHandler::instance().viewState();
		if (vs.m_viewMode == SVViewState::VM_GeometryEditMode) {

			IBKMK::Vector3D centerPoint;
			if ( project().haveSelectedSurfaces(centerPoint) ) {
				vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
				vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
				m_coordinateSystemObject.setTranslation( VICUS::IBKVector2QVector(centerPoint) );
				IBKMK::Vector3D v;
				project().boundingBoxofSelectedSurfaces(v);

									vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
									vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
									SVViewStateHandler::instance().m_propEditGeometryWidget->setBoundingBox(v);
									m_coordinateSystemObject.setTranslation( VICUS::IBKVector2QVector(centerPoint) );		}
			else {
				vs.m_sceneOperationMode = SVViewState::NUM_OM;
				vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
			}
			// now tell all UI components to toggle their view state
			SVViewStateHandler::instance().setViewState(vs);
		}
	}

	// create geometry object (if already existing, nothing happens here)
	if (updateBuilding) {
		m_opaqueGeometryObject.create(m_buildingShader->shaderProgram()); // Note: does nothing, if already existing

		// transfer data from building geometry to vertex array caches
		generateBuildingGeometry();
	}

	// create network
	if (updateNetwork) {
		// we use the same shader as for building elements
		m_networkGeometryObject.create(m_buildingShader->shaderProgram()); // Note: does nothing, if already existing

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
	m_opaqueGeometryObject.destroy();
	m_networkGeometryObject.destroy();
	m_selectedGeometryObject.destroy();
	m_coordinateSystemObject.destroy();
	m_newGeometryObject.destroy();
}


void Vic3DScene::resize(int width, int height, qreal retinaScale) {
	float farDistance = 1000;
	// retrieve far viewing distance from project, if one exists
	if (SVProjectHandler::instance().isValid())
		farDistance = (float)SVProjectHandler::instance().viewSettings().m_farDistance;
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


bool Vic3DScene::inputEvent(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos, QPoint & newLocalMousePos) {

	// NOTE: In this function we handle only those keyboard inputs that affect the scene navigation.
	//       Single key release events are handled in Vic3DSceneView, since they do not require repeated screen redraws.

	bool needRepaint = false;

	Transform3D oldCameraTransform = m_camera;

	// *** Keyboard navigation ***

	// translation and rotation works always (no trigger key)

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

	// To avoid duplicate picking operation, we create the pick object here.
	// Then, when we actually need picking, we check if the pick was already executed, and then only
	// retrieve the pick result values
	PickObject pickObject(localMousePos);

	// *** Mouse ***

	newLocalMousePos = localMousePos;
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
		/// \todo Stephan/Dirk: implement something like the orbit controller:
		///                - add a flag that indicates "we are in translate mode"
		///                - when mid-mousebutton is pressed:
		///                  + set the flag
		///                  + pick the current location in screen (if picked point is invalid/on far plane, select middle
		///                    between far and near plane), also remember distance from viewer (i.e. line intersection factor)
		///                - when mouse is moved while mid-mousebutton is pressed
		///                  + move the camera such, that the selected point remains under the mouse cursor at the
		///                    same distance from viewer
		///                - clear the flag, when button is released
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

			// configure the pick object and pick a point on the XY plane/or any visible surface
			if (!pickObject.m_pickPerformed)
				pick(pickObject);

			IBKMK::Vector3D nearestPoint;
			for (const PickObject::PickResult & r : pickObject.m_candidates) {
				if (r.m_snapPointType == PickObject::RT_Object ||
					r.m_snapPointType == PickObject::RT_GlobalXYPlane) {
					nearestPoint = r.m_pickPoint;
					break;
				}
			}

			// for orbit-controller, we  take the closest point of either
			m_orbitControllerOrigin = VICUS::IBKVector2QVector(nearestPoint);
			qDebug() << "Entering orbit controller mode, rotation around" << m_orbitControllerOrigin;
			m_orbitControllerObject.m_transform.setTranslation(m_orbitControllerOrigin);

			// Rotation matrix around origin point
			m_mouseMoveDistance = 0;

			m_orbitControllerActive = true;
			needRepaint = true;
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
				int mouseInversionFactor = SVSettings::instance().m_invertYMouseAxis ? -1 : 1;

				QVector3D LocalRight = m_camera.right().normalized();
				// set rotation around "right" axis for y-mouse-delta
				orbitTrans.rotate(MOUSE_ROTATION_SPEED * mouse_dy * mouseInversionFactor, LocalRight);

				// rotate vector to camera
				lineOfSight = orbitTrans.toMatrix() * lineOfSight;

				// rotate the camera around the same angles
				m_camera.rotate(MOUSE_ROTATION_SPEED * mouse_dx, GlobalUpwardsVector);
				m_camera.rotate(MOUSE_ROTATION_SPEED * mouse_dy * mouseInversionFactor, LocalRight);

#if 1
				// fix "roll" error due to rounding
				// only do this when we are not viewing the scene from vertically from above/below
				float cosViewAngle = QVector3D::dotProduct(m_camera.forward(), GlobalUpwardsVector);
				if (std::fabs(cosViewAngle) < 0.6f) {
					// up and forward vectors should be always in a vertical plane
					// forward and z-axis form a vertical plane with normal
					QVector3D verticalPlaneNormal = QVector3D::crossProduct(m_camera.forward(), GlobalUpwardsVector);
					verticalPlaneNormal.normalize();

					// the camera right angle should always match this normal vector
					float cosBeta = QVector3D::dotProduct(verticalPlaneNormal, m_camera.right().normalized());
					if (cosBeta > -1 && cosBeta < 1) {
						float beta = std::acos(cosBeta)/3.14159265f*180;
						// which direction to rotate?
						m_camera.rotate(beta, m_camera.forward());
						float cosBeta2 = QVector3D::dotProduct(verticalPlaneNormal, m_camera.right().normalized());
						if (std::fabs(std::fabs(cosBeta2) - 1) > 1e-5f)
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

		// check if the mouse was moved not very far -> we have a mouse click
		if (m_mouseMoveDistance < MOUSE_MOVE_DISTANCE_ORBIT_CONTROLLER) {
			qDebug() << "Mouse (selection) click received" << m_mouseMoveDistance;
			handleLeftMouseClick(keyboardHandler, pickObject);
			needRepaint = true;
		}
		else {
			qDebug() << "Leaving orbit controller mode" << m_mouseMoveDistance;
			needRepaint = true;
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

	// store camera position in view settings, but only if we have a project
	// Note: the check is necessary, because the paint event may be called as part of closing the window
	//       and updating the UI to the "no project" state
	if (SVProjectHandler::instance().isValid()) {
		SVProjectHandler::instance().viewSettings().m_cameraTranslation = VICUS::QVector2IBKVector(m_camera.translation());
		SVProjectHandler::instance().viewSettings().m_cameraRotation = m_camera.rotation();
	}
	updateWorld2ViewMatrix();
	// end of camera movement


	// *** adjusting the local coordinate system ***

	QVector3D oldPos = m_coordinateSystemObject.translation();
	// if in "place vertex" mode, perform picking operation and snap coordinate system to grid
	if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_PlaceVertex) {
		// First we need to define what objects/surfaces we check for a picking operation,
		// snapping is done later.

		if (!pickObject.m_pickPerformed)
			pick(pickObject);

		// now we handle the snapping rules and also the locking
		snapLocalCoordinateSystem(pickObject);

		needRepaint = true;
//		qDebug() << localMousePos << VICUS::IBKVector2QVector(o.m_pickPoint) << m_coordinateSystemObject.translation();

		// update the movable coordinate system's location in the new polygon object
		m_newGeometryObject.updateLocalCoordinateSystemPosition(m_coordinateSystemObject.translation());
	}

	// if in "align coordinate system mode" perform picking operation and update local coordinate system orientation
	if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem) {

		if (!pickObject.m_pickPerformed)
			pick(pickObject);
		needRepaint = true;
		// get nearest match
		IBKMK::Vector3D nearestPoint;
		unsigned int uniqueID = 0;
		for (const PickObject::PickResult & r : pickObject.m_candidates) {
			if (r.m_snapPointType == PickObject::RT_Object) {
				nearestPoint = r.m_pickPoint;
				uniqueID = r.m_uniqueObjectID;
				break;
			}
			if (r.m_snapPointType == PickObject::RT_GlobalXYPlane) {
				nearestPoint = r.m_pickPoint;
				break;
			}
		}
		if (uniqueID != 0) {
			// lookup object
			const VICUS::Object * obj = project().objectById(uniqueID);
			// should be a surface
			const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(obj);
			Q_ASSERT(s != nullptr);
			// now get normal vector
			IBKMK::Vector3D n = s->m_geometry.normal();
			// get also local X and Y vectors
			IBKMK::Vector3D localX = s->m_geometry.localX();
			IBKMK::Vector3D localY = s->m_geometry.localY();
			// compose rotation angle from global to local system via the following math:
			// we have local system axes x,y,z and global axes g1, g2, g3
			// and the rotation matrix R should do:
			//  R(g1) = x,  R(g2) = y, R(g3) = z
			//
			// now, if we describe a body axis like:
			//  x = B11.g1 + B21.g2 + B31.g3
			//  y = B11.g1 + B21.g2 + B31.g3
			//  z = B11.g1 + B21.g2 + B31.g3
			//
			// or in matrix writing
			//  R[1,0,0] = [B11, B21, B31]
			//
			// each of the columns of the rotation matrix is the normalized local coordinate axis
			//
			// now build the rotation matrix
//			QMatrix3x3 R;
//			float * r = R.data();
//			*(QVector3D*)r = VICUS::IBKVector2QVector(localX.normalized());
//			r+=3;
//			*(QVector3D*)r = VICUS::IBKVector2QVector(localY.normalized());
//			r+=3;
//			*(QVector3D*)r = VICUS::IBKVector2QVector(n.normalized());
//			qDebug() << R;
//			QQuaternion q = QQuaternion::fromRotationMatrix(R);
//			qDebug() << q;

			// or use the ready-made Qt function (which surprisingly gives the same result :-)
			QQuaternion q2 = QQuaternion::fromAxes(VICUS::IBKVector2QVector(localX.normalized()),
												   VICUS::IBKVector2QVector(localY.normalized()),
												   VICUS::IBKVector2QVector(n.normalized()));
//			qDebug() << q2;
			m_coordinateSystemObject.setRotation(q2);
		}
		else {
			// restore to global orientation
			m_coordinateSystemObject.setRotation(QQuaternion());
		}

		m_coordinateSystemObject.setTranslation( VICUS::IBKVector2QVector(nearestPoint) );
	}


	// determine if we actually need a repaint: this is normally only necessary of the camera or
	// the local coordinate system were moved
	if (oldPos != m_coordinateSystemObject.translation() || needRepaint ||
			m_camera.translation() != oldCameraTransform.translation() ||
			m_camera.rotation() != oldCameraTransform.rotation())
	{
		return true;
	}
	return false;
}


void Vic3DScene::render() {
	glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

	// set the background color = clear color
	const SVSettings::ThemeSettings & s = SVSettings::instance().m_themeSettings[SVSettings::instance().m_theme];
	QVector3D backgroundColor = VICUS::QVector3DFromQColor(s.m_sceneBackgroundColor);

	glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), 1.0f);

	QVector3D viewPos = m_camera.translation();

	const SVViewState & vs = SVViewStateHandler::instance().viewState();


	// enable depth testing, important for the grid and for the drawing order of several objects
	// needed for all opaque geometry
	glEnable(GL_DEPTH_TEST);

	m_fixedColorTransformShader->bind();
	m_fixedColorTransformShader->shaderProgram()->setUniformValue(m_fixedColorTransformShader->m_uniformIDs[0], m_worldToView);

	// *** selection object ***

	m_selectedGeometryObject.render();

	// *** orbit controller indicator ***

	if (m_orbitControllerActive && m_mouseMoveDistance > MOUSE_MOVE_DISTANCE_ORBIT_CONTROLLER) {
		// Note: uses also m_fixedColorTransformShader, which is already active with up-to-date worldToView matrix
		m_orbitControllerObject.render();
	}

	// *** new geometry object (opqaue lines) ***

	m_newGeometryObject.renderOpaque();

	m_fixedColorTransformShader->release();



	// *** movable coordinate system  ***

	if (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex ||
		vs.m_sceneOperationMode == SVViewState::OM_SelectedGeometry ||
		vs.m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem)
	{
		m_coordinateSystemShader->bind();

		// When we translate/rotate the local coordinate system, we actually move the world with respect to the camera
		// by left-multiplying the model2world matrix with the coordinate system object.

		// Suppose the local coordinate system shall be located at 20,0,0 and the camera looks at the coordinate
		// system's origin from 20,-40,2. Now, inside the shader, all coordinates are multiplied by the
		// model2world matrix, which basically moves all coordinates +20 in x direction. Now light and view position
		// (the latter only being used to compute the phong shading) are at 40, -40, 2, wheras the local coordinate
		// system is moved from local 0,0,0 to the desired 20,0,0.
		// Consequently, the light and view position cause the phong shader to draw the sphere as if lighted slightly
		// from the direction of positive x.

		// To fix this, we translate/rotate the view/light position inversely to the model2world transformation and
		// this revert the effect introduced by the model2world matrix on the light/view coordinates.
		QVector3D translatedViewPos = m_coordinateSystemObject.inverseTransformationMatrix() * viewPos;
//		qDebug() << viewPos << m_coordinateSystemObject.translation();

		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[0], m_worldToView);
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[1], translatedViewPos); // lightpos
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[2], VICUS::QVector3DFromQColor(m_lightColor));
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[3], translatedViewPos); // viewpos
		m_coordinateSystemObject.renderOpaque();
		m_coordinateSystemShader->release();
	}


	// *** opaque background geometry ***

	// tell OpenGL to show only faces whose normal vector points towards us
	glEnable(GL_CULL_FACE);

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


	// *** grid ***

	m_gridShader->bind();
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[0], m_worldToView);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[2], backgroundColor);
	float farDistance = (float)std::max(500., SVProjectHandler::instance().viewSettings().m_farDistance);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[3], farDistance);
	m_gridObject.render();
	m_gridShader->release();



	// *** transparent geometry ***

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// tell OpenGL to show all planes
	glDisable(GL_CULL_FACE);
	// disable update of depth test but still use it
	glDepthMask (GL_FALSE);


	// ... windows, ...


	// *** new polygon draw object (transparent plane) ***

	if (m_newGeometryObject.canDrawTransparent() != 0) {
		m_fixedColorTransformShader->bind();
		// Note: worldToView uniform has already been set
		m_newGeometryObject.renderTransparent();
		m_fixedColorTransformShader->release();
	}

	// re-enable updating of z-buffer
	glDepthMask(GL_TRUE);

	glDisable(GL_BLEND);
}


void Vic3DScene::setViewState(const SVViewState & vs) {
	// if we are currently constructing geometry, and we switch the view mode to
	// "Parameter edit" mode, reset the new polygon object
	if (vs.m_viewMode == SVViewState::VM_PropertyEditMode) {
		m_newGeometryObject.clear();
	}
}


void Vic3DScene::generateBuildingGeometry() {
	// get VICUS project data
	const VICUS::Project & p = project();

	// we rebuild the entire geometry here, so this may be slow

	// clear out existing cache

	m_opaqueGeometryObject.m_vertexBufferData.clear();
	m_opaqueGeometryObject.m_colorBufferData.clear();
	m_opaqueGeometryObject.m_indexBufferData.clear();
	m_opaqueGeometryObject.m_vertexStartMap.clear();

	m_opaqueGeometryObject.m_vertexBufferData.reserve(100000);
	m_opaqueGeometryObject.m_colorBufferData.reserve(100000);
	m_opaqueGeometryObject.m_indexBufferData.reserve(100000);

	// we want to draw triangles
	m_opaqueGeometryObject.m_drawTriangleStrips = false;

	// we now process all surfaces and add their coordinates and
	// normals

	// TODO : set colors for each surface: hereby use the current
	// highlighting-filter object, which relates object properties to colors

	// recursively process all buildings, building levels etc.

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				for (const VICUS::Surface & s : r.m_surfaces) {

					// remember where the vertexes for this surface start in the buffer
					m_opaqueGeometryObject.m_vertexStartMap[s.uniqueID()] = currentVertexIndex;

					// now we store the surface data into the vertex/color and index buffers
					// the indexes are advanced and the buffers enlarged as needed.
					// actually, this adds always two surfaces (for culling).
					addSurface(s, currentVertexIndex, currentElementIndex,
							   m_opaqueGeometryObject.m_vertexBufferData,
							   m_opaqueGeometryObject.m_colorBufferData,
							   m_opaqueGeometryObject.m_indexBufferData);
				}
			}
		}
	}

	// now the plain geometry
	for (const VICUS::Surface & s : p.m_plainGeometry) {

		// remember where the vertexes for this surface start in the buffer
		m_opaqueGeometryObject.m_vertexStartMap[s.uniqueID()] = currentVertexIndex;

		// now we store the surface data into the vertex/color and index buffers
		// the indexes are advanced and the buffers enlarged as needed.
		// actually, this adds always two surfaces (for culling).
		addSurface(s, currentVertexIndex, currentElementIndex,
				   m_opaqueGeometryObject.m_vertexBufferData,
				   m_opaqueGeometryObject.m_colorBufferData,
				   m_opaqueGeometryObject.m_indexBufferData);
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
	m_networkGeometryObject.m_vertexStartMap.clear();

	m_networkGeometryObject.m_vertexBufferData.reserve(100000);
	m_networkGeometryObject.m_colorBufferData.reserve(100000);
	m_networkGeometryObject.m_indexBufferData.reserve(100000);

	// process all network elements

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;


	// add cylinders for all pipes
	for (const VICUS::Network & network : p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge & e : network.m_edges) {
			double radius = e.m_visualizationRadius;
			QColor pipeColor = Qt::red;
			if (!e.m_visible || !network.m_visible || e.m_selected)
				pipeColor.setAlpha(0);

			m_networkGeometryObject.m_vertexStartMap[e.uniqueID()] = currentVertexIndex;
			addCylinder(e.m_node1->m_position, e.m_node2->m_position, pipeColor, radius,
						currentVertexIndex, currentElementIndex,
						m_networkGeometryObject.m_vertexBufferData,
						m_networkGeometryObject.m_colorBufferData,
						m_networkGeometryObject.m_indexBufferData);
		}

		// add spheres for nodes
		for (const VICUS::NetworkNode & no : network.m_nodes) {
			double radius = no.m_visualizationRadius;
			QColor col = no.m_visualizationColor;
			if (!no.m_visible || !network.m_visible)
				col.setAlpha(0);

			m_networkGeometryObject.m_vertexStartMap[no.uniqueID()] = currentVertexIndex;
			addSphere(no.m_position, col, radius,
						currentVertexIndex, currentElementIndex,
						m_networkGeometryObject.m_vertexBufferData,
						m_networkGeometryObject.m_colorBufferData,
						m_networkGeometryObject.m_indexBufferData);
		}
	}

}


void Vic3DScene::deselectAll() {
	// compose undo-action of objects currently selected
	if (m_selectedGeometryObject.m_selectedObjects.empty())
		return; // nothing selected, nothing to do

	std::set<unsigned int> nodeIDs;
	for (const VICUS::Object * o : m_selectedGeometryObject.m_selectedObjects)
		nodeIDs.insert(o->uniqueID());
	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selected cleared"),
														 SVUndoTreeNodeState::SelectedState, nodeIDs, false);
	undo->push();
}


void Vic3DScene::deleteSelected() {
	// process all project geometry and keep (in a copy) only those that need to be removed
	std::vector<unsigned int> selectedObjectIDs;
	for (const VICUS::Object * obj : m_selectedGeometryObject.m_selectedObjects)
		selectedObjectIDs.push_back(obj->uniqueID());

	// clear selected objects (since they are now removed)
	m_selectedGeometryObject.m_selectedObjects.clear();
	SVUndoDeleteSelected * undo = new SVUndoDeleteSelected(tr("Removing selected geometry"),
														   selectedObjectIDs);
	// clear selection
	undo->push();
}


void Vic3DScene::leaveCoordinateSystemAdjustmentMode(bool abort) {
	// restore original local coordinate system
	if (abort) {
		m_coordinateSystemObject.setTransform(m_oldCoordinateSystemTransform);
		qDebug() << "Aborting 'Align coordinate system' mode (no change)";
	}
	else {
		// finish aligning coordinate system and keep selected rotation in coordinate system
		// but restore origin of local coordinate system object
		m_coordinateSystemObject.setTranslation(m_oldCoordinateSystemTransform.translation());
		qDebug() << "Leaving 'Align coordinate system' mode";
	}
	// switch back to previous view state
	SVViewStateHandler::instance().restoreLastViewState();
}


void Vic3DScene::enterCoordinateSystemAdjustmentMode() {
	// store current transformation of local coordinate system object
	m_oldCoordinateSystemTransform = m_coordinateSystemObject.transform();
	// turn on AlignLocalCoordinateSystem mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_AlignLocalCoordinateSystem;
	SVViewStateHandler::instance().setViewState(vs);
	qDebug() << "Entering 'Align coordinate system' mode";
}


void Vic3DScene::pick(PickObject & pickObject) {

	// local mouse coordinates
	int my = pickObject.m_localMousePos.y();
	int mx = pickObject.m_localMousePos.x();

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

	// compute line-of-sight equation
	IBKMK::Vector3D nearPoint = VICUS::QVector2IBKVector(nearResult.toVector3D()); // line offset = nearPoint
	IBKMK::Vector3D farPoint = VICUS::QVector2IBKVector(farResult.toVector3D());
	IBKMK::Vector3D direction = farPoint - nearPoint;	// direction vector of line-of-sight

	// now do the actual picking
#define SHOW_PICK_TIME
#ifdef SHOW_PICK_TIME
	QElapsedTimer pickTimer;
	pickTimer.start();
#endif

	// get intersection with global xy plane
	IBKMK::Vector3D intersectionPoint;
	double t;
	// X-Y-Plane is picked from both sides
	if (xyPlane.intersectsLine(nearPoint, direction, intersectionPoint, t, true, true)) {
		// got an intersection point, store it
		PickObject::PickResult r;
		r.m_snapPointType = PickObject::RT_GlobalXYPlane;
		r.m_depth = t;
		r.m_pickPoint = intersectionPoint;
		pickObject.m_candidates.push_back(r);
	}

	// if lock is enabled, compute intersection point with plane, or in case of axis lock, the closest point on
	// axis.
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	if (vs.m_locks != 0) {
		// get reference point for relative translation/plane/line snap
		IBKMK::Vector3D offset = referencePoint();
		IBKMK::Vector3D a, b;
		bool planeSnap = true;
		// now process all the different combinations of locks
		switch (vs.m_locks) {
			case SVViewState::L_LocalX :
				a = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(0,1,0)) );
				b = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(0,0,1)) );
			break;
			case SVViewState::L_LocalY :
				a = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(1,0,0)) );
				b = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(0,0,1)) );
			break;
			case SVViewState::L_LocalZ :
				a = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(1,0,0)) );
				b = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(0,1,0)) );
			break;
			case SVViewState::L_LocalX | SVViewState::L_LocalY :
			case SVViewState::L_LocalX | SVViewState::L_LocalY | SVViewState::L_LocalZ : // this is actually invalid, but we treat it as "X+Y locked"
				a = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(0,0,1)) );
				planeSnap = false;
			break;
			case SVViewState::L_LocalX | SVViewState::L_LocalZ :
				a = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(0,1,0)) );
				planeSnap = false;
			break;
			case SVViewState::L_LocalY | SVViewState::L_LocalZ :
				a = VICUS::QVector2IBKVector(m_coordinateSystemObject.transform().rotation().rotatedVector(QVector3D(1,0,0)) );
				planeSnap = false;
			break;
		}
		// plane intersection?
		if (planeSnap) {
			VICUS::PlaneGeometry pg(VICUS::PlaneGeometry::T_Rectangle, offset, offset+a, offset+a+b);
			if (pg.intersectsLine(nearPoint, direction, intersectionPoint, t, true, true)) {
				// got an intersection point, store it
				PickObject::PickResult r;
				r.m_snapPointType = PickObject::RT_LocalPlaneFixedAxis;
				r.m_depth = t;
				r.m_pickPoint = intersectionPoint;
				pickObject.m_candidates.push_back(r);
			}
		}
		else {
			// line2line intersection
			double dist;
			IBKMK::Vector3D closestPoint;
			double lineFactor;
			IBKMK::lineToLineDistance(nearPoint, direction,
									  offset, a,
									  dist, closestPoint, lineFactor);
			// check distance against cylinder radius
			PickObject::PickResult r;
			r.m_snapPointType = PickObject::RT_LocalFixedAxis;
			r.m_depth = dist;
			r.m_pickPoint = offset + lineFactor*a;
			pickObject.m_candidates.push_back(r);
		}
	}


	// now process all surfaces and update p to hold the closest hit
	const VICUS::Project & prj = project();

	// first try surfaces in buildings
	for (const VICUS::Building & b : prj.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				for (const VICUS::Surface & s : r.m_surfaces) {
					// skip invisible or inactive surfaces
					if (!s.m_visible)
						continue;
					IBKMK::Vector3D intersectionPoint;
					double dist;
					// check if we hit the surface - since we show the surface from both sides, we
					// can also pick both sides
					if (s.m_geometry.intersectsLine(nearPoint, direction, intersectionPoint, dist, true)) {
						PickObject::PickResult r;
						r.m_snapPointType = PickObject::RT_Object;
						r.m_depth = dist;
						r.m_pickPoint = intersectionPoint;
						r.m_uniqueObjectID = s.uniqueID();
						pickObject.m_candidates.push_back(r);
					}
				}
			}
		}
	}
	// now try plain geometry
	for (const VICUS::Surface & s : prj.m_plainGeometry) {
		// skip invisible or inactive surfaces
		if (!s.m_visible)
			continue;
		IBKMK::Vector3D intersectionPoint;
		double dist;
		// dump geometry is rendered front/back facing and also picked from both sides
		if (s.m_geometry.intersectsLine(nearPoint, direction, intersectionPoint, dist, true)) {
			PickObject::PickResult r;
			r.m_snapPointType = PickObject::RT_Object;
			r.m_depth = dist;
			r.m_pickPoint = intersectionPoint;
			r.m_uniqueObjectID = s.uniqueID();
			pickObject.m_candidates.push_back(r);
		}
	}

	// process all networks
	for (const VICUS::Network & n : prj.m_geometricNetworks) {

		// process all nodes
		for (const VICUS::NetworkNode & no : n.m_nodes) {

			// skip invisible nodes
			if (!no.m_visible)
				continue;

			// compute closest distance between nodal center point and line
			double dist;
			IBKMK::Vector3D closestPoint;
			double linePointDist = IBKMK::lineToPointDistance(nearPoint, direction, no.m_position, dist, closestPoint);
			// check distance against radius of sphere
			if (linePointDist < no.m_visualizationRadius) {
				PickObject::PickResult r;
				r.m_snapPointType = PickObject::RT_NetworkNode;
				r.m_depth = dist;
				r.m_pickPoint = closestPoint;
				r.m_uniqueObjectID = no.uniqueID();
				pickObject.m_candidates.push_back(r);
			}
		}

		// process all edges
		for (const VICUS::NetworkEdge & e : n.m_edges) {

			// skip invisible nodes
			if (!e.m_visible)
				continue;

			// compute closest distance between nodal center point and line
			double dist;
			IBKMK::Vector3D closestPoint;
			double lineFactor;
			double line2LineDistance = IBKMK::lineToLineDistance(nearPoint, direction,
																 e.m_node1->m_position, e.m_node2->m_position - e.m_node1->m_position,
																 dist, closestPoint, lineFactor);
			// check distance against cylinder radius
			if (line2LineDistance < e.m_visualizationRadius && lineFactor >= 0 && lineFactor <= 1) {
				PickObject::PickResult r;
				r.m_snapPointType = PickObject::RT_NetworkEdge;
				r.m_depth = dist;
				r.m_pickPoint = closestPoint;
				r.m_uniqueObjectID = e.uniqueID();
				pickObject.m_candidates.push_back(r);
			}
		}
	}

	// finally sort the pick candidates based on depth value
	std::sort(pickObject.m_candidates.begin(), pickObject.m_candidates.end());

#ifdef SHOW_PICK_TIME
	qDebug() << "Pick duration = " << pickTimer.elapsed() << "ms";
#endif

	pickObject.m_pickPerformed = true;
}


struct ClosestPointFinder {
	bool operator()(const std::pair<float, QVector3D> & lhs, const std::pair<float, QVector3D> & rhs) {
		return lhs.first < rhs.first;
	}
};


void Vic3DScene::snapLocalCoordinateSystem(const PickObject & pickObject) {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();

	// we have now several surfaces/objects stored in the pickObject

	// first we handle the situation without snap
	if (!vs.m_snapEnabled || true) {
		// no snapping enabled - three options:
		// a) no axis lock -> place coordinate system on either a visible surface, or the global XY plane
		// b) plane lock is enabled -> place coordinate system on locked plane or visible surface, whatever is closest
		// c) place coordinate system on locked axis position
		if (vs.m_locks == 0) {
			// get snap point with XY plane
			IBKMK::Vector3D p;
			for (const PickObject::PickResult & r : pickObject.m_candidates) {
				if (r.m_snapPointType == PickObject::RT_GlobalXYPlane) {
					p = r.m_pickPoint;
					break;
				}
				if (r.m_snapPointType == PickObject::RT_Object) {
					p = r.m_pickPoint;
					break;
				}
			}
			m_coordinateSystemObject.setTranslation( VICUS::IBKVector2QVector(p) );
			return;
		}
		// if we have plane snap?
		switch (vs.m_locks) {
			case SVViewState::L_LocalX :
			case SVViewState::L_LocalY :
			case SVViewState::L_LocalZ :
			{
				// search for plane or object
				IBKMK::Vector3D p;
				for (const PickObject::PickResult & r : pickObject.m_candidates) {
					if (r.m_snapPointType == PickObject::RT_LocalPlaneFixedAxis) {
						p = r.m_pickPoint;
						break;
					}
					if (r.m_snapPointType == PickObject::RT_Object) {
						p = r.m_pickPoint;
						break;
					}
				}
				m_coordinateSystemObject.setTranslation( VICUS::IBKVector2QVector(p) );
			} break;

			// all others are line-snap
			default :
			{
				// search for plane or object
				IBKMK::Vector3D p;
				for (const PickObject::PickResult & r : pickObject.m_candidates) {
					if (r.m_snapPointType == PickObject::RT_LocalFixedAxis) {
						p = r.m_pickPoint;
						break;
					}
					if (r.m_snapPointType == PickObject::RT_Object) {
						p = r.m_pickPoint;
						break;
					}
				}
				m_coordinateSystemObject.setTranslation( VICUS::IBKVector2QVector(p) );
			} break;
		}
		return;
	}



#if 0
	// first, we need to filter out, which ones we snap to
	// for that purpose, we do the following:
	// - loop over all snap candidates
	int snapOptions = vs.m_snapOptionMask;

	// Snapping works as follows:
	// We process all snap options and compute the relative distance to
	// the current coordinate system's location and we store these distances in a list.
	// Then, we select the closes snap point.
	std::set<std::pair<float, QVector3D>, ClosestPointFinder> snapPoints;

	// If the distance between current coord
	QVector3D pickPoint = VICUS::IBKVector2QVector(pickObject.m_pickPoint);

	if (snapOptions & SVViewState::Snap_XYPlane_Grid) {
		// now determine which grid line is closest
		QVector3D closestPoint;
		if (m_gridObject.closestSnapPoint(VICUS::IBKVector2QVector(pickObject.m_pickPoint), closestPoint)) {
			// this is in world coordinates, use this as transformation vector for the
			// coordinate system
			float dist = (closestPoint - pickPoint).lengthSquared();
			snapPoints.insert( std::make_pair(dist, closestPoint) );
		}
	}

	// for snap options that require an object, look it up
	const VICUS::Surface * s = nullptr;
	if (pickObject.m_uniqueObjectID != 0 && snapOptions > SVViewState::Snap_XYPlane_Grid) {
		const VICUS::Object * o = project().objectById(pickObject.m_uniqueObjectID);
		s = dynamic_cast<const VICUS::Surface *>(o);
	}
	// handle all object-related snaps
	if (s != nullptr) {
		if (snapOptions & SVViewState::Snap_ObjectVertex) {
			// insert distances to all vertexes of selected object
			for (const IBKMK::Vector3D & v : s->m_geometry.vertexes()) {
				QVector3D p = VICUS::IBKVector2QVector(v);
				float dist = (p - pickPoint).lengthSquared();
				snapPoints.insert( std::make_pair(dist, p) );
			}
		}
		if (snapOptions & SVViewState::Snap_ObjectCenter) {
			// insert center point
			IBKMK::Vector3D center(0,0,0);
			for (const IBKMK::Vector3D & v : s->m_geometry.vertexes())
				center += v;
			center /= s->m_geometry.vertexes().size();
			QVector3D p = VICUS::IBKVector2QVector(center);
			float dist = (p - pickPoint).lengthSquared();
			snapPoints.insert( std::make_pair(dist, p) );
		}
		if (snapOptions & SVViewState::Snap_ObjectEdgeCenter) {
			// process all edges
			IBKMK::Vector3D lastNode = s->m_geometry.vertexes().front();
			for (unsigned int i=0; i<s->m_geometry.vertexes().size()+1; ++i) {
				IBKMK::Vector3D center = lastNode;
				lastNode = s->m_geometry.vertexes()[i % s->m_geometry.vertexes().size()];
				center += lastNode;
				center /= 2;
				QVector3D p = VICUS::IBKVector2QVector(center);
				float dist = (p - pickPoint).lengthSquared();
				snapPoints.insert( std::make_pair(dist, p) );
			}
		}
	}

	// take closes snap point and snap to it
	QVector3D newCoordinatePoint;
	if (snapPoints.empty()) {
		// no snap points? no snapping
		newCoordinatePoint = VICUS::IBKVector2QVector(pickObject.m_pickPoint);
	}
	else {
		QVector3D closestPoint = snapPoints.begin()->second;
		newCoordinatePoint = closestPoint;
//		qDebug() << (s != nullptr ? "object snap" : "grid snap") << closestPoint;
	}

	// if we have x,y or z local axis lock enabled, allow only movement in local x, y or z direction, that means,
	// get the projection onto the locked axis

	m_coordinateSystemObject.setTranslation(newCoordinatePoint);
#endif
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


void Vic3DScene::handleLeftMouseClick(const KeyboardMouseHandler & keyboardHandler, PickObject & o) {
	// do different things depending on current scene operation mode

	switch (SVViewStateHandler::instance().viewState().m_sceneOperationMode) {

		// *** place a vertex ***
		case SVViewState::NUM_OM :
		case SVViewState::OM_SelectedGeometry : {
			// selection handling
			handleSelection(keyboardHandler, o);
			return;
		}

		// *** place a vertex ***
		case SVViewState::OM_PlaceVertex : {
			// get current coordinate system's position
			IBKMK::Vector3D p = VICUS::QVector2IBKVector(m_coordinateSystemObject.translation());
			// append a vertex (this will automatically update the draw buffer) and also
			// modify the vertexListWidget.
			m_newGeometryObject.appendVertex(p);
			return;
		}

		// *** align coordinate system ***
		case SVViewState::OM_AlignLocalCoordinateSystem : {
			// finish aligning coordinate system and keep selected rotation in coordinate system
			// but restore origin of local coordinate system object
			m_coordinateSystemObject.setTranslation(m_oldCoordinateSystemTransform.translation());
			// switch back to previous view state
			SVViewStateHandler::instance().restoreLastViewState();
			qDebug() << "Leaving 'Align coordinate system' mode";
			return;
		}
	}

}


void Vic3DScene::handleSelection(const KeyboardMouseHandler & keyboardHandler, PickObject & o) {
	// this will be a selection click - execute pick() operation
	if (!o.m_pickPerformed)
		pick(o);

	// check if any of the pick candidates is of type object
	unsigned int uniqueID = 0;
	for (const PickObject::PickResult & r : o.m_candidates) {
		if (r.m_snapPointType == PickObject::RT_Object) {
			uniqueID = r.m_uniqueObjectID;
			break;
		}
	}

	if (uniqueID != 0) {
		// find the selected object
		const VICUS::Object * obj = project().objectById(uniqueID);

		// create undo-action that toggles the selection
		bool withoutChildren = keyboardHandler.keyDown(Qt::Key_Shift);
		SVUndoTreeNodeState * action = SVUndoTreeNodeState::createUndoAction(tr("Selection changed"),
															   SVUndoTreeNodeState::SelectedState,
															   uniqueID,
															   !withoutChildren,
															   !obj->m_selected);
		action->push();
		return;
	}
}


IBKMK::Vector3D Vic3DScene::referencePoint() const {
	// for now, return the position of the last added vertex, if any
	if (m_newGeometryObject.vertexList().empty())
		return IBKMK::Vector3D();
	else
		return m_newGeometryObject.vertexList().back();
}


} // namespace Vic3D
