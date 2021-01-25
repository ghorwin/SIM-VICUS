#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#include <QMatrix4x4>
#include <QOpenGLTimeMonitor>
#include <QElapsedTimer>

#include "Vic3DOpenGLWindow.h"
#include "Vic3DShaderProgram.h"
#include "Vic3DKeyboardMouseHandler.h"
#include "Vic3DGridObject.h"
#include "Vic3DCamera.h"
#include "Vic3DScene.h"

class QOpenGLFramebufferObject;

class ModificationInfo;

namespace Vic3D {

/*! The class SceneView extends the primitive OpenGLWindow
	by adding keyboard/mouse event handling, and rendering of different
	objects (that encapsulate shader programs and buffer object).
*/
class SceneView : public OpenGLWindow {
	Q_OBJECT
public:
	SceneView();
	virtual ~SceneView() override;

	/*! Renders current scene into a framebuffer and saves it to file. */
	void dumpScreenshot(const QString & imgFilePath);

public slots:

	/*! Connected to SVProjectHandler::modified() */
	void onModified( int modificationType, ModificationInfo * data );

	/*! Connected to SVPreferencesPageStyle::styleChanged(). */
	void onStyleChanged();

	/*! Connected to view-state handler. */
	void onViewStateChanged();

	/*! Action to make all selected objects invisible. */
	void onHideSelected();

	/*! Action to show all selected objects. */
	void onShowSelected();

	/*! Action to delete all selected objects. */
	void onDeleteSelected();

	/*! Action to select all objects. */
	void onSelectAll();

	/*! Action to deselect all selected objects. */
	void onDeselectAll();

signals:
	/*! Emitted when numbers where typed.
		Only during "PlaceVertex" scene operation mode.
	*/
	void numberKeyPressed(Qt::Key k);

protected:
	void initializeGL() override;
	void resizeGL(int width, int height) override;
	void paintGL() override;

	// Functions to handle key press and mouse press events, all the work is done in class KeyboardMouseHandler
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	/*! Tests, if any relevant input was received and registers a state change. */
	void checkInput();

	/*! This function is called first thing in the paintGL() routine and
		processes input received so far and updates camera position.
	*/
	bool processInput();

	/*! If set to true, an input event was received, which will be evaluated at next repaint. */
	bool						m_inputEventReceived;

	/*! The input handler, that encapsulates the event handling code. */
	KeyboardMouseHandler		m_keyboardMouseHandler;

	/*! All shader programs used in the scene. */
	std::vector<ShaderProgram>	m_shaderPrograms;

	Vic3DScene					m_mainScene;

	QElapsedTimer				m_cpuTimer;

	unsigned int				m_paintCounter = 0;

	/*! Framebuffer object (including image storage) for screenshots - multisample variant. */
	QOpenGLFramebufferObject	*m_screenShotMultiSampleFrameBuffer = nullptr;
	/*! Framebuffer object (including image storage) for screenshots - downsample variant (includes anti-aliasing). */
	QOpenGLFramebufferObject	*m_screenShotDownSampleFrameBuffer = nullptr;
};

} // namespace Vic3D

#endif // SCENEVIEW_H
