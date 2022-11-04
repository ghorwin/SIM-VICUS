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

#include <clipper.hpp>

namespace Vic3D {

float SCALE_FACTOR = 1e4;

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

ClipperLib::IntPoint RubberbandObject::toClipperIntPoint(const QVector3D & p) {
	return ClipperLib::IntPoint(int(Vic3D::SCALE_FACTOR*p.x() ), int(SCALE_FACTOR*p.y() ) );
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

	// Construct a polygon in NDC
	int w = m_viewport.width();
	int h = m_viewport.height();

	ClipperLib::Path pathRubberband;
	pathRubberband << toClipperIntPoint(QVector3D(2*m_topLeftView.x()/w, 2*m_topLeftView.y()/h, 0));
	pathRubberband << toClipperIntPoint(QVector3D(2*m_topLeftView.x()/w, 2*m_bottomRightView.y()/h, 0));
	pathRubberband << toClipperIntPoint(QVector3D(2*m_bottomRightView.x()/w, 2*m_bottomRightView.y()/h, 0));
	pathRubberband << toClipperIntPoint(QVector3D(2*m_bottomRightView.x()/w, 2*m_topLeftView.y()/h, 0));

	ClipperLib::Clipper clp;
	// clp.AddPaths(otherPoly, ClipperLib::ptClip, true);

	// Add main polygon to clipper
	clp.AddPath(pathRubberband, ClipperLib::ptSubject, true);

	// ============================
//	for (IBKMK::Vector2D v2D : vertexes)
// 		qDebug() << "x: " << v2D.m_x << " | y: " << v2D.m_y;


	// get a list of IDs of nodes to be selected (only those who are not yet selected)
	std::set<unsigned int> nodeIDs;
	const QMatrix4x4 &mat = m_scene->worldToView();

	// 2) ========================
	for(const VICUS::Building &b : prj.m_buildings) {
		for(const VICUS::BuildingLevel &bl : b.m_buildingLevels) {
			for(const VICUS::Room &r : bl.m_rooms) {
				for(const VICUS::Surface &s : r.m_surfaces) {

					qDebug() << "------------------";
					qDebug() << s.m_displayName;

					bool foundAnyPoint = false;
					bool foundAllPoints = false;
					int selectionCount = 0;
					ClipperLib::Path pathSurface;
					for(const IBKMK::Vector3D &v3D : s.polygon3D().vertexes()) {
						// Bring our point to NDC
						QVector4D projectedP = mat * QVector4D(v3D.m_x, v3D.m_y, v3D.m_z, 1.0);
						projectedP /= projectedP.w();

						// Convert to ClipperLib
						pathSurface << toClipperIntPoint(projectedP.toVector3D() );
					}

					clp.AddPath(pathSurface, ClipperLib::ptClip, true);
					ClipperLib::Paths intersectionPaths;
					clp.Execute(ClipperLib::ctIntersection, intersectionPaths, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

					if(intersectionPaths.empty())
						continue;

					double intersectionArea = ClipperLib::Area(intersectionPaths[0]);
					double surfaceArea = ClipperLib::Area(pathSurface);


					if( (m_touchGeometry && intersectionArea>0.0) || IBK::near_equal(surfaceArea, intersectionArea) )
						nodeIDs.insert(s.m_id);



					const IBKMK::Vector3D &offset = s.geometry().offset();
					const IBKMK::Vector3D &localX = s.geometry().localX();
					const IBKMK::Vector3D &localY = s.geometry().localX();

					// Also check windows
					for(const VICUS::SubSurface &ss : s.subSurfaces()) {

						qDebug() << "------------------";
						qDebug() << ss.m_displayName;
						foundAnyPoint = false;
						foundAllPoints = false;
						selectionCount = 0;

						ClipperLib::Path pathSubSurface;

						for(const IBKMK::Vector2D v2D : ss.m_polygon2D.vertexes()) {
							IBKMK::Vector3D v3D = offset + localX * v2D.m_x + localY * v2D.m_y;
							QVector4D projectedP = mat * QVector4D(v3D.m_x, v3D.m_y, v3D.m_z, 1.0);
							projectedP /= projectedP.w();

							qDebug() << s.m_displayName <<"-> x: " << projectedP.x() << " | y: " << projectedP.y() << " | z: " << projectedP.z();

							// Convert to ClipperLib
							pathSubSurface << toClipperIntPoint(projectedP.toVector3D() );
						}

						clp.AddPath(pathSubSurface, ClipperLib::ptClip, true);
						ClipperLib::Paths intersectionPaths;
						clp.Execute(ClipperLib::ctIntersection, intersectionPaths, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

						if(intersectionPaths.empty())
							continue;

						ClipperLib::Path &path = intersectionPaths[0];

						double intersectionArea = ClipperLib::Area(path);
						double surfaceArea = ClipperLib::Area(pathSubSurface);

						if( (m_touchGeometry && intersectionArea>0.0) || IBK::near_equal(surfaceArea, intersectionArea) )
							nodeIDs.insert(ss.m_id);

					}
				}
			}
		}
	}

	// UNDO ACTION ============================

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Select objects"),
														 SVUndoTreeNodeState::SelectedState, nodeIDs, m_selectGeometry);
	// select all
	undo->push();

}


} // namespace Vic3D
