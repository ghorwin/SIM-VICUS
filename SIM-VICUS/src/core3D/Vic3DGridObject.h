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

#ifndef Vic3DGridObjectH
#define Vic3DGridObjectH

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <QVector3D>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace VICUS {
	class GridPlane;
}

namespace Vic3D {

class ShaderProgram;

/*! This class holds all data needed to draw a grid on the screen.
	We have only a coordinate buffer, which holds tightly packed 2 * vec2 (start and end points of lines, xz coords)
	with y=0 implied.
	Grid colors are uniforms, as is background color.

	The grid is drawn with the grid shader program, which is passed to the create() function.

	The grid lines for major and minor grid are always generated, but minor
*/
class GridObject {
public:
	/*! The function is called during OpenGL initialization, where the OpenGL context is current. */
	void create(ShaderProgram * shaderProgram, std::vector<VICUS::GridPlane> & gridPlanes);
	void destroy();

	/*! Binds the buffer and paints. */
	void render();

	/*! Determines the closest grid snap point (depending on snap-to-grid properties). */
	bool closestSnapPoint(QVector3D worldCoords, QVector3D & snapCoords) const;

	/*! Shader program, that the grid is painted with. */
	ShaderProgram				*m_gridShader = nullptr;

	/*! Color for major grid lines. */
	QVector3D					m_majorGridColor;
	/*! Color for minor grid lines. */
	QVector3D					m_minorGridColor;

	/*! Holds the number of vertices (2 for each line), updated in create(), used in render().
		If zero, grid is disabled.
	*/
	GLsizei						m_vertexCount;

	/*! Index, where minor grid starts. If same as m_vertexCount, minor grid is disabled. */
	GLsizei						m_minorGridStartVertex;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Holds positions of grid lines. */
	QOpenGLBuffer				m_vbo;

	/*! Cached grid width. */
	float						m_width		= 999;
	/*! Cached grid spacing. */
	float						m_spacing	= 999;

	/*! Cached grid line count (both directions). */
	unsigned int				m_gridLineCount;
	/*! Cached min grid coordinate (same for x and y). */
	double						m_minGrid;
	/*! Cached max grid coordinate (same for x and y). */
	double						m_maxGrid;
	/*! Cached grid step size for minor grid (same for x and y). */
	double						m_step;

	/*! Set to false if no grid is visible - speeds up rendering a little. */
	bool						m_anyGridVisible = true;
};

} // namespace Vic3D


#endif // Vic3DGridObjectH
