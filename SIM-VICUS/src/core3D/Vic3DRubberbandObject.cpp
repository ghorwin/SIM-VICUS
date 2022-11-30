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

float SCALE_FACTOR = 1e8;

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

	// draw lines
	QVector4D col;

	// Do the coloring
	if(m_selectGeometry)
		col = QVector4D(0.219607843, 0.839215686, 0.219607843, 1.0);
	else
		col = QVector4D(0.960784314, 0.258823529, 0.258823529, 1.0);

	float dash = 0.0;
	float gap = 0.0;
	// if geometry is touched set alpha to 0.5
	if(m_touchGeometry) {
		glEnable(GL_BLEND);
		//col.setW(0.5);
		dash = 20.0;
		gap = 15.0;
	}

	/*
	dashedLines.m_uniformNames.append("worldToView");
	dashedLines.m_uniformNames.append("resolution");
	dashedLines.m_uniformNames.append("dashSize");
	dashedLines.m_uniformNames.append("gapSize");
	dashedLines.m_uniformNames.append("fixedColor");
	 */

	m_rubberbandShaderProgram->shaderProgram()->setUniformValue(m_rubberbandShaderProgram->m_uniformIDs[1], QVector2D((float)m_viewport.width(), (float)m_viewport.height()));
	m_rubberbandShaderProgram->shaderProgram()->setUniformValue(m_rubberbandShaderProgram->m_uniformIDs[2], dash);
	m_rubberbandShaderProgram->shaderProgram()->setUniformValue(m_rubberbandShaderProgram->m_uniformIDs[3], gap);
	m_rubberbandShaderProgram->shaderProgram()->setUniformValue(m_rubberbandShaderProgram->m_uniformIDs[4], col);
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

	// qDebug() << "x: " << bottomRight.x() << " |  y: " << bottomRight.y();

	m_topLeftView = QVector3D(SVSettings::instance().m_ratio * m_topLeft.x() + -m_viewport.width()/2,
							  -SVSettings::instance().m_ratio *m_topLeft.y() + m_viewport.height()/2, 0);
	m_bottomRightView = QVector3D(SVSettings::instance().m_ratio *bottomRight.x() - m_viewport.width()/2,
								  -SVSettings::instance().m_ratio *bottomRight.y() + m_viewport.height()/2, 0);

	if(m_topLeft.x() < bottomRight.x()) // we select
		m_touchGeometry = true;
	else
		m_touchGeometry = false;

	if(m_topLeft.y() > bottomRight.y()) // select everything
		m_selectGeometry = false;
	else
		m_selectGeometry = true;

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

	m_vertexCount = int(rubberbandVertexBufferData.size());

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
	m_vbo.allocate(rubberbandVertexBufferData.data(), int(rubberbandVertexBufferData.size())*sizeof(VertexC));

	m_vao.release(); // Mind: always release VAO before index buffer
	m_vbo.release();
}

ClipperLib::IntPoint RubberbandObject::toClipperIntPoint(const QVector3D & p) {
	return ClipperLib::IntPoint(int(Vic3D::SCALE_FACTOR*p.x() ), int(SCALE_FACTOR*p.y() ) );
}

void RubberbandObject::setViewport(const QRect & viewport) {
	m_viewport = viewport;
}

bool RubberbandObject::surfaceIntersectionClippingAreaWithRubberband(const QMatrix4x4 &mat, const VICUS::Surface &surf,
																	 const ClipperLib::Path &pathRubberband,
																	 double &intersectionArea, double &surfaceArea) {
	ClipperLib::Clipper clp;
	// clp.AddPaths(otherPoly, ClipperLib::ptClip, true);

	// Add main polygon to clipper
	clp.AddPath(pathRubberband, ClipperLib::ptSubject, true);

	ClipperLib::Path pathSurface;
	try {
		for(const IBKMK::Vector3D &v3D : surf.polygon3D().vertexes()) {
			// Bring our point to NDC
			QVector4D projectedP = mat * QVector4D(v3D.m_x, v3D.m_y, v3D.m_z, 1.0);
			projectedP /= projectedP.w();

			// qDebug() << s.m_displayName << " Surface point -> x: " << projectedP.x() << " | y: " << projectedP.y() << " | z: " << projectedP.z();

			// Convert to ClipperLib
			pathSurface << toClipperIntPoint(projectedP.toVector3D() );
		}
	} catch(...) {
		return false; // Skip broken polygons
	}

	clp.AddPath(pathSurface, ClipperLib::ptClip, true);
	ClipperLib::Paths intersectionPaths;
	clp.Execute(ClipperLib::ctIntersection, intersectionPaths, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

	if(intersectionPaths.empty()) {
		// qDebug() << "Skipped";
		return false;
	}

	for(ClipperLib::Path &path : intersectionPaths)
		intersectionArea += std::abs(ClipperLib::Area(path));

	surfaceArea = std::abs(ClipperLib::Area(pathSurface));

	return true;
}

void RubberbandObject::selectObjectsBasedOnRubberband() {

	VICUS::Project prj = SVProjectHandler::instance().project();

	// Construct a polygon in NDC
	int w = m_viewport.width();
	int h = m_viewport.height();

	// Path of Rubberband in NDC as CLipper Path
	ClipperLib::Path pathRubberband;
	pathRubberband << toClipperIntPoint(QVector3D(2*m_topLeftView.x()/w, 2*m_topLeftView.y()/h, 0));
	pathRubberband << toClipperIntPoint(QVector3D(2*m_topLeftView.x()/w, 2*m_bottomRightView.y()/h, 0));
	pathRubberband << toClipperIntPoint(QVector3D(2*m_bottomRightView.x()/w, 2*m_bottomRightView.y()/h, 0));
	pathRubberband << toClipperIntPoint(QVector3D(2*m_bottomRightView.x()/w, 2*m_topLeftView.y()/h, 0));

	std::vector<IBKMK::Vector2D> verts2D;
	for(const ClipperLib::IntPoint &ip : pathRubberband)
		verts2D.push_back(IBKMK::Vector2D(ip.X/SCALE_FACTOR, ip.Y/SCALE_FACTOR)); // TODO Improve
	IBKMK::Polygon2D poly(verts2D);

	// get a list of IDs of nodes to be selected (only those who are not yet selected)
	std::set<unsigned int> nodeIDs;

	// Store world to view matrix.
	const QMatrix4x4 &mat = m_scene->worldToView();

	// Find all de-/selected Geometry of Buildings
	for(const VICUS::Building &b : prj.m_buildings) {
		for(const VICUS::BuildingLevel &bl : b.m_buildingLevels) {
			for(const VICUS::Room &r : bl.m_rooms) {

				// We want to also select the room, when all surfaces are selected
				unsigned int surfaceSelectionCount = 0;

				for(const VICUS::Surface &s : r.m_surfaces) {

					// qDebug() << "------------------";
					// qDebug() << s.m_displayName;
					double intersectionArea = 0.0;
					double surfaceArea = 0.0;
					if(!surfaceIntersectionClippingAreaWithRubberband(mat, s, pathRubberband, intersectionArea, surfaceArea))
						continue;

					// qDebug() << "Intersection Area: " << intersectionArea << " | Surface Area: " << surfaceArea;
					if( (m_touchGeometry && intersectionArea>0.0) || IBK::near_equal(surfaceArea, intersectionArea) ) {
						++surfaceSelectionCount;
						nodeIDs.insert(s.m_id);
					}

					const IBKMK::Vector3D &offset = s.geometry().offset();
					const IBKMK::Vector3D &localX = s.geometry().localX();
					const IBKMK::Vector3D &localY = s.geometry().localY();

					// Also check windows
					for(const VICUS::SubSurface &ss : s.subSurfaces()) {

						// qDebug() << "------------------";
						// qDebug() << ss.m_displayName;

						ClipperLib::Clipper clp;
						// clp.AddPaths(otherPoly, ClipperLib::ptClip, true);

						// Add main polygon to clipper
						clp.AddPath(pathRubberband, ClipperLib::ptSubject, true);


						ClipperLib::Path pathSubSurface;

						for(const IBKMK::Vector2D v2D : ss.m_polygon2D.vertexes()) {
							IBKMK::Vector3D v3D = offset + localX * v2D.m_x + localY * v2D.m_y;
							QVector4D projectedP = mat * QVector4D(v3D.m_x, v3D.m_y, v3D.m_z, 1.0);
							projectedP /= projectedP.w();

							// qDebug() << ss.m_displayName << " Sub surface point -> x: " << v3D.m_x << " | " << projectedP.x() << " | y: " << v3D.m_y << " | " << projectedP.y() << " | z: " << v3D.m_z << " | " << projectedP.z();

							// Convert to ClipperLib
							pathSubSurface << toClipperIntPoint(projectedP.toVector3D() );
						}

						clp.AddPath(pathSubSurface, ClipperLib::ptClip, true);
						ClipperLib::Paths intersectionPaths;
						clp.Execute(ClipperLib::ctIntersection, intersectionPaths, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

						if(intersectionPaths.empty()) {
							// qDebug() << "Skipped";
							continue;
						}

						ClipperLib::Path &path = intersectionPaths[0];

						double intersectionArea = std::abs(ClipperLib::Area(path));
						double surfaceArea = std::abs(ClipperLib::Area(pathSubSurface));

						// qDebug() << "Intersection Area: " << intersectionArea << " | Surface Area: " << surfaceArea;

						if( (m_touchGeometry && intersectionArea>0.0) || IBK::near_equal(surfaceArea, intersectionArea) )
							nodeIDs.insert(ss.m_id);

					}
				}

				if(surfaceSelectionCount == r.m_surfaces.size())
					nodeIDs.insert(r.m_id);
			}
		}
	}

	const Vic3D::Camera &c = m_scene->camera();

	// TODO: Select Network Geometry.
	for(const VICUS::Network &n : prj.m_geometricNetworks) {
		for(const VICUS::NetworkEdge &ne : n.m_edges) {
			const IBKMK::Vector3D &p1 = ne.m_node1->m_position;
			const IBKMK::Vector3D &p2 = ne.m_node2->m_position;

			// project onto NDC
			QVector4D projectedP1 = mat * QVector4D(p1.m_x, p1.m_y, p1.m_z, 1.0);
			projectedP1 /= projectedP1.w();

			// project onto NDC
			QVector4D projectedP2 = mat * QVector4D(p2.m_x, p2.m_y, p2.m_z, 1.0);
			projectedP2 /= projectedP2.w();

			bool isP1Inside = false;
			bool isP2Inside = false;
			IBKMK::Vector2D intersectionP;
			// Check if already in Rubberband
			if(IBKMK::pointInPolygon(verts2D, IBKMK::Vector2D(projectedP1.x(), projectedP1.y() ) ) > 0) {
				isP1Inside = true;
			}
			// Check if already in Rubberband
			if(IBKMK::pointInPolygon(verts2D, IBKMK::Vector2D(projectedP2.x(), projectedP2.y() ) ) > 0) {
				isP2Inside = true;
			}

			if(m_touchGeometry && (isP1Inside || isP2Inside)) {
				nodeIDs.insert(ne.m_id);
				continue;
			}
			else if(isP1Inside && isP2Inside) {
				nodeIDs.insert(ne.m_id);
				continue;
			}
			else if(m_touchGeometry &&
					IBKMK::intersectsLine2D(verts2D, IBKMK::Vector2D(projectedP1.x(), projectedP1.y()),
											IBKMK::Vector2D(projectedP2.x(), projectedP2.y()), intersectionP) ) {
				nodeIDs.insert(ne.m_id);
				break;
			}

		}
		for(const VICUS::NetworkNode &nn : n.m_nodes) {
			// projected radius on NDC
			const double &r = nn.m_visualizationRadius;
			const IBKMK::Vector3D &v3D = nn.m_position;

			// project onto NDC
			QVector4D projectedP = mat * QVector4D(v3D.m_x, v3D.m_y, v3D.m_z, 1.0);
			projectedP /= projectedP.w();

			bool isInside = false;
			// Check if already in Rubberband
			if(IBKMK::pointInPolygon(verts2D, IBKMK::Vector2D(projectedP.x(), projectedP.y() ) ) > 0) {
				isInside = true;
			}

			if(m_touchGeometry && isInside) {
				nodeIDs.insert(nn.m_id);
				continue;
			}

			// Calculate radius
			IBKMK::Vector3D orthoVec = v3D + QVector2IBKVector(c.right())*nn.m_visualizationRadius;
			// project onto NDC
			QVector4D projectedRadPoint = mat * QVector4D(orthoVec.m_x, orthoVec.m_y, orthoVec.m_z, 1.0);
			projectedRadPoint /= projectedRadPoint.w();

			QVector3D radVec = projectedRadPoint.toVector3D() - projectedP.toVector3D();

			// Distance
			double projectedRadius = radVec.length();

			/*! Computes the distance between a line (defined through offset point a, and directional vector d) and a point p.
				\return Returns the shortest distance between line and point. Factor lineFactor contains the scale factor for
						the line equation and p2 contains the closest point on the line (p2 = a + lineFactor*d).
			*/
			double minDist = std::numeric_limits<double>::max();
			for(unsigned int i1=0; i1<verts2D.size(); ++i1) {
				unsigned int i2 = (i1+1) % verts2D.size();
				IBKMK::Vector2D p1 = verts2D[i1];
				IBKMK::Vector2D p2 = verts2D[i2];
				double lineFactor;
				IBKMK::Vector3D closestPoint;
				IBKMK::Vector2D diff = p2-p1;
				double dist = IBKMK::lineToPointDistance(IBKMK::Vector3D(p1.m_x, p1.m_y, 0),
														 IBKMK::Vector3D(diff.m_x, diff.m_y, 0),
														 IBKMK::Vector3D(projectedP.x(), projectedP.y(), 0),
														 lineFactor, closestPoint);
				if(lineFactor > 1) {
					dist = p2.distanceTo(IBKMK::Vector2D(projectedP.x(), projectedP.y()));
				}
				else if(lineFactor < 0) {
					dist = p1.distanceTo(IBKMK::Vector2D(projectedP.x(), projectedP.y()));
				}

				if(m_touchGeometry && (dist < projectedRadius)) {
					nodeIDs.insert(nn.m_id);
					break;
				}
				else
					minDist = std::min(dist, minDist);
			}

			if(!m_touchGeometry && isInside && (minDist > projectedRadius) )
				nodeIDs.insert(nn.m_id);

		}

	}

	// Also select all plain geomerties
	for(const VICUS::Surface &surf : prj.m_plainGeometry.m_surfaces) {

		// qDebug() << "------------------";
		// qDebug() << s.m_displayName;
		double intersectionArea = 0.0;
		double surfaceArea = 0.0;
		if(!surfaceIntersectionClippingAreaWithRubberband(mat, surf, pathRubberband, intersectionArea, surfaceArea))
			continue;

		// qDebug() << "Intersection Area: " << intersectionArea << " | Surface Area: " << surfaceArea;
		if( (m_touchGeometry && intersectionArea>0.0) || IBK::near_equal(surfaceArea, intersectionArea) ) {
			nodeIDs.insert(surf.m_id);
		}
	}

	// Create UNDO Action

	SVUndoTreeNodeState * undo = new SVUndoTreeNodeState(tr("Select objects"),
														 SVUndoTreeNodeState::SelectedState, nodeIDs, m_selectGeometry);
	// select all
	undo->push();

}


} // namespace Vic3D
