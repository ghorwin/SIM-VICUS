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

#include "Vic3DSurfaceNormalsObject.h"

#include <QOpenGLShaderProgram>

#include <VICUS_Project.h>
#include <SV_Conversions.h>

#include "SVProjectHandler.h"
#include "SVSettings.h"

#include "Vic3DVertex.h"
#include "Vic3DShaderProgram.h"

namespace Vic3D {

void SurfaceNormalsObject::create(ShaderProgram * shaderProgram) {
	// initialize only if we haven't done so yet
	if (!m_vao.isCreated()) {
		m_shaderProgram = shaderProgram;

		m_vao.create();		// create Vertex Array Object
		m_vao.bind();		// and bind it

		// Create Vertex Buffer Object (to store stuff in GPU memory)
		m_vbo.create();
		m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
		m_vbo.bind();

		// layout(location = 0) = vec3 vertex coordinates
		m_shaderProgram->shaderProgram()->enableAttributeArray(0); // array with index/id 0
		m_shaderProgram->shaderProgram()->setAttributeBuffer(0, GL_FLOAT, 0, 3 /* vec3 */, sizeof(VertexC));

		m_vao.release(); // Mind: always release VAO before index buffer
		m_vbo.release();
	}
}


void SurfaceNormalsObject::destroy() {
	m_vao.destroy();
	m_vbo.destroy();
}


void SurfaceNormalsObject::updateVertexBuffers() {
	// populate vertex buffer object and push to GPU
	std::vector<VertexC>		vertexBufferData;
	m_vertexCount = 0;

	const VICUS::Project & prj = project();

	// get all visible and selected objects
	std::set<const VICUS::Object*> objs;
	prj.selectObjects(objs, VICUS::Project::SG_Building, true, true);

	std::set<const VICUS::SubSurface *> subSurfacesToShowNormalsFor;

	// now process all surface objects
	for (const VICUS::Object* o : objs) {
		const VICUS::SubSurface * subSurf = dynamic_cast<const VICUS::SubSurface *>(o);
		if (subSurf != nullptr) {
			// remember subsurface to show
			subSurfacesToShowNormalsFor.insert(subSurf);
			continue;
		}

		// process all surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s == nullptr)
			continue;
		// skip all surfaces with invalid polygons
		if (!s->geometry().isValid())
			continue;
		// remember to process subsurfaces later on
		if (!s->geometry().holeTriangulationData().empty() && s->geometry().holeTriangulationData().size() == s->subSurfaces().size()) {
			for (unsigned int i=0; i<s->subSurfaces().size(); ++i)
				subSurfacesToShowNormalsFor.insert(&s->subSurfaces()[i]);
		}
		const std::vector<IBKMK::Vector3D> & vertexes = s->geometry().triangulationData().m_vertexes;
		// take the normal vector and normalize
		IBKMK::Vector3D n = s->geometry().normal();
		n.normalize(); // now has length 1 (as in 1 m)
		// process all vertexes
		for (const IBKMK::Vector3D & v : vertexes) {
			vertexBufferData.push_back(VertexC(IBKVector2QVector(v)));
			vertexBufferData.push_back(VertexC(IBKVector2QVector(v + n)));
		}
		m_vertexCount += vertexes.size()*2;
	}

	// now process all sub-surfaces - these are either selected directly or selected indirectly as part of their parent
	// surface
	for (const VICUS::SubSurface * subSurf : subSurfacesToShowNormalsFor) {
		// get parent surface
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface * >(subSurf->m_parent);
		// only handle sub-surface if we have a valid polygon
		if (!s->geometry().holeTriangulationData().empty() && s->geometry().holeTriangulationData().size() == s->subSurfaces().size()) {
			// which sub-surface index do we have?
			for (unsigned int j=0; j<s->subSurfaces().size(); ++j) {
				if (&(s->subSurfaces()[j]) == subSurf) {
					const VICUS::PlaneTriangulationData & holeTriangulation = s->geometry().holeTriangulationData()[j];
					const std::vector<IBKMK::Vector3D> & vertexes = holeTriangulation.m_vertexes;
					// take the normal vector and normalize
					IBKMK::Vector3D n = s->geometry().normal();
					n.normalize(); // now has length 1 (as in 1 m)
					// process all vertexes
					for (const IBKMK::Vector3D & v : vertexes) {
						vertexBufferData.push_back(VertexC(IBKVector2QVector(v)));
						vertexBufferData.push_back(VertexC(IBKVector2QVector(v + n)));
					}
					m_vertexCount += vertexes.size()*2;
					break;
				}
			}
		}
	}

	m_vbo.bind();
	m_vbo.allocate(vertexBufferData.data(), vertexBufferData.size()*sizeof(VertexC));
	m_vbo.release();
}


void SurfaceNormalsObject::render() {
	if (m_vertexCount == 0)
		return;
	m_vao.bind();
	QColor normalsColor;
	if (SVSettings::instance().m_theme == SVSettings::TT_Dark)
		normalsColor = QColor("#4ec4f6");
	else
		normalsColor = QColor("#005eaa");
	QVector4D col(normalsColor.redF(), normalsColor.greenF(), normalsColor.blueF(), 1.0);
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[1], col);
	glDrawArrays(GL_LINES, 0, m_vertexCount);
	m_vao.release();
}

} // namespace Vic3D
