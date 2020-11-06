#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>
#include <QDebug>

#include "Vic3DShaderProgram.h"
#include "SVProjectHandler.h"


void Vic3DScene::create(ShaderProgram * gridShader) {
	m_gridShader = gridShader;

	// *** initialize camera placement and model placement in the world

	// move camera a little back (mind: positive z) and look straight ahead
	m_camera.translate(0,17,50);
	// look slightly down
	m_camera.rotate(-5, m_camera.right());
	// look slightly left
	m_camera.rotate(-10, QVector3D(0.0f, 0.0f, 1.0f));
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
	m_gridObject.create(m_gridShader->shaderProgram());

	// transfer other properties
}


void Vic3DScene::destroy() {
	m_gridObject.destroy();
}

void Vic3DScene::resize(int width, int height, qreal retinaScale) {
	// the projection matrix need to be updated only for window size changes
	m_projection.setToIdentity();
	// create projection matrix, i.e. camera lens
	m_projection.perspective(
				/* vertical angle */ 45.0f,
				/* aspect ratio */   width / float(height),
				/* near */           0.1f,
				/* far */            1000.0f
		);
	// Mind: to not use 0.0 for near plane, otherwise depth buffering and depth testing won't work!

	// update cached world2view matrix
	updateWorld2ViewMatrix();

	// update viewport in scenes
	m_viewPort = QRect(0, 0, static_cast<int>(width * retinaScale), static_cast<int>(height * retinaScale) );

}


void Vic3DScene::render() {
	glViewport(m_viewPort.x(), m_viewPort.y(), m_viewPort.width(), m_viewPort.height());

	// set the background color = clear color
	glClearColor(m_background.x(), m_background.y(), m_background.z(), 1.0f);

	// *** grid ***

	// enable depth testing, important for the grid and for the drawing order of several objects
	glEnable(GL_DEPTH_TEST);

	m_gridShader->bind();
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[0], m_worldToView);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[1], m_gridObject.m_gridColor);
	m_gridShader->shaderProgram()->setUniformValue(m_gridShader->m_uniformIDs[2], m_background);

	m_gridObject.render();

	m_gridShader->shaderProgram()->release();


	// *** opaque background geometry ***

	// tell OpenGL to show only faces whose normal vector points towards us
	glEnable(GL_CULL_FACE);



	// *** opaque building geometry ***

	// culling off, so that we see front and back sides of surfaces
	glDisable(GL_CULL_FACE);



	// *** transparent building geometry ***

}

