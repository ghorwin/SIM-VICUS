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

#include "Vic3DGridObject.h"

#include <QOpenGLShaderProgram>

#include <vector>

#include <VICUS_GridPlane.h>
#include "SVProjectHandler.h"
#include "Vic3DShaderProgram.h"

#include "SVSettings.h"

namespace Vic3D {

void GridObject::create(ShaderProgram * shaderProgram,
						std::vector<VICUS::GridPlane> & gridPlanes)
{
	m_gridShader = shaderProgram;

	// retrieve colors for first main grid (the other grid planes only have one color)
	SVSettings::ThemeType tt = SVSettings::instance().m_theme;
	const SVSettings::ThemeSettings & s = SVSettings::instance().m_themeSettings[tt];


	// resize vectors
	unsigned int nPlanes = gridPlanes.size();
	m_gridOffsets.resize(nPlanes);
	m_gridColors.resize(nPlanes);

	// now generate buffer data for all grids

	for (unsigned int i=0; i<nPlanes; ++i) {

		VICUS::GridPlane & gp = gridPlanes[i];

		// transfer grid colors
		if (i==0) {
			// transfer color of main grid
			QColor gridColor(s.m_minorGridColor);
			m_gridColors[0] = QVector3D((float)gridColor.redF(), (float)gridColor.greenF(), (float)gridColor.blueF());
			gridColor = s.m_majorGridColor;
			m_gridColors[1] = QVector3D((float)gridColor.redF(), (float)gridColor.greenF(), (float)gridColor.blueF());
		}
		else {
			// major grid
			m_gridColors[i*2 + 1] = QVector3D((float)gp.m_color.redF(), (float)gp.m_color.greenF(), (float)gp.m_color.blueF());
			// minor grid is always main color but a little brighter/darker depending on theme
			QColor minorGridCol;
			if (tt == SVSettings::TT_Dark)
				minorGridCol = gp.m_color.darker(100);
			else
				minorGridCol = gp.m_color.lighter(100);
			m_gridColors[i*2 + 0] = QVector3D((float)minorGridCol.redF(), (float)minorGridCol.greenF(), (float)minorGridCol.blueF());
		}

	// transfer grid dimensions
	float width = (float)prj.m_viewSettings.m_gridWidth;
	float spacing = (float)prj.m_viewSettings.m_gridSpacing;
	if (spacing <= 0)
		spacing = 1;

	if (width == m_width && spacing == m_spacing)
		return; // nothing to do since grid hasn't changed

	unsigned int N = width/spacing; // number of lines to draw in x and y direction
	// grid is centered around origin, and expands to width/2 in -x, +x, -y and +y direction
	// so N must be an even number, if not, we just add 1 and adjust the width
	//
	// For example, if you set width = 50, with spacing = 10, you get 5 grid cells in x and y direction
	// The coordinate origin would be in the middle of the third cell, and thus not on a grid line
	// Hence, we add one to get an even number of grid cells to both sides of the coordinate axes.
	if (N % 2 != 0) {
		++N;
		width = spacing*N; // adjust width
	}

	// N is currently the number of grid cells, i.e. number of "spacing" steps to take from left to right side of grid
	// Since we want to draw grid lines rather than cells, we increase the number by 1
	++N;

	// create a temporary buffer that will contain the x-y coordinates of all grid lines
	std::vector<float>			gridVertexBufferData;


	// we have 2*N lines for the major grid, each line requires two vertexes
	m_minorGridStartVertex = 2*N*2;
	// we have ((N-1)*10 + 1) lines for the entire grid including the minor grid (10 per grid cell + 1 in either direction)
	unsigned int minorGridFraction = 10;
	unsigned int N_minor = (N-1)*minorGridFraction + 1;
	m_vertexCount = 2*N_minor*2;

	m_gridLineCount = N_minor;

	// each vertex requires two floats (x and y coordinates)
	gridVertexBufferData.resize(m_vertexCount*2);
	float * gridVertexBufferPtr = gridVertexBufferData.data();

	// compute major grid lines first y = const
	float x1 = -width*0.5f;
	float x2 = width*0.5f;
	float y1 = -width*0.5f;
	float y2 = width*0.5f;

	// cache grid properties (used in snapping algorithm)
	m_minGrid = x1;
	m_maxGrid = x2;
	m_step = width/(N_minor-1);

	// add x coordinate line
	for (unsigned int i=0; i<N; ++i) {
		// skip main coordinate line
		if (i == N/2)
			continue;
		float y = width/(N-1)*i-width*0.5f;
		gridVertexBufferPtr[0] = x1;
		gridVertexBufferPtr[1] = y;
		gridVertexBufferPtr[2] = x2;
		gridVertexBufferPtr[3] = y;
		gridVertexBufferPtr += 4;
	}
	// compute grid lines with x = const
	for (unsigned int i=0; i<N; ++i) {
		// skip main coordinate line
		if (i == N/2)
			continue;
		float x = width/(N-1)*i-width*0.5f;
		gridVertexBufferPtr[0] = x;
		gridVertexBufferPtr[1] = y1;
		gridVertexBufferPtr[2] = x;
		gridVertexBufferPtr[3] = y2;
		gridVertexBufferPtr += 4;
	}

	// now add minor grid lines, hereby skip over major grid lines
	x1 = -width*0.5f;
	x2 = width*0.5f;
	for (unsigned int i=0; i<N_minor; ++i) {
		// skip grid lines that coincide with major grid lines
		if (i % minorGridFraction == 0) continue;
		float y = width/(N_minor-1)*i-width*0.5f;
		gridVertexBufferPtr[0] = x1;
		gridVertexBufferPtr[1] = y;
		gridVertexBufferPtr[2] = x2;
		gridVertexBufferPtr[3] = y;
		gridVertexBufferPtr += 4;
	}
	// compute grid lines with x = const
	y1 = -width*0.5f;
	y2 = width*0.5f;
	for (unsigned int i=0; i<N_minor; ++i) {
		// skip grid lines that coincide with major grid lines
		if (i % minorGridFraction == 0) continue;
		float x = width/(N_minor-1)*i-width*0.5f;
		gridVertexBufferPtr[0] = x;
		gridVertexBufferPtr[1] = y1;
		gridVertexBufferPtr[2] = x;
		gridVertexBufferPtr[3] = y2;
		gridVertexBufferPtr += 4;
	}

	// add main coordinates as last so that they overwrite all others

	// x-axis
	gridVertexBufferPtr[0] = x1;
	gridVertexBufferPtr[1] = 0;
	gridVertexBufferPtr[2] = x2;
	gridVertexBufferPtr[3] = 0;
	gridVertexBufferPtr += 4;

	// y-axis
	gridVertexBufferPtr[0] = 0;
	gridVertexBufferPtr[1] = y1;
	gridVertexBufferPtr[2] = 0;
	gridVertexBufferPtr[3] = y2;
	gridVertexBufferPtr += 4;

	// insert x and y axis line vertexes into buffer for special handling

	// Create Vertex Array Object and buffers if not done, yet
	if (!m_vao.isCreated()) {
		m_vao.create();		// create Vertex Array Object
		m_vao.bind();		// and bind it

		// Create Vertex Buffer Object (to store stuff in GPU memory)
		m_vbo.create();
		m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_vbo.bind();

		// layout(location = 0) = vec2 position
		m_gridShader->shaderProgram()->enableAttributeArray(0); // array with index/id 0
		m_gridShader->shaderProgram()->setAttributeBuffer(0, GL_FLOAT,
									  0 /* position/vertex offset */,
									  2 /* two floats per position = vec2 */,
									  0 /* vertex after vertex, no interleaving */);
	}

	m_vbo.bind();
	unsigned long vertexMemSize = gridVertexBufferData.size()*sizeof(float);
	m_vbo.allocate(gridVertexBufferData.data(), vertexMemSize);

	m_vao.release(); // Mind: always release VAO before index buffer
	m_vbo.release();
}


void GridObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
}

QVector3D qColorToQVector3D(const QColor &c){
	return QVector3D( c.redF(), c.greenF(), c.blueF() );
}


void GridObject::render() {
	// grid disabled?
	if (m_vertexCount == 0)
		return;

	m_vao.bind();

	// draw x axis
	QColor xLine;
	if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
		xLine = QColor("#b30404");
	else
		xLine = QColor("#b30404");

	// draw y axis
	QColor yLine;
	if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
		yLine = QColor("#0d8429");
	else
		yLine = QColor("#26a343");

//  QVector3D xColor(1.0f, 0.2f, 0.2f);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], qColorToQVector3D(xLine) );
	glDrawArrays(GL_LINES, m_vertexCount-4, 2);

	// TODO : Stephan farbe schick machen
//	QVector3D yColor(0.2f, 1.0f, 0.2f);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], qColorToQVector3D(yLine));
	glDrawArrays(GL_LINES, m_vertexCount-2, 2);

	// draw major grid lines
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_majorGridColor);

	// When drawing lines, we need two indexes for one line (start and end point)
	// The first parameter is the index to start from, the second parameter is the number of indexes
	// to use for drawing. So, if you want to draw 10 lines, pass 20 as "count" argument.
	//
	glDrawArrays(GL_LINES, 0, m_minorGridStartVertex-4);

	// draw minor grid lines, if enabled
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_minorGridColor);
	glDrawArrays(GL_LINES, m_minorGridStartVertex-4, m_vertexCount-m_minorGridStartVertex);


	m_vao.release();
}


bool GridObject::closestSnapPoint(QVector3D worldCoords, QVector3D & snapCoords) const {
	// z-coordinate should be close to zero for us to snap at the grid
	if (std::fabs(worldCoords.z()) > 1e-4)
		return false;

	// Indexes of the grid lines
	unsigned int i = 0;
	unsigned int j = 0;

	if (worldCoords.x() > m_maxGrid)
		i = m_gridLineCount-1;
	else if (worldCoords.x() > m_minGrid) {
		double offset = worldCoords.x() - m_minGrid;
		i = std::floor(offset/m_step);
		if (offset - i*m_step > m_step/2.0)
			++i;
	}
	if (worldCoords.y() > m_maxGrid)
		j = m_gridLineCount-1;
	else if (worldCoords.y() > m_minGrid) {
		double offset = worldCoords.y() - m_minGrid;
		j = std::floor(offset/m_step);
		if (offset - j*m_step > m_step/2.0)
			++j;
	}

	// recompute snap coordinates
	snapCoords = QVector3D(i*m_step + m_minGrid, j*m_step + m_minGrid, 0);
	return true;
}

} // namespace Vic3D
