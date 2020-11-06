#include "Vic3DScene.h"

#include <QOpenGLShaderProgram>
#include <QDebug>
#include <QCursor>

#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
#include "SVProjectHandler.h"


void Vic3DScene::create(ShaderProgram * gridShader) {
	m_gridShader = gridShader;

	// *** initialize camera placement and model placement in the world

	// move camera a little back (mind: positive y) and look straight ahead
	m_camera.translate(0, 0, 20);
	m_camera.translate( m_camera.forward()*0.1);
	// look slightly down
//	m_camera.rotate(70, m_camera.right());
	// look slightly left
	//m_camera.rotate(-10, QVector3D(0.0f, 1.0f, 0.0f));
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

	// update viewport
	m_viewPort = QRect(0, 0, static_cast<int>(width * retinaScale), static_cast<int>(height * retinaScale) );
}


void Vic3DScene::updateWorld2ViewMatrix() {
	// transformation steps:
	//   model space -> transform -> world space
	//   world space -> camera/eye -> camera view
	//   camera view -> projection -> normalized device coordinates (NDC)
	m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
}


void Vic3DScene::inputEvent(const KeyboardMouseHandler & keyboardHandler) {
	// check for trigger key to enable fly-through-mode
	if (keyboardHandler.buttonDown(Qt::RightButton)) {

		// Handle translations
		QVector3D translation;
		if (keyboardHandler.keyDown(Qt::Key_W)) 		translation += m_camera.forward();
		if (keyboardHandler.keyDown(Qt::Key_S)) 		translation -= m_camera.forward();
		if (keyboardHandler.keyDown(Qt::Key_A)) 		translation -= m_camera.right();
		if (keyboardHandler.keyDown(Qt::Key_D)) 		translation += m_camera.right();
		if (keyboardHandler.keyDown(Qt::Key_Q)) 		translation -= m_camera.up();
		if (keyboardHandler.keyDown(Qt::Key_E)) 		translation += m_camera.up();

		float transSpeed = 0.8f;
		if (keyboardHandler.keyDown(Qt::Key_Shift))
			transSpeed = 0.1f;
		m_camera.translate(transSpeed * translation);

		// Handle rotations
		// get and reset mouse delta (pass current mouse cursor position)
		QPoint mouseDelta = keyboardHandler.mouseDelta(QCursor::pos()); // resets the internal position
		static const float rotatationSpeed  = 0.4f;
		const QVector3D LocalUp(0.0f, 0.0f, 1.0f); // same as in Camera::up()
		m_camera.rotate(-rotatationSpeed * mouseDelta.x(), LocalUp);
		m_camera.rotate(-rotatationSpeed * mouseDelta.y(), m_camera.right());

	}
	int wheelDelta = keyboardHandler.wheelDelta();
	if (wheelDelta != 0) {
		float transSpeed = 8.f;
		if (keyboardHandler.keyDown(Qt::Key_Shift))
			transSpeed = 0.8f;
		m_camera.translate(wheelDelta * transSpeed * m_camera.forward());
	}

	updateWorld2ViewMatrix();
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

