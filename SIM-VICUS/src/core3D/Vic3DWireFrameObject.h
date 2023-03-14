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

#ifndef Vic3DWireFrameObjectH
#define Vic3DWireFrameObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "VICUS_Surface.h"
#include "Vic3DVertex.h"
#include "Vic3DTransform3D.h"

#include <set>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace VICUS {
	class Object;
}

namespace Vic3D {

class ShaderProgram;

/*! A container for geometry to be rendered with either triangle strips or triangles.
	This object is used multiple times and is not associated with any particular
	part of the scene or building. Hence, it is populated with data, externally.

	Rendering is done in a dual pass - first the geometry is rendered in polygon-mode, and
	afterwards in fill-mode. When in polygon mode, the shader is configured to brighten/darken
	the colors a bit.

	The object is ment to be used for movable, selected geometry, and uses the shaders:
	- VertexNormalColorWithTransform.vert
	- phong_lighting.frag (in fill mode)
	- phong_lighting_wireframe.frag (in polygon mode)

	The switch of the shader programs is done in the render function.
*/
class WireFrameObject {
public:
	WireFrameObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		This only initializes the buffers and vertex array object, but does not allocate data.
		This is done in a call to updateBuffers();
	*/
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Resizes vertex and element buffers on GPU memory and copies data from locally
		stored vertex/element arrays to GPU.
		This might be a lengthy operation, so call this as infrequently as possible.

		Basically transfers data in m_vertexBufferData, m_colorBufferData and
		m_elementBufferData to GPU memory.
		Calls the function updateColorBuffer() internally to update the color buffer.
	*/
	void updateBuffers();

	/*! Binds the vertex array object and renders the geometry. */
	void render();

	/*! Returns transform object. */
	const Transform3D & transform() const { return m_transform; }

	/*! Resets transformation and clears all transform vectors. */
	void resetTransformation();

	/*! Returns components of current transformation.
		If scaleFactors is not (0,0,0), we have a local scaling operation.
		If scaleFactors is (0,0,0) and rotation is QQuaternion(), than we only have translation, otherwise
		rotation with translation.
	*/
	void currentTransformation(QVector3D & translation, QQuaternion & rotation, QVector3D & scalefactors) {
		translation = m_translation;
		rotation = m_rotation;
		scalefactors = m_scaling;
	}

	void translate(const QVector3D & translation);
	void rotate(const QQuaternion & rotation, const QVector3D & offset);
	void localScaling(const QVector3D & offset, const QQuaternion & toLocal, const QVector3D & localScaleFactors);

	/*! This set caches the list of current selected objects.
		This set is processed in updateBuffers() to fill the coordinate buffers.
	*/
	std::set<const VICUS::Object*>		m_selectedObjects;

	ShaderProgram						*m_shaderProgram = nullptr;

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<VertexC>				m_vertexBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLuint>					m_indexBufferData;

	/*! Maps unique surface/node ID to vertex start index in m_vertexBufferData. */
	std::map<unsigned int, unsigned int>	m_vertexStartMap;

	/*! VertexArrayObject, references the vertex, color and index buffers. */
	QOpenGLVertexArrayObject			m_vao;

	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer						m_vertexBufferObject;
	/*! Handle for index buffer on GPU memory */
	QOpenGLBuffer						m_indexBufferObject;

private:
	/*! The transformation from model coordinates to (current) world coordinates. */
	Transform3D							m_transform;


	// *** Variables below cache the manual transformation made to the selection/wireframe object. */

	/*! Caches the translation vector. */
	QVector3D							m_translation;
	/*! Caches the scaling vector - if not 0,0,0 the "local scaling operation" is used. */
	QVector3D							m_scaling;
	/*! Caches the rotation matrix. */
	QQuaternion							m_rotation;
};

} // namespace Vic3D

#endif // Vic3DWireFrameObjectH
