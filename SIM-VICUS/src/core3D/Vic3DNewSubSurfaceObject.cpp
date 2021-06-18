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

#include "Vic3DNewSubSurfaceObject.h"

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

NewSubSurfaceObject::NewSubSurfaceObject() :
	m_vertexBufferObject(QOpenGLBuffer::VertexBuffer), // VertexBuffer is the default, so default constructor would have been enough
	m_indexBufferObject(QOpenGLBuffer::IndexBuffer) // make this an Index Buffer
{
	// make us known to the world
	SVViewStateHandler::instance().m_newSubSurfaceObject = this;
}


void NewSubSurfaceObject::create(QOpenGLShaderProgram * shaderProgram) {
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
	shaderProgram->enableAttributeArray(VERTEX_ARRAY_INDEX);
	shaderProgram->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(Vertex));

	// normals
	shaderProgram->enableAttributeArray(NORMAL_ARRAY_INDEX);
	shaderProgram->setAttributeBuffer(NORMAL_ARRAY_INDEX, GL_FLOAT, offsetof(Vertex, m_normal), 3 /* vec3 */, sizeof(Vertex));


	m_colorBufferObject.bind(); // now color buffer is active in vao

	// colors
	shaderProgram->enableAttributeArray(COLOR_ARRAY_INDEX);
	shaderProgram->setAttributeBuffer(COLOR_ARRAY_INDEX, GL_UNSIGNED_BYTE, 0, 4, 4 /* bytes = sizeof(char) */);

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
}


void NewSubSurfaceObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_indexBufferObject.destroy();
}


void NewSubSurfaceObject::generateSubSurfaces(const std::vector<const VICUS::Surface*> & sel, const WindowComputationData & inputData) {
	// populate PlaneGeometry-objects

	m_surfaceGeometries.clear();

	// TODO : Dirk

	updateBuffers();
}

void NewSubSurfaceObject::updateBuffers() {
	if (m_indexBufferData.empty())
		return;

	// transfer data stored in m_vertexBufferData
	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex));
	m_vertexBufferObject.release();

	m_indexBufferObject.bind();
	m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GLuint));
	m_indexBufferObject.release();

	// also update the color buffer
	m_colorBufferObject.bind();
	m_colorBufferObject.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA) );
	m_colorBufferObject.release();
}


void NewSubSurfaceObject::renderOpaque() {
	if (m_vertexBufferData.empty())
		return;


	m_vao.bind();

	// first render opaque polygon

	// here we render the lines around the sub-surfaces; invalid surfaces get a red line
	glDrawElements(GL_LINES, m_lineIndex, GL_UNSIGNED_INT,
				   (const GLvoid*)(sizeof(GLuint) * (unsigned long)((GLsizei)m_indexBufferData.size() - m_lineIndex)) );
	// release buffers again
	m_vao.release();
}


void NewSubSurfaceObject::renderTransparent() {
	// we expect:
	//   glDepthMask(GL_FALSE);
	//   shader program has already transform uniform set
	//   glDisable(GL_CULL_FACE);

	// the render code below is the same for all geometry types, since only the index buffer is used
	if (!m_indexBufferData.empty()) {
		// bind all buffers ("position", "normal" and "color" arrays)
		m_vao.bind();

		// put OpenGL in offset mode
		glEnable(GL_POLYGON_OFFSET_FILL);
		// offset plane geometry a bit so that our transparent planes are always drawn in front of opaque planes
		glPolygonOffset(-0.1f, -2.0f);
		// now draw the geometry
		glDrawElements(GL_TRIANGLES, 0,
			GL_UNSIGNED_INT, (const GLvoid*)(sizeof(GLuint) * (unsigned long)m_lineIndex));
		// turn off line offset mode
		glDisable(GL_POLYGON_OFFSET_FILL);

		// release buffers again
		m_vao.release();
	}
}

} // namespace Vic3D
