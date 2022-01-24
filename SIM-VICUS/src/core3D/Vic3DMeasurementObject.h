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

#ifndef Vic3DMeasurementObjectH
#define Vic3DMeasurementObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace Vic3D {

class ShaderProgram;

/*! This is the line object drawn when measuring distances in the 3D scene.
*/
class MeasurementObject {
public:
	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * shaderProgram);
	void destroy();

	/*! Modifies the location of the local coordinate system (i.e. new end point). */
	void setTranslation(const QVector3D & translation);

	/*! Binds the buffer and paints. */
	void render();

	/*! Returns the distances between start and end points in [m]. */
	double distance() const { return (double)m_endPoint.distanceToPoint(m_startPoint); }

	/*! Shader program, that the grid is painted with. */
	ShaderProgram				*m_gridShader = nullptr;

	/*! Holds the number of vertices (2 for each line), updated in create(), used in render().
		If zero, grid is disabled.
	*/
	GLsizei						m_vertexCount;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Holds positions of grid lines. */
	QOpenGLBuffer				m_vbo;

	/*! Starting point of the line distance measurement. */
	QVector3D					m_startPoint;
	/*! Starting point of the line distance measurement. */
	QVector3D					m_endPoint;
};

} // namespace Vic3D


#endif // Vic3DMeasurementObjectH
