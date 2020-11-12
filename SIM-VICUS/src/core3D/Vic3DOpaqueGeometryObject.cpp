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
	m_vertexBufferData.resize(N_Vertices);
	m_colorBufferData.resize(N_Vertices);
	for (int v=0; v<N_Vertices; ++v) {
		// coordinates
		m_vertexBufferData[v].x = 100*vertices[3*v]/2;
		m_vertexBufferData[v].y = 100*vertices[3*v+1]/2;
		m_vertexBufferData[v].z = 100*vertices[3*v+2]/2;

		// colors
		m_colorBufferData[v].r = vertexColors[v].red();
		m_colorBufferData[v].g = vertexColors[v].green();
		m_colorBufferData[v].b = vertexColors[v].blue();
		m_colorBufferData[v].a = vertexColors[v].alpha();
	}

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

	m_elementBufferData = std::vector<GLshort>(indices, indices + 9);

	// create a new buffer for the vertices and colors, interleaved storage
	m_vbo.create();
	m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

	m_vboColors.create();
	m_vboColors.setUsagePattern(QOpenGLBuffer::StaticDraw);
	m_vboColors.bind();
	m_vboColors.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA) );

	// create a new buffer for the indexes
	m_ebo.create();
	m_ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// create and bind Vertex Array Object - must be bound *before* the element buffer is bound,
	// because the VAO remembers and manages element buffers as well
	m_vao.create();
	m_vao.bind();


	m_ebo.bind(); // this registers this index buffer in the currently bound vao

	m_vbo.bind(); // this registers this buffer data object in the currently bound vao; in subsequent
				  // calls to shaderProgramm->setAttributeBuffer() the buffer object is associated with the
				  // respective attribute array that's fed into the shader. When the vao is later bound before
				  // rendering, this association is remembered so that the vertex fetch stage pulls data from
				  // this vbo

	// layout location 0 - vec3 with coordinates
	shaderProgramm->enableAttributeArray(0);
	shaderProgramm->setAttributeBuffer(0, GL_FLOAT, 0, 3, 3*sizeof(float));

	// layout location 1 - vec3 with colors
	m_vboColors.bind(); // now color buffer is active in vao
	shaderProgramm->enableAttributeArray(1);
	shaderProgramm->setAttributeBuffer(1, GL_UNSIGNED_BYTE, 0, 4, 4*sizeof(char));

	// Release (unbind) all

	// Mind: you can release the buffer data objects (vbo and vboColors) before or after releasing vao. It does not
	//       matter, because the buffers are associated already with the attribute arrays.
	//       However, YOU MUST NOT release the index buffer (ebo) before releasing the vao, since this would remove
	//       the index buffer association with the vao and when binding the vao before rendering, the element buffer
	//       would not be known and a call to glDrawElements() crashes!
	m_vao.release();

	m_vbo.release();
	m_vboColors.release();
	m_ebo.release();

	updateBuffers();
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
	m_vbo.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex2));
	m_vbo.release();

	m_ebo.bind();
	m_ebo.allocate(m_elementBufferData.data(), m_elementBufferData.size()*sizeof(GL_UNSIGNED_SHORT));
	m_ebo.release();
	// also update the color buffer
//	updateColorBuffer();
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
