#include "Vic3DSceneView.h"

#include <QExposeEvent>
#include <QOpenGLShaderProgram>
#include <QDateTime>
#include <QOpenGLFramebufferObject>

#include "SVDebugApplication.h"

#include "Vic3DPickObject.h"
#include "OpenGLException.h"
#include "SVProjectHandler.h"
#include "SVSettings.h"
#include "Vic3DConstants.h"
#include "SVViewStateHandler.h"

namespace Vic3D {

SceneView::SceneView() :
	m_inputEventReceived(false)
{
	// tell keyboard handler to monitor certain keys - only those needed for scene navigation
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_W);
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_A);
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_S);
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_D);
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_Q);
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_E);
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_R);
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_F);
	m_keyboardMouseHandler.addRecognizedKey(Qt::Key_Shift);

	// *** create scene (no OpenGL calls are being issued below, just the data structures are created.

	m_shaderPrograms.resize(NUM_SHADER_PROGRAMS);

	// Shaderprogram : grid (painting grid lines)
	ShaderProgram grid(":/shaders/grid.vert",":/shaders/grid.frag");
	grid.m_uniformNames.append("worldToView"); // mat4
	grid.m_uniformNames.append("gridColor"); // vec3
	grid.m_uniformNames.append("backColor"); // vec3
	grid.m_uniformNames.append("farplane"); // float
	m_shaderPrograms[SHADER_GRID] = grid;

	// Shaderprogram : regular geometry (opaque geometry with lighting)
	ShaderProgram blocks(":/shaders/VertexNormalColor.vert",":/shaders/phong_lighting.frag");
	blocks.m_uniformNames.append("worldToView");
	blocks.m_uniformNames.append("lightPos");
	blocks.m_uniformNames.append("lightColor");
	blocks.m_uniformNames.append("viewPos");
	m_shaderPrograms[SHADER_OPAQUE_GEOMETRY] = blocks;

	// Shaderprogram : simple lines with uniform/fixed color, but additional model2world transformation matrix
	ShaderProgram lines(":/shaders/VertexWithTransform.vert",":/shaders/fixed_color.frag");
	lines.m_uniformNames.append("worldToView");
	lines.m_uniformNames.append("modelToWorld");
	lines.m_uniformNames.append("fixedColor");
	m_shaderPrograms[SHADER_LINES] = lines;

	// Shaderprogram : vertices with position and color, additional modelToWorld transformation matrix
	ShaderProgram coordSystem(":/shaders/VertexNormalColorWithTransform.vert",":/shaders/phong_lighting.frag");
	coordSystem.m_uniformNames.append("worldToView");
	coordSystem.m_uniformNames.append("lightPos");
	coordSystem.m_uniformNames.append("lightColor");
	coordSystem.m_uniformNames.append("viewPos");
	coordSystem.m_uniformNames.append("modelToWorld");
	m_shaderPrograms[SHADER_COORDINATE_SYSTEM] = coordSystem;

	// Shaderprogram : transparent planes / opaque geometry without lighting
	ShaderProgram transparentGeo(":/shaders/VertexNormalColor.vert",":/shaders/simple.frag");
	transparentGeo.m_uniformNames.append("worldToView");
	m_shaderPrograms[SHADER_TRANSPARENT_GEOMETRY] = transparentGeo;

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SceneView::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SceneView::onViewStateChanged);
}


SceneView::~SceneView() {
	if (m_context) {
		m_context->makeCurrent(this);

		for (ShaderProgram & p : m_shaderPrograms)
			p.destroy();

		m_mainScene.destroy();

		delete m_screenShotMultiSampleFrameBuffer;
		delete m_screenShotDownSampleFrameBuffer;
	}
}


void SceneView::dumpScreenshot(const QString & imgFilePath) {
	if (m_screenShotMultiSampleFrameBuffer == nullptr)
		return; // not initialized yet
	// store current scene size
	QSize sceneSize = m_mainScene.currentSize();

	// resize main scene
	m_mainScene.resize(m_screenShotMultiSampleFrameBuffer->size().width(), m_screenShotMultiSampleFrameBuffer->size().height(), 1);

	// make framebuffer (for multi-sample rendering) active
	m_screenShotMultiSampleFrameBuffer->bind();
	// clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render *only* main scene (grid, opaque planes, ...)
	m_mainScene.render();

	// release framebuffer again
	m_screenShotMultiSampleFrameBuffer->bindDefault();

	// restore main scene's viewport
	m_mainScene.resize(sceneSize.width(), sceneSize.height(), devicePixelRatio());

	// now downsample the framebuffers color buffer (all other buffers are not needed for the screenshot)
	QOpenGLFramebufferObject::blitFramebuffer(
		m_screenShotDownSampleFrameBuffer, m_screenShotMultiSampleFrameBuffer,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// finally dump buffer to file
	QImage screenShot = m_screenShotDownSampleFrameBuffer->toImage();
	screenShot.save(imgFilePath);
}


void SceneView::onModified(int modificationType, ModificationInfo * data) {
	// relay change notification to scene objects
	m_mainScene.onModified(modificationType, data);

	SVProjectHandler::ModificationTypes mod = (SVProjectHandler::ModificationTypes)modificationType;
	switch (mod) {
		case SVProjectHandler::AllModified :
			m_mainScene.updateWorld2ViewMatrix(); // reposition camera
		break;
		case SVProjectHandler::GridModified :
			resizeGL(width(), height());
		break;
		default:; // nothing to do for other modification events

	}
	// finally render
	renderLater();
}


void SceneView::onStyleChanged() {
	// notify grid and selection objects to rebuild their appearance
	m_mainScene.onModified(SVProjectHandler::GridModified, nullptr);

	// need double-painting on Linux/Mac (first in back-buffer than on screen)
	renderNow();
	renderLater();
}


void SceneView::onViewStateChanged() {
	// tell scene view about the viewstate change
	m_mainScene.setViewState(SVViewStateHandler::instance().viewState());

	// and call renderLater(), to update the view when geometry modification is through or objects have been added
	renderLater();
}


void SceneView::initializeGL() {
	FUNCID(SceneView::initializeGL);
	try {
		// initialize shader programs
		for (ShaderProgram & p : m_shaderPrograms) {
			try {
				p.create();
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString("Error creating/compiling shaders %1 and %2")
									 .arg(p.m_vertexShaderFilePath.toStdString())
									 .arg(p.m_fragmentShaderFilePath.toStdString()), FUNC_ID);

			}
		}

		// initialize scenes to draw

		m_mainScene.create(this, m_shaderPrograms);

		int thumbnailWidth = (int)SVSettings::instance().m_thumbNailSize;
		int thumbnailHeight = (int)(thumbnailWidth*0.75);

		// create a multisample-framebuffer, that's where we render into
		QOpenGLFramebufferObjectFormat muliSampleFormat;
		muliSampleFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		muliSampleFormat.setMipmap(true);
		muliSampleFormat.setSamples(4);
		muliSampleFormat.setTextureTarget(GL_TEXTURE_2D);
		muliSampleFormat.setInternalTextureFormat(GL_RGBA32F_ARB);
		m_screenShotMultiSampleFrameBuffer = new QOpenGLFramebufferObject(thumbnailWidth, thumbnailHeight, muliSampleFormat);

		// create framebuffer for downsampling - from this buffer we create the actual screenshot image file
		// Mind: dimensions of both buffers must be exactly the same, for the blitbuffer-operation to work
		QOpenGLFramebufferObjectFormat downSampledFormat;
		downSampledFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
		downSampledFormat.setMipmap(true);
		downSampledFormat.setTextureTarget(GL_TEXTURE_2D);
		downSampledFormat.setInternalTextureFormat(GL_RGBA32F_ARB);
		m_screenShotDownSampleFrameBuffer = new QOpenGLFramebufferObject(thumbnailWidth, thumbnailHeight, downSampledFormat);

		m_screenShotMultiSampleFrameBuffer->bindDefault();

#ifdef Q_OS_MAC
		glEnable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(0xFFFF);
#else
		glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
#endif // Q_OS_MAC

#ifdef SHOW_TIMINGS
		// create timer
		m_gpuTimers.setSampleCount(2);
		m_gpuTimers.create();
#endif // SHOW_TIMINGS

		onStyleChanged();

		onModified(SVProjectHandler::AllModified, nullptr);
	}
	catch (OpenGLException & ex) {
		throw OpenGLException(ex, "OpenGL initialization failed.", FUNC_ID);
	}
}


void SceneView::resizeGL(int width, int height) {
	m_mainScene.resize(width, height, devicePixelRatio());
}


void SceneView::paintGL() {
	m_cpuTimer.start();
	if (((SVDebugApplication *)qApp)->m_aboutToTerminate)
		return;

	// process input, i.e. check if any keys have been pressed
	bool needRepaint = true;
	if (m_inputEventReceived) {
		// if paintGl was called because of an input event,
		// only repaint if needed
		needRepaint = processInput();
	}

#ifdef SHOW_TIMINGS
	m_gpuTimers.reset();
	m_gpuTimers.recordSample();
#endif // SHOW_TIMINGS

	// clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render main scene (grid, opaque planes, ...)
	m_mainScene.render();

	qint64 elapsedMs = m_cpuTimer.elapsed();
//	qDebug() << ++m_paintCounter << "Total paintGL time: " << elapsedMs << "ms";

	// Done painting

#ifdef SHOW_TIMINGS
	m_gpuTimers.recordSample();
#endif // SHOW_TIMINGS


	// Check for some changes in the input handler, which in turn may restart the paint loop,
	// to avoid excessive CPU load when polling input, we only check for
	// input when holding a key (WASD etc.) requires this.
	if (m_keyboardMouseHandler.anyKeyDown())
		checkInput();

#ifdef SHOW_TIMINGS
	QVector<GLuint64> intervals = m_gpuTimers.waitForIntervals();
	for (GLuint64 it : intervals)
		qDebug() << "  " << it*1e-6 << "ms/frame";
	QVector<GLuint64> samples = m_gpuTimers.waitForSamples();
	Q_ASSERT(!samples.isEmpty());
	qDebug() << "Total render time: " << (samples.back() - samples.front())*1e-6 << "ms/frame";

#endif
}


void SceneView::keyPressEvent(QKeyEvent *event) {
	m_keyboardMouseHandler.keyPressEvent(event);
	// in place vertex mode, filter out number keys - we do this right at the moment that
	// the key is pressed, since afterwards we switch focus to the coordinate line edit
	// which handles repeating keys
	if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_PlaceVertex) {
		Qt::Key k = static_cast<Qt::Key>(event->key());
		if ((k >= Qt::Key_0 && k <= Qt::Key_9) ||
			k == Qt::Key_Comma || k == Qt::Key_Period ||
			k == Qt::Key_Space || k == Qt::Key_Return ||
			k == Qt::Key_Backspace || k == Qt::Key_Minus )
		{
			emit numberKeyPressed(k);
		}
	}
	checkInput();
}

void SceneView::keyReleaseEvent(QKeyEvent *event) {
	qDebug() << "SceneView::keyReleaseEvent";
	m_keyboardMouseHandler.keyReleaseEvent(event);
	checkInput();

	// handle everything that are not scene navigation keys
	Qt::Key k = static_cast<Qt::Key>(event->key());
	switch (k) {

		// *** Escape ***
		case Qt::Key_Escape : {
			// different operation depending on scene's operation mode
			switch (SVViewStateHandler::instance().viewState().m_sceneOperationMode) {

				// *** place a vertex ***
				case SVViewState::OM_PlaceVertex : {
					// abort "place vertex" operation
					// reset new polygon object, so that it won't be drawn anylonger
					SVViewStateHandler::instance().m_newGeometryObject->clear();
					// signal, that we are no longer in "add vertex" mode
					SVViewState vs = SVViewStateHandler::instance().viewState();
					vs.m_sceneOperationMode = SVViewState::NUM_OM;
					vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
					// now tell all UI components to toggle their view state
					SVViewStateHandler::instance().setViewState(vs);
				} break;

				case SVViewState::OM_AlignLocalCoordinateSystem :
					m_mainScene.leaveCoordinateSystemAdjustmentMode(true);
				break;

				default:
					// default mode - Escape clears selection
					m_mainScene.deselectAll();
			}
		} break;


		// *** Enter/Return ***
		case Qt::Key_Return : {
			// different operation depending on scene's operation mode
			switch (SVViewStateHandler::instance().viewState().m_sceneOperationMode) {

				// Note: place vertex mode is ended by "Enter" press through the "coordinate input widget" in the
				//       geometry view's toolbar - either with coordinates, or without, there the polygon
				//       is finished (if possible, otherwise an error message pops up)

				// *** align coordinate system ***
				case SVViewState::OM_AlignLocalCoordinateSystem : {
					m_mainScene.leaveCoordinateSystemAdjustmentMode(true);
				} break;

				default:; // in all other modes, Enter has no effect (for now)
			}
		} break;


		// *** F3 - toggle "snap mode" mode ****
		case Qt::Key_F3 : {
			SVViewState vs = SVViewStateHandler::instance().viewState();
			if (vs.m_snapEnabled) {
				vs.m_snapEnabled = false;
				qDebug() << "Snap turned off";
			}
			else {
				vs.m_snapEnabled = true;
				qDebug() << "Snap turned on";
			}
			SVViewStateHandler::instance().setViewState(vs);
			// Nothing further to be done - the coordinate system position is adjusted below for
			// all view modes that require snapping
		} break;


		// *** F4 - toggle "align coordinate system" mode ****
		case Qt::Key_F4 : {
			SVViewState vs = SVViewStateHandler::instance().viewState();
			if (vs.m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem)
				m_mainScene.leaveCoordinateSystemAdjustmentMode(true);
			else
				m_mainScene.enterCoordinateSystemAdjustmentMode();
		} break;


			// *** X,Y,Z locks - only in "place vertex" mode and transform modes ***
		case Qt::Key_X :
		case Qt::Key_Y :
		case Qt::Key_Z :
		{
			SVViewState vs = SVViewStateHandler::instance().viewState();
			if (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex) {
				if (k == Qt::Key_X)
					vs.m_locks ^= SVViewState::L_LocalX;
				else if (k == Qt::Key_Y)
					vs.m_locks ^= SVViewState::L_LocalY;
				else if (k == Qt::Key_Z)
					vs.m_locks ^= SVViewState::L_LocalZ;
				qDebug() << "Locks: " << (vs.m_locks & SVViewState::L_LocalX) << (vs.m_locks & SVViewState::L_LocalY) << (vs.m_locks & SVViewState::L_LocalZ);
			}
			SVViewStateHandler::instance().setViewState(vs);
			// Nothing further to be done - the coordinate system position is adjusted below for
			// all view modes that require snapping

		} break;

		// *** Delete selected geometry ***
		case Qt::Key_Delete : {
			m_mainScene.deleteSelected();
		} break;

		default :; // ignore the rest
	} // switch

}

void SceneView::mousePressEvent(QMouseEvent *event) {
	m_keyboardMouseHandler.mousePressEvent(event);
	checkInput();
}

void SceneView::mouseReleaseEvent(QMouseEvent *event) {
	m_keyboardMouseHandler.mouseReleaseEvent(event);
	checkInput();
}

void SceneView::mouseMoveEvent(QMouseEvent * /*event*/) {
	checkInput();
}

void SceneView::wheelEvent(QWheelEvent *event) {
	m_keyboardMouseHandler.wheelEvent(event);
	checkInput();
}


void SceneView::checkInput() {
	// this function is called from the Qt event look whenever _any_ key/mouse event was issued
	// we now check for any condition that might require a repaint

	// special handling for moving coordinate system (only during place vertex mode, since this will
	// cause the scene to update at monitor refresh rate)
	if (SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_PlaceVertex ||
		SVViewStateHandler::instance().viewState().m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem)
	{
		m_inputEventReceived = true;
		renderLater();
		return;
	}

	// check if any of the know keys is held/was pressed
	if (m_keyboardMouseHandler.anyKeyDown()) {
		m_inputEventReceived = true;
		//			qDebug() << "SceneView::checkInput() inputEventReceived";
		renderLater();
		return;
	}

	// has the mouse been moved while the right button was held (first-person controller)?
	if ( m_keyboardMouseHandler.buttonDown(Qt::RightButton)) {
		m_inputEventReceived = true;
		//			qDebug() << "SceneView::checkInput() inputEventReceived: " << QCursor::pos() << m_keyboardMouseHandler.mouseDownPos();
		renderLater();
		return;
	}

	// is the left mouse butten been held (orbit controller) or has it been released (left-mouse-button click)?
	if (m_keyboardMouseHandler.buttonDown(Qt::LeftButton) ||
		m_keyboardMouseHandler.buttonReleased(Qt::LeftButton))
	{
		m_inputEventReceived = true;
		renderLater();
		return;
	}

	// is the middle mouse butten been held (translate camera)?
	if (m_keyboardMouseHandler.buttonDown(Qt::MidButton)){
		m_inputEventReceived = true;
		renderLater();
		return;
	}

	// scroll-wheel turned?
	if (m_keyboardMouseHandler.wheelDelta() != 0) {
		m_inputEventReceived = true;
		renderLater();
		return;
	}
}


bool SceneView::processInput() {
	// function must only be called if an input event has been received
	Q_ASSERT(m_inputEventReceived);
	m_inputEventReceived = false;
//	qDebug() << "SceneView::processInput()";

	// here, we check for registered key/mouse action combinations
	// these are configured as follows:
	// - mouse key down
	// - mouse key up
	// - mouse key held + mouse move
	// - key press
	//
	// - each event may have a modifier key associated

	// we now loop through all registered key events and call the associated actions


	// for now, delegate the call to the scene object, so that it can alter it's camera position
	QPoint localMousePos;
	if (m_keyboardMouseHandler.buttonReleased(Qt::LeftButton))
		localMousePos = mapFromGlobal(m_keyboardMouseHandler.mouseReleasePos());
	else
		localMousePos = mapFromGlobal(m_keyboardMouseHandler.mouseDownPos());
	QPoint newLocalMousePos;

	bool needRepaint = m_mainScene.inputEvent(m_keyboardMouseHandler, localMousePos, newLocalMousePos);

	if (localMousePos != newLocalMousePos) {
		QCursor c = cursor();
		c.setPos(mapToGlobal(newLocalMousePos));
		setCursor(c);
	}

	// resets the internal position for the next move and wheel scroll
	m_keyboardMouseHandler.resetMouseDelta(QCursor::pos());
	m_keyboardMouseHandler.resetWheelDelta();

	// finally, reset "WasPressed" key states
	m_keyboardMouseHandler.clearWasPressedKeyStates();

	// not need to request update here, since we are called from paint anyway
	return needRepaint;
}



} // namespace Vic3D
