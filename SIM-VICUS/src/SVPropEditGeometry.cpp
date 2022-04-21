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

#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

#include <IBK_physics.h>

#include <IBKMK_3DCalculations.h>
#include <IBKMK_Quaternion.h>

#include <QtExt_ValidatingLineEdit.h>
#include <QtExt_ValidatingInputBase.h>

#include <VICUS_Project.h>
#include <VICUS_Object.h>
#include <VICUS_BuildingLevel.h>
#include <VICUS_Polygon3D.h>
#include <VICUS_utilities.h>

#include <QMessageBox>
#include <QQuaternion>

#include <SV_Conversions.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVUndoModifySurfaceGeometry.h"
#include "SVUndoAddSurface.h"
#include "SVUndoAddZone.h"
#include "SVUndoCopyZones.h"
#include "SVUndoCopySurfaces.h"
#include "SVUndoCopySubSurfaces.h"
#include "SVPropVertexListWidget.h"
#include "SVGeometryView.h"
#include "SVPropAddWindowWidget.h"
#include "SVLocalCoordinateView.h"

#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DWireFrameObject.h"
#include "Vic3DTransform3D.h"
#include "Vic3DSceneView.h"

#include <QLocale>
#include <QWheelEvent>

/*! helper function to compare two IBKMK vectors */
template <int digits>
bool checkVectors(const IBKMK::Vector3D &v1, const IBKMK::Vector3D &v2 ) {
	return ( IBK::nearly_equal<digits>(v1.m_x, v2.m_x) &&
			 IBK::nearly_equal<digits>(v1.m_y, v2.m_y) &&
			 IBK::nearly_equal<digits>(v1.m_z, v2.m_z) );
}


class LineEditFormater : public QtExt::FormatterBase {
public:
	~LineEditFormater() override;
	QString formatted(double value) override {
		return QString("%L1").arg(value, 0, 'f', 3);
	}
};

// dummy destructor needed to tell compiler to place virtual function table in this object file
LineEditFormater::~LineEditFormater() {}


// *** Widget implementation ***

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);
	m_ui->verticalLayoutMaster->setMargin(0);
	//	m_ui->verticalLayoutPage1->setMargin(0);
	//	m_ui->verticalLayoutPage2->setMargin(0);

	SVViewStateHandler::instance().m_propEditGeometryWidget = this;

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropEditGeometry::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropEditGeometry::onViewStateChanged);

	m_ui->lineEditTranslateX->setFormatter(new LineEditFormater);
	m_ui->lineEditTranslateY->setFormatter(new LineEditFormater);
	m_ui->lineEditTranslateZ->setFormatter(new LineEditFormater);

	m_ui->lineEditScaleX->setup(0, 1E5, tr("A positive value is required here."), false, true);
	m_ui->lineEditScaleY->setup(0, 1E5, tr("A positive value is required here."), false, true);
	m_ui->lineEditScaleZ->setup(0, 1E5, tr("A positive value is required here."), false, true);

	m_ui->lineEditScaleX->setFormatter(new LineEditFormater);
	m_ui->lineEditScaleY->setFormatter(new LineEditFormater);
	m_ui->lineEditScaleZ->setFormatter(new LineEditFormater);

	m_ui->lineEditScaleX->installEventFilter(this);
	m_ui->lineEditScaleY->installEventFilter(this);
	m_ui->lineEditScaleZ->installEventFilter(this);


#if 0
	m_ui->lineEditInclination->installEventFilter(this);
	m_ui->lineEditOrientation->installEventFilter(this);
#endif
}


SVPropEditGeometry::~SVPropEditGeometry() {
	delete m_ui;
}


void SVPropEditGeometry::enableTransformation() {
	m_ui->pushButtonApply->setEnabled(true);
	m_ui->pushButtonCancel->setEnabled(true);
}


void SVPropEditGeometry::setModificationType(ModificationType modType) {
	m_ui->stackedWidget->setCurrentIndex(modType);
	updateUi(); // update all inputs
}


void SVPropEditGeometry::setCoordinates(const Vic3D::Transform3D &t) {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;

	// is being called from local coordinate system object, whenever this has changed location (regardless of
	// its own visibility)
	m_lcsTransform =  t;

	// compute dimensions of bounding box (dx, dy, dz) and center point of all selected surfaces
	m_bbDim[OM_Local] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Local],
											  QVector2IBKVector(cso->translation() ),
											  QVector2IBKVector(cso->localXAxis() ),
											  QVector2IBKVector(cso->localYAxis() ),
											  QVector2IBKVector(cso->localZAxis() ) );
	m_bbDim[OM_Global] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Global]);

	updateInputs();
}


void SVPropEditGeometry::setRotation(const IBKMK::Vector3D &normal) {
	m_normal = normal.normalized();

	m_ui->lineEditInclination->setText( QString("%L1").arg(std::acos(normal.m_z)/IBK::DEG2RAD, 0, 'f', 3) );
	// positive y Richtung = Norden = Orientation 0°
	// positive x Richtung = Osten = Orientation 90°

	double orientation = std::atan2(normal.m_x, ( normal.m_y == 0. ? 1E-8 : normal.m_y ) ) /IBK::DEG2RAD ;
	m_ui->lineEditOrientation->setText( QString("%L1").arg(orientation < 0 ? ( orientation + 360 ) : orientation, 0, 'f', 3 ) );
}


// *** PUBLIC SLOTS ***

void SVPropEditGeometry::onModified(int modificationType, ModificationInfo * ) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
	case SVProjectHandler::AllModified:
	case SVProjectHandler::BuildingGeometryChanged:
	case SVProjectHandler::NodeStateModified:
		// When the building geometry has changed, we need to update the geometrical info
		// in the widget based on the current selection.

		// Also, if the selection has changed, we need to distinguish between no selection and selection
		// and update the buttons accordingly.
		updateUi();
		break;

	default: ; // just to make compiler happy
	}
}


void SVPropEditGeometry::onViewStateChanged() {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	if (vs.m_sceneOperationMode == SVViewState::NUM_OM) {
		SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	}
	else {
//		m_ui->widgetEdit->setEnabled(true);
//		//m_ui->pushButtonEdit->setEnabled(true);
//		updateCoordinateSystemLook();
	}
}



// *** PROTECTED FUNCTIONS ***

bool SVPropEditGeometry::eventFilter(QObject * target, QEvent * event) {

	if ( event->type() == QEvent::Wheel ) {
		// we listen to scroll wheel turns only for some line edits
		if (target == m_ui->lineEditTranslateX ||
				target == m_ui->lineEditTranslateY ||
				target == m_ui->lineEditTranslateZ ||
				target == m_ui->lineEditInclination ||
				target == m_ui->lineEditOrientation ||
				target == m_ui->lineEditScaleX ||
				target == m_ui->lineEditScaleY ||
				target == m_ui->lineEditScaleZ )
		{
			double delta = 0.1; // default
			switch (m_ui->stackedWidget->currentIndex()) {
				case MT_Translate				:
				case MT_Scale					: delta = 0.01; break;
				case MT_Rotate					: delta = 1; break;
				case NUM_MT : ; // just to make compiler happy
			}

			QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
			// offset are changed in 0.01 steps
			double offset = (wheelEvent->delta()>0) ? delta : -delta;
			onWheelTurned(offset, qobject_cast<QtExt::ValidatingLineEdit*>(target)); // we know that target points to a ValidatingLineEdit
		}
	}
//	else if ( event->type() == QEvent::FocusOut ) {
//		QLineEdit *edit = dynamic_cast<QLineEdit*>(target);

//		qDebug() << "Focus out -> return press event on " << edit;
//		if(edit != nullptr)
//			emit edit->returnPressed();
//	}
	return false;
}


// *** PRIVATE SLOTS***

void SVPropEditGeometry::on_radioButtonTranslationAbsolute_toggled(bool) {
	updateInputs();
}

void SVPropEditGeometry::on_lineEditTranslateX_editingFinishedSuccessfully() {
	updateTranslationPreview();
}

void SVPropEditGeometry::on_lineEditTranslateY_editingFinishedSuccessfully() {
	updateTranslationPreview();
}

void SVPropEditGeometry::on_lineEditTranslateZ_editingFinishedSuccessfully() {
	updateTranslationPreview();
}



void SVPropEditGeometry::on_radioButtonScaleResize_toggled(bool) {
	updateInputs();
}

void SVPropEditGeometry::on_lineEditScaleX_editingFinishedSuccessfully() {
	updateScalePreview();
}

void SVPropEditGeometry::on_lineEditScaleY_editingFinishedSuccessfully() {
	updateScalePreview();
}

void SVPropEditGeometry::on_lineEditScaleZ_editingFinishedSuccessfully() {
	updateScalePreview();
}





// *** PRIVATE FUNCTIONS ***

void SVPropEditGeometry::updateUi() {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;

	// update our selection lists
	std::set<const VICUS::Object*> sel;

	// first we get how many surfaces are selected
	project().selectObjects(sel, VICUS::Project::SG_All, false, false);

	// we also have to cache all existing names, so we take alle existing objects
	m_selSurfaces.clear();
	m_selRooms.clear();
	m_selSubSurfaces.clear();
	m_selBuildings.clear();
	m_selBuildingLevels.clear();

	m_subSurfNames.clear();
	m_surfNames.clear();
	m_buildingNames.clear();
	m_buildingLevelNames.clear();


	// process all selected objects and sort them into vectors
	for (const VICUS::Object * o : sel) {
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr ) {
			m_surfNames.insert(s->m_displayName );
			if (s->m_selected && s->m_visible)
				m_selSurfaces.push_back(s);
		}
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(o);
		if (r != nullptr ) {
			m_roomNames.insert(r->m_displayName );
			if (r->m_selected && r->m_visible)
				m_selRooms.push_back(r);
		}
		const VICUS::SubSurface * sub = dynamic_cast<const VICUS::SubSurface *>(o);
		if (sub != nullptr ) {
			m_subSurfNames.insert(sub->m_displayName );
			if (sub->m_selected && sub->m_visible)
				m_selSubSurfaces.push_back(sub);
		}
		const VICUS::BuildingLevel * bl = dynamic_cast<const VICUS::BuildingLevel *>(o);
		if (bl != nullptr ) {
			m_buildingLevelNames.insert(bl->m_displayName );
			if (bl->m_selected && bl->m_visible)
				m_selBuildingLevels.push_back(bl);
		}
		const VICUS::Building * b = dynamic_cast<const VICUS::Building *>(o);
		if (b != nullptr ) {
			m_subSurfNames.insert(b->m_displayName );
			if (b->m_selected && b->m_visible)
				m_selBuildings.push_back(b);
		}
	}


#if 0
	// handling if surfaces are selected
	if (!m_selSurfaces.empty()) {

		if ( m_selSurfaces.size() == 1 ) {
			const VICUS::Surface *s = m_selSurfaces[0];
			m_rotationState = RS_Normal;
			m_ui->toolButtonAbs->setEnabled(true);
			setRotation(s->geometry().normal() );
		}
		else {

			switch (m_rotationState) {
			case SVPropEditGeometry::RS_XAxis:
				setRotation( QVector2IBKVector(cso->localXAxis() ) );
				break;
			case SVPropEditGeometry::RS_YAxis:
				setRotation( QVector2IBKVector(cso->localYAxis() ) );
				break;
			case SVPropEditGeometry::RS_Normal:
			case SVPropEditGeometry::RS_ZAxis:
				m_rotationState = RS_ZAxis;
				setRotation( QVector2IBKVector(cso->localZAxis() ) );
				break;
			case SVPropEditGeometry::NUM_RS:
				break;

			}
			if(m_modificationType == ModificationType::MT_Rotate) {
				setToolButtonsRotationState(false);
			}
		}

	}
	else {

		// handling if only sub-surfaces are selected
		if (!m_selSubSurfaces.empty()) {
			if ( m_selSubSurfaces.size() == 1 ) {
				const VICUS::SubSurface *sub = m_selSubSurfaces[0];
				const VICUS::Surface *s = dynamic_cast<const VICUS::Surface*>(sub->m_parent);
				setRotation(s->geometry().normal() );
				m_ui->toolButtonAbs->setEnabled(true);
			}
			else {
				if(m_modificationType == ModificationType::MT_Rotate) {
					setToolButtonsRotationState(false);
				}
				setRotation( QVector2IBKVector(cso->localZAxis() ) );
			}
		}
	}
#endif
	// compute dimensions of bounding box (dx, dy, dz) and center point of all selected surfaces
	m_bbDim[OM_Local] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Local],
											  QVector2IBKVector(cso->translation() ),
											  QVector2IBKVector(cso->localXAxis() ),
											  QVector2IBKVector(cso->localYAxis() ),
											  QVector2IBKVector(cso->localZAxis() ) );
	m_bbDim[OM_Global] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Global]);

	SVViewStateHandler::instance().m_localCoordinateViewWidget->setBoundingBoxDimension(m_bbDim[OM_Global]);
	// position local coordinate system
	cso->setTranslation(IBKVector2QVector(m_bbCenter[OM_Global]) );
	// Note: setting new coordinates to the local coordinate system object will in turn call setCoordinates()
	//       and indirectly also updateInputs()
}


void SVPropEditGeometry::updateTranslationPreview() {
	double newX = m_ui->lineEditTranslateX->value();
	double newY = m_ui->lineEditTranslateY->value();
	double newZ = m_ui->lineEditTranslateZ->value();
	QVector3D translation = QVector3D((float)newX, (float)newY, (float)newZ);
	if (m_ui->radioButtonTranslationAbsolute->isChecked()) {
		// obtain new absolute position of local coordinate system
		// subtract original lcs position
		Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
		Q_ASSERT(cso != nullptr);
		// obtain offset and rotation of local coordinate system
		QVector3D offset = cso->translation();
		translation -= offset;
	}

	// adjust wireframe object transform
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.setTranslation(translation);
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();

	// also enable the apply/cancel buttons
	enableTransformation();
}


void SVPropEditGeometry::updateScalePreview() {
	QVector3D scaleFactors;
	if (m_ui->radioButtonScaleResize->isChecked()) {
		// compute scale factors
		double newX = m_ui->lineEditScaleX->value();
		double newY = m_ui->lineEditScaleY->value();
		double newZ = m_ui->lineEditScaleZ->value();
		// Mind: dimension in x, y, z might be zero, for example, if only a single
		//       surface had been selected. In this case the scale factor must be always exactly 1
		//       regardless of the user input

		if (IBK::near_equal(m_bbDim[OM_Local].m_x,0.0,1e-6))			scaleFactors.setX(1);
		else															scaleFactors.setX(float(newX/m_bbDim[OM_Local].m_x));

		if (IBK::near_equal(m_bbDim[OM_Local].m_y,0.0,1e-6))			scaleFactors.setY(1);
		else															scaleFactors.setY(float(newY/m_bbDim[OM_Local].m_y));

		if (IBK::near_equal(m_bbDim[OM_Local].m_z,0.0,1e-6))			scaleFactors.setZ(1);
		else															scaleFactors.setZ(float(newZ/m_bbDim[OM_Local].m_z));
	}
	else {
		scaleFactors.setX( (float)m_ui->lineEditScaleX->value());
		scaleFactors.setY( (float)m_ui->lineEditScaleY->value());
		scaleFactors.setZ( (float)m_ui->lineEditScaleZ->value());

	}
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	Q_ASSERT(cso != nullptr);
	// obtain offset and rotation of local coordinate system
	QVector3D offset = cso->translation();
	QQuaternion rot = cso->transform().rotation();

	// adjust wireframe object transform
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.setLocalScaling(offset, rot, scaleFactors);
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();

	// also enable the apply/cancel buttons
	enableTransformation();
}


#if 0
void SVPropEditGeometry::updateOrientationMode() {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	// we update the button state
	m_ui->toolButtonLocalCoordinateOrientation->setChecked(m_orientationMode == OM_Local);

	// we have to update our bounding box dimensions in our specific coordinate system
	// compute dimensions of bounding box (dx, dy, dz) and center point of all selected surfaces
	m_bbDim[OM_Local] = project().boundingBox( m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Local],
											   QVector2IBKVector(cso->translation() ),
											   QVector2IBKVector(cso->localXAxis() ),
											   QVector2IBKVector(cso->localYAxis() ),
											   QVector2IBKVector(cso->localZAxis() ) );
	m_bbDim[OM_Global] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Global]);

	SVViewStateHandler::instance().m_localCoordinateViewWidget->setBoundingBoxDimension(m_bbDim[m_orientationMode]);
	// we also update all the line edits and boxes
	updateInputs();
}
#endif


void SVPropEditGeometry::onWheelTurned(double offset, QtExt::ValidatingLineEdit * lineEdit) {
	if (!lineEdit->isValid())
		return; // invalid input, do nothing
	double val = lineEdit->value();
	val += offset;
	lineEdit->setValue(val); // this does not trigger any signals, so we need to send change info manually
	onLineEditTextChanged(lineEdit);
}





void SVPropEditGeometry::onLineEditTextChanged(QtExt::ValidatingLineEdit * lineEdit) {
	// not valid? don't send a signal
	if (!lineEdit->isValid())
		return;

	if (lineEdit == m_ui->lineEditTranslateX)
		on_lineEditTranslateX_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditTranslateY)
		on_lineEditTranslateY_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditTranslateZ)
		on_lineEditTranslateZ_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditScaleX)
		on_lineEditScaleX_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditScaleY)
		on_lineEditScaleY_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditScaleZ)
		on_lineEditScaleZ_editingFinishedSuccessfully();

#if 0
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	// take transformation value, and if valid, modify transform in wire frame object


	lineEdit->setModified(true);

	ModificationState state = m_modificationState[m_modificationType];
	ModificationType type = m_modificationType;

	// compose all transformation values
	switch (type) {

	// Translation operation
	case MT_Translate: {

		// compose translation vector depending on translation mode
		switch (state) {
		case MS_Absolute : {
			// for this operation, we need all three coordinates
			QVector3D targetPos((float)m_ui->lineEditX->value(), (float)m_ui->lineEditY->value(), (float)m_ui->lineEditZ->value());
			// now compose a transform object and set it in the wireframe object
			Vic3D::Transform3D trans;
			// compute offset from current local coordinate system position
			QVector3D translation;
			if (m_orientationMode == OM_Local) {
				QVector3D localTrans (cso->translation() );
				QVector3D newTrans( localTrans.x()*cso->localXAxis().x() + localTrans.y()*cso->localXAxis().y() + localTrans.z()*cso->localXAxis().z(),
									localTrans.x()*cso->localYAxis().x() + localTrans.y()*cso->localYAxis().y() + localTrans.z()*cso->localYAxis().z(),
									localTrans.x()*cso->localZAxis().x() + localTrans.y()*cso->localZAxis().y() + localTrans.z()*cso->localZAxis().z() );


				if (lineEdit == m_ui->lineEditX) {
					translation = (targetPos.x() - newTrans.x()) * cso->localXAxis();
				}
				else if (lineEdit == m_ui->lineEditY) {
					translation = (targetPos.y() - newTrans.y()) * cso->localYAxis();
				}
				else {
					translation = (targetPos.z() - newTrans.z()) * cso->localZAxis();
				}
			}
			else
				translation = targetPos - m_lcsTransform.translation();

			trans.setTranslation(translation);
			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = trans;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
		} break;

			// inputs are global coordinate offsets
		case MS_Relative : {
			// for this operation, we need all three coordinates
			QVector3D translation((float)m_ui->lineEditX->value(), (float)m_ui->lineEditY->value(), (float)m_ui->lineEditZ->value());
			// now compose a transform object and set it in the wireframe object
			Vic3D::Transform3D trans;

			if (m_orientationMode == OM_Local) {
				QVector3D newTrans( translation.x()*cso->localXAxis().x() + translation.y()*cso->localYAxis().x() + translation.z()*cso->localZAxis().x(),
									translation.x()*cso->localXAxis().y() + translation.y()*cso->localYAxis().y() + translation.z()*cso->localZAxis().y(),
									translation.x()*cso->localXAxis().z() + translation.y()*cso->localYAxis().z() + translation.z()*cso->localZAxis().z() );
				translation = newTrans;
			}

			trans.setTranslation(translation);
			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = trans;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
		} break;
		}
	} break;

		// Scale operation
	case MT_Scale: {
		switch (state) {
		case MS_Absolute: {
			// for this operation, we need at first the dimensions of the bounding box from
			IBKMK::Vector3D targetScale(m_ui->lineEditX->value(), m_ui->lineEditY->value(), m_ui->lineEditZ->value());
			// compute offset from current local coordinate system position
			QVector3D scale;

			if (m_orientationMode == OM_Local) {
				// we know the local bounding box
				// we can scale up the local bounding box by the factor
				// finally we calc back our dimensions of the global bounding box

				IBKMK::Vector3D newBBDimLocal = m_bbDim[OM_Local];
				IBKMK::Vector3D newBBDimGlobal = m_bbDim[OM_Global];

				QVector3D local;
				double diff;

				if (lineEdit == m_ui->lineEditX) { // we scale with the x axis
					newBBDimLocal.m_x = targetScale.m_x;
					diff = newBBDimLocal.m_x - m_bbDim[OM_Local].m_x;
					local = cso->localXAxis();
				}
				else if (lineEdit == m_ui->lineEditY) {  // we scale with the y axis
					newBBDimLocal.m_y = targetScale.m_y;
					diff = newBBDimLocal.m_y - m_bbDim[OM_Local].m_y;
					local = cso->localYAxis();
				}
				else { // we scale with the z axis
					newBBDimLocal.m_z = targetScale.m_z;
					diff = newBBDimLocal.m_z - m_bbDim[OM_Local].m_z;
					local = cso->localZAxis();
				}

				newBBDimGlobal.m_x = m_bbDim[OM_Global].m_x + diff * std::fabs(local.x());
				newBBDimGlobal.m_y = m_bbDim[OM_Global].m_y + diff * std::fabs(local.y());
				newBBDimGlobal.m_z = m_bbDim[OM_Global].m_z + diff * std::fabs(local.z());

				scale.setX(newBBDimGlobal.m_x / (std::fabs(m_bbDim[OM_Global].m_x) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_x));
				scale.setY(newBBDimGlobal.m_y / (std::fabs(m_bbDim[OM_Global].m_y) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_y));
				scale.setZ(newBBDimGlobal.m_z / (std::fabs(m_bbDim[OM_Global].m_z) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_z));
			}
			else {


				scale.setX( m_bbDim[m_orientationMode].m_x < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_x ) < 1E-4 ? 1.0 : targetScale.m_x / ( m_bbDim[m_orientationMode].m_x < 1E-4 ? 1.0 : m_bbDim[m_orientationMode].m_x ) ) );
				scale.setY( m_bbDim[m_orientationMode].m_y < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_y ) < 1E-4 ? 1.0 : targetScale.m_y / ( m_bbDim[m_orientationMode].m_y < 1E-4 ? 1.0 : m_bbDim[m_orientationMode].m_y ) ) );
				scale.setZ( m_bbDim[m_orientationMode].m_z < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_z ) < 1E-4 ? 1.0 : targetScale.m_z / ( m_bbDim[m_orientationMode].m_z < 1E-4 ? 1.0 : m_bbDim[m_orientationMode].m_z ) ) );
				// now compose a transform object and set it in the wireframe object
				// first we scale our selected objects

			}

			Vic3D::Transform3D scaling;
			scaling.setScale(scale);


			// and then we also have to guarantee that the center point of the bounding box stays at the same position
			// this can be achieved since the center points are also just scaled by the specified scaling factors
			// so we know how big the absolute translation has to be
			IBKMK::Vector3D trans;

			const QVector3D &transObj = cso->transform().translation();

			trans.m_x = std::fabs(transObj.x()) < 1E-4 ? 0.0 : transObj.x() * ( 1 - scale.x() );
			trans.m_y = std::fabs(transObj.y()) < 1E-4 ? 0.0 : transObj.y() * ( 1 - scale.y() );
			trans.m_z = std::fabs(transObj.z()) < 1E-4 ? 0.0 : transObj.z() * ( 1 - scale.z() );
			scaling.setTranslation( IBKVector2QVector(trans) );

			// we give our transformation to the wire frame object
			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = scaling;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow(); // needed right now since two surfaces are shown
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
		} break;

		case MS_Relative: {
			// for this operation, we get directly the surface scaling factors from the line edits
			// so it is basically like the absolute scaling, but we do not have to calculate the scaling factors
			QVector3D  scale(m_ui->lineEditX->value(), m_ui->lineEditY->value(), m_ui->lineEditZ->value());
			// compute offset from current local coordinate system position
			const Vic3D::CoordinateSystemObject *cso = cso;

			QVector3D lcsTrans = cso->translation();

			QVector3D newScale;
			if (m_orientationMode == OM_Local) {
				// we know the local bounding box
				// we can scale up the local bounding box by the factor
				// finally we calc back our dimensions of the global bounding box

				IBKMK::Vector3D newBBDimLocal = m_bbDim[OM_Local];
				IBKMK::Vector3D newBBDimGlobal = m_bbDim[OM_Global];

				QVector3D local;
				double diff;

				if (lineEdit == m_ui->lineEditX) { // we scale with the x axis
					newBBDimLocal.m_x = scale.x() * m_bbDim[OM_Local].m_x;
					diff = newBBDimLocal.m_x - m_bbDim[OM_Local].m_x;
					local = cso->localXAxis();
				}
				else if (lineEdit == m_ui->lineEditY) {  // we scale with the y axis
					newBBDimLocal.m_y = scale.y() * m_bbDim[OM_Local].m_y;
					diff = newBBDimLocal.m_y - m_bbDim[OM_Local].m_y;
					local = cso->localYAxis();
				}
				else { // we scale with the z axis
					newBBDimLocal.m_z = scale.z() * m_bbDim[OM_Local].m_z;
					diff = newBBDimLocal.m_z - m_bbDim[OM_Local].m_z;
					local = cso->localZAxis();
				}

				newBBDimGlobal.m_x = m_bbDim[OM_Global].m_x + diff * local.x();
				newBBDimGlobal.m_y = m_bbDim[OM_Global].m_y + diff * local.y();
				newBBDimGlobal.m_z = m_bbDim[OM_Global].m_z + diff * local.z();

				scale.setX(newBBDimGlobal.m_x / (std::fabs(m_bbDim[OM_Global].m_x) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_x));
				scale.setY(newBBDimGlobal.m_y / (std::fabs(m_bbDim[OM_Global].m_y) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_y));
				scale.setZ(newBBDimGlobal.m_z / (std::fabs(m_bbDim[OM_Global].m_z) < 1e-4 ? 1e-4 : m_bbDim[OM_Global].m_z));
			}
			else {



			}


			Vic3D::Transform3D scaling;
			scaling.setScale(scale);

			// and the we also hav to guarantee that the center point of the bounding box stays at the same position
			// this can be achieved since the center points are also just scaled by the specified scaling factors
			// so we know how big the absolute translation has to be
			IBKMK::Vector3D trans;
			const QVector3D &transObj = cso->transform().translation();

			trans.m_x = std::fabs(transObj.x()) < 1E-4 ? 0.0 : transObj.x() * ( 1.0 - scale.x() );
			trans.m_y = std::fabs(transObj.y()) < 1E-4 ? 0.0 : transObj.y() * ( 1.0 - scale.y() );
			trans.m_z = std::fabs(transObj.z()) < 1E-4 ? 0.0 : transObj.z() * ( 1.0 - scale.z() );

			scaling.setTranslation( IBKVector2QVector(trans) );

			// we give our transformation to the wire frame object
			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = scaling;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();

		} break;

		}
	} break;

		// Rotation operations
	case MT_Rotate: {
		switch (state) {

		case MS_Absolute: {
			// get the rotation angles
			double oriRad = m_ui->lineEditOrientation->value() * IBK::DEG2RAD;
			double incliRad = (90-m_ui->lineEditInclination->value()) * IBK::DEG2RAD;


			Vic3D::Transform3D rota;

			if (m_orientationMode == OM_Global) {
				switch (m_rotationState) {
				case SVPropEditGeometry::RS_Normal: {
					IBKMK::Vector3D newNormal(	std::sin( oriRad ) * std::cos( incliRad ),
												std::cos( oriRad ) * std::cos( incliRad ),
												std::sin( incliRad ) );

					// we only want to rotate if the normal vectors are not the same
					if ( checkVectors<4>( m_normal, newNormal ) )
						return; // do nothing

					// we find the rotation axis by taking the cross product of the normal vector and the normal vector we want to
					// rotate to
					IBKMK::Vector3D rotationAxis ( m_normal.crossProduct(newNormal).normalized() );
					qDebug() << "Rotation axis: " << rotationAxis.m_x << "\t" << rotationAxis.m_y << "\t" << rotationAxis.m_z;

					// we now also have to find the angle between both normals

					double angle = (float)angleBetweenVectorsDeg(m_normal, newNormal);

					rota.rotate(angle, IBKVector2QVector(rotationAxis) );
					qDebug() << "Roation angle: " << angle << " °";
				}
					break;
				case SVPropEditGeometry::RS_XAxis:
					break;
				case SVPropEditGeometry::RS_YAxis:
					break;
				case SVPropEditGeometry::RS_ZAxis:
					break;


				}
			}
			else {


				//						if ( m_ui->toolButtonX->isChecked() )
				//							rota.setRotation((float)m_ui->lineEditX->value(), m_cso->localXAxis());
				//						else if ( m_ui->toolButtonY->isChecked() )
				//							rota.setRotation((float)m_ui->lineEditY->value(), m_cso->localYAxis());
				//						else if ( m_ui->toolButtonZ->isChecked() )
				//							rota.setRotation((float)m_ui->lineEditZ->value(), m_cso->localZAxis());
			}

			// we take the QQuarternion to rotate
			QVector4D rotVec = rota.rotation().toVector4D();
			IBKMK::Vector3D center = QVector2IBKVector(cso->translation() );
			IBKMK::Vector3D newCenter = center;
			IBKMK::Quaternion centerRota((double) rotVec.w(), (double) rotVec.x(), (double) rotVec.y(), (double) rotVec.z());

			centerRota.rotateVector(newCenter);

			// we also have to find the center point after rotation and translate our center back to its origin
			rota.setTranslation(IBKVector2QVector(center - newCenter) );

			// we give our tranfsformation to the wire frame object
			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = rota;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
		} break;
		case MS_Relative: {
			// now compose a transform object and set it in the wireframe object
			Vic3D::Transform3D rota;
			if (m_orientationMode == OM_Global) {
				if ( lineEdit == m_ui->lineEditX )
					rota.setRotation((float)m_ui->lineEditX->value(), 1, 0, 0);
				else if ( lineEdit == m_ui->lineEditY )
					rota.setRotation((float)m_ui->lineEditY->value(), 0, 1, 0);
				else if ( lineEdit == m_ui->lineEditZ )
					rota.setRotation((float)m_ui->lineEditZ->value(), 0, 0, 1);
			}
			else {
				if ( lineEdit == m_ui->lineEditX )
					rota.setRotation((float)m_ui->lineEditX->value(), cso->localXAxis());
				else if ( lineEdit == m_ui->lineEditY )
					rota.setRotation((float)m_ui->lineEditY->value(), cso->localYAxis());
				else if ( lineEdit == m_ui->lineEditZ )
					rota.setRotation((float)m_ui->lineEditZ->value(), cso->localZAxis());
			}


			// and then we also have to guarantee that the center point of the bounding box stays at the same position
			// this can be achieved since the center points are also just rotated by the specified rotation
			// so we know how big the absolute translation has to be
			QVector4D rotVec = rota.rotation().toVector4D();
			IBKMK::Vector3D newCenter = QVector2IBKVector(cso->translation() );
			IBKMK::Quaternion centerRota((double) rotVec.w(), (double) rotVec.x(), (double) rotVec.y(), (double) rotVec.z());
			centerRota.rotateVector(newCenter);

			// we also have to find the center point after rotation and translate our center back to its origin
			rota.setTranslation(IBKVector2QVector(QVector2IBKVector(cso->translation() ) - newCenter) );

			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = rota;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
		} break;
		}
	} break;
	}
#endif
}















#if 0


void SVPropEditGeometry::on_lineEditX_returnPressed() {
	// check if entered value is valid, if not reset it to its default
	if ( !m_ui->lineEditX->isValid() ) {
		m_ui->lineEditX->setValue( m_originalValues.m_x );
		return;
	}
	if ( std::fabs( m_originalValues.m_x - m_ui->lineEditX->value() ) < 1E-3 )
		return;

	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( !lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	//	double tempXValue = m_ui->lineEditX->value();
	switch ( m_modificationType ) {
	case MT_Translate: translate(); break;
	case MT_Scale: scale(); break;
	case MT_Rotate: rotate(); break;
	}
	lineEdit->setModified(false);
}

void SVPropEditGeometry::on_lineEditY_returnPressed() {
	// check if entered value is valid, if not reset it to its default
	if ( !m_ui->lineEditY->isValid() ) {
		m_ui->lineEditX->setValue( m_originalValues.m_y );
		return;
	}
	if ( std::fabs( m_originalValues.m_y - m_ui->lineEditY->value() ) < 1E-3 )
		return;

	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( !lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	//	double tempXValue = m_ui->lineEditX->value();
	switch ( m_modificationType ) {
	case MT_Translate: translate(); break;
	case MT_Scale: scale(); break;
	case MT_Rotate: rotate(); break;
	}
	lineEdit->setModified(false);
}

void SVPropEditGeometry::on_lineEditZ_returnPressed(){
	// check if entered value is valid, if not reset it to its default
	if ( !m_ui->lineEditZ->isValid() ) {
		m_ui->lineEditZ->setValue( m_originalValues.m_z );
		return;
	}
	if ( std::fabs( m_originalValues.m_z - m_ui->lineEditZ->value() ) < 1E-3 )
		return;

	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( !lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	switch ( m_modificationType ) {
	case MT_Translate: translate(); break;
	case MT_Scale: scale(); break;
	case MT_Rotate: rotate(); break;
	}
}

#endif

void SVPropEditGeometry::updateInputs() {
//	ModificationState state = m_modificationState[m_modificationType];

	switch (m_ui->stackedWidget->currentIndex()) {

		// *** Translation page ***
		case MT_Translate : {
			if (m_ui->radioButtonTranslationAbsolute->isChecked()) {

				m_ui->labelTranslateX->setText("X [m]:");
				m_ui->labelTranslateY->setText("Y [m]:");
				m_ui->labelTranslateZ->setText("Z [m]:");

				// cache current local coordinate systems position as fall-back values
				m_originalValues = QVector2IBKVector(m_lcsTransform.translation());
			}
			else {
				m_ui->labelTranslateX->setText("ΔX [m]:");
				m_ui->labelTranslateY->setText("ΔY [m]:");
				m_ui->labelTranslateZ->setText("ΔZ [m]:");

				m_originalValues.set(0,0,0);
			}
			m_ui->lineEditTranslateX->setValue( m_originalValues.m_x );
			m_ui->lineEditTranslateY->setValue( m_originalValues.m_y );
			m_ui->lineEditTranslateZ->setValue( m_originalValues.m_z );
		} break;

#if 0

		case MT_Rotate: {
			showDeg();
			showRotation(false);

			m_ui->labelX->setText("X");
			m_ui->labelY->setText("Y");
			m_ui->labelZ->setText("Z");

			switch (state) {
			case MS_Absolute: {
				showRotation();
				showDeg(false);

				m_ui->lineEditX->setValue(0);
				m_ui->lineEditY->setValue(0);
				m_ui->lineEditZ->setValue(0);
			}
				break;

			default:
				m_ui->lineEditX->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
				m_ui->lineEditY->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
				m_ui->lineEditZ->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
			}

		} break;

#endif

		case MT_Scale: {

			if (m_ui->radioButtonScaleResize->isChecked()) {
				m_ui->labelScaleX->setText("L<sub>X</sub> [m]:");
				m_ui->labelScaleY->setText("W<sub>Y</sub> [m]:");
				m_ui->labelScaleZ->setText("H<sub>Z</sub> [m]:");

				// always local bounding box
				m_originalValues = m_bbDim[OM_Local];

				m_ui->lineEditScaleX->setEnabled(!IBK::near_equal(m_bbDim[OM_Local].m_x,0.0,1e-6));
				m_ui->lineEditScaleY->setEnabled(!IBK::near_equal(m_bbDim[OM_Local].m_y,0.0,1e-6));
				m_ui->lineEditScaleZ->setEnabled(!IBK::near_equal(m_bbDim[OM_Local].m_z,0.0,1e-6));

			}
			else {
				m_ui->labelScaleX->setText("s<sub>X</sub>:");
				m_ui->labelScaleY->setText("s<sub>Y</sub>:");
				m_ui->labelScaleZ->setText("s<sub>Z</sub>:");

				m_originalValues = IBKMK::Vector3D( 1,1,1 );

				m_ui->lineEditScaleX->setEnabled(true);
				m_ui->lineEditScaleY->setEnabled(true);
				m_ui->lineEditScaleZ->setEnabled(true);
			}
			m_ui->lineEditScaleX->setValue(m_originalValues.m_x );
			m_ui->lineEditScaleY->setValue(m_originalValues.m_y );
			m_ui->lineEditScaleZ->setValue(m_originalValues.m_z );
		} break;

	} // switch modification type

	// reset wireframe transform
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();

	// disable apply and cancel buttons
	m_ui->pushButtonApply->setEnabled(false);
	m_ui->pushButtonCancel->setEnabled(false);
}



#if 0
void SVPropEditGeometry::setToolButton() {


	m_ui->toolButtonTrans->setChecked(false);
	m_ui->toolButtonRotate->setChecked(false);
	m_ui->toolButtonScale->setChecked(false);

	switch (m_modificationType) {
	case MT_Translate:		m_ui->toolButtonTrans->setChecked(true);			break;
	case MT_Rotate:			m_ui->toolButtonRotate->setChecked(true);			break;
	case MT_Scale:			m_ui->toolButtonScale->setChecked(true);			break;
	}
}

void SVPropEditGeometry::setToolButtonAbsMode() {
	m_ui->toolButtonRel->blockSignals(true);
	m_ui->toolButtonAbs->blockSignals(true);

	switch (m_modificationType) {
	case MT_Translate:
		m_ui->toolButtonAbs->setText( tr("Move to world coordinates") );
		m_ui->toolButtonRel->setText( tr("Relative translation") );
		break;
	case MT_Rotate:
		m_ui->toolButtonAbs->setText( tr("Align surface to angles") );
		m_ui->toolButtonRel->setText( tr("Relative rotation") );
		break;
	case MT_Scale:
		m_ui->toolButtonAbs->setText( tr("Resize surfaces") );
		m_ui->toolButtonRel->setText( tr("Relative scaling") );
		break;
	}

	bool checkAbsState = m_modificationState[m_modificationType] == ModificationState::MS_Absolute;

	m_ui->toolButtonAbs->setChecked(checkAbsState);
	m_ui->toolButtonRel->setChecked(!checkAbsState);

	m_ui->toolButtonAbs->blockSignals(false);
	m_ui->toolButtonRel->blockSignals(false);

}

void SVPropEditGeometry::setToolButtonsRotationState(bool absOn) {
	m_ui->toolButtonAbs->setEnabled(absOn);
	m_ui->toolButtonRel->setChecked(!absOn);
}

void SVPropEditGeometry::showDeg(const bool & show) {
	if ( show ) {
		m_ui->labelXDeg->show();
		m_ui->labelYDeg->show();
		m_ui->labelZDeg->show();
	}
	else {
		m_ui->labelXDeg->hide();
		m_ui->labelYDeg->hide();
		m_ui->labelZDeg->hide();
	}
}

void SVPropEditGeometry::showRotation(const bool & abs) {
	// we show all that is necessary for absolute Rotation Mode
	m_ui->lineEditOrientation->blockSignals(true);
	m_ui->lineEditInclination->blockSignals(true);
	if ( abs ) {
		m_ui->widgetXYZ->hide();
		m_ui->widgetRota->show();
	}
	else {
		m_ui->widgetXYZ->show();
		m_ui->widgetRota->hide();
	}
	m_ui->lineEditOrientation->blockSignals(false);
	m_ui->lineEditInclination->blockSignals(false);
}


void SVPropEditGeometry::on_toolButtonTrans_clicked() {
	setState(MT_Translate, m_modificationState[MT_Translate]);
}

void SVPropEditGeometry::on_toolButtonRotate_clicked() {
	setState(MT_Rotate, m_modificationState[MT_Rotate]);
}

void SVPropEditGeometry::on_toolButtonScale_clicked() {
	setState(MT_Scale, m_modificationState[MT_Scale]);
}


void SVPropEditGeometry::on_lineEditOrientation_returnPressed() {
	// check if entered value is valid, if not reset it to its default
	double orientation = std::atan2(m_normal.m_x, ( m_normal.m_y == 0. ? 1E-8 : m_normal.m_y ) ) /IBK::DEG2RAD ;
	if ( !m_ui->lineEditOrientation->isValid() ) {
		m_ui->lineEditOrientation->setValue( orientation < 0 ? ( orientation + 360 ) : orientation  );
		return;
	}
	if ( std::fabs( orientation < 0 ? ( orientation + 360 ) : orientation - m_ui->lineEditOrientation->value() ) < 1E-3 )
		return;


	rotate();
}

void SVPropEditGeometry::on_lineEditInclination_returnPressed() {

	if ( !m_ui->lineEditInclination->isValid() ) {
		m_ui->lineEditInclination->setValue( std::acos(m_normal.m_z)/IBK::DEG2RAD );
		return;
	}

	rotate();
}


void SVPropEditGeometry::on_lineEditOrientation_editingFinished() {
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( !lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	on_lineEditOrientation_returnPressed();
}

void SVPropEditGeometry::on_lineEditInclination_editingFinished() {
	QLineEdit * lineEdit = qobject_cast<QLineEdit *>( QObject::sender() );

	// Ignore undesirable signals.
	if ( !lineEdit->isModified() )
		return;
	lineEdit->setModified( false );

	on_lineEditInclination_returnPressed();
}


void SVPropEditGeometry::on_lineEditXCopy_editingFinished() {
	if ( m_ui->lineEditXCopy->isValid() )
		m_translation.m_x = m_ui->lineEditXCopy->value();

	m_ui->lineEditXCopy->setValue(m_translation.m_x );
}

void SVPropEditGeometry::on_lineEditYCopy_editingFinished() {
	if ( m_ui->lineEditYCopy->isValid() )
		m_translation.m_y = m_ui->lineEditYCopy->value();

	m_ui->lineEditYCopy->setValue(m_translation.m_y );
}

void SVPropEditGeometry::on_lineEditZCopy_editingFinished() {
	if ( m_ui->lineEditZCopy->isValid() )
		m_translation.m_z = m_ui->lineEditZCopy->value();

	m_ui->lineEditZCopy->setValue(m_translation.m_z );
}


void SVPropEditGeometry::on_lineEditX_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditX);
}

void SVPropEditGeometry::on_lineEditY_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditY);
}

void SVPropEditGeometry::on_lineEditZ_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditZ);
}


void SVPropEditGeometry::on_lineEditOrientation_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditOrientation);
}

void SVPropEditGeometry::on_lineEditInclination_textChanged(const QString &) {
	onLineEditTextChanged(m_ui->lineEditInclination);
}


void SVPropEditGeometry::on_pushButtonThreePointRotation_clicked() {
	// when clicked, we set the scene into three-point-rotation mode
	// TODO Stephan
}


void SVPropEditGeometry::on_pushButtonFlipNormals_clicked() {
	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;

	// flip all plane geometries
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			VICUS::Surface modS(*s);
			modS.flip();
			modifiedSurfaces.push_back(modS);
		}
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	// create undo-action
	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Flipped normal vectors"), modifiedSurfaces );
	undo->push();
	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
}


void SVPropEditGeometry::on_toolButtonLocalCoordinateOrientation_clicked(bool checked) {
	// we set the state of our local coordinate system
	m_orientationMode = checked ? OM_Local : OM_Global;

	// we update all the specific parameters
	updateOrientationMode();
}



void SVPropEditGeometry::on_toolButtonAbs_clicked(bool /*checked*/) {

	// set new state
	m_modificationState[m_modificationType] = ModificationState::MS_Absolute;
	// update tool buttons
	setToolButtonAbsMode();
	// now update inputs
	updateInputs();
}

void SVPropEditGeometry::on_toolButtonRel_clicked(bool /*checked*/) {

	m_ui->toolButtonAbs->setChecked(false);
	// set new state
	m_modificationState[m_modificationType] = ModificationState::MS_Relative;
	// update tool buttons
	setToolButtonAbsMode();
	// now update inputs
	updateInputs();
}

void SVPropEditGeometry::on_toolButtonNormal_clicked() {
	m_rotationState = RS_Normal;
	setToolButtonsRotationState(m_modificationState[m_modificationType] == MS_Absolute);
}


void SVPropEditGeometry::on_toolButtonX_clicked() {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	m_rotationState = RS_XAxis;
	setToolButtonsRotationState(m_modificationState[m_modificationType] == MS_Absolute);
	if (m_orientationMode == OM_Local)
		setRotation( QVector2IBKVector(cso->localXAxis() ) );
	else
		setRotation( IBKMK::Vector3D(1,0,0));
}

void SVPropEditGeometry::on_toolButtonY_clicked() {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	m_rotationState = RS_YAxis;
	setToolButtonsRotationState(m_modificationState[m_modificationType] == MS_Absolute);
	if (m_orientationMode == OM_Local)
		setRotation( QVector2IBKVector(cso->localYAxis() ) );
	else
		setRotation( IBKMK::Vector3D(0,1,0));
}

void SVPropEditGeometry::on_toolButtonZ_clicked() {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	m_rotationState = RS_ZAxis;
	setToolButtonsRotationState(m_modificationState[m_modificationType] == MS_Absolute);

	if (m_orientationMode == OM_Local)
		setRotation( QVector2IBKVector(cso->localZAxis() ) );
	else
		setRotation( IBKMK::Vector3D(0,0,1));
}

void SVPropEditGeometry::on_pushButtonCenteHorizontal_clicked(){

	/// first we take the selection of all objects and let the bounding box calculate
	/// our new center point for all objects. Alternative would be the center point
	/// of all individual bounding boxes. but we might do this later
	///
	/// Then we take all selected objects and we calculate the local difference between
	/// the new center point (only z-value) and the old z-value. This is our local
	/// z-value for the translation. We then translate all vertexes of the surface
	///
	/// We have to differentiate between surfaces and sub surfaces
	IBKMK::Vector3D center, oldCenter;
	IBKMK::Vector3D bb = project().boundingBox(m_selSurfaces, m_selSubSurfaces, center);

	std::vector<VICUS::Surface>			modifiedSurfaces;
	std::set<const VICUS::Surface*>		handledSurfaces;

	std::vector<const VICUS::Surface*>		emptySurfs;
	std::vector<const VICUS::SubSurface*>	emptySubSurfs;

	for(const VICUS::Surface *surf : m_selSurfaces) {
		std::vector<const VICUS::Surface*> surfs;
		surfs.push_back(surf);
		IBKMK::Vector3D bbOld = project().boundingBox(surfs, emptySubSurfs, oldCenter);

		IBKMK::Vector3D translation (0,0,center.m_z - oldCenter.m_z);

		// we have now our translation
		std::vector<IBKMK::Vector3D> vertexes = surf->polygon3D().vertexes();
		for ( IBKMK::Vector3D & v : vertexes ) {
			// use just this instead of making a QVetor3D
			v += translation;
		}
		VICUS::Surface modS(*surf);
		modS.setPolygon3D( VICUS::Polygon3D(vertexes) );

		modifiedSurfaces.push_back(modS);
	}
	for(const VICUS::SubSurface *subSurf : m_selSubSurfaces) {

		// we keep our original surface
		VICUS::Surface *parentSurf = dynamic_cast<VICUS::Surface*>(subSurf->m_parent);
		if (parentSurf != nullptr && parentSurf->m_selected && parentSurf->m_visible)
			continue; //

		if(handledSurfaces.find(parentSurf) != handledSurfaces.end())
			continue; // surface already handled

		// parentSurf->m_selected = true; // !!!! only for now till we adjust the function !!!
		VICUS::Surface modS(*parentSurf);

		// we cache our poldon data
		IBKMK::Vector3D offset3d = modS.geometry().offset();
		const IBKMK::Vector3D &localX = modS.geometry().localX();
		const IBKMK::Vector3D &localY = modS.geometry().localY();

		std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );
		// now we also have to scale the sub surfaces
		for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

			if (!modS.subSurfaces()[i].m_selected || !modS.subSurfaces()[i].m_visible)
				continue; // skip deselected surfaces

			VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(modS.subSurfaces()[i]);
			std::vector<const VICUS::SubSurface*> subSurfs;
			subSurfs.push_back(&subS);
			IBKMK::Vector3D bbOld = project().boundingBox(emptySurfs, subSurfs, oldCenter);

			IBKMK::Vector3D translation (0,0,center.m_z - oldCenter.m_z);

			qDebug() << "0\t0\t" << translation.m_z;

			Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());
			newSubSurfs[i] = modS.subSurfaces()[i];

			// we only modify our selected sub surface
			std::vector<IBKMK::Vector2D> newSubSurfVertexes (subS.m_polygon2D.vertexes().size());

			for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
				IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

				// we now calculate the 3D points of the sub surface
				// afterwards we scale up the surface
				IBKMK::Vector3D v3D = offset3d + localX * v2D.m_x + localY * v2D.m_y;

				Vic3D::Transform3D t;
				t.translate(IBKVector2QVector(translation) );
				t.setTranslation(t.toMatrix()*IBKVector2QVector(v3D) );
				v3D = QVector2IBKVector(t.translation() );

				// and we calculate back the projection on the plane
				// we have to take the offset of our new scaled polygon
				IBKMK::planeCoordinates(modS.geometry().offset(), localX, localY, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);

			}
			newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
		}
		// we cache that we already handled all selected sub surfaces of the surface
		// handledSurfaces.insert(parentSurf);

		// we update the 2D polyline
		modS.setSubSurfaces(newSubSurfs);
		handledSurfaces.insert(parentSurf);

		modifiedSurfaces.push_back(modS);
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undoSurf = new SVUndoModifySurfaceGeometry(tr("Translated surface geometry"), modifiedSurfaces );
	undoSurf->push();

}

#endif







void SVPropEditGeometry::updateCoordinateSystemLook() {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	if (SVViewStateHandler::instance().m_geometryView == nullptr)
		return; // do nothing while initializing
	// adjust appearance of local coordinate system
	if (m_ui->stackedWidget->currentIndex() == MT_Align) {
		// put local coordinate system back into "plain" mode
		if (cso->m_geometryTransformMode != 0) {
			cso->m_geometryTransformMode = 0;
			SVViewStateHandler::instance().m_geometryView->refreshSceneView();
		}
	}
	else {
		// put local coordinate system back into correct transform mode
		switch (m_ui->stackedWidget->currentIndex()) {
			case MT_Translate:
				if (cso->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_Translate) {
					cso->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_Translate;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
				break;

			case MT_Rotate:
				if (cso->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_RotateMask) {
					cso->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_RotateMask;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
				break;

			case MT_Scale:
				if (cso->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_ScaleMask) {
					cso->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_ScaleMask;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
				break;

			case NUM_MT : ; // just to make compiler happy
		}
	}
}


void SVPropEditGeometry::translate() {
	// get translation vector from selected geometry object
	IBKMK::Vector3D translation = QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation());

	if (translation == IBKMK::Vector3D())
		return;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface>			modifiedSurfaces;
	std::set<VICUS::Surface*>			handledSurfaces;

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// ======= SURFACE IS TRANSLATED =======
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			VICUS::Surface modS(*s);
			const_cast<VICUS::Polygon3D&>(modS.geometry().polygon3D()).translate(translation);
			modifiedSurfaces.push_back(modS);
		}

		// TODO : Netzwerk-Transformationen

		// ======= SUB SURFACE IS TRANSLATED =======
		// handle only selected sub surfaces where parent is not selected
		const VICUS::SubSurface * ss = dynamic_cast<const VICUS::SubSurface *>(o);
		if (ss != nullptr) {

			// we keep our original surface
			VICUS::Surface *parentSurf = dynamic_cast<VICUS::Surface*>(ss->m_parent);
			if (parentSurf != nullptr && parentSurf->m_selected && parentSurf->m_visible)
				continue; // no parent exits, should not really happen

			if(handledSurfaces.find(parentSurf) != handledSurfaces.end())
				continue; // surface already handled

			// parentSurf->m_selected = true; // !!!! only for now till we adjust the function !!!
			VICUS::Surface modS(*parentSurf);

			// we cache our polygon data for easier access
			IBKMK::Vector3D offset3d = modS.geometry().offset();
			const IBKMK::Vector3D &localX = modS.geometry().localX();
			const IBKMK::Vector3D &localY = modS.geometry().localY();

			// holds our newly translated subsurfaces
			std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );

			// now we also have to scale the sub surfaces
			for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

				// sub surfaces and holes have to have the same sice!
				Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());
				newSubSurfs[i] = modS.subSurfaces()[i];

				// skip deselected and invisible surfaces
				if (!modS.subSurfaces()[i].m_selected || !modS.subSurfaces()[i].m_visible)
					continue;

				// we only modify our selected sub surface
				VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(modS.subSurfaces()[i]);
				std::vector<IBKMK::Vector2D> newSubSurfVertexes(subS.m_polygon2D.vertexes().size());

				// now we handle all our sub surfaces
				for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
					IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

					// we now calculate the 3D points of the sub surface
					// afterwards we scale up the surface
					IBKMK::Vector3D v3D = offset3d + localX * v2D.m_x + localY * v2D.m_y;

					Vic3D::Transform3D t;
					t.translate(IBKVector2QVector(translation) );
					// we translate the point in 3D space
					t.setTranslation(t.toMatrix()*IBKVector2QVector(v3D) );
					v3D = QVector2IBKVector(t.translation() );

					// and we calculate back the projection on the plane
					// we have to take the offset of our new scaled polygon
					IBKMK::planeCoordinates(modS.geometry().offset(), localX, localY, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);

				}

				newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
			}
			// we cache that we already handled all selected sub surfaces of the surface
			handledSurfaces.insert(parentSurf);

			// we update the 2D polyline
			modS.setSubSurfaces(newSubSurfs);


			modifiedSurfaces.push_back(modS);
		}
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
	return;

	SVUndoModifySurfaceGeometry * undoSurf = new SVUndoModifySurfaceGeometry(tr("Translated surface geometry"), modifiedSurfaces );
	undoSurf->push();

	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
}


void SVPropEditGeometry::scale() {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	// we now apply the already specified transformation
	// get translation and scale vector from selected geometry object
	IBKMK::Vector3D scale = QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.scale());
	IBKMK::Vector3D trans = QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation());

	if (scale == IBKMK::Vector3D() || scale == IBKMK::Vector3D(1,1,1) )
		return;
	QVector3D transLCSO = cso->translation();

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface>			modifiedSurfaces;
	std::set<const VICUS::Surface*>		handledSurfaces;

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			std::vector<IBKMK::Vector3D> vertexes = s->polygon3D().vertexes();
			for ( IBKMK::Vector3D & v : vertexes ) {
				// use just this instead of making a QVetor3D
				Vic3D::Transform3D t;
				t.scale(IBKVector2QVector(scale) );
				t.translate(IBKVector2QVector(trans) );
				t.setTranslation(t.toMatrix()*IBKVector2QVector(v) );
				v = QVector2IBKVector(t.translation());
			}
			VICUS::Surface modS(*s);

			modS.setPolygon3D( VICUS::Polygon3D(vertexes) );

			// we cache our poldon data
			IBKMK::Vector3D offset3dOld = s->geometry().offset();
			IBKMK::Vector3D offset3dNew = modS.geometry().offset();

			const IBKMK::Vector3D &localXOld = s->geometry().localX();
			const IBKMK::Vector3D &localYOld = s->geometry().localY();

			const IBKMK::Vector3D &localXNew = modS.geometry().localX();
			const IBKMK::Vector3D &localYNew = modS.geometry().localY();

			std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );
			// now we also have to scale the sub surfaces
			for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

				Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());

				newSubSurfs[i] = modS.subSurfaces()[i];
				VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(s->subSurfaces()[i]);
				std::vector<IBKMK::Vector2D> newSubSurfVertexes(subS.m_polygon2D.vertexes().size());

				for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
					IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

					// we now calculate the 3D points of the sub surface
					// afterwards we scale up the surface
					IBKMK::Vector3D v3D = offset3dOld + localXOld * v2D.m_x + localYOld * v2D.m_y;

					Vic3D::Transform3D t;
					if (s->subSurfaces()[i].m_selected && s->subSurfaces()[i].m_visible) {
						t.scale(IBKVector2QVector(scale) );
						t.translate(IBKVector2QVector(trans) );
						t.setTranslation(t.toMatrix()*IBKVector2QVector(v3D) );
						v3D = QVector2IBKVector(t.translation() );
					}
					// and we calculate back the projection on the plane
					// we have to take the offset of our new scaled polygon
					IBKMK::planeCoordinates(offset3dNew, localXNew, localYNew, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);

				}

				newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
			}
			// we update the 2D polyline
			modS.setSubSurfaces(newSubSurfs);

			// We update the floor area
			if (modS.m_parent != nullptr && modS.geometry().normal().m_z < -0.707)
				VICUS::KeywordList::setParameter(dynamic_cast<VICUS::Room*>(modS.m_parent)->m_para, "Room::para_t", VICUS::Room::P_Area, modS.geometry().area() );

			modifiedSurfaces.push_back(modS);
		}

		// handle only selected sub surfaces where parent is not selected
		const VICUS::SubSurface * ss = dynamic_cast<const VICUS::SubSurface *>(o);
		if (ss != nullptr) {

			VICUS::Surface *parentSurf = dynamic_cast<VICUS::Surface*>(ss->m_parent);
			if (parentSurf != nullptr && ss->m_parent->m_selected && ss->m_parent->m_visible)
				continue; // already handled by surface scaling

			if(handledSurfaces.find(parentSurf) != handledSurfaces.end())
				continue; // surface already handled

			// parentSurf->m_selected = true; // !!!! only for now till we adjust the function !!!
			VICUS::Surface modS(*parentSurf);

			// we cache our poldon data
			IBKMK::Vector3D offset3d = modS.geometry().offset();
			const IBKMK::Vector3D &localX = modS.geometry().localX();
			const IBKMK::Vector3D &localY = modS.geometry().localY();

			std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );
			// now we also have to scale the sub surfaces
			for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

				Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());
				newSubSurfs[i] = modS.subSurfaces()[i];

				if (!modS.subSurfaces()[i].m_selected || !modS.subSurfaces()[i].m_visible)
					continue;

				// we only modify our selected sub surface
				VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(modS.subSurfaces()[i]);
				std::vector<IBKMK::Vector2D> newSubSurfVertexes(subS.m_polygon2D.vertexes().size());


				for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
					IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

					// we now calculate the 3D points of the sub surface
					// afterwards we scale up the surface
					IBKMK::Vector3D v3D = offset3d + localX * v2D.m_x + localY * v2D.m_y;

					Vic3D::Transform3D t;
					t.scale(IBKVector2QVector(scale) );
					t.translate(IBKVector2QVector(trans) );
					t.setTranslation(t.toMatrix()*IBKVector2QVector(v3D) );
					v3D = QVector2IBKVector(t.translation() );

					// and we calculate back the projection on the plane
					// we have to take the offset of our new scaled polygon
					IBKMK::planeCoordinates(modS.geometry().offset(), localX, localY, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);
				}

				newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
			}
			handledSurfaces.insert(parentSurf);

			// we update the 2D polyline
			modS.setSubSurfaces(newSubSurfs);
			modifiedSurfaces.push_back(modS);
		}

		// TODO : Netzwerk zeugs
	}

	// ToDO Update Volumen

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undoSurf = new SVUndoModifySurfaceGeometry(tr("Scaled surface geometry."), modifiedSurfaces );
	undoSurf->push();

	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	cso->setTranslation(transLCSO);
}


void SVPropEditGeometry::rotate() {
	FUNCID(SVPropEditGeometry::rotate);
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;

	// we now apply the already specified transformation
	// get rotation and scale vector from selected geometry object
	QVector4D qrotate = SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.rotation().toVector4D();
	// use IBKMK class to permorm rotation with IBKMK Vector3D
	IBKMK::Quaternion rotate((double) qrotate.w(),(double) qrotate.x(),(double) qrotate.y(),(double) qrotate.z());
	IBKMK::Vector3D trans = QVector2IBKVector( SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation() );

	if (rotate == IBKMK::Quaternion())
		return;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface>			modifiedSurfaces;
	std::set<const VICUS::Surface*>		handledSurfaces;
	QVector3D transLCSO = cso->translation();

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			VICUS::Surface modS(*s);
			IBKMK::Polygon3D &poly = const_cast<IBKMK::Polygon3D&>(modS.polygon3D());
			// TODO : Stephan, can we fix already broken polygons?
			if (!poly.isValid())
				continue; // skip invalid polygons

			// we copy the surface's local, normal and offset
			IBKMK::Vector3D localX = poly.localX();
			IBKMK::Vector3D normal = poly.normal();
			IBKMK::Vector3D offset = poly.offset();

			// we rotate our axis and offset
			rotate.rotateVector(localX);
			rotate.rotateVector(normal);
			rotate.rotateVector(offset);

			// we need to calculate our new rotation
			try {
				// we set our rotated axises
				poly.setRotation(normal, localX);
				// we have to mind the we rotate around our
				// local coordinate system center point
				poly.translate(offset-poly.offset()+trans);
			} catch (IBK::Exception &ex) {
				throw IBK::Exception(IBK::FormatString("%2.\nPolygon '%1' could not be rotated").arg(s->m_displayName.toStdString()).arg(ex.what()), FUNC_ID);
				continue;
			}

			modifiedSurfaces.push_back(modS);
		}
		const VICUS::SubSurface * ss = dynamic_cast<const VICUS::SubSurface *>(o);
		if (ss != nullptr) {
			VICUS::Surface *parentSurf = dynamic_cast<VICUS::Surface*>(ss->m_parent);
			if (parentSurf != nullptr && ss->m_parent->m_selected && ss->m_parent->m_visible)
				continue; // already handled by surface scaling

			if(handledSurfaces.find(parentSurf) != handledSurfaces.end())
				continue; // surface already handled

			// parentSurf->m_selected = true; // !!!! only for now till we adjust the function !!!
			VICUS::Surface modS(*parentSurf);

			// we cache our poldon data
			IBKMK::Vector3D offset3d = modS.geometry().offset();
			const IBKMK::Vector3D &localX = modS.geometry().localX();
			const IBKMK::Vector3D &localY = modS.geometry().localY();

			std::vector<VICUS::SubSurface> newSubSurfs(modS.subSurfaces().size() );
			// now we also have to scale the sub surfaces
			for ( unsigned int i = 0; i<modS.subSurfaces().size(); ++i ) {

				Q_ASSERT(modS.subSurfaces().size() == modS.geometry().holes().size());
				newSubSurfs[i] = modS.subSurfaces()[i];

				if (!modS.subSurfaces()[i].m_selected || !modS.subSurfaces()[i].m_visible)
					continue;

				// we only modify our selected sub surface
				VICUS::SubSurface &subS = const_cast<VICUS::SubSurface &>(modS.subSurfaces()[i]);
				std::vector<IBKMK::Vector2D> newSubSurfVertexes(subS.m_polygon2D.vertexes().size());


				for ( unsigned int j=0; j<subS.m_polygon2D.vertexes().size(); ++j ) {
					IBKMK::Vector2D v2D = subS.m_polygon2D.vertexes()[j];

					// we now calculate the 3D points of the sub surface
					IBKMK::Vector3D v3D = offset3d + localX * v2D.m_x + localY * v2D.m_y;
					rotate.rotateVector(v3D);
					v3D = v3D + trans;

					// and we calculate back the projection on the plane
					// we have to take the offset of our new scaled polygon
					IBKMK::planeCoordinates(modS.geometry().offset(), localX, localY, v3D, newSubSurfVertexes[j].m_x, newSubSurfVertexes[j].m_y);
				}

				newSubSurfs[i].m_polygon2D = newSubSurfVertexes;
			}
			handledSurfaces.insert(parentSurf);

			// we update the 2D polyline
			modS.setSubSurfaces(newSubSurfs);
			modifiedSurfaces.push_back(modS);

		}
		// TODO : Netzwerk zeugs
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	blockSignals(true);
	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Rotated geometry"), modifiedSurfaces );
	undo->push();
	// reset local transformation matrix
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	cso->setTranslation(transLCSO);
	blockSignals(false);

}

