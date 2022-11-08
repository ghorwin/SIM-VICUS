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

#ifndef Vic3DSceneViewH
#define Vic3DSceneViewH

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

	/*! This function can be used to toggle the visibility of the surface normal vectors.
		Called from SVMainWindow::on_actionViewShowSurfaceNormals_toggled().
	*/
	void setNormalVectorsVisible(bool visible);

	/*! Called when the global hot key has been pressed, simply relays the call to the main scene. */
	void toggleAlignCoordinateSystem();
	/*! Called when the global hot key has been pressed, simply relays the call to the main scene.. */
	void toggleTranslateCoordinateSystem();
	/*! Called when the global hot key has been pressed, simply relays the call to the main scene.. */
	void toggleMeasurementMode();
	/*! Called when the global hot key has been pressed, simply relays the call to the main scene.. */
	void toggleRubberbandMode();

	/*! Resets the camera position to be looking nicely onto the scene.
		See SVGeometryView::resetCamera().
	*/
	void resetCamera(int position);

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

	/*! Connected to SVViewStateHandler::refreshColors(). */
	void onColorRefreshNeeded();

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
	void focusOutEvent(QFocusEvent* event) override;

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

	Scene						m_mainScene;

	QElapsedTimer				m_cpuTimer;

	unsigned int				m_paintCounter = 0;

	/*! Framebuffer object (including image storage) for screenshots - multisample variant. */
	QOpenGLFramebufferObject	*m_screenShotMultiSampleFrameBuffer = nullptr;
	/*! Framebuffer object (including image storage) for screenshots - downsample variant (includes anti-aliasing). */
	QOpenGLFramebufferObject	*m_screenShotDownSampleFrameBuffer = nullptr;
};

} // namespace Vic3D

#endif // Vic3DSceneViewH
