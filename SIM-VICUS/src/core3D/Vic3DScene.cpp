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

#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
#include "Vic3DPickObject.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DConstants.h"
#include "Vic3DSceneView.h"

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVViewState.h"
#include "SVSettings.h"
#include "SVUndoTreeNodeState.h"

const float TRANSLATION_SPEED = 1.2f;
const float MOUSE_ROTATION_SPEED = 0.5f;


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

	// we create the new polygon object here, but data is added once it is used
	m_newPolygonObject.create(m_fixedColorTransformShader, &m_coordinateSystemObject);
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
			m_newPolygonObject.clear();
			break;

		case SVProjectHandler::GeometryChanged :
			updateBuilding = true;
			break;

		case SVProjectHandler::GridModified :
			updateGrid = true;
			break;

		case SVProjectHandler::NetworkModified :
			updateNetwork = true;
			break;

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
				const VICUS::Surface * s = dynamic_cast<const VICUS::Surface*>(obj);
				// skip all node except surfaces, since only surfaces are drawn
				if (s == nullptr)
					continue;

				// get vertex start address of selected node
				Q_ASSERT(m_opaqueGeometryObject.m_vertexStartMap.find(id) != m_opaqueGeometryObject.m_vertexStartMap.end());
				unsigned int vertexStart = m_opaqueGeometryObject.m_vertexStartMap[id];
				smallestVertexIndex = std::min(smallestVertexIndex, vertexStart);
				// now update the color buffer for this surface depending
				updateSurfaceColors(*s, vertexStart, m_opaqueGeometryObject.m_colorBufferData);
				largestVertexIndex = std::min(smallestVertexIndex, vertexStart);

				// update selection set, but only keep visible and selected objects in the set
				if (s->m_selected && s->m_visible) {
					if (m_selectedGeometryObject.m_selectedSurfaces.insert(s).second)
						selectionModified = true;
				}
				else {
					std::set<const VICUS::Surface*>::const_iterator it = m_selectedGeometryObject.m_selectedSurfaces.find(s);
					if (it != m_selectedGeometryObject.m_selectedSurfaces.end()) {
						m_selectedGeometryObject.m_selectedSurfaces.erase(*it);
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
			if (selectionModified /* && info->m_changedStateType == SVUndoTreeNodeState::SelectedState */)
				m_selectedGeometryObject.updateBuffers();

		} break;

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

		m_selectedGeometryObject.create(m_fixedColorTransformShader);
		m_selectedGeometryObject.updateBuffers();
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
	m_opaqueGeometryObject.destroy();
	m_networkGeometryObject.destroy();
	m_selectedGeometryObject.destroy();
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

	// we implement the following controls

	// *** Keyboard ***

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

	// *** Escape ***
	if (keyboardHandler.keyDown(Qt::Key_Escape)) {
		// different operation depending on scene's operation mode
		switch (SVViewStateHandler::instance().viewState().m_sceneOperationMode) {

			// *** place a vertex ***
			case SVViewState::OM_PlaceVertex : {
				// abort "place vertex" operation
				// reset new polygon object, so that it won't be drawn anylonger
				m_newPolygonObject.clear();
				// signal, that we are no longer in "add vertex" mode
				SVViewState vs = SVViewStateHandler::instance().viewState();
				vs.m_sceneOperationMode = SVViewState::NUM_OM;
				vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
				// now tell all UI components to toggle their view state
				SVViewStateHandler::instance().setViewState(vs);
			} break;

			case SVViewState::OM_AlignLocalCoordinateSystem:
			break;

			default:
				// default mode - Escape clears selection
				if (!m_selectedGeometryObject.m_selectedSurfaces.empty()) {
					clearSelectionOfObjects();
				}
		}
	}


	// *** Enter/Return ***
	if (keyboardHandler.keyDown(Qt::Key_Return)) {
		// different operation depending on scene's operation mode
		switch (SVViewStateHandler::instance().viewState().m_sceneOperationMode) {

			// *** place a vertex ***
			case SVViewState::OM_PlaceVertex : {
				// finish "place vertex" operation, this on
				m_newPolygonObject.finish();
			} break;

			default:; // in all other modes, Enter has no effect (for now)

		}
	}


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
			PickObject o(localMousePos, PickObject::P_XY_Plane | PickObject::P_Surface | PickObject::P_BackSide);
			pick(o);
			// and store pick point
			m_orbitControllerOrigin = VICUS::IBKVector2QVector(o.m_pickPoint);
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
		if (m_mouseMoveDistance < 10) {
			// TODO : click code
			qDebug() << "Mouse (selection) click received" << m_mouseMoveDistance;
			handleLeftMouseClick(keyboardHandler, localMousePos);
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

	// store camera position in view settings, but only if we have a project
	// Note: the check is necessary, because the paint event may be called as part of closing the window
	//       and updating the UI to the "no project" state
	if (SVProjectHandler::instance().isValid()) {
		SVProjectHandler::instance().viewSettings().m_cameraTranslation = VICUS::QVector2IBKVector(m_camera.translation());
		SVProjectHandler::instance().viewSettings().m_cameraRotation = m_camera.rotation();
	}
	updateWorld2ViewMatrix();


	// if in "place vertex" mode, perform picking operation and snap coordinate system to grid
	if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_PlaceVertex) {
		/// \todo Stephan: customize picking object rules based on current snap selection
		PickObject o(localMousePos, PickObject::P_XY_Plane);
		pick(o);
		// now determine which grid line is closest
		QVector3D closestPoint;
		if (m_gridObject.closestSnapPoint(VICUS::IBKVector2QVector(o.m_pickPoint), closestPoint)) {
			// this is in world coordinates, use this as transformation vector for the
			// coordinate system
			m_coordinateSystemObject.m_transform.setTranslation(closestPoint);
		}

		// update the movable coordinate system's location in the new polygon object
		m_newPolygonObject.updateLastVertex(m_coordinateSystemObject.m_transform.translation());
	}

}


void Vic3DScene::render() {
	glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

	// set the background color = clear color
	const SVSettings::ThemeSettings & s = SVSettings::instance().m_themeSettings[SVSettings::instance().m_theme];
	QVector3D backgroundColor = VICUS::QVector3DFromQColor(s.m_sceneBackgroundColor);

	glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), 1.0f);

	QVector3D viewPos = m_camera.translation();

	// *** grid ***

	// enable depth testing, important for the grid and for the drawing order of several objects
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	m_gridShader->bind();
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[0], m_worldToView);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[2], backgroundColor);
	m_gridObject.render();
	m_gridShader->release();


	// *** selection object ***

	m_fixedColorTransformShader->bind();
	m_fixedColorTransformShader->shaderProgram()->setUniformValue(m_fixedColorTransformShader->m_uniformIDs[0], m_worldToView);

	m_selectedGeometryObject.render();

	// *** orbit controller indicator ***

	if (m_orbitControllerActive) {
		// Note: uses also m_fixedColorTransformShader, which is already active with up-to-date worldToView matrix
		m_orbitControllerObject.render();
	}

	m_fixedColorTransformShader->release();


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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// tell OpenGL to show all planes
	glDisable(GL_CULL_FACE);
	// disable update of depth test but still use it
	glDepthMask (GL_FALSE);


	// ... windows, ...


	// *** new polygon draw object ***

	if (m_newPolygonObject.hasData() != 0) {
		m_fixedColorTransformShader->bind();
		// Note: worldToView uniform has already been set
		m_newPolygonObject.render();
		m_fixedColorTransformShader->release();
	}

	// re-enable updating of z-buffer
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}


void Vic3DScene::setViewState(const SVViewState & vs) {
	// adjust scene based on view state
	switch (vs.m_sceneOperationMode) {
		case SVViewState::OM_PlaceVertex :
			m_coordinateSystemActive = true;
		break;

		default:
			m_coordinateSystemActive = false;
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

	m_networkGeometryObject.m_vertexBufferData.reserve(100000);
	m_networkGeometryObject.m_colorBufferData.reserve(100000);
	m_networkGeometryObject.m_indexBufferData.reserve(100000);

	// process all network elements

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;


	// add cylinders for all pipes
	for (const VICUS::Network & n : p.m_geometricNetworks) {
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
			QColor color = Qt::black;
			if (no.m_type == VICUS::NetworkNode::NT_Mixer)
				color = Qt::green;
			else if (no.m_type == VICUS::NetworkNode::NT_Building)
				color = Qt::cyan;
			addSphere(no.m_position, color,
						radius,
						currentVertexIndex, currentElementIndex,
						m_networkGeometryObject.m_vertexBufferData,
						m_networkGeometryObject.m_colorBufferData,
						m_networkGeometryObject.m_indexBufferData);
		}
	}

}


void Vic3DScene::clearSelectionOfObjects() {
	// compose undo-action of objects currently selected
	if (m_selectedGeometryObject.m_selectedSurfaces.empty())
		return; // nothing selected, nothing to do

	std::set<unsigned int> nodeIDs;
	for (const VICUS::Surface * s : m_selectedGeometryObject.m_selectedSurfaces)
		nodeIDs.insert(s->uniqueID());
	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Selected cleared"), SVUndoTreeNodeState::SelectedState, nodeIDs, false);
	undo->push();
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

	// now do the actual picking
	selectNearestObject(nearResult.toVector3D(), farResult.toVector3D(), pickObject);
}


void Vic3DScene::selectNearestObject(const QVector3D & nearPoint, const QVector3D & farPoint, PickObject & pickObject) {
//	QElapsedTimer pickTimer;
//	pickTimer.start();

	// compute view direction (vector for line equation)
	QVector3D d = farPoint - nearPoint;
	IBKMK::Vector3D d2 = VICUS::QVector2IBKVector(d);
	IBKMK::Vector3D nearPoint2 = VICUS::QVector2IBKVector(nearPoint);

	// execute pick operation based on selection in pick object
	if (pickObject.m_pickMask & PickObject::P_XY_Plane) {
		// get intersection with xy plane
		VICUS::PlaneGeometry xyPlane(VICUS::PlaneGeometry::T_Rectangle, IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(1,0,0), IBKMK::Vector3D(0,1,0));
		IBKMK::Vector3D intersectionPoint;
		double t;
		if (xyPlane.intersectsLine(nearPoint2, d2, intersectionPoint, t, true, true)) {
			// got an intersection point, store it
			pickObject.m_dist = t;
			pickObject.m_pickPoint = intersectionPoint;
		}
	}

	// pick on surfaces?
	if (pickObject.m_pickMask & PickObject::P_Surface) {
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
						if (s.m_geometry.intersectsLine(nearPoint2, d2, intersectionPoint, dist, pickObject.m_pickMask & PickObject::P_BackSide)) {
							if (dist < pickObject.m_dist) {
								pickObject.m_dist = dist;
								pickObject.m_pickPoint = intersectionPoint;
								pickObject.m_uniqueObjectID = s.uniqueID();
							}
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
			if (s.m_geometry.intersectsLine(nearPoint2, d2, intersectionPoint, dist, pickObject.m_pickMask & PickObject::P_BackSide)) {
				if (dist < pickObject.m_dist) {
					pickObject.m_dist = dist;
					pickObject.m_pickPoint = intersectionPoint;
					pickObject.m_uniqueObjectID = s.uniqueID();
				}
			}
		}

	}
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


void Vic3DScene::handleLeftMouseClick(const KeyboardMouseHandler & keyboardHandler, const QPoint & localMousePos) {
	// do different things depending on current scene operation mode

	switch (SVViewStateHandler::instance().viewState().m_sceneOperationMode) {

		// *** place a vertex ***
		case SVViewState::OM_PlaceVertex : {
			// get current coordinate system's position
			IBKMK::Vector3D p = VICUS::QVector2IBKVector(m_coordinateSystemObject.m_transform.translation());
			// append a vertex (this will automatically update the draw buffer) and also
			// modify the vertexListWidget.
			m_newPolygonObject.appendVertex(p);
			return;
		}
		default :; // in all other modes we do selection stuff
	}

	// this will be a selection click - execute pick() operation
	PickObject o(localMousePos, PickObject::P_Surface | PickObject::P_BackSide);
	pick(o);
	if (o.m_uniqueObjectID != 0) {
		// find the selected object
		const VICUS::Object * obj = project().objectById(o.m_uniqueObjectID);
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface*>(obj);
		// for now, it must be a surface, since only surfaces are drawn
		Q_ASSERT(s != nullptr);

		// create undo-action that toggles the selection
		bool selected = s->m_selected;
		bool withoutChildren = keyboardHandler.keyDown(Qt::Key_Shift);
		// compose an undo action that selects/de-selects objects
		SVUndoTreeNodeState * action = SVUndoTreeNodeState::createUndoAction(tr("Selection changed"),
															   SVUndoTreeNodeState::SelectedState,
															   o.m_uniqueObjectID,
															   !withoutChildren,
															   !selected);
		action->push();
	}
}


} // namespace Vic3D
