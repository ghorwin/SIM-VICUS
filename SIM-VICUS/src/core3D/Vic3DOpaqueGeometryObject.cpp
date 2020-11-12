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


#if 1

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	float vertices[] = {
		 0.8f,  0.8f, 0.0f,  // top right		= 0
		 0.8f, -0.8f, 0.0f,  // bottom right	= 1
		-0.8f,  0.8f, 0.0f,   // top left		= 2
		-0.8f, -0.8f, 0.0f,  // bottom left		= 3
		1.8f,  0.8f, 0.0f,  // top right		= 0
		1.8f, -0.8f, 0.0f,  // bottom right	= 1
		1.0f,  0.8f, 0.0f,   // top left		= 2
		1.0f, -0.8f, 0.0f  // bottom left		= 3
	};

	QColor vertexColors [] = {
		// left rect

		QColor("#ff0000"),  // red    // top right
		QColor("#00ff00"), // green  // bottom right
		QColor("#0000ff"), // blue   // top left
		QColor("#ffffff"), // white  // bottom left

		// right rect

		QColor("#ff0000"),	// red - top right
		QColor("#ff00ff"),	// magenta - bottom right
		QColor("#ffffff"),	// white - top left
		QColor("#00ff00")	// green - bottom left
	};

	// create buffer for 2 interleaved attributes: position and color, 4 vertices, 3 floats each
	unsigned int N_Vertices = 8;
	std::vector<float> vertexBufferData(N_Vertices*3);
	std::vector<char> colorBufferData(N_Vertices*4);
	// create new data buffer - the following memory copy stuff should
	// be placed in some convenience class in later tutorials
	// copy data in interleaved mode with pattern p0c0|p1c1|p2c2|p3c3
	float * buf = vertexBufferData.data();
	for (int v=0; v<N_Vertices; ++v, buf += 3) {
		// coordinates
		buf[0] = 100*vertices[3*v]/2;
		buf[1] = 100*vertices[3*v+1]/2;
		buf[2] = 100*vertices[3*v+2]/2;

		// colors
		colorBufferData[v*4 + 0] = vertexColors[v].red();
		colorBufferData[v*4 + 1] = vertexColors[v].green();
		colorBufferData[v*4 + 2] = vertexColors[v].blue();
		colorBufferData[v*4 + 3] = vertexColors[v].alpha();
	}

	// create a new buffer for the vertices and colors, interleaved storage
	m_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	m_vbo.create();
	m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_vbo.bind();
	// now copy buffer data over: first argument pointer to data, second argument: size in bytes
	m_vbo.allocate(vertexBufferData.data(), vertexBufferData.size()*sizeof(float) );
//	m_vbo.release();

	m_vboColors = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	m_vboColors.create();
	m_vboColors.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_vboColors.bind();
	m_vboColors.allocate(colorBufferData.data(), colorBufferData.size()*sizeof(char) );

	// create and bind Vertex Array Object - must be bound *before* the element buffer is bound,
	// because the VAO remembers and manages element buffers as well
	m_vao.create();
	m_vao.bind();

//	glEnable(GL_CULL_FACE);
//#define USE_DEGENERATED_TRIANGLE_RESTART
#ifdef USE_DEGENERATED_TRIANGLE_RESTART
	GLushort indices[] = {  // note that we start from 0!
		0, 1, 2, 3,
		3, // duplicate last of first strip
		4, // duplicate first of second strip
		4, 5, 6, 7
	};

	// Note: when inserting a gap from even to odd element, insert 2; otherweise 3
	m_indexCount = 10;
#else
	glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
	GLushort indices[] = {  // note that we start from 0!
		0, 1, 2, 3,
		0xFFFF,
		4, 5, 6, 7
	};
//	m_indexCount = 9;
#endif
	// create a new buffer for the indexes
	m_ebo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); // Mind: use 'IndexBuffer' here
	m_ebo.create();
	m_ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_ebo.bind();
	m_ebo.allocate(indices, sizeof(indices) );

	m_elementBufferData = std::vector<GLshort>(indices, indices + 9);

	// stride = number of bytes for one vertex (with all its attributes) = 3+3 floats = 6*4 = 24 Bytes

	// layout location 0 - vec3 with coordinates
	m_vbo.bind();
	shaderProgramm->enableAttributeArray(0);
	shaderProgramm->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3*sizeof(float));

	// layout location 1 - vec3 with colors
	m_vboColors.bind();
	shaderProgramm->enableAttributeArray(1);
	shaderProgramm->setAttributeBuffer(1, GL_UNSIGNED_BYTE, 0, 4, 4*sizeof(char));

	m_vao.release();
	m_vbo.release();
	m_vboColors.release();

	// Release (unbind) all
	m_vao.release();
#else
	// set shader attributes
	// tell shader program we have two data arrays to be used as input to the shaders
	// create and bind vertex buffer
	m_vbo.create();
	m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// create and bind color buffer
	m_vboColors.create();
	m_vboColors.setUsagePattern(QOpenGLBuffer::StaticDraw);


	// create and bind Vertex Array Object
	m_vao.create();
	m_vao.bind();

	m_vbo.bind(); // now m_vbo is "active"

	// index 0 = position
	shaderProgramm->enableAttributeArray(0); // array with index/id 0
	shaderProgramm->setAttributeBuffer(0, GL_FLOAT, 0, 3 /* vec3 */, sizeof(Vertex));
	// index 1 = normals
//	shaderProgramm->enableAttributeArray(1); // array with index/id 1
//	shaderProgramm->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, nx), 3 /* vec3 */, sizeof(Vertex));
	m_vbo.release();


	m_vboColors.bind();  // now m_vboColors is "active"


	// index 2 = color
	shaderProgramm->enableAttributeArray(1);
	shaderProgramm->setAttributeBuffer(0, GL_UNSIGNED_BYTE, 0, 4 /* vec4 */, 4 /* bytes = sizeof(ColorRGBA) */);

	// create and bind element buffer
	m_ebo.create();
	m_ebo.bind();
	m_ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// Release (unbind) all
	m_vao.release();

	m_vboColors.release();
	m_ebo.release();


	// store some test geometry - plane in x-z-plane facing in positive y direction
	Vertex p1(QVector3D(0,0,0), QVector3D(0,1,0));
	Vertex p2(QVector3D(100,0,0), QVector3D(0,1,0));
	Vertex p3(QVector3D(100,10,0), QVector3D(0,1,0));
	Vertex p4(QVector3D(0,10,0), QVector3D(0,1,0));

	m_vertexBufferData = {p1 ,p2 ,p4 ,p3};
	m_colorBufferData = { QColor(Qt::red), QColor(Qt::green), QColor(Qt::blue), QColor(Qt::magenta) };

	// setup element buffer for triangle strip
	m_elementBufferData = { 0, 1, 2, 3 };

	updateBuffers();
#endif
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
	m_vbo.release();

	m_ebo.bind();
	m_ebo.allocate(m_elementBufferData.data(), m_elementBufferData.size()*sizeof(GL_UNSIGNED_SHORT));
	m_ebo.release();
	// also update the color buffer
	updateColorBuffer();
}


void OpaqueGeometryObject::updateColorBuffer() {
	m_vboColors.bind();
	m_vboColors.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA));
	m_vboColors.release();
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
