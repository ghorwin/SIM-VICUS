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

#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QCursor>
#include <QPalette>
#include <QApplication>
#include <QRandomGenerator>

#include <VICUS_Project.h>
#include <SVConversions.h>
#include <VICUS_ViewSettings.h>
#include <VICUS_NetworkLine.h>
#include <SVConversions.h>

#include <IBKMK_3DCalculations.h>

#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
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
#include "SVNavigationTreeWidget.h"
#include "SVMeasurementWidget.h"
#include "SVMainWindow.h"
#include "SVGeometryView.h"
#include "SVPropertyWidget.h"
#include "SVLocalCoordinateView.h"


const float TRANSLATION_SPEED = 1.2f;
const float MOUSE_ROTATION_SPEED = 0.5f;

// Size of the local coordinate system window
const int SUBWINDOWSIZE = 150;

/// \todo All: adjust the THRESHOLD based on DPI/Screenresolution or have it as user option
const float MOUSE_MOVE_DISTANCE_ORBIT_CONTROLLER = 5;

namespace Vic3D {

void Scene::create(SceneView * parent, std::vector<ShaderProgram> & shaderPrograms) {
	m_parent = parent;
	m_gridShader = &shaderPrograms[SHADER_GRID];
	m_surfaceNormalsShader = &shaderPrograms[SHADER_LINES];
	m_measurementShader = &shaderPrograms[SHADER_LINES];
	m_buildingShader = &shaderPrograms[SHADER_OPAQUE_GEOMETRY];
	m_fixedColorTransformShader = &shaderPrograms[SHADER_WIREFRAME];
	m_coordinateSystemShader = &shaderPrograms[SHADER_COORDINATE_SYSTEM];
	m_transparencyShader = &shaderPrograms[SHADER_TRANSPARENT_GEOMETRY];
	m_rubberbandShader = &shaderPrograms[SHADER_DASHED_LINES];

	// the orbit controller object is static in geometry, so it can be created already here
	m_orbitControllerObject.create(m_fixedColorTransformShader);

	// same for the coordinate system object
	m_coordinateSystemObject.create(m_coordinateSystemShader);

	// and for the small coordinate system object
	m_smallCoordinateSystemObject.create(m_coordinateSystemShader, m_transparencyShader);

	// Rubberband object
	m_rubberbandObject.create(this, m_rubberbandShader);

	// we create the new geometry object here, but data is added once it is used
	m_newGeometryObject.create(m_fixedColorTransformShader);
	m_newSubSurfaceObject.create(m_buildingShader->shaderProgram());
	m_measurementObject.create(m_measurementShader);

	// create surface normals object already, though we update vertex buffer object later when we actually have geometry
	m_surfaceNormalsObject.create(m_surfaceNormalsShader);

	m_measurementWidget = SVViewStateHandler::instance().m_measurementWidget;
}


void Scene::onModified(int modificationType, ModificationInfo * /*data*/) {

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
		// set scene operation mode to "normal" if we are in place vertex mode
		SVViewState vs = SVViewStateHandler::instance().viewState();
		if (!vs.inPropertyEditingMode()) {
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
			// we have no selection, switch to "add geometry" mode
			vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
		}

		// clear selection object, to avoid accessing invalidated pointers
		m_selectedGeometryObject.m_selectedObjects.clear();
		SVViewStateHandler::instance().setViewState(vs);
	} break;

	case SVProjectHandler::BuildingTopologyChanged :
		refreshColors();
		break;

	case SVProjectHandler::BuildingGeometryChanged : {
		updateBuilding = true;
		updateSelection = true;
		// we might have just deleted all selected items, in this case switch back to AddGeometry

		std::set<const VICUS::Object*> selectedObjects;

		project().selectObjects(selectedObjects, VICUS::Project::SG_All, true, true);
		// if we have a selection, switch scene operation mode to OM_SelectedGeometry
		SVViewState vs = SVViewStateHandler::instance().viewState();
		if (!vs.inPropertyEditingMode()) {
			if (selectedObjects.empty()) {
				vs.m_sceneOperationMode = SVViewState::NUM_OM;
				vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
			}
			else {
				vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
				// Do not modify property widget mode
			}
			SVViewStateHandler::instance().setViewState(vs);
		}

	} break;

	case SVProjectHandler::GridModified :
		updateGrid = true;
		updateCamera = true;
		break;

	case SVProjectHandler::NetworkGeometryChanged :
		updateNetwork = true;
		updateSelection = true;
		refreshColors();
		break;

	case SVProjectHandler::ComponentInstancesModified : {
		const SVViewState & vs = SVViewStateHandler::instance().viewState();
		if (vs.inPropertyEditingMode()) {
			refreshColors();
			if (vs.m_objectColorMode == SVViewState::OCM_InterlinkedSurfaces)
				m_transparentBuildingObject.updateBuffers();
		}
		return;
	}

	case SVProjectHandler::SubSurfaceComponentInstancesModified :
		// changes in sub-surface assignments may change the transparency of constructions, hence
		// requires a re-setup of building geometry
		updateBuilding = true;
		break;

		// *** selection and visibility properties changed ***
	case SVProjectHandler::NodeStateModified : {
		// for now trigger update of building and network geometry objects
		// TODO : for large objects this may not necessarily be fast, especially if only few
		//        elements are selected, but at least this is very robust
		updateBuilding = true;
		updateNetwork = true;

		// Now check if our new selection set is different from the previous selection set.
		std::set<const VICUS::Object*> selectedObjects;

		project().selectObjects(selectedObjects, VICUS::Project::SG_All, true, true);
		if (selectedObjects != m_selectedGeometryObject.m_selectedObjects)
			updateSelection = true;

		// If we have a selection, switch scene operation mode to OM_SelectedGeometry.
		// If we no longer have a selection, and we are in geometry mode+edit mode -> switch back to default operation mode NUM_OP
		SVViewState vs = SVViewStateHandler::instance().viewState();
		if (selectedObjects.empty()) {
			vs.m_sceneOperationMode = SVViewState::NUM_OM;

			// vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
		}
		else {
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
			// Do not modify property widget mode
		}
		SVViewStateHandler::instance().setViewState(vs);

	} break;

	default:
		return; // do nothing by default
	} // switch

	// *** initialize camera placement and model placement in the world ***

	if (updateCamera) {
		QVector3D cameraTrans = IBKVector2QVector(SVProjectHandler::instance().viewSettings().m_cameraTranslation);
		m_camera.setTranslation(cameraTrans);
		m_camera.setRotation( SVProjectHandler::instance().viewSettings().m_cameraRotation.toQuaternion() );
	}

	// re-create grid with updated properties
	// since grid object is very small, this function also regenerates the grid line buffers and
	// uploads the data to the GPU
	if (updateGrid)
		m_gridObject.create(m_gridShader, project().m_viewSettings.m_gridPlanes);

	if (updateSelection) {
		// update selected objects
		m_selectedGeometryObject.create(m_fixedColorTransformShader);
		m_selectedGeometryObject.updateBuffers();
	}

	if (updateBuilding) {
		// create geometry object (if already existing, nothing happens here)
		m_buildingGeometryObject.create(m_buildingShader->shaderProgram()); // Note: does nothing, if already existing
		m_transparentBuildingObject.create(m_buildingShader);

		// transfer data from building geometry to vertex array caches
		const SVViewState & vs = SVViewStateHandler::instance().viewState();
		if (vs.inPropertyEditingMode())
			recolorObjects(vs.m_objectColorMode, vs.m_colorModePropertyID); // only changes color set in objects
		generateBuildingGeometry();
		generateTransparentBuildingGeometry();
	}

	// create network
	if (updateNetwork) {
		// In PropertyEditMode changes to the Network data usually require recoloring
		// of network objects. Hence, we recolor the objects here. Note, for buildings, re-coloring
		// operations are separate from building geometry modification operations.
		const SVViewState & vs = SVViewStateHandler::instance().viewState();
		if (vs.inPropertyEditingMode())
			recolorObjects(vs.m_objectColorMode, vs.m_colorModePropertyID); // only changes color set in objects
		// we use the same shader as for building elements
		m_networkGeometryObject.create(m_buildingShader->shaderProgram()); // Note: does nothing, if already existing

		// Fill vertex, color and index buffer data from network geometry and
		// transfer data to vertex array caches on GPU
		generateNetworkGeometry();
	}

	// update all GPU buffers (transfer cached data to GPU)
	if (updateBuilding || updateSelection) {
		m_buildingGeometryObject.updateBuffers();
		m_transparentBuildingObject.updateBuffers();
		m_surfaceNormalsObject.updateVertexBuffers();
	}

	if (updateNetwork || updateSelection)
		m_networkGeometryObject.updateBuffers();

	// store current coloring mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	m_lastColorMode = vs.m_objectColorMode;
}


void Scene::destroy() {
	m_gridObject.destroy();
	m_orbitControllerObject.destroy();
	m_buildingGeometryObject.destroy();
	m_transparentBuildingObject.destroy();
	m_networkGeometryObject.destroy();
	m_selectedGeometryObject.destroy();
	m_measurementObject.destroy();
	m_coordinateSystemObject.destroy();
	m_smallCoordinateSystemObject.destroy();
	m_newGeometryObject.destroy();
	m_newSubSurfaceObject.destroy();
	m_surfaceNormalsObject.destroy();
}


void Scene::resize(int width, int height, qreal retinaScale) {
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
	// Mind: do not use 0.0 for near plane, otherwise depth buffering and depth testing won't work!

	// the small view projection matrix is constant
	m_smallViewProjection.setToIdentity();
	// create projection matrix, i.e. camera lens
	m_smallViewProjection.perspective(
				/* vertical angle */ 45.0f,
				/* aspect ratio */   1, // it's a square window
				/* near */           0.1f,
				/* far */            farDistance
				);

	// update cached world2view matrix
	updateWorld2ViewMatrix();

	// update viewport
	m_viewPort = QRect(0, 0, static_cast<int>(width * retinaScale), static_cast<int>(height * retinaScale) );

	m_smallViewPort = QRect(0, 0, static_cast<int>(SUBWINDOWSIZE * retinaScale), static_cast<int>(SUBWINDOWSIZE * retinaScale) );
}


void Scene::updateWorld2ViewMatrix() {
	// transformation steps:
	//   model space -> transform -> world space
	//   world space -> camera/eye -> camera view
	//   camera view -> projection -> normalized device coordinates (NDC)
	m_worldToView = m_projection * m_camera.toMatrix(); // * m_transform.toMatrix();


	// update small coordinate system camera

	// take the current camera
	m_smallCoordinateSystemObject.m_smallViewCamera = m_camera;
	// move it into origin
	m_smallCoordinateSystemObject.m_smallViewCamera.setTranslation(QVector3D(0,0,0));
	// move 10 units backwards
	m_smallCoordinateSystemObject.m_smallViewCamera.translate( -6*m_smallCoordinateSystemObject.m_smallViewCamera.forward());
	// store in m_smallCoordinateSystemMatrix
	m_smallCoordinateSystemObject.m_worldToSmallView = m_smallViewProjection * m_smallCoordinateSystemObject.m_smallViewCamera.toMatrix();
}


bool Scene::inputEvent(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos, QPoint & newLocalMousePos) {
	//	QPoint lastLeftButtonPos;
	//	if (keyboardMouseHandler.buttonReleased(Qt::LeftButton))
	//		lastLeftButtonPos = mapFromGlobal(m_keyboardMouseHandler.mouseReleasePos());
	//	else
	//		localMousePos = mapFromGlobal(m_keyboardMouseHandler.mouseDownPos());

	// NOTE: In this function we handle only those keyboard inputs that affect the scene navigation.
	//       Single key release events are handled in Vic3DSceneView, since they do not require repeated screen redraws.

	bool needRepaint = false;

	Transform3D oldCameraTransform = m_camera;

	// *** Keyboard navigation ***

	// translation and rotation works always (no trigger key) unless the Ctrl-key is held at the same time
	// (to avoid the scene moving when pressing Ctrl+S
	if (!keyboardHandler.keyDown(Qt::Key_Control)) {

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

	}


	// To avoid duplicate picking operation, we create the pick object here.
	// Then, when we actually need picking, we check if the pick was already executed, and then only
	// retrieve the pick result values

	// we need to mind the pixel ratio when HighDPI Scaling is active
	// This is because localMousePos gives us sort of 'hardware' pixe
	// whereas we need 'logical' pixel
	PickObject pickObject(localMousePos * SVSettings::instance().m_ratio);

	// *** Mouse ***

	newLocalMousePos = localMousePos;
	// retrieve mouse delta
	QPoint mouseDelta = keyboardHandler.mouseDelta(QCursor::pos());
	//	qDebug() << mouseDelta << m_navigationMode;
	int mouse_dx = mouseDelta.x();
	int mouse_dy = mouseDelta.y();

	// if right mouse button is pressed, mouse delta is translated into first camera perspective rotations
	if (keyboardHandler.buttonDown(Qt::RightButton)) {
		// only do first-person mode, if not in any other mode
		if (m_navigationMode == NUM_NM) {
			m_navigationMode = NM_FirstPerson;
			qDebug() << "Entering first-person controller mode";
		}
		if (m_navigationMode == NM_FirstPerson) {
			// get and reset mouse delta (pass current mouse cursor position)
			const QVector3D LocalUp(0.0f, 0.0f, 1.0f); // same as in Camera::up()
			m_camera.rotate(-MOUSE_ROTATION_SPEED * mouse_dx, LocalUp);
			m_camera.rotate(-MOUSE_ROTATION_SPEED * mouse_dy, m_camera.right());
			// cursor wrap adjustment
			adjustCursorDuringMouseDrag(mouseDelta, localMousePos, newLocalMousePos, pickObject);
		}
	}

	// middle mouse button moves the geometry (panning)
	if (keyboardHandler.buttonDown(Qt::MidButton)) {
		// only do panning, if not in any other mode
		// qDebug() << m_camera.translation().x() << "\t" << m_camera.translation().y() << "\t" << m_camera.translation().z();

		if (m_navigationMode == NUM_NM) {
			// we enter pan mode
			panStart(localMousePos*SVSettings::instance().m_ratio, pickObject, false);
			// qDebug() << "Panning starts";
		}
		if ((mouseDelta != QPoint(0,0)) && (m_navigationMode == NM_Panning)) {

			// get the new far point, the point B'
			IBKMK::Vector3D newFarPoint = calculateFarPoint(localMousePos*SVSettings::instance().m_ratio, m_panOriginalTransformMatrix);

			IBKMK::Vector3D BBDashVec = m_panFarPointStart-newFarPoint;

			const IBKMK::Vector3D &lineOfSight = m_panObjectStart - m_panCameraStart;
			double dampening = std::min(1.0, (80.0 / lineOfSight.magnitude()));
//			qDebug() << "Dampening factor: " << dampening;

			if (keyboardHandler.keyDown(Qt::Key_Space))
				dampening = 1;

			IBKMK::Vector3D cameraTrans = dampening * m_panCABARatio*BBDashVec;

			// translate camera
			// qDebug() << "Camera translation";
			m_camera.setTranslation(IBKVector2QVector(m_panCameraStart + cameraTrans));
			// cursor wrap adjustment
//			qDebug() << "Adjust Dragging: " << mouseDelta << localMousePos << newLocalMousePos;
			adjustCursorDuringMouseDrag(mouseDelta, localMousePos, newLocalMousePos, pickObject);
		}
	}


	// left mouse button starts orbit controller (or interactive transformation), or performs a left-click
	if (keyboardHandler.buttonDown(Qt::LeftButton)) {

		// enter rubberBandMode only if not in any other mode
		if (keyboardHandler.keyDown(Qt::Key_Control)) {
			if (m_navigationMode == NUM_NM) {
				qDebug() << "Entering rubberband selection mode";
				m_navigationMode = NM_RubberbandSelection;
				m_rubberbandObject.setStartPoint(QVector3D(localMousePos.x(), localMousePos.y(), 0));
				return true;
			}
		}

		// only do orbit controller/interactive transformation mode, if not in any other mode
		if (m_navigationMode == NUM_NM) {

			// we may enter orbital controller mode, or start a transform operation

			// configure the pick object and pick a point on the XY plane/or any visible surface
			if (!pickObject.m_pickPerformed)
				pick(pickObject);

			// *** enter translation mode? ***
			if (pickObject.m_candidates.front().m_resultType == PickObject::RT_CoordinateSystemCenter) {
				m_navigationMode = NM_InteractiveTranslation;
				// Store origin of translation
				m_coordinateSystemObject.m_originalTranslation = m_coordinateSystemObject.translation();
				qDebug() << "Entering interactive translation mode";
			}

			// *** enter rotation or scale mode? ***
			else if (pickObject.m_candidates.front().m_resultType == PickObject::RT_AxisEndMarker) {
				// which mode is the coordinate system in?
				if (m_coordinateSystemObject.m_geometryTransformMode == Vic3D::CoordinateSystemObject::TM_RotateMask) {
					m_navigationMode = NM_InteractiveRotation;
					// which axis?
					switch (pickObject.m_candidates.front().m_objectID) {
					case 0 :
						// rotate around z
						m_coordinateSystemObject.m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_RotateZ;
						m_rotationAxis = QVector2IBKVector( m_coordinateSystemObject.localZAxis() );
						m_rotationVectorX = QVector2IBKVector( m_coordinateSystemObject.localXAxis() );
						m_rotationVectorY = QVector2IBKVector( m_coordinateSystemObject.localYAxis() );
						break;

					case 1 :
						m_coordinateSystemObject.m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_RotateX;
						m_rotationAxis = QVector2IBKVector( m_coordinateSystemObject.localXAxis() );
						m_rotationVectorX = QVector2IBKVector( m_coordinateSystemObject.localYAxis() );
						m_rotationVectorY = QVector2IBKVector( m_coordinateSystemObject.localZAxis() );
						break;

					case 2 :
						m_coordinateSystemObject.m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_RotateY;
						m_rotationAxis = QVector2IBKVector( m_coordinateSystemObject.localYAxis() );
						m_rotationVectorX = QVector2IBKVector( m_coordinateSystemObject.localZAxis() );
						m_rotationVectorY = QVector2IBKVector( m_coordinateSystemObject.localXAxis() );
						break;
					}

					// store original rotation matrix
					m_coordinateSystemObject.m_originalRotation = m_coordinateSystemObject.transform().rotation();
					// store rotation offset point
					m_coordinateSystemObject.m_originalTranslation = m_coordinateSystemObject.translation();

					//					// compute and store bounding box
					//					std::vector<const VICUS::Surface*> selSurfaces;
					//					std::vector<const VICUS::SubSurface*> selSubSurfaces;

					//					project().selectedSurfaces(selSurfaces, VICUS::Project::SG_All);
					//					project().boundingBox(selSurfaces, selSubSurfaces, m_boundingBoxCenterPoint);

					qDebug() << "Entering interactive rotation mode";
				}
				else if (m_coordinateSystemObject.m_geometryTransformMode == Vic3D::CoordinateSystemObject::TM_ScaleMask) {
					m_navigationMode = NM_InteractiveScaling;
					// which axis?
					switch (pickObject.m_candidates.front().m_objectID) {
					case 0 : m_coordinateSystemObject.m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_ScaleX; break;
					case 1 : m_coordinateSystemObject.m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_ScaleY; break;
					case 2 : m_coordinateSystemObject.m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_ScaleZ; break;
					}

					// store LCS rotation matrix
					m_coordinateSystemObject.m_originalRotation = m_coordinateSystemObject.transform().rotation();
					// store scale offset point
					m_coordinateSystemObject.m_originalTranslation = m_coordinateSystemObject.translation();
					// store 100% magnitude for scaling
					m_nominalScalingDistance = (pickObject.m_candidates.front().m_pickPoint - QVector2IBKVector(m_coordinateSystemObject.m_originalTranslation)).magnitude();
					// This is normally the distance of the axis cylinder from the origin: AXIS_LENGTH = 2 m

					qDebug() << "Entering interactive scaling mode";
					//					qDebug() << "1,0,0 -> rotated = " << m_coordinateSystemObject.m_originalRotation.rotatedVector(QVector3D(1,0,0));
				}
			}
			else {
				m_orbitControllerOrigin = IBKVector2QVector(pickObject.m_candidates.front().m_pickPoint);
				m_orbitControllerObject.m_transform.setTranslation(m_orbitControllerOrigin);

				m_mouseMoveDistance = 0;
				needRepaint = true;

				m_navigationMode = NM_OrbitController;
				qDebug() << "Entering orbit controller mode, rotation around" << m_orbitControllerOrigin;
			}
		}
		else {

			// ** left mouse button held and mouse dragged **
			if (mouseDelta != QPoint(0,0)) {

				if (m_navigationMode == NM_RubberbandSelection) {
					// update rubberband
					m_rubberbandObject.setRubberband(m_viewPort, QVector3D(localMousePos.x(), localMousePos.y(), 0));
					return true;
				}

				// record the distance that the mouse was moved
				m_mouseMoveDistance += mouse_dx*mouse_dx + mouse_dy*mouse_dy;

				switch (m_navigationMode) {
				case Vic3D::Scene::NM_Panning:
				case Vic3D::Scene::NM_FirstPerson:
				case Vic3D::Scene::NM_RubberbandSelection: // just to make compiler happy
				case Vic3D::Scene::NUM_NM:
					break; // for these modes, do nothing (can happen for multi-mouse-button-press and dragging)

				case NM_OrbitController : {
					// vector from pick point (center of orbit) to camera position
					QVector3D lineOfSight = m_camera.translation() - m_orbitControllerOrigin;

					// create a transformation object
					Transform3D orbitTrans;

					// mouse x translation = rotation around rotation axis

					double dampening = std::min(1.0, (80.0 / lineOfSight.length()));

					if (keyboardHandler.keyDown(Qt::Key_Space))
						dampening = 1;

//					qDebug() << "Dampening factor: " << dampening;

					const QVector3D GlobalUpwardsVector(0.0f, 0.0f, 1.0f);
					// set rotation around z axis for x-mouse-delta
					orbitTrans.rotate(MOUSE_ROTATION_SPEED * dampening * mouse_dx, GlobalUpwardsVector);

					// mouse y translation = rotation around "right" axis
					int mouseInversionFactor = SVSettings::instance().m_invertYMouseAxis ? -1 : 1;

					QVector3D LocalRight = m_camera.right().normalized();
					// set rotation around "right" axis for y-mouse-delta
					orbitTrans.rotate(MOUSE_ROTATION_SPEED * 0.5 * dampening * mouse_dy * mouseInversionFactor, LocalRight);

					// rotate vector to camera
					lineOfSight = orbitTrans.toMatrix() * lineOfSight;

					// rotate the camera around the same angles
					m_camera.rotate(MOUSE_ROTATION_SPEED * dampening * mouse_dx, GlobalUpwardsVector);
					m_camera.rotate(MOUSE_ROTATION_SPEED * 0.5 * dampening * mouse_dy * mouseInversionFactor, LocalRight);



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

					// move camera
					m_camera.setTranslation(newCamPos);

					// cursor wrap adjustment
					adjustCursorDuringMouseDrag(mouseDelta, localMousePos, newLocalMousePos, pickObject);
				} break; // orbit controller active

				case NM_InteractiveTranslation: {
					// pick a point and snap to some point in the scene
					if (!pickObject.m_pickPerformed)
						pick(pickObject);

					// now we handle the snapping rules and also the locking
					snapLocalCoordinateSystem(pickObject);

					//		qDebug() << localMousePos << IBKVector2QVector(o.m_pickPoint) << m_coordinateSystemObject.translation();

					// determine vector to snapped mouse position
					QVector3D newPoint = m_coordinateSystemObject.translation();
					// vector offset from starting point to current location
					QVector3D translationVector = newPoint - m_coordinateSystemObject.m_originalTranslation;
					// now set this in the wireframe object as translation
					m_selectedGeometryObject.translate(translationVector);


				} break;// interactive translation active


				case NM_InteractiveRotation: {
					// take the line-of-sight and compute intersection point with current rotation plane
					// rotation plane = rotation axis (e.g. TM_RotateX) and offset

					// store current's coordinate system position
					IBKMK::Vector3D coordinateSystemLocation = QVector2IBKVector( m_coordinateSystemObject.translation() );

					// pick a point and snap to some point in the scene
					if (!pickObject.m_pickPerformed)
						pick(pickObject); // TODO : if a performance hit, reduce to only compute line-of-sight

					IBKMK::Vector3D intersectionPoint;
					double dist;
					bool haveIntersection = IBKMK::linePlaneIntersection(coordinateSystemLocation, m_rotationAxis, /* rotation plane */
																		 pickObject.m_lineOfSightOffset, pickObject.m_lineOfSightDirection,
																		 intersectionPoint, dist);
					if (!haveIntersection) {
						IBK::IBK_Message("Cannot compute intersection between line of sight and rotation plane.");
					}

					if (haveIntersection) {
						// compute angle between intersection point and offset and rotation start point and offset
						// snap angle to 10 degrees, except if shift is pressed

						double x,y;
						bool res = planeCoordinates(coordinateSystemLocation, m_rotationVectorX, m_rotationVectorY,
													intersectionPoint, x, y);
						if (res) {
							double angle = std::atan2(y,x)/IBK::DEG2RAD;
							if (angle < 0)
								angle = 360 + angle;

							// snap angle
							if (!keyboardHandler.keyDown(Qt::Key_Shift))
								angle = std::floor(angle/10 + 0.5)*10;
							IBK::IBK_Message(IBK::FormatString("Rotation angle = %1 deg\n").arg(angle), IBK::MSG_PROGRESS);

							// compute rotation for local coordinate system
							QQuaternion q = QQuaternion::fromAxisAndAngle( IBKVector2QVector(m_rotationAxis), (float)angle);
							QQuaternion coordinateSystemRotation = q*m_coordinateSystemObject.m_originalRotation;

							// rotate local coordinate system (translation isn't needed)
							m_coordinateSystemObject.setRotation(coordinateSystemRotation);

							// determine new center point if selected geometry were rotated around origin
							IBKMK::Vector3D newCenter = QVector2IBKVector( q.rotatedVector( m_coordinateSystemObject.m_originalTranslation ) );
							// now rotate selected geometry and move it back into original center

							// now set this in the wireframe object as translation
							m_selectedGeometryObject.rotate(q, IBKVector2QVector(QVector2IBKVector(m_coordinateSystemObject.m_originalTranslation)-newCenter) );
						}
					}

				} break;// interactive translation active


				case NM_InteractiveScaling: {
					// pick a point and snap to some point in the scene
					if (!pickObject.m_pickPerformed)
						pick(pickObject);

					// now we handle the snapping rules and also the locking
					snapLocalCoordinateSystem(pickObject); // snap to axis

					double scalingDistance = (QVector2IBKVector(m_coordinateSystemObject.translation()) - QVector2IBKVector(m_coordinateSystemObject.m_originalTranslation)).magnitude();
					// now that we have the scaling distance, we reset the local coordinates location
					m_coordinateSystemObject.setTranslation(m_coordinateSystemObject.m_originalTranslation);

					// scale factor
					double scaleFactor = scalingDistance/m_nominalScalingDistance;
					//						qDebug() << "scaleFactor = " << scaleFactor;

					// compose local scale vector based on which local axis was selected
					QVector3D scaleVector(1,1,1);
					switch (m_coordinateSystemObject.m_geometryTransformMode) {
					case Vic3D::CoordinateSystemObject::TM_ScaleX : scaleVector.setX((float)scaleFactor); break;
					case Vic3D::CoordinateSystemObject::TM_ScaleY : scaleVector.setY((float)scaleFactor); break;
					case Vic3D::CoordinateSystemObject::TM_ScaleZ : scaleVector.setZ((float)scaleFactor); break;
					}

					// now set this in the wireframe object as translation
					m_selectedGeometryObject.localScaling(m_coordinateSystemObject.m_originalTranslation, m_coordinateSystemObject.m_originalRotation, scaleVector);
				} break;// interactive translation active

				} // switch
			} // mouse dragged
		}

	} // left button down


	// *** mouse button release ***

	if (keyboardHandler.buttonReleased(Qt::LeftButton)) {

		// check if the mouse was moved not very far -> we have a mouse click
		if (m_mouseMoveDistance < MOUSE_MOVE_DISTANCE_ORBIT_CONTROLLER) {
			handleLeftMouseClick(keyboardHandler, pickObject);
			needRepaint = true;
		}
		if (m_navigationMode == NM_OrbitController) {
			qDebug() << "Leaving orbit controller mode";
			needRepaint = true;
		}
		if (m_navigationMode == NM_InteractiveTranslation) {
			qDebug() << "Leaving interactive translation mode";
			if (SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr)
				SVViewStateHandler::instance().m_propEditGeometryWidget->finishTransformation();
			needRepaint = true;
		}
		if (m_navigationMode == NM_InteractiveRotation) {
			qDebug() << "Leaving interactive rotation mode";
			if (SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr)
				SVViewStateHandler::instance().m_propEditGeometryWidget->finishTransformation();
			needRepaint = true;
		}
		if (m_navigationMode == NM_InteractiveScaling) {
			qDebug() << "Leaving interactive scaling mode";
			if (SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr)
				SVViewStateHandler::instance().m_propEditGeometryWidget->finishTransformation();
			needRepaint = true;
		}
		if (m_navigationMode == NM_RubberbandSelection) {
			qDebug() << "Leaving rubberband selection mode";
			m_rubberbandObject.selectObjectsBasedOnRubberband();
			m_rubberbandObject.reset();
		}
		// clear orbit controller flag
		m_navigationMode = NUM_NM;


	} // left button released

	if (keyboardHandler.buttonReleased(Qt::MidButton)) {
		if (m_navigationMode == NM_Panning) {
			qDebug() << "Leaving panning mode";
			m_navigationMode = NUM_NM;
			m_panObjectDepth = 0.01; // reset pan object depth
		}
	}

	if (keyboardHandler.buttonReleased(Qt::RightButton)) {
		if (m_navigationMode == NM_FirstPerson) {
			qDebug() << "Leaving first-person controller mode";
			m_navigationMode = NUM_NM;
		}
	}


	// scroll wheel does fast zoom in/out
	int wheelDelta = keyboardHandler.wheelDelta();
	if (wheelDelta != 0) {

		// configure the pick object and pick a point on the XY plane/or any visible surface
		if (!pickObject.m_pickPerformed)
			pick(pickObject);

		// move forward along camera's forward vector
#ifdef Q_OS_MAC
		if (wheelDelta > 2)
			wheelDelta = 2;
		if (wheelDelta < -2)
			wheelDelta = -2;
#endif
		double transSpeed = 0.05;
		if (keyboardHandler.keyDown(Qt::Key_Shift))
			transSpeed *= 0.1;

		if (keyboardHandler.keyDown(Qt::Key_Space))
			transSpeed *= 10;

		IBKMK::Vector3D moveDist = transSpeed*pickObject.m_candidates.front().m_depth*pickObject.m_lineOfSightDirection;

		const IBKMK::Vector3D &lineOfSight = pickObject.m_candidates.front().m_depth*pickObject.m_lineOfSightDirection;
		double dampening = std::min(1.0, (80.0 / (lineOfSight.magnitude())));
		double moveFactor = std::max(1.0, (1.0 / (5 * moveDist.magnitude())));
		// qDebug() << "Dampening factor: " << dampening;
		// qDebug() << "Move distance: " << moveDist.magnitude();
		// move camera along line of sight towards selected object
		m_camera.translate(IBKVector2QVector(dampening*wheelDelta*moveFactor*moveDist));
	}

	// store camera position in view settings, but only if we have a project
	// Note: the check is necessary, because the paint event may be called as part of closing the window
	//       and updating the UI to the "no project" state
	if (SVProjectHandler::instance().isValid()) {
		SVProjectHandler::instance().viewSettings().m_cameraTranslation = QVector2IBKVector(m_camera.translation());
		SVProjectHandler::instance().viewSettings().m_cameraRotation = m_camera.rotation();
		// when camera has been moved, also update the local coordinate system
		m_coordinateSystemObject.updateCoordinateSystemSize();
	}
	updateWorld2ViewMatrix();
	// end of camera movement


	// *** adjusting the local coordinate system ***

	QVector3D oldPos = m_coordinateSystemObject.translation();
	// if in "place vertex" mode, perform picking operation and snap coordinate system to grid
	if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_PlaceVertex ||
			SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_MoveLocalCoordinateSystem)
	{
		// follow line of sign and determine possible objects to hit
		if (!pickObject.m_pickPerformed)
			pick(pickObject);

		// now we handle the snapping rules and also the locking
		snapLocalCoordinateSystem(pickObject);

		needRepaint = true;
		//		qDebug() << localMousePos << IBKVector2QVector(o.m_pickPoint) << m_coordinateSystemObject.translation();

		// update the movable coordinate system's location in the new polygon object
		QVector3D newPoint = m_coordinateSystemObject.translation();
		if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_PlaceVertex)
			m_newGeometryObject.updateLocalCoordinateSystemPosition(newPoint);
	}

	// measurement is updated in scene
	if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_MeasureDistance) {

		// follow line of sign and determine possible objects to hit
		if (!pickObject.m_pickPerformed)
			pick(pickObject);

		// now we handle the snapping rules and also the locking; updates local coordinate system location
		snapLocalCoordinateSystem(pickObject);
		m_measurementWidget->setLocalAxes(m_coordinateSystemObject.localXAxis(),
										  m_coordinateSystemObject.localYAxis(),
										  m_coordinateSystemObject.localZAxis());
		// initially, we have not start point so we show the local coordinate system's position
		if (m_measurementObject.m_startPoint == INVALID_POINT ) {
			m_measurementWidget->showStartPoint(m_coordinateSystemObject.translation() );
		}
		// while measuring, endPoint is QVector3D() and we update LCS position as end point
		// otherwise measurement has ended and we do no longer update any coordinates
		else if (m_measurementObject.m_endPoint == INVALID_POINT ) {
			// end point is the same as camera position
			m_measurementObject.setMeasureLine(m_coordinateSystemObject.translation(), m_camera.forward() );
			// update end point's coordinates and show distance in widget
			m_measurementWidget->showEndPoint(m_coordinateSystemObject.translation() );
		}
	}
	// if in "align coordinate system mode" perform picking operation and update local coordinate system orientation
	if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem) {
		// follow line of sign and determine possible objects to hit
		if (!pickObject.m_pickPerformed)
			pick(pickObject);
		needRepaint = true;


		// get nearest match; look for an object
		IBKMK::Vector3D nearestPoint;
		bool found = false;
		unsigned int uniqueID = 0;
		for (const PickObject::PickResult & r : pickObject.m_candidates) {
			if (r.m_resultType == PickObject::RT_Object) {
				nearestPoint = r.m_pickPoint;
				uniqueID = r.m_objectID;
				found = true;
				break;
			}
		}
		// no object found, try a plane
		if (!found) {
			for (const PickObject::PickResult & r : pickObject.m_candidates) {
				if (r.m_resultType == PickObject::RT_GridPlane) {
					nearestPoint = r.m_pickPoint;
					uniqueID = r.m_objectID;
					found = true;
					break;
				}
			}
		}
		// did we hit anything?
		if (found) {
			// object found?
			if (uniqueID != 0) {
				// lookup object
				const VICUS::Object * obj = project().objectById(uniqueID);
				// should be a surface
				const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(obj);
				// but maybe a nullptr, if we hit a sphere/cylinder from a network object
				if (s != nullptr) {
					// now get normal vector
					IBKMK::Vector3D n = s->geometry().normal();
					// get also local X and Y vectors
					IBKMK::Vector3D localX = s->geometry().localX();
					IBKMK::Vector3D localY = s->geometry().localY();
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
					//			*(QVector3D*)r = IBKVector2QVector(localX.normalized());
					//			r+=3;
					//			*(QVector3D*)r = IBKVector2QVector(localY.normalized());
					//			r+=3;
					//			*(QVector3D*)r = IBKVector2QVector(n.normalized());
					//			qDebug() << R;
					//			QQuaternion q = QQuaternion::fromRotationMatrix(R);
					//			qDebug() << q;

					// or use the ready-made Qt function (which surprisingly gives the same result :-)
					QQuaternion q2 = QQuaternion::fromAxes(IBKVector2QVector(localX.normalized()),
														   IBKVector2QVector(localY.normalized()),
														   IBKVector2QVector(n.normalized()));
					//			qDebug() << q2;
					m_coordinateSystemObject.setRotation(q2);
				}
			}
			else {
				// restore to global orientation
				m_coordinateSystemObject.setRotation(QQuaternion());
			}

			m_coordinateSystemObject.setTranslation( IBKVector2QVector(nearestPoint) );
		}
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


void Scene::render() {
	// we must have a valid project to render
	Q_ASSERT(SVProjectHandler::instance().isValid());

	glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

	// set the background color = clear color
	const SVSettings::ThemeSettings & s = SVSettings::instance().m_themeSettings[SVSettings::instance().m_theme];
	QVector3D backgroundColor = QtExt::QVector3DFromQColor(s.m_sceneBackgroundColor);

	glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), 1.0f);

	QVector3D viewPos = m_camera.translation();

	const SVViewState & vs = SVViewStateHandler::instance().viewState();

	// enable depth testing, important for the grid and for the drawing order of several objects
	// needed for all opaque geometry
	glEnable(GL_DEPTH_TEST);
	// enable depth mask update (only disabled for transparent geometry)
	glDepthMask(GL_TRUE);
	// turn off culling
	glDisable(GL_CULL_FACE);


	// --- Fixed Transform Shader Enabled ---

	m_fixedColorTransformShader->bind();
	m_fixedColorTransformShader->shaderProgram()->setUniformValue(m_fixedColorTransformShader->m_uniformIDs[0], m_worldToView);

	// *** selection object ***

	// do not render, if in subsurface mode
	if (vs.m_propertyWidgetMode != SVViewState::PM_AddSubSurfaceGeometry)
		m_selectedGeometryObject.render(); // might do nothing, if nothing is selected

	// *** orbit controller indicator ***

	if (m_navigationMode == NM_OrbitController && m_mouseMoveDistance > MOUSE_MOVE_DISTANCE_ORBIT_CONTROLLER) {
		// Note: uses also m_fixedColorTransformShader, which is already active with up-to-date worldToView matrix
		m_orbitControllerObject.render();
	}

	// *** new geometry object (opqaue lines) ***

	m_newGeometryObject.renderOpaque(); // might do nothing, if no geometry is being edited

	m_fixedColorTransformShader->release();

	// --- Fixed Transform Shader Disabled ---



	// *** opaque background geometry ***

	// tell OpenGL to show only faces whose normal vector points towards us
	glEnable(GL_CULL_FACE);

	// *** opaque building geometry ***

	m_buildingShader->bind();
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[0], m_worldToView);

	// Note: you can't use a QColor here directly and pass it as uniform to a shader expecting a vec3. Qt internally
	//       passes QColor as vec4. Use the converter QtExt::QVector3DFromQColor() for that.
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[2], QtExt::QVector3DFromQColor(m_lightColor));

	// set view position
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[1], viewPos);

	//#define FIXED_LIGHT_POSITION
#ifdef FIXED_LIGHT_POSITION
	// use a fixed light position
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[1], m_lightPos);
#else
	// use view position as light position
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[3], viewPos);
#endif // FIXED_LIGHT_POSITION

	// only render opaque building geometry, if not in InterlinkedSurfaces mode
	if (vs.m_objectColorMode != SVViewState::OCM_InterlinkedSurfaces) {

		// render network geometry
		m_networkGeometryObject.renderOpaque();

		// render opaque part of building geometry
		m_buildingGeometryObject.renderOpaque();

		// render opaque part of new sub-surface object
		if (vs.m_propertyWidgetMode == SVViewState::PM_AddSubSurfaceGeometry)
			m_newSubSurfaceObject.renderOpaque(); // might do nothing, if no sub-surface is being created
	}

	m_buildingShader->release();

	if (vs.m_objectColorMode != SVViewState::OCM_InterlinkedSurfaces) {
		// *** surface normals

		if (m_surfaceNormalsVisible) {
			m_surfaceNormalsShader->bind();
			m_surfaceNormalsShader->shaderProgram()->setUniformValue(m_surfaceNormalsShader->m_uniformIDs[0], m_worldToView);
			m_surfaceNormalsObject.render();
			m_surfaceNormalsShader->release();
		}
	}

	// *** grid ***

	if (m_gridObject.m_anyGridVisible) {
		m_gridShader->bind();
		m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[0], m_worldToView);
		m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[2], backgroundColor);
		float farDistance = (float)std::max(500., SVProjectHandler::instance().viewSettings().m_farDistance);
		m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[3], farDistance);
		m_gridObject.render();
		m_gridShader->release();
	}

	// *** transparent geometry ***

	glEnable(GL_BLEND);
	// For now disabled!
	glDepthMask(GL_FALSE); //disable z-testing

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 0.1);
	glEnable(GL_ALPHA_TEST);

	// tell OpenGL to show all planes
	glDisable(GL_CULL_FACE);
	// disable update of depth test but still use it

	if (vs.m_objectColorMode != SVViewState::OCM_InterlinkedSurfaces) {

		// ... windows, ...
		m_buildingShader->bind();
		if (m_buildingGeometryObject.canDrawTransparent() != 0) {
			m_buildingGeometryObject.renderTransparent();
		}
		if (vs.m_propertyWidgetMode == SVViewState::PM_AddSubSurfaceGeometry)
			m_newSubSurfaceObject.renderTransparent(); // might do nothing, if no subsurface is being constructed
		m_buildingShader->release();


		// *** new polygon draw object (transparent plane) ***

		if (m_newGeometryObject.canDrawTransparent() != 0) {
			m_fixedColorTransformShader->bind();
			// Note: worldToView uniform has already been set
			m_newGeometryObject.renderTransparent();
			m_fixedColorTransformShader->release();
		}
	}
	else {

		// render now transparent building geometry object
		m_buildingShader->bind();
		m_transparentBuildingObject.renderTransparent();
		m_buildingShader->release();

	}

	// turn off blending
	glDisable(GL_BLEND);
	// re-enable updating of z-buffer
	glDepthMask(GL_TRUE);
	// tell OpenGL to turn on culling
	glEnable(GL_CULL_FACE);

	// clear depth buffer because we want to paint on top of all
	glClear(GL_DEPTH_BUFFER_BIT);

	// *** movable coordinate system  ***

	bool drawLocalCoordinateSystem =
			vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex ||
			vs.m_sceneOperationMode == SVViewState::OM_SelectedGeometry ||
			vs.m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem ||
			vs.m_sceneOperationMode == SVViewState::OM_MoveLocalCoordinateSystem ||
			vs.m_sceneOperationMode == SVViewState::OM_MeasureDistance ;

	// do not draw LCS if we are in sub-surface mode
	if (vs.m_propertyWidgetMode == SVViewState::PM_AddSubSurfaceGeometry)
		drawLocalCoordinateSystem = false;
	if (drawLocalCoordinateSystem) {
		m_coordinateSystemShader->bind();

		// When we translate/rotate the local coordinate system, we actually move the world with respect to the camera
		// by left-multiplying the model2world matrix with the coordinate system object.

		// Suppose the local coordinate system shall be located at 20,0,0 and the camera looks at the coordinate
		// system's origin from 20,-40,2. Now, inside the shader, all coordinates are multiplied by the
		// model2world matrix, which basically moves all coordinates +20 in x direction. Now light and view position
		// (the latter only being used to compute the phong shading) are at 40, -40, 2, whereas the local coordinate
		// system is moved from local 0,0,0 to the desired 20,0,0.
		// Consequently, the light and view position cause the phong shader to draw the sphere as if lighted slightly
		// from the direction of positive x.

		// To fix this, we translate/rotate the view/light position inversely to the model2world transformation and
		// this reverts the effect introduced by the model2world matrix on the light/view coordinates.
		QVector3D translatedViewPos = m_coordinateSystemObject.inverseTransformationMatrix() * viewPos;
		//		qDebug() << viewPos << m_coordinateSystemObject.translation();

		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[0], m_worldToView);
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[1], translatedViewPos); // lightpos
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[2], QtExt::QVector3DFromQColor(m_lightColor));
		m_coordinateSystemShader->shaderProgram()->setUniformValue(m_coordinateSystemShader->m_uniformIDs[3], translatedViewPos); // viewpos
		m_coordinateSystemObject.renderOpaque();
		m_coordinateSystemShader->release();
	}
	else {
		SVViewStateHandler::instance().m_localCoordinateViewWidget->clearCoordinates();
	}

	// in measurement mode we always draw the line, except for the initial state where we are waiting for the first click
	if (vs.m_sceneOperationMode == SVViewState::OM_MeasureDistance && m_measurementObject.m_startPoint != INVALID_POINT) {

		// measurement line's coordinates are adjusted in inputEvent()
		m_measurementShader->bind();
		m_measurementShader->shaderProgram()->setUniformValue(m_measurementShader->m_uniformIDs[0], m_worldToView);
		m_measurementObject.render();
		m_measurementShader->release();
	}

	// clear depth buffer because we want to paint on top of all
	glClear(GL_DEPTH_BUFFER_BIT);
	// turn on blending
	glEnable(GL_BLEND);

	if (m_smallCoordinateSystemObjectVisible) {
		glViewport(m_smallViewPort.x(), m_smallViewPort.y(), m_smallViewPort.width(), m_smallViewPort.height());

		// blending is still enabled, so that should work
		m_smallCoordinateSystemObject.render();
	}

	if (m_navigationMode == NM_RubberbandSelection) {
		glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		// TODO : we don't really need a transformation matrix and can just use a shader without transform
		QMatrix4x4 mat;
		mat.setToIdentity();
		mat.ortho(  -m_viewPort.width()/2,  m_viewPort.width()/2, -m_viewPort.height()/2, m_viewPort.height()/2, -1.0f, 1.0f );

		m_rubberbandShader->bind();
		m_rubberbandShader->shaderProgram()->setUniformValue(m_rubberbandShader->m_uniformIDs[0], mat);
		m_rubberbandObject.render();
		m_rubberbandShader->release();

		glEnable(GL_DEPTH_TEST);

	}
}


void Scene::setViewState(const SVViewState & vs) {
	bool colorUpdateNeeded = false;
	if (vs.inPropertyEditingMode()) {
		// if we are currently constructing geometry, and we switch the view mode to
		// "Parameter edit" mode, reset the new polygon/geometry object
		m_newGeometryObject.clear();
		// update scene coloring if in property edit mode
		if (m_lastColorMode != vs.m_objectColorMode ||
				m_lastColorObjectID != vs.m_colorModePropertyID)
		{
			colorUpdateNeeded = true;
		}
	}
	else {
		//		Q_ASSERT(vs.m_objectColorMode == SVViewState::OCM_None);
		if (m_lastColorMode != vs.m_objectColorMode) {
			colorUpdateNeeded = true;
		}
	}

	// for results color view we need to update the color buffers _always_ as the colors have been
	// changed directly in the results property widget in SVPropResultsWidget::updateColors()
	if (vs.m_objectColorMode == SVViewState::OCM_ResultColorView)
		colorUpdateNeeded = true;

	if (colorUpdateNeeded) {

		// different update handling
		bool updateBuilding = false;
		bool updateNetwork = false;
		if (m_lastColorMode > 0 && m_lastColorMode < SVViewState::OCM_Network)
			updateBuilding = true;
		if (vs.m_objectColorMode > 0 && vs.m_objectColorMode < SVViewState::OCM_Network)
			updateBuilding = true;
		if (vs.m_objectColorMode >= SVViewState::OCM_ResultColorView)
			updateBuilding = true;
		if (m_lastColorMode >= SVViewState::OCM_Network)
			updateNetwork = true;
		if (vs.m_objectColorMode >= SVViewState::OCM_Network)
			updateNetwork = true;
		if (vs.m_objectColorMode >= SVViewState::OCM_ResultColorView)
			updateNetwork = true;

		// update the color properties in the data objects (does not update GPU memory!)
		recolorObjects(vs.m_objectColorMode, vs.m_colorModePropertyID);

		if (updateBuilding) {
			qDebug() << "Updating surface coloring of buildings";
			generateBuildingGeometry();
			generateTransparentBuildingGeometry();
			// TODO : Andreas, Performance update, only update affected part of color buffer
			m_buildingGeometryObject.updateColorBuffer();
			m_transparentBuildingObject.updateColorBuffer();
		}
		if (updateNetwork) {
			qDebug() << "Updating surface coloring of networks";
			// TODO : Andreas, Performance update, only update and transfer color buffer
			generateNetworkGeometry();
			m_networkGeometryObject.updateBuffers();
		}

		m_lastColorMode = vs.m_objectColorMode;
		m_lastColorObjectID = vs.m_colorModePropertyID;
	}
}


void Scene::refreshColors() {
	Q_ASSERT(SVProjectHandler::instance().isValid()); // must have a valid project

	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	recolorObjects(vs.m_objectColorMode, vs.m_colorModePropertyID);
	qDebug() << "Updating surface coloring of buildings";
	// TODO : Andreas, Performance update, only update and transfer color buffer
	generateBuildingGeometry();
	generateTransparentBuildingGeometry();
	m_buildingGeometryObject.updateColorBuffer();
	m_transparentBuildingObject.updateColorBuffer();

	qDebug() << "Updating surface coloring of networks";
	// TODO : Andreas, Performance update, only update and transfer color buffer
	generateNetworkGeometry();
	m_networkGeometryObject.updateBuffers();
}


void Scene::generateBuildingGeometry() {
	//	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	//	// when we show transparent building, we do not need to update the building geometry
	//	// when we switch object color mode, this function will be called again anyways
	//	if (vs.m_objectColorMode == SVViewState::OCM_InterlinkedSurfaces)
	//		return;

	QElapsedTimer t;
	t.start();
	// get VICUS project data
	const VICUS::Project & p = project();

	// we rebuild the entire geometry here, so this may be slow

	// clear out existing cache

	m_buildingGeometryObject.m_vertexBufferData.clear();
	m_buildingGeometryObject.m_colorBufferData.clear();
	m_buildingGeometryObject.m_indexBufferData.clear();
	m_buildingGeometryObject.m_vertexStartMap.clear();

	m_buildingGeometryObject.m_vertexBufferData.reserve(100000);
	m_buildingGeometryObject.m_colorBufferData.reserve(100000);
	m_buildingGeometryObject.m_indexBufferData.reserve(100000);

	// we want to draw triangles
	m_buildingGeometryObject.m_drawTriangleStrips = false;

	// we now process all surfaces and add their coordinates and
	// normals

	// recursively process all buildings, building levels etc.

	const SVDatabase & db = SVSettings::instance().m_db;

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	// set collects pointers to all visible, not selected sub-surfaces
	std::vector<std::pair<const VICUS::SubSurface *, const VICUS::PlaneTriangulationData*> > transparentSubsurfaces;

	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				for (const VICUS::Surface & s : r.m_surfaces) {

					// remember where the vertexes for this surface start in the buffer
					m_buildingGeometryObject.m_vertexStartMap[s.m_id] = currentVertexIndex;

					// now we store the surface data into the vertex/color and index buffers
					// the indexes are advanced and the buffers enlarged as needed.
					// actually, this adds always two surfaces (for culling).
					addSurface(s, currentVertexIndex, currentElementIndex,
							   m_buildingGeometryObject.m_vertexBufferData,
							   m_buildingGeometryObject.m_colorBufferData,
							   m_buildingGeometryObject.m_indexBufferData);

					// process all subsurfaces, add opaque surfaces but remember transparent surfaces for later
					for (unsigned int i=0; i<s.subSurfaces().size(); ++i) {
						const VICUS::SubSurface & sub = s.subSurfaces()[i];
						if (sub.m_subSurfaceComponentInstance != nullptr &&
								sub.m_subSurfaceComponentInstance->m_idSubSurfaceComponent != VICUS::INVALID_ID)
						{
							// lookup subsurface component - if it exists
							const VICUS::SubSurfaceComponent * comp = db.m_subSurfaceComponents[sub.m_subSurfaceComponentInstance->m_idSubSurfaceComponent];
							if (comp != nullptr) {
								// now select transparent or opaque surface based on type
								if (comp->m_type == VICUS::SubSurfaceComponent::CT_Window) {
									// only add to transparent subsurfaces if visible and not selected
									if (sub.m_visible && !sub.m_selected && !s.geometry().holeTriangulationData().empty())
										transparentSubsurfaces.push_back(std::make_pair(&sub, &s.geometry().holeTriangulationData()[i]) );
									continue; // next surface
								}
							}
						}

						// not a transparent surface, just add surface as opaque surface
						addSubSurface(s, i, currentVertexIndex, currentElementIndex,
									  m_buildingGeometryObject.m_vertexBufferData,
									  m_buildingGeometryObject.m_colorBufferData,
									  m_buildingGeometryObject.m_indexBufferData);
					}
				}
			}
		}
	}

	// now the plain geometry
	for (const VICUS::Surface & s : p.m_plainGeometry.m_surfaces) {

		// remember where the vertexes for this surface start in the buffer
		m_buildingGeometryObject.m_vertexStartMap[s.m_id] = currentVertexIndex;

		// now we store the surface data into the vertex/color and index buffers
		// the indexes are advanced and the buffers enlarged as needed.
		// actually, this adds always two surfaces (for culling).
		addSurface(s, currentVertexIndex, currentElementIndex,
				   m_buildingGeometryObject.m_vertexBufferData,
				   m_buildingGeometryObject.m_colorBufferData,
				   m_buildingGeometryObject.m_indexBufferData);
	}

	// done with all opaque planes, remember start index for transparent geometry
	m_buildingGeometryObject.m_transparentStartIndex = m_buildingGeometryObject.m_indexBufferData.size();

	// now add all transparent surfaces
	for (std::pair<const VICUS::SubSurface *, const VICUS::PlaneTriangulationData*> & p : transparentSubsurfaces) {
		QColor col = p.first->m_color;

		const VICUS::PlaneTriangulationData* triangu = dynamic_cast<const VICUS::PlaneTriangulationData*>(p.second);

		if(triangu == nullptr)
			continue;

		addPlane(*triangu, col, currentVertexIndex, currentElementIndex,
				 m_buildingGeometryObject.m_vertexBufferData,
				 m_buildingGeometryObject.m_colorBufferData,
				 m_buildingGeometryObject.m_indexBufferData, false);
	}

	if (t.elapsed() > 20)
		qDebug() << t.elapsed() << "ms for building generation";
}



void Scene::generateTransparentBuildingGeometry(const HighlightingMode &mode) {
	FUNCID(Scene::generateTransparentBuildingGeometry);

	//	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	//	if (vs.m_objectColorMode != SVViewState::OCM_InterlinkedSurfaces)
	//		return;

	QElapsedTimer t;
	t.start();
	// get VICUS project data
	const VICUS::Project & p = project();

	// we rebuild the entire geometry here, so this may be slow
	// clear out existing cache

	m_transparentBuildingObject.m_vertexBufferData.clear();
	m_transparentBuildingObject.m_colorBufferData.clear();
	m_transparentBuildingObject.m_indexBufferData.clear();
	m_transparentBuildingObject.m_vertexStartMap.clear();

	m_transparentBuildingObject.m_vertexBufferData.reserve(100000);
	m_transparentBuildingObject.m_colorBufferData.reserve(100000);
	m_transparentBuildingObject.m_indexBufferData.reserve(100000);

	// we now process all surfaces and add their coordinates and normals

	// recursively process all buildings, building levels etc.

	const SVDatabase & db = SVSettings::instance().m_db;

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	// coloring rules
	// - surfaces without connections to other surfaces are drawn in very transparent light gray
	// - surfaces with connections to other surfaces are drawn in slightly darker, transparent gray
	// - subsurfaces are drawn in very dark, very transparent gray
	// - active surface is drawn in silver, little transparent only

	// generate a list of surfaces that are connected
	std::map<const VICUS::Surface *, const VICUS::Surface *> connectedSurfaces;
	for (const VICUS::ComponentInstance & ci : p.m_componentInstances) {
		if (ci.m_sideASurface == nullptr || ci.m_sideBSurface == nullptr)
			continue;

		// remember connected surfaces
		connectedSurfaces[ci.m_sideASurface] = ci.m_sideBSurface;
		connectedSurfaces[ci.m_sideBSurface] = ci.m_sideASurface;

		// both polygons must be valid to continue, otherwise we cannot draw a connection line
		// invalid polygons should be highlighted somewhere else

		if (!ci.m_sideASurface->geometry().isValid() ||
				!ci.m_sideBSurface->geometry().isValid())
			continue;

		// both polygons must be visible
		if (!ci.m_sideASurface->m_visible || !ci.m_sideBSurface->m_visible)
			continue;

		// generate geometry for the "Links"

		// we need to generate the "centerpoint" of the polygon. Then we move along the local coordinate systems
		// direction of the plane in 0.1 cm x 0.1 cm

	}

	std::set<VICUS::Surface *> handledSurfaces;
	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				for (const VICUS::Surface & s : r.m_surfaces) {

					// remember where the vertexes for this surface start in the buffer
					m_transparentBuildingObject.m_vertexStartMap[s.m_id] = currentVertexIndex;
					// TODO : also store average depth of polygon

					// general slightly gray - strong transparent
					QColor col(255,255,255,20);
					if (connectedSurfaces.find(&s) != connectedSurfaces.end()) {
						if(m_surfaceColor.find(s.m_id) != m_surfaceColor.end()) {
							col = m_surfaceColor[s.m_id];

							const VICUS::Surface &s2 = *connectedSurfaces[&s];

							if(!s.m_visible && !s2.m_visible)
								continue;

							const double BOX_THICKNESS = 0.05;
							IBKMK::Vector3D s1center = s.geometry().centerPoint();
							std::vector<IBKMK::Vector3D> v1;
							v1.push_back(s1center + s.geometry().localX()*BOX_THICKNESS + s.geometry().localY()*BOX_THICKNESS);
							v1.push_back(s1center + s.geometry().localX()*BOX_THICKNESS - s.geometry().localY()*BOX_THICKNESS);
							v1.push_back(s1center - s.geometry().localX()*BOX_THICKNESS - s.geometry().localY()*BOX_THICKNESS);
							v1.push_back(s1center - s.geometry().localX()*BOX_THICKNESS + s.geometry().localY()*BOX_THICKNESS);

							IBKMK::Vector3D s2center = s2.geometry().centerPoint();
							std::vector<IBKMK::Vector3D> v2;
							v2.push_back(s2center + s2.geometry().localX()*BOX_THICKNESS + s2.geometry().localY()*BOX_THICKNESS);
							v2.push_back(s2center + s2.geometry().localX()*BOX_THICKNESS - s2.geometry().localY()*BOX_THICKNESS);
							v2.push_back(s2center - s2.geometry().localX()*BOX_THICKNESS - s2.geometry().localY()*BOX_THICKNESS);
							v2.push_back(s2center - s2.geometry().localX()*BOX_THICKNESS + s2.geometry().localY()*BOX_THICKNESS);

							// make sure both polygons "rotate" into the same direction
							IBKMK::Vector3D n1 = (v1[0] - v1[1]).crossProduct(v1[2]-v1[1]);
							IBKMK::Vector3D n2 = (v2[0] - v2[1]).crossProduct(v2[2]-v2[1]);
							IBKMK::Vector3D centerLine = s2center - s1center;

							// make sure normals of both polygons point along center line
							if (centerLine.scalarProduct(n1) < 0)
								std::reverse(v1.begin(), v1.end());
							if (centerLine.scalarProduct(n2) < 0)
								std::reverse(v2.begin(), v2.end());

							// now determine rotation offset for second polygon until both rects are aligned
							// this can be checked by computing the sum of the vertex distances and pick the combination that is shortest
							unsigned int shortestOffset = 0;
							double len = std::numeric_limits<double>::max();
							for (unsigned int k=0; k<4; ++k) {
								double distSum = 0;
								for (unsigned int i=0; i<4; ++i) {
									double dist = v1[i].distanceTo(v2[(i + k) % 4]);
									distSum += dist;
								}
								if (distSum < len) {
									shortestOffset = k;
									len = distSum;
								}
							}

							// now rotation v2 by the offset
							std::vector<IBKMK::Vector3D> v2mod;
							if (shortestOffset == 0) {
								v2mod.swap(v2);
							}
							else {
								for (unsigned int i=0; i<4; ++i)
									v2mod.push_back(v2[(i+shortestOffset) % 4]);
							}

							v1.insert(v1.end(), v2mod.begin(), v2mod.end()); // combine polygons

							// add geometry to buffer
							QColor linkBoxColor = col;
//                            if(mode == HM_ColoredSurfaces) {
//                                linkBoxColor.setAlpha(255);
//                            } else {
//                            }
							linkBoxColor = QColor(196,0,0,226);
							if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
								linkBoxColor = QColor(255,64,32,226);

							addBox(v1, linkBoxColor, currentVertexIndex, currentElementIndex,
								   m_transparentBuildingObject.m_vertexBufferData,
								   m_transparentBuildingObject.m_colorBufferData,
								   m_transparentBuildingObject.m_indexBufferData);
						} else {
							if(mode == HM_ColoredSurfaces) {
								int r = QRandomGenerator::global()->generate()%255;
								int g = QRandomGenerator::global()->generate()%255;
								int b = QRandomGenerator::global()->generate()%255;

								col = QColor(r,g,b,200);
							} else {
								col = QColor(255,255,255,64);
							}

							m_surfaceColor[connectedSurfaces[&s]->m_id] = col;
							m_surfaceColor[s.m_id] = col;
						}
					}

					//col = QColor(64,64,64,32);s
					if (s.m_visible && !s.m_selected) {
						addPlane(s.geometry().triangulationData(), col, currentVertexIndex, currentElementIndex,
								 m_transparentBuildingObject.m_vertexBufferData,
								 m_transparentBuildingObject.m_colorBufferData,
								 m_transparentBuildingObject.m_indexBufferData, false);
					}

					col = QColor(64,64,64,32); // for subsurfaces
					// process all subsurfaces
					for (unsigned int i=0; i<s.subSurfaces().size(); ++i) {
						const VICUS::SubSurface & sub = s.subSurfaces()[i];
						if (!sub.m_visible || sub.m_selected)
							continue;
						if (sub.m_subSurfaceComponentInstance != nullptr &&
								sub.m_subSurfaceComponentInstance->m_idSubSurfaceComponent != VICUS::INVALID_ID)
						{
							// lookup subsurface component - if it exists
							const VICUS::SubSurfaceComponent * comp = db.m_subSurfaceComponents[sub.m_subSurfaceComponentInstance->m_idSubSurfaceComponent];
							if (comp != nullptr) {

								bool foundHole = false;
								unsigned int idx = 0;
								// Find correct hole index
								for(;idx<s.geometry().holes().size(); ++idx) {
									const VICUS::PlaneGeometry::Hole & h = s.geometry().holes()[idx];
									if(!h.m_isChildSurface && h.m_idObject == sub.m_id) {
										foundHole = true;
										break;
									}
								}

								if(!foundHole) {
									IBK::IBK_Message(IBK::FormatString("Corresponding hole triangulation data of sub-surface '%1' was not found!").arg(sub.m_displayName.toStdString()), IBK::MSG_ERROR, FUNC_ID);
									continue;
								}

								if(s.geometry().holeTriangulationData().empty())
									continue;

								addPlane(s.geometry().holeTriangulationData()[idx], col, currentVertexIndex, currentElementIndex,
										 m_transparentBuildingObject.m_vertexBufferData,
										 m_transparentBuildingObject.m_colorBufferData,
										 m_transparentBuildingObject.m_indexBufferData, false);
							}
						}
					}
				}
			}
		}
	}



	if (t.elapsed() > 20)
		qDebug() << t.elapsed() << "ms for transparent geometry generation";
}


void Scene::generateNetworkGeometry() {
	QElapsedTimer t;
	t.start();
	// get VICUS project data and database
	const VICUS::Project & p = project();
	const SVDatabase & db = SVSettings::instance().m_db;

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

		// update pointers of network elements, they are only runtime and not part of the project data
		// so they can be modified with const_cast
		const_cast<VICUS::Network &>(network).updateNodeEdgeConnectionPointers();
		// the visualization radius is also only runtime
		const_cast<VICUS::Network &>(network).updateVisualizationRadius(db.m_pipes);

		for (const VICUS::NetworkEdge & e : network.m_edges) {
			double radius = e.m_visualizationRadius;
			QColor pipeColor = e.m_color;
			if (!e.m_visible || e.m_selected)
				pipeColor.setAlpha(0);

			m_networkGeometryObject.m_vertexStartMap[e.m_id] = currentVertexIndex;
			addCylinder(e.m_node1->m_position, e.m_node2->m_position, pipeColor, radius,
						currentVertexIndex, currentElementIndex,
						m_networkGeometryObject.m_vertexBufferData,
						m_networkGeometryObject.m_colorBufferData,
						m_networkGeometryObject.m_indexBufferData);
		}

		// add spheres for nodes
		for (const VICUS::NetworkNode & no : network.m_nodes) {
			double radius = no.m_visualizationRadius;
			QColor col = no.m_color;
			if (!no.m_visible || !network.m_visible)
				col.setAlpha(0);

			m_networkGeometryObject.m_vertexStartMap[no.m_id] = currentVertexIndex;
			addSphere(no.m_position, col, radius,
					  currentVertexIndex, currentElementIndex,
					  m_networkGeometryObject.m_vertexBufferData,
					  m_networkGeometryObject.m_colorBufferData,
					  m_networkGeometryObject.m_indexBufferData);
		}
	}

	// done with all opaque geometry, remember start index for transparent geometry
	m_networkGeometryObject.m_transparentStartIndex = m_networkGeometryObject.m_indexBufferData.size();

	if (t.elapsed() > 20)
		qDebug() << t.elapsed() << "ms for network generation";
}


void Scene::recolorObjects(SVViewState::ObjectColorMode ocm, unsigned int id) const {
	// Note: the meaning of the filter id depends on the coloring mode

	// In ResultColorView the colors of nodes, edges, surfaces etc. are exclusively controlled
	// by the property widget SVPropResultsWidget. Hence we just return here.
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	if (vs.m_objectColorMode == SVViewState::OCM_ResultColorView)
		return;


	// get VICUS project data
	const VICUS::Project & p = project();

	// coloring works as follows:
	// - we process all surfaces and sub-surfaces
	// - if we are not in a coloring mode (OCM_None) we select a default color for the surface/sub-surface/node/edge
	//   (this is done always as default initialization step)
	// - if we are in a coloring mode, all surfaces except those we need to color are grayed out

	// process all surfaces and initialize colors
	for (const VICUS::Building & b : p.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				for (const VICUS::Surface & s : r.m_surfaces) {

					// update surface opaque color
					// here, it does not matter if the surfaces is selected or invisible -
					// we just update the color that would be used if the surface would be drawn

					if (ocm == SVViewState::OCM_None) {
						// const cast is ok here, since color is not a project property
						if (!s.m_displayColor.isValid())
							const_cast<VICUS::Surface&>(s).initializeColorBasedOnInclination();
						const_cast<VICUS::Surface&>(s).m_color = s.m_displayColor;
					}
					else {
						// set "not interested/not applicable" color
						s.m_color = QColor(64,64,64,255); // dark opaque gray
					}

					// now the subsurfaces
					for (const VICUS::SubSurface & sub : s.subSurfaces()) {
						if (ocm == SVViewState::OCM_None) {
							// const cast is ok here, since color is not a project property
							const_cast<VICUS::SubSurface&>(sub).updateColor();
						}
						else {
							// set "not interested/not applicable" color
							sub.m_color = QColor(72,72,82,192); // will be drawn opaque in most modes
						}
					}
				}
			}
		}
	}

	// initialize network colors, independent of current color mode
	for (const VICUS::Network & net: p.m_geometricNetworks) {
		// we only update runtime-only properties, hence we can use a const-cast here
		const_cast<VICUS::Network&>(net).setDefaultColors();
	}

	const SVDatabase & db = SVSettings::instance().m_db;

	// different handling different color modes
	// Mind: the subsurfaces are drawn either transparent or opaque, depending on the associated
	//       component. Without a subsurface-component assigned, they will be opaque and should
	//       have a different color than regular surfaces without component

	switch (ocm) {
	case SVViewState::OCM_InterlinkedSurfaces:
	case SVViewState::OCM_None:
		break;


		// *** OCM_SelectedSurfacesHighlighted
	case SVViewState::OCM_SelectedSurfacesHighlighted: {
		for (const VICUS::Building & b : p.m_buildings) {
			for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
				for (const VICUS::Room & r : bl.m_rooms) {
					for (const VICUS::Surface & s : r.m_surfaces) {

						// change color of selected surfaces
						if (s.m_selected)
							s.m_color = QColor(255,144,0,255); // nice orange
					}
				}
			}
		}
	} break;


		// *** OCM_Components
	case SVViewState::OCM_Components: {
		// now color all surfaces that appear somewhere in a ComponentInstance
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			QColor col;
			if (ci.m_idComponent == VICUS::INVALID_ID)
				col = QColor(96,0,0);
			else {
				// lookup component definition
				const VICUS::Component * comp = db.m_components[ci.m_idComponent];
				if (comp == nullptr)
					col = QColor(148,64,64);
				else
					col = comp->m_color;
			}
			if (ci.m_sideASurface != nullptr)
				ci.m_sideASurface->m_color = col;
			if (ci.m_sideBSurface != nullptr)
				ci.m_sideBSurface->m_color = col;
		}
	} break;


		// *** OCM_ComponentOrientation
	case SVViewState::OCM_ComponentOrientation: {
		// now color all surfaces, this works by first looking up the components, associated with each surface
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			// lookup component definition
			const VICUS::Component * comp = db.m_components[ci.m_idComponent];
			if (comp == nullptr)
				continue; // no component definition - keep default (gray) color

			// color surfaces when either filtering is off (id == 0)
			// or when component ID matches selected id
			if (id == VICUS::INVALID_ID || ci.m_idComponent == id) {
				// color side A surfaces with blue,
				// side B surfaces with orange
				if (ci.m_sideASurface != nullptr)
					ci.m_sideASurface->m_color = QColor(47,125,212);
				if (ci.m_sideBSurface != nullptr)
					ci.m_sideBSurface->m_color = QColor(255, 206, 48);
			}
		}

		// now color all sub-surfaces, this works by first looking up the components, associated with each surface
		for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
			// lookup component definition
			const VICUS::SubSurfaceComponent * comp = db.m_subSurfaceComponents[ci.m_idSubSurfaceComponent];
			if (comp == nullptr)
				continue; // no component definition - keep default (uninterested) color

			// color surfaces when either filtering is off (id == 0)
			// or when component ID matches selected id
			if (id == VICUS::INVALID_ID || ci.m_idSubSurfaceComponent == id) {
				// color side A surfaces with blue,
				// side B surfaces with orange
				// colors slightly brighter than components, to allow differntiation
				if (ci.m_sideASubSurface != nullptr)
					ci.m_sideASubSurface->m_color = QColor(92,149,212, 128); // set slightly transparent to have effect on windows
				if (ci.m_sideBSubSurface != nullptr)
					ci.m_sideBSubSurface->m_color = QColor(255, 223, 119, 128); // set slightly transparent to have effect on windows
			}
		}
	} break;


		// *** OCM_SubSurfaceComponents
	case SVViewState::OCM_SubSurfaceComponents: {
		// now color all sub-surfaces, this works by first looking up the components, associated with each surface
		for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
			// lookup component definition
			const VICUS::SubSurfaceComponent * comp = db.m_subSurfaceComponents[ci.m_idSubSurfaceComponent];
			if (comp == nullptr)
				continue; // no component definition - keep default (uninterested) color
			if (ci.m_sideASubSurface != nullptr) {
				ci.m_sideASubSurface->m_color = comp->m_color;
				// TODO : decide upon alpha value based on component type
				ci.m_sideASubSurface->m_color.setAlpha(128);
			}
			if (ci.m_sideBSubSurface != nullptr) {
				ci.m_sideBSubSurface->m_color = comp->m_color;
				ci.m_sideBSubSurface->m_color.setAlpha(128);
			}
		}
	} break;


		// *** OCM_SurfaceHeating
	case SVViewState::OCM_SurfaceHeating: {
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			// lookup component definition
			const VICUS::Component * comp = db.m_components[ci.m_idComponent];
			if (comp == nullptr)
				continue; // no component definition - keep default (gray) color
			// lookup surface heating definition
			const VICUS::SurfaceHeating * surfHeat = db.m_surfaceHeatings[ci.m_idSurfaceHeating];
			if (surfHeat != nullptr) {
				if (ci.m_sideASurface != nullptr)
					ci.m_sideASurface->m_color = surfHeat->m_color;
				if (ci.m_sideBSurface != nullptr)
					ci.m_sideBSurface->m_color = surfHeat->m_color;
			}
			else {
				if (comp->m_activeLayerIndex != VICUS::INVALID_ID) {
					if (ci.m_sideASurface != nullptr)
						ci.m_sideASurface->m_color = QColor("#758eb3");
					if (ci.m_sideBSurface != nullptr)
						ci.m_sideBSurface->m_color = QColor("#758eb3");
				}

			}
		}
	} break;


		// *** OCM_BoundaryConditions
	case SVViewState::OCM_BoundaryConditions: {
		// now color all surfaces, this works by first looking up the components, associated with each surface
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			// lookup component definition
			const VICUS::Component * comp = db.m_components[ci.m_idComponent];
			if (comp == nullptr)
				continue; // no component definition - keep default (gray) color
			if (ci.m_sideASurface != nullptr && comp->m_idSideABoundaryCondition != VICUS::INVALID_ID) {
				// lookup boundary condition definition
				const VICUS::BoundaryCondition * bc = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
				if (bc != nullptr)
					ci.m_sideASurface->m_color = bc->m_color;
			}
			if (ci.m_sideBSurface != nullptr && comp->m_idSideBBoundaryCondition != VICUS::INVALID_ID) {
				// lookup boundary condition definition
				const VICUS::BoundaryCondition * bc = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
				if (bc != nullptr)
					ci.m_sideBSurface->m_color = bc->m_color;
			}
		}
		// now color all sub-surfaces, this works by first looking up the components, associated with each surface
		for (const VICUS::SubSurfaceComponentInstance & ci : project().m_subSurfaceComponentInstances) {
			// lookup component definition
			const VICUS::SubSurfaceComponent * comp = db.m_subSurfaceComponents[ci.m_idSubSurfaceComponent];
			if (comp == nullptr)
				continue; // no component definition - keep default (uninterested) color
			if (ci.m_sideASubSurface != nullptr && comp->m_idSideABoundaryCondition != VICUS::INVALID_ID) {
				// lookup boundary condition definition
				const VICUS::BoundaryCondition * bc = db.m_boundaryConditions[comp->m_idSideABoundaryCondition];
				if (bc != nullptr) {
					ci.m_sideASubSurface->m_color = bc->m_color.lighter(50);
					ci.m_sideASubSurface->m_color.setAlpha(128);
				}
			}
			if (ci.m_sideBSubSurface != nullptr && comp->m_idSideBBoundaryCondition != VICUS::INVALID_ID) {
				// lookup boundary condition definition
				const VICUS::BoundaryCondition * bc = db.m_boundaryConditions[comp->m_idSideBBoundaryCondition];
				if (bc != nullptr) {
					ci.m_sideBSubSurface->m_color = bc->m_color.lighter(50);
					ci.m_sideBSubSurface->m_color.setAlpha(128);
				}
			}
		}
	} break;


		// *** OCM_SupplySystems
	case SVViewState::OCM_SupplySystems: {
		// now color all surfaces, this works by first looking up the components, associated with each surface
		for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
			// lookup component definition
			const VICUS::Component * comp = db.m_components[ci.m_idComponent];
			if (comp == nullptr)
				continue; // no component definition - keep default (gray) color
			// lookup surface heating definition
			const VICUS::SupplySystem * supplySys = db.m_supplySystems[ci.m_idSupplySystem];
			if (supplySys != nullptr) {
				if (ci.m_sideASurface != nullptr)
					ci.m_sideASurface->m_color = supplySys->m_color;
				if (ci.m_sideBSurface != nullptr)
					ci.m_sideBSurface->m_color = supplySys->m_color;
			}
			else {
				if (comp->m_activeLayerIndex != VICUS::INVALID_ID) {
					if (ci.m_sideASurface != nullptr)
						ci.m_sideASurface->m_color = QColor("#758eb3");
					if (ci.m_sideBSurface != nullptr)
						ci.m_sideBSurface->m_color = QColor("#758eb3");
				}

			}
		}
	} break;


		// *** OCM_ZoneTemplates
	case SVViewState::OCM_ZoneTemplates: {
		for (const VICUS::Building & b : p.m_buildings) {
			for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
				for (const VICUS::Room & r : bl.m_rooms) {
					// skip all without zone template
					if (r.m_idZoneTemplate == VICUS::INVALID_ID)
						continue; // they keep the default gray
					if (id == VICUS::INVALID_ID || r.m_idZoneTemplate == id) {
						// lookup zone template
						const VICUS::ZoneTemplate * zt = db.m_zoneTemplates[r.m_idZoneTemplate];
						if (zt == nullptr)
							continue; // no definition - keep default (gray) color
						// color all surfaces of room based on zone template color
						for (const VICUS::Surface & s : r.m_surfaces)
							s.m_color = zt->m_color;
						// TODO : subsurfaces

					}
				}
			}
		}
	} break;


		// *** OCM_AcousticRoomType
	case SVViewState::OCM_AcousticRoomType: {
		for (const VICUS::Building & b : p.m_buildings) {
			for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
				for (const VICUS::Room & r : bl.m_rooms) {
					// skip all without zone template
					if (r.m_idAcousticTemplate == VICUS::INVALID_ID)
						continue; // they keep the default gray
					if (id == VICUS::INVALID_ID || r.m_idAcousticTemplate == id) {
						// lookup zone template
						const VICUS::AcousticTemplate * at = db.m_acousticTemplates[r.m_idAcousticTemplate];
						if (at == nullptr)
							continue; // no definition - keep default (gray) color
						// color all surfaces of room based on zone template color
						for (const VICUS::Surface & s : r.m_surfaces)
							s.m_color = at->m_color;
						// TODO : subsurfaces

					}
				}
			}
		}
	} break;


		// *** Networks
	case SVViewState::OCM_Network:
	case SVViewState::OCM_NetworkNode:
	case SVViewState::OCM_NetworkEdge:
	case SVViewState::OCM_NetworkHeatExchange:
	case SVViewState::OCM_NetworkSubNetworks: {
		for (const VICUS::Network & net: p.m_geometricNetworks){

			switch (ocm) {
			case SVViewState::OCM_NetworkNode:
				net.setDefaultColors();
				break;
			case SVViewState::OCM_NetworkEdge: {
				for (const VICUS::NetworkNode & node: net.m_nodes)
					node.m_color = Qt::lightGray;
				for (const VICUS::NetworkEdge & edge: net.m_edges){
					edge.m_color = Qt::lightGray;
					unsigned int id = edge.m_idPipe;
					if (db.m_pipes[id] != nullptr)
						edge.m_color = db.m_pipes[id]->m_color;
				}
			} break;
			case SVViewState::OCM_NetworkHeatExchange: {
				for (const VICUS::NetworkEdge & edge: net.m_edges)
					edge.m_color = VICUS::Network::colorHeatExchangeType(edge.m_heatExchange.m_modelType);
				for (const VICUS::NetworkNode & node: net.m_nodes)
					node.m_color = VICUS::Network::colorHeatExchangeType(node.m_heatExchange.m_modelType);
			} break;
			case SVViewState::OCM_NetworkSubNetworks: {
				for (const VICUS::NetworkNode & node: net.m_nodes){
					node.m_color = Qt::lightGray;
					unsigned int id = node.m_idSubNetwork;
					if (db.m_subNetworks[id] != nullptr)
						node.m_color = db.m_subNetworks[id]->m_color;
				}
			} break;

			default:
				break;
			} // switch
		} // for
	} break;

	case SVViewState::OCM_ResultColorView:
		break;


	} // end of endless switch

}


void Scene::selectAll() {
	std::set<const VICUS::Object *> selObjects;
	// select all objects, wether they are selected already or not, and whether they are visible or not
	project().selectObjects(selObjects, VICUS::Project::SG_All, false, false);

	// get a list of IDs of nodes to be selected (only those who are not yet selected)
	std::set<unsigned int> nodeIDs;
	for (const VICUS::Object * o : selObjects) {
		if (!o->m_selected)
			nodeIDs.insert(o->m_id);
	}

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("All objects selected"),
														 SVUndoTreeNodeState::SelectedState, nodeIDs, true);
	// select all
	undo->push();
}

void deselectChildSurfaces(std::set<unsigned int> &objIDs, const VICUS::Surface &s) {
	for(const VICUS::Surface &cs : s.childSurfaces()) {
		if(cs.m_selected)
			objIDs.insert(cs.m_id);
		deselectChildSurfaces(objIDs, cs);
	}
}

void Scene::deselectAll() {
	std::set<unsigned int> objIDs;
	// add all selected objects, whether they are visible or not

	// get VICUS project data
	const VICUS::Project & p = project();
	for (const VICUS::Building & b : p.m_buildings) {
		if (b.m_selected)
			objIDs.insert(b.m_id);
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			if (bl.m_selected)
				objIDs.insert(bl.m_id);
			for (const VICUS::Room & r : bl.m_rooms) {
				if (r.m_selected)
					objIDs.insert(r.m_id);
				for (const VICUS::Surface & s : r.m_surfaces) {
					if (s.m_selected)
						objIDs.insert(s.m_id);
					for (const VICUS::SubSurface & sub : s.subSurfaces()) {
						if (sub.m_selected)
							objIDs.insert(sub.m_id);
					}
					deselectChildSurfaces(objIDs, s);
				}
			}
		}
	}

	// now the plain geometry
	for (const VICUS::Surface & s : p.m_plainGeometry.m_surfaces) {
		if (s.m_selected)
			objIDs.insert(s.m_id);
	}

	// now network stuff

	// add cylinders for all pipes
	for (const VICUS::Network & network : p.m_geometricNetworks) {
		for (const VICUS::NetworkEdge & e : network.m_edges) {
			if (e.m_selected)
				objIDs.insert(e.m_id);
		}

		// add spheres for nodes
		for (const VICUS::NetworkNode & no : network.m_nodes) {
			if (no.m_selected)
				objIDs.insert(no.m_id);
		}
		if (network.m_selected)
			objIDs.insert(network.m_id);
	}

	// if nothing is selected, do nothing
	if (objIDs.empty())
		return;
	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selected cleared"),
														 SVUndoTreeNodeState::SelectedState, objIDs, false);
	undo->push();
}


void Scene::deleteSelected() {
	// generate vector of surfaces
	std::set<const VICUS::Object *> selectedObjects;
	project().selectObjects(selectedObjects, VICUS::Project::SG_All, true, true);

	// nothing selected - nothing to do
	if (selectedObjects.empty())
		return;

	// clear selected objects (since they are now removed)
	SVUndoDeleteSelected * undo = new SVUndoDeleteSelected(tr("Removing selected geometry"), selectedObjects);
	// clear selection
	undo->push();
}


void Scene::showSelected() {
	std::set<const VICUS::Object*> selectedObjects;
	// process all objects in project, take all objects that are selected, yet invisible
	project().selectObjects(selectedObjects, VICUS::Project::SG_All, true, false);
	std::set<unsigned int> selectedObjectIDs;
	// Note: all objects in m_selectedGeometryObject.m_selectedObjects are required to be visible!
	for (const VICUS::Object * obj : selectedObjects)
		selectedObjectIDs.insert(obj->m_id);

	if (selectedObjectIDs.empty())
		return;

	SVUndoTreeNodeState * action = new SVUndoTreeNodeState(tr("Visibility changed"),
														   SVUndoTreeNodeState::VisibilityState,
														   selectedObjectIDs,
														   true);
	action->push();
}


void Scene::hideSelected() {
	// process all project geometry and create data for a state modification undo action
	std::set<unsigned int> selectedObjectIDs;
	// Note: all objects in m_selectedGeometryObject.m_selectedObjects are required to be visible!
	for (const VICUS::Object * obj : m_selectedGeometryObject.m_selectedObjects)
		selectedObjectIDs.insert(obj->m_id);

	if (selectedObjectIDs.empty())
		return;

	SVUndoTreeNodeState * action = new SVUndoTreeNodeState(tr("Visibility changed"),
														   SVUndoTreeNodeState::VisibilityState,
														   selectedObjectIDs,
														   false);
	action->push();
}


void Scene::leaveAnySpecialMode() {
	// turn on AlignLocalCoordinateSystem mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	// leave any mode we are currently in and restore previous state
	switch (vs.m_sceneOperationMode) {
	case SVViewState::OM_AlignLocalCoordinateSystem :
		leaveCoordinateSystemAdjustmentMode(true);
		break;
	case SVViewState::OM_MoveLocalCoordinateSystem :
		leaveCoordinateSystemTranslationMode(true);
		break;
	case SVViewState::OM_MeasureDistance:
		leaveMeasurementMode();
		break;

	case SVViewState::OM_SelectedGeometry:
	case SVViewState::OM_PlaceVertex:
	case SVViewState::OM_ThreePointRotation:
	case SVViewState::NUM_OM:
		break; // nothing to do when we leave these modes
	}
}


void Scene::enterCoordinateSystemAdjustmentMode() {
	// store current transformation of local coordinate system object
	m_oldCoordinateSystemTransform = m_coordinateSystemObject.transform();
	// turn on AlignLocalCoordinateSystem mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_AlignLocalCoordinateSystem;
	SVViewStateHandler::instance().setViewState(vs);
	qDebug() << "Entering 'Align coordinate system' mode";
}


void Scene::leaveCoordinateSystemAdjustmentMode(bool abort) {
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
	// switch back to defined view state
	setDefaultViewState();
}


void Scene::enterCoordinateSystemTranslationMode() {
	// store current transformation of local coordinate system object
	m_oldCoordinateSystemTransform = m_coordinateSystemObject.transform();
	// turn on AlignLocalCoordinateSystem mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_MoveLocalCoordinateSystem;
	SVViewStateHandler::instance().setViewState(vs);
	qDebug() << "Entering 'Translate coordinate system' mode";
}


void Scene::leaveCoordinateSystemTranslationMode(bool abort) {
	// restore original local coordinate system
	if (abort) {
		m_coordinateSystemObject.setTransform(m_oldCoordinateSystemTransform);
		qDebug() << "Aborting 'Translate coordinate system' mode (no change)";
	}
	else {
		// finish aligning coordinate system
		qDebug() << "Leaving 'Translate coordinate system' mode";
	}
	// switch back to defined view state
	// do we have any selected geometries
	setDefaultViewState();
}


void Scene::enterMeasurementMode() {
	// store current transformation of local coordinate system object
	m_oldCoordinateSystemTransform = m_coordinateSystemObject.transform();
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_MeasureDistance;
	SVViewStateHandler::instance().setViewState(vs);
	qDebug() << "Entering 'Measurement' mode";

	m_measurementWidget->show();
	// move measurement widget to correct position
	SVViewStateHandler::instance().m_geometryView->moveMeasurementWidget();

	SVViewStateHandler::instance().m_coordinateSystemObject->setOrbColor(SVViewStateHandler::instance().m_measurementWidget->m_color);
}


void Scene::leaveMeasurementMode() {
	// restore original local coordinate system
	m_coordinateSystemObject.setTransform(m_oldCoordinateSystemTransform);
	m_measurementObject.reset();
	qDebug() << "Leaving 'Measurement' mode";

	SVViewStateHandler::instance().m_geometryView->hideMeasurementWidget();

	SVViewStateHandler::instance().m_coordinateSystemObject->setOrbColor(QColor("burlywood"));
	SVViewStateHandler::instance().m_coordinateSystemObject->m_originalTranslation = QVector3D(0,0,0);

	// switch back to defined view state
	// do we have any selected geometries
	setDefaultViewState();
}


void Scene::pick(PickObject & pickObject) {
	// clear pick candidates
	pickObject.m_candidates.clear();

	// local mouse coordinates
	int my = pickObject.m_localMousePos.y();
	int mx = pickObject.m_localMousePos.x();

	// viewport dimensions
	double Dx = m_viewPort.width();
	double Dy = m_viewPort.height();

	double nx = (2*mx-Dx)/Dx;
	double ny = -(2*my-Dy)/Dy;

	// invert world2view matrix, with m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
	bool invertible;
	QMatrix4x4 projectionMatrixInverted = m_worldToView.inverted(&invertible);
	if (!invertible) {
		qWarning()<< "Cannot invert projection matrix.";

		// Add a dummy point to ensure that we always have a least one (though meaningless) pick candidate.
		PickObject::PickResult r;
		r.m_resultType = PickObject::RT_GridPlane;
		r.m_objectID = 0;
		r.m_depth = 1;
		r.m_pickPoint.set(0,0,0);
		pickObject.m_candidates.push_back(r);
		return;
	}

	// mouse position in NDC space, one point on near plane and one point on far plane
	QVector4D nearPos(float(nx), float(ny), -1, 1.0);
	QVector4D farPos (float(nx), float(ny),  1, 1.0);

	// transform from NDC to model coordinates
	QVector4D nearResult = projectionMatrixInverted*nearPos;
	QVector4D farResult = projectionMatrixInverted*farPos;
	// don't forget normalization!
	nearResult /= nearResult.w();
	farResult /= farResult.w();

	// compute line-of-sight equation
	IBKMK::Vector3D nearPoint = QVector2IBKVector(nearResult.toVector3D()); // line offset = nearPoint
	// Note: Since the NearPlane-distance is 1 and thus nearly attached to the camera (distance 0 is not permitted!),
	//       the calculated nearPoint is actually (almost) the camera position, regardless of where one has clicked
	//       on the screen!
	//       So we might actually just take the camera position here and avoid the extra work, but since there may
	//       be later the demand for "zoom" and perspective adjustment, we leave it this way.
	IBKMK::Vector3D farPoint = QVector2IBKVector(farResult.toVector3D());
	IBKMK::Vector3D direction = farPoint - nearPoint;	// direction vector of line-of-sight
	pickObject.m_lineOfSightOffset = nearPoint;
	pickObject.m_lineOfSightDirection = direction;

	// *** now do the actual picking ***
	//
	// We collect a list of pick candidates, all with pick points along the line-of-sight.
	// In this function we only collect the pick candidates. The calling function has to
	// pick the right one and filter out unwanted candidates.

	//#define SHOW_PICK_TIME
#ifdef SHOW_PICK_TIME
	QElapsedTimer pickTimer;
	pickTimer.start();
#endif


	// *** intersection with grid plane ***

	IBKMK::Vector3D intersectionPoint;
	double t;
	// process all grid planes - being transparent, these are picked from both sides
	for (unsigned int i=0; i< project().m_viewSettings.m_gridPlanes.size(); ++i) {
		if (project().m_viewSettings.m_gridPlanes[i].intersectsLine(nearPoint, direction, t, intersectionPoint)) {
			// got an intersection point, store it
			PickObject::PickResult r;
			r.m_resultType = PickObject::RT_GridPlane;
			r.m_objectID = i;
			r.m_depth = t;
			r.m_pickPoint = intersectionPoint;
			pickObject.m_candidates.push_back(r);
		}
	}

	// now process all surfaces and update p to hold the closest hit
	const VICUS::Project & prj = project();


	// *** surfaces of buildings ***

	for (const VICUS::Building & b : prj.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				for (const VICUS::Surface & s : r.m_surfaces) {
					// skip invisible or inactive surfaces
					//					if (!s.m_visible)
					//						continue;
					IBKMK::Vector3D intersectionPoint;
					double dist;
					// check if we hit the surface - since we show the surface from both sides, we
					// can also pick both sides
					int holeIndex;
					if (s.geometry().intersectsLine(nearPoint, direction, intersectionPoint, dist, holeIndex, true)) {
						// if there is no whole in the surface, we only require that the surface is visible - otherwise
						// ignore the click
						if (holeIndex == -1 && !s.m_visible)
							continue; // skip

						// we may have a click...
						PickObject::PickResult r;
						r.m_resultType = PickObject::RT_Object;
						r.m_depth = dist;
						r.m_pickPoint = intersectionPoint;
						r.m_holeIdx = holeIndex;

						// if we have hole, check if we did click on it
						if (holeIndex != -1) {
							const VICUS::Object *obj = nullptr;
							obj = SVProjectHandler::instance().project().objectById(s.geometry().holes()[(unsigned int)holeIndex].m_idObject);
							if (obj == nullptr) {// guard against dangling IDs
								// Note: cannot use an assert/exception here, as hole ID is user-data from potentially corrupt data file
//								(IBK::FormatString("Invalid hole ID #%1 in surface #%2")
								continue;
							}

							// hole must be visible to be checked
							if (!obj->m_visible)
								continue;
							r.m_objectID = obj->m_id;
						}
						else {
							r.m_objectID = s.m_id;
						}
						// register click candidate
						pickObject.m_candidates.push_back(r);
					}
				}
			}
		}
	}


	// *** now try plain geometry ***

	for (const VICUS::Surface & s : prj.m_plainGeometry.m_surfaces) {
		// skip invisible or inactive surfaces
		if (!s.m_visible)
			continue;
		IBKMK::Vector3D intersectionPoint;
		double dist;
		// dump geometry is rendered front/back facing and also picked from both sides
		int holeIndex;
		if (s.geometry().intersectsLine(nearPoint, direction, intersectionPoint, dist, holeIndex, true)) {
			PickObject::PickResult r;
			r.m_resultType = PickObject::RT_Object;
			r.m_depth = dist;
			r.m_pickPoint = intersectionPoint;
			// TODO : Dirk, can "dump geometry" contain sub-surfaces?
			r.m_objectID = s.m_id;
			pickObject.m_candidates.push_back(r);
		}
	}


	// *** process all networks ***

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
				r.m_resultType = PickObject::RT_Object;
				r.m_depth = dist; // the depth to the point on the line-of-sight that is closest to the sphere's center point
				r.m_pickPoint = closestPoint; // this
				r.m_objectID = no.m_id;
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
				r.m_resultType = PickObject::RT_Object;
				r.m_depth = dist;
				r.m_pickPoint = closestPoint;
				r.m_objectID = e.m_id;
				pickObject.m_candidates.push_back(r);
			}
		}
	}

	// *** local coordinate system pick points ***

	// only do this when not panning/orbiting/first-person-viewing etc.
	if (m_navigationMode == NUM_NM) {

		// also, we do not snap to the local coordinate system spheres, when we are in align coordinate system
		// mode. Basically, we only snap to the local coordinate system, when in "selected geometry" mode
		if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_SelectedGeometry) {

			PickObject::PickResult r;
			if (m_coordinateSystemObject.pick(nearPoint, direction, r)) {

				// If local coordinate system is in "transformation" mode,
				// ignore the other pick points and just use the local coordinate system.
				// But only do this, if we are not in "align coordinate system" mode.
				if (m_coordinateSystemObject.m_geometryTransformMode != CoordinateSystemObject::TM_None)
					pickObject.m_candidates.clear();
				pickObject.m_candidates.push_back(r);
			}
		}
	}

	// also add the artifical intersection point with the far plane as "worst-of-all" candidate - this might
	// be needed for orbit controller operation

	PickObject::PickResult r;
	r.m_resultType = PickObject::RT_GridPlane;
	r.m_objectID = 0;
	r.m_depth = direction.magnitude();
	r.m_pickPoint = farPoint;
	pickObject.m_candidates.push_back(r);

	// finally sort the pick candidates based on depth value
	std::sort(pickObject.m_candidates.begin(), pickObject.m_candidates.end());

#ifdef SHOW_PICK_TIME
	qDebug() << "Pick duration = " << pickTimer.elapsed() << "ms";
#endif

	pickObject.m_pickPerformed = true;
}

void Scene::pickChildSurfaces() {

}


struct SnapCandidate {
	bool operator<(const SnapCandidate & other) const {
		return m_distToLineOfSight < other.m_distToLineOfSight;
	}

	/*! The correspondig snap location. */
	IBKMK::Vector3D		m_pickPoint;
	/*! Distance of this candidate to pick point on line-of-sight. */
	double				m_distToLineOfSight;
};


void Scene::snapLocalCoordinateSystem(const PickObject & pickObject) {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();

	const float SNAP_DISTANCES_THRESHHOLD = 2; // in m, should be enough, right?


	SVViewState::Locks actualLockOption = vs.m_locks;
	SVViewState::Locks axisLoc = SVViewState::NUM_L; // initialization just to silence compiler warning

	// if we have axis lock, determine offset and direction
	IBKMK::Vector3D axisLockOffset;
	if (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex) {

		// for now, return the position of the last added vertex, if any
		if (m_newGeometryObject.vertexList().empty())
			axisLockOffset = IBKMK::Vector3D();
		else
			axisLockOffset = m_newGeometryObject.vertexList().back();
	}
	else if (vs.m_sceneOperationMode == SVViewState::OM_SelectedGeometry) {
		if (m_coordinateSystemObject.m_geometryTransformMode == Vic3D::CoordinateSystemObject::TM_Translate ||
				(m_coordinateSystemObject.m_geometryTransformMode & Vic3D::CoordinateSystemObject::TM_ScaleMask) != 0)
		{
			switch (m_coordinateSystemObject.m_geometryTransformMode) {
			case Vic3D::CoordinateSystemObject::TM_ScaleX : axisLoc = SVViewState::L_LocalX; break;
			case Vic3D::CoordinateSystemObject::TM_ScaleY : axisLoc = SVViewState::L_LocalY; break;
			case Vic3D::CoordinateSystemObject::TM_ScaleZ : axisLoc = SVViewState::L_LocalZ; break;
			}
			axisLockOffset = QVector2IBKVector(m_coordinateSystemObject.m_originalTranslation);
		}
	}
	else if (vs.m_sceneOperationMode == SVViewState::OM_MeasureDistance) {
		axisLockOffset = QVector2IBKVector(m_coordinateSystemObject.m_originalTranslation);
	}
	// override user-selected axis lock for scaling operation
	if (axisLoc != SVViewState::NUM_L)
		actualLockOption = axisLoc;

	// get direction in case of axis lock
	IBKMK::Vector3D direction;
	switch (actualLockOption) {
	case SVViewState::L_LocalX : direction = QVector2IBKVector(m_coordinateSystemObject.localXAxis()); break;
	case SVViewState::L_LocalY : direction = QVector2IBKVector(m_coordinateSystemObject.localYAxis()); break;
	case SVViewState::L_LocalZ : direction = QVector2IBKVector(m_coordinateSystemObject.localZAxis()); break;
	case SVViewState::NUM_L: ; // no lock
	}


	// compute closest point between line-of-sight and locked axis
	double lineOfSightDist, lockedAxisDist;
	IBKMK::Vector3D closestPointOnAxis;
	IBKMK::lineToLineDistance(axisLockOffset, direction,
							  pickObject.m_lineOfSightOffset, pickObject.m_lineOfSightDirection,
							  lockedAxisDist, closestPointOnAxis, lineOfSightDist);


	IBKMK::Vector3D		snapPoint; // default snap point = origin
	std::string snapInfo = "no hit (fall-back to coordinate origin)";

	// *** no snap ***
	//
	if (!vs.m_snapEnabled) {

		// take closest hit and be done
		snapPoint = pickObject.m_candidates.front().m_pickPoint;
		// if we have a locked axis, and the "closest point to axis" is actually closer than the pick point candidate,
		// we take the closest point instead
		if (actualLockOption != SVViewState::NUM_L && lineOfSightDist < pickObject.m_candidates.front().m_depth) {
			snapPoint = closestPointOnAxis;
			snapInfo = "closest point on locked axis";
		}
		else
			snapInfo = "nearest object/plane hit";

		// now our candidate is stored as snapPoint - if axis lock is enabled, we still project the point
		// towards the locked axis (line) defined via last coordinate's position and locked local axis vector
	}


	// *** with snap ***
	//
	else {

		// by default, we snap to the closest point on a locked axis, if one is locked
		if (actualLockOption != SVViewState::NUM_L) {
			snapPoint = closestPointOnAxis;
			snapInfo = "closest point on locked axis";
		}

		// we have now several surfaces/objects stored in the pickObject as candidates
		// we take the first hit and process the snap options

		int snapOptions = vs.m_snapOptionMask;
		const PickObject::PickResult & r = pickObject.m_candidates.front();

		QVector3D pickPoint = IBKVector2QVector(r.m_pickPoint);

		// depending on type of object, process the different snap options
		if (r.m_resultType == PickObject::RT_GridPlane) {
			// use the intersection point with the grid as default snap point (in case no grid point is close enough)
			// but only, if there is no axis lock enabled
			if (actualLockOption == SVViewState::NUM_L) {
				snapPoint = r.m_pickPoint;
				snapInfo = "intersection point with plane (fall-back if no grid-point is in snap distance)";
			}
			// do we snap to grid?
			if (snapOptions & SVViewState::Snap_GridPlane) {
				// now determine which grid line is closest
				IBKMK::Vector3D closestPoint;
				Q_ASSERT(r.m_objectID < project().m_viewSettings.m_gridPlanes.size());
				project().m_viewSettings.m_gridPlanes[r.m_objectID].closestSnapPoint(r.m_pickPoint, closestPoint);
				// this is in world coordinates, use this as transformation vector for the
				// coordinate system
				float dist = (IBKVector2QVector(closestPoint) - pickPoint).lengthSquared();
				// close enough?
				if (dist < SNAP_DISTANCES_THRESHHOLD) {
					// got a snap point, store it
					snapPoint = closestPoint;
					snapInfo = "snap to grid point";
				}
			}
		}

		if (r.m_resultType == PickObject::RT_Object) {
			// We hit an object!

			// find out if this is a surface, a network node or an edge
			const VICUS::Object * obj = project().objectById(r.m_objectID);
			Q_ASSERT(obj != nullptr);

			// *** surfaces ***

			const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(obj);
			if (s != nullptr) {
				// a surface object might have several snap points which we collect first in a vector
				std::vector<SnapCandidate> snapCandidates;
				float closestDepthSoFar = SNAP_DISTANCES_THRESHHOLD;

				// for now we snap to the vertexes of the outer polygon and all holes
				const std::vector<IBKMK::Vector3D> & sVertexes = s->geometry().triangulationData().m_vertexes;

				// we always add the intersection point with the surface as fall-back snappoint,
				// but with a large distance so that it is only used as last resort
				SnapCandidate sc;
				sc.m_distToLineOfSight = (double)SNAP_DISTANCES_THRESHHOLD*2;
				sc.m_pickPoint = r.m_pickPoint;
				snapCandidates.push_back(sc);

				if (snapOptions & SVViewState::Snap_ObjectVertex) {
					// insert distances to all vertexes of selected object
					for (const IBKMK::Vector3D & v : sVertexes) {
						QVector3D p = IBKVector2QVector(v);
						float dist = (p - pickPoint).lengthSquared();
						// Only add if close enough (< SNAP_DISTANCES_THRESHHOLD) and if there isn't yet
						// another snap point that's closer.
						if (dist < closestDepthSoFar && dist < SNAP_DISTANCES_THRESHHOLD) {
							sc.m_distToLineOfSight = (double)dist;
							sc.m_pickPoint = v;
							snapCandidates.push_back(sc);
							closestDepthSoFar = dist;
						}
					}
				}
				if (snapOptions & SVViewState::Snap_ObjectCenter) {
					// insert center point
					IBKMK::Vector3D center(0,0,0);
					for (const IBKMK::Vector3D & v : sVertexes)
						center += v;
					center /= sVertexes.size();
					QVector3D p = IBKVector2QVector(center);
					float dist = (p - pickPoint).lengthSquared();
					if (dist < closestDepthSoFar && dist < SNAP_DISTANCES_THRESHHOLD) {
						sc.m_distToLineOfSight = (double)dist;
						sc.m_pickPoint = center;
						snapCandidates.push_back(sc);
						closestDepthSoFar = dist;
					}
				}
				if (snapOptions & SVViewState::Snap_ObjectEdgeCenter) {
					// process all edges
					IBKMK::Vector3D lastNode = sVertexes.front();
					for (unsigned int i=0; i<sVertexes.size()+1; ++i) {
						IBKMK::Vector3D center = lastNode;
						lastNode = sVertexes[i % sVertexes.size()];
						center += lastNode;
						center /= 2;
						QVector3D p = IBKVector2QVector(center);
						float dist = (p - pickPoint).lengthSquared();
						if (dist < closestDepthSoFar && dist < SNAP_DISTANCES_THRESHHOLD) {
							sc.m_distToLineOfSight = (double)dist;
							sc.m_pickPoint = center;
							snapCandidates.push_back(sc);
							closestDepthSoFar = dist;
						}
					}
				}

				// now we take the snap point that's closest - even if all the snap options of an object are
				// turned off, we still get the intersection point as last straw to pick.
				std::sort(snapCandidates.begin(), snapCandidates.end());
				snapPoint = snapCandidates.front().m_pickPoint;
				snapInfo = "snap to object";
			} // if (s != nullptr)

			// *** sub-surfaces ***

			const VICUS::SubSurface * sub = dynamic_cast<const VICUS::SubSurface *>(obj);
			if (sub != nullptr) {
				s = dynamic_cast<const VICUS::Surface *>(sub->m_parent);
				IBK_ASSERT(r.m_holeIdx != -1);
				// a sub-surface object might have several snap points which we collect first in a vector
				std::vector<SnapCandidate> snapCandidates;
				float closestDepthSoFar = SNAP_DISTANCES_THRESHHOLD;

				// for now we snap to the vertexes of the outer polygon and all holes
				const std::vector<IBKMK::Vector3D> & sVertexes = s->geometry().holeTriangulationData()[(unsigned int)r.m_holeIdx].m_vertexes;

				// we always add the intersection point with the surface as fall-back snappoint,
				// but with a large distance so that it is only used as last resort
				SnapCandidate sc;
				sc.m_distToLineOfSight = (double)SNAP_DISTANCES_THRESHHOLD*2;
				sc.m_pickPoint = r.m_pickPoint;
				snapCandidates.push_back(sc);

				if (snapOptions & SVViewState::Snap_ObjectVertex) {
					// insert distances to all vertexes of selected object
					for (const IBKMK::Vector3D & v : sVertexes) {
						QVector3D p = IBKVector2QVector(v);
						float dist = (p - pickPoint).lengthSquared();
						// Only add if close enough (< SNAP_DISTANCES_THRESHHOLD) and if there isn't yet
						// another snap point that's closer.
						if (dist < closestDepthSoFar && dist < SNAP_DISTANCES_THRESHHOLD) {
							sc.m_distToLineOfSight = (double)dist;
							sc.m_pickPoint = v;
							snapCandidates.push_back(sc);
							closestDepthSoFar = dist;
						}
					}
				}
				if (snapOptions & SVViewState::Snap_ObjectCenter) {
					// insert center point
					IBKMK::Vector3D center(0,0,0);
					for (const IBKMK::Vector3D & v : sVertexes)
						center += v;
					center /= sVertexes.size();
					QVector3D p = IBKVector2QVector(center);
					float dist = (p - pickPoint).lengthSquared();
					if (dist < closestDepthSoFar && dist < SNAP_DISTANCES_THRESHHOLD) {
						sc.m_distToLineOfSight = (double)dist;
						sc.m_pickPoint = center;
						snapCandidates.push_back(sc);
						closestDepthSoFar = dist;
					}
				}
				if (snapOptions & SVViewState::Snap_ObjectEdgeCenter) {
					// process all edges
					IBKMK::Vector3D lastNode = sVertexes.front();
					for (unsigned int i=0; i<sVertexes.size()+1; ++i) {
						IBKMK::Vector3D center = lastNode;
						lastNode = sVertexes[i % sVertexes.size()];
						center += lastNode;
						center /= 2;
						QVector3D p = IBKVector2QVector(center);
						float dist = (p - pickPoint).lengthSquared();
						if (dist < closestDepthSoFar && dist < SNAP_DISTANCES_THRESHHOLD) {
							sc.m_distToLineOfSight = (double)dist;
							sc.m_pickPoint = center;
							snapCandidates.push_back(sc);
							closestDepthSoFar = dist;
						}
					}
				}

				// now we take the snap point that's closest - even if all the snap options of an object are
				// turned off, we still get the intersection point as last straw to pick.
				std::sort(snapCandidates.begin(), snapCandidates.end());
				snapPoint = snapCandidates.front().m_pickPoint;
				snapInfo = "snap to object";
			} // if (s != nullptr)


			// currently there is such snapping to nodes, yet

			/// \todo Add snapping to nodes (i.e. when drawing edges)

		} // object snap

	} // with snapping

	//	qDebug() << "Snap to: " << snapInfo.c_str();

	// we now have a snap point
	// if we also have line snap on, calculate the projection of this intersection point with the line

	// compute projection of current snapPoint onto line defined by refPoint and direction
	if (actualLockOption != SVViewState::NUM_L) {
		double lineFactor;
		IBKMK::Vector3D projectedPoint;
		IBKMK::lineToPointDistance(axisLockOffset, direction, snapPoint, lineFactor, projectedPoint);
		snapPoint = projectedPoint; // update snap point with projected point
	}

	// take closest snap point and snap to it
	QVector3D newCoordinatePoint = IBKVector2QVector(snapPoint);
	m_coordinateSystemObject.setTranslation(newCoordinatePoint);
}


void Scene::adjustCursorDuringMouseDrag(const QPoint & mouseDelta, const QPoint & localMousePos,
										QPoint & newLocalMousePos, PickObject & pickObject)
{
	QPoint localMousePosScaled = localMousePos * SVSettings::instance().m_ratio;
	// cursor position moves out of window?
	const int WINDOW_MOVE_MARGIN = 50;
	if (localMousePosScaled.x() < WINDOW_MOVE_MARGIN && mouseDelta.x() < 0) {
//		qDebug() << "Resetting mousepos to right side of window.";
		newLocalMousePos.setX(m_viewPort.width()/SVSettings::instance().m_ratio-WINDOW_MOVE_MARGIN);
	}
	else if (localMousePosScaled.x() > (m_viewPort.width()-WINDOW_MOVE_MARGIN) && mouseDelta.x() > 0) {
//		qDebug() << "Resetting mousepos to left side of window.";
		newLocalMousePos.setX(WINDOW_MOVE_MARGIN);
	}

	if (localMousePosScaled.y() < WINDOW_MOVE_MARGIN && mouseDelta.y() < 0) {
//		qDebug() << "Resetting mousepos to bottom side of window.";
		newLocalMousePos.setY(m_viewPort.height()/SVSettings::instance().m_ratio-WINDOW_MOVE_MARGIN);
	}
	else if (localMousePosScaled.y() > (m_viewPort.height()-WINDOW_MOVE_MARGIN) && mouseDelta.y() > 0) {
//		qDebug() << "Resetting mousepos to top side of window.";
		newLocalMousePos.setY(WINDOW_MOVE_MARGIN);
	}

	// if panning is enabled, reset the pan start positions/variables
	if (m_navigationMode == NM_Panning && newLocalMousePos != localMousePos) {
		bool useMouseWarping = false;
#ifdef Q_OS_LINUX
#if HAVE_X11
		if (QX11Info::isPlatformX11()) {
			useMouseWarping = true;
		}
#endif
#else
		useMouseWarping = true;
#endif
		if (useMouseWarping) {
			pickObject.m_localMousePos = newLocalMousePos * SVSettings::instance().m_ratio;
			pick(pickObject);
			panStart(newLocalMousePos, pickObject, true);
		}
	}
}


void Scene::handleLeftMouseClick(const KeyboardMouseHandler & keyboardHandler, PickObject & o) {
	// do different things depending on current scene operation mode

	switch (SVViewStateHandler::instance().viewState().m_sceneOperationMode) {

	// *** place a vertex ***
	case SVViewState::NUM_OM :
	case SVViewState::OM_SelectedGeometry : {
		// selection handling
		handleSelection(keyboardHandler, o);
		return;
	}

	case SVViewState::OM_MeasureDistance : {
		// we start a new measurement when:
		// a) no start and no end point have been set yet (i.e. mode has just started)
		// b) last measurement is complete, i.e. both start and end point are set

		// new starting point for measurement selected
		if (m_measurementObject.m_startPoint == INVALID_POINT)
		{
			// if we start from "initial mode", show the distance widget
			// store new start point
			m_measurementObject.m_startPoint = m_coordinateSystemObject.translation();
			m_measurementWidget->showStartPoint(m_measurementObject.m_startPoint);
			m_coordinateSystemObject.m_originalTranslation = m_coordinateSystemObject.translation();
		}
		else if (m_measurementObject.m_startPoint != INVALID_POINT && m_measurementObject.m_endPoint != INVALID_POINT) {
			// first reset object and widget
			m_measurementObject.reset();
			m_measurementWidget->reset();
			// then start next measurement from current LCS position
			m_measurementObject.m_startPoint = m_coordinateSystemObject.translation();
			m_measurementWidget->showStartPoint(m_measurementObject.m_startPoint);
			m_coordinateSystemObject.m_originalTranslation = m_coordinateSystemObject.translation();
		}
		else {
			// finish measurement mode by fixing the end point
			m_measurementObject.m_endPoint = m_coordinateSystemObject.translation();
		}
		return;
	}

		// *** place a vertex ***
	case SVViewState::OM_PlaceVertex : {
		// get current coordinate system's position
		IBKMK::Vector3D p = QVector2IBKVector(m_coordinateSystemObject.translation());
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
		leaveCoordinateSystemAdjustmentMode(false);
		return;
	}

		// *** move coordinate system ***
	case SVViewState::OM_MoveLocalCoordinateSystem : {
		// finish moving coordinate system and current local coordinate system location
		leaveCoordinateSystemTranslationMode(false);
		return;
	}

		// *** three point rotation
	case SVViewState::OM_ThreePointRotation:
		// TODO
		break;

	} // switch

}


void Scene::handleSelection(const KeyboardMouseHandler & keyboardHandler, PickObject & o) {
	// this will be a selection click - execute pick() operation
	pick(o);

	// check if any of the pick candidates is of type object
	unsigned int uniqueID = 0;
	for (const PickObject::PickResult & r : o.m_candidates) {
		if (r.m_resultType == PickObject::RT_Object)
		{
			uniqueID = r.m_objectID;
			break;
		}
	}

	if (uniqueID != 0) {
		// find the selected object
		const VICUS::Object * obj = project().objectById(uniqueID);

		// if using shift-click, we go up one level, select the parent and all its children
		bool selectChildren = true;
		if (keyboardHandler.keyDown(Qt::Key_Shift)) {
			// check for valid parent - anonymous geometry does not have parents!
			if (obj->m_parent != nullptr) {
				obj = obj->m_parent;
				uniqueID = obj->m_id;
			}
		}
		else if (keyboardHandler.keyDown(Qt::Key_Alt)) {
			selectChildren = false;
		}

		// create undo-action that toggles the selection
		SVUndoTreeNodeState * action = SVUndoTreeNodeState::createUndoAction(tr("Selection changed"),
																			 SVUndoTreeNodeState::SelectedState,
																			 uniqueID,
																			 selectChildren, // if true, select all children, otherwise only the object itself
																			 !obj->m_selected);
		action->push();

		// now the data model and the views have been updated, also now signal the navigation tree view to scroll
		// to the first node
		SVViewStateHandler::instance().m_navigationTreeWidget->scrollToObject(uniqueID);

		return;
	}
}


IBKMK::Vector3D Scene::calculateFarPoint(const QPoint & mousPos, const QMatrix4x4 & projectionMatrixInverted) {
	// local mouse coordinates
	int my = mousPos.y();
	int mx = mousPos.x();

	// viewport dimensions
	double Dx = m_viewPort.width();
	double Dy = m_viewPort.height();

	double nx = (2*mx-Dx)/Dx;
	double ny = -(2*my-Dy)/Dy;

	// mouse position in NDC space, one point on near plane and one point on far plane
	QVector4D nearPos(float(nx), float(ny), -1, 1.0);
	QVector4D farPos (float(nx), float(ny),  1, 1.0);

	// transform from NDC to model coordinates
	QVector4D farResult = projectionMatrixInverted*farPos;
	// don't forget normalization!
	farResult /= farResult.w();

	return QVector2IBKVector(farResult.toVector3D());
}


void Scene::panStart(const QPoint & localMousePos, PickObject & pickObject, bool reuseDepth) {
	qDebug() << "Entering panning mode";
	m_navigationMode = NM_Panning;

	// configure the pick object and pick a point on the XY plane/or any visible surface
	if (!pickObject.m_pickPerformed)
		pick(pickObject);

	std::vector<PickObject::PickResult> &candidates = pickObject.m_candidates;
	// if we had reset the mouse when panning over the sides of the screen, we want to keep the same panning
	// depth - hence we recalculate the pick candidate to be along the line of sight, yet at same depth as
	// stored in m_panObjectDepth
	if (reuseDepth) {
		candidates.front().m_depth = m_panObjectDepth;
		candidates.front().m_pickPoint =
				pickObject.m_lineOfSightOffset + pickObject.m_lineOfSightDirection*candidates.front().m_depth;
	}

	// if pick point is very far away, we limit the depth and adjust the pick point

	// we need to store initial camera pos, selected object pos, far point
	m_panCameraStart = QVector2IBKVector(m_camera.translation());	// Point A
	m_panObjectStart = candidates.front().m_pickPoint;			// Point C
	m_panFarPointStart = pickObject.m_lineOfSightOffset + pickObject.m_lineOfSightDirection;	// Point B
	m_panObjectDepth = candidates.front().m_depth;
	double BADistance = (m_panFarPointStart - m_panCameraStart).magnitude(); // Same as far distance?
	double CADistance = (m_panObjectStart - m_panCameraStart).magnitude();
	m_panCABARatio = CADistance/BADistance;
	m_panMousePos = localMousePos;

	// invert world2view matrix, with m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
	bool invertible;
	m_panOriginalTransformMatrix = m_worldToView.inverted(&invertible);
	if (!invertible) {
		qWarning()<< "Cannot invert projection matrix.";
		m_panOriginalTransformMatrix = QMatrix4x4();
	}
}

void Scene::setDefaultViewState() {
	// this function is called when we return from any intermediate mode like "align
	// coordinate system" mode and get back to what we where doing before

	// if we are in geometry mode, we need to distinguish between PlaceVertex mode or SelectedGeometry mode
	// or NUM mode (nothing)

	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_locks = SVViewState::NUM_L; // no axis is locked

	switch (vs.m_propertyWidgetMode) {
		case SVViewState::PM_AddGeometry:
		case SVViewState::PM_EditGeometry:
		case SVViewState::PM_SiteProperties:
		case SVViewState::PM_BuildingProperties:
		case SVViewState::PM_ResultsProperties:
		case SVViewState::PM_NetworkProperties: {
			// do we have any selected geometries
			std::set<const VICUS::Object *> sel;
			project().selectObjects(sel, VICUS::Project::SG_All, true, true);
			if (sel.empty()) {
				vs.m_sceneOperationMode = SVViewState::NUM_OM;
				if (vs.m_propertyWidgetMode == SVViewState::PM_EditGeometry)
					vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
			}
			else
				vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
			SVViewStateHandler::instance().setViewState(vs);
			return;
		}

		case SVViewState::PM_VertexList:
			vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
			SVViewStateHandler::instance().setViewState(vs);
			return;

		case SVViewState::PM_AddSubSurfaceGeometry:
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
			SVViewStateHandler::instance().setViewState(vs);
			return;
	} // switch
}

const QMatrix4x4 & Scene::worldToView() const {
	return m_worldToView;
}

void Scene::updatedHighlightingMode(HighlightingMode mode) {
	m_surfaceColor.clear();
	generateTransparentBuildingGeometry(mode);
	SVUndoTreeNodeState::ModifiedNodes modInfo;
	SVProjectHandler::instance().setModified( SVProjectHandler::NodeStateModified, &modInfo);
}

const Camera & Scene::camera() const{
	return m_camera;
}

} // namespace Vic3D
