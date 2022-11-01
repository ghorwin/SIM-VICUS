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
#include "Vic3DScene.h"

#include "SVConversions.h"
#include "SVViewStateHandler.h"
#include "SVPropEditGeometry.h"
#include "SVViewStateHandler.h"
#include "SVSettings.h"
#include "SVConversions.h"
#include "SVProjectHandler.h"
#include "SVUndoTreeNodeState.h"

#include <IBKMK_3DCalculations.h>
#include <IBKMK_2DCalculations.h>

namespace Vic3D {

RubberbandObject::RubberbandObject() {

}


void RubberbandObject::create(Scene * scene, ShaderProgram * rubberbandShaderProgram) {
	m_scene = scene;
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

	// define line stipple
	GLint factor = 1;
	GLushort pattern = 0xAAAA;

	// draw lines
	QVector4D col;

	// Do the coloring
	if(m_selectGeometry)
		col = QVector4D(0.0, 1.0, 0.0, 1.0);
	else
		col = QVector4D(1.0, 0.0, 0.0, 1.0);

	// if geometry is touched set alpha to 0.5
	if(m_touchGeometry) {
		glEnable(GL_BLEND);
		col.setW(0.5);
	}

	m_rubberbandShaderProgram->shaderProgram()->setUniformValue(m_rubberbandShaderProgram->m_uniformIDs[1], col);
	//glLineStipple(factor, pattern);
	glDrawArrays(GL_LINES, 0, m_vertexCount);

	// if geometry is touched set alpha to 0.l5
	if(m_touchGeometry)
		glDisable(GL_BLEND);

	m_vao.release();
}

void RubberbandObject::setStartPoint(const QVector3D &topLeft) {
	m_topLeft = topLeft;
}

void RubberbandObject::setRubberband(const QVector3D &bottomRight) {
	// create a temporary buffer that will contain the x-y coordinates of all grid lines
	std::vector<VertexC>	rubberbandVertexBufferData;

	qDebug() << "x: " << bottomRight.x() << " |  y: " << bottomRight.y();

	m_topLeftView = QVector3D(SVSettings::instance().m_ratio * m_topLeft.x() + -m_viewport.width()/2,
									   -SVSettings::instance().m_ratio *m_topLeft.y() + m_viewport.height()/2, 0);
	m_bottomRightView = QVector3D(SVSettings::instance().m_ratio *bottomRight.x() - m_viewport.width()/2,
										   -SVSettings::instance().m_ratio *bottomRight.y() + m_viewport.height()/2, 0);

	if(m_topLeft.x() < bottomRight.x()) // we select
		m_selectGeometry = true;
	else
		m_selectGeometry = false;

	if(m_topLeft.y() > bottomRight.y()) // select everything
		m_touchGeometry = false;
	else
		m_touchGeometry = true;

	// NOTE: We draw 2 lines to make them thicker.

	// line 1
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_topLeftView)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_topLeftView.x(), m_bottomRightView.y(), 0)));

	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_topLeftView.x()+1, m_topLeftView.y(), 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_topLeftView.x()+1, m_bottomRightView.y(), 0)));

	// Line 2
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_topLeftView.x(), m_bottomRightView.y(), 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_bottomRightView)));

	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_topLeftView.x(), m_bottomRightView.y()-1, 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_bottomRightView.x(), m_bottomRightView.y()-1, 0)));

	// Line 3
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_bottomRightView)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_bottomRightView.x(), m_topLeftView.y(), 0)));

	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_bottomRightView.x()-1, m_bottomRightView.y(), 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_bottomRightView.x()-1, m_topLeftView.y(), 0)));

	// Line 4
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_bottomRightView.x(), m_topLeftView.y(), 0)));
	rubberbandVertexBufferData.push_back(VertexC(m_topLeftView));

	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_bottomRightView.x(), m_topLeftView.y()+1, 0)));
	rubberbandVertexBufferData.push_back(VertexC(QVector3D(m_topLeftView.x(), m_topLeftView.y()+1, 0)));

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

QVector3D RubberbandObject::convertViewportToWorldCoordinates(int x, int y) {

	// viewport dimensions
	double Dx = m_viewport.width();
	double Dy = m_viewport.height();

	double nx = (2*x)/Dx;
	double ny = -(2*y)/Dy;

	// invert world2view matrix, with m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
	bool invertible;
	QMatrix4x4 projectionMatrixInverted = m_scene->worldToView().inverted(&invertible);
	if (!invertible) {
		qWarning()<< "Cannot invert projection matrix.";
		return QVector3D();
	}

	// mouse position in NDC space, one point on near plane and one point on far plane
	QVector4D nearPos(float(nx), float(ny), -1, 1.0);

	// transform from NDC to model coordinates
	QVector4D nearResult = projectionMatrixInverted*nearPos;
	// don't forget normalization!
	nearResult /= nearResult.w();

	return nearResult.toVector3D();
}

void RubberbandObject::setViewport(const QRect & viewport) {
	m_viewport = viewport;
}

void RubberbandObject::selectObjectsBasedOnRubberband() {
	const Camera &c = m_scene->camera();

	const QVector3D &forward = c.forward();
	const QVector3D &right = c.right(); //
	const QVector3D &up = c.up(); //

	VICUS::Project prj = SVProjectHandler::instance().project();

	// Camera pane

	// 1) Construct polygon in scene. ================

	std::vector<IBKMK::Vector2D> vertexes;

	QVector3D topLeft = convertViewportToWorldCoordinates(m_topLeftView.x(), m_topLeftView.y());
	QVector3D topRight = convertViewportToWorldCoordinates(m_bottomRightView.x(), m_topLeftView.y());
	QVector3D bottomRight = convertViewportToWorldCoordinates(m_bottomRightView.x(), m_bottomRightView.y());
	QVector3D bottomLeft = convertViewportToWorldCoordinates(m_topLeftView.x(), m_bottomRightView.y());

	IBKMK::Vector2D p1,p2,p3,p4;
	IBKMK::planeCoordinates(QVector2IBKVector(c.translation()), QVector2IBKVector(right),
							QVector2IBKVector(up), QVector2IBKVector(topLeft), p1.m_x, p1.m_y);
	vertexes.push_back(p1);
	IBKMK::planeCoordinates(QVector2IBKVector(c.translation()), QVector2IBKVector(right),
							QVector2IBKVector(up), QVector2IBKVector(topRight), p2.m_x, p2.m_y);
	vertexes.push_back(p2);
	IBKMK::planeCoordinates(QVector2IBKVector(c.translation()), QVector2IBKVector(right),
							QVector2IBKVector(up), QVector2IBKVector(bottomRight), p3.m_x, p3.m_y);
	vertexes.push_back(p3);
	IBKMK::planeCoordinates(QVector2IBKVector(c.translation()), QVector2IBKVector(right),
							QVector2IBKVector(up), QVector2IBKVector(bottomLeft), p4.m_x, p4.m_y);
	vertexes.push_back(p4);

	// ============================

	// get a list of IDs of nodes to be selected (only those who are not yet selected)
	std::set<unsigned int> nodeIDs;

	// 2) ========================
	for(const VICUS::Building &b : prj.m_buildings)
		for(const VICUS::BuildingLevel &bl : b.m_buildingLevels)
			for(const VICUS::Room &r : bl.m_rooms)
				for(const VICUS::Surface &s : r.m_surfaces) {
					bool foundAnyPoint = false;
					bool foundAllPoints = false;
					int selectionCount = 0;
					for(const IBKMK::Vector3D &v3D : s.polygon3D().vertexes()) {
						IBKMK::Vector3D projectedP;

						// Project point in camera pane
						IBKMK::pointProjectedOnPlane(QVector2IBKVector(c.translation()), QVector2IBKVector(forward), v3D, projectedP);

						// Check if point lies inside polygon
						IBKMK::Vector2D p;
						IBKMK::planeCoordinates(QVector2IBKVector(c.translation()), QVector2IBKVector(right),
												QVector2IBKVector(up), projectedP, p.m_x, p.m_y);

						int res =  IBKMK::pointInPolygon(vertexes, p);

						if(res == -1)
							continue;

						foundAnyPoint = true;
						++selectionCount;
					}
					foundAllPoints = selectionCount == s.polygon3D().vertexes().size();

					if(!foundAnyPoint)
						continue;

					if(m_touchGeometry || foundAllPoints)
						nodeIDs.insert(s.m_id);
				}

	// UNDO ACTION ============================

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Select objects"),
														 SVUndoTreeNodeState::SelectedState, nodeIDs, true);
	// select all
	undo->push();

}


} // namespace Vic3D
