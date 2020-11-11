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
	if (m_vao.isCreated())
		return;
	// create and bind Vertex Array Object
	m_vao.create();
	m_vao.bind();

	// set shader attributes
	// tell shader program we have two data arrays to be used as input to the shaders
	// create and bind vertex buffer
	m_vbo.create();
	m_vbo.bind(); // now m_vbo is "active"
	m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// index 0 = position
	shaderProgramm->enableAttributeArray(0); // array with index/id 0
	shaderProgramm->setAttributeBuffer(0, GL_FLOAT, 0, 3 /* vec3 */, sizeof(Vertex));
	// index 1 = normals
	shaderProgramm->enableAttributeArray(1); // array with index/id 1
	shaderProgramm->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, nx), 3 /* vec3 */, sizeof(Vertex));


	// create and bind color buffer
	m_vboColors.create();
	m_vboColors.bind();  // now m_vboColors is "active"
	m_vboColors.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// index 2 = color
	shaderProgramm->enableAttributeArray(2);
	shaderProgramm->setAttributeBuffer(0, GL_UNSIGNED_BYTE, 0, 4 /* vec4 */, 4 /* bytes = sizeof(ColorRGBA) */);

	// create and bind element buffer
	m_ebo.create();
	m_ebo.bind();
	m_ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// Release (unbind) all
	m_vao.release();

	m_vbo.release();
	m_vboColors.release();
	m_ebo.release();


	// store some test geometry - plane in x-z-plane facing in positive y direction
	Vertex p1(QVector3D(0,0,0), QVector3D(0,1,0));
	Vertex p2(QVector3D(100,0,0), QVector3D(0,1,0));
	Vertex p3(QVector3D(100,10,0), QVector3D(0,1,0));
	Vertex p4(QVector3D(0,10,0), QVector3D(0,1,0));

	m_vertexBufferData = {p1 ,p2 ,p3 ,p4};
	m_colorBufferData = { QColor(Qt::red), QColor(Qt::green), QColor(Qt::blue), QColor(Qt::magenta) };

	// setup element buffer for triangle strip
	m_elementBufferData = { 0, 1, 2, 3 };

	updateBuffers();
}


void OpaqueGeometryObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
	m_vboColors.destroy();
	m_ebo.destroy();
}


void OpaqueGeometryObject::updateBuffers() {
	// transfer data stored in m_vertexBufferData
	m_vbo.bind();
	m_vbo.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex));
	m_ebo.bind();
	m_ebo.allocate(m_elementBufferData.data(), m_elementBufferData.size()*sizeof(GL_UNSIGNED_SHORT));
	// also update the color buffer
	updateColorBuffer();
}


void OpaqueGeometryObject::updateColorBuffer() {
	m_vboColors.bind();
	m_vboColors.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA));
}


void OpaqueGeometryObject::render() {
	// bind all buffers ("position", "normal" and "color" arrays)
	m_vao.bind();
	// now draw the geometry
	glDrawElements(GL_TRIANGLE_STRIP, m_elementBufferData.size(), GL_UNSIGNED_SHORT, nullptr);
	// release buffers again
	m_vao.release();
}

} // namespace Vic3D
