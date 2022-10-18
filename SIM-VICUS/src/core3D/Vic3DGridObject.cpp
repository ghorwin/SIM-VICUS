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
#include "SVConversions.h"

#include "Vic3DShaderProgram.h"
#include "Vic3DTransform3D.h"

#include "SVSettings.h"

namespace Vic3D {


void GridObject::create(ShaderProgram * shaderProgram,
						const std::vector<VICUS::GridPlane> & gridPlanes)
{
	m_gridShader = shaderProgram;

	// retrieve colors for first main grid (the other grid planes only have one color)
	SVSettings::ThemeType tt = SVSettings::instance().m_theme;
	const SVSettings::ThemeSettings & s = SVSettings::instance().m_themeSettings[tt];


	// resize vectors
	unsigned int nPlanes = gridPlanes.size();

	// we make space for each plane, even though they may not be used
	m_gridOffsets.clear();
	m_gridColors.resize(2*nPlanes);
	m_gridPlaneVisible.resize(nPlanes);
	m_planeTransformationMatrix.resize(nPlanes);

	// now generate buffer data for all grids
	std::vector<float>			gridVertexBufferData;
	// buffer that will contain the x-y coordinates of all grid lines
	m_gridOffsets.push_back(0); // first grid starts at index 0

	m_anyGridVisible = false;

	// Process all planes
	// Note: only data for visible planes is processed, the rest are filtered out
	for (unsigned int i=0; i<nPlanes; ++i) {

		// we only modify runtime variables, so the const-cast is ok here
		VICUS::GridPlane & gp = const_cast<VICUS::GridPlane &>(gridPlanes[i]);
		// if invisible, skip
		m_gridPlaneVisible[i] = gp.m_isVisible;
		if (!gp.m_isVisible) {
			// store offsets for invisible plane
			GLsizei lastOffset = (GLsizei)m_gridOffsets.back();
			m_gridOffsets.push_back(lastOffset); // start of minor grid (vertex count major grid lines 2*N*2)
			m_gridOffsets.push_back(lastOffset);  // start of next major grid (vertex count _all_ grid lines 2*N_minor*2)
			continue;
		}

		m_anyGridVisible = true;

		// generate plane transformation matrix
		Transform3D trans;
		trans.setTranslation(IBKVector2QVector(gp.m_offset));
		trans.setRotation( QQuaternion::fromAxes(IBKVector2QVector(gp.m_localX),
												 IBKVector2QVector(gp.m_localY),
												 IBKVector2QVector(gp.m_normal) ));
		m_planeTransformationMatrix[i] = trans.toMatrix();

		// transfer grid colors

		// special handling for "main grid" at index 0
		if (i==0) {
			m_gridColors[0] = QtExt::QVector3DFromQColor(s.m_majorGridColor);
			m_gridColors[1] = QtExt::QVector3DFromQColor(s.m_minorGridColor);
		}
		else {
			// major grid
			m_gridColors[i*2 + 0] = QtExt::QVector3DFromQColor(gp.m_color);
			// minor grid is always main color but a little brighter/darker depending on theme
			QColor minorGridCol;
			if (tt == SVSettings::TT_Dark)
				minorGridCol = gp.m_color.darker(200);
			else
				minorGridCol = gp.m_color.lighter(200);
			m_gridColors[i*2 + 1] = QtExt::QVector3DFromQColor(minorGridCol);
		}

		// transfer grid dimensions
		double width = gp.m_width;
		double spacing = gp.m_spacing;
		if (spacing <= 0)
			spacing = 1;

		unsigned int N = (unsigned int)(width/spacing); // number of lines to draw in x and y direction

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

		// we have ((N-1)*10 + 1) lines for the entire grid including the minor grid (10 per grid cell + 1 in either direction)
		unsigned int minorGridFraction = 10;
		unsigned int N_minor = (N-1)*minorGridFraction + 1;

		// update GridPlane data structure (needed for snapping)
		gp.m_nGridLines = N_minor;	// total number of grid lines
		gp.m_gridExtends = width/2;	// local  -x, +x, -y, +y extends of grid

		// create a temporary buffer for the grid line vertexes of current grid plane
		std::vector<float>			currentGridVertexBufferData;
		currentGridVertexBufferData.resize(N_minor*4*2);
		float * gridVertexBufferPtr = currentGridVertexBufferData.data();
		// vertex buffer data mapping:
		// - x-major grid center line 4 Vertexes                                       <-- m_gridOffsets[i*2 + 0]
		// - y-major grid center line 4 Vertexes
		// - x-major grid lines without center line (N-1) * 4 Vertexes
		// - y-major grid lines without center line (N-1) * 4 Vertexes
		// - x-minor grid lines without major grid lines (N_minor - N) * 4 Vertexes    <-- m_gridOffsets[i*2 + 1]
		// - x-minor grid lines without major grid lines (N_minor - N) * 4 Vertexes

		// we store first the major grid lines in the vertex buffer, than the minor grid lines
		// we have 2*N lines for the major grid, each line requires 2 vertexes
		// Mind: the offsets are stored using "line indexes", not vertex indexes!
		GLsizei lastOffset = (GLsizei)m_gridOffsets.back();
		m_gridOffsets.push_back(lastOffset + (GLsizei)N*4); // start of minor grid (vertex count major grid lines 2*N*2)
		// add offset for start of next grid
		m_gridOffsets.push_back(lastOffset + (GLsizei)N_minor*4);  // start of next major grid (vertex count _all_ grid lines 2*N_minor*2)

		// compute major grid lines first y = const
		float widthF = (float)width;
		float x1 = -widthF*0.5f;
		float x2 = widthF*0.5f;
		float y1 = -widthF*0.5f;
		float y2 = widthF*0.5f;

		// center lines (have special colors)

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

		// add x coordinate line (major grid)
		for (unsigned int i=0; i<N; ++i) {
			// skip main coordinate line
			if (i == N/2)
				continue;
			float y = widthF/(N-1)*i-widthF*0.5f;
			gridVertexBufferPtr[0] = x1;
			gridVertexBufferPtr[1] = y;
			gridVertexBufferPtr[2] = x2;
			gridVertexBufferPtr[3] = y;
			gridVertexBufferPtr += 4;
		}
		// add y coordinate line (major grid)
		for (unsigned int i=0; i<N; ++i) {
			// skip main coordinate line
			if (i == N/2)
				continue;
			float x = widthF/(N-1)*i-widthF*0.5f;
			gridVertexBufferPtr[0] = x;
			gridVertexBufferPtr[1] = y1;
			gridVertexBufferPtr[2] = x;
			gridVertexBufferPtr[3] = y2;
			gridVertexBufferPtr += 4;
		}

		// now add minor grid lines, hereby skip over major grid lines
		for (unsigned int i=0; i<N_minor; ++i) {
			// skip grid lines that coincide with major grid lines
			if (i % minorGridFraction == 0) continue;
			float y = widthF/(N_minor-1)*i-widthF*0.5f;
			gridVertexBufferPtr[0] = x1;
			gridVertexBufferPtr[1] = y;
			gridVertexBufferPtr[2] = x2;
			gridVertexBufferPtr[3] = y;
			gridVertexBufferPtr += 4;
		}
		for (unsigned int i=0; i<N_minor; ++i) {
			// skip grid lines that coincide with major grid lines
			if (i % minorGridFraction == 0) continue;
			float x = widthF/(N_minor-1)*i-widthF*0.5f;
			gridVertexBufferPtr[0] = x;
			gridVertexBufferPtr[1] = y1;
			gridVertexBufferPtr[2] = x;
			gridVertexBufferPtr[3] = y2;
			gridVertexBufferPtr += 4;
		}

		// now copy local grid buffer into global grid buffer
		gridVertexBufferData.insert(gridVertexBufferData.end(), currentGridVertexBufferData.begin(), currentGridVertexBufferData.end());
	}


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



void GridObject::render() {
	// grid disabled?
	if (!m_anyGridVisible)
		return;

	m_vao.bind();

	// process all planes
	unsigned int nPlanes = m_gridPlaneVisible.size();
	for (unsigned int i=0; i<nPlanes; ++i) {

		// if invisible, skip
		if (!m_gridPlaneVisible[i])
			continue;

		// get starting offset for grid
		GLsizei startOffset = m_gridOffsets[2*i];
		GLsizei minorGridOffset = m_gridOffsets[2*i+1];
		GLsizei nextOffset = m_gridOffsets[2*i+2];

		m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[4], m_planeTransformationMatrix[i]);

		// draw main x axis
		QColor xLine;
		if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
			xLine = QColor("#b30404");
		else
			xLine = QColor("#b30404");
		m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], QtExt::QVector3DFromQColor(xLine) );
		// When drawing lines, we need two indexes for one line (start and end point)
		// The first parameter is the index to start from, the second parameter is the number of indexes
		// to use for drawing. So, if you want to draw 10 lines, pass 20 as "count" argument.
		glDrawArrays(GL_LINES, startOffset, 2);
		startOffset += 2;

		// draw y axis
		QColor yLine;
		if ( SVSettings::instance().m_theme == SVSettings::TT_Dark )
			yLine = QColor("#0d8429");
		else
			yLine = QColor("#26a343");
		m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], QtExt::QVector3DFromQColor(yLine) );
		glDrawArrays(GL_LINES, startOffset, 2);
		startOffset += 2;

		// major grid
		GLsizei vertexcount = minorGridOffset - startOffset;
		m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_gridColors[2*i] );
		glDrawArrays(GL_LINES, startOffset, vertexcount);

		// minor grid
		vertexcount = nextOffset - minorGridOffset;
		m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_gridColors[2*i+1] );
		glDrawArrays(GL_LINES, minorGridOffset, vertexcount);
	}

	m_vao.release();
}


} // namespace Vic3D
