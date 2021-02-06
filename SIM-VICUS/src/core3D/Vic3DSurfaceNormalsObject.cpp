/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#include "Vic3DSurfaceNormalsObject.h"

#include <QOpenGLShaderProgram>

#include <vector>

#include <VICUS_Project.h>
#include "SVProjectHandler.h"
#include "Vic3DShaderProgram.h"

#include "SVSettings.h"

namespace Vic3D {

void SurfaceNormalsObject::create(ShaderProgram * shaderProgram) {
	m_shaderProgram = shaderProgram;

	// Create Vertex Array Object and buffers if not done, yet
	if (!m_vao.isCreated()) {
		m_vao.create();		// create Vertex Array Object
		m_vao.bind();		// and bind it

		// Create Vertex Buffer Object (to store stuff in GPU memory)
		m_vbo.create();
		m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_vbo.bind();

		// layout(location = 0) = vec2 position
		m_shaderProgram->shaderProgram()->enableAttributeArray(0); // array with index/id 0
		m_shaderProgram->shaderProgram()->setAttributeBuffer(0, GL_FLOAT,
									  0 /* position/vertex offset */,
									  2 /* two floats per position = vec2 */,
									  0 /* vertex after vertex, no interleaving */);
	}

	m_vao.release(); // Mind: always release VAO before index buffer
	m_vbo.release();
}


void SurfaceNormalsObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
}


void SurfaceNormalsObject::updateVertexBuffers() {
	// populate vertex buffer object and push to GPU

}


void SurfaceNormalsObject::render() {
	m_vao.bind();
	glDrawArrays(GL_LINES, m_vertexCount, 2);
	m_vao.release();
}

} // namespace Vic3D
