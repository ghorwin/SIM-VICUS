#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

#include "VICUS_Project.h"
#include "VICUS_Conversions.h"

#include "SVViewStateHandler.h"
#include "SVProjectHandler.h"
#include "SVUndoModifySurfaceGeometry.h"

#include "Vic3DNewPolygonObject.h"
#include "Vic3DCoordinateSystemObject.h"
#include "Vic3DTransform3D.h"

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);
	layout()->setMargin(0);
	SVViewStateHandler::instance().m_coordinateSystemObject->m_propEditGeometry = this;
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
	m_ui->lineEditXValue->setText( QString("%L1").arg( t.translation().x() ) );
	m_ui->lineEditYValue->setText( QString("%L1").arg( t.translation().y() ) );
	m_ui->lineEditZValue->setText( QString("%L1").arg( t.translation().z() ) );
}


void SVPropEditGeometry::on_pushButtonAddPolygon_clicked() {
	// reset new polygon object
	SVViewStateHandler::instance().m_newPolygonObject->clear();
	// signal, that we want to start adding a new polygon
	SVViewState vs = SVViewStateHandler::instance().viewState();
	vs.m_sceneOperationMode = SVViewState::OM_PlaceVertex;
	vs.m_propertyWidgetMode = SVViewState::PM_VertexList;
	// now tell all UI components to toggle their view state
	SVViewStateHandler::instance().setViewState(vs);
}


void SVPropEditGeometry::on_toolButtonAddZoneBox_clicked() {
	// configure the view to go into "place first vertex" mode,
	// toggle property widget to show "placed vertexes" widget


}

void SVPropEditGeometry::on_pushButtonTranslate_clicked()
{
	// now we update all selected surfaces
	VICUS::Project p = project();
	Vic3D::Transform3D trans;
	QVector3D transVec ( m_ui->doubleSpinBoxTranslateX->value(),
						 m_ui->doubleSpinBoxTranslateY->value(),
						 m_ui->doubleSpinBoxTranslateZ->value() );

	std::vector<VICUS::Surface*> surfaces;
	IBKMK::Vector3D centerPoint;
	p.selectedSurfaces(surfaces);

	if ( m_ui->radioButtonRelative->isChecked() ) {
		for ( VICUS::Surface* s : surfaces ) {
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				// use just this instead of making a QVetor3D
				t.setTranslation( v.m_x, v.m_y, v.m_z );
				t.translate( transVec );
				vs.push_back( IBKMK::Vector3D ( t.translation().x(), t.translation().y(), t.translation().z() ) );
			}
			s->m_geometry.setVertexes(vs);
		}
	}
	else if ( m_ui->radioButtonLocal->isChecked() ) {
		for ( VICUS::Surface* s : surfaces ) {
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
			s->m_geometry.setVertexes(vs);
		}
	}
	else {
		p.haveSelectedSurfaces(centerPoint);
		for ( VICUS::Surface* s : surfaces ) {
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				t.setTranslation( transVec.x() + ( v.m_x - centerPoint.m_x ),
								  transVec.y() + ( v.m_y - centerPoint.m_y),
								  transVec.z() + ( v.m_z - centerPoint.m_z) );
				vs.push_back( IBKMK::Vector3D ( t.translation().x(), t.translation().y(), t.translation().z() ) );
			}
			s->m_geometry.setVertexes(vs);
		}
	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), surfaces );
	undo->push();


}

void SVPropEditGeometry::on_pushButtonScale_clicked()
{
	// now we update all selected surfaces
	VICUS::Project p = project();
	Vic3D::Transform3D trans;
	QVector3D scaleVec (	m_ui->doubleSpinBoxScaleX->value(),
							m_ui->doubleSpinBoxScaleY->value(),
							m_ui->doubleSpinBoxScaleZ->value() );

	std::vector<VICUS::Surface*> surfaces;
	IBKMK::Vector3D centerPoint;
	IBKMK::Vector3D centerPointLocal;
	p.selectedSurfaces(surfaces);
	p.haveSelectedSurfaces(centerPointLocal);

	// check if scale factor is not Null
	if ( IBK::nearly_equal<3>( scaleVec.length(), 0.0 ) )
		return;

	if ( m_ui->radioButtonScaleRelative->isChecked() ) {
		for ( VICUS::Surface* s : surfaces ) {
			centerPoint = s->m_geometry.centerPoint();
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				t.setTranslation( centerPoint.m_x + scaleVec.x() * ( v.m_x - centerPoint.m_x ),
								  centerPoint.m_y + scaleVec.y() * ( v.m_y - centerPoint.m_y),
								  centerPoint.m_z + scaleVec.z() * ( v.m_z - centerPoint.m_z) );
				vs.push_back( IBKMK::Vector3D ( t.translation().x(), t.translation().y(), t.translation().z() ) );
			}
			s->m_geometry.setVertexes(vs);
		}
	}
	else if ( m_ui->radioButtonScaleLocal->isChecked() ) {

		QVector3D xAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis();
		QVector3D yAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localYAxis();
		QVector3D zAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis();

		for ( VICUS::Surface* s : surfaces ) {
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				QVector3D v3D ( VICUS::IBKVector2QVector(v) );
				double localScaleFactorX = ( v.m_x - centerPointLocal.m_x ) / ( IBK::nearly_equal<4>(xAxis.x(), 0.0) ? 1E10 : xAxis.x() ) +
										   ( v.m_y - centerPointLocal.m_y ) / ( IBK::nearly_equal<4>(yAxis.x(), 0.0) ? 1E10 : yAxis.x() ) +
										   ( v.m_z - centerPointLocal.m_z ) / ( IBK::nearly_equal<4>(zAxis.x(), 0.0) ? 1E10 : zAxis.x() );

				double localScaleFactorY = ( v.m_x - centerPointLocal.m_x ) / ( IBK::nearly_equal<4>(xAxis.y(), 0.0) ? 1E10 : xAxis.y() ) +
										   ( v.m_y - centerPointLocal.m_y ) / ( IBK::nearly_equal<4>(yAxis.y(), 0.0) ? 1E10 : yAxis.y() ) +
										   ( v.m_z - centerPointLocal.m_z ) / ( IBK::nearly_equal<4>(zAxis.y(), 0.0) ? 1E10 : zAxis.y() );

				double localScaleFactorZ = ( v.m_x - centerPointLocal.m_x ) / ( IBK::nearly_equal<4>(xAxis.z(), 0.0) ? 1E10 : xAxis.z() ) +
										   ( v.m_y - centerPointLocal.m_y ) / ( IBK::nearly_equal<4>(yAxis.z(), 0.0) ? 1E10 : yAxis.z() ) +
										   ( v.m_z - centerPointLocal.m_z ) / ( IBK::nearly_equal<4>(zAxis.z(), 0.0) ? 1E10 : zAxis.z() );
				// t.scale(localScaleFactorX*scaleVec.x(), localScaleFactorY*scaleVec.y(), localScaleFactorZ*scaleVec.z());

				double x = centerPointLocal.m_x + scaleVec.x() * ( localScaleFactorX * xAxis.x() + localScaleFactorY * yAxis.x() + localScaleFactorZ * zAxis.x() );
				double y = centerPointLocal.m_y + scaleVec.y() * ( localScaleFactorX * xAxis.y() + localScaleFactorY * yAxis.y() + localScaleFactorZ * zAxis.y() );
				double z = centerPointLocal.m_z + scaleVec.z() * ( localScaleFactorX * xAxis.z() + localScaleFactorY * yAxis.z() + localScaleFactorZ * zAxis.z() );

				t.setTranslation( x, y, z );
				vs.push_back( VICUS::QVector2IBKVector( t.translation() ) );
			}
			s->m_geometry.setVertexes(vs);
		}
	}
	else {



	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), surfaces );
	undo->push();
}

void SVPropEditGeometry::on_pushButtonRotate_clicked()
{
	// now we update all selected surfaces
	VICUS::Project p = project();
	Vic3D::Transform3D trans;
	QVector3D rotateVec ( m_ui->doubleSpinBoxRotateX->value(),
						  m_ui->doubleSpinBoxRotateY->value(),
						  m_ui->doubleSpinBoxRotateZ->value() );

	std::vector<VICUS::Surface*> surfaces;
	IBKMK::Vector3D centerPoint (0,0,0);
	IBKMK::Vector3D centerPointLocal;

	p.selectedSurfaces(surfaces);
	p.haveSelectedSurfaces(centerPointLocal);

	for ( VICUS::Surface* s : surfaces ) {
		std::vector<IBKMK::Vector3D> vs;

		IBKMK::Vector3D normal = s->m_geometry.normal();
		for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
			Vic3D::Transform3D tTrans, tRota;

			// translate point back to coordinate center
			QVector3D v3D ( VICUS::IBKVector2QVector(v) );

			if ( m_ui->radioButtonRotateAbsolute->isChecked() ) {

				tTrans.setTranslation( VICUS::IBKVector2QVector(-1*centerPoint) );
				v3D = tTrans.toMatrix() * v3D;

				// rotate around specified axis
				if ( !IBK::nearly_equal<3>( rotateVec.x(), 0.0 ) )
					tRota.rotate( rotateVec.x(), 1, 0, 0 );
				if ( !IBK::nearly_equal<3>( rotateVec.y(), 0.0 ) )
					tRota.rotate( rotateVec.y(), 0, 1, 0 );
				if ( !IBK::nearly_equal<3>( rotateVec.z(), 0.0 ) )
					tRota.rotate( rotateVec.z(), 0, 0, 1 );
				v3D = tRota.toMatrix() * v3D;

			} else if ( m_ui->radioButtonRotateLocal->isChecked() ) {

				centerPoint = centerPointLocal;

				tTrans.setTranslation( VICUS::IBKVector2QVector(-1*centerPoint) );
				v3D = tTrans.toMatrix() * v3D;

				QVector3D xAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localXAxis();
				QVector3D yAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localYAxis();
				QVector3D zAxis = SVViewStateHandler::instance().m_coordinateSystemObject->localZAxis();

				// rotate around specified axis
				if ( !IBK::nearly_equal<3>( rotateVec.x(), 0.0 ) )
					tRota.rotate( rotateVec.x(), xAxis );
				if ( !IBK::nearly_equal<3>( rotateVec.y(), 0.0 ) )
					tRota.rotate( rotateVec.y(), yAxis );
				if ( !IBK::nearly_equal<3>( rotateVec.z(), 0.0 ) )
					tRota.rotate( rotateVec.z(), zAxis );
				v3D = tRota.toMatrix() * v3D;
			}

			else {
				centerPoint = s->m_geometry.centerPoint();

				tTrans.setTranslation( VICUS::IBKVector2QVector(-1*centerPoint) );
				v3D = tTrans.toMatrix() * v3D;

				// rotate around specified axis
				if ( !IBK::nearly_equal<3>( rotateVec.x(), 0.0 ) )
					tRota.rotate( rotateVec.x(), VICUS::IBKVector2QVector(s->m_geometry.localX() ) );
				if ( !IBK::nearly_equal<3>( rotateVec.y(), 0.0 ) )
					tRota.rotate( rotateVec.y(), VICUS::IBKVector2QVector(s->m_geometry.localY() ) );
				if ( !IBK::nearly_equal<3>( rotateVec.z(), 0.0 ) )
					tRota.rotate( rotateVec.z(), normal.m_x, normal.m_y, normal.m_z );
				v3D = tRota.toMatrix() * v3D;
			}
			// translatae back to original center point
			tTrans.setTranslation( centerPoint.m_x, centerPoint.m_y, centerPoint.m_z );
			v3D = tTrans.toMatrix() * v3D;
			vs.push_back( IBKMK::Vector3D( v3D.x(), v3D.y(), v3D.z() ) );
		}
		s->m_geometry.setVertexes(vs);

	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), surfaces );
	undo->push();
}
