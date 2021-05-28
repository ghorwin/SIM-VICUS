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

#include "Vic3DOrbitControllerObject.h"

#include <cmath>

#include <QOpenGLShaderProgram>

#include "Vic3DShaderProgram.h"
#include "Vic3DVertex.h"
#include "SVSettings.h"

namespace Vic3D {

void OrbitControllerObject::create(ShaderProgram * shaderProgram) {
	m_shaderProgram = shaderProgram;

	// create a temporary buffer that will contain the coordinates of all lines segments
	std::vector<VertexC>			lineVertexBuffer;

	// rotation axis is z-axis
	lineVertexBuffer.resize(2);
	lineVertexBuffer[0] = QVector3D(0,0,-5.f);
	lineVertexBuffer[1] = QVector3D(0,0,+5.f);
	// create a circle with line segments
	const unsigned int N_SEGMENTS = 32;
	const double RADIUS = 2;
#define PI_CONST 3.14159265
	for (unsigned int i=0; i<=N_SEGMENTS; ++i) {
		double angle = 2*PI_CONST*((i+1) % N_SEGMENTS)/N_SEGMENTS;
		double x = std::sin(angle)*RADIUS;
		double y = std::cos(angle)*RADIUS;
		lineVertexBuffer.push_back( QVector3D(float(x),float(y),0) );
	}

	m_vertexCount = lineVertexBuffer.size();

	// Create Vertex Array Object and buffers if not done, yet
	if (!m_vao.isCreated()) {
		m_vao.create();		// create Vertex Array Object
		m_vao.bind();		// and bind it

		// Create Vertex Buffer Object (to store stuff in GPU memory)
		m_vbo.create();
		m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_vbo.bind();

		// layout(location = 0) = vec2 position
		m_shaderProgram->shaderProgram()->enableAttributeArray(0); // array with index/id 0
		m_shaderProgram->shaderProgram()->setAttributeBuffer(0, GL_FLOAT,
									  0 /* position/vertex offset */,
									  3 /* three coords per position = vec3 */,
									  sizeof(VertexC) /* vertex after vertex, no interleaving */);
	}

	m_vbo.bind();
	unsigned long vertexMemSize = lineVertexBuffer.size()*sizeof(VertexC);
	m_vbo.allocate(lineVertexBuffer.data(), vertexMemSize);

	m_vao.release(); // Mind: always release VAO before index buffer
	m_vbo.release();
}


void OrbitControllerObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
}


void OrbitControllerObject::render() {
	m_vao.bind();

	// set transformation matrix
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[1], m_transform.toMatrix());
	// line color
	if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], QVector4D(1.f, 1.f, .8f, 1.f));
	else
		m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], QVector4D(0.3f, 0.3f, 0.4f, 1.f));
	glDrawArrays(GL_LINES, 0, 2); // rotation axis
	// line strip making up the circle
	glDrawArrays(GL_LINE_STRIP, 2, m_vertexCount-2);

	m_vao.release();
}

} // namespace Vic3D
