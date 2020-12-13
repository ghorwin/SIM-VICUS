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

#include "SVViewStateHandler.h"
#include "SVPropEditGeometry.h"

namespace Vic3D {

CoordinateSystemObject::CoordinateSystemObject() {
	// make us known to the world
	SVViewStateHandler::instance().m_coordinateSystemObject = this;

	// Initialize transform
	m_transform.translate(QVector3D(0,0,0));
	updateInverse();
}


void CoordinateSystemObject::create(ShaderProgram * shaderProgram) {
	m_shaderProgram = shaderProgram;

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
	QOpenGLShaderProgram * shaderProg = shaderProgram->shaderProgram();
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

	double sizeFactor = 1;

	addSphere(IBKMK::Vector3D(0,0,0), QColor("burlywood"), 0.2*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(2,0,0), QColor(Qt::red), 0.02*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,2,0), QColor(0,196,0), 0.02*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addCylinder(IBKMK::Vector3D(0,0,0), IBKMK::Vector3D(0,0,2), QColor(32,32,255), 0.02*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addSphere(IBKMK::Vector3D(2,0,0), QColor(255, 245, 152), 0.1*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addSphere(IBKMK::Vector3D(0,2,0), QColor(255, 245, 152), 0.1*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);
	addSphere(IBKMK::Vector3D(0,0,2), QColor(255, 245, 152), 0.1*sizeFactor, currentVertexIndex, currentElementIndex,
			  m_vertexBufferData, m_colorBufferData, m_indexBufferData);

	// transfer geometry to GPU

	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(Vertex));
	m_vertexBufferObject.release();

	m_colorBufferObject.bind();
	m_colorBufferObject.allocate(m_colorBufferData.data(), m_colorBufferData.size()*sizeof(ColorRGBA) );
	m_colorBufferObject.release();

	m_indexBufferObject.bind();
	m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GLshort));
	m_indexBufferObject.release();
}


void CoordinateSystemObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_colorBufferObject.destroy();
	m_indexBufferObject.destroy();
}


void CoordinateSystemObject::renderOpaque() {
	// bind all buffers ("position", "normal" and "color" arrays)
	m_vao.bind();
	// draw with face culling on
	glEnable(GL_CULL_FACE);
	// set transformation matrix
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[4], m_transform.toMatrix());
	// now draw the geometry
	glDrawElements(GL_TRIANGLE_STRIP, m_indexBufferData.size(), GL_UNSIGNED_SHORT, nullptr);
	// release buffers again
	m_vao.release();
}


void CoordinateSystemObject::renderTransparent() {

}


void CoordinateSystemObject::setTranslation(const QVector3D & translation) {
	m_transform.setTranslation(translation);
	// tell the property widget for editing geometry our new position/rotation
	m_propEditGeometry->setCoordinates(m_transform);
	updateInverse();
}


void CoordinateSystemObject::setRotation(const QQuaternion & rotMatrix) {
	m_transform.setRotation(rotMatrix);
	updateInverse();
//	qDebug() << localXAxis() << localYAxis() << localZAxis();
}


void CoordinateSystemObject::setTransform(const Transform3D & transform) {
	m_transform = transform;
	updateInverse();
}


void CoordinateSystemObject::updateInverse() {
	m_inverseMatrix.setToIdentity();
	m_inverseMatrix.rotate(m_transform.rotation().conjugated());
	m_inverseMatrix.translate(-m_transform.translation());
}


} // namespace Vic3D
