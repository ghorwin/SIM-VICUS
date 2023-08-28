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

#include "Vic3DWireFrameObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <IBK_physics.h>

#include <VICUS_Project.h>
#include <SVConversions.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVSettings.h"
#include "Vic3DConstants.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DShaderProgram.h"
#include "qstyle.h"

namespace Vic3D {

WireFrameObject::WireFrameObject() :
	m_vertexBufferObject(QOpenGLBuffer::VertexBuffer), // VertexBuffer is the default, so default constructor would have been enough
	m_indexBufferObject(QOpenGLBuffer::IndexBuffer) // make this an Index Buffer
{
	SVViewStateHandler::instance().m_selectedGeometryObject = this;
}


void WireFrameObject::create(ShaderProgram * shaderProgram) {
	if (m_vao.isCreated())
		return;

	m_shaderProgram = shaderProgram;

	// *** create buffers on GPU memory ***

	// create a new buffer for the vertices and colors, separate buffers because we will modify colors way more often than geometry
	m_vertexBufferObject.create();
	m_vertexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw); // usage pattern will be used when tranferring data to GPU

	// create a new buffer for the indexes
	m_indexBufferObject = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); // Note: make sure this is an index buffer
	m_indexBufferObject.create();
	m_indexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);


	// *** create and bind Vertex Array Object ***

	// Note: VAO must be bound *before* the element buffer is bound,
	//       because the VAO remembers associated element buffers.
	m_vao.create();
	m_vao.bind(); // now the VAO is active and remembers states modified in following calls

	m_indexBufferObject.bind(); // this registers this index buffer in the currently bound VAO


	// *** set attribute arrays for shader fetch stage ***

#define VERTEX_ARRAY_INDEX 0

	m_vertexBufferObject.bind(); // this registers this buffer data object in the currently bound vao; in subsequent
	// calls to shaderProgramm->setAttributeBuffer() the buffer object is associated with the
	// respective attribute array that's fed into the shader. When the vao is later bound before
	// rendering, this association is remembered so that the vertex fetch stage pulls data from
	// this vbo

	// coordinates
	m_shaderProgram->shaderProgram()->enableAttributeArray(VERTEX_ARRAY_INDEX);
	m_shaderProgram->shaderProgram()->setAttributeBuffer(VERTEX_ARRAY_INDEX, GL_FLOAT, 0, 3 /* vec3 */, sizeof(VertexC));

	// Release (unbind) all

	// Mind: you can release the buffer data objects (vbo and vboColors) before or after releasing vao. It does not
	//       matter, because the buffers are associated already with the attribute arrays.
	//       However, YOU MUST NOT release the index buffer (ebo) before releasing the vao, since this would remove
	//       the index buffer association with the vao and when binding the vao before rendering, the element buffer
	//       would not be known and a call to glDrawElements() crashes!
	m_vao.release();

	m_vertexBufferObject.release();
	m_indexBufferObject.release();
}


void WireFrameObject::destroy() {
	m_vao.destroy();
	m_vertexBufferObject.destroy();
	m_indexBufferObject.destroy();
}

void WireFrameObject::updateBuffers() {
	// get all selected and visible objects
	m_selectedObjects.clear();
	project().selectObjects(m_selectedObjects, VICUS::Project::SG_All, true, true);

	if (m_selectedObjects.empty())
		return; // nothing to render, return. Note: the buffer remains unmodified on the GPU, yet we don't draw anything.

	// clear out existing cache

	m_vertexBufferData.clear();
	m_indexBufferData.clear();
	m_vertexStartMap.clear();

	m_vertexBufferData.reserve(100000);
	m_indexBufferData.reserve(100000);

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	qDebug() << m_selectedObjects.size() << " selected objects";

	for (const VICUS::Object * o : m_selectedObjects) {

		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			if (!s->geometry().isValid()) continue;
			addPlane(s->geometry().triangulationData(), currentVertexIndex, currentElementIndex, m_vertexBufferData, m_indexBufferData);

			continue;
		}

		const VICUS::SubSurface * sub = dynamic_cast<const VICUS::SubSurface *>(o);
		if (sub != nullptr) {
			const VICUS::Surface * surf = dynamic_cast<const VICUS::Surface *>(sub->m_parent);
			IBK_ASSERT(surf != nullptr);
			if (!surf->geometry().isValid()) continue;
			// get index of subsurface
			for (unsigned int i=0; i<surf->subSurfaces().size(); ++i) {
				if (&surf->subSurfaces()[i] == sub) { // Pointer comparison or comparison by unique ID?
					addPlane(surf->geometry().holeTriangulationData()[i],
							 currentVertexIndex, currentElementIndex, m_vertexBufferData, m_indexBufferData);
					break;
				}
			}
			continue;
		}

		const VICUS::NetworkEdge * e = dynamic_cast<const VICUS::NetworkEdge *>(o);
		if (e != nullptr) {
			double radius = e->m_visualizationRadius;
			addCylinder(e->m_node1->m_position, e->m_node2->m_position, radius,
						currentVertexIndex, currentElementIndex, m_vertexBufferData, m_indexBufferData);

			continue;
		}

		const VICUS::NetworkNode * n = dynamic_cast<const VICUS::NetworkNode *>(o);
		if (n != nullptr) {
			double radius = n->m_visualizationRadius;
			addSphere(n->m_position, radius,
					  currentVertexIndex, currentElementIndex, m_vertexBufferData, m_indexBufferData);
			continue;
		}

		const VICUS::DrawingLayer * drawingLayer = dynamic_cast<const VICUS::DrawingLayer *>(o);
		if (drawingLayer != nullptr) {

			const VICUS::Drawing *drawing = dynamic_cast<const VICUS::Drawing *>(drawingLayer->m_parent);

			Q_ASSERT(drawing != nullptr);

			for (const VICUS::Drawing::Line & line : drawing->m_lines){

				if (line.m_parentLayer != drawingLayer)
					continue;

				double width = DEFAULT_LINE_WEIGHT + line.lineWeight() * DEFAULT_LINE_WEIGHT_SCALING;

				// Create Vector from start and end point of the line, add point of origin to each coordinate and calculate z value
				double zCoordinate = line.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z;
				IBKMK::Vector3D p1 = IBKMK::Vector3D(line.m_point1.m_x + drawing->m_origin.m_x, line.m_point1.m_y + drawing->m_origin.m_y, zCoordinate);
				IBKMK::Vector3D p2 = IBKMK::Vector3D(line.m_point2.m_x + drawing->m_origin.m_x, line.m_point2.m_y + drawing->m_origin.m_y, zCoordinate);

				// scale Vector with selected unit
				p1 *= drawing->m_scalingFactor;
				p2 *= drawing->m_scalingFactor;

				// rotate Vectors
				QVector3D vec1 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1);
				QVector3D vec2 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2);

				// call addLine to draw line
				addLine(QVector2IBKVector(vec1), QVector2IBKVector(vec2), drawing->m_rotationMatrix,
						width, currentVertexIndex, currentElementIndex, m_vertexBufferData, m_indexBufferData);
			}

			for(const VICUS::Drawing::Point & point : drawing->m_points){

				if (point.m_parentLayer != drawingLayer)
					continue;

				// Create Vector from point, add point of origin to each coordinate and calculate z value
				IBKMK::Vector3D p(point.m_point.m_x + drawing->m_origin.m_x,
								  point.m_point.m_y + drawing->m_origin.m_y,
								  point.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z);

				// scale Vector with selected unit
				p *= drawing->m_scalingFactor;

				double pointWeight = (DEFAULT_LINE_WEIGHT + point.lineWeight() * DEFAULT_LINE_WEIGHT_SCALING) / 2;

				// rotation
				QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
				IBKMK::Vector3D p1 = QVector2IBKVector(vec);

				IBKMK::Vector3D pExt0 = IBKMK::Vector3D(p1.m_x - pointWeight, p1.m_y - pointWeight, p1.m_z);
				IBKMK::Vector3D pExt1 = IBKMK::Vector3D(p1.m_x + pointWeight, p1.m_y - pointWeight, p1.m_z);
				IBKMK::Vector3D pExt2 = IBKMK::Vector3D(p1.m_x - pointWeight, p1.m_y + pointWeight, p1.m_z);

				IBKMK::Polygon3D po(VICUS::Polygon2D::T_Rectangle, pExt0, pExt2, pExt1);
				VICUS::PlaneGeometry g1(po);

				addPlane(g1.triangulationData(), currentVertexIndex,
						 currentElementIndex, m_vertexBufferData,
						 m_indexBufferData);
			}

			for(const VICUS::Drawing::PolyLine & polyline : drawing->m_polylines){

				if (polyline.m_parentLayer != drawingLayer)
					continue;

				// Create Vector to store vertices of polyline
				std::vector<IBKMK::Vector3D> polylinePoints;

				// adds z-coordinate to polyline
				for(unsigned int i = 0; i < polyline.m_polyline.size(); i++){
					IBKMK::Vector3D p = IBKMK::Vector3D(polyline.m_polyline[i].m_x + drawing->m_origin.m_x,
														polyline.m_polyline[i].m_y + drawing->m_origin.m_y,
														polyline.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z);
					p *= drawing->m_scalingFactor;

					QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
					polylinePoints.push_back(QVector2IBKVector(vec));
				}

				addPolyLine(polylinePoints, drawing->m_rotationMatrix, polyline.m_endConnected,
							DEFAULT_LINE_WEIGHT + polyline.lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
							currentVertexIndex, currentElementIndex,
							m_vertexBufferData,	m_indexBufferData);
			}

			for(const VICUS::Drawing::Circle & circle : drawing->m_circles){

				if (circle.m_parentLayer != drawingLayer)
					continue;

				std::vector<IBKMK::Vector3D> circlePoints;

				for(int i = 0; i < Vic3D::SEGMENT_COUNT_CIRCLE; i++){
					IBKMK::Vector3D p = IBKMK::Vector3D(circle.m_center.m_x + circle.m_radius * cos(2 * IBK::PI * i / Vic3D::SEGMENT_COUNT_CIRCLE) + drawing->m_origin.m_x,
														circle.m_center.m_y + circle.m_radius * sin(2 * IBK::PI * i / Vic3D::SEGMENT_COUNT_CIRCLE) + drawing->m_origin.m_y,
														circle.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z);
					p *= drawing->m_scalingFactor;

					QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
					circlePoints.push_back(QVector2IBKVector(vec));
				}

				addPolyLine(circlePoints, drawing->m_rotationMatrix, true,
							DEFAULT_LINE_WEIGHT + circle.lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
							currentVertexIndex, currentElementIndex,
							m_vertexBufferData,
							m_indexBufferData);

			}

			for(const VICUS::Drawing::Arc & arc : drawing->m_arcs){

				if (!arc.m_parentLayer->m_visible)
					continue;

				std::vector<IBKMK::Vector3D> arcPoints;

				double startAngle = arc.m_startAngle;
				double endAngle = arc.m_endAngle;

				double angleDifference;

				if(startAngle > endAngle)
					angleDifference = 2 * IBK::PI - startAngle + endAngle;
				else
					angleDifference = endAngle - startAngle;


				int toCalcN = (int)(Vic3D::SEGMENT_COUNT_ARC * (2 * IBK::PI / angleDifference));

				double stepAngle = angleDifference / toCalcN;

				for(int i = 0; i < toCalcN; i++){
					IBKMK::Vector3D p = IBKMK::Vector3D(arc.m_center.m_x + arc.m_radius * cos(startAngle + i * stepAngle) + drawing->m_origin.m_x,
														arc.m_center.m_y + arc.m_radius * sin(startAngle + i * stepAngle) + drawing->m_origin.m_y,
														arc.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z);
					p *= drawing->m_scalingFactor;

					QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
					arcPoints.push_back(QVector2IBKVector(vec));
				}

				addPolyLine(arcPoints, drawing->m_rotationMatrix, false, DEFAULT_LINE_WEIGHT + arc.lineWeight() * DEFAULT_LINE_WEIGHT_SCALING,
							currentVertexIndex, currentElementIndex, m_vertexBufferData, m_indexBufferData);

			}

			for (const VICUS::Drawing::Ellipse & ellipse : drawing->m_ellipses){

				if (!ellipse.m_parentLayer->m_visible)
					continue;

				std::vector<IBKMK::Vector3D> ellipsePoints;

				// Assuming startAngle and endAngle are in radians, provided by your ellipse object
				double startAngle = ellipse.m_startAngle;
				double endAngle = ellipse.m_endAngle;

				double angleStep = (endAngle - startAngle) / Vic3D::SEGMENT_COUNT_ELLIPSE;

				double majorRadius = sqrt(pow(ellipse.m_majorAxis.m_x, 2) + pow(ellipse.m_majorAxis.m_y, 2));
				double minorRadius = majorRadius * ellipse.m_ratio;

				double rotationAngle = atan2(ellipse.m_majorAxis.m_y, ellipse.m_majorAxis.m_x);

				double x, y, rotated_x, rotated_y;

				for (unsigned int i = 0; i <= Vic3D::SEGMENT_COUNT_ELLIPSE; i++) {

					double currentAngle = startAngle + i * angleStep;

					x = majorRadius * cos(currentAngle);
					y = minorRadius * sin(currentAngle);

					rotated_x = x * cos(rotationAngle) - y * sin(rotationAngle);
					rotated_y = x * sin(rotationAngle) + y * cos(rotationAngle);

					IBKMK::Vector3D p = IBKMK::Vector3D(rotated_x + ellipse.m_center.m_x + drawing->m_origin.m_x,
														rotated_y + ellipse.m_center.m_y + drawing->m_origin.m_y,
														ellipse.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z);
					p *= drawing->m_scalingFactor;

					QVector3D vec = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p);
					ellipsePoints.push_back(QVector2IBKVector(vec));
				}

				// Change this line to false, so it doesn't connect the last point to the first point
				addPolyLine(ellipsePoints, drawing->m_rotationMatrix, startAngle == endAngle,
							DEFAULT_LINE_WEIGHT + ellipse.m_lineWeight * DEFAULT_LINE_WEIGHT_SCALING,
							currentVertexIndex,
							currentElementIndex, m_vertexBufferData,
							m_indexBufferData);
			}


			for(const VICUS::Drawing::Solid &solid : drawing->m_solids){

				if (!solid.m_parentLayer->m_visible)
					continue;


				IBKMK::Vector3D p1 = IBKMK::Vector3D(solid.m_point1.m_x + drawing->m_origin.m_x,
													 solid.m_point1.m_y + drawing->m_origin.m_y,
													 solid.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z);
				IBKMK::Vector3D p2 = IBKMK::Vector3D(solid.m_point2.m_x + drawing->m_origin.m_x,
													 solid.m_point2.m_y + drawing->m_origin.m_y,
													 solid.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z);
				/* IBKMK::Vector3D p3 = IBKMK::Vector3D(solid.m_point3.m_x + drawing.m_origin.m_x, solid.m_point3.m_y + drawing.m_origin.m_y, solid.m_zposition * Z_MULTIPLYER + drawing.m_origin.m_z); */
				IBKMK::Vector3D p4 = IBKMK::Vector3D(solid.m_point4.m_x + drawing->m_origin.m_x,
													 solid.m_point4.m_y + drawing->m_origin.m_y,
													 solid.m_zPosition * Z_MULTIPLYER + drawing->m_origin.m_z);

				p1 *= drawing->m_scalingFactor;
				p2 *= drawing->m_scalingFactor;
//				/* p3 *= drawing.m_scalingFactor; */
				p4 *= drawing->m_scalingFactor;

				QVector3D vec1 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p1);
				QVector3D vec2 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p2);
				/* QVector3D vec3 = drawing.m_rotationMatrix.toQuaternion() * IBKVector2QVector(p3); */
				QVector3D vec4 = drawing->m_rotationMatrix.toQuaternion() * IBKVector2QVector(p4);

				IBKMK::Polygon3D p(VICUS::Polygon2D::T_Rectangle, QVector2IBKVector(vec1), QVector2IBKVector(vec4), QVector2IBKVector(vec2));
				VICUS::PlaneGeometry g1(p);

				addPlane(g1.triangulationData(), currentVertexIndex, currentElementIndex, m_vertexBufferData, m_indexBufferData);
			}

		}

	}

	// transfer data to GPU
	m_vertexBufferObject.bind();
	m_vertexBufferObject.allocate(m_vertexBufferData.data(), m_vertexBufferData.size()*sizeof(VertexC));
	m_vertexBufferObject.release();

	m_indexBufferObject.bind();
	m_indexBufferObject.allocate(m_indexBufferData.data(), m_indexBufferData.size()*sizeof(GLuint));
	m_indexBufferObject.release();
}


void WireFrameObject::render() {
	if (m_selectedObjects.empty())
		return; // nothing to render
	// bind all buffers ("position", "normal" and "color" arrays)
	m_vao.bind();
	// set transformation matrix
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[1], m_transform.toMatrix());

	glDisable(GL_CULL_FACE);
	// set wireframe color (TODO : make this theme-dependent?)
	QColor selColor = SVSettings::instance().m_themeSettings[SVSettings::instance().m_theme].m_selectedSurfaceColor;
	double brightness = 0.299*selColor.redF() + 0.587*selColor.greenF() + 0.114*selColor.blueF();
	QColor wireFrameCol = selColor.lighter();
	if (brightness > 0.4)
		wireFrameCol = selColor.darker();
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], wireFrameCol);
	// put OpenGL in offset mode
	glEnable(GL_POLYGON_OFFSET_LINE);
	// offset the wire frame geometry a bit
	glPolygonOffset(0.0f, -2.0f);
	// select wire frame drawing
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// now draw the geometry
	glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
	// switch back to fill mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// turn off line offset mode
	glDisable(GL_POLYGON_OFFSET_LINE);

	// set selected plane color (QColor is passed as vec4, so no conversion is needed, here).
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], selColor);
	// now draw the geometry
	glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);

	m_vao.release();
}


void WireFrameObject::resetTransformation() {
	m_transform = Vic3D::Transform3D();
	m_translation = QVector3D(0,0,0);
	m_scaling = QVector3D(0,0,0);
	m_rotation  = QQuaternion();
}


void WireFrameObject::translate(const QVector3D & translation) {
	m_translation = translation;
	m_rotation = QQuaternion();
	m_scaling = QVector3D(0,0,0);
	m_transform = Vic3D::Transform3D();
	m_transform.setTranslation(translation);

}


void WireFrameObject::rotate(const QQuaternion & rotation, const QVector3D & offset) {
	m_translation = offset;
	m_rotation = rotation;
	m_scaling = QVector3D(0,0,0);
	m_transform = Vic3D::Transform3D();
	m_transform.setTranslation(offset);
	m_transform.setRotation(rotation);
}


void WireFrameObject::localScaling(const QVector3D & offset, const QQuaternion & toLocal, const QVector3D & localScaleFactors) {
	m_translation = offset;
	m_rotation = toLocal;
	m_scaling = localScaleFactors;
	m_transform.setLocalScaling(offset, toLocal, localScaleFactors);
}


} // namespace Vic3D
