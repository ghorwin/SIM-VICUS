#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QCursor>

#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
#include "SVProjectHandler.h"

#include <VICUS_Project.h>

/*! IBKMK::Vector3D to QVector3D conversion macro. */
#define VEC2VEC(v) QVector3D((v).m_x, (v).m_y, (v).m_z)

namespace Vic3D {

void addSurface(const VICUS::Surface & s,
				unsigned int & currentVertexIndex, unsigned int & currentElementIndex,
				std::vector<Vertex> & vertexBufferData, std::vector<ColorRGBA> & colorBufferData, std::vector<GLshort> & indexBufferData)
{
	// skip invalid geometry
	if (!s.m_geometry.isValid())
		return;

	// different handling based on surface type
	switch (s.m_geometry.m_type) {
		case VICUS::PlaneGeometry::T_Triangle : {
			// insert 3 vertexes
			vertexBufferData.resize(vertexBufferData.size()+3);
			colorBufferData.resize(colorBufferData.size()+3);
			// 4 elements (3 for the triangle, 1 primitive restart index)
			indexBufferData.resize(indexBufferData.size()+4);

			vertexBufferData[currentVertexIndex    ].m_coords = VEC2VEC(s.m_geometry.m_vertexes[0]);
			vertexBufferData[currentVertexIndex + 1].m_coords = VEC2VEC(s.m_geometry.m_vertexes[1]);
			vertexBufferData[currentVertexIndex + 2].m_coords = VEC2VEC(s.m_geometry.m_vertexes[2]);
			QVector3D n = VEC2VEC(s.m_geometry.m_normal);
			vertexBufferData[currentVertexIndex    ].m_normal = n;
			vertexBufferData[currentVertexIndex + 1].m_normal = n;
			vertexBufferData[currentVertexIndex + 2].m_normal = n;

			colorBufferData[currentVertexIndex     ] = s.m_color.dark();
			colorBufferData[currentVertexIndex  + 1] = s.m_color;
			colorBufferData[currentVertexIndex  + 2] = s.m_color.light();

			// anti-clock-wise winding order for all triangles in strip
			indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
			indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
			indexBufferData[currentElementIndex + 2] = currentVertexIndex + 2;
			indexBufferData[currentElementIndex + 3] = 0xFFFF; // set stop index


			// finally advance buffer indexes
			currentVertexIndex += 3;
			currentElementIndex += 4;
		} break;

		case VICUS::PlaneGeometry::T_Rectangle : {
			// insert 4 vertexes
			vertexBufferData.resize(vertexBufferData.size()+4);
			colorBufferData.resize(colorBufferData.size()+4);
			// 5 elements (4 for the two triangles in a strip, 1 primitive restart index)
			indexBufferData.resize(indexBufferData.size()+5);

			// 4 indexes anti-clock wise in vertex memory
			QVector3D a = VEC2VEC(s.m_geometry.m_vertexes[0]);
			QVector3D b = VEC2VEC(s.m_geometry.m_vertexes[1]);
			QVector3D d = VEC2VEC(s.m_geometry.m_vertexes[2]);
			vertexBufferData[currentVertexIndex    ].m_coords = a;
			vertexBufferData[currentVertexIndex + 1].m_coords = b;
			QVector3D c = b + (d - a);
			vertexBufferData[currentVertexIndex + 2].m_coords = c;
			vertexBufferData[currentVertexIndex + 3].m_coords = d;

			QVector3D n = VEC2VEC(s.m_geometry.m_normal);
			vertexBufferData[currentVertexIndex    ].m_normal = n;
			vertexBufferData[currentVertexIndex + 1].m_normal = n;
			vertexBufferData[currentVertexIndex + 2].m_normal = n;
			vertexBufferData[currentVertexIndex + 3].m_normal = n;

			colorBufferData[currentVertexIndex     ] = s.m_color.dark();
			colorBufferData[currentVertexIndex  + 1] = s.m_color;
			colorBufferData[currentVertexIndex  + 2] = s.m_color.light();
			colorBufferData[currentVertexIndex  + 3] = s.m_color;

			// anti-clock-wise winding order for all triangles in strip
			//
			// options are:
			// a) 3, 0, 2, 1
			// b) 0, 1, 3, 2
			indexBufferData[currentElementIndex    ] = currentVertexIndex + 0;
			indexBufferData[currentElementIndex + 1] = currentVertexIndex + 1;
			indexBufferData[currentElementIndex + 2] = currentVertexIndex + 3;
			indexBufferData[currentElementIndex + 3] = currentVertexIndex + 2;
			indexBufferData[currentElementIndex + 4] = 0xFFFF; // set stop index

			// finally advance buffer indexes
			currentVertexIndex += 4;
			currentElementIndex += 5;
		} break;

		case VICUS::PlaneGeometry::T_Polygon : {
			// insert as many vertexes as there are in the polygon
			unsigned int nvert = s.m_geometry.m_vertexes.size();
			vertexBufferData.resize(vertexBufferData.size()+nvert);
			colorBufferData.resize(colorBufferData.size()+nvert);
			// nvert indexes + primitive restart index
			indexBufferData.resize(indexBufferData.size()+nvert+1);

			QVector3D n = VEC2VEC(s.m_geometry.m_normal);

			// add all vertices to buffer
			for (unsigned int i=0; i<nvert; ++i) {
				// add vertex and
				unsigned int vIdx = currentVertexIndex + i;
				vertexBufferData[vIdx].m_coords = VEC2VEC(s.m_geometry.m_vertexes[i]);
				colorBufferData[vIdx] = s.m_color;
				// build up triangle strip index buffer
				bool odd = (i % 2 != 0);
				// odd vertices are added anti-clock-wise from start, even vertices are added clock-wise from end
				unsigned int j = i / 2;
				if (odd)
					indexBufferData[currentElementIndex + i] = currentVertexIndex + j;
				else
					indexBufferData[currentElementIndex + i] = currentVertexIndex + nvert - j - 1;
			}

			indexBufferData[currentElementIndex + nvert] = 0xFFFF; // set stop index

			// finally advance buffer indexes
			currentVertexIndex += nvert;
			currentElementIndex += nvert + 1;
		} break;
	} // switch

}


void Vic3DScene::create(ShaderProgram * gridShader, ShaderProgram * buildingShader) {
	m_gridShader = gridShader;
	m_buildingShader = buildingShader;

	// *** initialize camera placement and model placement in the world

	// move camera -100 back (negative y direction -> positive x is towards the right of the screen) and 50 up
	m_camera.translate(40, -100, 50);
	// look slightly up
	m_camera.rotate(60, m_camera.right());
}


void Vic3DScene::onModified(int modificationType, ModificationInfo * data) {

	// no shader - not initialized yet, skip

	if (m_gridShader == nullptr)
		return;

	bool updateGrid = false;
	bool updateBuilding = false;
	// filter out all modification types that we handle
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		case SVProjectHandler::AllModified :
			updateGrid = true;
			updateBuilding = true;
			break;

		case SVProjectHandler::GridModified :
			updateGrid = true;
			break;

		default:
			return; // do nothing by default
	}


	// re-create grid with updated properties
	// since grid object is very small, this function also regenerates the grid line buffers and
	// uploads the data to the GPU
	if (updateGrid)
		m_gridObject.create(m_gridShader->shaderProgram());

	// create geometry object (if already existing, nothing happens here)
	if (updateBuilding) {
		m_opaqueGeometryObject.create(m_buildingShader->shaderProgram());

		// transfer data from building geometry to vertex array caches
		generateBuildingGeometry();
	}

	// update all GPU buffers (transfer cached data to GPU)
	m_opaqueGeometryObject.updateBuffers();

	// transfer other properties
}


void Vic3DScene::destroy() {
	m_gridObject.destroy();
}


void Vic3DScene::resize(int width, int height, qreal retinaScale) {
	// the projection matrix need to be updated only for window size changes
	m_projection.setToIdentity();
	// create projection matrix, i.e. camera lens
	m_projection.perspective(
				/* vertical angle */ 45.0f,
				/* aspect ratio */   width / float(height),
				/* near */           0.1f,
				/* far */            1000.0f
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
	m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
}


void Vic3DScene::inputEvent(const KeyboardMouseHandler & keyboardHandler) {
	// check for trigger key to enable fly-through-mode
	if (keyboardHandler.buttonDown(Qt::RightButton)) {

		// Handle translations
		QVector3D translation;
		if (keyboardHandler.keyDown(Qt::Key_W)) 		translation += m_camera.forward();
		if (keyboardHandler.keyDown(Qt::Key_S)) 		translation -= m_camera.forward();
		if (keyboardHandler.keyDown(Qt::Key_A)) 		translation -= m_camera.right();
		if (keyboardHandler.keyDown(Qt::Key_D)) 		translation += m_camera.right();
		if (keyboardHandler.keyDown(Qt::Key_Q)) 		translation -= m_camera.up();
		if (keyboardHandler.keyDown(Qt::Key_E)) 		translation += m_camera.up();

		float transSpeed = 0.8f;
		if (keyboardHandler.keyDown(Qt::Key_Shift))
			transSpeed = 0.1f;
		m_camera.translate(transSpeed * translation);

		// Handle rotations
		// get and reset mouse delta (pass current mouse cursor position)
		QPoint mouseDelta = keyboardHandler.mouseDelta(QCursor::pos()); // resets the internal position
		static const float rotatationSpeed  = 0.4f;
		const QVector3D LocalUp(0.0f, 0.0f, 1.0f); // same as in Camera::up()
		m_camera.rotate(-rotatationSpeed * mouseDelta.x(), LocalUp);
		m_camera.rotate(-rotatationSpeed * mouseDelta.y(), m_camera.right());

	}
	int wheelDelta = keyboardHandler.wheelDelta();
	if (wheelDelta != 0) {
		float transSpeed = 8.f;
		if (keyboardHandler.keyDown(Qt::Key_Shift))
			transSpeed = 0.8f;
		m_camera.translate(wheelDelta * transSpeed * m_camera.forward());
	}

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
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_gridObject.m_gridColor);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[2], m_background);

	m_gridObject.render();

	m_gridShader->release();


	// *** opaque background geometry ***

#ifdef Q_OS_MAC
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);
#else
	glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
#endif // Q_OS_MAC

	// tell OpenGL to show only faces whose normal vector points towards us
	glEnable(GL_CULL_FACE);


	// *** opaque building geometry ***

	// culling off, so that we see front and back sides of surfaces
//	glDisable(GL_CULL_FACE);

	m_buildingShader->bind();
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[0], m_worldToView);
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[1], m_lightPos);
	m_buildingShader->shaderProgram()->setUniformValue(m_buildingShader->m_uniformIDs[2], m_lightColor);

	m_opaqueGeometryObject.render();

	m_buildingShader->release();



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



QColor Vic3DScene::color4Surface(const VICUS::Surface & s) const {
	// for now return always white
	return Qt::white;
}



} // namespace Vic3D
