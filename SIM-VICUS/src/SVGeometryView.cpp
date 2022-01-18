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

#include "SVGeometryView.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QToolBar>
#include <QAction>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolButton>
#include <QLine>

#include <IBK_StringUtils.h>

#include "Vic3DSceneView.h"
#include "SVPropertyWidget.h"
#include "SVPropVertexListWidget.h"
#include "SVViewStateHandler.h"
#include "SVLocalCoordinateView.h"
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

	// *** create toolbar and place it below the scene

	m_toolBar = new QToolBar(this);
	m_toolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_toolBar->setMaximumHeight(32);
	m_toolBar->layout()->setMargin(0);

//	m_dockWidget = new QWidget(this);
//	m_dockWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//	m_dockWidget->setMaximumHeight(120);

	QVBoxLayout * vbLay = new QVBoxLayout;
	vbLay->addWidget(m_sceneViewContainerWidget);
	vbLay->addWidget(m_toolBar);
	vbLay->setMargin(0);
	vbLay->setSpacing(0);

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
	m_splitter->setCollapsible(0, true);
	m_splitter->setCollapsible(1, true);
	m_splitter->setStretchFactor(0,1);
	m_splitter->setStretchFactor(1,0);

	setLayout(hlay);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_sceneViewContainerWidget->setFocusPolicy(Qt::StrongFocus); // we want to get all keyboard/mouse events

	setupToolBar();

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVGeometryView::onViewStateChanged);

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
	m_sceneView->renderLater();
}

void SVGeometryView::resetCamera(int positionID) {
	m_sceneView->resetCamera(positionID);
}


bool SVGeometryView::handleGlobalKeyPress(Qt::Key k) {
	switch (k) {
		case Qt::Key_0 :
		case Qt::Key_1 :
		case Qt::Key_2 :
		case Qt::Key_3 :
		case Qt::Key_4 :
		case Qt::Key_5 :
		case Qt::Key_6 :
		case Qt::Key_7 :
		case Qt::Key_9 :
		case Qt::Key_Comma :
		case Qt::Key_Period :
		case Qt::Key_Minus :
		case Qt::Key_Backspace :
		case Qt::Key_Enter :
		case Qt::Key_Return :
		case Qt::Key_Space :
			// when we are in an operation, where our coordinate input widget is visible, then we pass on
			// all the keys to this widget
			if (!m_actionCoordinateInput->isVisible())
				return false;
			// for enter and return, the line edit must have focus
			if (k == Qt::Key_Enter || k == Qt::Key_Return) {
				if (!m_lineEditCoordinateInput->hasFocus())
					return false;
			}
			onNumberKeyPressed(k);
		break;

		case Qt::Key_X :
			if (!m_xLockAction->isVisible())
				return false;
			m_xLockAction->trigger();
		break;

		case Qt::Key_Y :
			if (!m_yLockAction->isVisible())
				return false;
			m_yLockAction->trigger();
		break;

		case Qt::Key_Z :
			if (!m_zLockAction->isVisible())
				return false;
			m_zLockAction->trigger();
		break;

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
		case Qt::Key_F4 :
			if (m_actionlocalCoordinateSystemCoordinates->isVisible())
				m_sceneView->toggleAlignCoordinateSystem();
		break;

		// *** F5 - toggle "move local coordinate system" mode ****
		case Qt::Key_F5 :
			if (m_actionlocalCoordinateSystemCoordinates->isVisible())
				m_sceneView->toggleTranslateCoordinateSystem();
		break;

		default:
			return false; // not our key
	}
	return true;
}


void SVGeometryView::onViewStateChanged() {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	m_snapAction->blockSignals(true);
	m_snapAction->setChecked(vs.m_snapEnabled);
	m_snapAction->blockSignals(false);

	m_xLockAction->blockSignals(true);
	m_xLockAction->setChecked(vs.m_locks == SVViewState::L_LocalX);
	m_xLockAction->blockSignals(false);

	m_yLockAction->blockSignals(true);
	m_yLockAction->setChecked(vs.m_locks == SVViewState::L_LocalY);
	m_yLockAction->blockSignals(false);

	m_zLockAction->blockSignals(true);
	m_zLockAction->setChecked(vs.m_locks == SVViewState::L_LocalZ);
	m_zLockAction->blockSignals(false);

	bool lockVisible = (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex);
	m_xLockAction->setVisible(lockVisible);
	m_yLockAction->setVisible(lockVisible);
	m_zLockAction->setVisible(lockVisible);

	// NOTE: you cannot simply hide widgets added to a toolbar. Instead, you must change visibility of
	//       the associated actions.

	m_actionCoordinateInput->setVisible(lockVisible);

	bool localCoordinateSystemVisible =
	(vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex ||
		vs.m_sceneOperationMode == SVViewState::OM_SelectedGeometry ||
		vs.m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem ||
		vs.m_sceneOperationMode == SVViewState::OM_MoveLocalCoordinateSystem);

	if (vs.m_propertyWidgetMode == SVViewState::PM_AddSubSurfaceGeometry)
		localCoordinateSystemVisible = false;

	if (localCoordinateSystemVisible) {
		m_actionlocalCoordinateSystemCoordinates->setVisible(true);
		m_localCoordinateSystemView->setAlignCoordinateSystemButtonChecked(vs.m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem);
		m_localCoordinateSystemView->setMoveCoordinateSystemButtonChecked(vs.m_sceneOperationMode == SVViewState::OM_MoveLocalCoordinateSystem);
	}
	else {
		m_actionlocalCoordinateSystemCoordinates->setVisible(false);
	}
}


void SVGeometryView::onNumberKeyPressed(Qt::Key k) {
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
		case Qt::Key_Return :
		case Qt::Key_Enter : {
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
	// (if possible)

	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	if (m_lineEditCoordinateInput->text().trimmed().isEmpty()) {
		if (po->planeGeometry().isValid()) {
//			SVViewStateHandler::instance().m_propVertexListWidget->on_pushButtonCompletePolygon_clicked();
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
		// we want to allow also numbers with semikolon eg. "14,3" to be parsed
		std::replace(coordinateLine.begin(), coordinateLine.end(), ',', '.');
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

	// if we have a locked axis, the number entered will be taken for the axis.
	IBKMK::Vector3D offset;
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	if (vs.m_locks != SVViewState::NUM_L) {
		if (vec.size() != 1) {
			QMessageBox::critical(this, QString(), tr("Invalid coordinate input, expected only one coordinate for the locked axis."));
			m_lineEditCoordinateInput->setFocus();
			m_lineEditCoordinateInput->selectAll();
			return;
		}
		switch (vs.m_locks) {
			case SVViewState::L_LocalX:				offset.m_x = vec[0]; break;
			case SVViewState::L_LocalY:				offset.m_y = vec[0]; break;
			case SVViewState::L_LocalZ:				offset.m_z = vec[0]; break;
			case SVViewState::NUM_L :; // already checked and handled
		}
	}
	else {
		// without locks, we use the "x y z" coordinate format
		offset.m_x = vec[0];
		if (vec.size() > 1)
			offset.m_y = vec[1];
		if (vec.size() > 2)
			offset.m_z = vec[2];
	}

	// add last vertex, if existing
	if (po->planeGeometry().polygon().vertexes().size() > 0) {
		offset += po->planeGeometry().polygon().vertexes().back();
	}

	// check if adding the vertex would invalidate the polygon
	VICUS::PlaneGeometry p = po->planeGeometry();
	p.addVertex(offset);
	// two vertexes are always valid, so we do not check for valid vertexes then
	if (p.polygon().vertexes().size() > 2 && !p.isValid()) {
		QMessageBox::critical(this, QString(), tr("Adding this vertex would invalidate the polygon."));
		m_lineEditCoordinateInput->setFocus();
		m_lineEditCoordinateInput->selectAll();
		return;
	}

	// now add vertex
	po->appendVertex(offset);

	SVViewStateHandler::instance().m_geometryView->refreshSceneView();
	SVViewStateHandler::instance().m_geometryView->focusSceneView();

	// if successful, clear the input widget
	m_lineEditCoordinateInput->clear();
}


void SVGeometryView::on_actionSnap_toggled(bool on) {
	// switch toggle view state
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_snapEnabled = on;
	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}


void SVGeometryView::on_actionXLock_toggled(bool on) {
	// switch toggle view state
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (on)
		vs.m_locks = SVViewState::L_LocalX;
	else
		vs.m_locks = SVViewState::NUM_L;

	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}


void SVGeometryView::on_actionYLock_toggled(bool on) {
	// switch toggle view state
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (on)
		vs.m_locks = SVViewState::L_LocalY;
	else
		vs.m_locks = SVViewState::NUM_L;

	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}


void SVGeometryView::on_actionZLock_toggled(bool on) {
	// switch toggle view state
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (on)
		vs.m_locks = SVViewState::L_LocalZ;
	else
		vs.m_locks = SVViewState::NUM_L;

	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}


void SVGeometryView::setupToolBar() {

	m_snapAction = new QAction(tr("Snap"), this);
	m_snapAction->setCheckable(true);
	m_snapAction->setToolTip(tr("Toggles object snap on/off (F3)"));
	m_snapAction->setIcon(QIcon(":/gfx/actions/icon-gesture-drag.svg") );

	m_toolBar->addAction(m_snapAction);
	m_toolBar->setIconSize(QSize(24,24) );
	connect(m_snapAction, &QAction::toggled, this, &SVGeometryView::on_actionSnap_toggled);

	m_xLockAction = new QAction(tr("X"), this);
	m_xLockAction->setCheckable(true);
	m_toolBar->addAction(m_xLockAction);
	connect(m_xLockAction, &QAction::toggled, this, &SVGeometryView::on_actionXLock_toggled);

	m_yLockAction = new QAction(tr("Y"), this);
	m_yLockAction->setCheckable(true);
	m_toolBar->addAction(m_yLockAction);
	connect(m_yLockAction, &QAction::toggled, this, &SVGeometryView::on_actionYLock_toggled);

	m_zLockAction = new QAction(tr("Z"), this);
	m_zLockAction->setCheckable(true);
	m_toolBar->addAction(m_zLockAction);
	connect(m_zLockAction, &QAction::toggled, this, &SVGeometryView::on_actionZLock_toggled);

	m_toolBar->addSeparator();

	// the line edit for entering vertex coordinates
	m_lineEditCoordinateInput = new QLineEdit(m_toolBar);
	m_lineEditCoordinateInput->setToolTip(tr("Without axis lock, enter coordinates in format <x> <y> <z>. With axis lock enter only the offset in the respective axis direction."));
	m_actionCoordinateInput = m_toolBar->addWidget(m_lineEditCoordinateInput);
	connect(m_lineEditCoordinateInput, &QLineEdit::returnPressed,
			this, &SVGeometryView::coordinateInputFinished);
	m_lineEditCoordinateInput->setMaximumWidth(400);

	// stretcher
	QWidget * spacerWidget = new QWidget;
	spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_toolBar->addWidget(spacerWidget);

	// the local coordinate system info
	m_localCoordinateSystemView = new SVLocalCoordinateView(this);
	m_actionlocalCoordinateSystemCoordinates = m_toolBar->addWidget(m_localCoordinateSystemView);
}

