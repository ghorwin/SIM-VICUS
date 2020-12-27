#include "SVGeometryView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QToolBar>
#include <QAction>

#include "Vic3DSceneView.h"
#include "SVPropertyWidget.h"
#include "SVViewStateHandler.h"

SVGeometryView::SVGeometryView(QWidget *parent) :
	QWidget(parent)
{
	// *** create OpenGL window

	QSurfaceFormat format;
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setVersion(3,3);
	format.setSamples(4);	// enable multisampling (antialiasing)
	format.setDepthBufferSize(24);
#ifdef GL_DEBUG_
	format.setOption(QSurfaceFormat::DebugContext);
#endif // GL_DEBUG

	m_sceneView = new Vic3D::SceneView;
	m_sceneView->setFormat(format);

	// *** create window container widget

	QWidget *container = QWidget::createWindowContainer(m_sceneView);
	container->setFocusPolicy(Qt::TabFocus);
	container->setMinimumSize(QSize(640,400));

	// *** create toolbar and place it above the scene

	m_toolBar = new QToolBar(tr("Geometry tool bar"), this);
	QVBoxLayout * vbLay = new QVBoxLayout;
	vbLay->addWidget(m_toolBar);
	vbLay->addWidget(container);
	vbLay->setMargin(0);

	QWidget* w = new QWidget(this);
	w->setLayout(vbLay);

	// *** create property widget and toggle button

	m_propertyWidget = new SVPropertyWidget(this);

	// *** create splitter and put view and property widget into splitter

	m_splitter = new QSplitter(this);

	m_splitter->addWidget(w);
	m_splitter->addWidget(m_propertyWidget);

	// *** create the layout and insert splitter
	QHBoxLayout * hlay = new QHBoxLayout;
	hlay->setMargin(0);
	hlay->setSpacing(0);
	hlay->addWidget(m_splitter);
	m_splitter->setCollapsible(0, false);
	m_splitter->setCollapsible(1, false);
	m_splitter->setStretchFactor(0,1);
	m_splitter->setStretchFactor(1,0);

	setLayout(hlay);


	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	container->setFocusPolicy(Qt::StrongFocus); // we want to get all keyboard/mouse events

	setupActions();

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVGeometryView::onViewStateChanged);
}


void SVGeometryView::saveScreenShot(const QString & imgFilePath) {
	m_sceneView->dumpScreenshot(imgFilePath);
}


void SVGeometryView::onViewStateChanged() {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	m_snapAction->setChecked(vs.m_snapEnabled);
	m_xLockAction->setChecked(vs.m_locks & SVViewState::L_LocalX);
	m_yLockAction->setChecked(vs.m_locks & SVViewState::L_LocalY);
	m_zLockAction->setChecked(vs.m_locks & SVViewState::L_LocalZ);
	bool lockVisible = (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex);
	m_xLockAction->setVisible(lockVisible);
	m_yLockAction->setVisible(lockVisible);
	m_zLockAction->setVisible(lockVisible);
}


void SVGeometryView::setupActions() {
	m_snapAction = new QAction(tr("Snap"));
	m_snapAction->setCheckable(true);
	m_toolBar->addAction(m_snapAction);

	m_xLockAction = new QAction(tr("X"));
	m_xLockAction->setCheckable(true);
	m_toolBar->addAction(m_xLockAction);
	m_yLockAction = new QAction(tr("Y"));
	m_yLockAction->setCheckable(true);
	m_toolBar->addAction(m_yLockAction);
	m_zLockAction = new QAction(tr("Z"));
	m_zLockAction->setCheckable(true);
	m_toolBar->addAction(m_zLockAction);
}

