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
#include <QEvent>
#include <QKeyEvent>

#include <IBK_StringUtils.h>

#include <VICUS_Project.h>

#include "SVPropVertexListWidget.h"
#include "SVPropEditGeometry.h"
#include "SVViewStateHandler.h"
#include "SVLocalCoordinateView.h"
#include "Vic3DNewGeometryObject.h"
#include "SVProjectHandler.h"
#include "SVColorLegend.h"
#include "SVSnapOptionsDialog.h"


SVGeometryView::SVGeometryView(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVGeometryView)
{
	m_ui->setupUi(this);
	m_ui->mainLayout->setMargin(0);

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
	// Note: despite focus policy, the container widget does not receive focus when we click on the scene
//	m_sceneViewContainerWidget->setFocusPolicy(Qt::StrongFocus); // we want to get all keyboard/mouse events
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

	setFocusPolicy(Qt::StrongFocus); // we want to get all keyboard/mouse events

	// add special widgets to tool bars (things we cannot do in Qt Creator)
	setupToolBar();

	// *** create Measurement Widget
	m_measurementWidget = new SVMeasurementWidget(this); // sets the pointer to the widget in SVViewStateHandler::instance()
	m_colorLegend = new SVColorLegend(this);
	m_snapOptionsDialog = new SVSnapOptionsDialog(this);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVGeometryView::onViewStateChanged);
	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVGeometryView::onModified);

	SVViewStateHandler::instance().m_geometryView = this;
	onViewStateChanged();

	m_focusRootWidgets.insert(this);

	m_snapOptionsDialog->updateUi();
	m_snapOptionsDialog->setExpanded(false);
}


void SVGeometryView::saveScreenShot(const QString & imgFilePath) {
	m_sceneView->dumpScreenshot(imgFilePath);
}


void SVGeometryView::focusSceneView() {
	// m_sceneViewContainerWidget->setFocus();
}


void SVGeometryView::refreshSceneView() {
	m_sceneView->renderLater();
}


void SVGeometryView::resetCamera(Vic3D::SceneView::CameraPosition positionID) {
	m_sceneView->resetCamera(positionID);
}


bool SVGeometryView::handleGlobalKeyPressEvent(QKeyEvent * ke) {
	Qt::Key k = (Qt::Key)ke->key();
//	qDebug() << "SVGeometryView::handleGlobalKeyPress" << k;
	if (m_sceneView->isVisible()) {
		// we only relay the key event if the currently focused widget has one of the acceptable widgets as parent
		const QWidget * fw = qApp->focusWidget();
		bool sendEventToScene = false;
		if (fw == nullptr)
			sendEventToScene = true;
		while (fw)  {
			if (m_focusRootWidgets.contains(fw)) {
				sendEventToScene = true;
				break;
			}
			fw = fw->parentWidget();
		}
		// we always pass the key event if it is a special key
		if (k==Qt::Key_Shift || k==Qt::Key_Control || k==Qt::Key_Alt || k==Qt::Key_AltGr || sendEventToScene)
			m_sceneView->handleKeyPressEvent(ke);
	}
	switch (k) {
		case Qt::Key_0 :
		case Qt::Key_1 :
		case Qt::Key_2 :
		case Qt::Key_3 :
		case Qt::Key_4 :
		case Qt::Key_5 :
		case Qt::Key_6 :
		case Qt::Key_7 :
		case Qt::Key_8 :
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
			m_lineEditCoordinateInput->setText("");
			m_ui->actionXLock->trigger();
			break;

		case Qt::Key_Y :
			if (!m_ui->actionYLock->isVisible())
				return false;
			m_lineEditCoordinateInput->setText("");
			m_ui->actionYLock->trigger();
			break;

		case Qt::Key_Z :
			if (!m_ui->actionZLock->isVisible())
				return false;
			m_lineEditCoordinateInput->setText("");
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
			m_ui->actionSnap->setChecked(vs.m_snapEnabled);
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

		// TODO: shortcuts network and building edit mode

		default:
			return false; // not our key
	}
	return true;
}


bool SVGeometryView::handleGlobalKeyRelease(QKeyEvent * ke) {
//	Qt::Key k = (Qt::Key)ke->key();
//	qDebug() << "SVGeometryView::handleGlobalKeyRelease" << k;

	// avoid accidentally deleting anything from the scene
	if (!this->hasFocus() && ke->key() == Qt::Key_Delete)
		return true;

	// key release events are sent always, as these "do nothing" normally unless the scene was in a special state before
	if (m_sceneView->isVisible())
		m_sceneView->handleKeyReleaseEvent(ke);
	return true;
}


void SVGeometryView::moveTransparentSceneWidgets() {
	// Note: this function is called from resizeEvent() and indirectly from showEvent(), but here the
	//       widget wasn't created, yet. Hence, we need to protect against accessing the pointer.
	if (SVViewStateHandler::instance().m_measurementWidget != nullptr) {
		QPoint point = m_sceneViewContainerWidget->mapToGlobal(m_sceneViewContainerWidget->rect().bottomRight() );
		point.setX(point.x() - 10);
		point.setY(point.y() - 10);
		SVViewStateHandler::instance().m_measurementWidget->setPosition(point);
	}

	if (SVViewStateHandler::instance().m_colorLegend != nullptr) {
		QPoint point = m_sceneViewContainerWidget->mapToGlobal(m_sceneViewContainerWidget->rect().bottomRight() );
		point.setX(point.x() - 15);
		SVViewStateHandler::instance().m_colorLegend->setPosition(m_sceneViewContainerWidget->rect().height(), point);
	}

	if (SVViewStateHandler::instance().m_snapOptionsDialog != nullptr) {
		QPoint point = m_sceneViewContainerWidget->mapToGlobal(m_sceneViewContainerWidget->rect().bottomLeft() );
		point.setX(point.x() + 10);
		point.setY(point.y() - 10);
		SVViewStateHandler::instance().m_snapOptionsDialog->setPosition(point);
	}
}


void SVGeometryView::hideMeasurementWidget() {
	m_measurementWidget->hide();
	m_measurementWidget->reset();
	m_ui->actionMeasure->setChecked(false);
}

void SVGeometryView::switch2AddGeometry() {
	m_ui->actionAddGeometry->trigger();
}

void SVGeometryView::switch2BuildingParametrization() {
	m_ui->actionBuildingParametrization->trigger();
}

void SVGeometryView::switch2NetworkParametrization() {
	m_ui->actionNetworkParametrization->trigger();
}

void SVGeometryView::uncheckAllActionsInButtonBar() {
	m_ui->actionAddGeometry->setChecked(false);
	m_ui->actionBuildingParametrization->setChecked(false);
	m_ui->actionCopyGeometry->setChecked(false);
	m_ui->actionAlignGeometry->setChecked(false);
	m_ui->actionNetworkParametrization->setChecked(false);
	m_ui->actionRotateGeometry->setChecked(false);
	m_ui->actionScaleGeometry->setChecked(false);
	m_ui->actionSiteParametrization->setChecked(false);
	m_ui->actionTranslateGeometry->setChecked(false);
	m_ui->actionAcousticParametrization->setChecked(false);
	m_ui->actionShowResults->setChecked(false);
}


SVColorLegend * SVGeometryView::colorLegend() {
	return m_colorLegend;
}


void SVGeometryView::onModified(int modificationType, ModificationInfo *) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);

	// toggle network buttons based on wether we have networks in the project
	if (modType == SVProjectHandler::AllModified ||
		modType == SVProjectHandler::NetworkDataChanged ||
		modType == SVProjectHandler::NetworkGeometryChanged) {
		const VICUS::Project &p = project();
		m_ui->actionNetworkParametrization->setEnabled(!p.m_geometricNetworks.empty());
	}

	switch (modType) {
		case SVProjectHandler::AllModified:
		case SVProjectHandler::BuildingGeometryChanged:
		case SVProjectHandler::NodeStateModified: {
			// update our selection lists
			std::set<const VICUS::Object*> sel;

			// first we get how many surfaces are selected
			project().selectObjects(sel, VICUS::Project::SG_All, true, true);
			bool haveSurfaceOrDrawing = false;
			bool haveSubSurface = false;
			bool haveRoom = false;
			bool haveBuildingLevel = false;
			bool haveBuilding = false;
			for (const VICUS::Object* o : sel) {
				if (dynamic_cast<const VICUS::Drawing*>(o) != nullptr) {
					haveSurfaceOrDrawing = true;
					continue;
				}
				if (dynamic_cast<const VICUS::Surface*>(o) != nullptr) {
					haveSurfaceOrDrawing = true;
					continue;
				}
				if (dynamic_cast<const VICUS::SubSurface*>(o) != nullptr) {
					haveSubSurface = true;
					haveSurfaceOrDrawing = true;
					continue;
				}
				if (dynamic_cast<const VICUS::Room*>(o) != nullptr) {
					haveRoom = true;
					continue;
				}
				if (dynamic_cast<const VICUS::BuildingLevel*>(o) != nullptr) {
					haveBuildingLevel = true;
					continue;
				}
				if (dynamic_cast<const VICUS::Building*>(o) != nullptr) {
					haveBuilding = true;
					continue;
				}
			}
			m_ui->actionTranslateGeometry->setEnabled(haveSurfaceOrDrawing);
			m_ui->actionRotateGeometry->setEnabled(haveSurfaceOrDrawing);
			m_ui->actionScaleGeometry->setEnabled(haveSurfaceOrDrawing);
			m_ui->actionAlignGeometry->setEnabled(haveSurfaceOrDrawing);
			m_ui->actionCopyGeometry->setEnabled(haveSurfaceOrDrawing || haveSubSurface || haveRoom || haveBuildingLevel || haveBuilding);
		} break;

		default:;
	} // switch
}


void SVGeometryView::onViewStateChanged() {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	m_ui->actionSnap->blockSignals(true);
	m_ui->actionSnap->setChecked(vs.m_snapEnabled);
	m_ui->actionSnap->blockSignals(false);

	m_ui->actionMeasure->blockSignals(true);
	bool state = vs.m_sceneOperationMode == SVViewState::OM_MeasureDistance;
	m_ui->actionMeasure->setChecked(state);
	m_ui->actionMeasure->blockSignals(false);

	m_ui->actionXLock->blockSignals(true);
	state = vs.m_locks == SVViewState::L_LocalX;
	m_ui->actionXLock->setChecked(state);
	m_ui->actionXLock->blockSignals(false);

	m_ui->actionYLock->blockSignals(true);
	state = vs.m_locks == SVViewState::L_LocalY;
	m_ui->actionYLock->setChecked(state);
	m_ui->actionYLock->blockSignals(false);

	m_ui->actionZLock->blockSignals(true);
	state = vs.m_locks == SVViewState::L_LocalZ;
	m_ui->actionZLock->setChecked(state);
	m_ui->actionZLock->blockSignals(false);

	bool lockVisible = (vs.m_sceneOperationMode == SVViewState::OM_PlaceVertex ||
						vs.m_propertyWidgetMode == SVViewState::PM_EditGeometry ||
						vs.m_sceneOperationMode == SVViewState::OM_MeasureDistance );
	m_ui->actionXLock->setVisible(lockVisible);
	m_ui->actionYLock->setVisible(lockVisible);
	m_ui->actionZLock->setVisible(lockVisible);

	// NOTE: you cannot simply hide widgets added to a toolbar. Instead, you must change visibility of
	//       the associated actions.
	m_actionCoordinateInput->setVisible(lockVisible);

	bool geometryModeActive = vs.m_propertyWidgetMode == SVViewState::PM_EditGeometry ||
							  vs.m_propertyWidgetMode == SVViewState::PM_AddGeometry ||
							  vs.m_propertyWidgetMode == SVViewState::PM_VertexList ||
							  vs.m_propertyWidgetMode == SVViewState::PM_BuildingProperties ||
							  vs.m_propertyWidgetMode == SVViewState::PM_NetworkProperties ;
	m_ui->geometryToolBar->setEnabled(geometryModeActive);
	m_ui->actionMeasure->setEnabled(geometryModeActive); // to disable short-cut as well

	// the local coordinate system is always enabled, except in mode AddSubSurfaceGeometry
	bool localCoordinateSystemEnabled = vs.m_propertyWidgetMode != SVViewState::PM_AddSubSurfaceGeometry;


	if (localCoordinateSystemEnabled) {
		m_actionlocalCoordinateSystemCoordinates->setEnabled(true);
		m_localCoordinateSystemView->setAlignCoordinateSystemButtonChecked(vs.m_sceneOperationMode == SVViewState::OM_AlignLocalCoordinateSystem);
		m_localCoordinateSystemView->setMoveCoordinateSystemButtonChecked(vs.m_sceneOperationMode == SVViewState::OM_MoveLocalCoordinateSystem);
		// reset local coordinate system appearance
		Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
		Q_ASSERT(cso != nullptr);
		if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry) {
			// put local coordinate system back into "plain" mode
			if (cso->m_geometryTransformMode != 0) {
				cso->m_geometryTransformMode = 0;
				// and clear transformation in wireframe object
				Vic3D::WireFrameObject * selectionObject = SVViewStateHandler::instance().m_selectedGeometryObject;
				Q_ASSERT(selectionObject != nullptr);
				selectionObject->resetTransformation();
				SVViewStateHandler::instance().m_geometryView->refreshSceneView();
			}
		}
	}
	else {
		m_actionlocalCoordinateSystemCoordinates->setEnabled(false);
	}

	// hide measurement widget when no longer needed
	if (vs.m_sceneOperationMode != SVViewState::OM_MeasureDistance)
		hideMeasurementWidget();

	// show color legend
	m_colorLegend->setVisible(vs.m_propertyWidgetMode == SVViewState::PM_ResultsProperties);

	// show snap options dialog
	m_snapOptionsDialog->setVisible(vs.m_snapEnabled);
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

	// either, the line edit coordinate input is empty, in which case the polygon object may be completed (if possible)

	Vic3D::NewGeometryObject * po = SVViewStateHandler::instance().m_newGeometryObject;
	if (m_lineEditCoordinateInput->text().trimmed().isEmpty()) {
		if (po->planeGeometry().isValid()) {
			if (!SVViewStateHandler::instance().m_propVertexListWidget->completePolygonIfPossible())
				QMessageBox::critical(this, QString(), tr("Invalid polygon (must be planar and not winding)."));
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

	po->appendVertexOffset(offset);
	// finally clear the coordinate input - next enter press will complete the polygon, if possible
	m_lineEditCoordinateInput->clear();
	m_sceneViewContainerWidget->setFocus();
}



void SVGeometryView::on_actionBuildingParametrization_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionBuildingParametrization->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// show building properties widget
	vs.m_propertyWidgetMode = SVViewState::PM_BuildingProperties;
	// turn off any special scene modes
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	SVViewStateHandler::instance().setViewState(vs);
	// we need to manually update the color mode, since above we reset it to OCM_None.
	// there is no simple way to obtain the color mode from the currently active tool box index in the property widget
	SVViewStateHandler::instance().m_propertyWidget->updateColorMode();
}


void SVGeometryView::on_actionAddGeometry_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionAddGeometry->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
	vs.m_objectColorMode = SVViewState::OCM_None;
	if (SVProjectHandler::instance().isValid()) {
		// we choose the operation based on the selection state
		std::set<const VICUS::Object *> sel;
		project().selectObjects(sel, VICUS::Project::SG_All, true, true);
		if (sel.empty())
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
		else
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
	}
	SVViewStateHandler::instance().setViewState(vs);
}


void SVGeometryView::on_actionTranslateGeometry_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionTranslateGeometry->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.inPropertyEditingMode())
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_objectColorMode = SVViewState::OCM_None;
		// we choose the operation based on the selection state
		std::set<const VICUS::Object *> sel;
		project().selectObjects(sel, VICUS::Project::SG_All, true, true);
		if (sel.empty())
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
		else
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Translate);
}


void SVGeometryView::on_actionRotateGeometry_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionRotateGeometry->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.inPropertyEditingMode())
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_objectColorMode = SVViewState::OCM_None;
		// we choose the operation based on the selection state
		std::set<const VICUS::Object *> sel;
		project().selectObjects(sel, VICUS::Project::SG_All, true, true);
		if (sel.empty())
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
		else
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Rotate);
}


void SVGeometryView::on_actionScaleGeometry_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionScaleGeometry->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.inPropertyEditingMode())
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_objectColorMode = SVViewState::OCM_None;
		// we choose the operation based on the selection state
		std::set<const VICUS::Object *> sel;
		project().selectObjects(sel, VICUS::Project::SG_All, true, true);
		if (sel.empty())
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
		else
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Scale);
}


void SVGeometryView::on_actionAlignGeometry_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionAlignGeometry->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.inPropertyEditingMode())
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_objectColorMode = SVViewState::OCM_None;
		// we choose the operation based on the selection state
		std::set<const VICUS::Object *> sel;
		project().selectObjects(sel, VICUS::Project::SG_All, true, true);
		if (sel.empty())
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
		else
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Align);
}


void SVGeometryView::on_actionCopyGeometry_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionCopyGeometry->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// switch to geometry mode, show addGeometry property widget
	if (vs.m_propertyWidgetMode != SVViewState::PM_EditGeometry ||
		vs.inPropertyEditingMode())
	{
		vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
		vs.m_objectColorMode = SVViewState::OCM_None;
		// we choose the operation based on the selection state
		std::set<const VICUS::Object *> sel;
		project().selectObjects(sel, VICUS::Project::SG_All, true, true);
		if (sel.empty())
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
		else
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
		SVViewStateHandler::instance().setViewState(vs);
	}
	Q_ASSERT(SVViewStateHandler::instance().m_propEditGeometryWidget != nullptr);
	SVViewStateHandler::instance().m_propEditGeometryWidget->setModificationType(SVPropEditGeometry::MT_Copy);
}


void SVGeometryView::on_actionAcousticParametrization_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionAcousticParametrization->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// show building properties widget
	vs.m_propertyWidgetMode = SVViewState::PM_BuildingAcousticProperties;
	vs.m_objectColorMode = SVViewState::OCM_AcousticRoomType;
	// turn off any special scene modes
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	SVViewStateHandler::instance().setViewState(vs);
}


void SVGeometryView::on_actionNetworkParametrization_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionNetworkParametrization->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// show building properties widget
	vs.m_propertyWidgetMode = SVViewState::PM_NetworkProperties;
	// turn off any special scene modes
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	SVViewStateHandler::instance().setViewState(vs);
	// we need to manually update the color mode, since above we reset it to OCM_None.
	// there is no simple way to obtain the color mode from the currently active tool box index in the property widget
	SVViewStateHandler::instance().m_propertyWidget->updateColorMode();
}


void SVGeometryView::on_actionSiteParametrization_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionSiteParametrization->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// show building properties widget
	vs.m_propertyWidgetMode = SVViewState::PM_SiteProperties;
	vs.m_objectColorMode = SVViewState::OCM_None;
	// turn off any special scene modes
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	SVViewStateHandler::instance().setViewState(vs);
}


void SVGeometryView::on_actionShowResults_triggered() {
	uncheckAllActionsInButtonBar();
	m_ui->actionShowResults->setChecked(true);

	SVViewState vs = SVViewStateHandler::instance().viewState();
	// show building properties widget
	vs.m_propertyWidgetMode = SVViewState::PM_ResultsProperties;
	// turn off any special scene modes
	vs.m_sceneOperationMode = SVViewState::NUM_OM;
	vs.m_objectColorMode = SVViewState::OCM_ResultColorView;
	SVViewStateHandler::instance().setViewState(vs);
}



// *** Protected Functions ***


void SVGeometryView::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event); // call parent's class implementation
	moveTransparentSceneWidgets(); // adjust position of measurement widget
}


// *** Private Functions ***

void SVGeometryView::setupToolBar() {

	// *** Geometry Tool Bar ***

	// the line edit for entering vertex coordinates
	m_lineEditCoordinateInput = new QLineEdit(m_ui->geometryToolBar);
	m_lineEditCoordinateInput->setToolTip(tr("Without axis lock, enter coordinates in format <x> <y> <z>. With axis lock enter only the offset in the respective axis direction."));
	m_actionCoordinateInput = m_ui->geometryToolBar->addWidget(m_lineEditCoordinateInput);
	connect(m_lineEditCoordinateInput, &QLineEdit::returnPressed,
			this, &SVGeometryView::coordinateInputFinished);
	m_lineEditCoordinateInput->setMaximumWidth(400);
	m_lineEditCoordinateInput->installEventFilter(this);

	// stretcher
	QWidget * spacerWidget = new QWidget;
	spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_ui->geometryToolBar->addWidget(spacerWidget);

	// the local coordinate system info
	m_localCoordinateSystemView = new SVLocalCoordinateView(this);
	m_actionlocalCoordinateSystemCoordinates = m_ui->geometryToolBar->addWidget(m_localCoordinateSystemView);


	// *** Mode Switching Tool Bar ***

	// initialize view mode buttons
	m_ui->actionAddGeometry->blockSignals(true);
	m_ui->actionAddGeometry->setChecked(true);
	m_ui->actionAddGeometry->blockSignals(false);

	// TODO: set other actions unchecked ?
}


void SVGeometryView::on_actionMeasure_triggered(bool on) {
	m_ui->actionMeasure->setChecked(on);
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (on && vs.m_sceneOperationMode == SVViewState::OM_MeasureDistance) {
		qDebug() << "Measurement mode is already on, yet button is off...";
		return;
	}
	m_sceneView->toggleMeasurementMode();
}


void SVGeometryView::on_actionSnap_triggered(bool on) {
	if (on)
		m_snapOptionsDialog->setExpanded(true);
	m_snapOptionsDialog->setVisible(on);
	// switch toggle view state
	m_ui->actionSnap->setChecked(on);
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_snapEnabled = on;
	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}

void SVGeometryView::on_actionXLock_triggered(bool on) {
	// switch toggle view state
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (on)
		vs.m_locks = SVViewState::L_LocalX;
	else
		vs.m_locks = SVViewState::NUM_L;

	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}


void SVGeometryView::on_actionYLock_triggered(bool on) {
	// switch toggle view state
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (on)
		vs.m_locks = SVViewState::L_LocalY;
	else
		vs.m_locks = SVViewState::NUM_L;

	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}


void SVGeometryView::on_actionZLock_triggered(bool on) {
	// switch toggle view state
	SVViewState vs = SVViewStateHandler::instance().viewState();
	if (on)
		vs.m_locks = SVViewState::L_LocalZ;
	else
		vs.m_locks = SVViewState::NUM_L;

	SVViewStateHandler::instance().setViewState(vs);
	focusSceneView();
}

