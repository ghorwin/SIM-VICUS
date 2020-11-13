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

	const unsigned int N = width/spacing; // number of lines to draw in x and y direction
	// width is in "space units", whatever that means for you (meters, km, nanometers...)
	//float width = 5000;
	// grid is centered around origin, and expands to width/2 in -x, +x, -y and +y direction

	// create a temporary buffer that will contain the x-y coordinates of all grid lines
	std::vector<float>			gridVertexBufferData;
	// we have 2*N lines, each line requires two vertexes, with two floats (x and y coordinates) each.
	m_bufferSize = 2*N*2*2;
	gridVertexBufferData.resize(m_bufferSize);
	float * gridVertexBufferPtr = gridVertexBufferData.data();
	// compute grid lines with y = const
	float x1 = -width*0.5f;
	float x2 = width*0.5f;
	for (unsigned int i=0; i<N; ++i, gridVertexBufferPtr += 4) {
		float y = width/(N-1)*i-width*0.5f;
		gridVertexBufferPtr[0] = x1;
		gridVertexBufferPtr[1] = y;
		gridVertexBufferPtr[2] = x2;
		gridVertexBufferPtr[3] = y;
	}
	// compute grid lines with x = const
	float y1 = -width*0.5f;
	float y2 = width*0.5f;
	for (unsigned int i=0; i<N; ++i, gridVertexBufferPtr += 4) {
		float x = width/(N-1)*i-width*0.5f;
		gridVertexBufferPtr[0] = x;
		gridVertexBufferPtr[1] = y1;
		gridVertexBufferPtr[2] = x;
		gridVertexBufferPtr[3] = y2;
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
	unsigned long vertexMemSize = m_bufferSize*sizeof(float);
	m_vbo.allocate(gridVertexBufferData.data(), vertexMemSize);


	// adjust m_bufferSize and m_minorGridStart
	m_minorGridBufferStart = m_bufferSize-500;

	m_vao.release(); // Mind: always release VAO before index buffer
	m_vbo.release();
}


void GridObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
}


void GridObject::render() {
	// grid disabled?
	if (m_bufferSize == 0)
		return;

	m_vao.bind();

	// draw major grid lines
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_majorGridColor);
	glDrawArrays(GL_LINES, 0, m_minorGridBufferStart);
	if (m_minorGridBufferStart == m_bufferSize)
		return; // minor grid disabled
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_minorGridColor);
	glDrawArrays(GL_LINES, m_minorGridBufferStart, m_bufferSize-m_minorGridBufferStart);

	// draw first major grid lines, then minor grid lines with different color
	m_vao.release();
}

} // namespace Vic3D
