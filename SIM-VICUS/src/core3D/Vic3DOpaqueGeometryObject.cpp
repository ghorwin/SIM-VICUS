#include "Vic3DOpaqueGeometryObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

namespace Vic3D {

OpaqueGeometryObject::OpaqueGeometryObject() :
	m_vbo(QOpenGLBuffer::VertexBuffer), // VertexBuffer is the default, so default constructor would have been enough
	m_vboColors(QOpenGLBuffer::VertexBuffer),
	m_ebo(QOpenGLBuffer::IndexBuffer) // make this an Index Buffer
{
}


void OpaqueGeometryObject::create(QOpenGLShaderProgram * shaderProgramm) {
	// create and bind Vertex Array Object
	m_vao.create();
	m_vao.bind();

	// create and bind element buffer
	m_ebo.create();
	m_ebo.bind();
	m_ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// create and bind vertex buffer
	m_vbo.create();
	m_vbo.bind();
	m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
//	int vertexMemSize = m_vertexBufferData.size()*sizeof(Vertex);
//	qDebug() << "OpaqueGeometryObject - VertexBuffer size =" << vertexMemSize/1024.0 << "kByte";
//	m_vbo.allocate(m_vertexBufferData.data(), vertexMemSize);

//	int elementMemSize = m_elementBufferData.size()*sizeof(GLuint);
//	qDebug() << "OpaqueGeometryObject - ElementBuffer size =" << elementMemSize/1024.0 << "kByte";
//	m_ebo.allocate(m_elementBufferData.data(), elementMemSize);

	// set shader attributes
	// tell shader program we have two data arrays to be used as input to the shaders

	// index 0 = position
	shaderProgramm->enableAttributeArray(0); // array with index/id 0
	shaderProgramm->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(Vertex));
	// index 1 = normals
	shaderProgramm->enableAttributeArray(1); // array with index/id 1
	shaderProgramm->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, nx), 3, sizeof(Vertex));

	m_vbo.release();

	// create and bind color buffer
	m_vboColors.create();
	m_vboColors.bind();
	m_vboColors.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// index 2 = color
	shaderProgramm->enableAttributeArray(2);
	shaderProgramm->setAttributeBuffer(0, GL_FLOAT, 0, 4, sizeof(ColorRGBA));

	// Release (unbind) all
	m_vao.release();
	m_vboColors.release();

	m_ebo.release();
}


void OpaqueGeometryObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
	m_vboColors.destroy();
	m_ebo.destroy();
}


void OpaqueGeometryObject::render() {
	// set the geometry ("position", "normal" and "color" arrays)
	m_vao.bind();

	// now draw the geometry
	glDrawElements(GL_TRIANGLE_STRIP, m_elementBufferData.size(), GL_UNSIGNED_SHORT, nullptr);
	// release vertices again
	m_vao.release();
}

} // namespace Vic3D
