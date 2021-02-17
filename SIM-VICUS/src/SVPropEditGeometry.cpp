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
#include "Vic3DTransform3D.h"

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

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);

	SVViewStateHandler::instance().m_propEditGeometryWidget = this;


	//	m_ui->doubleSpinBoxRotateX->setSuffix(" °");
	//	m_ui->doubleSpinBoxRotateY->setSuffix(" °");
	//	m_ui->doubleSpinBoxRotateZ->setSuffix(" °");

	connect(&SVProjectHandler::instance(), &SVProjectHandler::modified,
			this, &SVPropEditGeometry::onModified);

	m_ui->lineEditX->setup(-1E5,1E5,tr("X Value"),true, true);
	m_ui->lineEditY->setup(-1E5,1E5,tr("Y Value"),true, true);
	m_ui->lineEditZ->setup(-1E5,1E5,tr("Z Value"),true, true);

	// not needed anymore
	//	connect(m_ui->lineEditX, SIGNAL(editingFinishedSuccessfully()), this, SLOT(on_lineEditX_editingFinished() ) );
	//	connect(m_ui->lineEditY, SIGNAL(editingFinishedSuccessfully()), this, SLOT(on_lineEditY_editingFinished() ) );
	//	connect(m_ui->lineEditZ, SIGNAL(editingFinishedSuccessfully()), this, SLOT(on_lineEditZ_editingFinished() ) );

	m_ui->lineEditX->setText( QString("%L1").arg( 0.0, 0, 'f', 3) );
	m_ui->lineEditY->setText( QString("%L1").arg( 0.0, 0, 'f', 3) );
	m_ui->lineEditZ->setText( QString("%L1").arg( 0.0, 0, 'f', 3) );

	m_ui->lineEditCopyX->setText( QString("%L1").arg( 0.0, 0, 'f', 3) );
	m_ui->lineEditCopyY->setText( QString("%L1").arg( 0.0, 0, 'f', 3) );
	m_ui->lineEditCopyZ->setText( QString("%L1").arg( 0.0, 0, 'f', 3) );

	m_ui->lineEditX->installEventFilter(this);
	m_ui->lineEditY->installEventFilter(this);
	m_ui->lineEditZ->installEventFilter(this);

	m_modificationState[MT_Translate] = MS_Absolute;
	m_modificationState[MT_Rotate] = MS_Absolute;
	m_modificationState[MT_Scale] = MS_Absolute;

	// set initial states
	setState(MT_Translate, MS_Absolute, true);
	showRotation(false);
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

	if ( m_modificationType == MT_Translate && m_modificationState[MT_Translate] == MS_Absolute ) {
		m_xTransValue = m_localCoordinatePosition.translation().x();
		m_yTransValue = m_localCoordinatePosition.translation().y();
		m_zTransValue = m_localCoordinatePosition.translation().z();

		m_ui->lineEditX->setText( QString("%L1").arg( m_localCoordinatePosition.translation().x(),0, 'f', 3 ) );
		m_ui->lineEditY->setText( QString("%L1").arg( m_localCoordinatePosition.translation().y(),0, 'f', 3 ) );
		m_ui->lineEditZ->setText( QString("%L1").arg( m_localCoordinatePosition.translation().z(),0, 'f', 3 ) );
	}

}


void SVPropEditGeometry::setBoundingBox(const IBKMK::Vector3D &v) {

	if (m_modificationType == MT_Scale && m_modificationState[m_modificationType] == MS_Absolute) {
		m_xScaleValue = v.m_x;
		m_yScaleValue = v.m_y;
		m_zScaleValue = v.m_z;

		m_ui->lineEditX->setText( QString("%L1").arg( v.m_x, 0, 'f', 3) );
		m_ui->lineEditY->setText( QString("%L1").arg( v.m_y, 0, 'f', 3) );
		m_ui->lineEditZ->setText( QString("%L1").arg( v.m_z, 0, 'f', 3) );
	}
}

void SVPropEditGeometry::setRotation(const IBKMK::Vector3D &normal) {

	normal.normalized();
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
			update(false);
			break;
		case SVProjectHandler::NodeStateModified:
			// when the building geometry has changed, we need to update the geometrical info
			// in the widget based on the current selection
			// also, we assume any change in node states (visibility/selection) may impact our local
			// coordinate system position
			update(true); // this might update the location of the local coordinate system!
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




void SVPropEditGeometry::on_lineEditX_editingFinished()
{
	if ( m_ui->lineEditX->isValid()){

		double tempXValue = m_ui->lineEditX->value();
		m_ui->lineEditX->setText( QString("%L1").arg(tempXValue,0, 'f', 3 ) );

		switch ( m_modificationType ) {
			case ( MT_Translate ):	m_xTransValue = tempXValue; break;
			case ( MT_Rotate ):		m_xRotaValue = tempXValue; break;
			case ( MT_Scale ):		m_xScaleValue = tempXValue; break;
		}
	}
	else {
		switch ( m_modificationType ) {
			case ( MT_Translate ):	m_ui->lineEditX->setText( QString("%L1").arg(m_xTransValue,0, 'f', 3 ) ); break;
			case ( MT_Rotate ):		m_ui->lineEditX->setText( QString("%L1").arg(m_xRotaValue,0, 'f', 3 ) ); break;
			case ( MT_Scale ):		m_ui->lineEditX->setText( QString("%L1").arg(m_xScaleValue,0, 'f', 3 ) ); break;
		}
	}
}

void SVPropEditGeometry::on_lineEditY_editingFinished()
{
	if ( m_ui->lineEditY->isValid()){
		double tempYValue = m_ui->lineEditY->value();
		m_ui->lineEditY->setText( QString("%L1").arg(tempYValue,0, 'f', 3 ) );
		switch ( m_modificationType ) {
			case ( MT_Translate ):	m_yTransValue = tempYValue; break;
			case ( MT_Rotate ):		m_yRotaValue = tempYValue; break;
			case ( MT_Scale ):		m_yScaleValue = tempYValue; break;
		}
	}
	else {
		switch ( m_modificationType ) {
			case ( MT_Translate ):	m_ui->lineEditY->setText( QString("%L1").arg(m_yTransValue,0, 'f', 3 ) ); break;
			case ( MT_Rotate ):		m_ui->lineEditY->setText( QString("%L1").arg(m_yRotaValue,0, 'f', 3 ) ); break;
			case ( MT_Scale ):		m_ui->lineEditY->setText( QString("%L1").arg(m_yScaleValue,0, 'f', 3 ) ); break;
		}
	}
}

void SVPropEditGeometry::on_lineEditZ_editingFinished()
{
	if ( m_ui->lineEditZ->isValid()){
		double tempZValue = m_ui->lineEditZ->value();
		m_ui->lineEditZ->setText( QString("%L1").arg(tempZValue,0, 'f', 3 ) );

		switch ( m_modificationType ) {
			case ( MT_Translate ):	m_zTransValue = tempZValue; break;
			case ( MT_Rotate ):		m_zRotaValue = tempZValue; break;
			case ( MT_Scale ):		m_zScaleValue = tempZValue; break;
		}
	}
	else {
		switch ( m_modificationType ) {
			case ( MT_Translate ):	m_ui->lineEditY->setText( QString("%L1").arg(m_yTransValue,0, 'f', 3 ) ); break;
			case ( MT_Rotate ):		m_ui->lineEditY->setText( QString("%L1").arg(m_yRotaValue,0, 'f', 3 ) ); break;
			case ( MT_Scale ):		m_ui->lineEditY->setText( QString("%L1").arg(m_yScaleValue,0, 'f', 3 ) ); break;
		}
	}
}


void SVPropEditGeometry::translate(const QVector3D & transVec, const ModificationState &state) {
	// now we update all selected surfaces
	Vic3D::Transform3D trans;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;

	IBKMK::Vector3D centerPoint;
	std::vector<const VICUS::Surface*> surfaces;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);

	if ( state == MS_Relative ) {
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
	}
	else if ( state == MS_Local ) {
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


void SVPropEditGeometry::scale(const QVector3D & scaleVec, const ModificationState &state, const bool &wheel) {

	// now we update all selected surfaces
	Vic3D::Transform3D trans;

	// compose vector of modified surface geometries
	std::vector<VICUS::Surface> modifiedSurfaces;
	std::vector<const VICUS::Surface*> surfaces;

	IBKMK::Vector3D centerPoint;
	IBKMK::Vector3D centerPointLocal;

	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
	IBKMK::Vector3D boundingBox = project().boundingBox(surfaces, centerPointLocal);

	if ( wheel ){
		for ( VICUS::Surface s : m_relScaleSurfaces) {
			for (const VICUS::Surface *surf : surfaces) {
				if ( s.m_id == surf->m_id ) {
					VICUS::Surface* surfNonConst = const_cast<VICUS::Surface*>(surf);
					surfNonConst->m_geometry.setVertexes(s.m_geometry.vertexes());
				}
			}
		}
	}


	// check if scale factor is not Null
	if ( IBK::nearly_equal<3>( scaleVec.length(), 0.0 ) )
		return;

	if ( state == MS_Relative ) {
		for (const VICUS::Surface* s : surfaces ) {
			centerPoint = s->m_geometry.centerPoint();
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				t.setTranslation( centerPoint.m_x + scaleVec.x() * ( v.m_x - centerPoint.m_x ),
								  centerPoint.m_y + scaleVec.y() * ( v.m_y - centerPoint.m_y ),
								  centerPoint.m_z + scaleVec.z() * ( v.m_z - centerPoint.m_z ) );
				vs.push_back( QtExt::QVector2IBKVector( t.translation() ) );
			}
			VICUS::Surface newS(*s);
			newS.m_geometry.setVertexes(vs);
			modifiedSurfaces.push_back(newS);
		}
	}
	else if ( state == MS_Local ) {

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
				QVector3D p = QtExt::IBKVector2QVector(centerPointLocal)+ localScaleFactorX * scaleVec.x() * xAxis
						+ localScaleFactorY * scaleVec.y() * yAxis
						+ localScaleFactorZ * scaleVec.z() * zAxis;
				t.setTranslation(p);
				vs.push_back(QtExt::QVector2IBKVector(t.translation() ) );
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
				vs.push_back( QtExt::QVector2IBKVector( t.translation() ) );
			}
			VICUS::Surface newS(*s);
			newS.m_geometry.setVertexes(vs);
			modifiedSurfaces.push_back(newS);
		}

	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), modifiedSurfaces );
	undo->push();

}


void SVPropEditGeometry::rotate(const QVector3D & rotateVecDeg, const ModificationState &state) {

	// now we update all selected surfaces
	VICUS::Project p = project();
	Vic3D::Transform3D trans;

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
									   QtExt::QVector2IBKVector(SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis() ) );

		for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
			Vic3D::Transform3D tTrans, tRota;

			// translate point back to coordinate center
			QVector3D v3D ( QtExt::IBKVector2QVector(v) );

			if ( state == MS_Absolute ) {

				centerPoint = centerPointLocal;

				if ( checkVectors<4>( normal, QtExt::QVector2IBKVector(rotateVecDeg) ) )
					return; // do nothing

				QVector3D rotationAxis ( QtExt::IBKVector2QVector(normal.crossProduct(QtExt::QVector2IBKVector(rotateVecDeg) ) ) ) ;

				tTrans.setTranslation( QtExt::IBKVector2QVector(-1*centerPoint) );
				v3D = tTrans.toMatrix() * v3D;

				tRota.rotate( angleBetweenVectorsDeg( normal, QtExt::QVector2IBKVector(rotateVecDeg) ), rotationAxis );
				v3D = tRota.toMatrix() * v3D;

			} else if ( state == MS_Local ) {

				centerPoint = centerPointLocal;

				tTrans.setTranslation( QtExt::IBKVector2QVector(-1*centerPoint) );
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

				tTrans.setTranslation( QtExt::IBKVector2QVector(-1*centerPoint) );
				v3D = tTrans.toMatrix() * v3D;

				Vic3D::CoordinateSystemObject coordinateSystem;

				// rotate around specified axis
				if ( !IBK::nearly_equal<3>( rotateVecDeg.x(), 0.0 ) )
					tRota.rotate( rotateVecDeg.x(), 1, 0, 0 );
				if ( !IBK::nearly_equal<3>( rotateVecDeg.y(), 0.0 ) )
					tRota.rotate( rotateVecDeg.y(), 0, 1, 0 );
				if ( !IBK::nearly_equal<3>( rotateVecDeg.z(), 0.0 ) )
					tRota.rotate( rotateVecDeg.z(), 0, 0, 1 );
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

void SVPropEditGeometry::copy(const QVector3D &transVec)
{
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
}


// *** private functions ***

void SVPropEditGeometry::update(const bool &updateScalingSurfaces) {
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
			m_ui->labelIndication->setText("Normal:");
			SVViewStateHandler::instance().m_propEditGeometryWidget->setRotation(s->m_geometry.normal() );
		}
		else {
			m_ui->labelIndication->setText(tr("z-Axis:"));
			SVViewStateHandler::instance().m_propEditGeometryWidget->setRotation(
						QtExt::QVector2IBKVector(SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis() ) );
		}
		IBKMK::Vector3D center, boundingBox;
		boundingBox = project().boundingBox(surfaces, center);

		// update for relative scaling
		if ( updateScalingSurfaces ) {
			setRelativeScalingSurfaces();
		}


		// update local coordinates
		Vic3D::Transform3D t;
		t.setTranslation(QtExt::IBKVector2QVector(center) );
		setBoundingBox(project().boundingBox(surfaces, boundingBox) );
		setCoordinates( t );

		// update scaling factor
		SVViewStateHandler::instance().m_coordinateSystemObject->setTranslation(QtExt::IBKVector2QVector(center) );
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


void SVPropEditGeometry::on_comboBox_activated(int newIndex)
{
	// set new state
	m_modificationState[m_modificationType] = (ModificationState)newIndex;

	setState(m_modificationType, m_modificationState[m_modificationType]);
}


bool SVPropEditGeometry::eventFilter(QObject * target, QEvent * event)
{	const ModificationState &state = m_modificationState[m_modificationType];

	if ( event->type() == QEvent::Wheel ) {
		QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
		if (target == m_ui->lineEditX){
			if (m_ui->lineEditX->isValid()) {
				int sign = -1;
				if ( wheelEvent->delta()>0 )
					sign = 1;

				switch (m_modificationType) {
					case MT_Translate:
						m_xTransValue = std::floor( (m_xTransValue + sign*0.01+0.005)*100)/100;
						m_ui->lineEditX->setText( QString("%L1").arg(m_xTransValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								translate( QVector3D ( m_xTransValue, m_yTransValue, m_zTransValue ),
										   m_modificationState[MT_Translate] );
								break;

							case MS_Relative:
								translate( QVector3D ( sign*0.01, 0, 0 ),
										   m_modificationState[MT_Translate] );
								break;
							case MS_Local:
								translate( QVector3D ( sign*0.01, 0, 0 ),
										   m_modificationState[MT_Translate] );
								break;
						}
						break;
					case MT_Scale:

						m_xScaleValue = std::floor( (m_xScaleValue + sign*0.1+0.05)*10)/10;
						m_ui->lineEditX->setText( QString("%L1").arg(m_xScaleValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale] );
								break;
							case MS_Local:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale], true );
								break;
							case MS_Relative:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale], true );
								break;
						}
						break;
					case MT_Rotate:
						m_xRotaValue = std::floor(m_xRotaValue + sign*1+0.5);
						m_ui->lineEditX->setText( QString("%L1").arg(m_xRotaValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								rotate( QVector3D ( sign, m_yRotaValue, m_zRotaValue ),
										m_modificationState[MT_Rotate] );
								break;
							case MS_Local:
								rotate( QVector3D ( sign, 0, 0 ),
										m_modificationState[MT_Rotate] );
								break;
							case MS_Relative:
								rotate( QVector3D ( sign, 0, 0 ),
										m_modificationState[MT_Rotate] );
								break;
						}
						break;
				}
			}
		}
		else if (target == m_ui->lineEditY){
			if (m_ui->lineEditY->isValid()) {
				int sign = -1;
				if ( wheelEvent->angleDelta().ry()>0 )
					sign = 1;

				switch (m_modificationType) {
					case MT_Translate:
						m_yTransValue = std::floor( (m_yTransValue + sign*0.01+0.005)*100)/100;
						m_ui->lineEditY->setText( QString("%L1").arg(m_yTransValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								translate( QVector3D ( m_xTransValue, m_yTransValue, m_zTransValue ),
										   m_modificationState[MT_Translate] );
								break;
							case MS_Relative:
								translate( QVector3D ( 0, sign*0.01, 0 ),
										   m_modificationState[MT_Translate] );
								break;
							case MS_Local:
								translate( QVector3D ( 0, sign*0.01, 0),
										   m_modificationState[MT_Translate] );
								break;
						}
						break;
					case MT_Scale:
						m_yScaleValue = std::floor( (m_yScaleValue + sign*0.1+0.05)*10)/10;
						m_ui->lineEditY->setText( QString("%L1").arg(m_yScaleValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale] );
								break;
							case MS_Local:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale], true );
								break;
							case MS_Relative:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale], true );
								break;
						}
						break;
					case MT_Rotate:
						m_yRotaValue = std::floor(m_yRotaValue + sign*1+0.5);
						m_ui->lineEditY->setText( QString("%L1").arg(m_yRotaValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								rotate( QVector3D ( 0, sign, 0 ),
										m_modificationState[MT_Rotate] );
								break;
							case MS_Local:
								rotate( QVector3D (0, sign, 0 ),
										m_modificationState[MT_Rotate] );
								break;
							case MS_Relative:
								rotate( QVector3D ( 0, sign, 0 ),
										m_modificationState[MT_Rotate] );
								break;
						}
						break;
				}
			}
		}
		else if (target == m_ui->lineEditZ){
			if (m_ui->lineEditZ->isValid()) {
				int sign = -1;
				if ( wheelEvent->delta()>0 )
					sign = 1;

				switch (m_modificationType) {
					case MT_Translate:
						m_zTransValue = std::floor( (m_zTransValue + sign*0.01+0.005)*100 )/100;
						m_ui->lineEditZ->setText( QString("%L1").arg(m_zTransValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								translate( QVector3D ( m_xTransValue, m_yTransValue, m_zTransValue ),
										   m_modificationState[MT_Translate] );
								break;
							case MS_Relative:
								translate( QVector3D ( 0, 0, sign*0.01 ),
										   m_modificationState[MT_Translate] );
								break;
							case MS_Local:
								translate( QVector3D ( 0, 0, sign*0.01 ),
										   m_modificationState[MT_Translate] );
								break;
						}
						break;
					case MT_Scale:
						m_zScaleValue = std::floor( (m_zScaleValue + sign*0.1+0.05)*10 )/10;
						m_ui->lineEditZ->setText( QString("%L1").arg(m_zScaleValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale] );
								break;
							case MS_Local:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale], true );
								break;
							case MS_Relative:
								scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
									   m_modificationState[MT_Scale], true );
								break;
						}
						break;
					case MT_Rotate:
						m_zRotaValue = std::floor(m_zRotaValue + sign*1+0.5);
						m_ui->lineEditZ->setText( QString("%L1").arg(m_zRotaValue,0, 'f', 3) );
						switch (state) {
							case MS_Absolute:
								rotate( QVector3D ( m_xRotaValue, m_yRotaValue, m_zRotaValue ),
										m_modificationState[MT_Rotate] );
								break;
							case MS_Local:
								rotate( QVector3D ( 0, 0, sign ),
										m_modificationState[MT_Rotate] );
								break;
							case MS_Relative:
								rotate( QVector3D ( 0, 0, sign ),
										m_modificationState[MT_Rotate] );
								break;
						}
						break;
				}
			}
		}

	}
	return false;
}

void SVPropEditGeometry::on_lineEditX_returnPressed()
{
	if ( m_ui->lineEditX->isValid()){
		double tempXValue = m_ui->lineEditX->value();
		switch ( m_modificationType ) {
			case ( MT_Translate ): {
				m_xTransValue = tempXValue;
				translate( QVector3D ( m_xTransValue, m_yTransValue, m_zTransValue ),
						   m_modificationState[MT_Translate] );
				switch ( m_modificationState[m_modificationType] ) {
					case MS_Local:
					case MS_Relative:
						m_ui->lineEditX->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
						m_ui->lineEditY->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
						m_ui->lineEditZ->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
						break;
				}
				break;
			}
			case ( MT_Rotate ): {
				m_xRotaValue = tempXValue;
				rotate( QVector3D ( m_xRotaValue, m_yRotaValue, m_zRotaValue ),
						m_modificationState[MT_Rotate]);
				break;
			}
			case MT_Scale: {

				m_xScaleValue = tempXValue;

				// reset line edits
				switch (m_modificationState[MT_Scale]) {
					case MS_Absolute: {
						scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
							   m_modificationState[MT_Scale] );
						IBKMK::Vector3D boundingBox;
						std::vector<const VICUS::Surface*> surfaces;
						project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
						project().boundingBox(surfaces, boundingBox);

//						m_ui->lineEditX->setText( QString("%L1").arg( boundingBox.m_x,0, 'f', 3 ) );
//						m_ui->lineEditY->setText( QString("%L1").arg( boundingBox.m_y,0, 'f', 3 ) );
//						m_ui->lineEditZ->setText( QString("%L1").arg( boundingBox.m_z,0, 'f', 3 ) );

						break;
					}
					case MS_Relative:
					case MS_Local:
						scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
							   m_modificationState[MT_Scale], true );
						setRelativeScalingSurfaces();
						m_ui->lineEditX->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						m_ui->lineEditY->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						m_ui->lineEditZ->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						break;
				}
				break;
			}
		}
	}
}

void SVPropEditGeometry::on_lineEditY_returnPressed()
{
	if ( m_ui->lineEditY->isValid()){
		double tempYValue = m_ui->lineEditY->value();
		switch ( m_modificationType ) {
			case ( MT_Translate ):
				m_yTransValue = tempYValue;
				translate( QVector3D ( m_xTransValue, m_yTransValue, m_zTransValue ),
						   m_modificationState[MT_Translate] );
				break;
			case ( MT_Rotate ):
				m_yRotaValue = tempYValue;
				rotate( QVector3D ( m_xRotaValue, m_yRotaValue, m_zRotaValue ),
						m_modificationState[MT_Rotate]);
				break;
			case MT_Scale: {

				m_yScaleValue = tempYValue;

				// reset line edits
				switch (m_modificationState[MT_Scale]) {
					case MS_Absolute: {
						scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
							   m_modificationState[MT_Scale] );
						IBKMK::Vector3D center;
						std::vector<const VICUS::Surface*> surfaces;
						project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
						project().boundingBox(surfaces, center);

//						m_ui->lineEditX->setText( QString("%L1").arg( center.m_x,0, 'f', 3 ) );
//						m_ui->lineEditY->setText( QString("%L1").arg( center.m_y,0, 'f', 3 ) );
//						m_ui->lineEditZ->setText( QString("%L1").arg( center.m_z,0, 'f', 3 ) );

						break;
					}
					case MS_Relative:
					case MS_Local:
						scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
							   m_modificationState[MT_Scale], true );
						setRelativeScalingSurfaces();
						m_ui->lineEditX->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						m_ui->lineEditY->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						m_ui->lineEditZ->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						break;
				}
				break;
			}
		}
	}
}

void SVPropEditGeometry::on_lineEditZ_returnPressed()
{
	if ( m_ui->lineEditZ->isValid()){
		double tempZValue = m_ui->lineEditZ->value();
		switch ( m_modificationType ) {
			case ( MT_Translate ):
				m_zTransValue = tempZValue;
				translate( QVector3D ( m_xTransValue, m_yTransValue, m_zTransValue ),
						   m_modificationState[MT_Translate] );
				break;
			case ( MT_Rotate ):
				m_zRotaValue = tempZValue;
				rotate( QVector3D ( m_xRotaValue, m_yRotaValue, m_zRotaValue ),
						m_modificationState[MT_Rotate]);
				break;
			case MT_Scale: {

				m_zScaleValue = tempZValue;

				// reset line edits
				switch (m_modificationState[MT_Scale]) {
					case MS_Absolute: {
						scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
							   m_modificationState[MT_Scale] );
						IBKMK::Vector3D center;
						std::vector<const VICUS::Surface*> surfaces;
						project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
						project().boundingBox(surfaces, center);

//						m_ui->lineEditX->setText( QString("%L1").arg( center.m_x,0, 'f', 3 ) );
//						m_ui->lineEditY->setText( QString("%L1").arg( center.m_y,0, 'f', 3 ) );
//						m_ui->lineEditZ->setText( QString("%L1").arg( center.m_z,0, 'f', 3 ) );

						break;
					}
					case MS_Relative:
					case MS_Local:
						scale( QVector3D ( m_xScaleValue, m_yScaleValue, m_zScaleValue ),
							   m_modificationState[MT_Scale], true );
						setRelativeScalingSurfaces();
						m_ui->lineEditX->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						m_ui->lineEditY->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						m_ui->lineEditZ->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
						break;
				}
				break;
			}
		}
	}
}




void SVPropEditGeometry::setState(const SVPropEditGeometry::ModificationType & type, const SVPropEditGeometry::ModificationState & state,
								  const bool &updateComboBox)
{

	setToolButton(type);

	if ( updateComboBox ) {
		setComboBox(type, state);
		m_modificationState[type] = state;
	}

	switch (type) {
		case MT_Translate : {
			showDeg(false);
			showRotation(false);
			switch (state) {

				case MS_Absolute:

					m_ui->labelX->setText("X");
					m_ui->labelY->setText("Y");
					m_ui->labelZ->setText("Z");

					// set Current position
					m_xTransValue = m_localCoordinatePosition.translation().x();
					m_yTransValue = m_localCoordinatePosition.translation().y();
					m_zTransValue = m_localCoordinatePosition.translation().z();

					m_ui->lineEditX->setText( QString("%L1").arg( m_localCoordinatePosition.translation().x(),0, 'f', 3 ) );
					m_ui->lineEditY->setText( QString("%L1").arg( m_localCoordinatePosition.translation().y(),0, 'f', 3 ) );
					m_ui->lineEditZ->setText( QString("%L1").arg( m_localCoordinatePosition.translation().z(),0, 'f', 3 ) );
					break;
				default:

					m_ui->labelX->setText("ΔX");
					m_ui->labelY->setText("ΔY");
					m_ui->labelZ->setText("ΔZ");


					m_ui->lineEditX->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
					m_ui->lineEditY->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
					m_ui->lineEditZ->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );

			}
		}
			break;
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
					m_ui->horizontalLayoutAbsRotate->setEnabled(true);

					std::vector<const VICUS::Surface*> surfaces;

					IBKMK::Vector3D centerPoint (0,0,0);
					IBKMK::Vector3D centerPointLocal;

					project().selectedSurfaces(surfaces, VICUS::Project::SG_All);

					m_ui->labelRotateInclinationAbs->setEnabled(true);
					m_ui->labelRotateOrientationAbs->setEnabled(true);
					m_ui->lineEditInclination->setEnabled(true);
					m_ui->lineEditOrientation->setEnabled(true);

					if ( surfaces.size() == 1 )
						const VICUS::Surface* s = surfaces[0];
					else
						setRotation(QtExt::QVector2IBKVector(SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis() ) );


					if ( updateComboBox ) {
						m_ui->lineEditX->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
						m_ui->lineEditY->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
						m_ui->lineEditZ->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
					}

				}
					break;

				default:
					m_ui->lineEditX->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
					m_ui->lineEditY->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );
					m_ui->lineEditZ->setText( QString("%L1").arg( 0.0,0, 'f', 3 ) );

			}
		}
			break;
		case MT_Scale: {
			showDeg(false);
			showRotation(false);

			switch (state) {
				case MS_Absolute: {

					m_ui->labelX->setText("L<sub>X</sub>");
					m_ui->labelY->setText("W<sub>Y</sub>");
					m_ui->labelZ->setText("H<sub>Z</sub>");

					IBKMK::Vector3D centerPoint;
					std::vector<const VICUS::Surface*> surfaces;
					project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
					IBKMK::Vector3D v = project().boundingBox(surfaces, centerPoint);
					m_ui->lineEditX->setText( QString("%L1").arg( v.m_x,0, 'f', 3 ) );
					m_ui->lineEditY->setText( QString("%L1").arg( v.m_y,0, 'f', 3 ) );
					m_ui->lineEditZ->setText( QString("%L1").arg( v.m_z,0, 'f', 3 ) );

					m_xScaleValue = v.m_x;
					m_yScaleValue = v.m_y;
					m_zScaleValue = v.m_z;

					break;
				}
				default:
					m_ui->labelX->setText("X");
					m_ui->labelY->setText("Y");
					m_ui->labelZ->setText("Z");

					setRelativeScalingSurfaces();

					m_xScaleValue = 1.0;
					m_yScaleValue = 1.0;
					m_zScaleValue = 1.0;

					m_ui->lineEditX->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
					m_ui->lineEditY->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );
					m_ui->lineEditZ->setText( QString("%L1").arg( 1.0,0, 'f', 3 ) );

			}
		}
			break;
	}
}

void SVPropEditGeometry::setToolButton(const SVPropEditGeometry::ModificationType & type)
{
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
	}
}

void SVPropEditGeometry::setComboBox(const ModificationType & type, const ModificationState & state)
{
	m_ui->comboBox->clear();

	switch (type) {
		case MT_Translate:
			m_ui->comboBox->addItem( tr("move to position:") );
			m_ui->comboBox->addItem( tr("move relative using global coordinate system:") );
			m_ui->comboBox->addItem( tr("move relative using local coordinate system:") );
			break;
		case MT_Rotate:
			m_ui->comboBox->addItem( tr("rotate absolute:") );
			m_ui->comboBox->addItem( tr("rotate relative to center of each surface:") );
			m_ui->comboBox->addItem( tr("rotate relative using local coordinate system:") );
			break;
		case MT_Scale:
			m_ui->comboBox->addItem( tr("resize surfaces:") );
			m_ui->comboBox->addItem( tr("scale relative to center of each surface:") );
			m_ui->comboBox->addItem( tr("scale relative to local coordinate system:") );
			break;
	}

	m_ui->comboBox->setCurrentIndex((int)state);
}

void SVPropEditGeometry::setRelativeScalingSurfaces()
{
	std::vector<const VICUS::Surface*> surfaces;
	project().selectedSurfaces(surfaces, VICUS::Project::SG_All);
	for (const VICUS::Surface *s : surfaces ) {
		VICUS::Surface vs(*s);
		m_relScaleSurfaces.push_back(vs);
	}
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
	if ( abs ) {
		m_ui->labelRotateInclinationAbs->show();
		m_ui->labelRotateOrientationAbs->show();
		m_ui->lineEditInclination->show();
		m_ui->lineEditOrientation->show();

		m_ui->labelIndication->show();
		m_ui->labelOrientationDeg->show();
		m_ui->labelInclinationDeg->show();

		m_ui->labelX->hide();
		m_ui->labelY->hide();
		m_ui->labelZ->hide();
		m_ui->lineEditX->hide();
		m_ui->lineEditY->hide();
		m_ui->lineEditZ->hide();

	}
	else {
		m_ui->labelRotateInclinationAbs->hide();
		m_ui->labelRotateOrientationAbs->hide();
		m_ui->lineEditInclination->hide();
		m_ui->lineEditOrientation->hide();

		m_ui->labelIndication->hide();
		m_ui->labelOrientationDeg->hide();
		m_ui->labelInclinationDeg->hide();

		m_ui->labelX->show();
		m_ui->labelY->show();
		m_ui->labelZ->show();
		m_ui->lineEditX->show();
		m_ui->lineEditY->show();
		m_ui->lineEditZ->show();
	}
}

void SVPropEditGeometry::on_toolButtonTrans_clicked()
{
	setState(MT_Translate, m_modificationState[MT_Translate], true);
}

void SVPropEditGeometry::on_toolButtonRotate_clicked()
{
	setState(MT_Rotate, m_modificationState[MT_Rotate], true);
}

void SVPropEditGeometry::on_toolButtonScale_clicked()
{
	setState(MT_Scale, m_modificationState[MT_Rotate], true);
}

void SVPropEditGeometry::on_lineEditOrientation_returnPressed()
{
	if ( m_ui->lineEditOrientation->isValid() && m_ui->lineEditInclination->isValid() ) {
		double oriRad = m_ui->lineEditOrientation->value() * IBK::DEG2RAD;
		double incliRad = m_ui->lineEditInclination->value() * IBK::DEG2RAD;

		m_orientation = oriRad / IBK::DEG2RAD;
		m_inclination = incliRad / IBK::DEG2RAD;

		QVector3D newNormal (std::sin( oriRad ) * std::sin( incliRad ),
							 std::cos( oriRad ) * std::sin( incliRad ),
							 std::cos( incliRad ) );
		rotate(newNormal, m_modificationState[m_modificationType]);

	}

}

void SVPropEditGeometry::on_lineEditInclination_returnPressed()
{
	if ( m_ui->lineEditOrientation->isValid() && m_ui->lineEditInclination->isValid() ) {
		double oriRad = m_ui->lineEditOrientation->value() * IBK::DEG2RAD;
		double incliRad = m_ui->lineEditInclination->value() * IBK::DEG2RAD;

		m_orientation = oriRad / IBK::DEG2RAD;
		m_inclination = incliRad / IBK::DEG2RAD;

		QVector3D newNormal (std::sin( oriRad ) * std::sin( incliRad ),
							 std::cos( oriRad ) * std::sin( incliRad ),
							 std::cos( incliRad ) );
		rotate(newNormal, m_modificationState[m_modificationType]);
	}
}

void SVPropEditGeometry::on_lineEditOrientation_editingFinished()
{
	if ( m_ui->lineEditOrientation->isValid() )
		m_orientation = m_ui->lineEditOrientation->value();
}

void SVPropEditGeometry::on_lineEditInclination_editingFinished()
{
	if ( m_ui->lineEditInclination->isValid() )
		m_orientation = m_ui->lineEditInclination->value();
}


void SVPropEditGeometry::on_lineEditCopyX_returnPressed()
{
	if ( m_ui->lineEditCopyX->isValid() )
		m_xCopyValue = m_ui->lineEditCopyX->value();

	int count = m_ui->spinBoxCount->value();

	for ( size_t i = 0; i<count; ++i ) {
		copy(QVector3D (i*m_xCopyValue,
						i*m_yCopyValue,
						i*m_zCopyValue) );
	}

}

void SVPropEditGeometry::on_lineEditCopyY_returnPressed()
{

}

void SVPropEditGeometry::on_lineEditCopyZ_returnPressed()
{

}

void SVPropEditGeometry::on_lineEditCopyX_editingFinished()
{
	if ( m_ui->lineEditCopyX->isValid() )
		m_xCopyValue = m_ui->lineEditCopyX->value();
}

void SVPropEditGeometry::on_lineEditCopyY_editingFinished()
{
	if ( m_ui->lineEditCopyY->isValid() )
		m_yCopyValue = m_ui->lineEditCopyY->value();
}

void SVPropEditGeometry::on_lineEditCopyZ_editingFinished()
{
	if ( m_ui->lineEditCopyZ->isValid() )
		m_zCopyValue = m_ui->lineEditCopyZ->value();
}
