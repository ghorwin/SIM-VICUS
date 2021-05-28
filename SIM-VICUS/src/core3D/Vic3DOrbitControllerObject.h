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

#ifndef Vic3DOrbitControllerObjectH
#define Vic3DOrbitControllerObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

#include "Vic3DTransform3D.h"

namespace Vic3D {

class ShaderProgram;

/*! Draws the rotation axis for the orbital controller.
	The orbit indicator is a set of line segments, one for the rotation axis, and one for the ring.
*/
class OrbitControllerObject {
public:
	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Binds the buffer and paints. */
	void render();

	/*! The transformation object, transforms the rotation object drawn
		around the z-axis in the coordinate origin, moves it to the center of the orbit
		and rotates it such, that the orbit is correctly drawn.
	*/
	Transform3D					m_transform;

	/*! Shader program. */
	ShaderProgram				*m_shaderProgram = nullptr;

	/*! Holds the number of vertices (2 for each line), updated in create(), used in render(). */
	GLsizei						m_vertexCount;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Holds coordinates. */
	QOpenGLBuffer				m_vbo;

};

} // namespace Vic3D


#endif // Vic3DOrbitControllerObjectH
