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

#include "Vic3DOpaqueGeometryObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <VICUS_Project.h>
#include "SVProjectHandler.h"
#include "Vic3DGeometryHelpers.h"

namespace Vic3D {

OpaqueGeometryObject::OpaqueGeometryObject() :
	m_vertexBufferObject(QOpenGLBuffer::VertexBuffer), // VertexBuffer is the default, so default constructor would have been enough
	m_colorBufferObject(QOpenGLBuffer::VertexBuffer),
	m_indexBufferObject(QOpenGLBuffer::IndexBuffer) // make this an Index Buffer
{
}


void OpaqueGeometryObject::create(QOpenGLShaderProgram * shaderProgramm) {
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
	shaderProgramm->enableAttributeArray(VERTEX_ARRAY_INDEX);
	shaderProgramm->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(Vertex));

	// normals
	shaderProgramm->enableAttributeArray(NORMAL_ARRAY_INDEX);
	shaderProgramm->setAttributeBuffer(NORMAL_ARRAY_INDEX, GL_FLOAT, offsetof(Vertex, m_normal), 3 /* vec3 */, sizeof(Vertex));


	m_colorBufferObject.bind(); // now color buffer is active in vao

	// colors
	shaderProgramm->enableAttributeArray(COLOR_ARRAY_INDEX);
	shaderProgramm->setAttributeBuffer(COLOR_ARRAY_INDEX, GL_UNSIGNED_BYTE, 0, 4, 4 /* bytes = sizeof(char) */);

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


void OpaqueGeometryObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_colorBufferObject.destroy();
	m_indexBufferObject.destroy();
}


void OpaqueGeometryObject::updateBuffers() {
	if (m_indexBufferData.empty())
		return;

	// TODO Andreas, if a performance issue arises with very large geometries, use the same memory mapping as
	//               for the color buffer, in order to avoid the overhead for re-allocating when there is no
	//               buffer size change.

	// transfer data stored in m_vertexBufferData
	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex));
	m_vertexBufferObject.release();

	m_indexBufferObject.bind();
	m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GLuint));
	m_indexBufferObject.release();

	// also update the color buffer
	updateColorBuffer();
}


void OpaqueGeometryObject::updateColorBuffer() {
	if (m_colorBufferData.empty())
		return;
	m_colorBufferObject.bind();
	// if color buffer size differs, do a full re-allocate : this expands/shrinks the memory on GPU when
	// new projects are loaded or objects are created/removed
	if ((unsigned int)m_colorBufferObject.size() != m_colorBufferData.size()*sizeof(ColorRGBA))
		m_colorBufferObject.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA) );
	else  {
		auto ptr = m_colorBufferObject.mapRange(0, m_colorBufferData.size() * sizeof(ColorRGBA),
												QOpenGLBuffer::RangeInvalidateBuffer | QOpenGLBuffer::RangeWrite);
		std::memcpy(ptr, m_colorBufferData.data(),  m_colorBufferData.size()*sizeof(ColorRGBA));
		m_colorBufferObject.unmap();
	}
	m_colorBufferObject.release();
}


void OpaqueGeometryObject::renderOpaque() {
	// bind all buffers ("position", "normal" and "color" arrays)
	m_vao.bind();
	// now draw the geometry
	if (m_drawTriangleStrips)
		glDrawElements(GL_TRIANGLE_STRIP, m_transparentStartIndex, GL_UNSIGNED_INT, nullptr);
	else
		glDrawElements(GL_TRIANGLES, m_transparentStartIndex, GL_UNSIGNED_INT, nullptr);
	// release buffers again
	m_vao.release();
}


void OpaqueGeometryObject::renderTransparent() {
	// bind all buffers ("position", "normal" and "color" arrays)
	m_vao.bind();
	// now draw the geometry
	if (m_drawTriangleStrips)
		glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)m_indexBufferData.size() - m_transparentStartIndex,
			GL_UNSIGNED_INT, (const GLvoid*)(sizeof(GLuint) * (unsigned long)m_transparentStartIndex));
	else
		glDrawElements(GL_TRIANGLES, (GLsizei)m_indexBufferData.size() - m_transparentStartIndex,
			GL_UNSIGNED_INT, (const GLvoid*)(sizeof(GLuint) * (unsigned long)m_transparentStartIndex));
	// release buffers again
	m_vao.release();

}


} // namespace Vic3D
