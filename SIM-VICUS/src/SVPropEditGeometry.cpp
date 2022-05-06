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
#include "SVUndoCopyBuildingGeometry.h"
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
bool vectors_equal(const IBKMK::Vector3D &v1, const IBKMK::Vector3D &v2 ) {
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
	m_copyTranslationVector(0,0,3),
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

	m_ui->lineEditTranslateX->installEventFilter(this);
	m_ui->lineEditTranslateY->installEventFilter(this);
	m_ui->lineEditTranslateZ->installEventFilter(this);


	m_ui->lineEditRotateOrientation->setFormatter(new LineEditFormater);
	m_ui->lineEditRotateInclination->setFormatter(new LineEditFormater);
	m_ui->lineEditRotateInclination->setup(-90, 90, tr("Inclination must be between -90..90°."), true, true);
	m_ui->lineEditRotateX->setFormatter(new LineEditFormater);
	m_ui->lineEditRotateY->setFormatter(new LineEditFormater);
	m_ui->lineEditRotateZ->setFormatter(new LineEditFormater);

	m_ui->lineEditRotateOrientation->installEventFilter(this);
	m_ui->lineEditRotateInclination->installEventFilter(this);
	m_ui->lineEditRotateX->installEventFilter(this);
	m_ui->lineEditRotateY->installEventFilter(this);
	m_ui->lineEditRotateZ->installEventFilter(this);


	m_ui->lineEditScaleX->setup(0, 1E5, tr("A positive value is required here."), false, true);
	m_ui->lineEditScaleY->setup(0, 1E5, tr("A positive value is required here."), false, true);
	m_ui->lineEditScaleZ->setup(0, 1E5, tr("A positive value is required here."), false, true);

	m_ui->lineEditScaleX->setFormatter(new LineEditFormater);
	m_ui->lineEditScaleY->setFormatter(new LineEditFormater);
	m_ui->lineEditScaleZ->setFormatter(new LineEditFormater);

	m_ui->lineEditScaleX->installEventFilter(this);
	m_ui->lineEditScaleY->installEventFilter(this);
	m_ui->lineEditScaleZ->installEventFilter(this);

	m_ui->lineEditCopyX->setText( QString("%L1").arg(m_copyTranslationVector.m_x,0,'f',3));
	m_ui->lineEditCopyY->setText( QString("%L1").arg(m_copyTranslationVector.m_y,0,'f',3));
	m_ui->lineEditCopyZ->setText( QString("%L1").arg(m_copyTranslationVector.m_z,0,'f',3));

	m_ui->lineEditCopyX->setFormatter(new LineEditFormater);
	m_ui->lineEditCopyY->setFormatter(new LineEditFormater);
	m_ui->lineEditCopyZ->setFormatter(new LineEditFormater);

	for (int i=0; i<4; ++i)
		m_ui->stackedWidget->widget(i)->layout()->setMargin(0);
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
	updateInputs(); // update all inputs
	// only adjust local coordinate system, if this widget is visible
	if (this->isVisibleTo(qobject_cast<QWidget*>(parent())) ) {
		// Note: setting new coordinates to the local coordinate system object will in turn call setCoordinates()
		//       and indirectly also updateInputs()
		updateCoordinateSystemLook();
		adjustLocalCoordinateSystemForRotateToAngle();
	}
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


	m_normal = QVector2IBKVector(cso->localZAxis());
	updateInputs();
}


bool SVPropEditGeometry::handleGlobalKeyPress(Qt::Key k) {
	switch (k) {
		case Qt::Key_Escape :
			if (!m_ui->pushButtonCancel->isEnabled())
				return false;
			m_ui->pushButtonCancel->click();
		break;

		default:
			return false;
	}
	return true;
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
		updateUi();
		break;

	default: ; // just to make compiler happy
	}
}


void SVPropEditGeometry::onViewStateChanged() {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	if (vs.m_sceneOperationMode == SVViewState::NUM_OM) {
		SVViewStateHandler::instance().m_selectedGeometryObject->resetTransformation();
	}
}



// *** PROTECTED FUNCTIONS ***

bool SVPropEditGeometry::eventFilter(QObject * target, QEvent * event) {

	if ( event->type() == QEvent::Wheel ) {
		// we listen to scroll wheel turns only for some line edits
		if (target == m_ui->lineEditTranslateX ||
				target == m_ui->lineEditTranslateY ||
				target == m_ui->lineEditTranslateZ ||
				target == m_ui->lineEditRotateInclination ||
				target == m_ui->lineEditRotateOrientation ||
				target == m_ui->lineEditRotateX ||
				target == m_ui->lineEditRotateY ||
				target == m_ui->lineEditRotateZ ||
				target == m_ui->lineEditScaleX ||
				target == m_ui->lineEditScaleY ||
				target == m_ui->lineEditScaleZ ||
				target == m_ui->lineEditCopyX ||
				target == m_ui->lineEditCopyY ||
				target == m_ui->lineEditCopyZ )
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



void SVPropEditGeometry::on_radioButtonRotationAlignToAngles_toggled(bool) {
	updateInputs();
	adjustLocalCoordinateSystemForRotateToAngle();
}

void SVPropEditGeometry::on_lineEditRotateOrientation_editingFinishedSuccessfully() {
	updateRotationPreview();
}

void SVPropEditGeometry::on_lineEditRotateInclination_editingFinishedSuccessfully() {
	updateRotationPreview();
}

void SVPropEditGeometry::on_lineEditRotateX_editingFinishedSuccessfully() {
	m_ui->lineEditRotateY->setValue(0);
	m_ui->lineEditRotateZ->setValue(0);
	updateRotationPreview();
}

void SVPropEditGeometry::on_lineEditRotateY_editingFinishedSuccessfully() {
	m_ui->lineEditRotateX->setValue(0);
	m_ui->lineEditRotateZ->setValue(0);
	updateRotationPreview();
}

void SVPropEditGeometry::on_lineEditRotateZ_editingFinishedSuccessfully() {
	m_ui->lineEditRotateX->setValue(0);
	m_ui->lineEditRotateY->setValue(0);
	updateRotationPreview();
}

void SVPropEditGeometry::on_pushButtonThreePointRotation_clicked() {
	// when clicked, we set the scene into three-point-rotation mode
	// TODO Stephan
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


void SVPropEditGeometry::on_pushButtonCenterHorizontally_clicked() {
	QMessageBox::critical(this, QString(), tr("Not implemented, yet."));
}


void SVPropEditGeometry::on_pushButtonCenterVertically_clicked() {
	QMessageBox::critical(this, QString(), tr("Not implemented, yet."));
}


void SVPropEditGeometry::on_pushButtonFlipNormals_clicked() {
	// process all selected surfaces (not subsurfaces) and flip their normal vectors
	// this is done directly, without use of apply/cancel buttons
	std::vector<VICUS::Surface>			modifiedSurfaces;
	for (const VICUS::Surface* s : m_selSurfaces) {
		// create a copy of the surface
		VICUS::Surface modS(*s);
		modS.flip();
		modifiedSurfaces.push_back(modS);
	}
	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Surface normal flipped"), modifiedSurfaces );
	undo->push();

	// also disable apply and cancel buttons
	m_ui->pushButtonApply->setEnabled(false);
	m_ui->pushButtonCancel->setEnabled(false);
	SVViewStateHandler::instance().m_selectedGeometryObject->resetTransformation();
	// and update our inputs again
	updateUi();
}


void SVPropEditGeometry::on_lineEditCopyX_editingFinishedSuccessfully() {
	m_copyTranslationVector.m_x = m_ui->lineEditCopyX->value();
}

void SVPropEditGeometry::on_lineEditCopyY_editingFinishedSuccessfully() {
	m_copyTranslationVector.m_y = m_ui->lineEditCopyY->value();
}

void SVPropEditGeometry::on_lineEditCopyZ_editingFinishedSuccessfully() {
	m_copyTranslationVector.m_z = m_ui->lineEditCopyZ->value();
}



// *** PRIVATE FUNCTIONS ***

void SVPropEditGeometry::updateUi() {
	Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
	Q_ASSERT(cso != nullptr);

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

	// compute dimensions of bounding box (dx, dy, dz) and center point of all selected surfaces
	m_bbDim[OM_Local] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Local],
											  QVector2IBKVector(cso->translation() ),
											  QVector2IBKVector(cso->localXAxis() ),
											  QVector2IBKVector(cso->localYAxis() ),
											  QVector2IBKVector(cso->localZAxis() ) );
	m_bbDim[OM_Global] = project().boundingBox(m_selSurfaces, m_selSubSurfaces, m_bbCenter[OM_Global]);

	// NOTE: this function is being called even if edit geometry property widget is not
	SVViewStateHandler::instance().m_localCoordinateViewWidget->setBoundingBoxDimension(m_bbDim[OM_Global]);
	// position local coordinate system
	cso->setTranslation(IBKVector2QVector(m_bbCenter[OM_Global]) );

	// only adjust local coordinate system, if this widget is visible
	if (this->isVisibleTo(qobject_cast<QWidget*>(parent())) ) {
		// adjust local coordinate system based on selection
		adjustLocalCoordinateSystemForRotateToAngle();

		// Note: setting new coordinates to the local coordinate system object will in turn call setCoordinates()
		//       and indirectly also updateInputs()
		updateCoordinateSystemLook();
	}

	// enable buttons based on selections
	m_ui->pushButtonCopySurface->setEnabled(!m_selSurfaces.empty());
	m_ui->pushButtonCopySubsurface->setEnabled(!m_selSubSurfaces.empty());
	m_ui->pushButtonCopyRoom->setEnabled(!m_selRooms.empty());
	m_ui->pushButtonCopyBuildingLevel->setEnabled(!m_selBuildingLevels.empty());
	m_ui->pushButtonCopyBuilding->setEnabled(!m_selBuildings.empty());
}


void SVPropEditGeometry::updateTranslationPreview() {
	double newX = m_ui->lineEditTranslateX->value();
	double newY = m_ui->lineEditTranslateY->value();
	double newZ = m_ui->lineEditTranslateZ->value();
	QVector3D translation = QVector3D((float)newX, (float)newY, (float)newZ);
	if (m_ui->radioButtonTranslationAbsolute->isChecked()) {
		// obtain new absolute position of local coordinate system
		// subtract original lcs position
		// obtain offset and rotation of local coordinate system
		QVector3D offset = m_lcsTransform.translation();
		translation -= offset;
	}

	// adjust wireframe object transform
	SVViewStateHandler::instance().m_selectedGeometryObject->translate(translation);
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();

	// also enable the apply/cancel buttons
	enableTransformation();
}


void SVPropEditGeometry::updateRotationPreview() {

	Vic3D::Transform3D rot;
	if (m_ui->radioButtonRotationAlignToAngles->isChecked()) {
		// get the rotation angles in [Rad]
		double oriDeg = m_ui->lineEditRotateOrientation->value();
		// inclination is 90-inc...
		double incliRad = (90-m_ui->lineEditRotateInclination->value()) * IBK::DEG2RAD;
		double oldOrientationDeg = std::atan2(m_normal.m_x, ( m_normal.m_y == 0. ? 1E-8 : m_normal.m_y ) ) / IBK::DEG2RAD;
		if (oldOrientationDeg < 0)
			oldOrientationDeg += 360;
//		qDebug() << "Old orientation deg = " << oldOrientationDeg;
//		qDebug() << "New orientation deg = " << oriDeg;
//		qDebug() << "New 90-inclination deg = " << incliRad / IBK::DEG2RAD;

		// we need to do two different rotations, one along the same orientation, but to different
		// inclination - hereby the rotation axis is computed from old normal vector and new normal vector
		// and

		IBKMK::Vector3D newNormal(	std::sin( oldOrientationDeg * IBK::DEG2RAD) * std::cos( incliRad ),
									std::cos( oldOrientationDeg * IBK::DEG2RAD) * std::cos( incliRad ),
									std::sin( incliRad ) );

		QQuaternion rotationInc;

		// we only want to rotate if the normal vectors are not the same - in this case we may not be able
		// to compute the rotation axis
		if (!vectors_equal<4>( m_normal, newNormal ) ) {

			// we find the rotation axis by taking the cross product of the normal vector and the normal vector we want to
			// rotate to
			IBKMK::Vector3D rotationAxis ( m_normal.crossProduct(newNormal).normalized() );
	//		qDebug() << "Rotation axis: " << rotationAxis.m_x << "\t" << rotationAxis.m_y << "\t" << rotationAxis.m_z;

			// we now also have to find the angle between both normals

			double angle = angleBetweenVectorsDeg(m_normal, newNormal);

			rotationInc = QQuaternion::fromAxisAndAngle(IBKVector2QVector(rotationAxis), (float)angle);
	//		qDebug() << "Roation angle: " << angle << " °";
		}

		QQuaternion rotationOrientation;

		// we only rotate around global z if angles differ
		if (!IBK::near_equal(oriDeg, oldOrientationDeg, 1e-4)) {
			rotationOrientation = QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), (float)(oriDeg - oldOrientationDeg));
		}
		rot.setRotation(rotationOrientation*rotationInc);

	}
	else {
		Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
		Q_ASSERT(cso != nullptr);

		// only one of the inputs may have a value different from 0
		if (m_ui->lineEditRotateX->isValid() && m_ui->lineEditRotateX->value() != 0.)
			rot.setRotation((float)m_ui->lineEditRotateX->value(), cso->localXAxis());
		if (m_ui->lineEditRotateY->isValid() && m_ui->lineEditRotateY->value() != 0.)
			rot.setRotation((float)m_ui->lineEditRotateY->value(), cso->localYAxis());
		if (m_ui->lineEditRotateZ->isValid() && m_ui->lineEditRotateZ->value() != 0.)
			rot.setRotation((float)m_ui->lineEditRotateZ->value(), cso->localZAxis());
	}

	// adjust wireframe object transform
	// we need to compute the transformation so that new lcs offset point becomes the original point again
	QVector3D newLCSOrigin = rot.rotation().rotatedVector(m_lcsTransform.translation());
	QVector3D trans = m_lcsTransform.translation() - newLCSOrigin;

	SVViewStateHandler::instance().m_selectedGeometryObject->rotate(rot.rotation(), trans);
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
	// obtain offset and rotation of local coordinate system
	QVector3D offset = m_lcsTransform.translation();
	QQuaternion rot = m_lcsTransform.rotation();

	// adjust wireframe object transform
	SVViewStateHandler::instance().m_selectedGeometryObject->localScaling(offset, rot, scaleFactors);
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();

	// also enable the apply/cancel buttons
	enableTransformation();
}


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
	else if (lineEdit == m_ui->lineEditRotateOrientation)
		on_lineEditRotateOrientation_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditRotateInclination)
		on_lineEditRotateInclination_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditRotateX)
		on_lineEditRotateX_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditRotateY)
		on_lineEditRotateY_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditRotateZ)
		on_lineEditRotateZ_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditScaleX)
		on_lineEditScaleX_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditScaleY)
		on_lineEditScaleY_editingFinishedSuccessfully();
	else if (lineEdit == m_ui->lineEditScaleZ)
		on_lineEditScaleZ_editingFinishedSuccessfully();
}


void SVPropEditGeometry::updateInputs() {
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


		case MT_Rotate: {
			bool align2Angles = m_ui->radioButtonRotationAlignToAngles->isChecked();
			m_ui->labelRotateInclinationAbs->setEnabled(align2Angles);
			m_ui->labelRotateOrientationAbs->setEnabled(align2Angles);
			m_ui->labelRotationX->setEnabled(!align2Angles);
			m_ui->labelRotationY->setEnabled(!align2Angles);
			m_ui->labelRotationZ->setEnabled(!align2Angles);
			m_ui->lineEditRotateInclination->setEnabled(align2Angles);
			m_ui->lineEditRotateOrientation->setEnabled(align2Angles);
			m_ui->lineEditRotateX->setEnabled(!align2Angles);
			m_ui->lineEditRotateY->setEnabled(!align2Angles);
			m_ui->lineEditRotateZ->setEnabled(!align2Angles);

			m_ui->lineEditRotateX->setValue(0);
			m_ui->lineEditRotateY->setValue(0);
			m_ui->lineEditRotateZ->setValue(0);

			if (align2Angles) {
				m_ui->lineEditRotateInclination->setText( QString("%L1").arg(std::acos(m_normal.m_z)/IBK::DEG2RAD, 0, 'f', 3) );
				// positive y Richtung = Norden = Orientation 0°
				// positive x Richtung = Osten = Orientation 90°

				double orientation = std::atan2(m_normal.m_x, ( m_normal.m_y == 0. ? 1E-8 : m_normal.m_y ) ) /IBK::DEG2RAD ;
				m_ui->lineEditRotateOrientation->setText( QString("%L1").arg(orientation < 0 ? ( orientation + 360 ) : orientation, 0, 'f', 3 ) );
			}
			else {
				m_ui->lineEditRotateInclination->setValue(0);
				m_ui->lineEditRotateOrientation->setValue(0);
			}

		} break;

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

		case MT_Copy: {
			// Nothing for now?
		} break;

	} // switch modification type

	// reset wireframe transform
	SVViewStateHandler::instance().m_selectedGeometryObject->resetTransformation();
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();

	// disable apply and cancel buttons
	m_ui->pushButtonApply->setEnabled(false);
	m_ui->pushButtonCancel->setEnabled(false);
}


void SVPropEditGeometry::adjustLocalCoordinateSystemForRotateToAngle() {
	if (!this->isVisibleTo(qobject_cast<QWidget*>(parent())) )
		return;
	// special handling when rotation mode with "orient to angle" is selected
	if (m_ui->stackedWidget->currentIndex() == MT_Rotate &&
		m_ui->radioButtonRotationAlignToAngles->isChecked() &&
		!m_selSurfaces.empty())
	{
		// check all selected surfaces and obtain their orientation/inclination
		IBKMK::Vector3D normal;
		bool allTheSame = true;
		for (const VICUS::Surface* surf : m_selSurfaces) {
			if (normal.magnitudeSquared() == 0.0)
				normal = surf->geometry().normal();
			else {
				if (!vectors_equal<4>(normal, surf->geometry().normal())) {
					allTheSame = false;
					break;
				}
			}
		}
		// all surfaces have same orientation/inclinatin - set this in the local coordinate system object
		if (allTheSame) {
			Vic3D::CoordinateSystemObject *cso = SVViewStateHandler::instance().m_coordinateSystemObject;
			const VICUS::PlaneGeometry & geo = m_selSurfaces.front()->geometry();
			QQuaternion q2 = QQuaternion::fromAxes(IBKVector2QVector(geo.localX().normalized()),
												   IBKVector2QVector(geo.localY().normalized()),
												   IBKVector2QVector(geo.normal().normalized()));
			cso->setRotation(q2);
			m_normal = geo.normal();
			// also update line edits
			updateInputs();
		}
	}
}



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


IBKMK::Vector3D SVPropEditGeometry::localCopyTranslationVector() const {
	QQuaternion q = SVViewStateHandler::instance().m_coordinateSystemObject->transform().rotation();
	IBKMK::Quaternion qm = QQuaternion2IBKQuaternion(q);
	IBKMK::Vector3D localCopyTranslationVector(m_copyTranslationVector);
	qm.rotateVector(localCopyTranslationVector);
	return localCopyTranslationVector;
}


void SVPropEditGeometry::on_pushButtonCancel_clicked() {
	// TODO
	// reset LCS when it had been moved as part of an interactive transformation, reset its original position
	// for now we just reset the LCS based on the current selection - but actually, the state before starting the transform
	// includes the (modified) location/orientation of the LCS _after_ the selection had been made
//	SVViewStateHandler::instance().m_coordinateSystemObject->setTranslation(SVViewStateHandler::instance().m_coordinateSystemObject->m_originalTranslation);
//	SVViewStateHandler::instance().m_coordinateSystemObject->setRotation(SVViewStateHandler::instance().m_coordinateSystemObject->m_originalRotation);
	SVViewStateHandler::instance().m_selectedGeometryObject->resetTransformation();
	// also disable apply and cancel buttons
	m_ui->pushButtonApply->setEnabled(false);
	m_ui->pushButtonCancel->setEnabled(false);
	// and update our inputs again
	updateUi(); // NOTE: this updates the position of the LCS again, but not its orientation!
	const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
}


void SVPropEditGeometry::on_pushButtonApply_clicked() {
	FUNCID(SVPropEditGeometry::on_pushButtonApply_clicked);

	// retrieve current transformation from selection object
	QVector3D translation, scaling;
	QQuaternion rotation;
	SVViewStateHandler::instance().m_selectedGeometryObject->currentTransformation(translation, rotation, scaling);


	// distinguish operations
	// we have a local scaling operation, which we need to do in a special way
	bool haveScaling = (scaling != QVector3D(0,0,0));

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface>			modifiedSurfaces;
	std::set<const VICUS::Surface*>		handledSurfaces;

	// process all selected surfaces
	for (const VICUS::Surface* s : m_selSurfaces) {
		// create a copy of the surface
		VICUS::Surface modS(*s);
		IBKMK::Polygon3D origPoly = const_cast<IBKMK::Polygon3D&>(modS.polygon3D());

		if (haveScaling) {
			// local scaling involves translation, rotation _and_ changes to the local polyome... hence we better work
			// on 3D polygon coordinates
			IBKMK::Polygon3D poly = const_cast<IBKMK::Polygon3D&>(modS.polygon3D());

			// get the transformation matrix
			QMatrix4x4 transMat = SVViewStateHandler::instance().m_selectedGeometryObject->transform().toMatrix();
			std::vector<IBKMK::Vector3D> verts = poly.vertexes();
			for (IBKMK::Vector3D & v : verts) {
				v = QVector2IBKVector( transMat*IBKVector2QVector(v) );
			}
			// reconstruct the polygon with new vertexes
			if (!poly.setVertexes(verts)) {
				IBK::IBK_Message("Error scaling polygon of surface.", IBK::MSG_WARNING, FUNC_ID);
				continue;
			}
			// now also scale all windows within the polygon
			// If the localX and localY vectors would not be normalized, we would already be finished.
			// But since we normalize the localX and localY vectors, we need to scale the local subsurface polygon coordinates

			// simplest way is to get the bounding box (in local coordinates) before the scaling and afterwards
			IBKMK::Vector2D lowerValuesOrig, upperValuesOrig;
			IBKMK::Vector2D lowerValuesNew, upperValuesNew;
			origPoly.polyline().boundingBox(lowerValuesOrig, upperValuesOrig);
			poly.polyline().boundingBox(lowerValuesNew, upperValuesNew);
			IBK_ASSERT(std::fabs(upperValuesOrig.m_x) > 1e-4);
			IBK_ASSERT(std::fabs(upperValuesOrig.m_y) > 1e-4);
			double scaleX = upperValuesNew.m_x/upperValuesOrig.m_x;
			double scaleY = upperValuesNew.m_y/upperValuesOrig.m_y;

			std::vector<VICUS::SubSurface> modSubs = modS.subSurfaces();
			for (VICUS::SubSurface & sub : modSubs) {
				std::vector<IBKMK::Vector2D> polyVerts = sub.m_polygon2D.vertexes();
				for (unsigned int i=0; i<polyVerts.size(); ++i) {
					polyVerts[i].m_x *= scaleX;
					polyVerts[i].m_y *= scaleY;
				}
				// now set the new subsurface polygon
				sub.m_polygon2D.setVertexes(polyVerts);
			}

			modS.setPolygon3D((VICUS::Polygon3D)poly);
			modS.setSubSurfaces(modSubs);
			modifiedSurfaces.push_back(modS);
		}
		else {
			// we have translation and/or rotation
			// since we only manipulate the local coordinate system, we can just use in-place modifications
			// also, we do not redo the triangulation, which speeds up things a bit
			IBKMK::Polygon3D &poly = const_cast<IBKMK::Polygon3D&>(modS.polygon3D());

			// we copy the surface's local, normal and offset
			IBKMK::Vector3D localX = poly.localX();
			IBKMK::Vector3D normal = poly.normal();
			IBKMK::Vector3D offset = poly.offset();
			IBKMK::Vector3D trans = QVector2IBKVector( translation );

			if (rotation != QQuaternion()) {
				// we rotate our axis and offset
				IBKMK::Quaternion rotate = QQuaternion2IBKQuaternion(rotation);
				rotate.rotateVector(localX);
				rotate.rotateVector(normal);
				rotate.rotateVector(offset);
			}

			trans = QVector2IBKVector(translation);

			// we set our rotated axises
			poly.setRotation(normal, localX);
			// we have to mind the we rotate around our
			// local coordinate system center point
			poly.translate(offset-poly.offset()+trans);

			modifiedSurfaces.push_back(modS);
		}

		// TODO : Stephan, scaling of subsurfaces _without_ their parent
		//
		// Note: m_selSubSurfaces may contain subsurfs whose parent surfaces are also selected - these
		//       need to be ignored since they are implicitely handled already.
		//
		// For isolated selected subsurface, we need to identify the parent surface and do:
		// - generate 3D vertexes for subsurface
		// - apply transformation to 3D vertexes
		// - compute projection of 3D vertexes onto local plane of parent subsurface and update polylines accordingly
		//
		// Mind: it is possible that due to the transformation the new subsurface geometry lies outside the original
		//       surface - this may easily happen if, for example, a subsurface is translated out of its parent surface.
		//       In this case the subsurface will still exist, but won't be a hole in the parent's surface geometry.
		//       TODO : such invalid subsurfaces should be visualized somehow.... ???? -> TODO : Andreas
#if 0
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
		}
#endif
		// TODO : Netzwerk zeugs
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Geometry modified"), modifiedSurfaces );
	undo->push();

	// also disable apply and cancel buttons
	m_ui->pushButtonApply->setEnabled(false);
	m_ui->pushButtonCancel->setEnabled(false);
	SVViewStateHandler::instance().m_selectedGeometryObject->resetTransformation();
	// and update our inputs again
	updateUi();
}


void SVPropEditGeometry::on_pushButtonCopySurface_clicked() {
	SVUndoCopyBuildingGeometry * undo = SVUndoCopyBuildingGeometry::createUndoCopySurfaces(
				m_selSurfaces, localCopyTranslationVector());
	undo->push();
}


void SVPropEditGeometry::on_pushButtonCopySubsurface_clicked() {
	QMessageBox::information(this, QString(), tr("Not implemented, yet."));
	SVUndoCopyBuildingGeometry * undo = SVUndoCopyBuildingGeometry::createUndoCopySubSurfaces(
				m_selSubSurfaces, localCopyTranslationVector());
	undo->push();
}


void SVPropEditGeometry::on_pushButtonCopyRoom_clicked() {
	SVUndoCopyBuildingGeometry * undo = SVUndoCopyBuildingGeometry::createUndoCopyRooms(
				m_selRooms, localCopyTranslationVector());
	undo->push();
}


void SVPropEditGeometry::on_pushButtonCopyBuildingLevel_clicked() {
	SVUndoCopyBuildingGeometry * undo = SVUndoCopyBuildingGeometry::createUndoCopyBuildingLevels(
				m_selBuildingLevels, localCopyTranslationVector());
	undo->push();
}


void SVPropEditGeometry::on_pushButtonCopyBuilding_clicked() {
	SVUndoCopyBuildingGeometry * undo = SVUndoCopyBuildingGeometry::createUndoCopyBuildings(
				m_selBuildings, localCopyTranslationVector());
	undo->push();
}

