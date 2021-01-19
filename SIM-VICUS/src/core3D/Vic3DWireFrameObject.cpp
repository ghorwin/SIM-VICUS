#include "Vic3DWireFrameObject.h"

#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <VICUS_Project.h>
#include <VICUS_Conversions.h>

#include "SVProjectHandler.h"
#include "SVViewStateHandler.h"
#include "SVSettings.h"
#include "Vic3DGeometryHelpers.h"
#include "Vic3DShaderProgram.h"

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

	updateSelectedObjectsFromProject(m_selectedObjects);

	// clear out existing cache

	m_vertexBufferData.clear();
	m_indexBufferData.clear();
	m_vertexStartMap.clear();

	m_vertexBufferData.reserve(100000);
	m_indexBufferData.reserve(100000);

	// we want to draw triangles
	m_drawTriangleStrips = false;

	unsigned int currentVertexIndex = 0;
	unsigned int currentElementIndex = 0;

	qDebug() << m_selectedObjects.size() << " selected objects";

	for (const VICUS::Object * o : m_selectedObjects) {

		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			addPlane(s->m_geometry, currentVertexIndex, currentElementIndex, m_vertexBufferData, m_indexBufferData);
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
	if (m_drawTriangleStrips)
		glDrawElements(GL_TRIANGLE_STRIP, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
	else
		glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
	// switch back to fill mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// turn off line offset mode
	glDisable(GL_POLYGON_OFFSET_LINE);

	// set selected plane color (QColor is passed as vec4, so no conversion is needed, here).
	m_shaderProgram->shaderProgram()->setUniformValue(m_shaderProgram->m_uniformIDs[2], selColor);
	// now draw the geometry
	if (m_drawTriangleStrips)
		glDrawElements(GL_TRIANGLE_STRIP, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);
	else
		glDrawElements(GL_TRIANGLES, m_indexBufferData.size(), GL_UNSIGNED_INT, nullptr);

	m_vao.release();
}


void WireFrameObject::updateSelectedObjectsFromProject(std::set<const VICUS::Object*> & selectedObjects, bool takeInvisible) {
	selectedObjects.clear();

	// first, we take *all* objects that are selected

	// process all building surfaces
	const VICUS::Project & prj = project();
	// Buildings
	for (const VICUS::Building & b : prj.m_buildings) {
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels) {
			for (const VICUS::Room & r : bl.m_rooms) {
				for (const VICUS::Surface & s : r.m_surfaces) {
					if (s.m_selected)
						selectedObjects.insert(&s);
				}
			}
		}
	}

	// Networks
	for (const VICUS::Network & n : prj.m_geometricNetworks) {
		for (const VICUS::NetworkEdge & e : n.m_edges) {
			if (e.m_selected)
				selectedObjects.insert(&e);
		}

		for (const VICUS::NetworkNode & nod : n.m_nodes) {
			if (nod.m_selected)
				selectedObjects.insert(&nod);
		}
	}

	// Dumb plain geometry
	for (const VICUS::Surface & s : prj.m_plainGeometry) {
		if (s.m_selected)
			selectedObjects.insert(&s);
	}

	// now filter out those that are not wanted

	std::set<const VICUS::Object*> filteredSet;
	if (takeInvisible) {
		for (const VICUS::Object* o : selectedObjects)
			if (!o->m_visible)
				filteredSet.insert(o);
	}
	else {
		for (const VICUS::Object* o : selectedObjects)
			if (o->m_visible)
				filteredSet.insert(o);
	}
	selectedObjects.swap(filteredSet);
}

} // namespace Vic3D
