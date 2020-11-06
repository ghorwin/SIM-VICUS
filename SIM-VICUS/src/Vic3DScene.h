#ifndef VIC3DSCENE_H
#define VIC3DSCENE_H

#include <vector>

#include <QRect>
#include <QVector3D>

#include "Vic3DGridObject.h"

class ShaderProgram;

/*! Encapsulates all data for drawing a part of the final picture, including the viewport where the
	scene is rendered in.
*/
class Vic3DScene {
public:
	void create(ShaderProgram *gridShader);

	void destroy();

	void render();

	/*! Stores viewport geometry. */
	QRect					m_viewPort;

	/*! Stores address to shader program (managed by SceneView). */
	ShaderProgram			*m_gridShader			= nullptr;
	/*! Caches world 2 view matrix (updated in SceneView). */
	QMatrix4x4				*m_worldToView			= nullptr;

	/*! Background color */
	QVector3D				m_background = QVector3D(0.1, 0.15, 0.3);
	QVector3D				m_gridColor = QVector3D(0.5, 0.5, 0.7);

	GridObject				m_gridObject;


};

#endif // VIC3DSCENE_H
