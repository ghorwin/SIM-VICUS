#include "Vic3DSurfaceNormalsObject.h"

#include <QOpenGLShaderProgram>

#include <VICUS_Project.h>
#include <QtExt_Conversions.h>

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

	// get all visible objects
	std::set<const VICUS::Object*> objs;
	prj.selectObjects(objs, VICUS::Project::SG_Building, false, true);

	// now process all surface objects
	for (const VICUS::Object* o : objs) {
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s == nullptr)
			continue;
		// skip all surfaces with invalid polygons
		if (!s->geometry().isValid())
			continue;
		const std::vector<IBKMK::Vector3D> & vertexes = s->geometry().triangulationData().m_vertexes;
		// take the normal vector and normalize
		IBKMK::Vector3D n = s->geometry().normal();
		n.normalize(); // now has length 1 (as in 1 m)
		// process all vertexes
		for (const IBKMK::Vector3D & v : vertexes) {
			vertexBufferData.push_back(VertexC(QtExt::IBKVector2QVector(v)));
			vertexBufferData.push_back(VertexC(QtExt::IBKVector2QVector(v + n)));
		}
		m_vertexCount += vertexes.size()*2;
	}

	// TODO Andreas, add normals of subsurfaces

#if 0
	vertexBufferData.clear();
	m_vertexCount = 0;

	vertexBufferData.push_back(VertexC(QVector3D(0,0,0)));
	vertexBufferData.push_back(VertexC(QVector3D(10,10,10)));
	m_vertexCount = 2;

#endif

	m_vbo.bind();
	m_vbo.allocate(vertexBufferData.data(), vertexBufferData.size()*sizeof(VertexC));
	m_vbo.release();
}


void SurfaceNormalsObject::render() {
	if (m_vertexCount == 0)
		return;
	m_vao.bind();
	QColor normalsColor("#1bff0a");
	if (SVSettings::instance().m_theme == SVSettings::TT_White)
		normalsColor = QColor("#1b8e0a");
	QVector4D col(normalsColor.redF(), normalsColor.greenF(), normalsColor.blueF(), 1.0);
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[1], col);
	glDrawArrays(GL_LINES, 0, m_vertexCount);
	m_vao.release();
}

} // namespace Vic3D
