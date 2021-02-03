#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

#include "IBK_physics.h"

#include "VICUS_Project.h"
#include "VICUS_Conversions.h"

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVUndoModifySurfaceGeometry.h"
#include "SVPropVertexListWidget.h"
#include "SVGeometryView.h"

#include "Vic3DNewGeometryObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DTransform3D.h"

#include "QLocale"

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

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);
	layout()->setMargin(0);
	SVViewStateHandler::instance().m_propEditGeometryWidget = this;

	on_radioButtonRotateAbsolute_toggled(false);
	m_ui->doubleSpinBoxRotateInclinationAbs->setSuffix(" °");
	m_ui->doubleSpinBoxRotateOrientationAbs->setSuffix(" °");

	m_ui->doubleSpinBoxRotateX->setSuffix(" °");
	m_ui->doubleSpinBoxRotateY->setSuffix(" °");
	m_ui->doubleSpinBoxRotateZ->setSuffix(" °");

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropEditGeometry::onModified);
}


SVPropEditGeometry::~SVPropEditGeometry() {
	delete m_ui;
}


void SVPropEditGeometry::setCurrentTab(const SVPropEditGeometry::TabState & state) {
	m_ui->toolBoxGeometry->setCurrentIndex(state);
	if (state != TS_EditGeometry) {
		// clear edit page
		m_ui->lineEditXValue->clear();
		m_ui->lineEditYValue->clear();
		m_ui->lineEditZValue->clear();
		m_ui->lineEditXValue->setEnabled(false);
		m_ui->lineEditYValue->setEnabled(false);
		m_ui->lineEditZValue->setEnabled(false);
	}
	else {
		m_ui->lineEditXValue->setEnabled(true);
		m_ui->lineEditYValue->setEnabled(true);
		m_ui->lineEditZValue->setEnabled(true);
	}
}



void SVPropEditGeometry::setCoordinates(const Vic3D::Transform3D &t) {
	// is being call from local coordinate system object, whenever this has changed location (regardless of
	// its own visibility)
	m_ui->lineEditXValue->setText( QString("%L1").arg( t.translation().x() ) );
	m_ui->lineEditYValue->setText( QString("%L1").arg( t.translation().y() ) );
	m_ui->lineEditZValue->setText( QString("%L1").arg( t.translation().z() ) );

	if ( m_ui->radioButtonAbsolute->isChecked() ) {
		QLocale loc;

		m_ui->doubleSpinBoxTranslateX->setValue( loc.toDouble(m_ui->lineEditXValue->text() ) );
		m_ui->doubleSpinBoxTranslateY->setValue( loc.toDouble(m_ui->lineEditYValue->text() ) );
		m_ui->doubleSpinBoxTranslateZ->setValue( loc.toDouble(m_ui->lineEditZValue->text() ) );
	}

}


void SVPropEditGeometry::setBoundingBox(const IBKMK::Vector3D &v) {
//	QVector3D tmpScale ( m_ui->doubleSpinBoxScaleX->value(), m_ui->doubleSpinBoxScaleX->value() )

	if ( m_ui->radioButtonScaleAbsolute->isChecked() ) {
		m_ui->doubleSpinBoxScaleX->setValue( v.m_x );
		m_ui->doubleSpinBoxScaleY->setValue( v.m_y );
		m_ui->doubleSpinBoxScaleZ->setValue( v.m_z );
	}

}

void SVPropEditGeometry::setRotation(const IBKMK::Vector3D &normal) {
	//	QVector3D tmpScale ( m_ui->doubleSpinBoxScaleX->value(), m_ui->doubleSpinBoxScaleX->value() )
	normal.normalized();
	if ( m_ui->radioButtonRotateAbsolute->isChecked() ) {
		m_ui->doubleSpinBoxRotateInclinationAbs->setValue( std::acos(normal.m_z)/IBK::DEG2RAD );

		// positive y Richtung = Norden = Orientation 0°
		// positive x Richtung = Osten = Orientation 90°

		double orientation = std::atan2(normal.m_x, ( normal.m_y == 0 ? 1E-8 : normal.m_y ) ) /IBK::DEG2RAD ;
		m_ui->doubleSpinBoxRotateOrientationAbs->setValue( orientation < 0 ? ( orientation + 360 ) : orientation );
	}

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


void SVPropEditGeometry::on_pushButtonTranslate_clicked() {
	// now we update all selected surfaces
	Vic3D::Transform3D trans;
	QVector3D transVec ( m_ui->doubleSpinBoxTranslateX->value(),
						 m_ui->doubleSpinBoxTranslateY->value(),
						 m_ui->doubleSpinBoxTranslateZ->value() );


	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;

	IBKMK::Vector3D centerPoint;
	std::vector<const VICUS::Surface*> surfaces;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);

	if ( m_ui->radioButtonRelative->isChecked() ) {
		for (const VICUS::Surface* s : surfaces ) {
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
	}
	else if ( m_ui->radioButtonLocal->isChecked() ) {
		for (const VICUS::Surface* s : surfaces ) {
			std::vector<IBKMK::Vector3D> vs;
			QVector3D xAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis();
			QVector3D yAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localYAxis();
			QVector3D zAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis();
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				// use just this instead of making a QVetor3D
				t.setTranslation( v.m_x, v.m_y, v.m_z );
				t.translate( xAxis *transVec.x() );
				t.translate( yAxis *transVec.y() );
				t.translate( zAxis *transVec.z() );
				vs.push_back( IBKMK::Vector3D ( t.translation().x(), t.translation().y(), t.translation().z() ) );
			}
			VICUS::Surface newS(*s);
			newS.m_geometry.setVertexes(vs);
			modifiedSurfaces.push_back(newS);
		}
	}
	else {
		project().boundingBox(surfaces, centerPoint);
		for (const VICUS::Surface* s : surfaces ) {
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				t.setTranslation( transVec.x() + ( v.m_x - centerPoint.m_x ),
								  transVec.y() + ( v.m_y - centerPoint.m_y),
								  transVec.z() + ( v.m_z - centerPoint.m_z) );
				vs.push_back( IBKMK::Vector3D ( t.translation().x(), t.translation().y(), t.translation().z() ) );
			}
			VICUS::Surface newS(*s);
			newS.m_geometry.setVertexes(vs);
			modifiedSurfaces.push_back(newS);
		}
	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), modifiedSurfaces );
	undo->push();
}


void SVPropEditGeometry::on_pushButtonScale_clicked() {

	// TODO : Stephan, fix this like on_pushButtonTranslate_clicked()
	// now we update all selected surfaces
	Vic3D::Transform3D trans;
	QVector3D scaleVec (m_ui->doubleSpinBoxScaleX->value(),
						m_ui->doubleSpinBoxScaleY->value(),
						m_ui->doubleSpinBoxScaleZ->value() );

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;
	std::vector<const VICUS::Surface*> surfaces;

	IBKMK::Vector3D centerPoint;
	IBKMK::Vector3D centerPointLocal;
	IBKMK::Vector3D boundingBox;

	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
	project().boundingBox(surfaces, centerPointLocal);

	// check if scale factor is not Null
	if ( IBK::nearly_equal<3>( scaleVec.length(), 0.0 ) )
		return;

	if ( m_ui->radioButtonScaleRelative->isChecked() ) {
		for (const VICUS::Surface* s : surfaces ) {
			centerPoint = s->m_geometry.centerPoint();
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				t.setTranslation( centerPoint.m_x + scaleVec.x() * ( v.m_x - centerPoint.m_x ),
								  centerPoint.m_y + scaleVec.y() * ( v.m_y - centerPoint.m_y ),
								  centerPoint.m_z + scaleVec.z() * ( v.m_z - centerPoint.m_z ) );
				vs.push_back( VICUS::QVector2IBKVector( t.translation() ) );
			}
			VICUS::Surface newS(*s);
			newS.m_geometry.setVertexes(vs);
			modifiedSurfaces.push_back(newS);
		}
	}
	else if ( m_ui->radioButtonScaleLocal->isChecked() ) {

		QVector3D xAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis();
		QVector3D yAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localYAxis();
		QVector3D zAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis();

		for (const VICUS::Surface* s : surfaces ) {
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;

				// first we find the scaling factors of our local cooridnate system
				double localScaleFactorX = ( v.m_x - centerPointLocal.m_x ) / ( IBK::nearly_equal<4>(xAxis.x(), 0.0) ? 1E10 : xAxis.x() ) +
										   ( v.m_y - centerPointLocal.m_y ) / ( IBK::nearly_equal<4>(xAxis.y(), 0.0) ? 1E10 : xAxis.y() ) +
										   ( v.m_z - centerPointLocal.m_z ) / ( IBK::nearly_equal<4>(xAxis.z(), 0.0) ? 1E10 : xAxis.z() );

				double localScaleFactorY = ( v.m_x - centerPointLocal.m_x ) / ( IBK::nearly_equal<4>(yAxis.x(), 0.0) ? 1E10 : yAxis.x() ) +
										   ( v.m_y - centerPointLocal.m_y ) / ( IBK::nearly_equal<4>(yAxis.y(), 0.0) ? 1E10 : yAxis.y() ) +
										   ( v.m_z - centerPointLocal.m_z ) / ( IBK::nearly_equal<4>(yAxis.z(), 0.0) ? 1E10 : yAxis.z() );

				double localScaleFactorZ = ( v.m_x - centerPointLocal.m_x ) / ( IBK::nearly_equal<4>(zAxis.x(), 0.0) ? 1E10 : zAxis.x() ) +
										   ( v.m_y - centerPointLocal.m_y ) / ( IBK::nearly_equal<4>(zAxis.y(), 0.0) ? 1E10 : zAxis.y() ) +
										   ( v.m_z - centerPointLocal.m_z ) / ( IBK::nearly_equal<4>(zAxis.z(), 0.0) ? 1E10 : zAxis.z() );

				// then we scale our points
				QVector3D p = VICUS::IBKVector2QVector(centerPointLocal)+ localScaleFactorX * scaleVec.x() * xAxis
																		+ localScaleFactorY * scaleVec.y() * yAxis
																		+ localScaleFactorZ * scaleVec.z() * zAxis;
				t.setTranslation(p);
				vs.push_back(VICUS::QVector2IBKVector(t.translation() ) );
			}
			VICUS::Surface newS(*s);
			newS.m_geometry.setVertexes(vs);
			modifiedSurfaces.push_back(newS);
		}
	}
	else {

		for (const VICUS::Surface* s : surfaces ) {
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				IBKMK::Vector3D newScale;

				newScale.m_x = ( boundingBox.m_x == 0.0 ) ?  0 : ( scaleVec.x() / boundingBox.m_x );
				newScale.m_y = ( boundingBox.m_y == 0.0 ) ?  0 : ( scaleVec.y() / boundingBox.m_y );
				newScale.m_z = ( boundingBox.m_z == 0.0 ) ?  0 : ( scaleVec.z() / boundingBox.m_z );

				t.setTranslation( centerPointLocal.m_x + newScale.m_x * ( v.m_x - centerPointLocal.m_x ),
								  centerPointLocal.m_y + newScale.m_y * ( v.m_y - centerPointLocal.m_y),
								  centerPointLocal.m_z + newScale.m_z * ( v.m_z - centerPointLocal.m_z) );
				vs.push_back( VICUS::QVector2IBKVector( t.translation() ) );
			}
			VICUS::Surface newS(*s);
			newS.m_geometry.setVertexes(vs);
			modifiedSurfaces.push_back(newS);
		}

	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), modifiedSurfaces );
	undo->push();

}


void SVPropEditGeometry::on_pushButtonRotate_clicked() {
	// TODO : Stephan, fix this like on_pushButtonTranslate_clicked()

	// now we update all selected surfaces
	VICUS::Project p = project();
	Vic3D::Transform3D trans;
	QVector3D rotateVecDeg ( m_ui->doubleSpinBoxRotateX->value(),
						  m_ui->doubleSpinBoxRotateY->value(),
						  m_ui->doubleSpinBoxRotateZ->value() );

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;
	std::vector<const VICUS::Surface*> surfaces;

	IBKMK::Vector3D centerPoint (0,0,0);
	IBKMK::Vector3D centerPointLocal;

	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
	project().boundingBox(surfaces, centerPointLocal);

	for (const VICUS::Surface* s : surfaces ) {
		std::vector<IBKMK::Vector3D> vs;

		IBKMK::Vector3D normal = ( surfaces.size() == 1 ?
									s->m_geometry.normal() :
									VICUS::QVector2IBKVector(SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis() ) );

		for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
			Vic3D::Transform3D tTrans, tRota;

			// translate point back to coordinate center
			QVector3D v3D ( VICUS::IBKVector2QVector(v) );

			if ( m_ui->radioButtonRotateAbsolute->isChecked() ) {

				centerPoint = centerPointLocal;

				double oriRad = m_ui->doubleSpinBoxRotateOrientationAbs->value() * IBK::DEG2RAD;
				double incliRad = m_ui->doubleSpinBoxRotateInclinationAbs->value() * IBK::DEG2RAD;

				IBKMK::Vector3D newNormal (std::sin( oriRad ) * std::sin( incliRad ),
										   std::cos( oriRad ) * std::sin( incliRad ),
										   std::cos( incliRad ) );

				if ( checkVectors<4>( normal, newNormal ) )
					return; // do nothing

				QVector3D rotationAxis ( VICUS::IBKVector2QVector(normal.crossProduct(newNormal) ) ) ;

				tTrans.setTranslation( VICUS::IBKVector2QVector(-1*centerPoint) );
				v3D = tTrans.toMatrix() * v3D;

				tRota.rotate( angleBetweenVectorsDeg( normal, newNormal ), rotationAxis );
				v3D = tRota.toMatrix() * v3D;

			} else if ( m_ui->radioButtonRotateLocal->isChecked() ) {

				centerPoint = centerPointLocal;

				tTrans.setTranslation( VICUS::IBKVector2QVector(-1*centerPoint) );
				v3D = tTrans.toMatrix() * v3D;

				QVector3D xAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis();
				QVector3D yAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localYAxis();
				QVector3D zAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis();

				// rotate around specified axis
				if ( !IBK::nearly_equal<3>( rotateVecDeg.x(), 0.0 ) )
					tRota.rotate( rotateVecDeg.x(), xAxis );
				if ( !IBK::nearly_equal<3>( rotateVecDeg.y(), 0.0 ) )
					tRota.rotate( rotateVecDeg.y(), yAxis );
				if ( !IBK::nearly_equal<3>( rotateVecDeg.z(), 0.0 ) )
					tRota.rotate( rotateVecDeg.z(), zAxis );
				v3D = tRota.toMatrix() * v3D;
			}

			else {
				centerPoint = s->m_geometry.centerPoint();

				tTrans.setTranslation( VICUS::IBKVector2QVector(-1*centerPoint) );
				v3D = tTrans.toMatrix() * v3D;

				// rotate around specified axis
				if ( !IBK::nearly_equal<3>( rotateVecDeg.x(), 0.0 ) )
					tRota.rotate( rotateVecDeg.x(), VICUS::IBKVector2QVector(s->m_geometry.localX() ) );
				if ( !IBK::nearly_equal<3>( rotateVecDeg.y(), 0.0 ) )
					tRota.rotate( rotateVecDeg.y(), VICUS::IBKVector2QVector(s->m_geometry.localY() ) );
				if ( !IBK::nearly_equal<3>( rotateVecDeg.z(), 0.0 ) )
					tRota.rotate( rotateVecDeg.z(), normal.m_x, normal.m_y, normal.m_z );
				v3D = tRota.toMatrix() * v3D;
			}
			// translatae back to original center point
			tTrans.setTranslation( centerPoint.m_x, centerPoint.m_y, centerPoint.m_z );
			v3D = tTrans.toMatrix() * v3D;
			vs.push_back( IBKMK::Vector3D( v3D.x(), v3D.y(), v3D.z() ) );
		}
		VICUS::Surface newS(*s);
		newS.m_geometry.setVertexes(vs);
		modifiedSurfaces.push_back(newS);

	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), modifiedSurfaces );
	undo->push();
}


void SVPropEditGeometry::on_radioButtonScaleAbsolute_toggled(bool absScale) {

	std::vector<const VICUS::Surface*> surfaces;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
	IBKMK::Vector3D center;
	setBoundingBox(project().boundingBox(surfaces, center ) );
}



void SVPropEditGeometry::on_radioButtonRotateAbsolute_toggled(bool absRotate)
{
	if ( absRotate ) {

		m_ui->horizontalLayoutAbsRotate->setEnabled(true);

		std::vector<const VICUS::Surface*> surfaces;

		IBKMK::Vector3D centerPoint (0,0,0);
		IBKMK::Vector3D centerPointLocal;

		project().selectedSurfaces(surfaces, VICUS::Project::SG_All);

		m_ui->labelRotateInclinationAbs->setEnabled(true);
		m_ui->labelRotateOrientationAbs->setEnabled(true);
		m_ui->doubleSpinBoxRotateInclinationAbs->setEnabled(true);
		m_ui->doubleSpinBoxRotateOrientationAbs->setEnabled(true);

		m_ui->labelRotateX->setEnabled(false);
		m_ui->labelRotateY->setEnabled(false);
		m_ui->labelRotateZ->setEnabled(false);
		m_ui->doubleSpinBoxRotateX->setEnabled(false);
		m_ui->doubleSpinBoxRotateY->setEnabled(false);
		m_ui->doubleSpinBoxRotateZ->setEnabled(false);


		if ( surfaces.size() == 1 ) {
			const VICUS::Surface* s = surfaces[0];
			m_ui->doubleSpinBoxRotateX->setValue( s->m_geometry.normal().m_x );
			m_ui->doubleSpinBoxRotateY->setValue( s->m_geometry.normal().m_y );
			m_ui->doubleSpinBoxRotateZ->setValue( s->m_geometry.normal().m_z );
			setRotation(s->m_geometry.normal());
		}
		else
			setRotation(VICUS::QVector2IBKVector(SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis() ) );
	}
	else {
		m_ui->labelRotateInclinationAbs->setEnabled(false);
		m_ui->labelRotateOrientationAbs->setEnabled(false);
		m_ui->doubleSpinBoxRotateInclinationAbs->setEnabled(false);
		m_ui->doubleSpinBoxRotateOrientationAbs->setEnabled(false);

		m_ui->labelRotateX->setEnabled(true);
		m_ui->labelRotateY->setEnabled(true);
		m_ui->labelRotateZ->setEnabled(true);
		m_ui->doubleSpinBoxRotateX->setEnabled(true);
		m_ui->doubleSpinBoxRotateY->setEnabled(true);
		m_ui->doubleSpinBoxRotateZ->setEnabled(true);
	}


}

void SVPropEditGeometry::on_radioButtonAbsolute_toggled(bool absTrans)
{
	if ( absTrans ) {
		m_ui->labelTranslationX->setText("X");
		m_ui->labelTranslationY->setText("Y");
		m_ui->labelTranslationZ->setText("Z");

		QLocale loc;

		m_ui->doubleSpinBoxTranslateX->setValue( loc.toDouble(m_ui->lineEditXValue->text() ) );
		m_ui->doubleSpinBoxTranslateY->setValue( loc.toDouble(m_ui->lineEditYValue->text() ) );
		m_ui->doubleSpinBoxTranslateZ->setValue( loc.toDouble(m_ui->lineEditZValue->text() ) );
	}
	else {
		m_ui->labelTranslationX->setText("ΔX");
		m_ui->labelTranslationY->setText("ΔY");
		m_ui->labelTranslationZ->setText("ΔZ");
	}
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
			SVViewStateHandler::instance().m_propEditGeometryWidget->setRotation(s->m_geometry.normal() );
		}
		else
			SVViewStateHandler::instance().m_propEditGeometryWidget->setRotation(
						VICUS::QVector2IBKVector(SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis() ) );
		IBKMK::Vector3D center;

		// update local coordinates
		setBoundingBox(project().boundingBox(surfaces, center));
		Vic3D::Transform3D t;
		t.setTranslation(VICUS::IBKVector2QVector(center) );
		setCoordinates( t );

		SVViewStateHandler::instance().m_coordinateSystemObject->setTranslation(VICUS::IBKVector2QVector(center) );
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
