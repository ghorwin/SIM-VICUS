#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

#include <IBK_physics.h>

#include <VICUS_Project.h>
#include <VICUS_Object.h>
#include <VICUS_BuildingLevel.h>

#include <QMessageBox>

#include <QtExt_Conversions.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVUndoModifySurfaceGeometry.h"
#include "SVUndoAddSurface.h"
#include "SVUndoAddZone.h"
#include "SVUndoCopyZones.h"
#include "SVUndoCopySurfaces.h"
#include "SVPropVertexListWidget.h"
#include "SVGeometryView.h"

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

/*! Returns the inner Angle between two Vectors of a Polygon in Degree */
static double angleBetweenVectorsDeg ( const IBKMK::Vector3D &v1, const IBKMK::Vector3D &v2) {
	return std::acos( v1.scalarProduct(v2) / sqrt(v1.magnitude() * v2.magnitude() ) ) / IBK::DEG2RAD;
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
	m_ui->verticalLayoutPage1->setMargin(0);
	m_ui->verticalLayoutPage2->setMargin(0);

	SVViewStateHandler::instance().m_propEditGeometryWidget = this;

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropEditGeometry::onModified);

	connect(&SVViewStateHandler::instance(), &SVViewStateHandler::viewStateChanged,
			this, &SVPropEditGeometry::onViewStateChanged);

	m_ui->lineEditX->setup(-1E5,1E5,tr("X Value"),true, true);
	m_ui->lineEditY->setup(-1E5,1E5,tr("Y Value"),true, true);
	m_ui->lineEditZ->setup(-1E5,1E5,tr("Z Value"),true, true);

	m_ui->lineEditX->setFormatter(new LineEditFormater);
	m_ui->lineEditY->setFormatter(new LineEditFormater);
	m_ui->lineEditZ->setFormatter(new LineEditFormater);
	m_ui->lineEditInclination->setFormatter(new LineEditFormater);
	m_ui->lineEditOrientation->setFormatter(new LineEditFormater);

	m_ui->lineEditXCopy->setFormatter(new LineEditFormater);
	m_ui->lineEditYCopy->setFormatter(new LineEditFormater);
	m_ui->lineEditZCopy->setFormatter(new LineEditFormater);

	m_ui->lineEditX->installEventFilter(this);
	m_ui->lineEditY->installEventFilter(this);
	m_ui->lineEditZ->installEventFilter(this);
	m_ui->lineEditInclination->installEventFilter(this);
	m_ui->lineEditOrientation->installEventFilter(this);

	m_ui->lineEditXCopy->installEventFilter(this);
	m_ui->lineEditYCopy->installEventFilter(this);
	m_ui->lineEditZCopy->installEventFilter(this);

	m_modificationState[MT_Translate] = MS_Absolute;
	m_modificationState[MT_Rotate] = MS_Absolute;
	m_modificationState[MT_Scale] = MS_Absolute;

	// set initial states
	setState(MT_Translate, MS_Absolute);

	initializeCopy();

	m_ui->widgetXYZ->layout()->setMargin(0);
	m_ui->widgetRota->layout()->setMargin(0);

}


SVPropEditGeometry::~SVPropEditGeometry() {
	delete m_ui;
}


void SVPropEditGeometry::setCurrentPage(const SVPropEditGeometry::Operation & op) {
	switch (op) {
		case O_AddGeometry :
			m_ui->pushButtonAdd->setChecked(true);
			m_ui->pushButtonEdit->setChecked(false);
			m_ui->stackedWidget->setCurrentIndex(0);
		break;
		case O_EditGeometry :
			m_ui->pushButtonEdit->setChecked(true);
			m_ui->pushButtonAdd->setChecked(false);
			m_ui->stackedWidget->setCurrentIndex(1);
		break;
	}
}



void SVPropEditGeometry::setCoordinates(const Vic3D::Transform3D &t) {
	// is being call from local coordinate system object, whenever this has changed location (regardless of
	// its own visibility)
	m_localCoordinatePosition =  t;
	updateInputs();
}


void SVPropEditGeometry::setRotation(const IBKMK::Vector3D &normal) {
	// TODO nochmal nachdenken
	m_normal = normal.normalized();

	m_ui->lineEditInclination->setText( QString("%L1").arg(std::acos(normal.m_z)/IBK::DEG2RAD, 0, 'f', 3) );

	// positive y Richtung = Norden = Orientation 0°
	// positive x Richtung = Osten = Orientation 90°

	double orientation = std::atan2(normal.m_x, ( normal.m_y == 0. ? 1E-8 : normal.m_y ) ) /IBK::DEG2RAD ;
	m_ui->lineEditOrientation->setText( QString("%L1").arg(orientation < 0 ? ( orientation + 360 ) : orientation, 0, 'f', 3 ) );
}

void SVPropEditGeometry::onModified(int modificationType, ModificationInfo * ) {
	SVProjectHandler::ModificationTypes modType((SVProjectHandler::ModificationTypes)modificationType);
	switch (modType) {
	case SVProjectHandler::BuildingGeometryChanged:
	case SVProjectHandler::NodeStateModified:
		// when the building geometry has changed, we need to update the geometrical info
		// in the widget based on the current selection
		// also, we assume any change in node states (visibility/selection) may impact our local
		// coordinate system position
		update(); // this might update the location of the local coordinate system!
		break;

	default: ; // just to make compiler happy
	}
}


void SVPropEditGeometry::onViewStateChanged() {
	const SVViewState & vs = SVViewStateHandler::instance().viewState();
	if (vs.m_sceneOperationMode == SVViewState::OM_SelectedGeometry) {
		m_ui->pushButtonEdit->setEnabled(true);
	}
	else {
		setCurrentPage(O_AddGeometry);
		m_ui->pushButtonEdit->setEnabled(false);
		// clear current selection transformation matrix
		SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = Vic3D::Transform3D();
	}

	updateCoordinateSystemLook();
}



// *** slots ***

void SVPropEditGeometry::on_pushButtonAddPolygon_clicked() {
	// reset new polygon object and set it into polygon mode
	SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_Polygon);
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Polygon);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropEditGeometry::on_pushButtonAddRect_clicked() {
	// reset new polygon object and set it into rect mode
	SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_Rect);
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_Rect);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}


void SVPropEditGeometry::on_pushButtonAddZoneBox_clicked() {
	// reset new polygon object and set it into zone floor drawing mode
	SVViewStateHandler::instance().m_newGeometryObject->startNewGeometry(Vic3D::NewGeometryObject::NGM_ZoneFloor);
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
	// clear vertex list in property widget
	SVViewStateHandler::instance().m_propVertexListWidget->setup(Vic3D::NewGeometryObject::NGM_ZoneFloor);
	SVViewStateHandler::instance().m_geometryView->focusSceneView();
}




void SVPropEditGeometry::on_lineEditX_editingFinished() {
	on_lineEditX_returnPressed();
}

void SVPropEditGeometry::on_lineEditY_editingFinished()
{
	on_lineEditY_returnPressed();
}

void SVPropEditGeometry::on_lineEditZ_editingFinished()
{
	on_lineEditZ_returnPressed();
}


void SVPropEditGeometry::translate() {
	// get translation vector from selected geometry object
	IBKMK::Vector3D translation = QtExt::QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation());

	if (translation == IBKMK::Vector3D())
		return;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			VICUS::Surface modS(*s);
			std::vector<IBKMK::Vector3D> vertexes = modS.geometry().vertexes();
			for ( IBKMK::Vector3D & v : vertexes ) {
				// use just this instead of making a QVetor3D
				v += translation;
			}
			modS.m_polygon3D.setVertexes(vertexes);
			modifiedSurfaces.push_back(modS);
		}
		// TODO : Netzwerk zeugs
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Translated geometry"), modifiedSurfaces );
	undo->push();
}


void SVPropEditGeometry::scale() {
	// we now apply the already specified transformation
	// get translation and scale vector from selected geometry object
	IBKMK::Vector3D scale = QtExt::QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.scale());
	IBKMK::Vector3D trans = QtExt::QVector2IBKVector(SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation());

	if (scale == IBKMK::Vector3D())
		return;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			VICUS::Surface modS(*s);
			std::vector<IBKMK::Vector3D> vertexes = modS.geometry().vertexes();
			for ( IBKMK::Vector3D & v : vertexes ) {
				// use just this instead of making a QVetor3D
				Vic3D::Transform3D t;
				t.scale(QtExt::IBKVector2QVector(scale) );
				t.translate(QtExt::IBKVector2QVector(trans) );
				t.setTranslation(t.toMatrix()*QtExt::IBKVector2QVector(v) );
				v = QtExt::QVector2IBKVector(t.translation());
			}
			modS.m_polygon3D.setVertexes(vertexes);
			modifiedSurfaces.push_back(modS);
		}
		// TODO : Netzwerk zeugs
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Scaled geometry"), modifiedSurfaces );
	undo->push();
}


void SVPropEditGeometry::rotate() {
	// we now apply the already specified transformation
	// get rotation and scale vector from selected geometry object
	QQuaternion rotate = SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.rotation();
	QVector3D trans = SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.translation();

	if (rotate == QQuaternion())
		return;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;

	// process all selected objects
	for (const VICUS::Object * o : SVViewStateHandler::instance().m_selectedGeometryObject->m_selectedObjects) {
		// handle surfaces
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s != nullptr) {
			VICUS::Surface modS(*s);
			std::vector<IBKMK::Vector3D> vertexes = modS.m_geometry.vertexes();
			for ( IBKMK::Vector3D & v : vertexes ) {
				// use just this instead of making a QVetor3D
				QVector3D v3D = rotate.rotatedVector(QtExt::IBKVector2QVector(v) );

				Vic3D::Transform3D t;
				t.setTranslation( v3D );
				t.translate(trans);

				v = QtExt::QVector2IBKVector(t.translation() );
			}
			modS.m_geometry.setVertexes(vertexes);
			modifiedSurfaces.push_back(modS);
		}
		// TODO : Netzwerk zeugs
	}

	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Rotated geometry"), modifiedSurfaces );
	undo->push();
}


int SVPropEditGeometry::requestCopyOperation(QWidget * parent, const QString & title, const QString & text,
											 const QString & button1, const QString & button2)
{
	QMessageBox dlg(QMessageBox::Question, title, text, QMessageBox::Cancel, parent);

	QPushButton * btn = new QPushButton(button1);
	dlg.addButton(btn, (QMessageBox::ButtonRole)(QMessageBox::NRoles+1));
	dlg.setDefaultButton(btn);

	btn = new QPushButton(button2);
	dlg.addButton(btn, (QMessageBox::ButtonRole)(QMessageBox::NRoles+2));

	int res = dlg.exec();
	if (res == QMessageBox::Cancel)
		return -1;
	if (res == QMessageBox::NRoles+1)
		return 1;
	else
		return 2;
}

// *** private functions ***

void SVPropEditGeometry::update() {

	std::vector<const VICUS::Surface*> surfaces;

	// first we get how many surfaces are selected
	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);

	// we get the view state
	SVViewState vs = SVViewStateHandler::instance().viewState();

	if ( surfaces.size() > 0 ) {

		// we collect all selected objects
//		project().selectedBuildingObjects(m_selBuild, new VICUS::Building);
//		project().selectedBuildingObjects(m_selBuildLvls, new VICUS::BuildingLevel);
		project().selectedRooms(m_selRooms);
		project().selectedSurfaces(m_selSurfaces, VICUS::Project::SG_All);

		m_ui->pushButtonCopyRooms->setEnabled(m_selRooms.size()>0);
		m_ui->pushButtonCopyBuildingLvls->setEnabled(false);
		m_ui->pushButtonCopyBuilding->setEnabled(false);

		// adjust the view state to show selected geometry (i.e. local coordinate system is visible)
		// and edit geometry property widget (makes us visible), but only, if we are in
		// geometry editing mode
		if (vs.m_viewMode == SVViewState::VM_GeometryEditMode) {
			vs.m_sceneOperationMode = SVViewState::OM_SelectedGeometry;
			vs.m_propertyWidgetMode = SVViewState::PM_EditGeometry;
			SVViewStateHandler::instance().setViewState(vs);
		}

		if ( surfaces.size() == 1 ) {
			const VICUS::Surface *s = surfaces[0];
			m_ui->labelIndication->setText(tr("Normal:"));
			setRotation(s->m_geometry.normal() );
		}
		else {
			m_ui->labelIndication->setText(tr("z-Axis:"));
			setRotation( QtExt::QVector2IBKVector(SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis() ) );
		}
		// compute dimensions of bounding box (dx, dy, dz) and center point of all selected surfaces
		m_boundingBoxDimension = project().boundingBox(surfaces, m_boundingBoxCenter);

		// update local coordinates
		Vic3D::Transform3D t;
		t.setTranslation(QtExt::IBKVector2QVector(m_boundingBoxCenter) );
		setCoordinates( t ); // calls updateInputs() internally

		// update scaling factor
		SVViewStateHandler::instance().m_coordinateSystemObject->setTranslation(QtExt::IBKVector2QVector(m_boundingBoxCenter) );
	}
	else {
		// only switch view state back to "Add geometry", when we are in geometry mode
		if (vs.m_viewMode == SVViewState::VM_GeometryEditMode) {
			vs.m_sceneOperationMode = SVViewState::NUM_OM;
			vs.m_propertyWidgetMode = SVViewState::PM_AddGeometry;
			SVViewStateHandler::instance().setViewState(vs);
		}
	}

}


void SVPropEditGeometry::onWheelTurned(double offset, QtExt::ValidatingLineEdit * lineEdit) {
	if (!lineEdit->isValid())
		return; // invalid input, do nothing
	double val = lineEdit->value();
	val += offset;
	lineEdit->setValue(val); // this does not trigger any signals, so we need to send change info manually
	onLineEditTextChanged(lineEdit);
}


void SVPropEditGeometry::initializeCopy() {
	// initialize the translation vector
	m_translation = IBKMK::Vector3D( 0, 0, 0 );

	// we set up the lineEdit fields
	m_ui->lineEditXCopy->setValue( m_translation.m_x);
	m_ui->lineEditYCopy->setValue( m_translation.m_y);
	m_ui->lineEditZCopy->setValue( m_translation.m_z);
}


void SVPropEditGeometry::updateCoordinateSystemLook() {
	if (SVViewStateHandler::instance().m_geometryView == nullptr)
		return; // do nothing while initializing
	// adjust appearance of local coordinate system
	if (m_ui->stackedWidget->currentIndex() == 0) {
		// put local coordinate system back into "plain" mode
		if (SVViewStateHandler::instance().m_coordinateSystemObject->m_geometryTransformMode != 0) {
			SVViewStateHandler::instance().m_coordinateSystemObject->m_geometryTransformMode = 0;
			SVViewStateHandler::instance().m_geometryView->refreshSceneView();
		}
	}
	else {
		// put local coordinate system back into correct transform mode
		switch (m_modificationType) {
			case SVPropEditGeometry::MT_Translate:
				if (SVViewStateHandler::instance().m_coordinateSystemObject->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_Translate) {
					SVViewStateHandler::instance().m_coordinateSystemObject->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_Translate;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
			break;

			case SVPropEditGeometry::MT_Rotate:
				if (SVViewStateHandler::instance().m_coordinateSystemObject->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_RotateMask) {
					SVViewStateHandler::instance().m_coordinateSystemObject->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_RotateMask;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
			break;

			case SVPropEditGeometry::MT_Scale:
				if (SVViewStateHandler::instance().m_coordinateSystemObject->m_geometryTransformMode != Vic3D::CoordinateSystemObject::TM_ScaleMask) {
					SVViewStateHandler::instance().m_coordinateSystemObject->m_geometryTransformMode = Vic3D::CoordinateSystemObject::TM_ScaleMask;
					SVViewStateHandler::instance().m_geometryView->refreshSceneView();
				}
			break;
		}
	}
}


void SVPropEditGeometry::on_comboBox_activated(int newIndex) {
	// set new state
	m_modificationState[m_modificationType] = (ModificationState)newIndex;

	// now update inputs
	updateInputs();
}


bool SVPropEditGeometry::eventFilter(QObject * target, QEvent * event) {

	if ( event->type() == QEvent::Wheel ) {
		// we listen to scroll wheel turns only for some line edits
		if (target == m_ui->lineEditX ||
				target == m_ui->lineEditY ||
				target == m_ui->lineEditZ ||
				target == m_ui->lineEditInclination ||
				target == m_ui->lineEditOrientation ||
				target == m_ui->lineEditXCopy ||
				target == m_ui->lineEditYCopy||
				target == m_ui->lineEditZCopy )
		{	double delta = 0.1; // for copy operation

			switch (m_modificationType) {
				case MT_Translate				:
				case MT_Scale					: delta = 0.01; break;
				case MT_Rotate					: delta = 1; break;
			}

			QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
			// offset are changed in 0.01 steps
			double offset = (wheelEvent->delta()>0) ? delta : -delta;
			onWheelTurned(offset, qobject_cast<QtExt::ValidatingLineEdit*>(target)); // we know that target points to a ValidatingLineEdit
		}
	}
	return false;
}


void SVPropEditGeometry::on_lineEditX_returnPressed() {
	// check if entered value is valid, if not reset it to its default

	if ( !m_ui->lineEditX->isValid() ) {
		m_ui->lineEditX->setValue( m_originalValues.m_x);
		return;
	}

	//	double tempXValue = m_ui->lineEditX->value();
	switch ( m_modificationType ) {
		case MT_Translate: translate(); break;
		case MT_Scale: scale(); break;
		case MT_Rotate: rotate(); break;
	}
}

void SVPropEditGeometry::on_lineEditY_returnPressed() {
	// check if entered value is valid, if not reset it to its default

	if ( !m_ui->lineEditY->isValid() ) {
		m_ui->lineEditY->setValue( m_originalValues.m_y );
		return;
	}

	//	double tempXValue = m_ui->lineEditX->value();
	switch ( m_modificationType ) {
		case MT_Translate: translate(); break;
		case MT_Scale: scale(); break;
		case MT_Rotate: rotate(); break;
	}
}

void SVPropEditGeometry::on_lineEditZ_returnPressed(){
	// check if entered value is valid, if not reset it to its default

	if ( !m_ui->lineEditZ->isValid() ) {
		m_ui->lineEditZ->setValue( m_originalValues.m_z );
		return;
	}

	switch ( m_modificationType ) {
		case MT_Translate: translate(); break;
		case MT_Scale: scale(); break;
		case MT_Rotate: rotate(); break;
	}
}



void SVPropEditGeometry::setState(const SVPropEditGeometry::ModificationType & type,
								  const SVPropEditGeometry::ModificationState & state)
{
	// check/uncheck operation buttons
	setToolButton(type);

	// if combo box needs to be update (i.e. when a new operation has been selected), we need
	// to re-populate the combo box
	setComboBox(type, state); // triggers an update
	m_modificationState[type] = state;

	updateInputs();
	updateCoordinateSystemLook();
}


void SVPropEditGeometry::updateInputs() {

	ModificationState state = m_modificationState[m_modificationType];

	switch (m_modificationType) {
		case MT_Translate : {
			showDeg(false);
			showRotation(false);

			switch (state) {
				case MS_Absolute: {

					m_ui->labelX->setText("X");
					m_ui->labelY->setText("Y");
					m_ui->labelZ->setText("Z");

					// cache current local coordinate systems position as fall-back values
					m_originalValues = QtExt::QVector2IBKVector(m_localCoordinatePosition.translation());

					IBKMK::Vector3D trans(QtExt::QVector2IBKVector(m_localCoordinatePosition.translation()));
					m_ui->lineEditX->setValue(trans.m_x);
					m_ui->lineEditY->setValue(trans.m_y);
					m_ui->lineEditZ->setValue(trans.m_z);
				} break;

				default:
					m_ui->labelX->setText("ΔX");
					m_ui->labelY->setText("ΔY");
					m_ui->labelZ->setText("ΔZ");

					m_originalValues = IBKMK::Vector3D();

					m_ui->lineEditX->setValue(0);
					m_ui->lineEditY->setValue(0);
					m_ui->lineEditZ->setValue(0);
			} // switch

		} break;


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


		case MT_Scale: {

			showDeg(false);
			showRotation(false);

			switch (state) {
				case MS_Absolute: {

					m_ui->labelX->setText("L<sub>X</sub>");
					m_ui->labelY->setText("W<sub>Y</sub>");
					m_ui->labelZ->setText("H<sub>Z</sub>");

					m_originalValues = m_boundingBoxCenter;

					m_ui->lineEditX->setValue(m_boundingBoxDimension.m_x);
					m_ui->lineEditY->setValue(m_boundingBoxDimension.m_y);
					m_ui->lineEditZ->setValue(m_boundingBoxDimension.m_z);

					break;
				}
				default:
					m_ui->labelX->setText("X");
					m_ui->labelY->setText("Y");
					m_ui->labelZ->setText("Z");

					m_originalValues = IBKMK::Vector3D( 1,1,1 );

					m_ui->lineEditX->setValue(m_originalValues.m_x );
					m_ui->lineEditY->setValue(m_originalValues.m_y );
					m_ui->lineEditZ->setValue(m_originalValues.m_z );
			}
		} break;
	} // switch modification type
}


void SVPropEditGeometry::setToolButton(const SVPropEditGeometry::ModificationType & type) {
	m_modificationType = type;

	m_ui->toolButtonTrans->setChecked(false);
	m_ui->toolButtonRotate->setChecked(false);
	m_ui->toolButtonScale->setChecked(false);

	switch (type) {
		case MT_Translate:		m_ui->toolButtonTrans->setChecked(true);			break;
		case MT_Rotate:			m_ui->toolButtonRotate->setChecked(true);			break;
		case MT_Scale:			m_ui->toolButtonScale->setChecked(true);			break;
	}
}


void SVPropEditGeometry::setComboBox(const ModificationType & type, const ModificationState & state) {
	m_ui->comboBox->blockSignals(true);
	m_ui->comboBox->clear();

	switch (type) {
		case MT_Translate:
			m_ui->comboBox->addItem( tr("Move to world coordinates") );
			m_ui->comboBox->addItem( tr("Relative translation using global coordinate system") );
			m_ui->comboBox->addItem( tr("Relative translation using local coordinate system") );
			break;
		case MT_Rotate:
			m_ui->comboBox->addItem( tr("Align surface to angles") );
			m_ui->comboBox->addItem( tr("Relative rotation using global coordinate system") );
			m_ui->comboBox->addItem( tr("Relative rotation using local coordinate system") );
			break;
		case MT_Scale:
			m_ui->comboBox->addItem( tr("Resize surfaces") );
			m_ui->comboBox->addItem( tr("Relative scaling using global coordinate system") );
			m_ui->comboBox->addItem( tr("Relative scaling using local coordinate system") );
			break;
	}

	m_ui->comboBox->setCurrentIndex((int)state);
	m_ui->comboBox->blockSignals(false);

	on_comboBox_activated((int)state);
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
	if ( abs ) {
		m_ui->widgetXYZ->hide();
		m_ui->widgetRota->show();
	}
	else {
		m_ui->widgetXYZ->show();
		m_ui->widgetRota->hide();
	}
}


void SVPropEditGeometry::on_toolButtonTrans_clicked() {
	setState(MT_Translate, m_modificationState[MT_Translate]);
}


void SVPropEditGeometry::on_toolButtonRotate_clicked() {
	setState(MT_Rotate, m_modificationState[MT_Rotate]);
}


void SVPropEditGeometry::on_toolButtonScale_clicked() {
	setState(MT_Scale, m_modificationState[MT_Rotate]);
}


void SVPropEditGeometry::on_lineEditOrientation_returnPressed() {
	if ( !m_ui->lineEditOrientation->isValid() ) {
		m_ui->lineEditOrientation->setValue( m_originalValues.m_x );
		return;
	}

	rotate();
}


void SVPropEditGeometry::on_lineEditInclination_returnPressed() {

	if ( !m_ui->lineEditInclination->isValid() ) {
		m_ui->lineEditInclination->setValue( m_originalValues.m_y );
		return;
	}

	rotate();
}


void SVPropEditGeometry::on_lineEditOrientation_editingFinished() {
	on_lineEditOrientation_returnPressed();
}


void SVPropEditGeometry::on_lineEditInclination_editingFinished() {
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


void SVPropEditGeometry::onLineEditTextChanged(QtExt::ValidatingLineEdit * lineEdit) {
	// take transformation value, and if valid, modify transform in wire frame object

	if (!lineEdit->isValid())
		return;

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
					// compute offset from current local coordinate system position
					QVector3D translation = targetPos - m_localCoordinatePosition.translation();
					// now compose a transform object and set it in the wireframe object
					Vic3D::Transform3D trans;
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
					trans.setTranslation(translation);
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = trans;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;

					// inputs are local coordinate offsets
				case MS_Local : {
					// for this operation, we need all three coordinates
					const QVector3D LocalX(1.0f, 0.0f, 0.0f);
					const QVector3D LocalY(0.0f, 1.0f, 0.0f);
					const QVector3D LocalZ(0.0f, 0.0f, 1.0f);
					const QQuaternion& rot = m_localCoordinatePosition.rotation();
					QVector3D localX = rot.rotatedVector(LocalX);
					QVector3D localY = rot.rotatedVector(LocalY);
					QVector3D localZ = rot.rotatedVector(LocalZ);
					QVector3D translation = localX*(float)m_ui->lineEditX->value()
											+ localY*(float)m_ui->lineEditY->value()
											+ localZ*(float)m_ui->lineEditZ->value();
					// now compose a transform object and set it in the wireframe object
					Vic3D::Transform3D trans;
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
					IBKMK::Vector3D scale;

					scale.m_x = m_boundingBoxDimension.m_x < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_x ) < 1E-4 ? 1.0 : targetScale.m_x / ( m_boundingBoxDimension.m_x < 1E-4 ? 1.0 : m_boundingBoxDimension.m_x ) );
					scale.m_y = m_boundingBoxDimension.m_y < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_y ) < 1E-4 ? 1.0 : targetScale.m_y / ( m_boundingBoxDimension.m_y < 1E-4 ? 1.0 : m_boundingBoxDimension.m_y ) );
					scale.m_z = m_boundingBoxDimension.m_z < 1E-4 ? 1.0 : ( std::fabs(targetScale.m_z ) < 1E-4 ? 1.0 : targetScale.m_z / ( m_boundingBoxDimension.m_z < 1E-4 ? 1.0 : m_boundingBoxDimension.m_z ) );
					// now compose a transform object and set it in the wireframe object
					// first we scale our selected objects
					Vic3D::Transform3D scaling;
					scaling.setScale( QtExt::IBKVector2QVector(scale) );

					// and the we also hav to guarantee that the center point of the bounding box stays at the same position
					// this can be achieved since the center points are also just scaled by the specified scaling factors
					// so we know how big the absolute translation has to be
					IBKMK::Vector3D trans;
					trans.m_x = m_boundingBoxDimension.m_x < 1E-4 ? 0.0 : m_boundingBoxCenter.m_x * ( 1 - scale.m_x );
					trans.m_y = m_boundingBoxDimension.m_y < 1E-4 ? 0.0 : m_boundingBoxCenter.m_y * ( 1 - scale.m_y );
					trans.m_z = m_boundingBoxDimension.m_z < 1E-4 ? 0.0 : m_boundingBoxCenter.m_z * ( 1 - scale.m_z );
					scaling.setTranslation( QtExt::IBKVector2QVector(trans) );

					// we give our transformation to the wire frame object
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = scaling;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow(); // needed right now since two surfaces are shown
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;

				case MS_Local: {
					// for this operation, we get directly the surface scaling factors from the line edits
					// so it is basically like the absolute scaling, but we do not have to calculate the scaling factors
					// compute offset from current local coordinate system position
					const QVector3D xAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis();
					const QVector3D yAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localYAxis();
					const QVector3D zAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis();

					IBKMK::Vector3D scale(	m_ui->lineEditX->value()*double( xAxis.x()+yAxis.x()+zAxis.x() ),
											m_ui->lineEditY->value()*double( xAxis.y()+yAxis.y()+zAxis.y() ),
											m_ui->lineEditZ->value()*double( xAxis.z()+yAxis.z()+zAxis.z() ));

					Vic3D::Transform3D scaling;
					scaling.setScale( QtExt::IBKVector2QVector(scale) );

					// and the we also hav to guarantee that the center point of the bounding box stays at the same position
					// this can be achieved since the center points are also just scaled by the specified scaling factors
					// so we know how big the absolute translation has to be
					IBKMK::Vector3D trans;
					trans.m_x = m_boundingBoxDimension.m_x < 1E-4 ? 0.0 : m_boundingBoxCenter.m_x * ( 1 - scale.m_x );
					trans.m_y = m_boundingBoxDimension.m_y < 1E-4 ? 0.0 : m_boundingBoxCenter.m_y * ( 1 - scale.m_y );
					trans.m_z = m_boundingBoxDimension.m_z < 1E-4 ? 0.0 : m_boundingBoxCenter.m_z * ( 1 - scale.m_z );
					scaling.setTranslation( QtExt::IBKVector2QVector(trans) );

					// we give our transformation to the wire frame object
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = scaling;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;

				case MS_Relative: {
					// for this operation, we get directly the surface scaling factors from the line edits
					// so it is basically like the absolute scaling, but we do not have to calculate the scaling factors
					IBKMK::Vector3D  scale(m_ui->lineEditX->value(), m_ui->lineEditY->value(), m_ui->lineEditZ->value());
					// compute offset from current local coordinate system position
					Vic3D::Transform3D scaling;
					scaling.setScale( QtExt::IBKVector2QVector(scale) );

					// and the we also hav to guarantee that the center point of the bounding box stays at the same position
					// this can be achieved since the center points are also just scaled by the specified scaling factors
					// so we know how big the absolute translation has to be
					IBKMK::Vector3D trans;
					trans.m_x = m_boundingBoxDimension.m_x < 1E-4 ? 0.0 : m_boundingBoxCenter.m_x * ( 1 - scale.m_x );
					trans.m_y = m_boundingBoxDimension.m_y < 1E-4 ? 0.0 : m_boundingBoxCenter.m_y * ( 1 - scale.m_y );
					trans.m_z = m_boundingBoxDimension.m_z < 1E-4 ? 0.0 : m_boundingBoxCenter.m_z * ( 1 - scale.m_z );
					scaling.setTranslation( QtExt::IBKVector2QVector(trans) );

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
					double incliRad = m_ui->lineEditInclination->value() * IBK::DEG2RAD;

					IBKMK::Vector3D newNormal(	std::sin( oriRad ) * std::sin( incliRad ),
												std::cos( oriRad ) * std::sin( incliRad ),
												std::cos( incliRad ) );

					// we only want to rotate if the normal vectors are not the same
					if ( checkVectors<4>( m_normal, newNormal ) )
						return; // do nothing

					// we find the rotation axis by taking the cross product of the normal vector and the normal vector we want to
					// rotate to
					IBKMK::Vector3D rotationAxis ( m_normal.crossProduct(newNormal) );

					Vic3D::Transform3D rota;
					// we now also have to find the angle between both normals
					rota.rotate((float)angleBetweenVectorsDeg(m_normal, newNormal), QtExt::IBKVector2QVector(rotationAxis) );

					// we take the QQuarternion to rotate
					QQuaternion centerRota = rota.rotation();
					QVector3D newCenter = centerRota.rotatedVector(QtExt::IBKVector2QVector(m_boundingBoxCenter) );

					// we also have to find the center point after rotation and translate our center back to its origin
					rota.setTranslation(QtExt::IBKVector2QVector(m_boundingBoxCenter) - newCenter );

					// we give our tranfsformation to the wire frame object
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = rota;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;

				case MS_Local: {
					// now compose a transform object and set it in the wireframe object
					const QVector3D xAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis();
					const QVector3D yAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localYAxis();
					const QVector3D zAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis();

					Vic3D::Transform3D rota;
					if ( lineEdit == m_ui->lineEditX )
						rota.setRotation((float)m_ui->lineEditX->value(), xAxis);
					else if ( lineEdit == m_ui->lineEditY )
						rota.setRotation((float)m_ui->lineEditY->value(), yAxis);
					else if ( lineEdit == m_ui->lineEditZ )
						rota.setRotation((float)m_ui->lineEditZ->value(), zAxis);

					// and the we also hav to guarantee that the center point of the bounding box stays at the same position
					// this can be achieved since the center points are also just rotated by the specified rotation
					// so we know how big the absolute translation has to be
					QQuaternion centerRota = rota.rotation();
					QVector3D newCenter = centerRota.rotatedVector(QtExt::IBKVector2QVector(m_boundingBoxCenter) );

					rota.setTranslation(QtExt::IBKVector2QVector(m_boundingBoxCenter) - newCenter );

					// we give our tranfsformation to the wire frame object
					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = rota;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;

				case MS_Relative: {
					// now compose a transform object and set it in the wireframe object
					Vic3D::Transform3D rota;
					if ( lineEdit == m_ui->lineEditX )
						rota.setRotation((float)m_ui->lineEditX->value(), 1, 0, 0);
					else if ( lineEdit == m_ui->lineEditY )
						rota.setRotation((float)m_ui->lineEditY->value(), 0, 1, 0);
					else if ( lineEdit == m_ui->lineEditZ )
						rota.setRotation((float)m_ui->lineEditZ->value(), 0, 0, 1);

					// and the we also hav to guarantee that the center point of the bounding box stays at the same position
					// this can be achieved since the center points are also just rotated by the specified rotation
					// so we know how big the absolute translation has to be
					QQuaternion centerRota = rota.rotation();
					QVector3D newCenter = centerRota.rotatedVector(QtExt::IBKVector2QVector(m_boundingBoxCenter) );

					// we also have to find the center point after rotation and translate our center back to its origin
					rota.setTranslation(QtExt::IBKVector2QVector(m_boundingBoxCenter) - newCenter );

					SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = rota;
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
					const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
				} break;
			}
		} break;
	}
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


void SVPropEditGeometry::on_pushButtonCopyRooms_clicked() {
#if 0
	// selected objects
	std::set<const VICUS::Object*> allSelectedSurfs;
	project().selectObjects(allSelectedSurfs, VICUS::Project::SG_Building, false, false);

	std::vector<VICUS::Surface> allSurfaces;

	// we also find all that already exist
	std::set<QString> existingRoomNames;
	std::set<QString> existingSurfNames;
	for (const VICUS::Object *o : allSelectedSurfs) {
		if (const VICUS::Room *r = dynamic_cast<const VICUS::Room*>(o))
			existingRoomNames.insert(r->m_displayName);
		else if (const VICUS::Surface *s = dynamic_cast<const VICUS::Surface*>(o)) {
			existingSurfNames.insert(s->m_displayName);
			allSurfaces.push_back(*s);
		}
	}

	bool allSelected = true;

	// are all subsurfaces of selected rooms selected?
	for (const VICUS::Object *o : m_selRooms) {
		const VICUS::Room *room = dynamic_cast<const VICUS::Room*>(o);
		for(VICUS::Surface s : room->m_surfaces) {
			if ( !s.m_selected || !s.m_visible ) {
				allSelected = false;
				break;
			}
		}
	}

	// we get our copy method
	int copyAllSubSurfaces = 1;

	if ( !allSelected )
		copyAllSubSurfaces = SVPropEditCopyDialog::requestCopyMethod("Method for copying Rooms", tr("Copy all subsurfaces."),
																	 tr("Copy only visible and selected subsurfaces.") );
	if (copyAllSubSurfaces == -1)
		return;

	std::vector<VICUS::Room> newRooms;

	// we go through all objects and find the hierarchy
	for ( const VICUS::Room *r : m_selRooms ){
		// we find a room
		// we make a copy of the room
		VICUS::Room newRoom = r->clone();
		newRoom.m_id = VICUS::Project::uniqueId(project().m_plainGeometry);
		newRoom.m_displayName = VICUS::Project::uniqueName(r->m_displayName, existingRoomNames);

		VICUS::BuildingLevel * bl = dynamic_cast<VICUS::BuildingLevel*>(r->m_parent);
		Q_ASSERT(bl != nullptr);

		//	copiedObjects.insert(copiedObjects.end(), &newRoom);

		std::set<unsigned int> childObjects;
		newRoom.collectChildIDs(childObjects);

		std::vector<VICUS::Surface> newSurfaces;
		std::vector<VICUS::Surface*> newChilds;

		for ( unsigned int id : childObjects) {
			VICUS::Surface *s = dynamic_cast<VICUS::Surface*>(newRoom.findChild(id) );
			if ( s != nullptr ){
				if ( copyAllSubSurfaces == 0 && (!s->m_visible || !s->m_selected) )
					continue; // skip deselected and hidden surfaces

				VICUS::Surface newSurf = s->clone();
				newSurf.m_selected = true; // select copied surface

				// deselect old surface
				const_cast<VICUS::Surface*>(s)->m_selected = false;

				newSurf.m_displayName = VICUS::Project::uniqueName(s->m_displayName, existingSurfNames);
				newSurf.m_id = VICUS::Project::uniqueId(allSurfaces);

				std::vector<IBKMK::Vector3D> vertexes = newSurf.m_geometry.vertexes();
				for ( IBKMK::Vector3D &v : vertexes ) {
					v += m_translation;
				}
				newSurf.m_geometry.setVertexes(vertexes);
				newSurf.m_parent = &newRoom;

				newSurfaces.push_back(newSurf);
			}
		}

		newRoom.m_surfaces = newSurfaces;
		newRoom.updateParents();

		// add room to copied rooms
		newRooms.push_back(newRoom);
	}

	SVUndoCopyZones *undo = new SVUndoCopyZones("Copied Zones", 0, newRooms);
	undo->push();
#endif
}

void SVPropEditGeometry::on_pushButtonCopySurfaces_clicked() {
	// get all selected objects (only those that are part of buildings, visible and invisible)
	std::set<const VICUS::Object*> selectedObjs;
	project().selectObjects(selectedObjs, VICUS::Project::SG_Building, true, false);

	// extract all surfaces and create a vector with *all* selected surfaces and with
	// only the visible selected surfaces
	std::vector<const VICUS::Surface *> selectedSurfaces;
	std::vector<const VICUS::Surface *> visibleSurfaces;
	for (const VICUS::Object * o : selectedObjs) {
		const VICUS::Surface * s = dynamic_cast<const VICUS::Surface *>(o);
		if (s == nullptr) continue;
		if (s->m_visible)
			visibleSurfaces.push_back(s); // remember as visible
		selectedSurfaces.push_back(s);
	}

	// we get our copy method
	int copyAllSubSurfaces = 1;

	// if we have some invisible ones, ask user what to do
	if (visibleSurfaces.size() != selectedSurfaces.size())
		copyAllSubSurfaces = requestCopyOperation(this, tr("Method for copying surfaces"),
			tr("Some of the selected surfaces are invisible. Copy them anyway?"),
			tr("Copy all selected surfaces"), tr("Copy only selected and visible surfaces") );

	if (copyAllSubSurfaces == -1)
		return;

	// if copy operation is "visible only", take only the visible surfaces
	if (copyAllSubSurfaces == 2)
		selectedSurfaces = visibleSurfaces;

	// to generate new unique IDs, we need a vector with all surface IDs used so far
	std::vector<unsigned int> idsUsedBySurfaces;
	for (const VICUS::Building & b : project().m_buildings)
		for (const VICUS::BuildingLevel & bl : b.m_buildingLevels)
			for (const VICUS::Room & r : bl.m_rooms)
				for (const VICUS::Surface & s : r.m_surfaces)
					idsUsedBySurfaces.push_back(s.m_id);

	// now create a vector for the new surfaces
	std::vector<VICUS::Surface> newSurfaces;
	// unique-IDs of deselected surfaces
	std::set<unsigned int> deselectedSurfaces;

	// this map stores the old vs. new ID association, needed for copying component instance
	std::map<unsigned int, unsigned int> oldNewIDMap;

	// we go through all objects and find the hierarchy
	for (const VICUS::Surface * s : selectedSurfaces) {

		// we make a copy of the surface but with a new unique ID
		VICUS::Surface newSurf = s->clone();
		newSurf.m_selected = true; // select copied surface

		// deselect old surface
		// Warning: we must not do this directly in memory with a const_cast, because this screws up
		//          undo history
		deselectedSurfaces.insert(s->uniqueID());

		newSurf.m_displayName = s->m_displayName + "-2"; // we can always append a 2 here
		newSurf.m_id = VICUS::Project::uniqueId(idsUsedBySurfaces);
		idsUsedBySurfaces.push_back(newSurf.m_id); // also remember the new ID as used

		std::vector<IBKMK::Vector3D> vertexes = newSurf.m_geometry.vertexes();
		for ( IBKMK::Vector3D &v : vertexes ) {
			v += m_translation;
		}
		newSurf.m_geometry.setVertexes(vertexes);

		newSurfaces.push_back(newSurf);

		// remember old vs. new surface ID map
		oldNewIDMap[s->m_id] = newSurf.m_id;
	}

	// new component instances to be created
	std::vector<VICUS::ComponentInstance> newComponentInstances;

	std::vector<unsigned int> compInstanceIDs;
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances)
		compInstanceIDs.push_back(ci.m_id);

	// process all existing component instances
	for (const VICUS::ComponentInstance & ci : project().m_componentInstances) {
		// we create a copy of the component instance
		bool leftSideUsed =
				(ci.m_sideASurfaceID != VICUS::INVALID_ID && VICUS::Project::contains(m_selSurfaces, ci.m_sideASurfaceID));
		bool rightSideUsed =
				(ci.m_sideBSurfaceID != VICUS::INVALID_ID && VICUS::Project::contains(m_selSurfaces, ci.m_sideBSurfaceID));

		// skip unrelated component instances
		if (!leftSideUsed && !rightSideUsed)
			continue;

		// create copy of CI
		VICUS::ComponentInstance newCi;
		newCi.m_id = VICUS::Project::uniqueId(compInstanceIDs);
		compInstanceIDs.push_back(newCi.m_id);
		newCi.m_componentID = ci.m_componentID;
		if (leftSideUsed)
			newCi.m_sideASurfaceID = oldNewIDMap[ci.m_sideASurfaceID];
		if (rightSideUsed)
			newCi.m_sideBSurfaceID = oldNewIDMap[ci.m_sideBSurfaceID];
		newComponentInstances.push_back(newCi);
	}

	SVUndoCopySurfaces *undo = new SVUndoCopySurfaces("Copied Surfaces.", newSurfaces, deselectedSurfaces, newComponentInstances);
	undo->push();
}


void SVPropEditGeometry::on_pushButtonCopyBuildingLvls_clicked() {

}


void SVPropEditGeometry::on_pushButtonAdd_clicked() {
	setCurrentPage(O_AddGeometry);
	updateCoordinateSystemLook();
}


void SVPropEditGeometry::on_pushButtonEdit_clicked() {
	setCurrentPage(O_EditGeometry);
	updateCoordinateSystemLook();
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
			modS.m_geometry.flip();
			modifiedSurfaces.push_back(modS);
		}
	}


	// in case operation was executed without any selected objects - should be prevented
	if (modifiedSurfaces.empty())
		return;

	// create undo-action
	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Flipped normal vectors"), modifiedSurfaces );
	undo->push();
}
