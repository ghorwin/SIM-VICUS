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

#include "Vic3DMeasurementObject.h"

#include <QOpenGLShaderProgram>

#include <vector>

#include <VICUS_Project.h>
#include "SVProjectHandler.h"
#include "Vic3DShaderProgram.h"
#include "Vic3DVertex.h"
#include "Vic3DSceneView.h"
#include "Vic3DScene.h"

#include "SVSettings.h"
#include "SVGeometryView.h"

namespace Vic3D {

void MeasurementObject::create(ShaderProgram * shaderProgram) {
	m_measurementShader = shaderProgram;
}


void MeasurementObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
}

void MeasurementObject::reset() {
	m_startPoint = QVector3D();
	m_endPoint = QVector3D();

	destroy();

	m_vertexCount = 0;
}

void MeasurementObject::setMeasureLine(const QVector3D & end, const QVector3D & cameraForward) {
	// create a temporary buffer that will contain the x-y coordinates of all grid lines
	std::vector<VertexC>			measurementVertexBufferData;

	Q_ASSERT(m_startPoint != QVector3D() ); // start point always has to be set

	// start point
	measurementVertexBufferData.push_back(VertexC(m_startPoint));
	// end point
	measurementVertexBufferData.push_back(VertexC(end));

	QVector3D uprightVec = QVector3D::crossProduct(cameraForward, end-m_startPoint).normalized();

	QVector3D startLine1 = m_startPoint + 0.5 * uprightVec;
	QVector3D endLine1 = m_startPoint - 0.5 * uprightVec;

	QVector3D startLine2 = end + 0.5 * uprightVec;
	QVector3D endLine2 = end - 0.5 * uprightVec;

	measurementVertexBufferData.push_back(VertexC(startLine1));
	measurementVertexBufferData.push_back(VertexC(endLine1));

	measurementVertexBufferData.push_back(VertexC(startLine2));
	measurementVertexBufferData.push_back(VertexC(endLine2));

	m_vertexCount = measurementVertexBufferData.size();

	// Create Vertex Array Object and buffers if not done, yet
	if (!m_vao.isCreated()) {
		m_vao.create();		// create Vertex Array Object
		m_vao.bind();		// and bind it

		// Create Vertex Buffer Object (to store stuff in GPU memory)
		m_vbo.create();
		m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_vbo.bind();

		// layout(location = 0) = vec3 vertex coordinates
		m_measurementShader->shaderProgram()->enableAttributeArray(0); // array with index/id 0
		m_measurementShader->shaderProgram()->setAttributeBuffer(0, GL_FLOAT, 0, 3 /* vec3 */, sizeof(VertexC));
	}

	m_vbo.bind();
	m_vbo.allocate(measurementVertexBufferData.data(), measurementVertexBufferData.size()*sizeof(VertexC));

	m_vao.release(); // Mind: always release VAO before index buffer
	m_vbo.release();
}


void MeasurementObject::render() {
	// grid disabled?
	if (m_vertexCount == 0)
		return;

	m_vao.bind();

	// draw lines
	QColor measurementLineColor = QColor("#E30513");
	QVector4D col(measurementLineColor.redF(), measurementLineColor.greenF(), measurementLineColor.blueF(), 1.0);
	m_measurementShader->shaderProgram()->setUniformValue(m_measurementShader->m_uniformIDs[1], col);
	glDrawArrays(GL_LINES, 0, m_vertexCount);
	m_vao.release();
}


} // namespace Vic3D
