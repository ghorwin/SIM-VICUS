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

#include "Vic3DNewGeometryObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <VICUS_Project.h>
#include <QtExt_Conversions.h>

#include <IBKMK_3DCalculations.h>

#include <IBK_physics.h>

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

	m_polygonGeometry = VICUS::PlaneGeometry();
}


void NewGeometryObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_indexBufferObject.destroy();
}


void NewGeometryObject::setNewGeometryMode(NewGeometryObject::NewGeometryMode m) {
	m_newGeometryMode = m;
	updateBuffers(false);
}


void NewGeometryObject::flipGeometry() {
	VICUS::Polygon3D polygon = m_polygonGeometry.polygon();
	polygon.flip();
	m_polygonGeometry.setPolygon(polygon); // Note: no holes in this polygon, no need to flip the holes as well
	updateBuffers(false);
}


void NewGeometryObject::appendVertex(const IBKMK::Vector3D & p) {
	// check that not the same points are added twice
	if (!m_vertexList.empty() && p==m_vertexList.back()) {
		IBK::IBK_Message("Identical point added. Ignored.");
		return; // ignore the vertex
	}
	// nothing to do in passive mode
	if (m_passiveMode)
		return;
	// if in zone interactive mode, we leave it
	if (m_interactiveZoneExtrusionMode) {
		m_interactiveZoneExtrusionMode = false;
		// inform property widget about new zone height (for display in the line edit), this also
		// turns of the button for interactive mode
		SVViewStateHandler::instance().m_propVertexListWidget->setZoneHeight(m_zoneHeight);
	}
	switch (m_newGeometryMode) {
		case NGM_Rect :
			// if the rectangle is complete and we have a click, just ignore it
			if (m_polygonGeometry.isValid())
				return;

			// if we have already 2 points and a third is added (that is not the same as the first),
			// finish the shape by creating a polygon object
			if (m_vertexList.size() == 2) {
				if (p == m_vertexList.front()) {
					IBK::IBK_Message("Point is identical to first. Ignored.");
					return; // ignore the vertex
				}
				IBKMK::Vector3D a = m_vertexList.front();
				IBKMK::Vector3D b = m_vertexList.back();
				IBKMK::Vector3D c = QtExt::QVector2IBKVector(m_localCoordinateSystemPosition);
				IBKMK::Vector3D d = a + (c-b);
				m_polygonGeometry = VICUS::PlaneGeometry(VICUS::Polygon3D::T_Rectangle, a, b, d);
				// Note: the vertex list will still only contain 3 points!
				m_vertexList.push_back(c);
			}
			else {
				m_vertexList.push_back(p);
				m_polygonGeometry.setPolygon( VICUS::Polygon3D(m_vertexList) );
			}
			// also tell the vertex list widget about our new point
			SVViewStateHandler::instance().m_propVertexListWidget->addVertex(p);
		break;

		case NGM_Polygon :
			// if we have already a valid plane (i.e. normal vector not 0,0,0), then check if point is in plane
			if (m_polygonGeometry.normal() != IBKMK::Vector3D(0,0,0)) {
				IBKMK::Vector3D projected;
				IBKMK::pointProjectedOnPlane(m_polygonGeometry.offset(), m_polygonGeometry.normal(), p, projected);
				m_vertexList.push_back(projected);
			}
			else
				m_vertexList.push_back(p);
			m_polygonGeometry.setPolygon( VICUS::Polygon3D(m_vertexList) );
			// also tell the vertex list widget about our new point
			SVViewStateHandler::instance().m_propVertexListWidget->addVertex(p);
		break;

		case NGM_Zone:
		case NGM_Roof:
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
		case NGM_Rect :
			m_polygonGeometry.setPolygon( VICUS::Polygon3D(m_vertexList) );
			SVViewStateHandler::instance().m_propVertexListWidget->removeVertex(idx);
		break;
		case NGM_Zone :
		case NGM_Roof :
		case NUM_NGM :
			Q_ASSERT(false); // operation not allowed
			return;
	}
	updateBuffers(false);
}


void NewGeometryObject::removeLastVertex() {
	switch (m_newGeometryMode) {
		case NGM_Polygon :
		case NGM_Rect :
			Q_ASSERT(!m_vertexList.empty());
			removeVertex(m_vertexList.size()-1);
		break;

		case NGM_Zone :
		case NGM_Roof :
		case NUM_NGM :
			Q_ASSERT(false); // operation not allowed
			return;
	}
}


void NewGeometryObject::clear() {
	m_polygonGeometry.setPolygon( VICUS::Polygon3D() );
	m_generatedGeometry.clear();
	m_vertexList.clear();
	updateBuffers(false);
}


bool NewGeometryObject::canComplete() const {
	switch (m_newGeometryMode) {
		case NGM_Rect :
		case NGM_Polygon :
			return m_polygonGeometry.isValid();

		case NGM_Zone :
		case NGM_Roof :
		case NUM_NGM :
			return false;
	}
	return false;
}



void NewGeometryObject::updateLocalCoordinateSystemPosition(const QVector3D & p) {
	QVector3D newPoint = p;

	switch (m_newGeometryMode) {
		case NGM_Rect :
			// any change to the previously stored point?
			if (m_localCoordinateSystemPosition == newPoint)
				return;

			// if we have already 2 points, we try to compute a valid plane geometry (temporary) from the new point
			if (m_vertexList.size() == 2) {
				IBKMK::Polygon3D d;
				d.setVertexes(m_vertexList);
				d.addVertex(QtExt::QVector2IBKVector(p));
				if (d.isValid()) {
					// now compute orthogonal vector to first segment and normal of plane
					IBKMK::Vector3D first = m_vertexList[0] - m_vertexList[1];
					IBKMK::Vector3D second;
					first.crossProduct(d.normal(), second); // second = n x first
					second.normalize();
					qDebug() << QtExt::IBKVector2QVector(first) << QtExt::IBKVector2QVector(d.normal()) << QtExt::IBKVector2QVector(second);
					// compute projection onto line, starting at m_vertexList[1]

					// vector from new point to starting point
					IBKMK::Vector3D v = d.vertexes()[2] - d.vertexes()[1];
					// offset
					double scale = v.scalarProduct(second);
					IBKMK::Vector3D L = second*scale + d.vertexes()[1];
					newPoint = QtExt::IBKVector2QVector(L);
					qDebug() << QtExt::IBKVector2QVector(v) << scale << QtExt::IBKVector2QVector(L) << newPoint;
				}
			}

			// store new position
			m_localCoordinateSystemPosition = newPoint;

			// update buffer (but only that portion that depends on the local coordinate system's location)
			updateBuffers(true);
		break;

		case NGM_Polygon :
			// if we have already a valid plane (i.e. normal vector not 0,0,0),
			// then check if point is in plane
			if (m_polygonGeometry.normal() != IBKMK::Vector3D(0,0,0)) {
				IBKMK::Vector3D p2(QtExt::QVector2IBKVector(p));
				IBKMK::Vector3D projected;
				IBKMK::pointProjectedOnPlane(m_polygonGeometry.offset(), m_polygonGeometry.normal(), p2, projected);
				newPoint = QtExt::IBKVector2QVector(projected);
			}
			// any change to the previously stored point?
			if (m_localCoordinateSystemPosition == newPoint)
				return;

			// store new position
			m_localCoordinateSystemPosition = newPoint;

			// update buffer (but only that portion that depends on the local coordinate system's location)
			updateBuffers(true);
		break;

		case NGM_Zone: {
			Q_ASSERT(m_polygonGeometry.isValid());
			// only relevant if in interactive zone extrusion mode
			if (m_interactiveZoneExtrusionMode) {

				// compute the projection of the current coordinate systems position on the plane
				IBKMK::Vector3D p2(QtExt::QVector2IBKVector(p));
				IBKMK::Vector3D projected;
				IBKMK::pointProjectedOnPlane(m_polygonGeometry.offset(), m_polygonGeometry.normal(), p2, projected);
				newPoint = QtExt::IBKVector2QVector(projected);
				// now get the offset vector
				QVector3D verticalOffset = p-newPoint; // Note: this vector should be collinear to the plane's normal
				if (verticalOffset.length() < 0.001f)
					verticalOffset = QtExt::IBKVector2QVector( m_polygonGeometry.offset() + 1*planeGeometry().normal() );
				// and add it to the first vertex of the polygon
				newPoint = verticalOffset + QtExt::IBKVector2QVector(m_polygonGeometry.offset());
				m_localCoordinateSystemPosition = newPoint;
				// also store the absolute height and update the generated geometry
				setZoneHeight((double)verticalOffset.length()); // this also updates the buffers
				// inform property widget about new zone height (for display in the line edit)
				SVViewStateHandler::instance().m_propVertexListWidget->setZoneHeight(m_zoneHeight);
			}
		}
		break;

		case NGM_Roof :
			return; // nothing to do here

		case NUM_NGM :
			Q_ASSERT(false); // invalid call
			return;
	} // switch
}


void NewGeometryObject::setZoneHeight(double height) {
	Q_ASSERT(m_newGeometryMode == NGM_Zone);
	Q_ASSERT(m_polygonGeometry.isValid());

	m_zoneHeight = height;

	IBKMK::Vector3D offset = m_zoneHeight*planeGeometry().normal();

	// generate extruded ceiling plane
	VICUS::PlaneGeometry pg(planeGeometry());
	// now offset all the coordinates
	std::vector<IBKMK::Vector3D> vertexes(pg.polygon().vertexes());
	for (IBKMK::Vector3D & v : vertexes)
		v += offset;
	pg.setPolygon( VICUS::Polygon3D(vertexes) );
	m_generatedGeometry.clear();
	m_generatedGeometry.push_back(pg);
	updateBuffers(false);
}


void NewGeometryObject::setRoofGeometry(const RoofInputData & roofData, const std::vector<IBKMK::Vector3D> &floorPolygon) {
	Q_ASSERT(m_newGeometryMode == NGM_Roof);
	Q_ASSERT(m_polygonGeometry.isValid());
	// generate roof geometry

	// we need all the time a local coordinate system where all points have a z-value of 0

	// Get a _copy_ of the floor polygon
	std::vector<IBKMK::Vector3D> polyline = floorPolygon;

	if (roofData.m_rotate) {
		unsigned int vertexCount = polyline.size();
		std::vector<IBKMK::Vector3D> rotatedPolyline;
		for (unsigned int i=0; i<vertexCount; ++i)
			rotatedPolyline.push_back(polyline[(i + 1) % vertexCount]);
		rotatedPolyline.swap(polyline);
	}

	// All roof polygons stored in the following vector.
	// Once we are done, we generate PlaneGeometry objects for each polygon.
	std::vector<VICUS::Polygon3D> polys;

	// for all but complex shape
	if (roofData.m_type != RoofInputData::Complex) {

		// If there are only 3 points and the roof shape is not COMPLEX then a 4th point is always added.
		// If there are more than 3 points, all further points are discarded. This ensures that there is always a rectangle.
		if(polyline.size() > 3)
			polyline.erase(polyline.begin()+3, polyline.end());
		// Add fourth point
		polyline.push_back(polyline[2]+(polyline[0]-polyline[1]));

		//make a horizontal plane
		for(auto &p : polyline)
			p.m_z = polyline[0].m_z;

		//create floor polygon
		{
			VICUS::Polygon3D poly3d;
			poly3d.setVertexes(polyline);
			if(poly3d.normal().m_z > 0)
				poly3d.flip();
			m_polygonGeometry.setPolygon(poly3d);
		}

		//distance of point 0 to 1
		double distAB = polyline[1].distanceTo(polyline[0]);
		double height = roofData.m_height;
		double angle  = roofData.m_angle;
		//calculate height
		if (!roofData.m_isHeightPredefined)
			height = std::tan(roofData.m_angle * IBK::DEG2RAD) * distAB;
		//calculate angle
		else{
			///TODO Dirk->Andreas fehlerbehandlung?
			if(distAB>0)
				angle = std::atan(roofData.m_height/distAB);
			else
				angle = 0;
		}
		// all polygons of the roof room are stored following vector
		std::vector<std::vector<IBKMK::Vector3D> > polygons;

		// add already calculated floor polygon
		polygons.push_back(polyline);

		/// TODO Dirk check if floor polygon has the right normal
		IBKMK::Vector3D hFlapTile(0,0,roofData.m_hasFlapTile ? roofData.m_flapTileHeight : 0);
		switch (roofData.m_type){
			case RoofInputData::SinglePitchRoof:{
				// Create a single pitch roof with floor, roof, 3x wall, if flapTile>0 then additional 1x wall
				polygons.resize(roofData.m_hasFlapTile ? 6 : 5);
				// Note: floor polygon is already set at index [0]
				IBKMK::Vector3D h1(0,0,height);
				//roof
				polygons[1].push_back(polyline[0]+hFlapTile);
				polygons[1].push_back(polyline[1]+h1+hFlapTile);
				polygons[1].push_back(polyline[2]+h1+hFlapTile);
				polygons[1].push_back(polyline[3]+hFlapTile);
				//wall 1
				polygons[2].push_back(polyline[0]+hFlapTile);
				if(roofData.m_hasFlapTile) polygons[2].push_back(polyline[0]);
				polygons[2].push_back(polyline[1]);
				polygons[2].push_back(polyline[1]+h1+hFlapTile);
				//wall 2
				polygons[3].push_back(polyline[1]);
				polygons[3].push_back(polyline[2]);
				polygons[3].push_back(polyline[2]+h1+hFlapTile);
				polygons[3].push_back(polyline[1]+h1+hFlapTile);
				//wall 3
				polygons[4].push_back(polyline[2]);
				polygons[4].push_back(polyline[3]);
				if(roofData.m_hasFlapTile) polygons[4].push_back(polyline[3]+hFlapTile);
				polygons[4].push_back(polyline[2]+h1+hFlapTile);
				//wall 4 only by flap tile mode
				if(roofData.m_hasFlapTile){
					polygons[5].push_back(polyline[3]);
					polygons[5].push_back(polyline[0]);
					polygons[5].push_back(polyline[0]+hFlapTile);
					polygons[5].push_back(polyline[3]+hFlapTile);
				}

			}
			break;
			case RoofInputData::DoublePitchRoof:{
				// Create a double pitch roof with floor, 2x roof, 2x wall, if flapTile>0 then additional 2x wall
				polygons.resize(roofData.m_hasFlapTile ? 7 : 5);

				IBKMK::Vector3D middleBA= (polyline[1]-polyline[0])*0.5;
				IBKMK::Vector3D h1(0,0,height);
				//roof 1
				polygons[1].push_back(polyline[0]+hFlapTile);
				polygons[1].push_back(polyline[0]+ middleBA+h1+hFlapTile);
				polygons[1].push_back(polyline[3]+ middleBA+h1+hFlapTile);
				polygons[1].push_back(polyline[3]+hFlapTile);
				//roof 2
				polygons[2].push_back(polyline[1]+hFlapTile);
				polygons[2].push_back(polyline[2]+hFlapTile);
				polygons[2].push_back(polyline[2]- middleBA+h1+hFlapTile);
				polygons[2].push_back(polyline[1]- middleBA+h1+hFlapTile);
				//wall 1
				polygons[3].push_back(polyline[0]);
				polygons[3].push_back(polyline[1]);
				if(roofData.m_hasFlapTile) polygons[3].push_back(polyline[1]+hFlapTile);
				polygons[3].push_back(polyline[0]+ middleBA+h1+hFlapTile);
				if(roofData.m_hasFlapTile) polygons[3].push_back(polyline[0]+hFlapTile);
				//wall 2
				polygons[4].push_back(polyline[2]);
				polygons[4].push_back(polyline[3]);
				if(roofData.m_hasFlapTile) polygons[4].push_back(polyline[3]+hFlapTile);
				polygons[4].push_back(polyline[3]+ middleBA+h1+hFlapTile);
				if(roofData.m_hasFlapTile) polygons[4].push_back(polyline[2]+hFlapTile);

				//wall 3 & 4 only by flap tile mode
				if(roofData.m_hasFlapTile){
					polygons[5].push_back(polyline[3]);
					polygons[5].push_back(polyline[0]);
					polygons[5].push_back(polyline[0]+hFlapTile);
					polygons[5].push_back(polyline[3]+hFlapTile);

					polygons[6].push_back(polyline[1]);
					polygons[6].push_back(polyline[2]);
					polygons[6].push_back(polyline[2]+hFlapTile);
					polygons[6].push_back(polyline[1]+hFlapTile);
				}
			}
			break;
			case RoofInputData::MansardRoof:{
				// Create a mansard roof with floor, 4x roof, 2x wall, if flapTile>0 then additional 2x wall
				polygons.resize(roofData.m_hasFlapTile ? 9 : 7 );

				IBKMK::Vector3D middleBA= (polyline[1]-polyline[0])*0.5;
				IBKMK::Vector3D vec1= (polyline[1]-polyline[0])*0.1;
				IBKMK::Vector3D h1(0,0,roofData.m_height), h2(0,0,roofData.m_height*0.5);
				//roof 1
				polygons[1].push_back(polyline[0]+hFlapTile);
				polygons[1].push_back(polyline[0]+vec1+h2+hFlapTile);
				polygons[1].push_back(polyline[3]+vec1+h2+hFlapTile);
				polygons[1].push_back(polyline[3]+hFlapTile);
				//roof 2
				polygons[2].push_back(polyline[0]+vec1+h2+hFlapTile);
				polygons[2].push_back(polyline[0]+middleBA+h1+hFlapTile);
				polygons[2].push_back(polyline[3]+middleBA+h1+hFlapTile);
				polygons[2].push_back(polyline[3]+vec1+h2+hFlapTile);
				//roof 3
				polygons[3].push_back(polyline[1]+hFlapTile);
				polygons[3].push_back(polyline[2]+hFlapTile);
				polygons[3].push_back(polyline[2]-vec1+h2+hFlapTile);
				polygons[3].push_back(polyline[1]-vec1+h2+hFlapTile);
				//roof 4
				polygons[4].push_back(polyline[1]-vec1+h2+hFlapTile);
				polygons[4].push_back(polyline[2]-vec1+h2+hFlapTile);
				polygons[4].push_back(polyline[2]-middleBA+h1+hFlapTile);
				polygons[4].push_back(polyline[1]-middleBA+h1+hFlapTile);

				//wall 1
				polygons[5].push_back(polyline[0]);
				polygons[5].push_back(polyline[1]);
				if(roofData.m_hasFlapTile) polygons[5].push_back(polyline[1]+hFlapTile);
				polygons[5].push_back(polyline[1]-vec1+h2+hFlapTile);
				polygons[5].push_back(polyline[0]+middleBA+h1+hFlapTile);
				polygons[5].push_back(polyline[0]+vec1+h2+hFlapTile);
				if(roofData.m_hasFlapTile) polygons[5].push_back(polyline[0]+hFlapTile);
				//wall 2
				polygons[6].push_back(polyline[2]);
				polygons[6].push_back(polyline[3]);
				if(roofData.m_hasFlapTile) polygons[6].push_back(polyline[3]+hFlapTile);
				polygons[6].push_back(polyline[3]+vec1+h2+hFlapTile);
				polygons[6].push_back(polyline[3]+middleBA+h1+hFlapTile);
				polygons[6].push_back(polyline[2]-vec1+h2+hFlapTile);
				if(roofData.m_hasFlapTile) polygons[6].push_back(polyline[2]+hFlapTile);

				if(roofData.m_hasFlapTile){
					//wall 3
					polygons[7].push_back(polyline[3]);
					polygons[7].push_back(polyline[0]);
					polygons[7].push_back(polyline[0]+hFlapTile);
					polygons[7].push_back(polyline[3]+hFlapTile);

					//wall 4
					polygons[8].push_back(polyline[1]);
					polygons[8].push_back(polyline[2]);
					polygons[8].push_back(polyline[2]+hFlapTile);
					polygons[8].push_back(polyline[1]+hFlapTile);
				}
			}
			break;
			case RoofInputData::HipRoof:{
				// Create a hip roof with floor, 4 roof, 0x wall, if flapTile>0 then additional 4x wall
				polygons.resize(roofData.m_hasFlapTile ? 9 : 5);

				IBKMK::Vector3D middleBA= (polyline[1]-polyline[0])*0.5;
				IBKMK::Vector3D h1(0,0,height);
				IBKMK::Vector3D d1(0,0,0);
				double wid = polyline[2].distanceTo(polyline[1]);
				double len = polyline[1].distanceTo(polyline[0]);
				if(len != 0)
					d1 = (polyline[2]-polyline[1]) * 0.1;

				//roof 1
				polygons[1].push_back(polyline[3] + hFlapTile);
				polygons[1].push_back(polyline[0] + hFlapTile);
				polygons[1].push_back(polyline[0] + middleBA+d1+h1 + hFlapTile);
				polygons[1].push_back(polyline[3] + middleBA-d1+h1 + hFlapTile);
				//roof 2
				polygons[2].push_back(polyline[1] + hFlapTile);
				polygons[2].push_back(polyline[2] + hFlapTile);
				polygons[2].push_back(polyline[2] - middleBA-d1+h1 + hFlapTile);
				polygons[2].push_back(polyline[1] - middleBA+d1+h1 + hFlapTile);
				//roof 3
				polygons[3].push_back(polyline[0] + hFlapTile);
				polygons[3].push_back(polyline[1] + hFlapTile);
				polygons[3].push_back(polyline[0] + middleBA+d1+h1 + hFlapTile);
				//roof 4
				polygons[4].push_back(polyline[2] + hFlapTile);
				polygons[4].push_back(polyline[3] + hFlapTile);
				polygons[4].push_back(polyline[3] + middleBA-d1+h1 + hFlapTile);

				if(roofData.m_hasFlapTile){
					// additional walls
					for(unsigned int i1 = 0; i1<4; ++i1){
						unsigned int i2 = (i1+1) % 4;
						polygons[5+i1].push_back(polyline[i1]);
						polygons[5+i1].push_back(polyline[i2]);
						polygons[5+i1].push_back(polyline[i2]+hFlapTile);
						polygons[5+i1].push_back(polyline[i1]+hFlapTile);

					}
				}
			}
			break;
			case RoofInputData::Complex:
				//do nothing because the frist "if" does not allow complex
			break;
		}
		//add all polygons to the poly vec and flip all normals of the roof elements to positiv z-value
		//dont take floor polygon so we start by index 1
		for(unsigned int i=1; i<polygons.size(); ++i)
			polys.push_back(polygons[i]);
	}
	else {
		// create floor polygon
		double zHeight = polyline.front().m_z;
		{
			VICUS::Polygon3D poly3d;
			poly3d.setVertexes(polyline);
			if(poly3d.normal().m_z == 0)
			{
				/// TODO Dirk Fehler abfangen wenn das Polygon senkrecht zur x-y-Ebene aufgebaut wird
				return;
			}
			// transform all coordinates to x-y-plane
			for(IBKMK::Vector3D &p : polyline)
				p.m_z = zHeight;

			poly3d.setVertexes(polyline);

			if(poly3d.normal().m_z > 0)
				poly3d.flip();
			m_polygonGeometry.setPolygon(poly3d);
		}

		if(polyline.size()==3){


			//create middle point of all 3 points
			//then create 3 triangles
			IBKMK::Vector2D p0(polyline[0].m_x, polyline[0].m_y);
			IBKMK::Vector2D p1(polyline[1].m_x, polyline[1].m_y);
			IBKMK::Vector2D p2(polyline[2].m_x, polyline[2].m_y);
			IBKMK::Vector2D mid2D = ( p0 +  p1 + p2);
			mid2D *= (1/3.);

			//create 3x roof
			for(unsigned int i=0; i<3; ++i){
				VICUS::Polygon3D poly3d;
				poly3d.addVertex(IBKMK::Vector3D(mid2D.m_x, mid2D.m_y, roofData.m_height + zHeight));
				poly3d.addVertex(IBKMK::Vector3D(polyline[i].m_x, polyline[i].m_y,  zHeight));
				poly3d.addVertex(IBKMK::Vector3D(polyline[(i+1)%3].m_x, polyline[(i+1)%3].m_y,  + zHeight));
				polys.push_back(poly3d);
			}
		}
		else{
			//now create a complex roof structure

			unsigned int polySize = polyline.size();
			std::vector<IBK::point2D<double>> points2D(polySize);
			std::vector<std::pair<unsigned int, unsigned int>> edges;
			/// TODO Dirk->Andreas transformieren der 3D Punkte in 2D Punkte
			/// für polyline
			/// jetzt geht das erstmal nur über weglassen der z-Koordinate
			for(unsigned int i=0; i<polySize; ++i){
				points2D[i].m_x = polyline[i].m_x;
				points2D[i].m_y = polyline[i].m_y;
				edges.push_back(std::pair<unsigned int, unsigned int>(i, (i+1)%polySize));
			}

			IBKMK::Triangulation triangu;
			triangu.setPoints(points2D,edges);

			// For each triangle, store all edges that have neighbors.
			std::map<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>> neighboringEdgesOfTri;
			for(unsigned int i=0; i<triangu.m_triangles.size(); ++i){
				const IBKMK::Triangulation::triangle_t &tri1 = triangu.m_triangles[i];

				//create a set with the first three point indicies
				std::set<unsigned int> indexSet;
				indexSet.insert(tri1.i1);
				indexSet.insert(tri1.i2);
				indexSet.insert(tri1.i3);

				for(unsigned int j=i+1; j<triangu.m_triangles.size(); ++j){
					const IBKMK::Triangulation::triangle_t &tri2 = triangu.m_triangles[j];
					unsigned int counter = 3;
					std::set<unsigned int> tempSet;
					std::vector<unsigned int> saveIdx;
					tempSet = indexSet;
					//check if 2 indicies are in the set --> then we have a midpoint
					tempSet.insert(tri2.i1);
					if(tempSet.size() == counter)
						saveIdx.push_back(tri2.i1);
					else
						++counter;
					tempSet.insert(tri2.i2);
					if(tempSet.size() == counter)
						saveIdx.push_back(tri2.i2);
					else
						++counter;
					tempSet.insert(tri2.i3);
					if(tempSet.size() == counter)
						saveIdx.push_back(tri2.i3);
					else
						++counter;

					//found midpoint
					if(saveIdx.size()==2){
						//Store the two indices of the points that form the center
						neighboringEdgesOfTri[i].push_back(std::pair<unsigned int, unsigned int>(saveIdx[0], saveIdx[1]));
						neighboringEdgesOfTri[j].push_back(std::pair<unsigned int, unsigned int>(saveIdx[0], saveIdx[1]));
					}
				}
			}


			//Now three cases may have arisen.
			//1. one high point -> two roof triangles are created
			//2. two high points -> three roof triangles are created
			//3. three high points -> four roof triangles are created, one of them forms a horizontal plane
			for(auto &e : neighboringEdgesOfTri){
				//get triangle
				const IBKMK::Triangulation::triangle_t &tri = triangu.m_triangles[e.first];
				//get points
				std::vector<IBKMK::Vector3D> pts(3, IBKMK::Vector3D(0,0,0));
				pts[0].m_x = points2D[tri.i1].m_x;
				pts[0].m_y = points2D[tri.i1].m_y;

				pts[1].m_x = points2D[tri.i2].m_x;
				pts[1].m_y = points2D[tri.i2].m_y;

				pts[2].m_x = points2D[tri.i3].m_x;
				pts[2].m_y = points2D[tri.i3].m_y;

				switch(e.second.size()){
					case 1 :{
						IBKMK::Vector2D p1 = points2D[e.second.front().first];
						IBKMK::Vector2D p2 = points2D[e.second.front().second];
						IBKMK::Vector2D p3;
						if(tri.i1 != e.second.front().first && tri.i1 != e.second.front().second)
							p3 = points2D[tri.i1];
						else if (tri.i2 != e.second.front().first && tri.i2 != e.second.front().second)
							p3 = points2D[tri.i2];
						else
							p3 = points2D[tri.i3];

						IBKMK::Vector2D mid2D = p1+ (p2 - p1)*0.5;

						VICUS::Polygon3D poly3d;
						//first poly
						poly3d.addVertex(IBKMK::Vector3D(mid2D.m_x, mid2D.m_y, roofData.m_height + zHeight));
						poly3d.addVertex(IBKMK::Vector3D(p1.m_x, p1.m_y,  zHeight));
						poly3d.addVertex(IBKMK::Vector3D(p3.m_x, p3.m_y,  zHeight));
						polys.push_back(poly3d);

						//second poly
						poly3d.setVertexes(std::vector<IBKMK::Vector3D>{
											   IBKMK::Vector3D(mid2D.m_x, mid2D.m_y, roofData.m_height + zHeight),
											   IBKMK::Vector3D(p2.m_x, p2.m_y,  zHeight),
											   IBKMK::Vector3D(p3.m_x, p3.m_y,  zHeight)
										   });
						polys.push_back(poly3d);
					}
					break;
					case 2:{
						std::vector<IBKMK::Vector2D> pts2DVec{
							points2D[e.second.front().first],
							points2D[e.second.front().second],
							points2D[e.second.back().first],
							points2D[e.second.back().second]
						};
						//index of the common point
						IBKMK::Vector2D commonPoint;
						IBKMK::Vector2D p1, p2;

						//midpoints
						IBKMK::Vector2D mid2Da = pts2DVec[0]+ (pts2DVec[1] - pts2DVec[0])*0.5;
						IBKMK::Vector2D mid2Db = pts2DVec[2]+ (pts2DVec[3] - pts2DVec[2])*0.5;

						//find common point and the other two points
						if(pts2DVec[0] == pts2DVec[2] || pts2DVec[0] == pts2DVec[3]){
							commonPoint = pts2DVec[0];
							p1 = pts2DVec[1];
						}
						else{
							p1 = pts2DVec[0];
							commonPoint = pts2DVec[1];
						}
						p2 = commonPoint == pts2DVec[2] ? pts2DVec[3] : pts2DVec[2];

						VICUS::Polygon3D poly3d;
						//create first triangle with the two mid points and the common point
						poly3d.setVertexes(std::vector<IBKMK::Vector3D>{
											   IBKMK::Vector3D(mid2Da.m_x, mid2Da.m_y, roofData.m_height + zHeight),
											   IBKMK::Vector3D(mid2Db.m_x, mid2Db.m_y, roofData.m_height + zHeight),
											   IBKMK::Vector3D(commonPoint.m_x, commonPoint.m_y, zHeight)
										   });
						polys.push_back(poly3d);

						//create second triangle with the one mid point and the two other points
						poly3d.setVertexes(std::vector<IBKMK::Vector3D>{
											   IBKMK::Vector3D(mid2Da.m_x, mid2Da.m_y, roofData.m_height + zHeight),
											   IBKMK::Vector3D(p1.m_x, p1.m_y, zHeight),
											   IBKMK::Vector3D(p2.m_x, p2.m_y, zHeight)
										   });
						polys.push_back(poly3d);

						//create third triangle with the two mid points and the one other point
						poly3d.setVertexes(std::vector<IBKMK::Vector3D>{
											   IBKMK::Vector3D(mid2Da.m_x, mid2Da.m_y, roofData.m_height + zHeight),
											   IBKMK::Vector3D(mid2Db.m_x, mid2Db.m_y, roofData.m_height + zHeight),
											   commonPoint == pts2DVec[2] ? IBKMK::Vector3D(pts2DVec[3].m_x, pts2DVec[3].m_y, zHeight) :
											   IBKMK::Vector3D(pts2DVec[2].m_x, pts2DVec[2].m_y, zHeight)
										   });
						polys.push_back(poly3d);

					}
					break;
					case 3: {

						//index of the common point
						unsigned int idxCommon = 0;
						IBKMK::Vector2D commonPoint;
						std::vector<IBKMK::Vector2D> pts2DVec{points2D[tri.i1],points2D[tri.i2],points2D[tri.i3]};
						std::vector<IBKMK::Vector2D> midPts2DVec(3);

						//midpoints
						for(unsigned int i3=0; i3<3; ++i3){
							unsigned int i2 = (i3+1)%3;
							midPts2DVec[i3] = pts2DVec[i3] + (pts2DVec[i2]-pts2DVec[i3])*0.5;
						}

						VICUS::Polygon3D poly3d;
						//create first triangle --> all mid points
						poly3d.setVertexes(std::vector<IBKMK::Vector3D>{
											   IBKMK::Vector3D(midPts2DVec[0].m_x, midPts2DVec[0].m_y, roofData.m_height + zHeight),
											   IBKMK::Vector3D(midPts2DVec[1].m_x, midPts2DVec[1].m_y, roofData.m_height + zHeight),
											   IBKMK::Vector3D(midPts2DVec[2].m_x, midPts2DVec[2].m_y, roofData.m_height + zHeight)
										   });
						polys.push_back(poly3d);

						//create three more triangles
						//each has two mid points and a other point
						for(unsigned int i3=0; i3<3; ++i3){
							//find the other point which belongs to the two mid points
							poly3d.setVertexes(std::vector<IBKMK::Vector3D>{
												   IBKMK::Vector3D(midPts2DVec[i3].m_x, midPts2DVec[i3].m_y, roofData.m_height + zHeight),
												   IBKMK::Vector3D(midPts2DVec[(i3+1)%3].m_x, midPts2DVec[(i3+1)%3].m_y, roofData.m_height + zHeight),
												   IBKMK::Vector3D(pts2DVec[(i3+1)%3].m_x, pts2DVec[(i3+1)%3].m_y, zHeight)
											   });
							polys.push_back(poly3d);


						}
					}
					break;
				}
			}
		}

	}

	m_generatedGeometry.clear();
	for(unsigned int i=0; i<polys.size(); ++i){
		//flip polygon because wrong direction of normal
		if(polys[i].normal().m_z < 0 )
			polys[i].flip();

		VICUS::PlaneGeometry pg;
		pg.setPolygon(polys[i]);
		m_generatedGeometry.push_back(pg);
	}
	updateBuffers(false);
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
	if (m_polygonGeometry.polygon().vertexes().empty())
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

			if (m_polygonGeometry.isValid()) {
				addPlane(m_polygonGeometry.triangulationData(), currentVertexIndex, currentElementIndex,
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
				addPlane(pg.triangulationData(), currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);
				// re-add first vertex so that we have a closed loop
				m_vertexBufferData.push_back( m_vertexBufferData[0] );
			}
		}
		break;

		case NGM_Polygon : {

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

			if (m_polygonGeometry.isValid()) {
				addPlane(m_polygonGeometry.triangulationData(), currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);
				// re-add first vertex so that we have a closed loop
				m_vertexBufferData.push_back( m_vertexBufferData[0] );
				if (!m_passiveMode)
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

			if (!m_passiveMode) {
				// now also add a vertex for the current coordinate system's position
				m_vertexBufferData.push_back( VertexC(m_localCoordinateSystemPosition ) );

				// add again the first point of the polygon in order to draw a blue line from local coordinate system to first point of polygon
				m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(m_vertexList.front())) );
			}
		} break;


		case NGM_Zone : {
			// Buffer layout:
			//  - we add 2 polygons (top and bottom), and also nSegments planes for each wall, with m_zoneHeight as height.
			//    after each plane we add a vertex for the first point of the plane, so that we can draw outlines
			//
			//  - we render all planes transparent
			//  - for invalid polygons with just draw the outline of the placed vertexes
			//  - for valid polygons we draw the line segments of the individual planes only, but no triangulation grids
			//  - height of the polygon is taken from m_zoneHeight

			if (m_zoneHeight != 0.0 && m_generatedGeometry.size() == 1) { // note: protect against incomplete data - if just switching to Zone mode
				Q_ASSERT(m_polygonGeometry.isValid());
				// we need to create at first the base polygon
				addPlane(m_polygonGeometry.triangulationData(), currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);
				// now the roof geometry
				addPlane(m_generatedGeometry[0].triangulationData(), currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);

				const std::vector<IBKMK::Vector3D> & floorPolygonVertexes = m_polygonGeometry.polygon().vertexes();
				const std::vector<IBKMK::Vector3D> & ceilingPolygonVertexes = m_generatedGeometry[0].polygon().vertexes();

				// now add a line strip for the bottom polygon
				for (unsigned int i=0; i<=floorPolygonVertexes.size(); ++i)
					m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(floorPolygonVertexes[i % floorPolygonVertexes.size()])) );
				// now add a line strip for the top polygon
				for (unsigned int i=0; i<=ceilingPolygonVertexes.size(); ++i)
					m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(ceilingPolygonVertexes[i % ceilingPolygonVertexes.size()])) );

				// now add vertexes to draw the vertical zone walls
				for (unsigned int i=0; i<floorPolygonVertexes.size(); ++i) {
					m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(floorPolygonVertexes[i])) );
					m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(ceilingPolygonVertexes[i])) );
				}
			}

		} break;

		case NGM_Roof : {
			// we need to create at first the base polygon
			addPlane(m_polygonGeometry.triangulationData(), currentVertexIndex, currentElementIndex,
					 m_vertexBufferData, m_indexBufferData);
			// now all the planes belonging to the roof
			for (unsigned int i=0; i<m_generatedGeometry.size(); ++i)
				addPlane(m_generatedGeometry[i].triangulationData(), currentVertexIndex, currentElementIndex,
						 m_vertexBufferData, m_indexBufferData);

			m_lineStartVertex = currentVertexIndex;

			// now add a line strip for the bottom polygon
			const std::vector<IBKMK::Vector3D> & vertexes = m_polygonGeometry.triangulationData().m_vertexes;
			for (unsigned int i=0; i<=vertexes.size(); ++i)
				m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(vertexes[i % vertexes.size()])) );
			// now add a line strip for all other planes
			for (unsigned int j=0; j<m_generatedGeometry.size(); ++j) {
				const std::vector<IBKMK::Vector3D> & vertexes2 = m_generatedGeometry[j].triangulationData().m_vertexes;
				for (unsigned int i=0; i<=vertexes2.size(); ++i)
					m_vertexBufferData.push_back( VertexC(QtExt::IBKVector2QVector(vertexes2[i % vertexes2.size()])) );
			}
		}
		break;

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
			// draw the polygon line first
			if (m_vertexBufferData.size() > 1) {

				QColor lineCol;
				if (m_polygonGeometry.isValid()) {
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
				size_t vertexCount = m_polygonGeometry.triangulationData().m_vertexes.size();
				if (m_polygonGeometry.isValid()) {
					glDrawArrays(GL_LINE_STRIP, 0, vertexCount + 1);
					// start offset for the two lines of the local coordinate system - use the last vertex again
					offset = vertexCount + 1;
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

		case NGM_Zone: {
			QColor lineCol;
			if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
				lineCol = QColor("#32c5ea");
			else
				lineCol = QColor("#106d90");

			m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], lineCol);
			// draw outlines of bottom and top polygons first
			size_t vertexCount = m_polygonGeometry.triangulationData().m_vertexes.size();
			unsigned int offset = 2*vertexCount;
			glDrawArrays(GL_LINE_STRIP, (GLint)offset, vertexCount+1);
			// draw outlines of bottom and top polygons first
			glDrawArrays(GL_LINE_STRIP, (GLint)(offset + vertexCount+1), vertexCount+1);

			// now draw the zone wall segments
			offset += 2*vertexCount+2;
			for (unsigned int i=0; i<vertexCount; ++i) {
				glDrawArrays(GL_LINES, (GLint)(2*i + offset), 2);
			}

		} break;

		case NGM_Roof: {
			QColor lineCol;
			if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
				lineCol = QColor("#ff97bc");
			else
				lineCol = QColor("#c31818");

			m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], lineCol);
			// draw outlines of bottom and top polygons first
			size_t vertexCount = m_polygonGeometry.triangulationData().m_vertexes.size() + 1; // mind the additional vertex to close the polygon loop
			GLint lineStartVertex = (GLint)m_lineStartVertex;
			glDrawArrays(GL_LINE_STRIP, lineStartVertex, vertexCount);
			lineStartVertex += vertexCount;
			for (unsigned int j=0; j<m_generatedGeometry.size(); ++j) {
				const std::vector<IBKMK::Vector3D> & vertexes2 = m_generatedGeometry[j].triangulationData().m_vertexes;
				vertexCount = vertexes2.size() + 1;
				glDrawArrays(GL_LINE_STRIP, lineStartVertex, vertexCount);
				lineStartVertex += vertexCount;
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


bool NewGeometryObject::canDrawTransparent() const {
	return !m_indexBufferData.empty();
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

		// set selected plane color (QColor is passed as vec4, so no conversion is needed, here).
		QColor planeCol = QColor(255,0,128,64);
		if (m_newGeometryMode == NGM_Roof)
			planeCol = QColor(255,0,128,64);
		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], planeCol);
		// put OpenGL in offset mode
		glEnable(GL_POLYGON_OFFSET_FILL);
		// offset plane geometry a bit so that the plane is drawn behind the wireframe
		glPolygonOffset(0.1f, 2.0f);
		// now draw the geometry
		glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
		// turn off line offset mode
		glDisable(GL_POLYGON_OFFSET_FILL);

		// release buffers again
		m_vao.release();
	}
}

} // namespace Vic3D
