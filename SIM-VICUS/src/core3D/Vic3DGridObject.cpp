/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

#include "Vic3DGridObject.h"

#include <QOpenGLShaderProgram>

#include <vector>

#include <VICUS_Project.h>
#include "SVProjectHandler.h"
#include "Vic3DShaderProgram.h"

namespace Vic3D {

void GridObject::create(ShaderProgram * shaderProgram) {
	m_gridShader = shaderProgram;

	// get grid dimensions from project
	const VICUS::Project & prj = project();

	// transfer color
	QColor gridColor(prj.m_viewSettings.m_gridColor);
	m_minorGridColor = QVector3D((float)gridColor.redF(), (float)gridColor.greenF(), (float)gridColor.blueF());
	gridColor = Qt::white; // gridColor.light();
	m_majorGridColor = QVector3D((float)gridColor.redF(), (float)gridColor.greenF(), (float)gridColor.blueF());

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

	// each vertex requires two floats (x and y coordinates)
	gridVertexBufferData.resize(m_vertexCount*2);
	float * gridVertexBufferPtr = gridVertexBufferData.data();

	// compute major grid lines first y = const
	float x1 = -width*0.5f;
	float x2 = width*0.5f;
	float y1 = -width*0.5f;
	float y2 = width*0.5f;

	// x-axis
	gridVertexBufferPtr[0] = 0;
	gridVertexBufferPtr[1] = y1;
	gridVertexBufferPtr[2] = 0;
	gridVertexBufferPtr[3] = y2;
	gridVertexBufferPtr += 4;

	// y-axis
	gridVertexBufferPtr[0] = x1;
	gridVertexBufferPtr[1] = 0;
	gridVertexBufferPtr[2] = x2;
	gridVertexBufferPtr[3] = 0;
	gridVertexBufferPtr += 4;

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


void GridObject::render() {
	// grid disabled?
	if (m_vertexCount == 0)
		return;

	m_vao.bind();

	// draw major grid lines
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_majorGridColor);

	// When drawing lines, we need two indexes for one line (start and end point)
	// The first parameter is the index to start from, the second parameter is the number of indexes
	// to use for drawing. So, if you want to draw 10 lines, pass 20 as "count" argument.
	//
	glDrawArrays(GL_LINES, 0, m_minorGridStartVertex);
	if (m_minorGridStartVertex == m_vertexCount)
		return; // minor grid disabled
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_minorGridColor);
	glDrawArrays(GL_LINES, m_minorGridStartVertex, m_vertexCount-m_minorGridStartVertex);

	// draw first major grid lines, then minor grid lines with different color
	m_vao.release();
}

} // namespace Vic3D
