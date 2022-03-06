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

/*! This class holds all data needed to draw grids on the screen.
	We have only a coordinate buffer, which holds tightly packed 2 * vec2 (start and end points of lines, xz coords)
	with y=0 implied.
	Grid colors are uniforms per grid, as is background color.

	The grid is drawn with the grid shader program, which is passed to the create() function.

	The grid lines for major and minor grid are always generated, but minor
*/
class GridObject {
public:
	/*! The function is called during OpenGL initialization, where the OpenGL context is current.
		Note: we store temporary values in the GridPlane objects themselves, instead of this object, since some
			  of the properties are needed for snapping as well.
	*/
	void create(ShaderProgram * shaderProgram, std::vector<VICUS::GridPlane> & gridPlanes);
	void destroy();

	/*! Binds the buffer and paints. */
	void render();

	/*! Shader program, that the grid is painted with. */
	ShaderProgram				*m_gridShader = nullptr;

	/*! Vector with grid index start offsets (size = 2*number of grid planes + 1), first index is always
		start of major grid lines, second index is start of minor grid lines.
	*/
	std::vector<GLsizei>		m_gridOffsets;
	/*! Vector with grid colors (size = 2*number of grid planes) (first is major color, second minor grid color). */
	std::vector<QVector3D>		m_gridColors;

	/*! Wraps an OpenGL VertexArrayObject, that references the vertex coordinates. */
	QOpenGLVertexArrayObject	m_vao;
	/*! Holds positions of grid lines. */
	QOpenGLBuffer				m_vbo;

	/*! Set to false if no grid is visible - speeds up rendering a little. */
	bool						m_anyGridVisible = true;

private:
	// local copy of grid planes, needed for rending
	std::vector<bool>			m_gridPlaneVisible;
};

} // namespace Vic3D


#endif // Vic3DGridObjectH
