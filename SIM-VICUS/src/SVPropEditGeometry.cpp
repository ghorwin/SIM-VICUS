#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

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

SVPropEditGeometry::SVPropEditGeometry(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::SVPropEditGeometry)
{
	m_ui->setupUi(this);
	layout()->setMargin(0);
	SVViewStateHandler::instance().m_coordinateSystemObject->m_propEditGeometry = this;
	SVViewStateHandler::instance().m_propEditGeometryWidget = this;
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


void SVPropEditGeometry::setBoundingBox(const IBKMK::Vector3D &v) {
//	QVector3D tmpScale ( m_ui->doubleSpinBoxScaleX->value(), m_ui->doubleSpinBoxScaleX->value() )

	if ( m_ui->radioButtonScaleAbsolute->isChecked() ) {
		m_ui->doubleSpinBoxScaleX->setValue( v.m_x );
		m_ui->doubleSpinBoxScaleY->setValue( v.m_y );
		m_ui->doubleSpinBoxScaleZ->setValue( v.m_z );
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
	project().selectedSurfaces(surfaces);

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
		project().haveSelectedSurfaces(centerPoint);
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
#if 0
	// now we update all selected surfaces
	Vic3D::Transform3D trans;
	QVector3D scaleVec (	m_ui->doubleSpinBoxScaleX->value(),
							m_ui->doubleSpinBoxScaleY->value(),
							m_ui->doubleSpinBoxScaleZ->value() );

	std::vector<VICUS::Surface*> surfaces;
	IBKMK::Vector3D centerPoint;
	IBKMK::Vector3D centerPointLocal;
	IBKMK::Vector3D boundingBox;
	p.selectedSurfaces(surfaces);
	p.haveSelectedSurfaces(centerPointLocal);
	p.boundingBoxofSelectedSurfaces(boundingBox);

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
				vs.push_back( VICUS::QVector2IBKVector( t.translation() ) );
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
				QVector3D p = VICUS::IBKVector2QVector(centerPointLocal) + localScaleFactorX * scaleVec.x() * xAxis
																		 + localScaleFactorY * scaleVec.y() * yAxis
																		 + localScaleFactorZ * scaleVec.z() * zAxis;
				t.setTranslation( p );
				vs.push_back( VICUS::QVector2IBKVector( t.translation() ) );
			}
			s->m_geometry.setVertexes(vs);
		}
	}
	else {

		for ( VICUS::Surface* s : surfaces ) {
			std::vector<IBKMK::Vector3D> vs;
			for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
				Vic3D::Transform3D t;
				QVector3D newScale;

				newScale.setX( ( boundingBox.m_x == 0.0 ) ?  0 : ( scaleVec.x() / boundingBox.m_x ) );
				newScale.setY( ( boundingBox.m_y == 0.0 ) ?  0 : ( scaleVec.y() / boundingBox.m_y ) );
				newScale.setZ( ( boundingBox.m_z == 0.0 ) ?  0 : ( scaleVec.z() / boundingBox.m_z ) );

				t.setTranslation( centerPointLocal.m_x + newScale.x() * ( v.m_x - centerPointLocal.m_x ),
								  centerPointLocal.m_y + newScale.y() * ( v.m_y - centerPointLocal.m_y),
								  centerPointLocal.m_z + newScale.z() * ( v.m_z - centerPointLocal.m_z) );
				vs.push_back( VICUS::QVector2IBKVector( t.translation() ) );
			}
			s->m_geometry.setVertexes(vs);
		}

	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), surfaces );
	undo->push();
#endif
}


void SVPropEditGeometry::on_pushButtonRotate_clicked() {
	// TODO : Stephan, fix this like on_pushButtonTranslate_clicked()
#if 0
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
#endif
}


void SVPropEditGeometry::on_radioButtonScaleAbsolute_toggled(bool /*abs*/) {
	IBKMK::Vector3D v;

	project().boundingBoxofSelectedSurfaces(v);

	setBoundingBox(v);
}


