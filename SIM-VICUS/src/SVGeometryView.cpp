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
#include "ui_SVGeometryView.h"

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

#include <VICUS_Project.h>

#include "Vic3DSceneView.h"
#include "SVPropertyWidget.h"
#include "SVPropVertexListWidget.h"
#include "SVPropEditGeometry.h"
#include "SVViewStateHandler.h"
#include "SVLocalCoordinateView.h"
#include "Vic3DNewGeometryObject.h"
#include "SVProjectHandler.h"

SVGeometryView::SVGeometryView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVGeometryView)
{
	m_ui->setupUi(this);

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

	// replace dummy widget with scene view container widget
	m_ui->sceneVBoxLayout->insertWidget(0, m_sceneViewContainerWidget);
	delete m_ui->widgetSceneViewDummy;

	// Layouting
	m_ui->sceneVBoxLayout->setMargin(0);
	m_ui->sceneVBoxLayout->setSpacing(0);
	m_ui->horizontalLayout->setSpacing(0);
	m_ui->splitter->setCollapsible(0, true);
	m_ui->splitter->setCollapsible(1, true);
	m_ui->splitter->setStretchFactor(0,1);
	m_ui->splitter->setStretchFactor(1,0);


	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_sceneViewContainerWidget->setFocusPolicy(Qt::StrongFocus); // we want to get all keyboard/mouse events

	// add special widgets to tool bars (things we cannot do in Qt Creator)
	setupToolBar();

	// *** create Measurement Widget
	m_measurementWidget = new SVMeasurementWidget(this); // sets the pointer to the widget in SVViewStateHandler::instance()

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVGeometryView::onViewStateChanged);
	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVGeometryView::onModified);

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
			if (!m_ui->actionXLock->isVisible())
				return false;
			m_ui->actionXLock->trigger();
		break;

		case Qt::Key_Y :
			if (!m_ui->actionYLock->isVisible())
				return false;
			m_ui->actionYLock->trigger();
		break;

		case Qt::Key_Z :
			if (!m_ui->actionZLock->isVisible())
				return false;
			m_ui->actionZLock->trigger();
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

		// *** C - toggle parametrization and geometry mode ***
		case Qt::Key_C :
			if (m_ui->actionToggleGeometryMode->isChecked())
				switch2ParametrizationMode();
			else
				switch2GeometryMode();
		break;

		default:
			return false; // not our key
	}
	return true;
}


void SVGeometryView::moveMeasurementWidget() {
	const QPoint &point = m_sceneViewContainerWidget->mapToGlobal(m_sceneViewContainerWidget->rect().bottomRight() );
	SVViewStateHandler::instance().m_measurementWidget->setPosition(point);
}


void SVGeometryView::switch2GeometryMode() {
	m_ui->actionToggleGeometryMode->trigger();
}


void SVGeometryView::switch2ParametrizationMode() {
	m_ui->actionToggleParametrizationMode->trigger();
}


void SVGeometryView::onModified(int modificationType, ModificationInfo *) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::NodeStateModified: {
			// update our selection lists
			std::set<const VICUS::Object*> sel;

			// first we get how many surfaces are selected
			project().selectObjects(sel, VICUS::Project::SG_All, true, true);
			bool haveSurface = false;
			for (const VICUS::Object* o : sel) {
				if (dynamic_cast<const VICUS::Surface*>(o) != nullptr ||
					dynamic_cast<const VICUS::SubSurface*>(o) != nullptr)
				{
					haveSurface = true;
					break;
				}
			}
			m_ui->actionTranslateGeometry->setEnabled(haveSurface);
			m_ui->actionRotateGeometry->setEnabled(haveSurface);
			m_ui->actionScaleGeometry->setEnabled(haveSurface);
			m_ui->actionAlignGeometry->setEnabled(haveSurface);
		} break;

		default: ; // just to make compiler happy
	} // switch
}


void SVGeometryView::onViewStateChanged() {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	m_ui->actionSnap->blockSignals(true);
	m_ui->actionSnap->setChecked(vs.m_snapEnabled);
	m_ui->actionSnap->blockSignals(false);

	m_ui->actionMeasure->blockSignals(true);
	m_ui->actionMeasure->setChecked(vs.m_sceneOperationMode == SVViewState::OM_MeasureDistance);
	m_ui->actionMeasure->blockSignals(false);

	m_ui->actionXLock->blockSignals(true);
	m_ui->actionXLock->setChecked(vs.m_locks == SVViewState::L_LocalX);
	m_ui->actionXLock->blockSignals(false);

	m_ui->actionYLock->blockSignals(true);
	m_ui->actionYLock->setChecked(vs.m_locks == SVViewState::L_LocalY);
	m_ui->actionYLock->blockSignals(false);

	m_ui->actionZLock->blockSignals(true);
	m_ui->actionZLock->setChecked(vs.m_locks == SVViewState::L_LocalZ);
	m_ui->actionZLock->blockSignals(false);

	bool lockVisible = (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex);
	m_ui->actionXLock->setVisible(lockVisible);
	m_ui->actionYLock->setVisible(lockVisible);
	m_ui->actionZLock->setVisible(lockVisible);

	bool geometryModeActive = (vs.m_viewMode == SVViewState::VM_GeometryEditMode);
	m_ui->geometryToolBar->setEnabled(geometryModeActive);
	m_ui->actionMeasure->setEnabled(geometryModeActive); // to disable short-cut as well

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
#ifdef POLYGON2D
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
#endif
}


void SVGeometryView::on_actionSnap_toggled(bool on) {
	// switch toggle view state
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_snapEnabled = on;
	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}


void SVGeometryView::on_actionMeasure_toggled(bool on) {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (on && vs.m_sceneOperationMode == SVViewState::OM_MeasureDistance) {
		qDebug() << "Measurement mode is already on, yet button is off...";
		return;
	}
	m_sceneView->toggleMeasurementMode();
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


void SVGeometryView::on_actionToggleGeometryMode_triggered() {
	// switch view state to geometry edit mode
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_viewMode = SVViewState::VM_GeometryEditMode;
	std::set<const VICUS::Object *> sel;
	project().selectObjects(sel, VICUS::Project::SG_All, true, true);
	// we choose the operation based on the selection state
	if (sel.empty()) {
		vs.m_sceneOperationMode = SVViewState::NUM_OM;
		vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
	}
	else {
		vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
	}
	vs.m_objectColorMode = SVViewState::OCM_None;
	SVViewStateHandler::instance().setViewState(vs);

	m_ui->actionToggleGeometryMode->setChecked(true);
	m_ui->actionToggleParametrizationMode->setChecked(false);
}


void SVGeometryView::on_actionToggleParametrizationMode_triggered() {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to property edit mode
	vs.m_viewMode = SVViewState::VM_PropertyEditMode;
	// turn off any special scene modes
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	// select property mode based on what's being selected in the mode selection
	// property widget (this sets m_propertyWidgetMode and m_objectColorMode)
	SVViewStateHandler::instance().setViewState(vs);

	m_ui->actionToggleParametrizationMode->setChecked(true);
	m_ui->actionToggleGeometryMode->setChecked(false);
}


void SVGeometryView::on_actionAddGeometry_triggered() {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_AddGeometry ||
		vs.m_viewMode != SVViewState::VM_GeometryEditMode)
	{
		vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
		vs.m_viewMode = SVViewState::VM_GeometryEditMode;
		SVViewStateHandler::instance().setViewState(vs);
	}
}


void SVGeometryView::on_actionTranslateGeometry_triggered() {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.m_viewMode != SVViewState::VM_GeometryEditMode)
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_viewMode = SVViewState::VM_GeometryEditMode;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Translate);
}


void SVGeometryView::on_actionRotateGeometry_triggered() {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.m_viewMode != SVViewState::VM_GeometryEditMode)
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_viewMode = SVViewState::VM_GeometryEditMode;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Rotate);
}


void SVGeometryView::on_actionScaleGeometry_triggered() {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.m_viewMode != SVViewState::VM_GeometryEditMode)
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_viewMode = SVViewState::VM_GeometryEditMode;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Scale);
}


void SVGeometryView::on_actionAlignGeometry_triggered() {
	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.m_viewMode != SVViewState::VM_GeometryEditMode)
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_viewMode = SVViewState::VM_GeometryEditMode;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Align);
}


// *** Protected Functions ***


void SVGeometryView::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event); // call parent's class implementation
	moveMeasurementWidget(); // adjust position of measurement widget
}


// *** Private Functions ***

void SVGeometryView::setupToolBar() {

	// *** Geometry Tool Bar ***

	// the line edit for entering vertex coordinates
	m_lineEditCoordinateInput = new QLineEdit(m_ui->geometryToolBar);
	m_lineEditCoordinateInput->setToolTip(tr("Without axis lock, enter coordinates in format <x> <y> <z>. With axis lock enter only the offset in the respective axis direction."));
	m_actionCoordinateInput = m_ui->geometryToolBar->insertWidget(m_ui->actionZLock, m_lineEditCoordinateInput);
	connect(m_lineEditCoordinateInput, &QLineEdit::returnPressed,
			this, &SVGeometryView::coordinateInputFinished);
	m_lineEditCoordinateInput->setMaximumWidth(400);

	// stretcher
	QWidget * spacerWidget = new QWidget;
	spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_ui->geometryToolBar->addWidget(spacerWidget);

	// the local coordinate system info
	m_localCoordinateSystemView = new SVLocalCoordinateView(this);
	m_actionlocalCoordinateSystemCoordinates = m_ui->geometryToolBar->addWidget(m_localCoordinateSystemView);


	// *** Mode Switching Tool Bar ***

	// initialize view mode buttons
	m_ui->actionToggleGeometryMode->blockSignals(true);
	m_ui->actionToggleGeometryMode->setChecked(true);
	m_ui->actionToggleGeometryMode->blockSignals(false);
	m_ui->actionToggleParametrizationMode->blockSignals(true);
	m_ui->actionToggleParametrizationMode->setChecked(false);
	m_ui->actionToggleParametrizationMode->blockSignals(false);
}


