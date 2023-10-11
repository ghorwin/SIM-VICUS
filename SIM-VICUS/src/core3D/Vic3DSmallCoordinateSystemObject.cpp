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

#include "Vic3DSmallCoordinateSystemObject.h"

#include <QOpenGLShaderProgram>

#include "IBKMK_3DCalculations.h"
#include "Vic3DShaderProgram.h"
#include "Vic3DVertex.h"
#include "Vic3DGeometryHelpers.h"
#include <SVConversions.h>

#include "SVViewStateHandler.h"
#include "SVPropEditGeometry.h"
#include "SVViewStateHandler.h"
#include "qpainterpath.h"

namespace Vic3D {

SmallCoordinateSystemObject::SmallCoordinateSystemObject() {
	// Initialize transform
	m_transform.translate(QVector3D(0,0,0));
	// world2view is identity matrix at first
	m_worldToSmallView.setToIdentity();
}


void SmallCoordinateSystemObject::create(ShaderProgram * opaquePhongShaderProgram, ShaderProgram * transparentShaderProgram) {
	m_opaquePhongShaderProgram = opaquePhongShaderProgram;
	m_transparentShaderProgram = transparentShaderProgram;

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
	QOpenGLShaderProgram * shaderProg = opaquePhongShaderProgram->shaderProgram();
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


	// *** generate geometry ***

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	double sizeFactor = 5;

	// Note: the following geometrical objects are added a triangle strips with restart-indexes in between

	addSphere(IBKMK::Vector3D(0,0,0), QColor("burlywood"), 0.05*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(2,0,0), QColor(Qt::red), 0.02*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,2,0), QColor(0,196,0), 0.02*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,0,2), QColor(32,32,255), 0.02*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addSphere(IBKMK::Vector3D(2,0,0), QColor(255, 245, 152), 0.03*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addSphere(IBKMK::Vector3D(0,2,0), QColor(255, 245, 152), 0.03*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addSphere(IBKMK::Vector3D(0,0,2), QColor(255, 245, 152), 0.03*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	// add a plane for the background
	m_planeStartIndex = currentElementIndex; // number of indexes to use in the TRIANGLE_STRIP draw call
	m_planeStartVertex = currentVertexIndex; // first index of the two triangles of the TRIANGLES draw call

	VICUS::RotationMatrix rotMat;
	rotMat.setQuaternion(QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), 0));

	QFont font("Arial");
	font.setBold(true);
	font.setPointSize(64);
	addText("N", font, Qt::AlignHCenter, 0, rotMat, IBKMK::Vector3D(0, 0, 0), IBKMK::Vector2D(-0.3, 2.5), 1, 0.0001,
			Qt::green, currentVertexIndex, currentElementIndex,	m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addText("E", font, Qt::AlignHCenter, 0, rotMat, IBKMK::Vector3D(0, 0, 0), IBKMK::Vector2D(2.4, -0.3), 1, 0.0001,
			Qt::red, currentVertexIndex, currentElementIndex, m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	//	// we now manually add the vertexes for two triangles (forming a plane) to the vertex buffer. These will be drawn with
	// draw elements
//	m_vertexBufferData.resize(m_vertexBufferData.size()+6);
//	m_colorBufferData.resize(m_colorBufferData.size()+6);
//	ColorRGBA planeCol(QColor(255,32,32,180) );

//	for (unsigned int i=0; i<6; ++i) {
//		m_colorBufferData[m_planeStartVertex+i] = planeCol;
//	}

//	// add vertexes for both triangles in anti-clock-wise winding order so that normal vector points towards view
//	m_vertexBufferData[m_planeStartVertex+0].m_coords = QVector3D(-1000,-1000,0);
//	m_vertexBufferData[m_planeStartVertex+1].m_coords = QVector3D( 1000,-1000,0);
//	m_vertexBufferData[m_planeStartVertex+2].m_coords = QVector3D(-1000, 1000,0);
//	m_vertexBufferData[m_planeStartVertex+3].m_coords = QVector3D( 1000,-1000,0); // second triangle
//	m_vertexBufferData[m_planeStartVertex+4].m_coords = QVector3D( 1000, 1000,0);
//	m_vertexBufferData[m_planeStartVertex+5].m_coords = QVector3D(-1000, 1000,0);

	// Note: the indexes added for the plane in the index array will not be used

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
}


void SmallCoordinateSystemObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_colorBufferObject.destroy();
	m_indexBufferObject.destroy();
}


void SmallCoordinateSystemObject::render() {
	// bind all buffers ("position", "normal" and "color" arrays)
	m_vao.bind();

	// disable depth test for rendering of transparent back plane
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	m_transparentShaderProgram->bind();

	// set transformation matrix to unity-matrix, so that the backplane fills the entire screen
	m_transparentShaderProgram->shaderProgram()->setUniformValue(m_transparentShaderProgram->m_uniformIDs[0], QMatrix4x4());
	//glDrawArrays(GL_TRIANGLES, (GLint)m_vertexBufferData.size() - 6, 6);

	m_transparentShaderProgram->release();

	glDisable(GL_BLEND);

	// clear only depth buffer, so that we can paint on top of everything else
	glClear(GL_DEPTH_BUFFER_BIT);
	// draw with face culling on
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// we have an existing color buffer and we paint on top of it
	// also, we have an empty depth buffer

	m_opaquePhongShaderProgram->bind();

	// set world 2 (small) view transformation matrix
	m_opaquePhongShaderProgram->shaderProgram()->setUniformValue(m_opaquePhongShaderProgram->m_uniformIDs[0], m_worldToSmallView);

	QVector3D viewPos = m_smallViewCamera.translation();

	// set light position
	m_opaquePhongShaderProgram->shaderProgram()->setUniformValue(m_opaquePhongShaderProgram->m_uniformIDs[1], viewPos); // lightpos
	m_opaquePhongShaderProgram->shaderProgram()->setUniformValue(m_opaquePhongShaderProgram->m_uniformIDs[2], QtExt::QVector3DFromQColor(Qt::white));
	m_opaquePhongShaderProgram->shaderProgram()->setUniformValue(m_opaquePhongShaderProgram->m_uniformIDs[3], viewPos); // viewpos

	// set transformation matrix
	m_opaquePhongShaderProgram->shaderProgram()->setUniformValue(m_opaquePhongShaderProgram->m_uniformIDs[4], m_transform.toMatrix());

	// now draw the geometry
	glDrawElements(GL_TRIANGLE_STRIP, m_planeStartIndex, GL_UNSIGNED_INT, nullptr);
	glDrawElements(GL_TRIANGLES, m_indexBufferData.size() - m_planeStartIndex, GL_UNSIGNED_INT,
				   (const GLvoid*)(sizeof(GLuint) * (unsigned long)m_planeStartIndex));

	m_opaquePhongShaderProgram->release();

	// release buffers again
	m_vao.release();
}



void SmallCoordinateSystemObject::setRotation(const QQuaternion & rotMatrix) {
	m_transform.setRotation(rotMatrix);
}


} // namespace Vic3D
