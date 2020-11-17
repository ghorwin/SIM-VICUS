/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#include "Vic3DOrbitControllerObject.h"

#include <QOpenGLShaderProgram>

#include "Vic3DShaderProgram.h"
#include "Vic3DVertex.h"

namespace Vic3D {

void OrbitControllerObject::create(ShaderProgram * shaderProgram) {
	m_shader = shaderProgram;

	// create a temporary buffer that will contain the coordinates of all lines segments
	std::vector<VertexC>			lineVertexBuffer;

	// rotation axis is z-axis
	lineVertexBuffer.resize(2);
	lineVertexBuffer[0] = QVector3D(0,0,-50.f);
	lineVertexBuffer[1] = QVector3D(0,0,+50.f);
	m_vertexCount = 2;

	// Create Vertex Array Object and buffers if not done, yet
	if (!m_vao.isCreated()) {
		m_vao.create();		// create Vertex Array Object
		m_vao.bind();		// and bind it

		// Create Vertex Buffer Object (to store stuff in GPU memory)
		m_vbo.create();
		m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_vbo.bind();

		// layout(location = 0) = vec2 position
		m_shader->shaderProgram()->enableAttributeArray(0); // array with index/id 0
		m_shader->shaderProgram()->setAttributeBuffer(0, GL_FLOAT,
									  0 /* position/vertex offset */,
									  3 /* three coords per position = vec3 */,
									  sizeof(VertexC) /* vertex after vertex, no interleaving */);
	}

	m_vbo.bind();
	unsigned long vertexMemSize = lineVertexBuffer.size()*sizeof(VertexC);
	m_vbo.allocate(lineVertexBuffer.data(), vertexMemSize);

	m_vao.release(); // Mind: always release VAO before index buffer
	m_vbo.release();
}


void OrbitControllerObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
}


void OrbitControllerObject::render() {
	m_vao.bind();

	// set transformation matrix
	m_shader->shaderProgram()->setUniformValue(m_shader->m_uniformIDs[1], m_transform.toMatrix());
	glDrawArrays(GL_LINES, 0, m_vertexCount);

	m_vao.release();
}

} // namespace Vic3D
