#ifndef VIC3DSCENE_H
#define VIC3DSCENE_H

#include <vector>

#include <QRect>
#include <QVector3D>

#include "Vic3DCamera.h"
#include "Vic3DGridObject.h"

class ShaderProgram;
class KeyboardMouseHandler;
class ModificationInfo;

/*! Encapsulates all data for drawing a part of the final picture, including the viewport where the
	scene is rendered in.
*/
class Vic3DScene {
public:
	void create(ShaderProgram *gridShader);

	/*! Triggered when SVProjectHandler::modified() is emitted. */
	void onModified( int modificationType, ModificationInfo * data );

	void destroy();

	void resize(int width, int height, qreal retinaScale);

	/*! Compines camera matrix and project matrix to form the world2view matrix. */
	void updateWorld2ViewMatrix();

	void inputEvent(const KeyboardMouseHandler & keyboardHandler);

	/*! Actually renders to the current OpenGL context. */
	void render();

private:

	/*! Stores viewport geometry. */
	QRect					m_viewPort;

	/*! Stores address to shader program (managed by SceneView). */
	ShaderProgram			*m_gridShader			= nullptr;

	/*! The projection matrix, updated whenever the viewport geometry changes (in resizeGL() ). */
	QMatrix4x4				m_projection;
	/*! World transformation matrix generator. */
	Transform3D				m_transform;
	/*! Camera position, orientation and lens data. */
	Camera					m_camera;
	/*! Cached world to view transformation matrix. */
	QMatrix4x4				m_worldToView;

	/*! Background color */
	QVector3D				m_background = QVector3D(0.1f, 0.15f, 0.3f);

	GridObject				m_gridObject;


};

#endif // VIC3DSCENE_H
