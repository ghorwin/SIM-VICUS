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

#ifndef Vic3DTransparentBuildingObjectH
#define Vic3DTransparentBuildingObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "Vic3DVertex.h"


namespace Vic3D {

class ShaderProgram;

/*! A container for geometry to be rendered fully transparent.
	This object is drawn only when in "interlinked surface" mode and has the option to highlight "active"
	elements. Hence, color buffer update is separate from geometry update.

	Geometry is pretty much the same as for the opaque building i.e. only visible, not-selected objects are
	included. Also, for each surface-surface-link a custom geometry is added (a box-like object).
*/
class TransparentBuildingObject {
public:
	TransparentBuildingObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		This only initializes the buffers and vertex array object, but does not allocate data.
		This is done in a call to updateBuffers();
	*/
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Resizes vertex and element buffers on GPU memory and copies data from locally stored vertex/element arrays to GPU.
		This might be a lengthy operation, so call this as infrequently as possible.

		Basically transfers data in m_vertexBufferData, m_colorBufferData and m_elementBufferData to GPU memory.
		Calls the function updateColorBuffer() internally to update the color buffer.
	*/
	void updateBuffers();
	/*! Only copies the color buffer m_colorBufferData to GPU memory.
		Call this function instead of updateBuffers(), if only colors of objects/visibility have changed.
	*/
	void updateColorBuffer();

	/*! Renders in transparent mode (OpenGL context is expected to be setup correctly, already). */
	void renderTransparent();

	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<Vertex>			m_vertexBufferData;
	/*! Color buffer in CPU memory, holds colors of all vertices (same size as m_vertexBufferData). */
	std::vector<ColorRGBA>		m_colorBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLuint>			m_indexBufferData;

	/*! Maps unique surface/node ID to vertex start index in m_vertexBufferData. */
	std::map<unsigned int, unsigned int>	m_vertexStartMap;

	/*! VertexArrayObject, references the vertex, color and index buffers. */
	QOpenGLVertexArrayObject	m_vao;

	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer				m_vertexBufferObject;
	/*! Handle for color buffer on GPU memory. */
	QOpenGLBuffer				m_colorBufferObject;
	/*! Handle for index buffer on GPU memory */
	QOpenGLBuffer				m_indexBufferObject;

private:
	/*! Shader program (not owned). */
	ShaderProgram					*m_shaderProgram = nullptr;

};

} // namespace Vic3D

#endif // Vic3DTransparentBuildingObjectH
