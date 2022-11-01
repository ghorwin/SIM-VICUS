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

#ifndef Vic3DRubberbandObjectH
#define Vic3DRubberbandObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

#include "Vic3DTransform3D.h"
#include "Vic3DVertex.h"
#include "Vic3DCamera.h"

namespace Vic3D {

class ShaderProgram;

/*! Draws the mini version of the local coordinate system.
	The coordinate system object is composed of several objects, and only some of them are shown
	together.

	Regular coordinate system object (drawn with SHADER_COORDINATE_SYSTEM):
	- one big sphere in the middle          indexBufferObject 0
	- one opaque cylinder per axis          indexBufferObject 1-3
	- one small sphere at each axis end     indexBufferObject 4-6

	After that (drawn with SHADER_TRANSPARENT_GEOMETRY):
	- a plane in the background

	The geometry up to the plane is drawn using triangle strips with index-based drawing.
	The plane is being drawn with drawArrays().
*/
class RubberbandObject {
public:

	RubberbandObject();

	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * opaquePhongShaderProgram);
	void destroy();

	/*! Binds the buffer and paints. */
	void render();

	void setStartPoint(const QVector3D &topLeft);
	void setRubberband(const QVector3D &bottomRight);

	QRect						m_viewportRect;

private:

	/*! Holds the number of vertices (2 for each line), updated in create(), used in render().
		If zero, grid is disabled.
	*/
	GLsizei						m_vertexCount;

	/*! The transformation object, transforms the coordinate system to its position and orientation in
		the scene.
	*/
	Transform3D					m_transform;

	QVector3D					m_topLeft;

	/*! Shader program. */
	ShaderProgram				*m_rubberbandShaderProgram = nullptr;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Handle for vertex buffer on GPU memory. */
	QOpenGLBuffer				m_vbo;



	/*! Vertex buffer in CPU memory, holds data of all vertices (coords and normals). */
	std::vector<Vertex>			m_vertexBufferData;
	/*! Color buffer in CPU memory, holds colors of all vertices (same size as m_vertexBufferData). */
	std::vector<ColorRGBA>		m_colorBufferData;
	/*! Index buffer on CPU memory. */
	std::vector<GLuint>			m_indexBufferData;

	unsigned int				m_planeStartIndex;
	unsigned int				m_planeStartVertex;
};

} // namespace Vic3D


#endif // Vic3DRubberbandObjectH
