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

#include "Vic3DRubberbandObject.h"

#include <QOpenGLShaderProgram>

#include "Vic3DShaderProgram.h"
#include "Vic3DVertex.h"
#include "Vic3DGeometryHelpers.h"
#include <SVConversions.h>

#include "SVViewStateHandler.h"
#include "SVPropEditGeometry.h"
#include "SVViewStateHandler.h"
#include "SVSettings.h"

namespace Vic3D {

RubberbandObject::RubberbandObject() {

}


void RubberbandObject::create(ShaderProgram * rubberbandShaderProgram) {
	m_rubberbandShaderProgram = rubberbandShaderProgram;
}


void RubberbandObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
}

void RubberbandObject::reset() {
	m_topLeft = QVector3D();

	destroy();
}


void RubberbandObject::render() {
	// grid disabled?
	if (m_vertexCount == 0)
		return;

	// bind vertex array
	m_vao.bind();

	// draw lines
	QVector4D col(1.0, 0.0, 0.0, 1.0);
	m_rubberbandShaderProgram->shaderProgram()->setUniformValue(m_rubberbandShaderProgram->m_uniformIDs[1], col);
	glDrawArrays(GL_LINES, 0, m_vertexCount);

	m_vao.release();
}

void RubberbandObject::setStartPoint(const QVector3D &topLeft) {
	m_topLeft = topLeft;
}

void RubberbandObject::setRubberband(const QVector3D &bottomRight) {
	// create a temporary buffer that will contain the x-y coordinates of all grid lines
	std::vector<VertexC>	rubberbandVertexBufferData;

	qDebug() << "x: " << bottomRight.x() << " |  y: " << bottomRight.y();

	QVector3D topLeftMoved = QVector3D(SVSettings::instance().m_ratio * m_topLeft.x() + -m_viewport.width()/2,
									   -m_topLeft.y() + m_viewport.height()/2, 0);
	QVector3D bottomRightMoved = QVector3D(SVSettings::instance().m_ratio *bottomRight.x() - m_viewport.width()/2,
										   -SVSettings::instance().m_ratio *bottomRight.y() + m_viewport.height()/2, 0);

	// if(m_topLeft.x() > bottomRight.x())

	// line 1
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(topLeftMoved)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(topLeftMoved.x(), bottomRightMoved.y(), 0)));

	rubberbandVertexBufferData.push_back(VertexC(QVector3D(topLeftMoved.x()+1, topLeftMoved.y(), 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(topLeftMoved.x()+1, bottomRightMoved.y(), 0)));

	// Line 2
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(topLeftMoved.x(), bottomRightMoved.y(), 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(bottomRightMoved)));

	rubberbandVertexBufferData.push_back(VertexC(QVector3D(topLeftMoved.x(), bottomRightMoved.y()-1, 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(bottomRightMoved.x(), bottomRightMoved.y()-1, 0)));

	// Line 3
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(bottomRightMoved)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(bottomRightMoved.x(), topLeftMoved.y(), 0)));

	rubberbandVertexBufferData.push_back(VertexC(QVector3D(bottomRightMoved.x()-1, bottomRightMoved.y(), 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(bottomRightMoved.x()-1, topLeftMoved.y(), 0)));


	// Line 4
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(bottomRightMoved.x(), topLeftMoved.y(), 0)));
	rubberbandVertexBufferData.push_back(VertexC(topLeftMoved));

	rubberbandVertexBufferData.push_back(VertexC(QVector3D(bottomRightMoved.x(), topLeftMoved.y()+1, 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(topLeftMoved.x(), topLeftMoved.y()+1, 0)));



	m_vertexCount = rubberbandVertexBufferData.size();

	// Create Vertex Array Object and buffers if not done, yet
	if (!m_vao.isCreated()) {
		m_vao.create();		// create Vertex Array Object
		m_vao.bind();		// and bind it

		// Create Vertex Buffer Object (to store stuff in GPU memory)
		m_vbo.create();
		m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_vbo.bind();

		// layout(location = 0) = vec3 vertex coordinates
		m_rubberbandShaderProgram->shaderProgram()->enableAttributeArray(0); // array with index/id 0
		m_rubberbandShaderProgram->shaderProgram()->setAttributeBuffer(0, GL_FLOAT, 0, 3 /* vec3 */, sizeof(VertexC));
	}

	m_vbo.bind();
	m_vbo.allocate(rubberbandVertexBufferData.data(), rubberbandVertexBufferData.size()*sizeof(VertexC));

	m_vao.release(); // Mind: always release VAO before index buffer
	m_vbo.release();
}

void RubberbandObject::setViewport(const QRect & viewport) {
	m_viewport = viewport;
}


} // namespace Vic3D
