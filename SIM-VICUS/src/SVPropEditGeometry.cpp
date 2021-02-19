#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

#include <IBK_physics.h>

#include <VICUS_Project.h>
#include <VICUS_Object.h>

#include <QtExt_Conversions.h>

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVUndoModifySurfaceGeometry.h"
#include "SVUndoAddSurface.h"
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
	QString formatted(double value) override {
		return QString("%L1").arg(value, 0, 'f', 3);
	}
};


// *** Widget implementation ***

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);

	SVViewStateHandler::instance().m_propEditGeometryWidget = this;

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropEditGeometry::onModified);

	m_ui->lineEditX->setup(-1E5,1E5,tr("X Value"),true, true);
	m_ui->lineEditY->setup(-1E5,1E5,tr("Y Value"),true, true);
	m_ui->lineEditZ->setup(-1E5,1E5,tr("Z Value"),true, true);

	m_ui->lineEditX->setFormatter(new LineEditFormater);
	m_ui->lineEditY->setFormatter(new LineEditFormater);
	m_ui->lineEditZ->setFormatter(new LineEditFormater);
	m_ui->lineEditInclination->setFormatter(new LineEditFormater);
	m_ui->lineEditOrientation->setFormatter(new LineEditFormater);

	m_ui->lineEditX->installEventFilter(this);
	m_ui->lineEditY->installEventFilter(this);
	m_ui->lineEditZ->installEventFilter(this);
	m_ui->lineEditInclination->installEventFilter(this);
	m_ui->lineEditOrientation->installEventFilter(this);

	m_modificationState[MT_Translate] = MS_Absolute;
	m_modificationState[MT_Rotate] = MS_Absolute;
	m_modificationState[MT_Scale] = MS_Absolute;

	// set initial states
	setState(MT_Translate, MS_Absolute);

	m_ui->widgetXYZ->layout()->setMargin(0);
	m_ui->widgetRota->layout()->setMargin(0);

}


SVPropEditGeometry::~SVPropEditGeometry() {
	delete m_ui;
}


void SVPropEditGeometry::setCurrentTab(const SVPropEditGeometry::TabState & state) {
	m_ui->toolBoxGeometry->setCurrentIndex(state);
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

	double orientation = std::atan2(normal.m_x, ( normal.m_y == 0 ? 1E-8 : normal.m_y ) ) /IBK::DEG2RAD ;
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
			std::vector<IBKMK::Vector3D> vertexes = modS.m_geometry.vertexes();
			for ( IBKMK::Vector3D & v : vertexes ) {
				// use just this instead of making a QVetor3D
				v += translation;
			}
			modS.m_geometry.setVertexes(vertexes);
			modifiedSurfaces.push_back(modS);
		}
		// TODO : Netzwerk zeugs
	}

	// also reset translation vector in selected geometry object
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.setTranslation(0,0,0);
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
			std::vector<IBKMK::Vector3D> vertexes = modS.m_geometry.vertexes();
			for ( IBKMK::Vector3D & v : vertexes ) {
				// use just this instead of making a QVetor3D
				Vic3D::Transform3D t;
				t.scale(QtExt::IBKVector2QVector(scale) );
				t.translate(QtExt::IBKVector2QVector(trans) );
				t.setTranslation(t.toMatrix()*QtExt::IBKVector2QVector(v) );
				v = QtExt::QVector2IBKVector(t.translation());
			}
			modS.m_geometry.setVertexes(vertexes);
			modifiedSurfaces.push_back(modS);
		}
		// TODO : Netzwerk zeugs
	}

	// also reset translation vector in selected geometry object
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.setTranslation(0,0,0);
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.setScale(1,1,1);
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

	// also reset translation vector in selected geometry object
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.setTranslation(0,0,0);
	SVViewStateHandler::instance().m_selectedGeometryObject->m_transform.setRotation(QQuaternion());
	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("Rotated geometry"), modifiedSurfaces );
	undo->push();
}

void SVPropEditGeometry::copy(const QVector3D &transVec)
{
#if 0
	// now we update all selected surfaces
	Vic3D::Transform3D trans;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;

	IBKMK::Vector3D centerPoint;
	std::vector<const VICUS::Surface*> surfaces;
	std::set<const VICUS::Object*> objects;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
	project().selectObjects(objects, VICUS::Project::SG_All, true, true);

	std::set<QString> existingRoomNames;

	for (const VICUS::Object *o : objects) {
		if (const VICUS::Room *room = dynamic_cast<const VICUS::Room*>(o))
			existingRoomNames.insert(room->m_displayName);
	}

	for ( const VICUS::Object *o : objects ){
		// check if also a room or building is selected
		if( const VICUS::Room *room = dynamic_cast<const VICUS::Room*>(o) ) {
			// room is also selected
			int i=0;
			std::set<unsigned int> surfs;

			VICUS::Room newRoom;
			newRoom = *room;
			newRoom.m_id = VICUS::Project::uniqueId(project().m_plainGeometry);
			newRoom.m_displayName = VICUS::Project::uniqueName(room->m_displayName, existingRoomNames);
			o->collectChildIDs(surfs);

			for (size_t i=0; i<surfs.size(); ++i) {
				for ( unsigned int ID : surfs ) {
					for ( const VICUS::Surface *s : surfaces ) {
						if ( s->m_id == ID ) {
							// copy surface
							// set new ID
							// new name


							VICUS::Surface newS = *s;
						}
					}
				}
			}
		}
		if( const VICUS::Building *building = dynamic_cast<const VICUS::Building*>( o ) ) {
			// building is also selected
			int i=0;
		}
	}


	for (const VICUS::Surface* s : surfaces ) {
		project().boundingBox(surfaces, centerPoint);
		std::vector<IBKMK::Vector3D> vs;
		for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
			Vic3D::Transform3D t;
			// use just this instead of making a QVetor3D
			t.setTranslation( v.m_x, v.m_y, v.m_z );
			t.translate( transVec );
			vs.push_back( IBKMK::Vector3D ( t.translation().x(), t.translation().y(), t.translation().z() ) );
		}
		VICUS::Surface newS(*s);
		newS.m_geometry.setVertexes(vs);
		modifiedSurfaces.push_back(newS);
	}

	VICUS::ComponentInstance ci;
	//	SVUndoAddSurface * undo = new SVUndoAddSurface(tr("modified surfaces"), modifiedSurfaces,, );
	//	undo->push();
#endif
}


// *** private functions ***

void SVPropEditGeometry::update() {
	// first we get how many surfaces are selected
	std::vector<const VICUS::Surface *> surfaces;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);

	// we get the view state
	SVViewState vs = SVViewStateHandler::instance().viewState();

	if ( surfaces.size() > 0 ) {
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
				target == m_ui->lineEditOrientation )
		{	double delta = 0.0;

			switch (m_modificationType) {
			case MT_Translate:
			case MT_Scale : delta = 0.01; break;
			case MT_Rotate : delta = 1; break;
			}

			QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
			// offset are changed in 0.01 steps
			double offset = (wheelEvent->delta()>0) ? delta : -delta;
			onWheelTurned(offset, (QtExt::ValidatingLineEdit*)target); // we know that target points to a ValidatingLineEdit
		}
	}
	return false;
}


void SVPropEditGeometry::on_lineEditX_returnPressed() {
	// check if entered value is valid, if not reset it to its default

	if ( !m_ui->lineEditX->isValid() ) {
		m_ui->lineEditX->setValue( m_originalValues.x() );
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
		m_ui->lineEditY->setValue( m_originalValues.y() );
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
		m_ui->lineEditZ->setValue( m_originalValues.z() );
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
}


void SVPropEditGeometry::updateInputs() {

	ModificationState state = m_modificationState[m_modificationType];

	switch (m_modificationType) {
	case MT_Translate : {
		showDeg(false);
		showRotation(false);
		switch (state) {

		case MS_Absolute:

			m_ui->labelX->setText("X");
			m_ui->labelY->setText("Y");
			m_ui->labelZ->setText("Z");

			// cache current local coordinate systems position as fall-back values
			m_originalValues = m_localCoordinatePosition.translation();

			m_ui->lineEditX->setValue(m_localCoordinatePosition.translation().x() );
			m_ui->lineEditY->setValue(m_localCoordinatePosition.translation().y() );
			m_ui->lineEditZ->setValue(m_localCoordinatePosition.translation().z() );
			break;

		default:

			m_ui->labelX->setText("ΔX");
			m_ui->labelY->setText("ΔY");
			m_ui->labelZ->setText("ΔZ");

			m_originalValues = QVector3D();

			m_ui->lineEditX->setValue(0);
			m_ui->lineEditY->setValue(0);
			m_ui->lineEditZ->setValue(0);

		}

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

			m_originalValues = QtExt::IBKVector2QVector(m_boundingBoxCenter);

			m_ui->lineEditX->setValue(m_boundingBoxDimension.m_x);
			m_ui->lineEditY->setValue(m_boundingBoxDimension.m_y);
			m_ui->lineEditZ->setValue(m_boundingBoxDimension.m_z);

			break;
		}
		default:
			m_ui->labelX->setText("X");
			m_ui->labelY->setText("Y");
			m_ui->labelZ->setText("Z");

			m_originalValues = QVector3D( 1,1,1 );

			m_ui->lineEditX->setValue(m_originalValues.x() );
			m_ui->lineEditY->setValue(m_originalValues.y() );
			m_ui->lineEditZ->setValue(m_originalValues.z() );

		}
	} break;
	}
}

void SVPropEditGeometry::setToolButton(const SVPropEditGeometry::ModificationType & type) {
	m_modificationType = type;

	m_ui->toolButtonTrans->setChecked(false);
	m_ui->toolButtonRotate->setChecked(false);
	m_ui->toolButtonScale->setChecked(false);

	switch (type) {
	case MT_Translate:
		m_ui->toolButtonTrans->setChecked(true);
		break;
	case MT_Rotate:
		m_ui->toolButtonRotate->setChecked(true);
		break;
	case MT_Scale:
		m_ui->toolButtonScale->setChecked(true);
		break;
	case NUM_MT:; // just to make compiler happy
	}
}

void SVPropEditGeometry::setComboBox(const ModificationType & type, const ModificationState & state) {
	m_ui->comboBox->blockSignals(true);
	m_ui->comboBox->clear();

	switch (type) {
	case MT_Translate:
		m_ui->comboBox->addItem( tr("move to position:") );
		m_ui->comboBox->addItem( tr("move relative using global coordinate system:") );
		m_ui->comboBox->addItem( tr("move relative using local coordinate system:") );
		break;
	case MT_Rotate:
		m_ui->comboBox->addItem( tr("rotate absolute:") );
		m_ui->comboBox->addItem( tr("rotate relative using global coordinate system:") );
		m_ui->comboBox->addItem( tr("rotate relative using local coordinate system:") );
		break;
	case MT_Scale:
		m_ui->comboBox->addItem( tr("resize surfaces:") );
		m_ui->comboBox->addItem( tr("scale relative to center of each surface:") );
		m_ui->comboBox->addItem( tr("scale relative to local coordinate system:") );
		break;
	case NUM_MT: ;// avoid compiler warning
	}

	m_ui->comboBox->setCurrentIndex((int)state);
	m_ui->comboBox->blockSignals(false);

	on_comboBox_activated((int)state);
}

void SVPropEditGeometry::showDeg(const bool & show)
{
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

void SVPropEditGeometry::showRotation(const bool & abs)
{
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
		m_ui->lineEditOrientation->setValue( m_originalValues.x() );
		return;
	}

	rotate();
}

void SVPropEditGeometry::on_lineEditInclination_returnPressed() {

	if ( !m_ui->lineEditInclination->isValid() ) {
		m_ui->lineEditInclination->setValue( m_originalValues.x() );
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


void SVPropEditGeometry::on_lineEditCopyX_returnPressed()
{
////	if ( m_ui->lineEditCopyX->isValid() )
////		m_xCopyValue = m_ui->lineEditCopyX->value();

//	int count = m_ui->spinBoxCount->value();

//	for ( size_t i = 0; i<count; ++i ) {
////		copy(QVector3D (i*m_xCopyValue,
////						i*m_yCopyValue,
////						i*m_zCopyValue) );
//	}

}

void SVPropEditGeometry::on_lineEditCopyY_returnPressed() {

}

void SVPropEditGeometry::on_lineEditCopyZ_returnPressed() {

}


void SVPropEditGeometry::on_lineEditCopyX_editingFinished() {
	on_lineEditX_returnPressed();
}

void SVPropEditGeometry::on_lineEditCopyY_editingFinished()
{
//	if ( m_ui->lineEditCopyY->isValid() )
//		m_yCopyValue = m_ui->lineEditCopyY->value();
}

void SVPropEditGeometry::on_lineEditCopyZ_editingFinished()
{
//	if ( m_ui->lineEditCopyZ->isValid() )
//		m_zCopyValue = m_ui->lineEditCopyZ->value();
}


void SVPropEditGeometry::onLineEditTextChanged(QtExt::ValidatingLineEdit * lineEdit) {
	// take transformation value, and if valid, modify transform in wire frame object

	if (!lineEdit->isValid())
		return;

	ModificationState state = m_modificationState[m_modificationType];
	ModificationType type = m_modificationType;

	// compose all transformation values
	switch (type) {
	case MT_Translate: {

		// compose translation vector depending on translation mode
		switch (state) {
		// Move local coordinate system to given global coordinates.
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
			QVector3D translation = localX*m_ui->lineEditX->value()
					+ localY*m_ui->lineEditY->value()
					+ localZ*m_ui->lineEditZ->value();
			// now compose a transform object and set it in the wireframe object
			Vic3D::Transform3D trans;
			trans.setTranslation(translation);
			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = trans;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
		} break;
		}
	} break;
	case MT_Scale: {
		switch (state) {
		case MS_Absolute: {
			// for this operation, we need at first the dimensions of the bounding box from
			QVector3D targetScale((float)m_ui->lineEditX->value(), (float)m_ui->lineEditY->value(), (float)m_ui->lineEditZ->value());
			// compute offset from current local coordinate system position
			QVector3D scale;

			scale.setX(targetScale.x() / ( m_boundingBoxDimension.m_x < 1E-4 ? 1.0 : m_boundingBoxDimension.m_x ) );
			scale.setY(targetScale.y() / ( m_boundingBoxDimension.m_y < 1E-4 ? 1.0 : m_boundingBoxDimension.m_y ) );
			scale.setZ(targetScale.z() / ( m_boundingBoxDimension.m_z < 1E-4 ? 1.0 : m_boundingBoxDimension.m_z ) );
			// now compose a transform object and set it in the wireframe object
			// first we scale our selected objects
			Vic3D::Transform3D scaling;
			scaling.setScale(scale);

			// and the we also hav to guarantee that the center point of the bounding box stays at the same position
			// this can be achieved since the center points are also just scaled by the specified scaling factors
			// so we know how big the absolute translation has to be
			QVector3D trans;
			trans.setX( m_boundingBoxDimension.m_x < 1E-4 ? 0.0 : m_boundingBoxCenter.m_x * ( 1 - scale.x() ) );
			trans.setY( m_boundingBoxDimension.m_y < 1E-4 ? 0.0 : m_boundingBoxCenter.m_y * ( 1 - scale.y() ) );
			trans.setZ( m_boundingBoxDimension.m_z < 1E-4 ? 0.0 : m_boundingBoxCenter.m_z * ( 1 - scale.z() ) );
			scaling.setTranslation(trans);

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

			QVector3D scale((float)m_ui->lineEditX->value()*( xAxis.x()+yAxis.x()+zAxis.x() ),
							(float)m_ui->lineEditY->value()*( xAxis.y()+yAxis.y()+zAxis.y() ),
							(float)m_ui->lineEditZ->value()*( xAxis.z()+yAxis.z()+zAxis.z() ));

			Vic3D::Transform3D scaling;
			scaling.setScale(scale);

			// and the we also hav to guarantee that the center point of the bounding box stays at the same position
			// this can be achieved since the center points are also just scaled by the specified scaling factors
			// so we know how big the absolute translation has to be
			QVector3D trans;
			trans.setX( m_boundingBoxDimension.m_x < 1E-4 ? 0.0 : m_boundingBoxCenter.m_x * ( 1 - scale.x() ) );
			trans.setY( m_boundingBoxDimension.m_y < 1E-4 ? 0.0 : m_boundingBoxCenter.m_y * ( 1 - scale.y() ) );
			trans.setZ( m_boundingBoxDimension.m_z < 1E-4 ? 0.0 : m_boundingBoxCenter.m_z * ( 1 - scale.z() ) );

			// we give our transformation to the wire frame object
			scaling.setTranslation(trans);
			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = scaling;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
		} break;
		case MS_Relative: {
			// for this operation, we get directly the surface scaling factors from the line edits
			// so it is basically like the absolute scaling, but we do not have to calculate the scaling factors
			QVector3D scale((float)m_ui->lineEditX->value(), (float)m_ui->lineEditY->value(), (float)m_ui->lineEditZ->value());
			// compute offset from current local coordinate system position
			Vic3D::Transform3D scaling;
			scaling.setScale(scale);

			// and the we also hav to guarantee that the center point of the bounding box stays at the same position
			// this can be achieved since the center points are also just scaled by the specified scaling factors
			// so we know how big the absolute translation has to be
			QVector3D trans;
			trans.setX( m_boundingBoxDimension.m_x < 1E-4 ? 0.0 : m_boundingBoxCenter.m_x * ( 1 - scale.x() ) );
			trans.setY( m_boundingBoxDimension.m_y < 1E-4 ? 0.0 : m_boundingBoxCenter.m_y * ( 1 - scale.y() ) );
			trans.setZ( m_boundingBoxDimension.m_z < 1E-4 ? 0.0 : m_boundingBoxCenter.m_z * ( 1 - scale.z() ) );

			// we give our transformation to the wire frame object
			scaling.setTranslation(trans);
			SVViewStateHandler::instance().m_selectedGeometryObject->m_transform = scaling;
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderNow();
			const_cast<Vic3D::SceneView*>(SVViewStateHandler::instance().m_geometryView->sceneView())->renderLater();
		} break;
		}
	} break;
	case MT_Rotate: {
		switch (state) {
		case MS_Absolute: {

			double oriRad = m_ui->lineEditOrientation->value() * IBK::DEG2RAD;
			double incliRad = m_ui->lineEditInclination->value() * IBK::DEG2RAD;

			QVector3D newNormal (std::sin( oriRad ) * std::sin( incliRad ),
										 std::cos( oriRad ) * std::sin( incliRad ),
										 std::cos( incliRad ) );

			// we only want to rotate if the normal vectors are not the same
			if ( checkVectors<4>( m_normal, QtExt::QVector2IBKVector(newNormal) ) )
				return; // do nothing

			// we find the rotation axis by taking the cross product of the normal vector and the normal vector we want to
			// rotate to
			QVector3D rotationAxis ( QtExt::IBKVector2QVector(m_normal.crossProduct(QtExt::QVector2IBKVector(newNormal) ) ) ) ;

			Vic3D::Transform3D rota;
			// we now also have to find the angle between both normals
			rota.rotate( angleBetweenVectorsDeg( m_normal, QtExt::QVector2IBKVector(newNormal) ), rotationAxis );

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

void SVPropEditGeometry::on_lineEditOrientation_textChanged(const QString &){
	 onLineEditTextChanged(m_ui->lineEditOrientation);
}

void SVPropEditGeometry::on_lineEditInclination_textChanged(const QString &)
{
	onLineEditTextChanged(m_ui->lineEditInclination);
}
