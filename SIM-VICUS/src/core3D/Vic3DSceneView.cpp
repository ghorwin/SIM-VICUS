/************************************************************************************

OpenGL with Qt - Tutorial
-------------------------
Autor      : Andreas Nicolai <andreas.nicolai@gmx.net>
Repository : https://github.com/ghorwin/OpenGLWithQt-Tutorial
License    : BSD License,
			 see https://github.com/ghorwin/OpenGLWithQt-Tutorial/blob/master/LICENSE

************************************************************************************/

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

namespace Vic3D {

SceneView::SceneView() :
	m_inputEventReceived(false)
{
	// tell keyboard handler to monitor certain keys
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
	m_shaderPrograms[SHADER_GRID] = grid;

	// Shaderprogram : regular geometry (opaque geometry with lighting)
	ShaderProgram blocks(":/shaders/VertexNormalColor.vert",":/shaders/phong_lighting.frag");
	blocks.m_uniformNames.append("worldToView");
	blocks.m_uniformNames.append("lightPos");
	blocks.m_uniformNames.append("lightColor");
	blocks.m_uniformNames.append("viewPos");
	m_shaderPrograms[SHADER_OPAQUE_GEOMETRY] = blocks;

	// Shaderprogram : simple lines with uniform/fixed color, but additional model2world transformation matrix
	ShaderProgram lines(":/shaders/lines.vert",":/shaders/simple.frag");
	lines.m_uniformNames.append("worldToView");
	lines.m_uniformNames.append("modelToWorld");
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
}


SceneView::~SceneView() {
	if (m_context) {
		m_context->makeCurrent(this);

		for (ShaderProgram & p : m_shaderPrograms)
			p.destroy();

		m_mainScene.destroy();

		m_gpuTimers.destroy();

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


void SceneView::addPolygonVertex(const IBKMK::Vector3D & p) {
	emit polygonPointAdded(p);
}


void SceneView::onModified(int modificationType, ModificationInfo * data) {
	// relay change notification to scene objects
	m_mainScene.onModified(modificationType, data);

	// finally render
	renderLater();
}


void SceneView::onStyleChanged() {
	m_mainScene.setSceneStyle(SVSettings::instance().m_theme == SVSettings::TT_Dark);
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
	if (m_inputEventReceived)
		processInput();
#ifdef SHOW_TIMINGS
	m_gpuTimers.reset();
	m_gpuTimers.recordSample();
#endif // SHOW_TIMINGS

	// clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render main scene (grid, opaque planes, ...)
	m_mainScene.render();


	// Done painting

#ifdef SHOW_TIMINGS
	m_gpuTimers.recordSample();
#endif // SHOW_TIMINGS


	// Check for some changes in the input handler, which in turn may restart the paint loop
	checkInput();

#ifdef SHOW_TIMINGS
	QVector<GLuint64> intervals = m_gpuTimers.waitForIntervals();
	for (GLuint64 it : intervals)
		qDebug() << "  " << it*1e-6 << "ms/frame";
	QVector<GLuint64> samples = m_gpuTimers.waitForSamples();
	Q_ASSERT(!samples.isEmpty());
	qDebug() << "Total render time: " << (samples.back() - samples.front())*1e-6 << "ms/frame";

	qint64 elapsedMs = m_cpuTimer.elapsed();
	qDebug() << "Total paintGL time: " << elapsedMs << "ms";
#endif
}


void SceneView::keyPressEvent(QKeyEvent *event) {
	m_keyboardMouseHandler.keyPressEvent(event);
	checkInput();
}

void SceneView::keyReleaseEvent(QKeyEvent *event) {
	m_keyboardMouseHandler.keyReleaseEvent(event);
	checkInput();
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


void SceneView::pick(const QPoint & globalMousePos) {
#if 0
	// local mouse coordinates
	QPoint localMousePos = mapFromGlobal(globalMousePos);
	int my = localMousePos.y();
	int mx = localMousePos.x();

	// viewport dimensions
	const qreal retinaScale = devicePixelRatio(); // needed for Macs with retina display
	qreal halfVpw = width()*retinaScale/2;
	qreal halfVph = height()*retinaScale/2;

	// invert world2view matrix, with m_worldToView = m_projection * m_camera.toMatrix() * m_transform.toMatrix();
	bool invertible;
	QMatrix4x4 projectionMatrixInverted = m_worldToView.inverted(&invertible);
	if (!invertible) {
		qWarning()<< "Cannot invert projection matrix.";
		return;
	}

	// mouse position in NDC space, one point on near plane and one point on far plane
	QVector4D nearPos(
				(mx - halfVpw) / halfVpw,
				-1*(my - halfVph) / halfVph,
				-1,
				1.0);

	QVector4D farPos(
				nearPos.x(),
				nearPos.y(),
				1,
				1.0);

	// transform from NDC to model coordinates
	QVector4D nearResult = projectionMatrixInverted*nearPos;
	QVector4D farResult = projectionMatrixInverted*farPos;
	// don't forget normalization!
	nearResult /= nearResult.w();
	farResult /= farResult.w();

	// update pick line vertices (visualize pick line)
	m_context->makeCurrent(this);
	m_pickLineObject.setPoints(nearResult.toVector3D(), farResult.toVector3D());

	// now do the actual picking - for now we implement a selection
	selectNearestObject(nearResult.toVector3D(), farResult.toVector3D());
#endif
}


void SceneView::checkInput() {
	// this function is called from the Qt event look whenever _any_ key/mouse event was issued

	// We need to loop over all registered input handlers and check if their conditions have been met.
	// We basically test, if the current state of the key handler requires a scene update
	// (camera movement) and if so, we just set a flag to do that upon next repaint
	// and we schedule a repaint. First thing in the paint event we process the input and update camera positions
	// and the like.

	// trigger key held?
		// any of the interesting keys held?
	if (m_keyboardMouseHandler.keyDown(Qt::Key_W) ||
			m_keyboardMouseHandler.keyDown(Qt::Key_A) ||
			m_keyboardMouseHandler.keyDown(Qt::Key_S) ||
			m_keyboardMouseHandler.keyDown(Qt::Key_D) ||
			m_keyboardMouseHandler.keyDown(Qt::Key_Q) ||
			m_keyboardMouseHandler.keyDown(Qt::Key_F) ||
			m_keyboardMouseHandler.keyDown(Qt::Key_R) ||
			m_keyboardMouseHandler.keyDown(Qt::Key_E))
	{
		m_inputEventReceived = true;
		//			qDebug() << "SceneView::checkInput() inputEventReceived";
		renderLater();
		return;
	}

	// special handling for snapping coordinate system
	if (m_mainScene.operationMode() == Vic3DScene::OM_Draw) {
		m_inputEventReceived = true;
		renderLater();
	}

	// has the mouse been moved?
	if ( m_keyboardMouseHandler.buttonDown(Qt::RightButton)) {
		m_inputEventReceived = true;
		//			qDebug() << "SceneView::checkInput() inputEventReceived: " << QCursor::pos() << m_keyboardMouseHandler.mouseDownPos();
		renderLater();
		return;
	}
	// has the left mouse butten been release
	if (m_keyboardMouseHandler.buttonDown(Qt::LeftButton)) {
		m_inputEventReceived = true;
		renderLater();
		return;
	}
	if (m_keyboardMouseHandler.buttonReleased(Qt::LeftButton)) {
		m_inputEventReceived = true;
		renderLater();
		return;
	}

	if(m_keyboardMouseHandler.buttonDown(Qt::MidButton)){
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


void SceneView::processInput() {
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
	m_mainScene.inputEvent(m_keyboardMouseHandler, localMousePos, newLocalMousePos);

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
}



} // namespace Vic3D
