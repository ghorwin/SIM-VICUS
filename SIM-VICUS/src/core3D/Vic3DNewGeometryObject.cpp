#include "Vic3DNewGeometryObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <VICUS_Project.h>
#include <QtExt_Conversions.h>

#include <IBKMK_3DCalculations.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "SVViewStateHandler.h"
#include "SVPropVertexListWidget.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DShaderProgram.h"

namespace Vic3D {

NewGeometryObject::NewGeometryObject() :
	m_vertexBufferObject(QOpenGLBuffer::VertexBuffer), // VertexBuffer is the default, so default constructor would have been enough
	m_indexBufferObject(QOpenGLBuffer::IndexBuffer) // make this an Index Buffer
{
	// make us known to the world
	SVViewStateHandler::instance().m_newGeometryObject = this;
}


void NewGeometryObject::create(ShaderProgram * shaderProgram) {
	m_shaderProgram = shaderProgram;

	// *** create buffers on GPU memory ***

	// create a new buffer for the vertices and colors, separate buffers because we will modify colors way more often than geometry
	m_vertexBufferObject.create();
	m_vertexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw); // usage pattern will be used when tranferring data to GPU

	// create a new buffer for the indexes
	m_indexBufferObject = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); // Note: make sure this is an index buffer
	m_indexBufferObject.create();
	m_indexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);


	// *** create and bind Vertex Array Object ***

	// Note: VAO must be bound *before* the element buffer is bound,
	//       because the VAO remembers associated element buffers.
	m_vao.create();
	m_vao.bind(); // now the VAO is active and remembers states modified in following calls

	m_indexBufferObject.bind(); // this registers this index buffer in the currently bound VAO


	// *** set attribute arrays for shader fetch stage ***

#define VERTEX_ARRAY_INDEX 0

	m_vertexBufferObject.bind(); // this registers this buffer data object in the currently bound vao; in subsequent
				  // calls to shaderProgramm->setAttributeBuffer() the buffer object is associated with the
				  // respective attribute array that's fed into the shader. When the vao is later bound before
				  // rendering, this association is remembered so that the vertex fetch stage pulls data from
				  // this vbo

	// coordinates
	m_shaderProgram->shaderProgram()->enableAttributeArray(VERTEX_ARRAY_INDEX);
	m_shaderProgram->shaderProgram()->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(VertexC));

	// Release (unbind) all

	// Mind: you can release the buffer data objects (vbo) before or after releasing vao. It does not
	//       matter, because the buffers are associated already with the attribute arrays.
	//       However, YOU MUST NOT release the index buffer (ebo) before releasing the vao, since this would remove
	//       the index buffer association with the vao and when binding the vao before rendering, the element buffer
	//       would not be known and a call to glDrawElements() crashes!
	m_vao.release();

	m_vertexBufferObject.release();
	m_indexBufferObject.release();

	m_planeGeometry = VICUS::PlaneGeometry();
}


void NewGeometryObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_indexBufferObject.destroy();
}


void NewGeometryObject::switchTo(NewGeometryObject::NewGeometryMode m) {
	switch (m) {
		case NGM_ZoneExtrusion :
			Q_ASSERT(m_newGeometryMode == NGM_ZoneFloor);
			m_newGeometryMode = m;
		break;
		default:;
	}
	updateBuffers(false);
}


void NewGeometryObject::flipGeometry() {
	// TODO : Andreas, improve performance?
	VICUS::Polygon3D polygon = m_planeGeometry.polygon();
	polygon.flip();
	m_planeGeometry.setPolygon(polygon); // Note: no holes in this polygon, no need to flip the holes as well
	updateBuffers(false);
}


void NewGeometryObject::appendVertex(const IBKMK::Vector3D & p) {
	// check that not the same points are added twice
	if (!m_vertexList.empty() && p==m_vertexList.back()) {
		IBK::IBK_Message("Identical point added. Ignored.");
		return; // ignore the vertex
	}
	switch (m_newGeometryMode) {
		case NGM_Rect :
			// if the rectangle is complete and we have a click, consider this as confirmation (rather than rejecting it)
			if (m_planeGeometry.isValid()) {
				finish();
				return;
			}
			// if we have already 2 points and a third is added (that is not the same as the first),
			// finish the shape by creating a polygon object
			if (m_vertexList.size() == 2) {
				if (p == m_vertexList.front()) {
					IBK::IBK_Message("Point is identical to first. Ignored.");
					return; // ignore the vertex
				}
				IBKMK::Vector3D a = m_vertexList.front();
				IBKMK::Vector3D b = m_vertexList.back();
				IBKMK::Vector3D c = p;
				IBKMK::Vector3D d = a + (c-b);
				m_planeGeometry = VICUS::PlaneGeometry(VICUS::Polygon3D::T_Rectangle, a, b, d);
				SVViewStateHandler::instance().m_propVertexListWidget->addVertex(p);
			}
			else {
				m_vertexList.push_back(p);
				m_planeGeometry.setPolygon( VICUS::Polygon3D(m_vertexList) );
				// also tell the vertex list widget about our new point
				SVViewStateHandler::instance().m_propVertexListWidget->addVertex(p);
			}
		break;

		case NGM_Polygon :
		case NGM_ZoneFloor :
			// if we have already a valid plane (i.e. normal vector not 0,0,0), then check if point is in plane
			if (m_planeGeometry.normal() != IBKMK::Vector3D(0,0,0)) {
				IBKMK::Vector3D projected;
				IBKMK::pointProjectedOnPlane(m_planeGeometry.triangleVertexes()[0], m_planeGeometry.normal(), p, projected);
				m_vertexList.push_back(projected);
			}
			else
				m_vertexList.push_back(p);
			m_planeGeometry.setPolygon( VICUS::Polygon3D(m_vertexList) );
			// also tell the vertex list widget about our new point
			SVViewStateHandler::instance().m_propVertexListWidget->addVertex(p);
		break;

		case NGM_ZoneExtrusion :
			// just signal the property list widget that we are done with the zone
			finish();
		break;

		case NUM_NGM:
			return; // nothing to do here
	}
	// finally update draw buffers
	updateBuffers(false);
}


void NewGeometryObject::appendVertexOffset(const IBKMK::Vector3D & offset) {
	if (m_vertexList.empty())
		appendVertex(offset);
	else
		appendVertex(m_vertexList.back() + offset);
}


void NewGeometryObject::removeVertex(unsigned int idx) {
	Q_ASSERT(idx < m_vertexList.size());
	m_vertexList.erase(m_vertexList.begin()+idx);
	switch (m_newGeometryMode) {
		case NGM_Polygon :
		case NGM_ZoneFloor :
			m_planeGeometry.setPolygon( VICUS::Polygon3D(m_vertexList) );
			SVViewStateHandler::instance().m_propVertexListWidget->removeVertex(idx);
		break;
		case NGM_Rect :
		case NGM_ZoneExtrusion :
		case NUM_NGM :
			Q_ASSERT(false); // operation not allowed
			return;
	}
	updateBuffers(false);
}


void NewGeometryObject::removeLastVertex() {
	switch (m_newGeometryMode) {
		case NGM_Polygon :
		case NGM_ZoneFloor :
			Q_ASSERT(!m_vertexList.empty());
			removeVertex(m_vertexList.size()-1);
		break;

		case NGM_Rect :
		case NGM_ZoneExtrusion :
		case NUM_NGM :
			Q_ASSERT(false); // operation not allowed
			return;
	}
}


void NewGeometryObject::clear() {
	m_planeGeometry.setPolygon( VICUS::Polygon3D() );
	m_vertexList.clear();
	updateBuffers(false);
}


bool NewGeometryObject::canComplete() const {
	switch (m_newGeometryMode) {
		case NGM_Rect :
		case NGM_Polygon :
			return m_planeGeometry.isValid();

		case NGM_ZoneFloor :
		case NGM_ZoneExtrusion :
		case NUM_NGM :
			return false;
	}
	return false;
}


bool NewGeometryObject::canDrawTransparent() const {
	return !m_indexBufferData.empty();
}


void NewGeometryObject::finish() {
	switch (m_newGeometryMode) {
		case NGM_Rect :
		case NGM_Polygon :
		case NGM_ZoneExtrusion :
			if (m_planeGeometry.isValid()) {
				// tell property widget to modify the project with our data
				SVViewStateHandler::instance().m_propVertexListWidget->on_pushButtonFinish_clicked();
			}
			return;
		case NGM_ZoneFloor : break; // nothing to do - cannot "finish" zone floor
		case NUM_NGM : break; // nothing to do
	}
}


VICUS::PlaneGeometry NewGeometryObject::offsetPlaneGeometry() const {
	Q_ASSERT(m_planeGeometry.isValid());
	VICUS::PlaneGeometry pg(planeGeometry());
	IBKMK::Vector3D offset = QtExt::QVector2IBKVector(m_localCoordinateSystemPosition) - m_planeGeometry.offset();
	// now offset all the coordinates
	std::vector<IBKMK::Vector3D> vertexes(pg.polygon().vertexes());
	for (IBKMK::Vector3D & v : vertexes)
		v += offset;
	pg.setPolygon( VICUS::Polygon3D(vertexes) );
	return pg;
}


void NewGeometryObject::updateLocalCoordinateSystemPosition(const QVector3D & p) {
	QVector3D newPoint = p;

	switch (m_newGeometryMode) {
		case NGM_Rect : // nothing to do - we can only ever add 3 points here
		break;

		case NGM_Polygon :
		case NGM_ZoneFloor :
			// if we have already a valid plane (i.e. normal vector not 0,0,0),
			// then check if point is in plane
			if (m_planeGeometry.normal() != IBKMK::Vector3D(0,0,0)) {
				IBKMK::Vector3D p2(QtExt::QVector2IBKVector(p));
				IBKMK::Vector3D projected;
				IBKMK::pointProjectedOnPlane(m_planeGeometry.offset(), m_planeGeometry.normal(), p2, projected);
				newPoint = QtExt::IBKVector2QVector(projected);
			}
		break;
		case NGM_ZoneExtrusion : {
			Q_ASSERT(m_planeGeometry.isValid());
			// we need to distinguish between interactive and fixed mode
			if (!m_interactiveZoneExtrusionMode) {
				IBKMK::Vector3D a = planeGeometry().polygon().vertexes()[0];
				IBKMK::Vector3D offset = m_zoneHeight*planeGeometry().normal();
				newPoint = QtExt::IBKVector2QVector(a+offset);
			}
			else {

				// compute the projection of the current coordinate systems position on the plane
				IBKMK::Vector3D p2(QtExt::QVector2IBKVector(p));
				IBKMK::Vector3D projected;
				IBKMK::pointProjectedOnPlane(m_planeGeometry.offset(), m_planeGeometry.normal(), p2, projected);
				newPoint = QtExt::IBKVector2QVector(projected);
				// now get the offset vector
				QVector3D verticalOffset = p-newPoint; // Note: this vector should be collinear to the plane's normal
				if (verticalOffset.length() < 0.001f)
					verticalOffset = QtExt::IBKVector2QVector( m_planeGeometry.offset() + 1*planeGeometry().normal() );
				// and add it to the first vertex of the polygon
				newPoint = verticalOffset + QtExt::IBKVector2QVector(m_planeGeometry.offset());
				// also store the absolute height
				m_zoneHeight = (double)verticalOffset.length();
				SVViewStateHandler::instance().m_propVertexListWidget->setExtrusionDistance(m_zoneHeight);
			}
		}
		break;

		case NUM_NGM :
			Q_ASSERT(false); // invalid call
			return;
	} // switch


	// any change to the previously stored point?
	if (m_localCoordinateSystemPosition == newPoint)
		return;

	// store new position
	m_localCoordinateSystemPosition = newPoint;

	// update buffer (but only that portion that depends on the local coordinate system's location)
	updateBuffers(true);
}


void NewGeometryObject::setZoneHeight(double height) {
	Q_ASSERT(m_newGeometryMode == NGM_ZoneExtrusion);
	m_zoneHeight = height;
	updateLocalCoordinateSystemPosition(QVector3D(0,0,0)); // point coordinates are irrelevant
}


void NewGeometryObject::updateBuffers(bool onlyLocalCSMoved) {
	// create geometry

	// memory layout:
	//   with valid polygon:          vertexBuffer = |polygon_vertexes|coordinate system vertex|
	//   without valid polygon:       vertexBuffer = |last_polygon_vertex|coordinate system vertex|

	// index buffer is only filled if valid polygon exists

	// first copy polygon from PlaneGeometry, if at least 3 vertexes are inserted
	// then add vertex to last

	/* Vertex buffer layout:
		vertexes plane geometry (only for valid polygon)
		vertexes polyline (m_vertexList)
		vertexe coordinate system
	*/

	m_vertexBufferData.clear();
	m_indexBufferData.clear();

	// no vertexes, nothing to draw - we need at least one vertex in the geometry, so that we
	// can draw a line from the last vertex to the current coordinate system's location
	if (m_planeGeometry.triangleVertexes().empty())
		return;

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	// different handling depending on the object that's being painted

	switch (m_newGeometryMode) {
		case NGM_Rect : {
			// Buffer layout:
			// - if we have 1 vertex: VC (first vertex) | VC (local coordinate system)
			// - if we have 2 vertexes: VC (first vertex) | VC (second vertex) | VC (local coordinate system) | VC (computed 4th vertex) | VC (first vertex)
			// - if we have a valid polygon: 4 * VC

			if (m_planeGeometry.isValid()) {
				addPlane(m_planeGeometry, currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);
				// re-add first vertex so that we have a closed loop
				m_vertexBufferData.push_back( m_vertexBufferData[0] );
			}
			else if (m_vertexList.size() == 1) {
				m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(m_vertexList.front())) );
				// now also add a vertex for the current coordinate system's position
				m_vertexBufferData.push_back( VertexC(m_localCoordinateSystemPosition ) );
			}
			else if (m_vertexList.size() == 2) {
				// create a temporary plane geometry from the three given vertexes
				IBKMK::Vector3D a = m_vertexList.front();
				IBKMK::Vector3D b = m_vertexList.back();
				IBKMK::Vector3D c = QtExt::QVector2IBKVector(m_localCoordinateSystemPosition);
				IBKMK::Vector3D d = a + (c-b);
				VICUS::PlaneGeometry pg(VICUS::Polygon3D::T_Rectangle, a, b, d);
				addPlane(pg, currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);
				// re-add first vertex so that we have a closed loop
				m_vertexBufferData.push_back( m_vertexBufferData[0] );
			}
		}
		break;

		case NGM_Polygon :
		case NGM_ZoneFloor : {

			// Buffer layout:
			//  - if we have a valid polygon:    npg x VC | VC (first vertex) | VC (last placed vertex) | VC (local coordinate system) | VC (first vertex)
			//  - if we have an invalid polygon: nvl x VC | VC (local coordinate system) | VC (first vertex)
			//
			//   VC  - Vertex with coordinates
			//   npg - number of vertexes in plane geometry
			//   nvl - number of vertexes in vertex list
			// Note: first vertex of nvl is also the vertex needed to close the loop
			//
			// Also note: actually, if the polygon would always be valid we would only need the vertexes of
			//            the polygon to draw the outline. But, alas, the user may add any kind of invalid vertexes
			//            and hence we also need to store

			// rendered will be:
			//   outline around polygon, using npg+1 vertexes for valid polygon, otherwise nvl vertexes
			//   render two lines: a) between last placed vertex and local coordinate system
			//                     b) between local coordinate system and first vertex
			//
			//   the transparent shape is drawn only for valid polygons

			if (m_planeGeometry.isValid()) {
				addPlane(m_planeGeometry, currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);
				// re-add first vertex so that we have a closed loop
				m_vertexBufferData.push_back( m_vertexBufferData[0] );
				// add last placed vertex to draw line to current local coordinate system
				m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(m_vertexList.back())) );
			}
			else {
				// for invalid polygons
				// put all the vertexes of the current polyline into buffer
				for (const IBKMK::Vector3D & v : m_vertexList)
					m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(v)) );
			}
			// last vertex in buffer is now the last placed vertex

			// now also add a vertex for the current coordinate system's position
			m_vertexBufferData.push_back( VertexC(m_localCoordinateSystemPosition ) );

			// add again the first point of the polygon in order to draw a blue line from local coordinate system to first point of polygon
			m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(m_vertexList.front())) );
		} break;


		case NGM_ZoneExtrusion : {
			// Buffer layout:
			//  - we add 2 polygons (top and bottom), and also nSegments planes for each wall, with m_zoneHeight as height.
			//    after each plane we add a vertex for the first point of the plane, so that we can draw outlines
			//
			//  - we render all planes transparent
			//  - for invalid polygons with just draw the outline of the placed vertexes
			//  - for valid polygons we draw the line segments of the individual planes only, but no triangulation grids
			//  - height of the polygon is taken from m_zoneHeight

			if (m_zoneHeight != 0.0) {
				Q_ASSERT(m_planeGeometry.isValid());
				// we need to create at first the base polygon
				addPlane(m_planeGeometry, currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);
				// re-add first vertex so that we have a closed loop
				m_vertexBufferData.push_back( m_vertexBufferData[0] ); ++currentVertexIndex;

				// now we create the polygon that is offset by m_zoneHeight along the plane normal vector
				VICUS::PlaneGeometry topPlane;
				std::vector<IBKMK::Vector3D> vertexes = m_planeGeometry.triangleVertexes();

				// the offset vector is the normal vector times the zoneHeight
				IBKMK::Vector3D offset = QtExt::QVector2IBKVector(m_localCoordinateSystemPosition) - vertexes[0];
				for (IBKMK::Vector3D & v : vertexes)
					v += offset;
				topPlane.setPolygon( VICUS::Polygon3D(vertexes) );
				addPlane(topPlane, currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);
				// re-add first vertex so that we have a closed loop
				m_vertexBufferData.push_back( m_vertexBufferData[m_planeGeometry.triangleVertexes().size()+1] ); ++currentVertexIndex;

				// now add vertexes to draw the vertical zone walls
				for (unsigned int i=0; i<vertexes.size(); ++i) {
					m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(m_planeGeometry.triangleVertexes()[i])) );
					m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(vertexes[i])) );
				}

			}

		} break;

		case NUM_NGM :
			Q_ASSERT(false); // invalid call
			return;
	}

	// transfer data stored in m_vertexBufferData
	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(VertexC));
	m_vertexBufferObject.release();

	if (!m_indexBufferData.empty()) {
		m_indexBufferObject.bind();
		m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GLuint));
		m_indexBufferObject.release();
	}

}


void NewGeometryObject::renderOpaque() {
	if (m_vertexBufferData.empty())
		return;

	// bind all buffers
	m_vao.bind();
	// set transformation matrix - unity matrix, since we draw with world coordinates
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[1], QMatrix4x4());

	// render lines - outline and wireframe mesh, depending on the geometry being created

	switch (m_newGeometryMode) {
		case NGM_Rect: {
			// we draw the outline first
			QColor lineCol;
			if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
				lineCol = QColor("#2ed655");
			else
				lineCol = QColor("#00ff48");
			m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], lineCol);
			glDrawArrays(GL_LINE_STRIP, 0, m_vertexBufferData.size());
		} break;

		case NGM_Polygon:
		case NGM_ZoneFloor:
			// draw the polygon line first
			if (m_vertexBufferData.size() > 1) {

				QColor lineCol;
				if (m_planeGeometry.isValid()) {
					if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
						lineCol = QColor("#2ed655");
					else
						lineCol = QColor("#00ff48");
				}
				else {
					if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
						lineCol = QColor("#c31818");
					else
						lineCol = QColor("#ed1f1f");
				}

				m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], lineCol);

				// if we have a valid polygon, draw the outline using the first npg+1 vertexes
				size_t offset = 0;
				if (m_planeGeometry.isValid()) {
					glDrawArrays(GL_LINE_STRIP, 0, m_planeGeometry.triangleVertexes().size() + 1);
					// start offset for the two lines of the local coordinate system - use the last vertex again
					offset = m_planeGeometry.triangleVertexes().size() + 1;
				}
				else {
					glDrawArrays(GL_LINE_STRIP, 0, m_vertexList.size());
					// start offset for the two lines of the local coordinate system - use the last vertex again
					offset = m_vertexList.size()-1;
				}
				// set wireframe color (TODO : make this theme-dependent?)
				if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
					lineCol = QColor("#32c5ea");
				else
					lineCol = QColor("#106d90");

				m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], lineCol);
				glDrawArrays(GL_LINE_STRIP, offset, 3);
			}
		break;
		case NGM_ZoneExtrusion: {
			QColor lineCol;
			if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
				lineCol = QColor("#32c5ea");
			else
				lineCol = QColor("#106d90");

			m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], lineCol);
			// draw outlines of bottom and top polygons first
			glDrawArrays(GL_LINE_STRIP, 0, m_planeGeometry.triangleVertexes().size()+1);
			// draw outlines of bottom and top polygons first
			glDrawArrays(GL_LINE_STRIP, m_planeGeometry.triangleVertexes().size()+1, m_planeGeometry.triangleVertexes().size()+1);

			// now draw the zone wall segments
			unsigned int offset = 2*m_planeGeometry.triangleVertexes().size()+2;
			for (unsigned int i=0; i<m_planeGeometry.triangleVertexes().size(); ++i) {
				glDrawArrays(GL_LINES, (int)(2*i + offset), 2);
			}

		} break;

		case NUM_NGM :
			Q_ASSERT(false); // invalid call
			return;
	} // switch


	// now draw the wireframe triangles - only done for polygons for now (maybe we do not need this anylonger?)
	if (!m_indexBufferData.empty() && m_newGeometryMode == NGM_Polygon) {
		glDisable(GL_CULL_FACE);

		// set wireframe color (TODO : make this theme-dependent?)
		QColor wireFrameCol;
		if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
			wireFrameCol = QColor(255,255,255,255);
		else
			wireFrameCol = QColor(120,120,120,255);

		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], wireFrameCol);
		// select wire frame drawing
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// now draw the geometry
		glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
		// switch back to fill mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// release buffers again
	m_vao.release();
}


void NewGeometryObject::renderTransparent() {
	// we expect:
	//   glDepthMask(GL_FALSE);
	//   shader program has already transform uniform set
	//   glDisable(GL_CULL_FACE);

	// the render code below is the same for all geometry types, since only the index buffer is used
	if (!m_indexBufferData.empty()) {
		// bind all buffers
		m_vao.bind();
		// set transformation matrix - unity matrix, since we draw with world coordinates
		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[1], QMatrix4x4());

		// now draw the geometry

		// disable updating of z-buffer
		glDepthMask(GL_FALSE);
		// set selected plane color (QColor is passed as vec4, so no conversion is needed, here).
		QColor planeCol = QColor(255,0,128,64);
		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], planeCol);
		// put OpenGL in offset mode
		glEnable(GL_POLYGON_OFFSET_FILL);
		// offset plane geometry a bit so that the plane is drawn behind the wireframe
		glPolygonOffset(0.1f, 2.0f);
		// now draw the geometry
		glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
		// turn off line offset mode
		glDisable(GL_POLYGON_OFFSET_FILL);

		glEnable(GL_CULL_FACE);

		// release buffers again
		m_vao.release();
	}
}

} // namespace Vic3D
