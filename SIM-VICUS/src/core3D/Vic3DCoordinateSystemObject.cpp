/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#include "Vic3DCoordinateSystemObject.h"

#include <QOpenGLShaderProgram>

#include "Vic3DShaderProgram.h"
#include "Vic3DVertex.h"
#include "Vic3DGeometryHelpers.h"
#include <VICUS_Conversions.h>

namespace Vic3D {

void CoordinateSystemObject::create(ShaderProgram * shaderProgram) {
	m_shader = shaderProgram;

	// Initialize transform
	m_transform.translate(QVector3D(1,1,1));

	// Create Vertex Array Object and buffers if not done, yet
	if (!m_vao.isCreated()) {
		// Create Vertex Buffer Object (to store stuff in GPU memory)
		m_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		m_vbo.create();
		m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

		// create a new buffer for the indexes
		m_indexBufferObject = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); // Mind: use 'IndexBuffer' here
		m_indexBufferObject.create();
		m_indexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);

		m_vao.create();		// create Vertex Array Object
		m_vao.bind();		// and bind it

		// now bind vertex buffer and index buffer
		m_indexBufferObject.bind();
		m_vbo.bind();

#define VERTEX_ARRAY_INDEX 0
#define COLOR_ARRAY_INDEX 1

		// layout(location = 0) = vec2 position
		m_shader->shaderProgram()->enableAttributeArray(VERTEX_ARRAY_INDEX);
		m_shader->shaderProgram()->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT,
									  0 /* position/vertex offset */,
									  3 /* three coords per position = vec3 */,
									  sizeof(VertexCR) /* vertex size */);
		m_shader->shaderProgram()->enableAttributeArray(COLOR_ARRAY_INDEX);
		m_shader->shaderProgram()->setAttributeBuffer(COLOR_ARRAY_INDEX, GL_FLOAT,
									  offsetof(VertexCR, m_colors) /* position/vertex offset */,
									  3 /* three coords per position = vec3 */,
									  sizeof(VertexCR) /* vertex size */);

		m_vao.release(); // Mind: always release VAO before index buffer
		m_vbo.release();
		m_indexBufferObject.release();
	}

	// *** generate geometry ***

	// create a temporary buffer that will contain the coordinates of all lines segments
	std::vector<VertexCR>		lineVertexBuffer;

	// first the coordinate lines (may be cylinders later)

	// we add three lines for the coordinate axes
	float length = 2;
	lineVertexBuffer.resize(6);
	lineVertexBuffer[0].m_coords = QVector3D(0,0,0);
	lineVertexBuffer[1].m_coords = QVector3D(length,0,0);
	lineVertexBuffer[2].m_coords = QVector3D(0,0,0);
	lineVertexBuffer[3].m_coords = QVector3D(0,length,0);
	lineVertexBuffer[4].m_coords = QVector3D(0,0,0);
	lineVertexBuffer[5].m_coords = QVector3D(0,0,length);

	lineVertexBuffer[0].m_colors = QVector3D(1,0,0);
	lineVertexBuffer[1].m_colors = QVector3D(1,0,0);
	lineVertexBuffer[2].m_colors = QVector3D(0,1,0);
	lineVertexBuffer[3].m_colors = QVector3D(0,1,0);
	lineVertexBuffer[4].m_colors = QVector3D(0,0,1);
	lineVertexBuffer[5].m_colors = QVector3D(0,0,1);

	unsigned int currentVertexIndex = 6;
	unsigned int currentElementIndex = 0;

	std::vector<QColor> cols(12, QColor(Qt::red));
	cols[0] = QColor(QColor("navy"));
//	for (unsigned int i=0; i<12; ++i) {
//		if (i % 3 == 0)			cols[i] = QColor(128,35,90);
//		else if (i % 3 == 0)	cols[i] = QColor(28,15,190);
//		else					cols[i] = QColor(80,255,120);
//	}
	addIkosaeder(IBKMK::Vector3D(0,0,0), cols, 0.8, currentVertexIndex, currentElementIndex, lineVertexBuffer, m_indexBufferData);

	// transfer geometry to GPU

	m_vbo.bind();
	m_vbo.allocate(lineVertexBuffer.data(), lineVertexBuffer.size()*sizeof(VertexCR));
	m_vbo.release();

	m_indexBufferObject.bind();
	m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GL_UNSIGNED_SHORT));
	m_indexBufferObject.release();
}


void CoordinateSystemObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
	m_indexBufferObject.destroy();
}


void CoordinateSystemObject::render() {
	m_vao.bind();

	// set transformation matrix
	m_shader->shaderProgram()->setUniformValue(m_shader->m_uniformIDs[1], m_transform.toMatrix());
	glDrawArrays(GL_LINES, 0, 6 /* Number of vertexes to process */);

	glDrawElements(GL_TRIANGLE_STRIP, m_indexBufferData.size(), GL_UNSIGNED_SHORT, nullptr);

	m_vao.release();
}

} // namespace Vic3D
