#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>

#include "Vic3DShaderProgram.h"


void Vic3DScene::create(ShaderProgram * gridShader) {
	m_gridShader = gridShader;
	m_gridObject.create(gridShader->shaderProgram());
}


void Vic3DScene::destroy() {
	m_gridObject.destroy();
}


void Vic3DScene::render() {
	glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

//	m_background.setRgbF(0.1, 0.15, 0.3);
//	m_gridColor.setRgbF(0.5, 0.5, 0.7);

	// set the background color = clear color
	glClearColor(m_background.x(), m_background.y(), m_background.z(), 1.0f);

	m_gridShader->bind();
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[0], *m_worldToView);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_gridColor);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[2], m_background);

	m_gridObject.render();

	m_gridShader->shaderProgram()->release();

}

