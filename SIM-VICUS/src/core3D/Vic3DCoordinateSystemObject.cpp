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

#include "Vic3DCoordinateSystemObject.h"

#include <QOpenGLShaderProgram>

#include <SV_Conversions.h>

#include <IBKMK_3DCalculations.h>

#include <VICUS_ViewSettings.h>

#include "Vic3DShaderProgram.h"
#include "Vic3DVertex.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DCamera.h"

#include "SVViewStateHandler.h"
#include "SVPropEditGeometry.h"
#include "SVLocalCoordinateView.h"
#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"

namespace Vic3D {

enum ElementIndexes {
	ELEMENT_CENTER_SPHERE_INDEX,
	ELEMENT_AXES_CYLINDER_INDEX,
	ELEMENT_AXES_SPHERE_INDEX,
	ELEMENT_CENTER_SPHERE_TRANSLATION_INDEX,
	ELEMENT_TRANSLATION_INDICATOR_INDEX,
	ELEMENT_ROTATION_INDICATOR_INDEX_X,
	ELEMENT_ROTATION_INDICATOR_INDEX_Y,
	ELEMENT_ROTATION_INDICATOR_INDEX_Z,
	ELEMENT_SCALE_INDICATOR_INDEX_X,
	ELEMENT_SCALE_INDICATOR_INDEX_Y,
	ELEMENT_SCALE_INDICATOR_INDEX_Z,
	ELEMENT_OPAQUE_END_INDEX,
	NUM_ELEMENT_INDEX
};


enum VertexIndexes {
	VERTEX_AXIS_LINES_START,
	VERTEX_ORBIT_X_LINE_START,
	VERTEX_ORBIT_Y_LINE_START,
	VERTEX_ORBIT_Z_LINE_START,
	NUM_VERTEX_INDEXES
};

const unsigned int N_ORBIT_LINE_SEGMENTS = 32;

// The length of the axis
const double AXIS_LENGTH = 2;
// Size of the center sphere in translation mode
const double TRANSLATION_CENTER_SPHERE_FACTOR = 0.3;
// Size of the rotation axis marker spheres in rotation mode
const double ROTATION_MARKER_SPHERE_FACTOR = 0.15;

CoordinateSystemObject::CoordinateSystemObject() {
	// make us known to the world
	SVViewStateHandler::instance().m_coordinateSystemObject = this;

	// Initialize transform
	m_transform.translate(QVector3D(0,0,0));
	updateInverse();
}


void CoordinateSystemObject::create(ShaderProgram * shaderProgram) {
	m_shaderProgram = shaderProgram;

	if (m_vao.isCreated())
		return;


	// *** create buffers on GPU memory ***

	// create a new buffer for the vertices and colors, separate buffers because we will modify colors way more often than geometry
	m_vertexBufferObject.create();
	m_vertexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw); // usage pattern will be used when tranferring data to GPU

	m_colorBufferObject.create();
	m_colorBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);

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
#define NORMAL_ARRAY_INDEX 1
#define COLOR_ARRAY_INDEX 2

	m_vertexBufferObject.bind(); // this registers this buffer data object in the currently bound vao; in subsequent
				  // calls to shaderProgramm->setAttributeBuffer() the buffer object is associated with the
				  // respective attribute array that's fed into the shader. When the vao is later bound before
				  // rendering, this association is remembered so that the vertex fetch stage pulls data from
				  // this vbo

	// coordinates
	QOpenGLShaderProgram * shaderProg = shaderProgram->shaderProgram();
	shaderProg->enableAttributeArray(VERTEX_ARRAY_INDEX);
	shaderProg->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(Vertex));

	// normals
	shaderProg->enableAttributeArray(NORMAL_ARRAY_INDEX);
	shaderProg->setAttributeBuffer(NORMAL_ARRAY_INDEX, GL_FLOAT, offsetof(Vertex, m_normal), 3 /* vec3 */, sizeof(Vertex));


	m_colorBufferObject.bind(); // now color buffer is active in vao

	// colors
	shaderProg->enableAttributeArray(COLOR_ARRAY_INDEX);
	shaderProg->setAttributeBuffer(COLOR_ARRAY_INDEX, GL_UNSIGNED_BYTE, 0, 4, 4 /* bytes = sizeof(char) */);

	// Release (unbind) all

	// Mind: you can release the buffer data objects (vbo and vboColors) before or after releasing vao. It does not
	//       matter, because the buffers are associated already with the attribute arrays.
	//       However, YOU MUST NOT release the index buffer (ebo) before releasing the vao, since this would remove
	//       the index buffer association with the vao and when binding the vao before rendering, the element buffer
	//       would not be known and a call to glDrawElements() crashes!
	m_vao.release();

	m_vertexBufferObject.release();
	m_colorBufferObject.release();
	m_indexBufferObject.release();

	m_coordinateSystemSizeFactor = 0; // set scale to invalid size so that next call to updateCoordinateSystemSize() definitely
	// updates the geometry buffers.
	updateCoordinateSystemSize();
}


void CoordinateSystemObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_colorBufferObject.destroy();
	m_indexBufferObject.destroy();
}


#define DRAW_ELEMENT(object)  glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)(m_objectStartIndexes[object+1] - m_objectStartIndexes[object]), \
	GL_UNSIGNED_INT, (const GLvoid*)(sizeof(GLuint) * m_objectStartIndexes[object]))

#define DRAW_ELEMENT_RANGE(firstObject, lastObject)  glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)(m_objectStartIndexes[lastObject+1] - m_objectStartIndexes[firstObject]), \
	GL_UNSIGNED_INT, (const GLvoid*)(sizeof(GLuint) * m_objectStartIndexes[firstObject]))

void CoordinateSystemObject::renderOpaque() {
	// bind all buffers ("position", "normal" and "color" arrays)
	m_vao.bind();
	// draw with face culling on
	glEnable(GL_CULL_FACE);
	// set transformation matrix
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[4], m_transform.toMatrix());

	// *** opaque geometry ***

	//	glDrawElements(GL_TRIANGLE_STRIP, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr /* (GLvoid*)(0) */);

	// draw opaque center sphere for all but translation mode
	if ((m_geometryTransformMode & TM_Translate) == 0)
		DRAW_ELEMENT(ELEMENT_CENTER_SPHERE_INDEX);

	// axis cylinders are always drawn
	DRAW_ELEMENT(ELEMENT_AXES_CYLINDER_INDEX);

	// we have different options to render
	switch (m_geometryTransformMode) {
		case TM_None :			DRAW_ELEMENT(ELEMENT_AXES_SPHERE_INDEX); break;

		case TM_RotateX		:
		case TM_RotateY		:
		case TM_RotateZ		:
			DRAW_ELEMENT(ELEMENT_AXES_SPHERE_INDEX);
			break;

		case TM_RotateMask	:
			DRAW_ELEMENT(ELEMENT_ROTATION_INDICATOR_INDEX_X);
			DRAW_ELEMENT(ELEMENT_ROTATION_INDICATOR_INDEX_Y);
			DRAW_ELEMENT(ELEMENT_ROTATION_INDICATOR_INDEX_Z);
		break;

		case TM_ScaleX :		DRAW_ELEMENT(ELEMENT_SCALE_INDICATOR_INDEX_X); break;
		case TM_ScaleY :		DRAW_ELEMENT(ELEMENT_SCALE_INDICATOR_INDEX_Y); break;
		case TM_ScaleZ :		DRAW_ELEMENT(ELEMENT_SCALE_INDICATOR_INDEX_Z); break;
		case TM_ScaleMask :
			DRAW_ELEMENT(ELEMENT_SCALE_INDICATOR_INDEX_X);
			DRAW_ELEMENT(ELEMENT_SCALE_INDICATOR_INDEX_Y);
			DRAW_ELEMENT(ELEMENT_SCALE_INDICATOR_INDEX_Z);
		break;

		case TM_Translate :
			DRAW_ELEMENT(ELEMENT_CENTER_SPHERE_TRANSLATION_INDEX);
			DRAW_ELEMENT(ELEMENT_AXES_SPHERE_INDEX);
		break;
	}


	// *** lines ***

	// render lock lines
	switch (SVViewStateHandler::instance().viewState().m_locks) {
		case SVViewState::L_LocalX:			glDrawArrays(GL_LINES, m_axisLinesVertexIndex[VERTEX_AXIS_LINES_START]  , 2); break;
		case SVViewState::L_LocalY:			glDrawArrays(GL_LINES, m_axisLinesVertexIndex[VERTEX_AXIS_LINES_START]+2, 2); break;
		case SVViewState::L_LocalZ:			glDrawArrays(GL_LINES, m_axisLinesVertexIndex[VERTEX_AXIS_LINES_START]+4, 2); break;
		case SVViewState::NUM_L:			break;
	}

	if (m_geometryTransformMode & TM_RotateX)
		glDrawArrays(GL_LINE_STRIP, m_axisLinesVertexIndex[VERTEX_ORBIT_X_LINE_START], N_ORBIT_LINE_SEGMENTS + 1);

	if (m_geometryTransformMode & TM_RotateY)
		glDrawArrays(GL_LINE_STRIP, m_axisLinesVertexIndex[VERTEX_ORBIT_Y_LINE_START], N_ORBIT_LINE_SEGMENTS + 1);

	if (m_geometryTransformMode & TM_RotateZ)
		glDrawArrays(GL_LINE_STRIP, m_axisLinesVertexIndex[VERTEX_ORBIT_Z_LINE_START], N_ORBIT_LINE_SEGMENTS + 1);

	// release buffers again
	m_vao.release();
}


void CoordinateSystemObject::renderTransparent() {

}


void CoordinateSystemObject::setTranslation(const QVector3D & translation) {
	m_transform.setTranslation(translation);
	// tell the property widget for editing geometry our new position/rotation
	if (SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr)
		SVViewStateHandler::instance().m_propEditGeometryWidget->setCoordinates(m_transform);
	SVViewStateHandler::instance().m_localCoordinateViewWidget->setCoordinates(m_transform);
	updateInverse();
}


void CoordinateSystemObject::setRotation(const QQuaternion & rotMatrix) {
	m_transform.setRotation(rotMatrix);
	updateInverse();
//	qDebug() << localXAxis() << localYAxis() << localZAxis();
}


void CoordinateSystemObject::setTransform(const Transform3D & transform) {
	m_transform = transform;
	updateInverse();
}


bool CoordinateSystemObject::pick(const IBKMK::Vector3D & nearPoint, const IBKMK::Vector3D & direction,
								  PickObject::PickResult & r) const
{
	double lineFactor;
	IBKMK::Vector3D closestPoint;
	IBKMK::Vector3D lotPoint;
	switch (m_geometryTransformMode) {
		case Vic3D::CoordinateSystemObject::TM_Translate : {
			bool hit = IBKMK::lineShereIntersection(nearPoint, direction,  // line of sight
													QVector2IBKVector(translation()), TRANSLATION_CENTER_SPHERE_FACTOR, // sphere
													lineFactor, // lineFactor (lineFactor*d = distance to intersection point with sphere)
													lotPoint // closest point on line-of-sight to sphere center
													);
			// check distance against radius of sphere
			if (hit) {
				r.m_resultType = PickObject::RT_CoordinateSystemCenter;
				r.m_depth = lineFactor;
				r.m_pickPoint = nearPoint + lineFactor*direction;
				r.m_objectID = 0; // not needed, since type is already expressive enough
				return true;
			}
		} break;

		case Vic3D::CoordinateSystemObject::TM_RotateMask : {
			// x-axis
			IBKMK::Vector3D sphereCenter = QVector2IBKVector( m_transform.toMatrix() * QVector3D(AXIS_LENGTH, 0, 0) );
			bool hit = IBKMK::lineShereIntersection(nearPoint, direction,  // line of sight
													sphereCenter, ROTATION_MARKER_SPHERE_FACTOR, // sphere
													lineFactor, // lineFactor (lineFactor*d = distance to intersection point with sphere)
													lotPoint // closest point on line-of-sight to sphere center
													);
			// check distance against radius of sphere
			if (hit) {
				r.m_resultType = PickObject::RT_AxisEndMarker;
				r.m_depth = lineFactor;
				r.m_pickPoint = nearPoint + lineFactor*direction;
				r.m_objectID = 0; // indicates x axis, i.e. rotation around z
				return true;
			}
			// y-axis
			sphereCenter = QVector2IBKVector( m_transform.toMatrix() * QVector3D(0, AXIS_LENGTH, 0) );
			hit = IBKMK::lineShereIntersection(nearPoint, direction,  // line of sight
													sphereCenter, ROTATION_MARKER_SPHERE_FACTOR, // sphere
													lineFactor, // lineFactor (lineFactor*d = distance to intersection point with sphere)
													lotPoint // closest point on line-of-sight to sphere center
													);
			// check distance against radius of sphere
			if (hit) {
				r.m_resultType = PickObject::RT_AxisEndMarker;
				r.m_depth = lineFactor;
				r.m_pickPoint = nearPoint + lineFactor*direction;
				r.m_objectID = 1; // indicates y axis
				return true;
			}
			// z-axis
			sphereCenter = QVector2IBKVector( m_transform.toMatrix() * QVector3D(0, 0, AXIS_LENGTH) );
			hit = IBKMK::lineShereIntersection(nearPoint, direction,  // line of sight
													sphereCenter, ROTATION_MARKER_SPHERE_FACTOR, // sphere
													lineFactor, // lineFactor (lineFactor*d = distance to intersection point with sphere)
													lotPoint // closest point on line-of-sight to sphere center
													);
			// check distance against radius of sphere
			if (hit) {
				r.m_resultType = PickObject::RT_AxisEndMarker;
				r.m_depth = lineFactor;
				r.m_pickPoint = nearPoint + lineFactor*direction;
				r.m_objectID = 2; // indicates z axis
				return true;
			}
		} break;

		case Vic3D::CoordinateSystemObject::TM_ScaleMask : {
			// x-axis
			IBKMK::Vector3D sphereCenter = QVector2IBKVector( m_transform.toMatrix() * QVector3D(AXIS_LENGTH, 0, 0) );
			bool hit = IBKMK::lineShereIntersection(nearPoint, direction,  // line of sight
													sphereCenter, ROTATION_MARKER_SPHERE_FACTOR, // sphere
													lineFactor, // lineFactor (lineFactor*d = distance to intersection point with sphere)
													lotPoint // closest point on line-of-sight to sphere center
													);
			// check distance against radius of sphere
			if (hit) {
				r.m_resultType = PickObject::RT_AxisEndMarker;
				r.m_depth = lineFactor;
				r.m_pickPoint = nearPoint + lineFactor*direction;
				r.m_objectID = 0; // indicates x axis, i.e. rotation around z
				return true;
			}
			// y-axis
			sphereCenter = QVector2IBKVector( m_transform.toMatrix() * QVector3D(0, AXIS_LENGTH, 0) );
			hit = IBKMK::lineShereIntersection(nearPoint, direction,  // line of sight
													sphereCenter, ROTATION_MARKER_SPHERE_FACTOR, // sphere
													lineFactor, // lineFactor (lineFactor*d = distance to intersection point with sphere)
													lotPoint // closest point on line-of-sight to sphere center
													);
			// check distance against radius of sphere
			if (hit) {
				r.m_resultType = PickObject::RT_AxisEndMarker;
				r.m_depth = lineFactor;
				r.m_pickPoint = nearPoint + lineFactor*direction;
				r.m_objectID = 1; // indicates y axis
				return true;
			}
			// z-axis
			sphereCenter = QVector2IBKVector( m_transform.toMatrix() * QVector3D(0, 0, AXIS_LENGTH) );
			hit = IBKMK::lineShereIntersection(nearPoint, direction,  // line of sight
													sphereCenter, ROTATION_MARKER_SPHERE_FACTOR, // sphere
													lineFactor, // lineFactor (lineFactor*d = distance to intersection point with sphere)
													lotPoint // closest point on line-of-sight to sphere center
													);
			// check distance against radius of sphere
			if (hit) {
				r.m_resultType = PickObject::RT_AxisEndMarker;
				r.m_depth = lineFactor;
				r.m_pickPoint = nearPoint + lineFactor*direction;
				r.m_objectID = 2; // indicates z axis
				return true;
			}
		} break;

		default : return false; // in all other modes we have nothing to pick
	}
	return false;
}


void CoordinateSystemObject::updateCoordinateSystemSize() {
	// compute scale factor
	double lastScale = m_coordinateSystemSizeFactor;
	m_coordinateSystemSizeFactor = 1;
	if (SVProjectHandler::instance().isValid()) {
		IBKMK::Vector3D cameraPos = SVProjectHandler::instance().viewSettings().m_cameraTranslation;
		IBKMK::Vector3D distance = QVector2IBKVector(m_transform.translation()) - cameraPos;
		double dist = distance.magnitude();
//		qDebug() << "Camera-LCS-Distance =" << dist;
		if (dist < 20) {
			m_coordinateSystemSizeFactor = dist/20;
		}
	}

	if (lastScale == m_coordinateSystemSizeFactor)
		return; // nothing to do

	// *** generate geometry ***

	// Buffer memory layout, both in vertex buffer and in index buffer

	// m_objectIndexStart vector holds starting element indexes for individual objects to be rendered with drawElements()
	// m_axisLinesVertexIndex vector holds starting vertex indexes for lines to be rendered with glDrawArrays()

	// 0 - center sphere. opaque (for regular coordinate system, and for rotation/scaling)
	// 1 - x-axis cylinder (for regular coordinate system)
	//   - y-axis cylinder (for regular coordinate system)
	//   - z-axis cylinder (for regular coordinate system)
	// 2 - x axis sphere (opaque) (for regular coordinate system)
	//   - y axis sphere (opaque) (for regular coordinate system)
	//   - z axis sphere (opaque) (for regular coordinate system)
	// 3 - center sphere. large (for translating)
	// 4 - translation x axis cube (opaque)
	// 5 - translation y axis cube (opaque)
	// 6 - translation z axis cube (opaque)
	// 7 - rotation x axis sphere (opaque)
	// 8 - rotation y axis sphere (opaque)
	// 9 - rotation z axis sphere (opaque)
	// 10 - scale x axis cylinder (opaque)
	// 11 - scale y axis cylinder  (opaque)
	// 12 - scale z axis cylinder  (opaque)

	// Afterwards only vertexes follow, without corresponding indexes:
	//
	// - 3 * 2  vertexes for the (infinite) axis lines
	// - 3 * n  vertexes for rotation orbits

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	m_objectStartIndexes.resize(NUM_ELEMENT_INDEX);
	m_axisLinesVertexIndex.resize(NUM_VERTEX_INDEXES);



	// add regular coordinate system

	// TODO : Styling - coordinate system colors should by style-dependent

	m_objectStartIndexes[ELEMENT_CENTER_SPHERE_INDEX] = currentElementIndex;

	addSphere(IBKMK::Vector3D(0,0,0), m_orbColor, 0.15*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	m_objectStartIndexes[ELEMENT_AXES_CYLINDER_INDEX] = currentElementIndex;

	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(AXIS_LENGTH,0,0), QColor(Qt::red), 0.02*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,AXIS_LENGTH,0), QColor(0,196,0), 0.02*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,0,AXIS_LENGTH), QColor(32,32,255), 0.02*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	m_objectStartIndexes[ELEMENT_AXES_SPHERE_INDEX] = currentElementIndex;

	addSphere(IBKMK::Vector3D(AXIS_LENGTH,0,0), QColor(255, 245, 152), 0.1*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addSphere(IBKMK::Vector3D(0,AXIS_LENGTH,0), QColor(255, 245, 152), 0.1*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addSphere(IBKMK::Vector3D(0,0,AXIS_LENGTH), QColor(255, 245, 152), 0.1*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	m_objectStartIndexes[ELEMENT_CENTER_SPHERE_TRANSLATION_INDEX] = currentElementIndex;

	QColor centerTransparentColor("gold");
	addSphere(IBKMK::Vector3D(0,0,0), centerTransparentColor, TRANSLATION_CENTER_SPHERE_FACTOR*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	m_objectStartIndexes[ELEMENT_TRANSLATION_INDICATOR_INDEX] = currentElementIndex;

#if 0
	addCube(IBKMK::Vector3D(2,0,0), QColor(255, 245, 152), 0.1*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addCube(IBKMK::Vector3D(0,2,0), QColor(255, 245, 152), 0.1*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addCube(IBKMK::Vector3D(0,0,2), QColor(255, 245, 152), 0.1*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
#endif

	// rotation indicators

	// x-axis: draw 4 green bullets in yz plane

	m_objectStartIndexes[ELEMENT_ROTATION_INDICATOR_INDEX_X] = currentElementIndex;
	// x-axis sphere is for rotation around z
	addSphere(IBKMK::Vector3D(AXIS_LENGTH,0,0), QColor(32,32,255), ROTATION_MARKER_SPHERE_FACTOR*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
//	for (int i=0; i<8; ++i) {
//		QVector3D pos(0,AXIS_LENGTH,0);
//		QQuaternion rot = QQuaternion::fromAxisAndAngle(QVector3D(1,0,0), 22.5 + i*45);
//		pos = rot.rotatedVector(pos);
//		addSphere(QVector2IBKVector(pos), QColor(Qt::red), ROTATION_MARKER_SPHERE_FACTOR*sizeFactor, currentVertexIndex, currentElementIndex,
//				  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
//	}

	m_objectStartIndexes[ELEMENT_ROTATION_INDICATOR_INDEX_Y] = currentElementIndex;
	// y-axis sphere is for rotation around x
	addSphere(IBKMK::Vector3D(0, AXIS_LENGTH,0), QColor(Qt::red), ROTATION_MARKER_SPHERE_FACTOR*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
//	for (int i=0; i<8; ++i) {
//		QVector3D pos(AXIS_LENGTH,0,0);
//		QQuaternion rot = QQuaternion::fromAxisAndAngle(QVector3D(0,1,0), 22.5 + i*45);
//		pos = rot.rotatedVector(pos);
//	}

	m_objectStartIndexes[ELEMENT_ROTATION_INDICATOR_INDEX_Z] = currentElementIndex;
	// z-axis sphere is for rotation around y
	addSphere(IBKMK::Vector3D(0,0,AXIS_LENGTH), QColor(Qt::green), ROTATION_MARKER_SPHERE_FACTOR*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
//	for (int i=0; i<8; ++i) {
//		QVector3D pos(AXIS_LENGTH,0,0);
//		QQuaternion rot = QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), 22.5 + i*45);
//		pos = rot.rotatedVector(pos);
//	}

	// scale indicators

	m_objectStartIndexes[ELEMENT_SCALE_INDICATOR_INDEX_X] = currentElementIndex;
	addCylinder(IBKMK::Vector3D(AXIS_LENGTH,0,0), IBKMK::Vector3D(AXIS_LENGTH + 0.3,0,0), QColor(Qt::red), 0.1*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData, true);

	m_objectStartIndexes[ELEMENT_SCALE_INDICATOR_INDEX_Y] = currentElementIndex;
	addCylinder(IBKMK::Vector3D(0,AXIS_LENGTH,0), IBKMK::Vector3D(0,AXIS_LENGTH + 0.3,0), QColor(0,196,0), 0.1*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData, true);

	m_objectStartIndexes[ELEMENT_SCALE_INDICATOR_INDEX_Z] = currentElementIndex;
	addCylinder(IBKMK::Vector3D(0,0,AXIS_LENGTH), IBKMK::Vector3D(0,0,AXIS_LENGTH + 0.3), QColor(32,32,255), 0.1*m_coordinateSystemSizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData, true);

	// the last index is just needed to know until which element index we need to draw
	m_objectStartIndexes[ELEMENT_OPAQUE_END_INDEX] = currentElementIndex;

	// add vertexes for x, y, and z axis lines
	m_axisLinesVertexIndex[VERTEX_AXIS_LINES_START] = m_vertexBufferData.size();
	m_vertexBufferData.push_back(Vertex(QVector3D(-100000,0,0),QVector3D(0,0,0)));
	m_vertexBufferData.push_back(Vertex(QVector3D(+100000,0,0),QVector3D(0,0,0)));
	m_vertexBufferData.push_back(Vertex(QVector3D(0,-100000,0),QVector3D(0,0,0)));
	m_vertexBufferData.push_back(Vertex(QVector3D(0,+100000,0),QVector3D(0,0,0)));
	m_vertexBufferData.push_back(Vertex(QVector3D(0,0,-100000),QVector3D(0,0,0)));
	m_vertexBufferData.push_back(Vertex(QVector3D(0,0,+100000),QVector3D(0,0,0)));
	m_colorBufferData.push_back(ColorRGBA(QColor(Qt::red)));
	m_colorBufferData.push_back(ColorRGBA(QColor(Qt::red)));
	m_colorBufferData.push_back(ColorRGBA(QColor(0,196,0)));
	m_colorBufferData.push_back(ColorRGBA(QColor(0,196,0)));
	m_colorBufferData.push_back(ColorRGBA(QColor(32,32,255)));
	m_colorBufferData.push_back(ColorRGBA(QColor(32,32,255)));

	// generate line segments for orbits

	m_axisLinesVertexIndex[VERTEX_ORBIT_X_LINE_START] = m_vertexBufferData.size();

	// create a circle with line segments
	const double RADIUS = 2;
#define PI_CONST 3.14159265
	for (unsigned int i=0; i<=N_ORBIT_LINE_SEGMENTS; ++i) {
		double angle = 2*PI_CONST*((i+1) % N_ORBIT_LINE_SEGMENTS)/N_ORBIT_LINE_SEGMENTS;
		double z = std::sin(angle)*RADIUS;
		double y = std::cos(angle)*RADIUS;
		m_vertexBufferData.push_back( Vertex(QVector3D(0,float(y),float(z)), QVector3D(0,float(y),float(z)) ) );
		m_colorBufferData.push_back(ColorRGBA(QColor(Qt::red)));
	}

	m_axisLinesVertexIndex[VERTEX_ORBIT_Y_LINE_START] = m_vertexBufferData.size();

	// create a circle with line segments
	for (unsigned int i=0; i<=N_ORBIT_LINE_SEGMENTS; ++i) {
		double angle = 2*PI_CONST*((i+1) % N_ORBIT_LINE_SEGMENTS)/N_ORBIT_LINE_SEGMENTS;
		double x = std::sin(angle)*RADIUS;
		double z = std::cos(angle)*RADIUS;
		m_vertexBufferData.push_back( Vertex(QVector3D(float(x),0,float(z)), QVector3D(float(x),0,float(z))) );
		m_colorBufferData.push_back(ColorRGBA(QColor(Qt::green)));
	}

	m_axisLinesVertexIndex[VERTEX_ORBIT_Z_LINE_START] = m_vertexBufferData.size();

	// create a circle with line segments
	for (unsigned int i=0; i<=N_ORBIT_LINE_SEGMENTS; ++i) {
		double angle = 2*PI_CONST*((i+1) % N_ORBIT_LINE_SEGMENTS)/N_ORBIT_LINE_SEGMENTS;
		double x = std::sin(angle)*RADIUS;
		double y = std::cos(angle)*RADIUS;
		m_vertexBufferData.push_back( Vertex(QVector3D(float(x),float(y),0), QVector3D(float(x),float(y),0)) );
		m_colorBufferData.push_back(ColorRGBA(QColor(Qt::blue)));
	}


	// transfer geometry to GPU

	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex));
	m_vertexBufferObject.release();

	m_colorBufferObject.bind();
	m_colorBufferObject.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA) );
	m_colorBufferObject.release();

	m_indexBufferObject.bind();
	m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GLuint));
	m_indexBufferObject.release();

#if 0
	// now create the coordinate lines
	m_lineVbo.create();
	m_lineVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

	m_lineVao.create();
	m_lineVao.bind(); // now the VAO is active and remembers states modified in following calls

	m_lineVbo.bind();

	// coordinates
	QOpenGLShaderProgram * shaderProg = shaderProgram->shaderProgram();
	shaderProg->enableAttributeArray(VERTEX_ARRAY_INDEX);
	shaderProg->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(Vertex));
#endif
}


void CoordinateSystemObject::updateInverse() {
	m_inverseMatrix.setToIdentity();
	m_inverseMatrix.rotate(m_transform.rotation().conjugated());
	m_inverseMatrix.translate(-m_transform.translation());
}

void CoordinateSystemObject::setOrbColor(const QColor &newOrbColor) {
	m_orbColor = newOrbColor;
	updateCoordinateSystemSize();
}


} // namespace Vic3D
