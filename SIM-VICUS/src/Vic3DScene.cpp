#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>

#include "Vic3DShaderProgram.h"
#include "SVProjectHandler.h"


void Vic3DScene::create(ShaderProgram * gridShader) {
	m_gridShader = gridShader;
}


void Vic3DScene::onModified(int modificationType, ModificationInfo * data) {

	// no shader - not initialized yet, skip

	if (m_gridShader == nullptr)
		return;

	// filter out all modification types that we handle
	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		case SVProjectHandler::AllModified :
		case SVProjectHandler::GridModified :
			break;

		default:
			return; // do nothing by default
	}

	// re-create grid with updated properties

	/// \todo get grid dimensions from project

	m_gridObject.create(m_gridShader->shaderProgram());
}


void Vic3DScene::destroy() {
	m_gridObject.destroy();
}


void Vic3DScene::render() {
	glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

	// set the background color = clear color
	glClearColor(m_background.x(), m_background.y(), m_background.z(), 1.0f);

	m_gridShader->bind();
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[0], *m_worldToView);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_gridColor);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[2], m_background);

	m_gridObject.render();

	m_gridShader->shaderProgram()->release();

}

