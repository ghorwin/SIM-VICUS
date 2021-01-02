#include "SVGeometryView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QToolBar>
#include <QAction>
#include <QLineEdit>
#include <QMessageBox>

#include <IBK_StringUtils.h>

#include "Vic3DSceneView.h"
#include "SVPropertyWidget.h"
#include "SVPropVertexListWidget.h"
#include "SVViewStateHandler.h"
#include "Vic3DNewGeometryObject.h"

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

	m_sceneViewContainerWidget = QWidget::createWindowContainer(m_sceneView);
	m_sceneViewContainerWidget->setFocusPolicy(Qt::TabFocus);
	m_sceneViewContainerWidget->setMinimumSize(QSize(640,400));

	// *** create toolbar and place it above the scene

	m_toolBar = new QToolBar(this);
	m_toolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_toolBar->setMaximumHeight(32);
	QVBoxLayout * vbLay = new QVBoxLayout;
	vbLay->addWidget(m_toolBar);
	vbLay->addWidget(m_sceneViewContainerWidget);
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

	m_sceneViewContainerWidget->setFocusPolicy(Qt::StrongFocus); // we want to get all keyboard/mouse events

	setupToolBar();

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVGeometryView::onViewStateChanged);

	connect(m_sceneView, &Vic3D::SceneView::numberKeyPressed,
			this, &SVGeometryView::onNumberKeyPressed);

	SVViewStateHandler::instance().m_geometryView = this;
	onViewStateChanged();
}


void SVGeometryView::saveScreenShot(const QString & imgFilePath) {
	m_sceneView->dumpScreenshot(imgFilePath);
}


void SVGeometryView::focusSceneView() {
	m_sceneViewContainerWidget->setFocus();
}


void SVGeometryView::refreshSceneView() {
	m_sceneView->renderNow();
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
	m_actionCoordinateInput->setVisible(lockVisible);
}


void SVGeometryView::onNumberKeyPressed(Qt::Key k) {
	qDebug() << k;
	QString text = m_lineEditCoordinateInput->text();
	switch (k) {
		case Qt::Key_0 : text += "0"; break;
		case Qt::Key_1 : text += "1"; break;
		case Qt::Key_2 : text += "2"; break;
		case Qt::Key_3 : text += "3"; break;
		case Qt::Key_4 : text += "4"; break;
		case Qt::Key_5 : text += "5"; break;
		case Qt::Key_6 : text += "6"; break;
		case Qt::Key_7 : text += "7"; break;
		case Qt::Key_8 : text += "8"; break;
		case Qt::Key_9 : text += "9"; break;
		case Qt::Key_Minus : text += "-"; break;
		case Qt::Key_Comma : text += ","; break;
		case Qt::Key_Period : text += "."; break;
		case Qt::Key_Space : text += " "; break;
		case Qt::Key_Backspace : {
			if (text.length()>0)
				text = text.left(text.length()-1);
		}
		break;
		case Qt::Key_Return : {
			coordinateInputFinished();
			return;
		}
		default: return;
	}
	m_lineEditCoordinateInput->setText(text);
	m_lineEditCoordinateInput->setFocus(); // shift focus to edit line - since user apparently wants to enter coordinates
}


void SVGeometryView::coordinateInputFinished() {
	// either, the line edit coordinate input is empty, in which case the polygon object may be completed
	// (if possible); or if in zoneFloor mode, the zoneFloor mode is completed

	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	if (m_lineEditCoordinateInput->text().trimmed().isEmpty()) {
		if (po->planeGeometry().isValid()) {
			if (po->newGeometryMode() == Vic3D::NewGeometryObject::NGM_ZoneFloor) {
				SVViewStateHandler::instance().m_propVertexListWidget->on_pushButtonFloorDone_clicked();
			}
			else
				po->finish();
		}
		else
			QMessageBox::critical(this, QString(), tr("Invalid polygon (must be planar and not winding)."));
		return;
	}

	// otherwise, if line edit is not empty, we parse it and if it is valid, we place a new vertex based on
	// the entered coordinates

	std::string coordinateLine = m_lineEditCoordinateInput->text().trimmed().toStdString();
	std::vector<double> vec;
	try {
		IBK::string2valueVector(coordinateLine, vec);
		if (vec.size() == 0)
			throw IBK::Exception("","");
	}
	catch (...) {
		QMessageBox::critical(this, QString(), tr("Invalid number format. Cannot parse coordinate inputs."));
		m_lineEditCoordinateInput->setFocus();
		m_lineEditCoordinateInput->selectAll();
		return;
	}

	IBKMK::Vector3D offset;
	offset.m_x = vec[0];
	if (vec.size() > 1)
		offset.m_y = vec[1];
	if (vec.size() > 2)
		offset.m_z = vec[2];

	// add last vertex, if existing
	if (po->planeGeometry().vertexes().size() > 0) {
		offset += po->planeGeometry().vertexes().back();
	}

	// check if adding the vertex would invalidate the polygon
	VICUS::PlaneGeometry p = po->planeGeometry();
	p.addVertex(offset);
	// two vertexes are always valid, so we do not check for valid vertexes then
	if (p.vertexes().size() > 2 && !p.isValid()) {
		QMessageBox::critical(this, QString(), tr("Adding this vertex would invalidate the polygon."));
		m_lineEditCoordinateInput->setFocus();
		m_lineEditCoordinateInput->selectAll();
		return;
	}

	// now add vertex
	po->appendVertex(offset);

	SVViewStateHandler::instance().m_geometryView->refreshSceneView();

	// if successful, clear the input widget
	m_lineEditCoordinateInput->clear();
}


void SVGeometryView::setupToolBar() {
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

	m_toolBar->addSeparator();

	m_lineEditCoordinateInput = new QLineEdit(m_toolBar);
	m_actionCoordinateInput = m_toolBar->addWidget(m_lineEditCoordinateInput);
	connect(m_lineEditCoordinateInput, &QLineEdit::returnPressed,
			this, &SVGeometryView::coordinateInputFinished);

//	QWidget* stretch = new QWidget();
//	stretch->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
//	m_toolBar->addWidget(stretch);
}


