#include "SVPropEditGeometry.h"
#include "ui_SVPropEditGeometry.h"

#include "VICUS_Project.h"

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
	QVector3D transVec ( m_ui->lineEditXValueTrans->text().toDouble(),
							m_ui->lineEditYValueTrans->text().toDouble(),
							m_ui->lineEditZValueTrans->text().toDouble() );

	std::vector<VICUS::Surface*> surfaces;
	p.selectedSurfaces(surfaces);

	for ( VICUS::Surface* s : surfaces ) {
		std::vector<IBKMK::Vector3D> vs;
		for ( IBKMK::Vector3D v : s->m_geometry.vertexes() ) {
			Vic3D::Transform3D t;
			t.setTranslation( v.m_x, v.m_y, v.m_z );
			t.translate( transVec );
			vs.push_back( IBKMK::Vector3D ( t.translation().x(), t.translation().y(), t.translation().z() ) );
		}
		s->m_geometry.setVertexes(vs);
	}

	SVUndoModifySurfaceGeometry * undo = new SVUndoModifySurfaceGeometry(tr("modified surfaces"), surfaces );
	undo->push();

	SVProjectHandler::instance().setModified( SVProjectHandler::GeometryChanged);
	SVProjectHandler::instance().setModified( SVProjectHandler::SelectionModified);
}
