#include "SVGeometryView.h"

#include <QVBoxLayout>

#include "Vic3DSceneView.h"


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

	m_sceneView = new SceneView;
	m_sceneView->setFormat(format);

	// *** create window container widget

	QWidget *container = QWidget::createWindowContainer(m_sceneView);
	container->setFocusPolicy(Qt::TabFocus);
	container->setMinimumSize(QSize(640,400));

	// *** create the layout and insert widget container

	QVBoxLayout * vlay = new QVBoxLayout;
	vlay->setMargin(0);
	vlay->setSpacing(0);
	vlay->addWidget(container);

	setLayout(vlay);

	container->setFocus();

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
