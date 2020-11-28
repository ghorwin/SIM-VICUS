#include "SVGeometryView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>

#include "Vic3DSceneView.h"
#include "SVPropertyWidget.h"

#if 0
const char * const TOGGLE_BUTTON_STYLE =

"QPushButton {"
"  background-color: #646669;"
"  border: 1px solid #3a3b3f;"
"  color: #F0F0F0;"
"  border-radius: 0px;"
"  padding: 1px 1px 1px 1px;"
"  outline: none;"
"}"

"QPushButton:disabled {"
"  background-color: #3a3b3f;"
"  border: 1px solid #3a3b3f;"
"  color: #787878;"
"  border-radius: 0px;"
"  padding: 1px;"
"}"

"QPushButton:checked {"
"  background-color: #3a3b3f;"
"  border: 1px solid #3a3b3f;"
"  border-radius: 0px;"
"  padding: 1px 1px 1px 1px;"
"  outline: none;"
"}"

"QPushButton:checked:disabled {"
"  background-color: #212124;"
"  border: 1px solid #3a3b3f;"
"  color: #787878;"
"  border-radius: 0px;"
"  padding: 1px 1px 1px 1px;"
"  outline: none;"
"}"

"QPushButton:checked:selected {"
"  background: #a08918;"
"  color: #3a3b3f;"
"}"

"QPushButton::menu-indicator {"
"  subcontrol-origin: padding;"
"  subcontrol-position: bottom right;"
"  bottom: 4px;"
"}"

"QPushButton:pressed {"
"  background-color: #212124;"
"  border: 1px solid #212124;"
"}"

"QPushButton:pressed:hover {"
"  border: 1px solid #b19834;"
"}"

"QPushButton:hover {"
"  border: 1px solid #b19834;"
"  color: #F0F0F0;"
"}"

"QPushButton:selected {"
"  background: #a08918;"
"  color: #3a3b3f;"
"}"

"QPushButton:hover {"
"  border: 1px solid #b19834;"
"  color: #F0F0F0;"
"}"

"QPushButton:focus {"
"  border: 1px solid #a08918;"
"}";
#endif

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

	// *** create property widget and toggle button

	m_propertyWidget = new SVPropertyWidget(this);

#if 0
	m_toggleButton = new QPushButton;
	m_toggleButton->setIcon(QIcon(":/qss_icons/rc/branch_closed_focus.png"));
	m_toggleButton->setMaximumWidth(32);
	m_toggleButton->setMinimumWidth(32);
	m_toggleButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_toggleButton->setBaseSize(QSize(32,100));
	m_toggleButton->setStyleSheet(TOGGLE_BUTTON_STYLE);
#endif


	// *** create splitter and put view and property widget into splitter

	m_splitter = new QSplitter(this);

	m_splitter->addWidget(container);
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
}


void SVGeometryView::saveScreenShot(const QString & imgFilePath) {
	m_sceneView->dumpScreenshot(imgFilePath);
}

